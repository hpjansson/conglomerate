/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-flow-holder-inlines.c
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
#include "cong-editor-area-flow-holder-inlines.h"
#include <libgnome/gnome-macros.h>

#include "cong-editor-area-composer.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaFlowHolderInlinesDetails
{
	CongEditorAreaComposer *outer_compose;

	GHashTable *hash_of_doc_node_to_area;
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

static CongEditorArea*
insert_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
		       CongEditorNode *editor_node);
static void
remove_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
		       CongEditorNode *editor_node);


/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaFlowHolderInlines, 
			cong_editor_area_flow_holder_inlines,
			CongEditorAreaFlowHolder,
			CONG_EDITOR_AREA_FLOW_HOLDER_TYPE );

static void
cong_editor_area_flow_holder_inlines_class_init (CongEditorAreaFlowHolderInlinesClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);
	CongEditorAreaFlowHolderClass *flow_holder_klass = CONG_EDITOR_AREA_FLOW_HOLDER_CLASS(klass);

	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
	area_klass->for_all = for_all;

	flow_holder_klass->insert_areas_for_node = insert_areas_for_node;
	flow_holder_klass->remove_areas_for_node = remove_areas_for_node;
}

static void
cong_editor_area_flow_holder_inlines_instance_init (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines)
{
	area_flow_holder_inlines->private = g_new0(CongEditorAreaFlowHolderInlinesDetails,1);
}

/* Exported function definitions: */
CongEditorArea*
cong_editor_area_flow_holder_inlines_construct (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines,
					CongEditorWidget3 *editor_widget)
{
	cong_editor_area_flow_holder_construct (CONG_EDITOR_AREA_FLOW_HOLDER(area_flow_holder_inlines),
						editor_widget);

	PRIVATE(area_flow_holder_inlines)->outer_compose = CONG_EDITOR_AREA_COMPOSER( cong_editor_area_composer_new (editor_widget,
													      GTK_ORIENTATION_HORIZONTAL,
													      0));

	PRIVATE(area_flow_holder_inlines)->hash_of_doc_node_to_area = g_hash_table_new (NULL,
										NULL); 


	cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (area_flow_holder_inlines),
								   CONG_EDITOR_AREA (PRIVATE(area_flow_holder_inlines)->outer_compose));

	cong_editor_area_protected_set_parent (CONG_EDITOR_AREA (PRIVATE(area_flow_holder_inlines)->outer_compose),
					       CONG_EDITOR_AREA (area_flow_holder_inlines));

	return CONG_EDITOR_AREA (area_flow_holder_inlines);
}

CongEditorArea*
cong_editor_area_flow_holder_inlines_new (CongEditorWidget3 *editor_widget)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_flow_holder_inlines_new");
#endif

	return cong_editor_area_flow_holder_inlines_construct
		(g_object_new (CONG_EDITOR_AREA_FLOW_HOLDER_INLINES_TYPE, NULL),
		 editor_widget);
}

/* Method implementation definitions: */
static void 
calc_requisition (CongEditorArea *area, 
		  int width_hint,
		  GtkRequisition *output)
{
	const GtkRequisition *child_req;

	CongEditorAreaFlowHolderInlines *area_flow_holder_inlines = CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(area);

	if (PRIVATE(area_flow_holder_inlines)->outer_compose) {

		child_req = cong_editor_area_get_requisition (CONG_EDITOR_AREA(PRIVATE(area_flow_holder_inlines)->outer_compose),
							      width_hint);
		g_assert(child_req);
		
		output->width = child_req->width;
		output->height = child_req->height;
	}
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaFlowHolderInlines *area_flow_holder_inlines = CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(area);

	if (PRIVATE(area_flow_holder_inlines)->outer_compose) {
		const GdkRectangle *rect = cong_editor_area_get_window_coords(area);

		cong_editor_area_set_allocation (CONG_EDITOR_AREA(PRIVATE(area_flow_holder_inlines)->outer_compose),
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
	CongEditorAreaFlowHolderInlines *area_flow_holder_inlines = CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(editor_area);

	if (PRIVATE(area_flow_holder_inlines)->outer_compose) {
		if ((*func)(CONG_EDITOR_AREA(PRIVATE(area_flow_holder_inlines)->outer_compose), user_data)) {
			return CONG_EDITOR_AREA(PRIVATE(area_flow_holder_inlines)->outer_compose);
		}
	}

	return NULL;
}

static CongEditorArea*
insert_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
		       CongEditorNode *editor_node)
{
	CongEditorAreaFlowHolderInlines *area_flow_holder_inlines = CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(area_flow_holder);
	CongEditorArea *new_area;
	CongNodePtr doc_node;
	CongEditorArea *prev_node;

	/* Get the editor node to generate its area: */
	new_area = cong_editor_node_generate_area (editor_node);

	doc_node = cong_editor_node_get_node(editor_node);

	g_hash_table_insert (PRIVATE(area_flow_holder_inlines)->hash_of_doc_node_to_area,
			     doc_node,
			     new_area);

	if (doc_node->prev) {
		CongEditorArea *prev_area = g_hash_table_lookup (PRIVATE(area_flow_holder_inlines)->hash_of_doc_node_to_area,
								 doc_node->prev);
		if (prev_area) {
			
			cong_editor_area_composer_pack_after (PRIVATE(area_flow_holder_inlines)->outer_compose,
							      new_area,
							      prev_area,
							      FALSE,
							      FALSE,
							      0);

			return new_area;
		}
	}

	/* Insert this area after any already present in the parent's insertion area: */
	cong_editor_area_composer_pack (PRIVATE(area_flow_holder_inlines)->outer_compose,
					new_area,
					FALSE,
					FALSE,
					0);
	return new_area;
}

static void
remove_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
		       CongEditorNode *editor_node)
{
	CongEditorAreaFlowHolderInlines *area_flow_holder_inlines = CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(area_flow_holder);
	CongNodePtr doc_node;
	CongEditorArea *area;
	
	doc_node = cong_editor_node_get_node(editor_node);
	area = g_hash_table_lookup  (PRIVATE(area_flow_holder_inlines)->hash_of_doc_node_to_area,
				     doc_node);
	g_assert (area);

	cong_editor_area_container_remove_child (CONG_EDITOR_AREA_CONTAINER(PRIVATE(area_flow_holder_inlines)->outer_compose),
						 area);

	g_hash_table_remove (PRIVATE(area_flow_holder_inlines)->hash_of_doc_node_to_area,
			     doc_node);
}
