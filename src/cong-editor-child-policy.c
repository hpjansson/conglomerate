/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-child-policy.c
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
#include "cong-editor-child-policy.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"
#include "cong-marshal.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorChildPolicyDetails
{
	CongEditorNode *editor_node;
};

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorChildPolicy, 
			cong_editor_child_policy,
			GObject,
			G_TYPE_OBJECT );

CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_child_policy, insert_areas_for_node);
CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_child_policy, remove_areas_for_node);

static void
cong_editor_child_policy_class_init (CongEditorChildPolicyClass *klass)
{
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_child_policy,
					      insert_areas_for_node);
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_child_policy,
					      remove_areas_for_node);
}

static void
cong_editor_child_policy_instance_init (CongEditorChildPolicy *child_policy)
{
	child_policy->private = g_new0(CongEditorChildPolicyDetails,1);
}

CongEditorChildPolicy*
cong_editor_child_policy_construct (CongEditorChildPolicy *child_policy,
				    CongEditorNode* editor_node)
{
	PRIVATE(child_policy)->editor_node = editor_node;

	return child_policy;
}


/**
 * cong_editor_child_policy_get_node:
 * @child_policy:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_child_policy_get_node (CongEditorChildPolicy *child_policy)
{
	g_return_val_if_fail (child_policy, NULL);

	return PRIVATE(child_policy)->editor_node;
}

#if 0
CongEditorWidget3*
cong_editor_child_policy_get_widget (CongEditorChildPolicy *child_policy)
{
	g_return_val_if_fail (child_policy, NULL);

	return PRIVATE(child_policy)->editor_widget;
}
#endif

/**
 * cong_editor_child_policy_insert_areas_for_node:
 * @child_policy:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorChildPolicy* 
cong_editor_child_policy_insert_areas_for_node (CongEditorChildPolicy *child_policy,
						CongEditorNode *node)
{
	g_return_val_if_fail (child_policy, NULL);
	g_return_val_if_fail (node, NULL);

	return CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_CHILD_POLICY_CLASS,
						       child_policy,
						       insert_areas_for_node, 
						       (child_policy, node));
}

/**
 * cong_editor_child_policy_remove_areas_for_node:
 * @child_policy:
 * @node:
 *
 * TODO: Write me
 */
void 
cong_editor_child_policy_remove_areas_for_node (CongEditorChildPolicy *child_policy,
						CongEditorNode *node)
{
	g_return_if_fail (child_policy);
	g_return_if_fail (node);

	CONG_EEL_CALL_METHOD (CONG_EDITOR_CHILD_POLICY_CLASS,
			      child_policy,
			      remove_areas_for_node, 
			      (child_policy, node));
}
