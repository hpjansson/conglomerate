/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-font.c
 *
 * Copyright (C) 2002 David Malcolm
 *
 * Conglomerate is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Conglomerate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 * Based on code by Hans Petter Jansson <hpj@ximian.com>
 */

#include "global.h"
#include "cong-font.h"

#define SUPPORT_GDK_FONTS 1

struct CongFont
{
	PangoFontDescription *font_desc;
	PangoFont *pango_font;
	PangoLayout *pango_layout;
#if SUPPORT_GDK_FONTS
	GdkFont *gdk_font;
	int asc;
	int desc;
#endif	
};

const char font_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                          "abcdefghijklmnopqrstuvwxyz"
                          "0123456789!@\"'/()[]{}*&#~";

CongFont*
cong_font_load(const gchar *font_name)
{
	CongFont *font;
	PangoFontMetrics *font_metrics;
	g_return_val_if_fail(font_name, NULL);

	font = g_new0(CongFont,1);

	font->font_desc = pango_font_description_from_string (font_name);
	g_assert (font->font_desc);

	font->pango_font = pango_context_load_font(gdk_pango_context_get(),
						   font->font_desc);

	/* We have a shared pango layout for all usage of this font */
	font->pango_layout = pango_layout_new(gdk_pango_context_get());

	pango_layout_set_font_description(font->pango_layout,
					  font->font_desc);
	
	/* Turn off line wrapping for the PangoLayout: */
	pango_layout_set_width(font->pango_layout,-1);
	
#if SUPPORT_GDK_FONTS
	font->gdk_font = gdk_font_from_description (font->font_desc);
	/* FIXME: add expection handling when font is not on system */
	g_assert(font->gdk_font);

	gdk_string_extents(font->gdk_font, font_chars, 0, 0, 0, &font->asc, &font->desc);                  
#else
#if 0
	font_metrics = pango_font_get_metrics(font->pango_font,
					      NULL);
	g_assert(font_metrics);
	font->asc = pango_font_metrics_get_ascent(font_metrics)/PANGO_SCALE;
	font->desc = pango_font_metrics_get_descent(font_metrics)/PANGO_SCALE;
	pango_font_metrics_unref(font_metrics);	
#endif
#endif

	return font;	
}

void
cong_font_delete(CongFont *font)
{
	g_assert(0); /* unimplemented */
}

gint
cong_font_get_height(CongFont *font,
		     const gchar *text) 
{
	gint result;

	g_return_val_if_fail(font,0);
	g_assert(text);
	g_return_val_if_fail(text,0);

#if SUPPORT_GDK_FONTS
	return font->asc + font->desc;
#else
	pango_layout_set_text(font->pango_layout,
			      text,
			      -1);

	pango_layout_get_pixel_size(font->pango_layout,
				    NULL, 
				    &result);
	
	return result;
#endif
}

PangoFontDescription*
cong_font_get_pango_description(CongFont *font)
{
	g_return_val_if_fail(font,NULL);

	return font->font_desc;	
}

void
cong_font_draw_string_slow(GdkDrawable *drawable,
			   CongFont *font, 
			   GdkGC *gc,
			   const gchar *text,
			   gint x,
			   gint y,
			   enum CongFontYPos y_pos)
{
	gint adjusted_y;
	gint text_height;
	
	g_return_if_fail(drawable);
	g_return_if_fail(font);
	g_return_if_fail(gc);
	g_return_if_fail(text);

#if SUPPORT_GDK_FONTS
	/* FIXME:  replace this with a Pango call */

	switch (y_pos) {
	default: g_assert_not_reached();
	
	case CONG_FONT_Y_POS_TOP:
		/* Render so that the Y coord is the ascent of the text: */
		adjusted_y = y + font->asc;
		break;

	case CONG_FONT_Y_POS_MIDDLE:
		/* Render so that the Y coord is the midpoint of the text (between the ascent and descent): */
		adjusted_y = y + (font->asc + font->desc)/2;
		break;

	case CONG_FONT_Y_POS_BASELINE:
		/* Render so that the Y coord is the baseline of the text: */
		adjusted_y = y;
		break;

	}

	gdk_draw_string(drawable, 
			font->gdk_font, 
			gc,
			x,
			adjusted_y, 
			text);
#else
	pango_layout_set_text(font->pango_layout,
			      text,
			      -1);

	pango_layout_get_pixel_size(font->pango_layout,
				    NULL,
				    &text_height);

	switch (y_pos) {
	default: g_assert_not_reached();
	
	case CONG_FONT_Y_POS_TOP:
		/* Render so that the Y coord is the ascent of the text: */
		adjusted_y = y;
		break;

	case CONG_FONT_Y_POS_MIDDLE:
		/* Render so that the Y coord is the midpoint of the text (between the ascent and descent): */
		adjusted_y = y + (text_height)/2;
		break;

	case CONG_FONT_Y_POS_BASELINE:
		/* Render so that the Y coord is the baseline of the text: */
		g_assert_not_reached();
		adjusted_y = y;
		break;

	}

	gdk_draw_layout(drawable, 
			gc,
			x, 
			adjusted_y,
			font->pango_layout);
#endif
}

gint
cong_font_string_width_slow(CongFont *font, 
			    const gchar *text)
{
	gint result;

	g_return_val_if_fail(font, 0);
	g_return_val_if_fail(text, 0);

#if SUPPORT_GDK_FONTS
	/* FIXME: replace with a Pango call */
	return gdk_string_width(font->gdk_font, 
				text);
#else
	pango_layout_set_text(font->pango_layout,
			      text,
			      -1);

	pango_layout_get_pixel_size(font->pango_layout,
				    &result,
				    NULL);
	
	return result;
#endif
}

