/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-composer.c
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
#include "cong-editor-area-composer.h"
#include <libgnome/gnome-macros.h>

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaComposerDetails
{
	GList *list_of_children;
	GtkOrientation orientation;
	guint spacing;
};

/* Method implementation prototypes: */
static void 
update_requisition (CongEditorArea *area, 
		    int width_hint);

static void
allocate_child_space (CongEditorArea *area);

static void 
for_all (CongEditorArea *editor_area, 
	 CongEditorCallbackFunc func, 
	 gpointer user_data);

static void
add_child (CongEditorAreaContainer *area_container,
	   CongEditorArea *child);


/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaComposer, 
			cong_editor_area_composer,
			CongEditorAreaContainer,
			CONG_EDITOR_AREA_CONTAINER_TYPE );

static void
cong_editor_area_composer_class_init (CongEditorAreaComposerClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);
	CongEditorAreaContainerClass *container_klass = CONG_EDITOR_AREA_CONTAINER_CLASS(klass);

	area_klass->update_requisition = update_requisition;
	area_klass->allocate_child_space = allocate_child_space;
	area_klass->for_all = for_all;

	container_klass->add_child = add_child;
}

static void
cong_editor_area_composer_instance_init (CongEditorAreaComposer *area_composer)
{
	area_composer->private = g_new0(CongEditorAreaComposerDetails,1);
}

/* Exported function definitions: */
CongEditorArea*
cong_editor_area_composer_construct (CongEditorAreaComposer *area_composer,
				     CongEditorWidget3 *editor_widget,
				     GtkOrientation orientation,
				     guint spacing)
{
	cong_editor_area_container_construct (CONG_EDITOR_AREA_CONTAINER(area_composer),
					      editor_widget);

	PRIVATE(area_composer)->orientation = orientation;
	PRIVATE(area_composer)->spacing = spacing;

	return CONG_EDITOR_AREA (area_composer);
}

CongEditorArea*
cong_editor_area_composer_new (CongEditorWidget3 *editor_widget,
			       GtkOrientation orientation,
			       guint spacing)
{
	return cong_editor_area_composer_construct
		(g_object_new (CONG_EDITOR_AREA_COMPOSER_TYPE, NULL),
		 editor_widget,
		 orientation,
		 spacing);
}

/* Method implementation definitions: */
static void 
update_requisition (CongEditorArea *area, 
		    int width_hint)
{
	CongEditorAreaComposer *area_composer = CONG_EDITOR_AREA_COMPOSER(area);
	GtkRequisition result;
	GList *iter;
	int child_count = 0;

	g_message ("composer::update_requisition");

	result.width = 0;
	result.height = 0;

	/* Get size requisition for all kids, add in the appropriate axis: */
	for (iter = PRIVATE(area_composer)->list_of_children; iter; iter=iter->next) {
		CongEditorArea *child;
		const GtkRequisition *child_req;

		child = CONG_EDITOR_AREA(iter->data);

		cong_editor_area_update_requisition (child, 
						     width_hint);

		child_req = cong_editor_area_get_requisition (child);
		g_assert(child_req);

		if (PRIVATE(area_composer)->orientation == GTK_ORIENTATION_HORIZONTAL) {
			result.width += child_req->width;
			if (result.height<child_req->height) {
				result.height = child_req->height;
			}
		} else {
			result.height += child_req->height;
			if (result.width<child_req->width) {
				result.width = child_req->width;
			}
		}
		
		child_count++;
	}

	if (child_count>1) {
		if (PRIVATE(area_composer)->orientation == GTK_ORIENTATION_HORIZONTAL) {
			result.width += PRIVATE(area_composer)->spacing * (child_count-1);
		} else {
			result.height += PRIVATE(area_composer)->spacing * (child_count-1);
		}		
	}

	cong_editor_area_set_requisition (area,
					  result.width,
					  result.height);
}

static void
allocate_child_space (CongEditorArea *area)
{
#if 1
	CongEditorAreaComposer *area_composer = CONG_EDITOR_AREA_COMPOSER(area);
	GList *iter;
	gint x;
	gint y;
	const GdkRectangle *rect = cong_editor_area_get_window_coords(area);

	x = rect->x;
	y = rect->y;

	for (iter = PRIVATE(area_composer)->list_of_children; iter; iter=iter->next) {
		CongEditorArea *child = CONG_EDITOR_AREA(iter->data);
		const GtkRequisition *child_req = cong_editor_area_get_requisition (child);
		g_assert(child_req);

		cong_editor_area_set_allocation (child,
						 x,
						 y,
						 child_req->width,
						 child_req->height);

		if (PRIVATE(area_composer)->orientation == GTK_ORIENTATION_HORIZONTAL) {
			x += child_req->width + PRIVATE(area_composer)->spacing;
		} else {
			y += child_req->height + PRIVATE(area_composer)->spacing;
		}
	}
#else
	g_message("unimplemented (area_composer->allocate_child_space)");
#endif
}

static void 
for_all (CongEditorArea *editor_area, 
	 CongEditorCallbackFunc func, 
	 gpointer user_data)
{
	GList *iter;
	CongEditorAreaComposer *area_composer = CONG_EDITOR_AREA_COMPOSER(editor_area);

	for (iter = PRIVATE(area_composer)->list_of_children; iter; iter=iter->next) {
		(*func)(CONG_EDITOR_AREA(iter->data), user_data);
	}
}


static void
add_child ( CongEditorAreaContainer *area_container,
	    CongEditorArea *child)
{
	CongEditorAreaComposer *area_composer = CONG_EDITOR_AREA_COMPOSER(area_container);

	PRIVATE(area_composer)->list_of_children = g_list_append (PRIVATE(area_composer)->list_of_children, 
								  child);

	/* FIXME: need to flag things as being invalid etc... */
}
