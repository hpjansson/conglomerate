/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-text.c
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

#include "global.h"
#include "cong-editor-area-text.h"
#include <libgnome/gnome-macros.h>
#include "cong-font.h"
#include "cong-ui-hooks.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaTextDetails
{
	gchar *text;

	GdkGC *gc;
	PangoLayout *pango_layout;
	CongFont *font;
};

/* Method implementation prototypes: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect);

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint);

static void
allocate_child_space (CongEditorArea *area);


/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaText, 
			cong_editor_area_text,
			CongEditorArea,
			CONG_EDITOR_AREA_TYPE );

static void
cong_editor_area_text_class_init (CongEditorAreaTextClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);

	area_klass->render_self = render_self;
	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
}

static void
cong_editor_area_text_instance_init (CongEditorAreaText *area_text)
{
	area_text->private = g_new0(CongEditorAreaTextDetails,1);
}

/* Exported function definitions: */
CongEditorArea*
cong_editor_area_text_construct (CongEditorAreaText *area_text,
				 CongEditorWidget3 *editor_widget,
				 CongFont *font,
				 const GdkColor *fg_col,
				 const gchar *text,
				 gboolean use_markup)
{
	g_return_val_if_fail (text, NULL);
/* 	g_return_val_if_fail (fg_col, NULL); */

	cong_editor_area_construct (CONG_EDITOR_AREA(area_text),
				    editor_widget);

	PRIVATE(area_text)->text = g_strdup(text);

	PRIVATE(area_text)->gc = gdk_gc_new(cong_gui_get_a_window()->window);

#if 0
	if (fg_col) {
		gdk_gc_set_foreground (PRIVATE(area_text)->gc, (GdkColor*)fg_col);
	}
#endif

	PRIVATE(area_text)->pango_layout = pango_layout_new(gtk_widget_get_pango_context (GTK_WIDGET(editor_widget)));

	if (use_markup) {
		pango_layout_set_markup (PRIVATE(area_text)->pango_layout,
					 text,
					 -1);
	} else {
		pango_layout_set_text (PRIVATE(area_text)->pango_layout,
				       text,
				       -1);
	}

	PRIVATE(area_text)->font = font;
	g_assert(PRIVATE(area_text)->font);

	pango_layout_set_font_description(PRIVATE(area_text)->pango_layout,
					  cong_font_get_pango_description(PRIVATE(area_text)->font));


	return CONG_EDITOR_AREA (area_text);
}

CongEditorArea*
cong_editor_area_text_new (CongEditorWidget3 *editor_widget,
			   CongFont *font,
			   const GdkColor *fg_col,
			   const gchar *text,
			   gboolean use_markup)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_text_new(\"%s\")", text);
#endif

	return cong_editor_area_text_construct
		(g_object_new (CONG_EDITOR_AREA_TEXT_TYPE, NULL),
		 editor_widget,
		 font,
		 fg_col,
		 text,
		 use_markup);
}

void
cong_editor_area_text_set_text (CongEditorAreaText *area_text,
				const gchar *text)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA_TEXT(area_text));
	g_return_if_fail (text);

	pango_layout_set_text (PRIVATE(area_text)->pango_layout,
			       text,
			       -1);

	cong_editor_area_queue_redraw (CONG_EDITOR_AREA(area_text));
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_text), GTK_ORIENTATION_HORIZONTAL);
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_text), GTK_ORIENTATION_VERTICAL);
}

void
cong_editor_area_text_set_markup (CongEditorAreaText *area_text,
				  const gchar *markup)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA_TEXT(area_text));
	g_return_if_fail (markup);

	pango_layout_set_markup (PRIVATE(area_text)->pango_layout,
				 markup,
				 -1);

	cong_editor_area_queue_redraw (CONG_EDITOR_AREA(area_text));
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_text), GTK_ORIENTATION_HORIZONTAL);
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_text), GTK_ORIENTATION_VERTICAL);
}

gint 
cong_editor_area_text_get_single_line_requisition (CongEditorAreaText *area_text,
						   GtkOrientation orientation)
{
	/* Bit hackish: use a REALLY BIG width to try to prevent wrapping: */
	return cong_editor_area_get_requisition (CONG_EDITOR_AREA(area_text),
						 orientation,
						 10000);
}

gboolean
cong_editor_area_text_xy_to_index (CongEditorAreaText *area_text,
				   int x,
				   int y,
				   int *index_,
				   int *trailing)
{
	const GdkRectangle *rect;

	g_return_val_if_fail (IS_CONG_EDITOR_AREA_TEXT(area_text), FALSE);
	g_return_val_if_fail (index_, FALSE);
	g_return_val_if_fail (trailing, FALSE);

	rect = cong_editor_area_get_window_coords (CONG_EDITOR_AREA(area_text));

	return pango_layout_xy_to_index (PRIVATE(area_text)->pango_layout,
					 (x - rect->x) * PANGO_SCALE,
					 (y - rect->y) * PANGO_SCALE,
					 index_,
					 trailing);
}

/* Method implementation definitions: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect)
{
	CongEditorAreaText *area_text = CONG_EDITOR_AREA_TEXT(area);
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);

	pango_layout_set_width (PRIVATE(area_text)->pango_layout,
				rect->width * PANGO_SCALE);

#if 0
	g_message("rendering text \"%s\" at (%i,%i,%i,%i)",
		  "fubar",
		  rect->x,
		  rect->y,
		  rect->width,
		  rect->height);
#endif

	gdk_draw_layout (window, 
			 PRIVATE(area_text)->gc,
			 rect->x, 
			 rect->y,
			 PRIVATE(area_text)->pango_layout);
}

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	gint width, height;
	CongEditorAreaText *area_text = CONG_EDITOR_AREA_TEXT(area);

	/* Try the suggested width; calculate how high that makes you want to be: */
	pango_layout_set_width (PRIVATE(area_text)->pango_layout,
				width_hint*PANGO_SCALE);

	pango_layout_get_pixel_size (PRIVATE(area_text)->pango_layout,
				     &width,
				     &height);	

	if (orientation == GTK_ORIENTATION_HORIZONTAL) {
		/* Shrink to size of width hint if above it: */
		if (width>width_hint) {
			return width_hint;
		} else {
			return width;
		}
	} else {
		return height;
	}
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaText *area_text = CONG_EDITOR_AREA_TEXT(area);	
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);

	/* and update our own space: */
	pango_layout_set_width (PRIVATE(area_text)->pango_layout,
				rect->width*PANGO_SCALE);
}
