/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-span-tag.c
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
#include "cong-editor-area-span-tag.h"
#include <libgnome/gnome-macros.h>

#include "cong-app.h"
#include "cong-editor-area-bin.h"
#include "cong-editor-area-text.h"
#include "cong-editor-area-composer.h"
#include "cong-editor-area-spacer.h"
#include "cong-editor-area-underline.h"
#include "cong-editor-area-pixbuf.h"
#include "cong-dispspec-element.h"

#define PRIVATE(x) ((x)->private)

#define V_SPACING (4)
#define H_SPACING (4)
#define H_INDENT (4)
#define FRAGMENT_WIDTH (45)

struct CongEditorAreaSpanTagDetails
{
	CongDispspecElement *ds_element;

	CongEditorArea *inner_bin;
	CongEditorArea *title_pixbuf;
	CongEditorArea *title_text;

	gboolean is_at_start;
	gboolean is_at_end;
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

static CongEditorArea*
for_all (CongEditorArea *editor_area, 
	 CongEditorAreaCallbackFunc func, 
	 gpointer user_data);

static void
add_child (CongEditorAreaContainer *area_container,
	   CongEditorArea *child);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaSpanTag, 
			cong_editor_area_span_tag,
			CongEditorAreaBin,
			CONG_EDITOR_AREA_BIN_TYPE );

static void
cong_editor_area_span_tag_class_init (CongEditorAreaSpanTagClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);
	CongEditorAreaContainerClass *container_klass = CONG_EDITOR_AREA_CONTAINER_CLASS(klass);

	area_klass->render_self = render_self;
	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
	area_klass->for_all = for_all;

	container_klass->add_child = add_child;

}

static void
cong_editor_area_span_tag_instance_init (CongEditorAreaSpanTag *area_span_tag)
{
	area_span_tag->private = g_new0(CongEditorAreaSpanTagDetails,1);
}


/* Exported function definitions: */
CongEditorArea*
cong_editor_area_span_tag_construct (CongEditorAreaSpanTag *area_span_tag,
				     CongEditorWidget3 *editor_widget,
				     CongDispspecElement *ds_element,
				     GdkPixbuf *pixbuf,
				     const gchar *text,
				     gboolean is_at_start,
				     gboolean is_at_end)
{
	g_return_val_if_fail (text, NULL);

	cong_editor_area_bin_construct (CONG_EDITOR_AREA_BIN(area_span_tag),
					editor_widget);

	PRIVATE(area_span_tag)->ds_element = ds_element;
	PRIVATE(area_span_tag)->is_at_start = is_at_start;
	PRIVATE(area_span_tag)->is_at_end = is_at_end;

	PRIVATE(area_span_tag)->inner_bin = cong_editor_area_bin_new (editor_widget);

	if (pixbuf) {
		PRIVATE(area_span_tag)->title_pixbuf = cong_editor_area_pixbuf_new (editor_widget,
										    pixbuf);

		cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (area_span_tag),
									   PRIVATE(area_span_tag)->title_pixbuf);
		
		
		cong_editor_area_protected_set_parent (PRIVATE(area_span_tag)->title_pixbuf,
						       CONG_EDITOR_AREA (area_span_tag));
	}

	PRIVATE(area_span_tag)->title_text = cong_editor_area_text_new (editor_widget,
									cong_app_get_font (cong_app_singleton(),
											   CONG_FONT_ROLE_SPAN_TAG),
									cong_dispspec_element_col (ds_element, 
												   CONG_DISPSPEC_GC_USAGE_BOLD_LINE),
									text,
									FALSE);

	cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (area_span_tag),
								   PRIVATE(area_span_tag)->title_text);

	cong_editor_area_protected_set_parent (PRIVATE(area_span_tag)->title_text,
					       CONG_EDITOR_AREA (area_span_tag));


	return CONG_EDITOR_AREA (area_span_tag);
}

CongEditorArea*
cong_editor_area_span_tag_new (CongEditorWidget3 *editor_widget,
			       CongDispspecElement *ds_element,
			       GdkPixbuf *pixbuf,
			       const gchar *text,
			       gboolean is_at_start,
			       gboolean is_at_end)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_span_tag_new(%s)", text);
#endif

	g_return_val_if_fail (editor_widget, NULL);
	g_return_val_if_fail (text, NULL);

	return cong_editor_area_span_tag_construct
		(g_object_new (CONG_EDITOR_AREA_SPAN_TAG_TYPE, NULL),
		 editor_widget,
		 ds_element,
		 pixbuf,
		 text,
		 is_at_start,
		 is_at_end);
}

/* Method implementation definitions: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect)
{
	CongEditorAreaSpanTag *area_span_tag = CONG_EDITOR_AREA_SPAN_TAG(area);
	CongDispspecElement *ds_element = PRIVATE(area_span_tag)->ds_element;
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);

	GdkGC *gc = cong_dispspec_element_gc(ds_element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE);

#if 0
	gint title_text_width_req = cong_editor_area_text_get_single_line_requisition (CONG_EDITOR_AREA_TEXT(PRIVATE(area_span_tag)->title_text),
										       GTK_ORIENTATION_HORIZONTAL);
#endif
	gint title_text_height_req = cong_editor_area_text_get_single_line_requisition (CONG_EDITOR_AREA_TEXT(PRIVATE(area_span_tag)->title_text),
											GTK_ORIENTATION_VERTICAL);
	/* Calculate start/end points on this this line: */
	gint end_x = rect->x + rect->width;
	gint line_y = rect->y + rect->height - (title_text_height_req/2);


#define UNDERLINE_ASCENT (5)
	/* was 2 */

	if (PRIVATE(area_span_tag)->is_at_start) {
		gdk_draw_line (window, 
			       gc, 
			       rect->x, line_y - UNDERLINE_ASCENT, 
			       rect->x, line_y);
	}

	if (PRIVATE(area_span_tag)->is_at_end) {
		gdk_draw_line (window, 
			       gc, 
			       end_x, line_y - UNDERLINE_ASCENT, 
			       end_x, line_y);
	}

	/* Draw the main underline: */
	if (cong_editor_area_is_hidden (PRIVATE(area_span_tag)->title_text)) {
		/* The text is hidden; draw the entire line in one go: */
		gdk_draw_line (window, 
			       gc, 
			       rect->x, line_y, 
			       end_x, line_y);
	} else {
		const GdkRectangle* text_rect = cong_editor_area_get_window_coords (PRIVATE(area_span_tag)->title_text);

		/* Break line into two parts: */
		gdk_draw_line (window, 
			       gc, 
			       rect->x, line_y, 
			       text_rect->x, line_y);

		gdk_draw_line (window, 
			       gc, 
			       text_rect->x + text_rect->width, line_y, 
			       end_x, line_y);
	}

	cong_editor_area_debug_render_state (area);
}

/**
 * calc_requisition
 * @area: the CongEditorArea being queried.
 * @orientation: which dimension?  GTK_ORIENTATION_HORIZONTAL or GTK_ORIENTATION_VERTICAL.
 * @width_hint:
 *
 * Calculate the height or width required by the span tag given by the area argument.
 *
 * Returns: the required dimension, as a gint.
 */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	CongEditorAreaSpanTag *span_tag = CONG_EDITOR_AREA_SPAN_TAG(area);

	gint inner_req = cong_editor_area_get_requisition (PRIVATE(span_tag)->inner_bin,
							   orientation,
							   width_hint);
#if 0
	gint title_pixbuf_req = cong_editor_area_get_requisition (PRIVATE(span_tag)->title_pixbuf,
								  orientation,
								  width_hint);
#endif

	if (orientation == GTK_ORIENTATION_HORIZONTAL) {
		/* Make sure there is enough space for title and for text */
		gint title_text_width_req = cong_editor_area_text_get_single_line_requisition (CONG_EDITOR_AREA_TEXT(PRIVATE(span_tag)->title_text),
											       orientation);
		return (MAX(inner_req, title_text_width_req));
	} else {		
		gint title_text_height_req = cong_editor_area_text_get_single_line_requisition (CONG_EDITOR_AREA_TEXT(PRIVATE(span_tag)->title_text),
												orientation);
	
		return inner_req + title_text_height_req;
	}
}

/**
 * allocate_child_space
 * @area CongEditorArea for parent span element
 *
 * Given area allocated for span element, allocate two subareas (its inner_bin and its title).
 * 
 * Returns: void
 */
static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaSpanTag *span_tag = CONG_EDITOR_AREA_SPAN_TAG(area);
	
	const GdkRectangle *rect = cong_editor_area_get_window_coords(area);
	
	gint inner_req_width =  cong_editor_area_get_requisition (PRIVATE(span_tag)->inner_bin,
								  GTK_ORIENTATION_HORIZONTAL,
								  rect->width);

	gint inner_req_height = cong_editor_area_get_requisition (PRIVATE(span_tag)->inner_bin,
								  GTK_ORIENTATION_VERTICAL,
								  rect->width);

	gint title_text_width_req = cong_editor_area_text_get_single_line_requisition (CONG_EDITOR_AREA_TEXT(PRIVATE(span_tag)->title_text),
										       GTK_ORIENTATION_HORIZONTAL);
	gint title_text_height_req = cong_editor_area_text_get_single_line_requisition (CONG_EDITOR_AREA_TEXT(PRIVATE(span_tag)->title_text),
											GTK_ORIENTATION_VERTICAL);

#if 0
	g_message ("single_line_req = (%i,%i)", 
		   title_text_width_req,
		   title_text_height_req);
#endif
	
	/* Set inner bin to have all the width, plus all the height it wants:*/
	cong_editor_area_set_allocation (PRIVATE(span_tag)->inner_bin,
					 rect->x + (rect->width - inner_req_width)/2,
					 rect->y,
					 inner_req_width,
					 inner_req_height);

	cong_editor_area_show (PRIVATE(span_tag)->title_text);
	cong_editor_area_set_allocation (PRIVATE(span_tag)->title_text,
					 rect->x + (rect->width - title_text_width_req)/2,
					 rect->y + rect->height - title_text_height_req,
					 title_text_width_req,
					 title_text_height_req);
}

static CongEditorArea*
for_all (CongEditorArea *editor_area, 
	 CongEditorAreaCallbackFunc func, 
	 gpointer user_data)
{
	CongEditorAreaSpanTag *span_tag = CONG_EDITOR_AREA_SPAN_TAG(editor_area);

	if ((*func)(PRIVATE(span_tag)->inner_bin, user_data)) {
		return PRIVATE(span_tag)->inner_bin;
	}
	if (PRIVATE(span_tag)->title_pixbuf) {
		if ((*func)(PRIVATE(span_tag)->title_pixbuf, user_data)) {
			return PRIVATE(span_tag)->title_pixbuf;
		}
	}
	if ((*func)(PRIVATE(span_tag)->title_text, user_data)) {
		return PRIVATE(span_tag)->title_text;
	}

	return NULL;
}

static void
add_child (CongEditorAreaContainer *area_container,
	   CongEditorArea *child)
{
	CongEditorAreaSpanTag *span_tag = CONG_EDITOR_AREA_SPAN_TAG(area_container);

	g_assert(PRIVATE(span_tag)->inner_bin);

	cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER( PRIVATE(span_tag)->inner_bin),
					       child);
}
