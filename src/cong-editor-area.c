/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area.c
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
#include "cong-editor-area.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaDetails
{
	CongEditorWidget3 *editor_widget;
	gboolean is_hidden;
	GdkRectangle window_area; /* allocated area in window space */
	GtkRequisition requisition;
};

CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_area, update_requisition);
CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_area, allocate_child_space);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorArea, 
			cong_editor_area,
			GObject,
			G_TYPE_OBJECT );

static void
cong_editor_area_class_init (CongEditorAreaClass *klass)
{
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_area,
					      update_requisition);

	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_area,
					      allocate_child_space);
}

static void
cong_editor_area_instance_init (CongEditorArea *area)
{
	area->private = g_new0(CongEditorAreaDetails,1);
}

CongEditorArea*
cong_editor_area_construct (CongEditorArea *area,
			    CongEditorWidget3* editor_widget)
{
	PRIVATE(area)->editor_widget = editor_widget;

	PRIVATE(area)->is_hidden = FALSE;

	/* FIXME: we forcibly set up the allocation for now: */
	cong_eel_rectangle_construct( &PRIVATE(area)->window_area,
				      0,0,
				      200,250);

#if 0
	PRIVATE(area)->requisition;
#endif
}


CongEditorWidget3*
cong_editor_area_get_widget (CongEditorArea *area)
{
	g_return_val_if_fail (area, NULL);

	return PRIVATE(area)->editor_widget;
}

#if 0

gboolean 
cong_editor_area_is_hidden (CongEditorArea *area);
#endif

const GdkRectangle*
cong_editor_area_get_window_coords (CongEditorArea *area)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA(area));

	return &PRIVATE(area)->window_area;
}

const GtkRequisition*
cong_editor_area_get_requisition (CongEditorArea *area)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA(area));

	/* FIXME: if not uptodate, call fn? */

	return &PRIVATE(area)->requisition;
}

void 
cong_editor_area_set_requisition (CongEditorArea *area,
				  gint width,
				  gint height)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA(area));

	PRIVATE(area)->requisition.width = width;
	PRIVATE(area)->requisition.height = height;
}

void 
cong_editor_area_debug_render_area (CongEditorArea *area,
				    GdkGC *gc)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA(area));
	g_return_if_fail (gc);

	gdk_draw_rectangle (GDK_DRAWABLE(cong_editor_area_get_gdk_window(area)),
			    gc,
			    FALSE,
			    PRIVATE(area)->window_area.x,
			    PRIVATE(area)->window_area.y,
			    PRIVATE(area)->window_area.width-1,
			    PRIVATE(area)->window_area.height-1);
}

/* CongEditorArea methods: */
static void
do_recursive_render (CongEditorArea *area,
		     gpointer user_data)
{
	cong_editor_area_recursive_render (area,
					   (const GdkRectangle*)user_data);
}

void
cong_editor_area_recursive_render (CongEditorArea *area,
				   const GdkRectangle *widget_rect)
{
	GdkRectangle intersected_area;

	g_return_if_fail (IS_CONG_EDITOR_AREA(area));

	/* Accept/reject tests: */
	if (PRIVATE(area)->is_hidden) {
		return;
	}

	/* Early accept/reject against the areas: */
	if (gdk_rectangle_intersect((GdkRectangle*)widget_rect,
				    (GdkRectangle*)cong_editor_area_get_window_coords(area),
				    &intersected_area)) {

		/* Render self: */
		CONG_EEL_CALL_METHOD (CONG_EDITOR_AREA_CLASS,
				      area,
				      render_self, 
				      (area, &intersected_area));
		
		/* Recurse over all children (internal and non-internal): */
		cong_editor_area_for_all (area, do_recursive_render, &intersected_area);
	}
}

#if 0
void
cong_editor_area_on_button_press (CongEditorArea *editor_area, 
				  GdkEventButton *event);

void
cong_editor_area_on_motion_notify (CongEditorArea *editor_area, 
				   GdkEventMotion *event);

void
cong_editor_area_on_key_press (CongEditorArea *editor_area, 
			       GdkEventKey *event);

#endif

void 
cong_editor_area_update_requisition (CongEditorArea *editor_area, 
				     int width_hint)
{
	g_return_if_fail (editor_area);

	CONG_EEL_CALL_METHOD (CONG_EDITOR_AREA_CLASS,
			      editor_area,
			      update_requisition, 
			      (editor_area, width_hint));
}

void 
cong_editor_area_set_allocation (CongEditorArea *editor_area,
				 gint x,
				 gint y,
				 gint width,
				 gint height)
{
	g_return_if_fail (editor_area);

	g_message("cong_editor_area_set_allocation(%i,%i,%i,%i)", x, y, width, height);

	PRIVATE(editor_area)->window_area.x = x;
	PRIVATE(editor_area)->window_area.y = y;
	PRIVATE(editor_area)->window_area.width = width;
	PRIVATE(editor_area)->window_area.height = height;

	/* Call hook to recursively allocate space to children: */
	CONG_EEL_CALL_METHOD (CONG_EDITOR_AREA_CLASS,
			      editor_area,
			      allocate_child_space, 
			      (editor_area));
}

void
cong_editor_area_for_all (CongEditorArea *editor_area, 
			  CongEditorAreaCallbackFunc func, 
			  gpointer user_data)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA(editor_area));
	g_return_if_fail (func);

	CONG_EEL_CALL_METHOD (CONG_EDITOR_AREA_CLASS,
			      editor_area,
			      for_all, 
			      (editor_area, func, user_data));
	
}


GdkWindow*
cong_editor_area_get_gdk_window(CongEditorArea *editor_area)
{
	g_return_val_if_fail (editor_area, NULL);

	return GTK_WIDGET(cong_editor_area_get_widget (editor_area))->window;
}


