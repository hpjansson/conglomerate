/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-underline.c
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
#include "cong-editor-area-underline.h"
#include <libgnome/gnome-macros.h>
#include "cong-ui-hooks.h"


#define PRIVATE(x) ((x)->private)

struct CongEditorAreaUnderlineDetails
{
	CongEditorAreaUnderlineStyle style;
	
	GdkGC *gc;
};

/* Method implementation prototypes: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect);

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaUnderline, 
			cong_editor_area_underline,
			CongEditorArea,
			CONG_EDITOR_AREA_TYPE );

static void
cong_editor_area_underline_class_init (CongEditorAreaUnderlineClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);

	area_klass->render_self = render_self;
	area_klass->calc_requisition = calc_requisition;
}

static void
cong_editor_area_underline_instance_init (CongEditorAreaUnderline *area_underline)
{
	area_underline->private = g_new0(CongEditorAreaUnderlineDetails,1);
}

/* Exported function definitions: */
/**
 * cong_editor_area_underline_construct:
 * @area_underline:
 * @editor_widget:
 * @style:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_underline_construct (CongEditorAreaUnderline *area_underline,
				      CongEditorWidget3 *editor_widget,
				      CongEditorAreaUnderlineStyle style)
{
	g_return_val_if_fail (area_underline, NULL);

	cong_editor_area_construct (CONG_EDITOR_AREA(area_underline),
				    editor_widget);

	PRIVATE(area_underline)->style = style;
	PRIVATE(area_underline)->gc = gdk_gc_new(cong_gui_get_a_window()->window);;

	return CONG_EDITOR_AREA (area_underline);
}

/**
 * cong_editor_area_underline_new:
 * @editor_widget:
 * @style:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_underline_new (CongEditorWidget3 *editor_widget,
				CongEditorAreaUnderlineStyle style)
{
	return cong_editor_area_underline_construct
		(g_object_new (CONG_EDITOR_AREA_UNDERLINE_TYPE, NULL),
		 editor_widget,
		 style);
}

/* Method implementation definitions: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect)
{
	CongEditorAreaUnderline *area_underline = CONG_EDITOR_AREA_UNDERLINE(area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);
	GdkGC *gc = PRIVATE(area_underline)->gc;
	gint mid_y = rect->y + (rect->height/2);

	switch ( PRIVATE(area_underline)->style) {
	default: g_assert_not_reached();
	case CONG_EDITOR_AREA_UNDERLINE_STYLE_SPAN_TAG_START:
		/* Vertical line: */
		gdk_draw_line (window,
			       gc,
			       rect->x,
			       rect->y,
			       rect->x,
			       mid_y);

		/* Horizontal line: */
		gdk_draw_line (window,
			       gc,
			       rect->x,
			       mid_y,
			       rect->x + rect->width - 1,
			       mid_y);
		break;

	case CONG_EDITOR_AREA_UNDERLINE_STYLE_SPAN_TAG_END:
		/* Horizontal line: */
		gdk_draw_line (window,
			       gc,
			       rect->x,
			       mid_y,
			       rect->x + rect->width - 1,
			       mid_y);
		/* Vertical line: */
		gdk_draw_line (window,
			       gc,
			       rect->x + rect->width - 1,
			       rect->y,
			       rect->x + rect->width - 1,
			       mid_y);

		break;

	case CONG_EDITOR_AREA_UNDERLINE_STYLE_SPELLING_ERROR:
		/* unimplemented */
		break;

	case CONG_EDITOR_AREA_UNDERLINE_STYLE_GRAMMAR_ERROR:
		/* unimplemented */
		break;
	}
}

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
#if 0
	CongEditorAreaUnderline *area_underline = CONG_EDITOR_AREA_UNDERLINE(area);
#endif

	if (orientation==GTK_ORIENTATION_HORIZONTAL) {
		return 0;
	} else {
		return 5;
	}
}
