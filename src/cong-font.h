/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-font.h
 *
 * Copyright (C) 2003 David Malcolm
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
 */

#ifndef __CONG_FONT_H__
#define __CONG_FONT_H__

/**
   An enum for describing the meaning of a y coordinate when rendering text.
 */
typedef enum {
	
	/* Render so that the Y coord is the ascent of the text: */
	CONG_FONT_Y_POS_TOP,

	/* Render so that the Y coord is the midpoint of the text (between the ascent and descent): */
	CONG_FONT_Y_POS_MIDDLE,

	/* Render so that the Y coord is the baseline of the text: */
	CONG_FONT_Y_POS_BASELINE,
} CongFontYPos;

CongFont*
cong_font_load(const gchar *font_name);

void
cong_font_delete(CongFont *font);

gint
cong_font_get_height(CongFont *font, const gchar *text);

PangoFontDescription*
cong_font_get_pango_description(CongFont *font);

/**
   Draw a string to the drawable using the font.

   It's slow as it creates and destroys a PangoLayout internally,
   but at least it's an easy API; handy for debugging etc.
 */
void
cong_font_draw_string_slow(GdkDrawable *drawable,
			   CongFont *font,
			   GdkGC *gc,
			   const gchar *text,
			   gint x,
			   gint y,
			   CongFontYPos y_pos);

/**
   Get the width of a string rendered in the font in pixels

   It's slow as it creates and destroys a PangoLayout internally,
   but at least it's an easy API; handy for debugging etc.
 */
gint
cong_font_string_width_slow(CongFont *font, 
			    const gchar *text);



#endif 
