/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-child-policy-flow-holder.c
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
#include "cong-editor-child-policy-flow-holder.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"
#include "cong-marshal.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorChildPolicyFlowHolderDetails
{
	CongEditorAreaFlowHolder *area_flow_holder;
};


static CongEditorChildPolicy* 
insert_areas_for_node (CongEditorChildPolicy *child_policy,
		       CongEditorNode *editor_node);

static void 
remove_areas_for_node (CongEditorChildPolicy *child_policy,
		       CongEditorNode *editor_node);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorChildPolicyFlowHolder, 
			cong_editor_child_policy_flow_holder,
			CongEditorChildPolicy,
			CONG_EDITOR_CHILD_POLICY_TYPE );

static void
cong_editor_child_policy_flow_holder_class_init (CongEditorChildPolicyFlowHolderClass *klass)
{
	CongEditorChildPolicyClass *child_policy_klass = CONG_EDITOR_CHILD_POLICY_CLASS(klass);

	child_policy_klass->insert_areas_for_node = insert_areas_for_node;
	child_policy_klass->remove_areas_for_node = remove_areas_for_node;
}

static void
cong_editor_child_policy_flow_holder_instance_init (CongEditorChildPolicyFlowHolder *child_policy_flow_holder)
{
	child_policy_flow_holder->private = g_new0(CongEditorChildPolicyFlowHolderDetails,1);
}

/**
 * cong_editor_child_policy_flow_holder_construct:
 * @child_policy_flow_holder:
 * @editor_node:
 * @flow_holder:
 *
 * TODO: Write me
 */
CongEditorChildPolicy*
cong_editor_child_policy_flow_holder_construct (CongEditorChildPolicyFlowHolder *child_policy_flow_holder,
						CongEditorNode *editor_node,
						CongEditorAreaFlowHolder *flow_holder)
{
	cong_editor_child_policy_construct (CONG_EDITOR_CHILD_POLICY(child_policy_flow_holder),
					    editor_node);
	
	PRIVATE(child_policy_flow_holder)->area_flow_holder = flow_holder;

	return CONG_EDITOR_CHILD_POLICY(child_policy_flow_holder);
}

/**
 * cong_editor_child_policy_flow_holder_new:
 * @editor_node:
 * @flow_holder:
 *
 * TODO: Write me
 */
CongEditorChildPolicy*
cong_editor_child_policy_flow_holder_new (CongEditorNode *editor_node,
					  CongEditorAreaFlowHolder *flow_holder)
{
#if DEBUG_EDITOR_CHILD_POLICY_LIFETIMES
	g_message("cong_editor_child_policy_flow_holder_new(%s)", text);
#endif

#if 0
	g_return_val_if_fail (editor_node, NULL);
#endif
	g_return_val_if_fail (flow_holder, NULL);

	return cong_editor_child_policy_flow_holder_construct
		(g_object_new (CONG_EDITOR_CHILD_POLICY_FLOW_HOLDER_TYPE, NULL),
		 editor_node,
		 flow_holder);
}

static CongEditorChildPolicy* 
insert_areas_for_node (CongEditorChildPolicy *child_policy,
		       CongEditorNode *editor_node)
{
	CongEditorChildPolicyFlowHolder *child_policy_flow_holder = CONG_EDITOR_CHILD_POLICY_FLOW_HOLDER(child_policy);

	return cong_editor_area_flow_holder_insert_areas_for_node (PRIVATE(child_policy_flow_holder)->area_flow_holder,
								   editor_node);	

#if 0
	g_hash_table_insert (PRIVATE(widget)->hash_of_editor_node_to_primary_area,
			     editor_node,
			     this_area);
#endif

#if 0
	/* If this node can ever have children, we need to add a container for them:
	   FIXME:  slightly hackish test */
	if (IS_CONG_EDITOR_AREA_CONTAINER(this_area) ) {
		CongEditorAreaFlowHolder *flow_holder;

		flow_holder = cong_editor_area_flow_holder_manufacture (widget,
									flow_type);

		cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER (this_area),
						      CONG_EDITOR_AREA(flow_holder));

		g_hash_table_insert (PRIVATE(widget)->hash_of_editor_node_to_child_flow_holder,
				     editor_node,
				     flow_holder);
	}
#endif
}

static void 
remove_areas_for_node (CongEditorChildPolicy *child_policy,
		       CongEditorNode *editor_node)
{
	CongEditorChildPolicyFlowHolder *child_policy_flow_holder = CONG_EDITOR_CHILD_POLICY_FLOW_HOLDER(child_policy);

	cong_editor_area_flow_holder_remove_areas_for_node (PRIVATE(child_policy_flow_holder)->area_flow_holder,
							    editor_node);	
}
