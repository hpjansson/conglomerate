/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-flow-holder.c
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
#include "cong-editor-area-flow-holder.h"
#include <libgnome/gnome-macros.h>

#include "cong-editor-area-composer.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaFlowHolderDetails
{
	CongEditorAreaComposer *outer_vcompose;

	GHashTable *hash_of_doc_node_to_area;
};


#if 0
typedef struct CongEditorAreaFlowHolderChildDetails CongEditorAreaFlowHolderChildDetails;

struct CongEditorAreaFlowHolderChildDetails
{
	CongEditorNode *child_node;
	CongEditorArea *child_area;
};
#endif

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


/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaFlowHolder, 
			cong_editor_area_flow_holder,
			CongEditorArea,
			CONG_EDITOR_AREA_TYPE );

static void
cong_editor_area_flow_holder_class_init (CongEditorAreaFlowHolderClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);

	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
	area_klass->for_all = for_all;
}

static void
cong_editor_area_flow_holder_instance_init (CongEditorAreaFlowHolder *area_flow_holder)
{
	area_flow_holder->private = g_new0(CongEditorAreaFlowHolderDetails,1);
}

/* Exported function definitions: */
CongEditorArea*
cong_editor_area_flow_holder_construct (CongEditorAreaFlowHolder *area_flow_holder,
					CongEditorWidget3 *editor_widget)
{
	cong_editor_area_construct (CONG_EDITOR_AREA(area_flow_holder),
				    editor_widget);

	PRIVATE(area_flow_holder)->outer_vcompose = CONG_EDITOR_AREA_COMPOSER( cong_editor_area_composer_new (editor_widget,
													      GTK_ORIENTATION_VERTICAL,
													      0));

	PRIVATE(area_flow_holder)->hash_of_doc_node_to_area = g_hash_table_new (NULL,
										NULL); 


	cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (area_flow_holder),
								   CONG_EDITOR_AREA (PRIVATE(area_flow_holder)->outer_vcompose));

	cong_editor_area_protected_set_parent (CONG_EDITOR_AREA (PRIVATE(area_flow_holder)->outer_vcompose),
					       CONG_EDITOR_AREA (area_flow_holder));

	return CONG_EDITOR_AREA (area_flow_holder);
}

CongEditorArea*
cong_editor_area_flow_holder_new (CongEditorWidget3 *editor_widget)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_flow_holder_new");
#endif

	return cong_editor_area_flow_holder_construct
		(g_object_new (CONG_EDITOR_AREA_FLOW_HOLDER_TYPE, NULL),
		 editor_widget);
}

CongEditorArea*
cong_editor_area_flow_holder_insert_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
						    CongEditorNode *editor_node)
{
	CongEditorArea *new_area;
	CongNodePtr doc_node;
	CongEditorArea *prev_node;

	/* Get the editor node to generate its area: */
	new_area = cong_editor_node_generate_area (editor_node);

	doc_node = cong_editor_node_get_node(editor_node);

	g_hash_table_insert (PRIVATE(area_flow_holder)->hash_of_doc_node_to_area,
			     doc_node,
			     new_area);

	if (doc_node->prev) {
		CongEditorArea *prev_area = g_hash_table_lookup (PRIVATE(area_flow_holder)->hash_of_doc_node_to_area,
								 doc_node->prev);
		g_assert(prev_area);
			
		cong_editor_area_composer_pack_after (PRIVATE(area_flow_holder)->outer_vcompose,
						      new_area,
						      prev_area,
						      FALSE,
						      FALSE,
						      0);
	} else {
		/* Insert this area after any already present in the parent's insertion area: */
		cong_editor_area_composer_pack (PRIVATE(area_flow_holder)->outer_vcompose,
						new_area,
						FALSE,
						FALSE,
						0);
	}

	return new_area;
}

void
cong_editor_area_flow_holder_remove_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
						    CongEditorNode *editor_node)
{
	CongNodePtr doc_node;
	CongEditorArea *area;
	
	doc_node = cong_editor_node_get_node(editor_node);
	area = g_hash_table_lookup  (PRIVATE(area_flow_holder)->hash_of_doc_node_to_area,
				     doc_node);
	g_assert (area);

	cong_editor_area_container_remove_child (CONG_EDITOR_AREA_CONTAINER(PRIVATE(area_flow_holder)->outer_vcompose),
						 area);

	g_hash_table_remove (PRIVATE(area_flow_holder)->hash_of_doc_node_to_area,
			     doc_node);
}



/* Method implementation definitions: */
static void 
calc_requisition (CongEditorArea *area, 
		  int width_hint,
		  GtkRequisition *output)
{
	const GtkRequisition *child_req;

	CongEditorAreaFlowHolder *area_flow_holder = CONG_EDITOR_AREA_FLOW_HOLDER(area);

	if (PRIVATE(area_flow_holder)->outer_vcompose) {

		child_req = cong_editor_area_get_requisition (CONG_EDITOR_AREA(PRIVATE(area_flow_holder)->outer_vcompose),
							      width_hint);
		g_assert(child_req);
		
		output->width = child_req->width;
		output->height = child_req->height;
	}
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaFlowHolder *area_flow_holder = CONG_EDITOR_AREA_FLOW_HOLDER(area);

	if (PRIVATE(area_flow_holder)->outer_vcompose) {
		const GdkRectangle *rect = cong_editor_area_get_window_coords(area);

		cong_editor_area_set_allocation (CONG_EDITOR_AREA(PRIVATE(area_flow_holder)->outer_vcompose),
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
	CongEditorAreaFlowHolder *area_flow_holder = CONG_EDITOR_AREA_FLOW_HOLDER(editor_area);

	if (PRIVATE(area_flow_holder)->outer_vcompose) {
		if ((*func)(CONG_EDITOR_AREA(PRIVATE(area_flow_holder)->outer_vcompose), user_data)) {
			return CONG_EDITOR_AREA(PRIVATE(area_flow_holder)->outer_vcompose);
		}
	}

	return NULL;
}
