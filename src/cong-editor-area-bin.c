/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-bin.c
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
#include "cong-editor-area-bin.h"
#include <libgnome/gnome-macros.h>

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaBinDetails
{
	CongEditorArea *only_child;
};

/* Method implementation prototypes: */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orienation,
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
static void
add_child_after (CongEditorAreaContainer *area_container,
		 CongEditorArea *new_child,
		 CongEditorArea *relative_to);
static void
remove_child (CongEditorAreaContainer *area_container,
	      CongEditorArea *child);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaBin, 
			cong_editor_area_bin,
			CongEditorAreaContainer,
			CONG_EDITOR_AREA_CONTAINER_TYPE );

static void
cong_editor_area_bin_class_init (CongEditorAreaBinClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);
	CongEditorAreaContainerClass *container_klass = CONG_EDITOR_AREA_CONTAINER_CLASS(klass);

	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
	area_klass->for_all = for_all;

	container_klass->add_child = add_child;
	container_klass->add_child_after = add_child_after;
	container_klass->remove_child = remove_child;

}

static void
cong_editor_area_bin_instance_init (CongEditorAreaBin *area_bin)
{
	area_bin->private = g_new0(CongEditorAreaBinDetails,1);
}

/* Exported function definitions: */
/**
 * cong_editor_area_bin_construct:
 * @area_bin:
 * @editor_widget:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_bin_construct (CongEditorAreaBin *area_bin,
				CongEditorWidget3 *editor_widget)
{
	cong_editor_area_container_construct (CONG_EDITOR_AREA_CONTAINER(area_bin),
					      editor_widget);

	return CONG_EDITOR_AREA (area_bin);
}

/**
 * cong_editor_area_bin_new:
 * @editor_widget:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_bin_new (CongEditorWidget3 *editor_widget)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_bin_new");
#endif
	
	return cong_editor_area_bin_construct
		(g_object_new (CONG_EDITOR_AREA_BIN_TYPE, NULL),
		 editor_widget);
}

/**
 * cong_editor_area_bin_get_child:
 * @area_bin:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_bin_get_child  (CongEditorAreaBin *area_bin)
{
	g_return_val_if_fail (IS_CONG_EDITOR_AREA_BIN(area_bin), NULL);

	return PRIVATE(area_bin)->only_child;
}


/* Method implementation definitions: */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	CongEditorAreaBin *bin;

	g_return_val_if_fail (IS_CONG_EDITOR_AREA_BIN(area), 0);

	bin = CONG_EDITOR_AREA_BIN(area);	

	if (PRIVATE(bin)->only_child) {

		return cong_editor_area_get_requisition (PRIVATE(bin)->only_child,
							 orientation,
							 width_hint);
	} else {
		return 0;
	}
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaBin *bin = CONG_EDITOR_AREA_BIN(area);

	if (PRIVATE(bin)->only_child) {
		const GdkRectangle *rect = cong_editor_area_get_window_coords(area);

		cong_editor_area_set_allocation (PRIVATE(bin)->only_child,
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
	CongEditorAreaBin *bin = CONG_EDITOR_AREA_BIN(editor_area);

	if (PRIVATE(bin)->only_child) {
		if ((*func)(PRIVATE(bin)->only_child, user_data)) {
			return PRIVATE(bin)->only_child;
		}
	}

	return NULL;
}

static void
add_child (CongEditorAreaContainer *area_container,
	   CongEditorArea *child,
	   gboolean add_to_end)
{
	CongEditorAreaBin *bin = CONG_EDITOR_AREA_BIN(area_container);

	if (PRIVATE(bin)->only_child) {
		g_error("cong_editor_area_bin::add_child called but already has a child");
	} else {
		g_object_ref (G_OBJECT(child));

		PRIVATE(bin)->only_child = child;
	}

	cong_editor_area_container_protected_postprocess_add_non_internal_child (area_container,
										 child);
	cong_editor_area_protected_set_parent (child,
					       CONG_EDITOR_AREA(area_container));
}

static void
add_child_after (CongEditorAreaContainer *area_container,
		 CongEditorArea *new_child,
		 CongEditorArea *relative_to)
{
	g_error("cong_editor_area_bin::add_child_after called; meaningless for a bin");
}

static void
remove_child (CongEditorAreaContainer *area_container,
	      CongEditorArea *child)
{
	CongEditorAreaBin *bin = CONG_EDITOR_AREA_BIN(area_container);

	g_return_if_fail (IS_CONG_EDITOR_AREA(child));

	if (child==PRIVATE(bin)->only_child) {
		PRIVATE(bin)->only_child = NULL;

		g_object_unref (G_OBJECT(child));
	} else {
		g_error("cong_editor_area_bin::remove_child called but with the wrong child");
	}


}
