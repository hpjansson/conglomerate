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

G_BEGIN_DECLS


typedef struct CongEditorArea CongEditorArea;
typedef struct CongEditorAreaClass CongEditorAreaClass;
typedef struct CongEditorAreaDetails CongEditorAreaDetails;

#define CONG_EDITOR_AREA_TYPE	      (cong_editor_area_get_type ())
#define CONG_EDITOR_AREA(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_TYPE, CongEditorArea)
#define CONG_EDITOR_AREA_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_TYPE, CongEditorAreaClass)
#define IS_CONG_EDITOR_AREA(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_TYPE)

typedef void (*CongEditorAreaCallbackFunc) (CongEditorArea *editor_area, 
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

	void (*update_requisition) (CongEditorArea *area, 
				    int width_hint);

	void (*allocate_child_space) (CongEditorArea *area);

	void (*for_all) (CongEditorArea *editor_area, 
			 CongEditorAreaCallbackFunc func, 
			 gpointer user_data);

};

GType
cong_editor_area_get_type (void);

CongEditorArea*
cong_editor_area_construct (CongEditorArea *area,
			    CongEditorWidget3 *editor_widget);

CongEditorWidget3*
cong_editor_area_get_widget (CongEditorArea *area);

gboolean 
cong_editor_area_is_hidden (CongEditorArea *area);

const GdkRectangle*
cong_editor_area_get_window_coords (CongEditorArea *area);

const GtkRequisition*
cong_editor_area_get_requisition (CongEditorArea *area);

void 
cong_editor_area_set_requisition (CongEditorArea *area,
				  gint width,
				  gint height);

void 
cong_editor_area_debug_render_area (CongEditorArea *area,
				    GdkGC *gc);

/* CongEditorArea methods: */
void
cong_editor_area_recursive_render (CongEditorArea *area,
				   const GdkRectangle *widget_rect);

void
cong_editor_area_on_button_press (CongEditorArea *editor_area, 
				  GdkEventButton *event);

void
cong_editor_area_on_motion_notify (CongEditorArea *editor_area, 
				   GdkEventMotion *event);

void
cong_editor_area_on_key_press (CongEditorArea *editor_area, 
			       GdkEventKey *event);

void 
cong_editor_area_update_requisition (CongEditorArea *editor_area, 
				     int width_hint);

void 
cong_editor_area_set_allocation (CongEditorArea *editor_area,
				 gint x,
				 gint y,
				 gint width,
				 gint height);


/* Iterate over all children of this area, even "internal" ones: */
void
cong_editor_area_for_all (CongEditorArea *editor_area, 
			  CongEditorAreaCallbackFunc func, 
			  gpointer user_data);

/* Handy utilities: */
GdkWindow*
cong_editor_area_get_gdk_window(CongEditorArea *editor_area);

G_END_DECLS

#endif
