/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area.h
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

#ifndef __CONG_EDITOR_AREA_H__
#define __CONG_EDITOR_AREA_H__

#include "cong-editor-widget.h"

G_BEGIN_DECLS

#define DEBUG_EDITOR_AREA_LIFETIMES 0

typedef struct CongEditorAreaDetails CongEditorAreaDetails;

#define CONG_EDITOR_AREA_TYPE	      (cong_editor_area_get_type ())
#define CONG_EDITOR_AREA(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_TYPE, CongEditorArea)
#define CONG_EDITOR_AREA_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_TYPE, CongEditorAreaClass)
#define IS_CONG_EDITOR_AREA(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_TYPE)

/* Return TRUE to stop traversal */
typedef gboolean (*CongEditorAreaCallbackFunc) (CongEditorArea *editor_area, 
						gpointer user_data);

struct CongEditorArea
{
	GObject object;

	CongEditorAreaDetails *private;
};

struct CongEditorAreaClass
{
	GObjectClass klass;

	void (*render_self) (CongEditorArea *area,
			     const GdkRectangle *widget_rect);

	gint (*calc_requisition) (CongEditorArea *area, 
				  GtkOrientation orientation,
				  int width_hint);

	void (*allocate_child_space) (CongEditorArea *area);

	CongEditorArea* (*for_all) (CongEditorArea *editor_area, 
				    CongEditorAreaCallbackFunc func, 
				    gpointer user_data);

	gboolean (*on_button_press) (CongEditorArea *editor_area, 
 				     GdkEventButton *event);

#if 0
	gboolean (*on_enter_notify) (GtkWidget *widget,
				     GdkEventCrossing *event,
				     gpointer user_data);
	gboolean (*on_leave_notify) (GtkWidget *widget,
				     GdkEventCrossing *event,
				     gpointer user_data);
#endif
	void (*on_motion_notify) (CongEditorArea *editor_area, 
				  GdkEventMotion *event);

	void (*on_key_press) (CongEditorArea *editor_area, 
			      GdkEventKey *event);

	/* Signal emission hooks: */
	void (*flush_requisition_cache) (CongEditorArea *area,
					 GtkOrientation orientation);

	void (*state_changed) (CongEditorArea *area);
};

GType
cong_editor_area_get_type (void);

CongEditorArea*
cong_editor_area_construct (CongEditorArea *area,
			    CongEditorWidget3 *editor_widget);

CongEditorWidget3*
cong_editor_area_get_widget (CongEditorArea *area);

CongDocument*
cong_editor_area_get_document (CongEditorArea *area);

gboolean 
cong_editor_area_is_hidden (CongEditorArea *area);

void
cong_editor_area_show (CongEditorArea *area);

void
cong_editor_area_hide (CongEditorArea *area);

GtkStateType
cong_editor_area_get_state (CongEditorArea *editor_area);

void
cong_editor_area_set_state (CongEditorArea *editor_area,
			    GtkStateType state);

GdkCursor*
cong_editor_area_get_cursor (CongEditorArea *area);

void
cong_editor_area_set_cursor (CongEditorArea *area,
			     GdkCursor *cursor);

const GdkRectangle*
cong_editor_area_get_window_coords (CongEditorArea *area);

guint
cong_editor_area_get_requisition (CongEditorArea *area,
				  GtkOrientation orientation,
				  int width_hint);

gint
cong_editor_area_get_requisition_width (CongEditorArea *area,
					int width_hint);

gint
cong_editor_area_get_requisition_height (CongEditorArea *area,
					 int width_hint);

gint
cong_editor_area_get_cached_requisition (CongEditorArea *area,
					 GtkOrientation orientation);

gint
cong_editor_area_get_allocation_width (CongEditorArea *area);

gint
cong_editor_area_get_allocation_height (CongEditorArea *area);

void 
cong_editor_area_debug_render_area (CongEditorArea *area,
				    GdkGC *gc);

void
cong_editor_area_debug_render_state (CongEditorArea *area);

/* CongEditorArea methods: */
CongEditorArea*
cong_editor_area_get_parent (CongEditorArea *area);

void
cong_editor_area_recursive_render (CongEditorArea *area,
				   const GdkRectangle *widget_rect);

gboolean
cong_editor_area_on_button_press (CongEditorArea *editor_area, 
				  GdkEventButton *event);

gboolean
cong_editor_area_on_motion_notify (CongEditorArea *editor_area, 
				   GdkEventMotion *event);

gboolean
cong_editor_area_on_key_press (CongEditorArea *editor_area, 
			       GdkEventKey *event);

guint
cong_editor_area_calc_requisition (CongEditorArea *editor_area, 
				   GtkOrientation orientation,
				   int width_hint);

gint
cong_editor_area_calc_requisition_width (CongEditorArea *editor_area, 
					 int width_hint);

gint
cong_editor_area_calc_requisition_height (CongEditorArea *editor_area, 
					  int width_hint);

void 
cong_editor_area_set_allocation (CongEditorArea *editor_area,
				 gint x,
				 gint y,
				 gint width,
				 gint height);

void
cong_editor_area_queue_redraw (CongEditorArea *editor_area);

void
cong_editor_area_flush_requisition_cache (CongEditorArea *editor_area,
					  GtkOrientation orientation);

/* Iterate over all children of this area, even "internal" ones.
 * Return value: the child that stopped the traveral, of NULL if none did
 */
CongEditorArea*
cong_editor_area_for_all (CongEditorArea *editor_area, 
			  CongEditorAreaCallbackFunc func, 
			  gpointer user_data);

/* Recurse over all children of this area, even "internal" ones.
 * At each node, call the pre_func, then recurse, then call the post_func.
 * Return value: the child that stopped the traveral, or NULL if none did
 */
CongEditorArea*
cong_editor_area_recurse (CongEditorArea *editor_area, 
			  CongEditorAreaCallbackFunc pre_func, 
			  CongEditorAreaCallbackFunc post_func, 
			  gpointer user_data);

gboolean
cong_editor_area_covers_xy (CongEditorArea *editor_area, 
			    gint x,
			    gint y);

/* Function gets immediate child (either "internal" or "non-internal") at the coords, if any: */
CongEditorArea*
cong_editor_area_get_immediate_child_at (CongEditorArea *area,
					 gint x,
					 gint y);

CongEditorArea*
cong_editor_area_get_deepest_child_at (CongEditorArea *area,
				       gint x,
				       gint y);


/* Handy utilities: */
GdkWindow*
cong_editor_area_get_gdk_window(CongEditorArea *editor_area);

/* Associate the editor area with a particular editor node: */
void
cong_editor_area_connect_node_signals (CongEditorArea *area,
				       CongEditorNode *editor_node);

/* Protected stuff: */
void
cong_editor_area_protected_postprocess_add_internal_child (CongEditorArea *area,
							   CongEditorArea *internal_child);

void
cong_editor_area_protected_set_parent (CongEditorArea *area,
				       CongEditorArea *parent);


G_END_DECLS

#endif
