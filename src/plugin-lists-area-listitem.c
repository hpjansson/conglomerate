/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-lists-area-listitem.c
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
#include "plugin-lists-area-listitem.h"
#include <libgnome/gnome-macros.h>

#include "cong-eel.h"
#include "cong-app.h"
#include "cong-editor-area-text.h"
#include "cong-editor-area-composer.h"
#include "cong-editor-area-bin.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaListitemDetails
{
	CongEditorArea *outer_hcompose;
	CongEditorAreaText *label;
	CongEditorArea *inner_area;
};

/* Method implementation prototypes: */
static void 
calc_requisition (CongEditorArea *area, 
		  int width_hint,
		  GtkRequisition *output);

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
GNOME_CLASS_BOILERPLATE(CongEditorAreaListitem, 
			cong_editor_area_listitem,
			CongEditorAreaBin,
			CONG_EDITOR_AREA_BIN_TYPE );

static void
cong_editor_area_listitem_class_init (CongEditorAreaListitemClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);
	CongEditorAreaContainerClass *container_klass = CONG_EDITOR_AREA_CONTAINER_CLASS(klass);

	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
	area_klass->for_all = for_all;

	container_klass->add_child = add_child;

}

static void
cong_editor_area_listitem_instance_init (CongEditorAreaListitem *area_listitem)
{
	area_listitem->private = g_new0(CongEditorAreaListitemDetails,1);
}


/* Exported function definitions: */
CongEditorArea*
cong_editor_area_listitem_construct (CongEditorAreaListitem *area_listitem,
				     CongEditorWidget3 *editor_widget,
				     const gchar *label)
{
	cong_editor_area_bin_construct (CONG_EDITOR_AREA_BIN(area_listitem),
					editor_widget);

	PRIVATE(area_listitem)->outer_hcompose = cong_editor_area_composer_new (editor_widget,
					       GTK_ORIENTATION_HORIZONTAL,
					       0);

	PRIVATE(area_listitem)->label = CONG_EDITOR_AREA_TEXT(cong_editor_area_text_new (editor_widget,
											 cong_app_singleton()->fonts[CONG_FONT_ROLE_TITLE_TEXT], 
											 NULL,
											 label,
											 FALSE));
	
	cong_editor_area_composer_pack ( CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_listitem)->outer_hcompose),
					 CONG_EDITOR_AREA(PRIVATE(area_listitem)->label),
					 FALSE,
					 FALSE,
					 10
					 );
	
	PRIVATE(area_listitem)->inner_area = cong_editor_area_bin_new (editor_widget);
	cong_editor_area_composer_pack ( CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_listitem)->outer_hcompose),
					 PRIVATE(area_listitem)->inner_area,
					 TRUE,
					 TRUE,
					 0
					 );		

	cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (area_listitem),
								   PRIVATE(area_listitem)->outer_hcompose);

	cong_editor_area_protected_set_parent (PRIVATE(area_listitem)->outer_hcompose,
					       CONG_EDITOR_AREA (area_listitem));

	return CONG_EDITOR_AREA (area_listitem);
}

CongEditorArea*
cong_editor_area_listitem_new (CongEditorWidget3 *editor_widget,
			       const gchar *label)

{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_listitem_new(%s)", label);
#endif

	return cong_editor_area_listitem_construct
		(g_object_new (CONG_EDITOR_AREA_LISTITEM_TYPE, NULL),
		 editor_widget,
		 label);
}

/* Method implementation definitions: */
static void 
calc_requisition (CongEditorArea *area, 
		  int width_hint,
		  GtkRequisition *output)
{
	const GtkRequisition *child_req;

	CongEditorAreaListitem *listitem = CONG_EDITOR_AREA_LISTITEM(area);

	if (PRIVATE(listitem)->outer_hcompose) {

		child_req = cong_editor_area_get_requisition (PRIVATE(listitem)->outer_hcompose,
							      width_hint);
		g_assert(child_req);
		
		output->width = child_req->width;
		output->height = child_req->height;
	}
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaListitem *listitem = CONG_EDITOR_AREA_LISTITEM(area);

	if (PRIVATE(listitem)->outer_hcompose) {
		const GdkRectangle *rect = cong_editor_area_get_window_coords(area);

		cong_editor_area_set_allocation (PRIVATE(listitem)->outer_hcompose,
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
	CongEditorAreaListitem *listitem = CONG_EDITOR_AREA_LISTITEM(editor_area);

	if (PRIVATE(listitem)->outer_hcompose) {
		if ((*func)(PRIVATE(listitem)->outer_hcompose, user_data)) {
			return PRIVATE(listitem)->outer_hcompose;
		}
	}

	return NULL;
}

static void
add_child (CongEditorAreaContainer *area_container,
	   CongEditorArea *child)
{
	CongEditorAreaListitem *listitem = CONG_EDITOR_AREA_LISTITEM(area_container);

	g_assert(PRIVATE(listitem)->inner_area);

	cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER( PRIVATE(listitem)->inner_area),
					       child);
}
