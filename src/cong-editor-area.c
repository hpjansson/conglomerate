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
#include "cong-marshal.h"
#include "cong-editor-node.h"
#include "cong-document.h"
#include "cong-selection.h"
#include "cong-ui-hooks.h"

#define PRIVATE(x) ((x)->private)

#define DEBUG_REQUISITIONS 0
#define DEBUG_ALLOCATIONS 0
#define DEBUG_RENDER_ALLOCATIONS 0
#define DEBUG_RENDERING 0

enum {
	BUTTON_PRESS_EVENT,
#if 0
	ENTER_NOTIFY_EVENT,
	LEAVE_NOTIFY_EVENT,
#endif
	MOTION_NOTIFY_EVENT,
	KEY_PRESS_EVENT,

	FLUSH_REQUISITION_CACHE,

	STATE_CHANGED,

	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

typedef struct RequisitionCache
{
	gboolean requisition_cache_valid;
	guint last_calculated_requisition;
	int cached_width_hint;
} RequisitionCache;

struct CongEditorAreaDetails
{
	CongEditorWidget3 *editor_widget;

	CongEditorArea *parent_area;

	gboolean is_hidden;
	GtkStateType state;
	GdkRectangle window_area; /* allocated area in window space */

	RequisitionCache requisition_cache[2];
	gboolean needs_recursive_allocation;

	/* If this area is directly associated with an editor_node, this is it: */
	CongEditorNode *editor_node;
	guint selection_change_handler_id;

	GdkCursor *cursor;
};
/* Declarations of the GObject handlers: */
static void
finalize (GObject *object);

static void
dispose (GObject *object);

/* Signal handler declarations: */
static void
on_child_flush_requisition_cache (CongEditorArea *child_area,
				  GtkOrientation orientation,
				  gpointer user_data);

static gboolean
on_signal_button_press_for_area_with_node (CongEditorArea *editor_area, 
					   GdkEventButton *event,
					   gpointer user_data);

static gboolean
on_signal_motion_notify_for_area_with_node (CongEditorArea *editor_area, 
					    GdkEventButton *event,
					    gpointer user_data);

static void
on_signal_is_selected_changed_for_area_with_node (CongEditorNode *editor_node,
						 gpointer user_data);

CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_area, calc_requisition);
#if 0
CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_area, allocate_child_space);
#endif

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorArea, 
			cong_editor_area,
			GObject,
			G_TYPE_OBJECT );

static void
cong_editor_area_class_init (CongEditorAreaClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_area,
					      calc_requisition);

#if 0
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_area,
					      allocate_child_space);
#endif

	/* Set up the various signals: */
	signals[BUTTON_PRESS_EVENT] = g_signal_new ("button_press_event",
						    CONG_EDITOR_AREA_TYPE,
						    G_SIGNAL_RUN_LAST,
						    G_STRUCT_OFFSET(CongEditorAreaClass, on_button_press),
						    NULL, NULL,
						    cong_cclosure_marshal_BOOLEAN__POINTER,
						    G_TYPE_BOOLEAN, 
						    1, G_TYPE_POINTER);

#if 0
	signals[ENTER_NOTIFY_EVENT] = g_signal_new ("enter_notify_event",
						     CONG_EDITOR_AREA_TYPE,
						     G_SIGNAL_RUN_FIRST,
						     G_STRUCT_OFFSET(CongEditorAreaClass, enter_notify_event),
						     NULL, NULL,
						     g_cclosure_marshal_VOID__VOID,
						     G_TYPE_NONE, 
						     0);
	signals[LEAVE_NOTIFY_EVENT] = g_signal_new ("leave_notify_event",
						     CONG_EDITOR_AREA_TYPE,
						     G_SIGNAL_RUN_FIRST,
						     G_STRUCT_OFFSET(CongEditorAreaClass, leave_notify_event),
						     NULL, NULL,
						     g_cclosure_marshal_VOID__VOID,
						     G_TYPE_NONE, 
						     0);
#endif
	signals[MOTION_NOTIFY_EVENT] = g_signal_new ("motion_notify_event",
						     CONG_EDITOR_AREA_TYPE,
						     G_SIGNAL_RUN_LAST,
						     G_STRUCT_OFFSET(CongEditorAreaClass, on_motion_notify),
						     NULL, NULL,
						     cong_cclosure_marshal_BOOLEAN__POINTER,
						     G_TYPE_BOOLEAN, 
						     1, G_TYPE_POINTER);

	signals[KEY_PRESS_EVENT] = g_signal_new ("key_press_event",
						 CONG_EDITOR_AREA_TYPE,
						 G_SIGNAL_RUN_LAST,
						 G_STRUCT_OFFSET(CongEditorAreaClass, on_key_press),
						 NULL, NULL,
						 cong_cclosure_marshal_BOOLEAN__POINTER,
						 G_TYPE_BOOLEAN, 
						 1, G_TYPE_POINTER);

	signals[FLUSH_REQUISITION_CACHE] = g_signal_new ("flush_requisition_cache",
							 CONG_EDITOR_AREA_TYPE,
							 G_SIGNAL_RUN_FIRST,
							 0, /* G_STRUCT_OFFSET(CongEditorAreaClass, flush_requisition_cache), */
							 NULL, NULL,
							 g_cclosure_marshal_VOID__ENUM,
							 G_TYPE_NONE, 
							 1, GTK_TYPE_ORIENTATION);
	signals[STATE_CHANGED] = g_signal_new ("state_changed",
					       CONG_EDITOR_AREA_TYPE,
					       G_SIGNAL_RUN_LAST,
					       G_STRUCT_OFFSET(CongEditorAreaClass, state_changed),
					       NULL, NULL,
					       g_cclosure_marshal_VOID__VOID,
					       G_TYPE_NONE,
					       0);
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
	PRIVATE(area)->state = GTK_STATE_NORMAL;

	/* FIXME: we forcibly set up the allocation for now: */
	cong_eel_rectangle_construct( &PRIVATE(area)->window_area,
				      0,0,
				      200,250);

#if 0
	PRIVATE(area)->requisition;
#endif

	PRIVATE(area)->needs_recursive_allocation = TRUE;

	return area;
}


CongEditorWidget3*
cong_editor_area_get_widget (CongEditorArea *area)
{
	g_return_val_if_fail (area, NULL);

	return PRIVATE(area)->editor_widget;
}

CongDocument*
cong_editor_area_get_document (CongEditorArea *area)
{
	g_return_val_if_fail (area, NULL);

	return cong_editor_widget3_get_document (cong_editor_area_get_widget (area));
}


gboolean 
cong_editor_area_is_hidden (CongEditorArea *area)
{
	return PRIVATE(area)->is_hidden;
}


void
cong_editor_area_show (CongEditorArea *area)
{
	PRIVATE(area)->is_hidden = FALSE;
	/* FIXME: do we need to emit any events? */
}

void
cong_editor_area_hide (CongEditorArea *area)
{
	PRIVATE(area)->is_hidden = TRUE;
	/* FIXME: do we need to emit any events? */
}

GtkStateType
cong_editor_area_get_state (CongEditorArea *area)
{
	g_return_val_if_fail (IS_CONG_EDITOR_AREA(area), GTK_STATE_NORMAL);

	return PRIVATE(area)->state;
}

void
cong_editor_area_set_state (CongEditorArea *area,
			    GtkStateType state)
{
	if (PRIVATE(area)->state != state) {
		PRIVATE(area)->state = state;

		g_signal_emit (G_OBJECT(area),
			       signals[STATE_CHANGED], 0);

		cong_editor_area_queue_redraw (area);
	}
}

GdkCursor*
cong_editor_area_get_cursor (CongEditorArea *area)
{
	if ( PRIVATE (area)->cursor) {
		gdk_cursor_ref (PRIVATE (area)->cursor);
		return PRIVATE (area)->cursor;
	} else {
		return NULL;
	}
}

void
cong_editor_area_set_cursor (CongEditorArea *area,
			     GdkCursor *cursor)
{
	if (PRIVATE (area)->cursor != cursor) {
		if (cursor) {
			gdk_cursor_ref (cursor);
		}

		if (PRIVATE (area)->cursor) {
			gdk_cursor_unref (PRIVATE (area)->cursor);
		}

		PRIVATE (area)->cursor = cursor;
	}
}


#if 1
const GdkRectangle*
cong_editor_area_get_window_coords (CongEditorArea *area)
{
	g_return_val_if_fail (IS_CONG_EDITOR_AREA(area), NULL);

	return &PRIVATE(area)->window_area;
}
#endif


guint
cong_editor_area_get_requisition (CongEditorArea *area,
				  GtkOrientation orientation,
				  int width_hint)
{
	RequisitionCache *cache;

	g_return_val_if_fail (IS_CONG_EDITOR_AREA(area), 0);

	cache = &PRIVATE(area)->requisition_cache[orientation];

	/* If not up-to-date, call fn to regenerate cache: */
	if ( (width_hint!=cache->cached_width_hint) 
	     || (!cache->requisition_cache_valid) ) {
		guint calculated_requisition;
		
#if DEBUG_REQUISITIONS
		g_message("cong_editor_area_get_requisition:  recalcing %s cache on %s with width_hint %i", 
			  ((orientation == GTK_ORIENTATION_HORIZONTAL)?"h":"v"),
			  G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(area)), 
			  width_hint);
#endif

		calculated_requisition = cong_editor_area_calc_requisition (area, 
									    orientation,
									    width_hint);

		cache->last_calculated_requisition = calculated_requisition;

		cache->cached_width_hint = width_hint;

		cache->requisition_cache_valid = TRUE;

#if DEBUG_REQUISITIONS
		g_message("cong_editor_area_get_requisition:  recalced  %s cache on %s with width_hint %i as %i", 
			  ((orientation == GTK_ORIENTATION_HORIZONTAL)?"h":"v"),
			  G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(area)), 
			  width_hint,
			  cache->last_calculated_requisition);
#endif
	}

	return cache->last_calculated_requisition;
}

gint
cong_editor_area_get_requisition_width (CongEditorArea *area,
					int width_hint)
{
	return cong_editor_area_get_requisition (area,
						 GTK_ORIENTATION_HORIZONTAL,
						 width_hint);

}

gint
cong_editor_area_get_requisition_height (CongEditorArea *area,
					 int width_hint)
{
	return cong_editor_area_get_requisition (area,
						 GTK_ORIENTATION_VERTICAL,
						 width_hint);
}

gint
cong_editor_area_get_cached_requisition (CongEditorArea *area,
					 GtkOrientation orientation)
{
	RequisitionCache *cache;

	g_return_val_if_fail (IS_CONG_EDITOR_AREA(area), 0);

	cache = &PRIVATE(area)->requisition_cache[orientation];

	return cache->last_calculated_requisition;
}


#if 0
void 
cong_editor_area_set_requisition (CongEditorArea *area,
				  gint width,
				  gint height)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA(area));

	PRIVATE(area)->requisition.width = width;
	PRIVATE(area)->requisition.height = height;
}
#endif

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

void
cong_editor_area_debug_render_state (CongEditorArea *area)
{
	switch (cong_editor_area_get_state (area)) {
	default: break;
	case GTK_STATE_PRELIGHT: 
		cong_editor_area_debug_render_area (area,
						    cong_editor_widget3_get_test_gc (cong_editor_area_get_widget (area)));
		break;
	case GTK_STATE_SELECTED:
		/* Render solid blue outline for now: */
		{
			GdkGC *gc = gdk_gc_new (GDK_DRAWABLE(cong_editor_area_get_gdk_window(area)));
			GdkColor col;

			col_to_gcol (&col, 0x0000ff);
			gdk_colormap_alloc_color(cong_gui_get_a_window()->style->colormap, &col, FALSE, TRUE);

			gdk_gc_set_foreground (gc, &col);
			gdk_draw_rectangle (GDK_DRAWABLE(cong_editor_area_get_gdk_window(area)),
					    gc,
					    FALSE,
					    PRIVATE(area)->window_area.x,
					    PRIVATE(area)->window_area.y,
					    PRIVATE(area)->window_area.width-1,
					    PRIVATE(area)->window_area.height-1);
		}
		break;		
	}

}


/* CongEditorArea methods: */
CongEditorArea*
cong_editor_area_get_parent (CongEditorArea *area)
{
	g_return_val_if_fail (IS_CONG_EDITOR_AREA(area), NULL);

	return PRIVATE(area)->parent_area;
}

static gboolean
do_recursive_render (CongEditorArea *area,
		     gpointer user_data)
{
	cong_editor_area_recursive_render (area,
					   (const GdkRectangle*)user_data);

	return FALSE;
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

#if DEBUG_RENDER_ALLOCATIONS
		/* Render test rectangle to show this area directly: */		
		{
			
			cong_editor_area_debug_render_area (area,
							    cong_editor_widget3_get_test_gc (cong_editor_area_get_widget (area)));
		}
#endif

#if DEBUG_RENDERING
		g_message("%s::render_self(%i,%i,%i,%i)",
			  G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(area)), 
			  intersected_area.x,
			  intersected_area.y,
			  intersected_area.width,
			  intersected_area.height);
#endif

		/* Render self: */
		CONG_EEL_CALL_METHOD (CONG_EDITOR_AREA_CLASS,
				      area,
				      render_self, 
				      (area, &intersected_area));
		
		/* Recurse over all children (internal and non-internal): */
		cong_editor_area_for_all (area, do_recursive_render, &intersected_area);


	}
}

gboolean
cong_editor_area_on_button_press (CongEditorArea *editor_area, 
				  GdkEventButton *event)
{
	gboolean result;

	g_return_val_if_fail (editor_area, FALSE);
	g_return_val_if_fail (event, FALSE);
	
	g_signal_emit (G_OBJECT(editor_area),
		       signals[BUTTON_PRESS_EVENT], 0,
		       event,
		       &result);

	return result;
}

gboolean
cong_editor_area_on_motion_notify (CongEditorArea *editor_area, 
				   GdkEventMotion *event)
{
	gboolean result;

	g_return_val_if_fail (editor_area, FALSE);
	g_return_val_if_fail (event, FALSE);
	
	g_signal_emit (G_OBJECT(editor_area),
		       signals[MOTION_NOTIFY_EVENT], 0,
		       event,
		       &result);

	return result;
}

gboolean
cong_editor_area_on_key_press (CongEditorArea *editor_area, 
			       GdkEventKey *event)
{
	gboolean result;

	g_return_val_if_fail (editor_area, FALSE);
	g_return_val_if_fail (event, FALSE);
	
	g_signal_emit (G_OBJECT(editor_area),
		       signals[KEY_PRESS_EVENT], 0,
		       event,
		       &result);

	return result;
}

guint
cong_editor_area_calc_requisition (CongEditorArea *editor_area, 
				   GtkOrientation orientation,
				   int width_hint)
{
	g_return_val_if_fail (editor_area, 0);

	return CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_AREA_CLASS,
						       editor_area,
						       calc_requisition, 
						       (editor_area, orientation, width_hint));
}

void 
cong_editor_area_set_allocation (CongEditorArea *editor_area,
				 gint x,
				 gint y,
				 gint width,
				 gint height)
{
	gboolean has_changed = TRUE;

	g_return_if_fail (editor_area);

	if (PRIVATE(editor_area)->window_area.x == x) {
		if (PRIVATE(editor_area)->window_area.y == y) {
			if (PRIVATE(editor_area)->window_area.width == width) {
				if (PRIVATE(editor_area)->window_area.height == height) {
					has_changed = FALSE;
				}
			}
		}
	}

#if DEBUG_ALLOCATIONS
	g_message ("cong_editor_area_set_allocation(%i,%i,%i,%i) on %s: changed: %s needs_recursive: %s", 
		   x, y, width, height, 
		   G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(editor_area)),
		   (has_changed?"YES":"NO "),
		   (PRIVATE(editor_area)->needs_recursive_allocation?"YES":"NO "));
#endif


	if (has_changed) {
		PRIVATE(editor_area)->window_area.x = x;
		PRIVATE(editor_area)->window_area.y = y;
		PRIVATE(editor_area)->window_area.width = width;
		PRIVATE(editor_area)->window_area.height = height;
	}

	if (has_changed || PRIVATE(editor_area)->needs_recursive_allocation) {

		PRIVATE(editor_area)->needs_recursive_allocation = FALSE;
		
		/* Call hook to recursively allocate space to children: */
		CONG_EEL_CALL_METHOD (CONG_EDITOR_AREA_CLASS,
				      editor_area,
				      allocate_child_space, 
				      (editor_area));
		
		cong_editor_area_queue_redraw (editor_area);
	}
}


void
cong_editor_area_queue_redraw (CongEditorArea *editor_area)
{
	const GdkRectangle *rect;
	
	g_return_if_fail (IS_CONG_EDITOR_AREA(editor_area));
	
	rect = &PRIVATE(editor_area)->window_area;
	
	gtk_widget_queue_draw_area (GTK_WIDGET(cong_editor_area_get_widget (editor_area)),
				    rect->x,
				    rect->y,
				    rect->width,
				    rect->height);
}

void
cong_editor_area_flush_requisition_cache (CongEditorArea *editor_area,
					  GtkOrientation orientation)
{
	RequisitionCache *cache;

	g_return_if_fail (IS_CONG_EDITOR_AREA(editor_area));

	cache = &PRIVATE(editor_area)->requisition_cache[orientation];
	
	if (cache->requisition_cache_valid) {

#if DEBUG_REQUISITIONS
		g_message("flush_requisition_cache called on a valid cache (%s)", G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(editor_area)));
#endif

		cache->requisition_cache_valid = FALSE;

		/* Ensure that allocation messages don't get optimised away: */
		PRIVATE(editor_area)->needs_recursive_allocation = TRUE;

		g_signal_emit (G_OBJECT(editor_area),
			       signals[FLUSH_REQUISITION_CACHE], 0,
			       orientation);
	}
}

CongEditorArea*
cong_editor_area_for_all (CongEditorArea *editor_area, 
			  CongEditorAreaCallbackFunc func, 
			  gpointer user_data)
{
	g_return_val_if_fail (IS_CONG_EDITOR_AREA(editor_area), NULL);
	g_return_val_if_fail (func, NULL);

	return CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_AREA_CLASS,
						       editor_area,
						       for_all, 
						       (editor_area, func, user_data));
	
}

#if 0
struct recursion_data
{
	CongEditorAreaCallbackFunc pre_func;
	CongEditorAreaCallbackFunc post_func;
	gpointer user_data;
};

static CongEditorArea*
for_all_recurse(CongEditorArea *editor_area, 
		gpointer user_data)
{
	struct recursion_data *rec_data = (struct recursion_data *)user_data;

	CongEditorArea *child_result;
       
	if (rec_data->pre_func) {

		if ( rec_data->pre_func(editor_area, rec_data->user_data)) {
			/* Stop traversal: */
			return editor_area;
		}
	}
	
	child_result =  cong_editor_area_for_all (editor_area, 
						  for_all_recurse, 
						  user_data);

	if (child_result) {
		return child_result;
	}

	if (rec_data->post_func) {

		if ( rec_data->post_func(editor_area, rec_data->user_data)) {
			/* Stop traversal: */
			return editor_area;
		}
	}

	return NULL;	
}


CongEditorArea*
cong_editor_area_recurse (CongEditorArea *editor_area, 
			  CongEditorAreaCallbackFunc pre_func, 
			  CongEditorAreaCallbackFunc post_func, 
			  gpointer user_data)
{
	struct recursion_data rec_data;

	g_return_if_fail (IS_CONG_EDITOR_AREA(editor_area));

	rec_data.pre_func = pre_func;
	rec_data.post_func = post_func;
	rec_data.user_data = user_data;

	return cong_editor_area_for_all (editor_area, 
					 for_all_recurse, 
					 &rec_data);
}
#endif

gboolean
cong_editor_area_covers_xy (CongEditorArea *editor_area, 
			    gint x,
			    gint y)
{
	return cong_eel_rectangle_contains (&PRIVATE(editor_area)->window_area,
					    x,
					    y);
}

struct search_for_xy
{
	gint x;
	gint y;
};

static gboolean is_area_at_xy (CongEditorArea *editor_area, 
			       gpointer user_data)
{
	struct search_for_xy *search = (struct search_for_xy *)user_data;

	return cong_editor_area_covers_xy (editor_area,
					   search->x,
					   search->y);	
}

/* Function gets immediate child (either "internal" or "non-internal") at the coords, if any: */
CongEditorArea*
cong_editor_area_get_immediate_child_at (CongEditorArea *area,
					 gint x,
					 gint y)
{
	struct search_for_xy search;

	search.x = x;
	search.y = y;

	return cong_editor_area_for_all (area, 
					 is_area_at_xy,
					 &search);
}
CongEditorArea*
cong_editor_area_get_deepest_child_at (CongEditorArea *area,
				       gint x,
				       gint y)
{
	CongEditorArea* child;

	/* Are we present at the location? */
	if (cong_editor_area_covers_xy (area,
					x,
					y)) {

		/* Recurse to children: */
		child = cong_editor_area_get_immediate_child_at (area,
								 x,
								 y);

		if (child) {
			/* Then recurse into this child: */
			return cong_editor_area_get_deepest_child_at (child,
								      x,
								      y);
		} else {
			/* Then this is the deepest area at the coords: */
			return area;
		}
	}

	return NULL;
}

GdkWindow*
cong_editor_area_get_gdk_window(CongEditorArea *editor_area)
{
	g_return_val_if_fail (editor_area, NULL);

	return GTK_WIDGET(cong_editor_area_get_widget (editor_area))->window;
}

/* Associate the editor area with a particular editor node: */
void
cong_editor_area_connect_node_signals (CongEditorArea *area,
				       CongEditorNode *editor_node)
{
	CongDocument *doc;

	g_return_if_fail (IS_CONG_EDITOR_AREA (area));
	g_return_if_fail (IS_CONG_EDITOR_NODE (editor_node));
	g_return_if_fail (PRIVATE(area)->editor_node==NULL);

	doc = cong_editor_node_get_document (editor_node);

	PRIVATE(area)->editor_node = editor_node;

	g_signal_connect (area,
			  "button_press_event",
			  G_CALLBACK(on_signal_button_press_for_area_with_node),
			  editor_node);

	g_signal_connect (area,
			  "motion_notify_event",
			  G_CALLBACK(on_signal_motion_notify_for_area_with_node),
			  editor_node);

	/* FIXME: need to disconnect these signals eventually: */
	PRIVATE(area)->selection_change_handler_id = g_signal_connect (editor_node,
								       "is_selected_changed",
								       G_CALLBACK(on_signal_is_selected_changed_for_area_with_node),
								       area);
}

/* Protected stuff: */
void
cong_editor_area_protected_postprocess_add_internal_child (CongEditorArea *area,
							   CongEditorArea *internal_child)
{
	g_return_if_fail ( IS_CONG_EDITOR_AREA(area));
	g_return_if_fail ( IS_CONG_EDITOR_AREA(internal_child));

	g_signal_connect (G_OBJECT(internal_child),
			  "flush_requisition_cache",
			  G_CALLBACK(on_child_flush_requisition_cache),
			  area);
}

void
cong_editor_area_protected_set_parent (CongEditorArea *area,
				       CongEditorArea *parent)
{
	g_return_if_fail ( IS_CONG_EDITOR_AREA(area));
	g_return_if_fail ( IS_CONG_EDITOR_AREA(parent));

	g_assert(NULL==PRIVATE(area)->parent_area);

	PRIVATE(area)->parent_area = parent;

	/* FIXME: refcount issues? */
}


/* Signal handler definitions: */
static void
on_child_flush_requisition_cache (CongEditorArea *child_area,
				  GtkOrientation orientation,
				  gpointer user_data)
{
	CongEditorArea *fake_parent_area = CONG_EDITOR_AREA(user_data);

#if DEBUG_REQUISITIONS
	g_message("on_child_flush_requisition_cache");
#endif

	g_return_if_fail (IS_CONG_EDITOR_AREA(child_area) );
	
	/* One of children has changed its requisition; so must we: */
	/* FIXME: we flush our cache in both axes, just to be sure... */
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(fake_parent_area), GTK_ORIENTATION_HORIZONTAL);
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(fake_parent_area), GTK_ORIENTATION_VERTICAL);
}

static gboolean
on_signal_button_press_for_area_with_node (CongEditorArea *editor_area, 
					   GdkEventButton *event,
					   gpointer user_data)
{

	CongEditorNode *editor_node = CONG_EDITOR_NODE(user_data);

	CongEditorWidget3* editor_widget;			
	CongDocument* doc;

	editor_widget = cong_editor_area_get_widget (editor_area);			
	doc = cong_editor_area_get_document (editor_area);

	/* Which button was pressed? */
	switch (event->button) {
	default: return FALSE;
	case 1: /* Normally the left mouse button: */
		{
			CongNodePtr node = cong_editor_node_get_node (editor_node);

			cong_document_select_node (doc, node);
		}
		return TRUE;

	case 3: /* Normally the right mouse button: */
		{
			GtkWidget* menu;

			menu = cong_ui_popup_init(doc, 
						  cong_editor_node_get_node (editor_node),
						  GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(editor_widget))));
			gtk_menu_popup (GTK_MENU(menu), 
					NULL, 
					NULL, 
					NULL, 
					NULL, event->button,
					event->time);		      
		}
		return TRUE;
	}		
}

static gboolean
on_signal_motion_notify_for_area_with_node (CongEditorArea *editor_area, 
					    GdkEventButton *event,
					    gpointer user_data)
{
#if 0
	CongEditorNode *editor_node = CONG_EDITOR_NODE(user_data);
#endif

	CongEditorWidget3* editor_widget;			
	CongDocument* doc;

	editor_widget = cong_editor_area_get_widget (editor_area);			
	doc = cong_editor_area_get_document (editor_area);

	cong_editor_widget3_set_prehighlight_editor_area (editor_widget,
							  editor_area);

	return TRUE;
}

static GtkStateType
calc_state (CongEditorArea *area)
{
	CongEditorWidget3 *editor_widget;
	CongDocument* doc;


	g_return_val_if_fail (IS_CONG_EDITOR_AREA (area), GTK_STATE_NORMAL);
	
	editor_widget = cong_editor_area_get_widget (area);
	doc = cong_editor_area_get_document (area);

	if (PRIVATE(area)->editor_node) {
		if (cong_editor_node_is_selected (PRIVATE(area)->editor_node)) {
			return GTK_STATE_SELECTED;
		}
	}

	if (cong_editor_widget3_get_prehighlight_editor_area (editor_widget) == area) {
		return GTK_STATE_PRELIGHT;
	}

	return GTK_STATE_NORMAL;
}

static void
on_signal_is_selected_changed_for_area_with_node (CongEditorNode *editor_node,
						  gpointer user_data)
{
	CongEditorArea *editor_area = CONG_EDITOR_AREA (user_data);

	g_assert (PRIVATE(editor_area)->editor_node);

	cong_editor_area_set_state (editor_area,
				    calc_state (editor_area));	
}

static void
finalize (GObject *object)
{
	CongEditorArea *editor_area = CONG_EDITOR_AREA (object);

	g_message ("cong_editor_area::finalize");
	
	g_free (editor_area->private);
	editor_area->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
dispose (GObject *object)
{
	CongEditorArea *editor_area = CONG_EDITOR_AREA (object);

	g_message ("cong_editor_area::dispose");

	g_assert (editor_area->private);

	if (PRIVATE (editor_area)->selection_change_handler_id) {
		CongDocument *doc = cong_editor_area_get_document (editor_area);

		g_signal_handler_disconnect (doc,
					     PRIVATE(editor_area)->selection_change_handler_id);

		PRIVATE(editor_area)->selection_change_handler_id = 0;		
	}

	if ( PRIVATE (editor_area)->cursor) {
		gdk_cursor_unref (PRIVATE (editor_area)->cursor);
		PRIVATE (editor_area)->cursor = NULL;
	}

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}
