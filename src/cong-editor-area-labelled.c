/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-labelled.c
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
#include "cong-editor-area-labelled.h"
#include <libgnome/gnome-macros.h>

#include "cong-eel.h"
#include "cong-app.h"
#include "cong-editor-area-text.h"
#include "cong-editor-area-composer.h"
#include "cong-editor-area-bin.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaLabelledDetails
{
	CongEditorArea *outer_hcompose;
	CongEditorArea *label;
	CongEditorArea *inner_area;
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
	   CongEditorArea *child,
	   gboolean add_to_end);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaLabelled, 
			cong_editor_area_labelled,
			CongEditorAreaBin,
			CONG_EDITOR_AREA_BIN_TYPE );

static void
cong_editor_area_labelled_class_init (CongEditorAreaLabelledClass *klass)
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
cong_editor_area_labelled_instance_init (CongEditorAreaLabelled *area_labelled)
{
	area_labelled->private = g_new0(CongEditorAreaLabelledDetails,1);
}


/* Exported function definitions: */
/**
 * cong_editor_area_labelled_construct:
 * @area_labelled:
 * @editor_widget:
 * @label:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_labelled_construct (CongEditorAreaLabelled *area_labelled,
				     CongEditorWidget3 *editor_widget,
				     CongEditorArea *label)
{
	cong_editor_area_bin_construct (CONG_EDITOR_AREA_BIN(area_labelled),
					editor_widget);

	PRIVATE(area_labelled)->outer_hcompose = cong_editor_area_composer_new (editor_widget,
					       GTK_ORIENTATION_HORIZONTAL,
					       0);

	PRIVATE(area_labelled)->label = label;
	
	cong_editor_area_composer_pack_end ( CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_labelled)->outer_hcompose),
					     CONG_EDITOR_AREA(PRIVATE(area_labelled)->label),
					     FALSE,
					     FALSE,
					     10
					     );
	
	PRIVATE(area_labelled)->inner_area = cong_editor_area_bin_new (editor_widget);
	cong_editor_area_composer_pack_end ( CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_labelled)->outer_hcompose),
					     PRIVATE(area_labelled)->inner_area,
					     TRUE,
					     TRUE,
					     0
					     );		

	cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (area_labelled),
								   PRIVATE(area_labelled)->outer_hcompose);

	cong_editor_area_protected_set_parent (PRIVATE(area_labelled)->outer_hcompose,
					       CONG_EDITOR_AREA (area_labelled));

	return CONG_EDITOR_AREA (area_labelled);
}

/**
 * cong_editor_area_labelled_new:
 * @editor_widget:
 * @label:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_labelled_new (CongEditorWidget3 *editor_widget,
			       CongEditorArea *label)

{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_labelled_new");
#endif

	return cong_editor_area_labelled_construct
		(g_object_new (CONG_EDITOR_AREA_LABELLED_TYPE, NULL),
		 editor_widget,
		 label);
}

/**
 * cong_editor_area_labelled_set_label:
 * @area_labelled:
 * @label:
 *
 * TODO: Write me
 */
void
cong_editor_area_labelled_set_label (CongEditorAreaLabelled *area_labelled,
				     CongEditorArea *label)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA_LABELLED (area_labelled));
	g_return_if_fail (label);

	g_assert_not_reached();

	/* FIXME: this has not been implemented properly; need to remove old label from container, then add new one... */

	PRIVATE(area_labelled)->label = label;

	cong_editor_area_protected_set_parent (PRIVATE(area_labelled)->outer_hcompose,
					       CONG_EDITOR_AREA (area_labelled));

	cong_editor_area_container_children_changed (CONG_EDITOR_AREA_CONTAINER (area_labelled));
}

/* Method implementation definitions: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect)
{
	cong_editor_area_debug_render_state (area);
}

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	CongEditorAreaLabelled *labelled = CONG_EDITOR_AREA_LABELLED(area);

	if (PRIVATE(labelled)->outer_hcompose) {

		return cong_editor_area_get_requisition (PRIVATE(labelled)->outer_hcompose,
							 orientation,
							 width_hint);
	} else {
		return 0;
	}
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaLabelled *labelled = CONG_EDITOR_AREA_LABELLED(area);

	if (PRIVATE(labelled)->outer_hcompose) {
		const GdkRectangle *rect = cong_editor_area_get_window_coords(area);

		cong_editor_area_set_allocation (PRIVATE(labelled)->outer_hcompose,
						 rect->x,
						 rect->y,
						 rect->width,
						 rect->height);
	}

}

static CongEditorArea*
for_all (CongEditorArea *editor_area, 
	 CongEditorAreaCallbackFunc func, 
	 gpointer user_data)
{
	CongEditorAreaLabelled *labelled = CONG_EDITOR_AREA_LABELLED(editor_area);

	if (PRIVATE(labelled)->outer_hcompose) {
		if ((*func)(PRIVATE(labelled)->outer_hcompose, user_data)) {
			return PRIVATE(labelled)->outer_hcompose;
		}
	}

	return NULL;
}

static void
add_child (CongEditorAreaContainer *area_container,
	   CongEditorArea *child,
	   gboolean add_to_end)
{
	CongEditorAreaLabelled *labelled = CONG_EDITOR_AREA_LABELLED(area_container);

	g_assert(PRIVATE(labelled)->inner_area);

	cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER( PRIVATE(labelled)->inner_area),
					       child,
					       add_to_end);
}
