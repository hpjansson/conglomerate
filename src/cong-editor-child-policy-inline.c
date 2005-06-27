/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-child-policy-inline.c
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
#include "cong-editor-child-policy-inline.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"
#include "cong-marshal.h"

#define PRIVATE(x) ((x)->private)

struct _CongEditorChildPolicyInlineDetails
{
	CongEditorAreaFlowHolderInlines *area_inlines;
};


static CongEditorChildPolicy* 
insert_areas_for_node (CongEditorChildPolicy *child_policy,
		       CongEditorNode *editor_node);

static void 
remove_areas_for_node (CongEditorChildPolicy *child_policy,
		       CongEditorNode *editor_node);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorChildPolicyInline, 
			cong_editor_child_policy_inline,
			CongEditorChildPolicy,
			CONG_EDITOR_CHILD_POLICY_TYPE );

static void
cong_editor_child_policy_inline_class_init (CongEditorChildPolicyInlineClass *klass)
{
	CongEditorChildPolicyClass *child_policy_klass = CONG_EDITOR_CHILD_POLICY_CLASS(klass);

	child_policy_klass->insert_areas_for_node = insert_areas_for_node;
	child_policy_klass->remove_areas_for_node = remove_areas_for_node;
}

static void
cong_editor_child_policy_inline_instance_init (CongEditorChildPolicyInline *child_policy_inline)
{
	child_policy_inline->private = g_new0(CongEditorChildPolicyInlineDetails,1);
}

/**
 * cong_editor_child_policy_inline_construct:
 * @child_policy_inline:
 * @editor_node:
 * @inlines:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorChildPolicy*
cong_editor_child_policy_inline_construct (CongEditorChildPolicyInline *child_policy_inline,
					   CongEditorNode *editor_node,
					   CongEditorAreaFlowHolderInlines *inlines)
{
	cong_editor_child_policy_construct (CONG_EDITOR_CHILD_POLICY(child_policy_inline),
					    editor_node);
	
	PRIVATE(child_policy_inline)->area_inlines = inlines;

	return CONG_EDITOR_CHILD_POLICY(child_policy_inline);
}

/**
 * cong_editor_child_policy_inline_new:
 * @editor_node:
 * @inlines:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorChildPolicy*
cong_editor_child_policy_inline_new (CongEditorNode *editor_node,
				     CongEditorAreaFlowHolderInlines *inlines)
{
#if DEBUG_EDITOR_CHILD_POLICY_LIFETIMES
	g_message("cong_editor_child_policy_inline_new(%s)", text);
#endif

#if 0
	g_return_val_if_fail (editor_node, NULL);
#endif
	g_return_val_if_fail (inlines, NULL);

	return cong_editor_child_policy_inline_construct
		(g_object_new (CONG_EDITOR_CHILD_POLICY_INLINE_TYPE, NULL),
		 editor_node,
		 inlines);
}


/* Whenever children are added or removed, we need to regenerate the lines of the flow_holder_inline */
/**
 * insert_areas_for_node:
 * @child_policy:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
static CongEditorChildPolicy* 
insert_areas_for_node (CongEditorChildPolicy *child_policy,
		       CongEditorNode *editor_node)
{
	CongEditorChildPolicyInline *child_policy_inline = CONG_EDITOR_CHILD_POLICY_INLINE(child_policy);
	
	cong_editor_area_flow_holder_inlines_reflow_required (PRIVATE(child_policy_inline)->area_inlines);

	/* Return a clone: */
	return cong_editor_child_policy_inline_new ( editor_node,
						     PRIVATE(child_policy_inline)->area_inlines);
}

/**
 * remove_areas_for_node:
 * @child_policy:
 * @editor_node:
 *
 * TODO: Write me
 */
static void 
remove_areas_for_node (CongEditorChildPolicy *child_policy,
		       CongEditorNode *editor_node)
{
	CongEditorChildPolicyInline *child_policy_inline = CONG_EDITOR_CHILD_POLICY_INLINE(child_policy);
	
	cong_editor_area_flow_holder_inlines_reflow_required (PRIVATE(child_policy_inline)->area_inlines);
}
