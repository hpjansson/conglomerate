/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-flow-holder-single.c
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
#include "cong-editor-area-flow-holder-single.h"
#include <libgnome/gnome-macros.h>

#include "cong-editor-area-bin.h"
#include "cong-editor-child-policy-flow-holder.h"

CongEditorChildPolicy*
cong_editor_widget_create_child_policy_for_node_with_single_area(CongEditorNode *editor_node,
								 CongEditorArea *editor_area);


#define PRIVATE(x) ((x)->private)

struct CongEditorAreaFlowHolderSingleDetails
{
	CongEditorArea *outer_bin;
	CongEditorNode *single_editor_node;
	CongEditorArea *single_editor_area;
};

/* Method implementation prototypes: */
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

static CongEditorChildPolicy*
insert_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
		       CongEditorNode *editor_node);
static void
remove_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
		       CongEditorNode *editor_node);


/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaFlowHolderSingle, 
			cong_editor_area_flow_holder_single,
			CongEditorAreaFlowHolder,
			CONG_EDITOR_AREA_FLOW_HOLDER_TYPE );

static void
cong_editor_area_flow_holder_single_class_init (CongEditorAreaFlowHolderSingleClass *klass)
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
cong_editor_area_flow_holder_single_instance_init (CongEditorAreaFlowHolderSingle *area_flow_holder_single)
{
	area_flow_holder_single->private = g_new0(CongEditorAreaFlowHolderSingleDetails,1);
}

/* Exported function definitions: */
CongEditorArea*
cong_editor_area_flow_holder_single_construct (CongEditorAreaFlowHolderSingle *area_flow_holder_single,
					CongEditorWidget3 *editor_widget)
{
	cong_editor_area_flow_holder_construct (CONG_EDITOR_AREA_FLOW_HOLDER(area_flow_holder_single),
						editor_widget);

	PRIVATE(area_flow_holder_single)->outer_bin = cong_editor_area_bin_new (editor_widget);

	cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (area_flow_holder_single),
								   CONG_EDITOR_AREA (PRIVATE(area_flow_holder_single)->outer_bin));

	cong_editor_area_protected_set_parent (CONG_EDITOR_AREA (PRIVATE(area_flow_holder_single)->outer_bin),
					       CONG_EDITOR_AREA (area_flow_holder_single));

	return CONG_EDITOR_AREA (area_flow_holder_single);
}

CongEditorArea*
cong_editor_area_flow_holder_single_new (CongEditorWidget3 *editor_widget)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_flow_holder_single_new");
#endif

	return cong_editor_area_flow_holder_single_construct
		(g_object_new (CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE_TYPE, NULL),
		 editor_widget);
}

/* Method implementation definitions: */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	CongEditorAreaFlowHolderSingle *area_flow_holder_single = CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE(area);

	if (PRIVATE(area_flow_holder_single)->outer_bin) {

		return  cong_editor_area_get_requisition (CONG_EDITOR_AREA(PRIVATE(area_flow_holder_single)->outer_bin),
							  orientation,
							  width_hint);
	} else {
		return 0;
	}
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaFlowHolderSingle *area_flow_holder_single = CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE(area);

	if (PRIVATE(area_flow_holder_single)->outer_bin) {
		const GdkRectangle *rect = cong_editor_area_get_window_coords(area);

		cong_editor_area_set_allocation (CONG_EDITOR_AREA(PRIVATE(area_flow_holder_single)->outer_bin),
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
	CongEditorAreaFlowHolderSingle *area_flow_holder_single = CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE(editor_area);

	if (PRIVATE(area_flow_holder_single)->outer_bin) {
		if ((*func)(CONG_EDITOR_AREA(PRIVATE(area_flow_holder_single)->outer_bin), user_data)) {
			return CONG_EDITOR_AREA(PRIVATE(area_flow_holder_single)->outer_bin);
		}
	}

	return NULL;
}

static CongEditorChildPolicy*
insert_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
		       CongEditorNode *editor_node)
{
	CongEditorAreaFlowHolderSingle *area_flow_holder_single = CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE(area_flow_holder);
	CongEditorArea *new_area;

	g_assert(PRIVATE(area_flow_holder_single)->single_editor_node==NULL);
	g_assert(PRIVATE(area_flow_holder_single)->single_editor_area==NULL);

	/* Get the editor node to generate its area: */
	new_area = cong_editor_node_generate_block_area (editor_node);

	cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER(PRIVATE(area_flow_holder_single)->outer_bin),
					      new_area);

	PRIVATE(area_flow_holder_single)->single_editor_node = editor_node;
	PRIVATE(area_flow_holder_single)->single_editor_area = new_area;

	return cong_editor_widget_create_child_policy_for_node_with_single_area(editor_node,
										new_area);
}

static void
remove_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
		       CongEditorNode *editor_node)
{
	CongEditorAreaFlowHolderSingle *area_flow_holder_single = CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE(area_flow_holder);
	CongEditorArea *area;

	g_assert(PRIVATE(area_flow_holder_single)->single_editor_node==editor_node);
	g_assert(PRIVATE(area_flow_holder_single)->single_editor_area);

	cong_editor_area_container_remove_child (CONG_EDITOR_AREA_CONTAINER(PRIVATE(area_flow_holder_single)->outer_bin),
						 PRIVATE(area_flow_holder_single)->single_editor_area);

	PRIVATE(area_flow_holder_single)->single_editor_node = NULL;
	PRIVATE(area_flow_holder_single)->single_editor_area = NULL;
}

CongEditorChildPolicy*
cong_editor_widget_create_child_policy_for_node_with_single_area(CongEditorNode *editor_node,
								 CongEditorArea *editor_area)
{
	if (IS_CONG_EDITOR_AREA_CONTAINER(editor_area) ) {
		CongEditorAreaFlowHolder *flow_holder;
		enum CongFlowType flow_type = cong_editor_node_get_flow_type (editor_node);
		
		flow_holder = cong_editor_area_flow_holder_manufacture (cong_editor_node_get_widget(editor_node),
									flow_type);
		
		cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER (editor_area),
						      CONG_EDITOR_AREA(flow_holder));
		
		return cong_editor_child_policy_flow_holder_new (editor_node,
								 flow_holder);
	} else {
		return NULL; /* for now */
#if 0
		return cong_editor_child_policy_none_new (editor_node);
#endif
	}
	
}
