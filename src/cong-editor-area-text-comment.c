/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-text-comment.c
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
#include "cong-editor-area-text-comment.h"
#include <libgnome/gnome-macros.h>
#include "cong-font.h"
#include "cong-eel.h"
#include "cong-ui-hooks.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaTextCommentDetails
{
#if 1
	GdkGC *gc;
#else
	gchar *text_comment;

	PangoLayout *pango_layout;
	CongFont *font;
#endif
};

/* Method implementation prototypes: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect);

#if 0
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint);
#endif

#if 0
static void
allocate_child_space (CongEditorArea *area);
#endif


/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaTextComment, 
			cong_editor_area_text_comment,
			CongEditorAreaText,
			CONG_EDITOR_AREA_TEXT_TYPE );

static void
cong_editor_area_text_comment_class_init (CongEditorAreaTextCommentClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);

	area_klass->render_self = render_self;
#if 0
	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
#endif
}

static void
cong_editor_area_text_comment_instance_init (CongEditorAreaTextComment *area_text_comment)
{
	area_text_comment->private = g_new0(CongEditorAreaTextCommentDetails,1);
}

/* Exported function definitions: */
/**
 * cong_editor_area_text_comment_construct:
 * @area_text_comment:
 * @editor_widget:
 * @font:
 * @fg_col:
 * @text_comment:
 * @use_markup:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_text_comment_construct (CongEditorAreaTextComment *area_text_comment,
					 CongEditorWidget3 *editor_widget,
					 CongFont *font,
					 const GdkColor *fg_col,
					 const gchar *text_comment,
					 gboolean use_markup)
{
	GdkColor color;

	g_return_val_if_fail (text_comment, NULL);
/* 	g_return_val_if_fail (fg_col, NULL); */

	cong_editor_area_text_construct (CONG_EDITOR_AREA_TEXT(area_text_comment),
					 editor_widget,
					 font,
					 fg_col,
					 text_comment,
					 use_markup);

	PRIVATE(area_text_comment)->gc = gdk_gc_new(cong_gui_get_a_window()->window);

	cong_eel_rgb_to_gdk_color (&color,
				   0xE0,
				   0xE0,
				   0x00);

	/* FIXME: this ought to respect the theme somehow */
	gdk_gc_set_foreground (PRIVATE(area_text_comment)->gc,
			       &color);

	return CONG_EDITOR_AREA (area_text_comment);
}

/**
 * cong_editor_area_text_comment_new:
 * @editor_widget:
 * @font:
 * @fg_col:
 * @text_comment:
 * @use_markup:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_text_comment_new (CongEditorWidget3 *editor_widget,
			   CongFont *font,
			   const GdkColor *fg_col,
			   const gchar *text_comment,
			   gboolean use_markup)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_text_comment_new(\"%s\")", text_comment);
#endif

	return cong_editor_area_text_comment_construct
		(g_object_new (CONG_EDITOR_AREA_TEXT_COMMENT_TYPE, NULL),
		 editor_widget,
		 font,
		 fg_col,
		 text_comment,
		 use_markup);
}

#if 0
void
cong_editor_area_text_comment_set_text_comment (CongEditorAreaTextComment *area_text_comment,
				const gchar *text_comment)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA_TEXT_COMMENT(area_text_comment));
	g_return_if_fail (text_comment);

	pango_layout_set_text_comment (PRIVATE(area_text_comment)->pango_layout,
			       text_comment,
			       -1);

	cong_editor_area_queue_redraw (CONG_EDITOR_AREA(area_text_comment));
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_text_comment), GTK_ORIENTATION_HORIZONTAL);
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_text_comment), GTK_ORIENTATION_VERTICAL);
}

void
cong_editor_area_text_comment_set_markup (CongEditorAreaTextComment *area_text_comment,
				  const gchar *markup)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA_TEXT_COMMENT(area_text_comment));
	g_return_if_fail (markup);

	pango_layout_set_markup (PRIVATE(area_text_comment)->pango_layout,
				 markup,
				 -1);

	cong_editor_area_queue_redraw (CONG_EDITOR_AREA(area_text_comment));
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_text_comment), GTK_ORIENTATION_HORIZONTAL);
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_text_comment), GTK_ORIENTATION_VERTICAL);
}

gint 
cong_editor_area_text_comment_get_single_line_requisition (CongEditorAreaTextComment *area_text_comment,
						   GtkOrientation orientation)
{
	/* Bit hackish: use a REALLY BIG width to try to prevent wrapping: */
	return cong_editor_area_get_requisition (CONG_EDITOR_AREA(area_text_comment),
						 orientation,
						 10000);
}

gboolean
cong_editor_area_text_comment_xy_to_index (CongEditorAreaTextComment *area_text_comment,
				   int x,
				   int y,
				   int *index_,
				   int *trailing)
{
	const GdkRectangle *rect;

	g_return_val_if_fail (IS_CONG_EDITOR_AREA_TEXT_COMMENT(area_text_comment), FALSE);
	g_return_val_if_fail (index_, FALSE);
	g_return_val_if_fail (trailing, FALSE);

	rect = cong_editor_area_get_window_coords (CONG_EDITOR_AREA(area_text_comment));

	return pango_layout_xy_to_index (PRIVATE(area_text_comment)->pango_layout,
					 (x - rect->x) * PANGO_SCALE,
					 (y - rect->y) * PANGO_SCALE,
					 index_,
					 trailing);
}
#endif

/* Method implementation definitions: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect)
{
	CongEditorAreaTextComment *area_text_comment = CONG_EDITOR_AREA_TEXT_COMMENT(area);
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);

	/* Fill the area in yellow: */
	gdk_draw_rectangle (window, 
			    PRIVATE(area_text_comment)->gc,
			    TRUE,
			    rect->x,
			    rect->y,
			    rect->width,
			    rect->height);

	/* Call base class: */
	CONG_EEL_CALL_PARENT(CONG_EDITOR_AREA_CLASS, 
			     render_self, 
			     (area, widget_rect));
}

#if 0
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	gint width, height;
	CongEditorAreaTextComment *area_text_comment = CONG_EDITOR_AREA_TEXT_COMMENT(area);

	/* Try the suggested width; calculate how high that makes you want to be: */
	pango_layout_set_width (PRIVATE(area_text_comment)->pango_layout,
				width_hint*PANGO_SCALE);

	pango_layout_get_pixel_size (PRIVATE(area_text_comment)->pango_layout,
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
	CongEditorAreaTextComment *area_text_comment = CONG_EDITOR_AREA_TEXT_COMMENT(area);	
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);

	/* and update our own space: */
	pango_layout_set_width (PRIVATE(area_text_comment)->pango_layout,
				rect->width*PANGO_SCALE);
}
#endif
