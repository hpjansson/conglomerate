/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-child-policy.h
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

#ifndef __CONG_EDITOR_CHILD_POLICY_H__
#define __CONG_EDITOR_CHILD_POLICY_H__

#include "cong-editor-widget.h"

G_BEGIN_DECLS

#define DEBUG_EDITOR_CHILD_POLICY_LIFETIMES 0

typedef struct CongEditorChildPolicyDetails CongEditorChildPolicyDetails;

#define CONG_EDITOR_CHILD_POLICY_TYPE	      (cong_editor_child_policy_get_type ())
#define CONG_EDITOR_CHILD_POLICY(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_CHILD_POLICY_TYPE, CongEditorChildPolicy)
#define CONG_EDITOR_CHILD_POLICY_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_CHILD_POLICY_TYPE, CongEditorChildPolicyClass)
#define IS_CONG_EDITOR_CHILD_POLICY(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_CHILD_POLICY_TYPE)

struct CongEditorChildPolicy
{
	GObject object;

	CongEditorChildPolicyDetails *private;
};

struct CongEditorChildPolicyClass
{
	GObjectClass klass;

	CongEditorChildPolicy* (*insert_areas_for_node) (CongEditorChildPolicy *child_policy,
							 CongEditorNode *node);

	void (*remove_areas_for_node) (CongEditorChildPolicy *child_policy,
				       CongEditorNode *node);

};

GType
cong_editor_child_policy_get_type (void);

CongEditorChildPolicy*
cong_editor_child_policy_construct (CongEditorChildPolicy *child_policy,
				    CongEditorNode *editor_node);

CongEditorNode*
cong_editor_child_policy_get_node (CongEditorChildPolicy *child_policy);

CongEditorWidget3*
cong_editor_child_policy_get_widget (CongEditorChildPolicy *child_policy);

CongEditorChildPolicy* 
cong_editor_child_policy_insert_areas_for_node (CongEditorChildPolicy *child_policy,
						CongEditorNode *node);

void 
cong_editor_child_policy_remove_areas_for_node (CongEditorChildPolicy *child_policy,
						CongEditorNode *node);

G_END_DECLS

#endif
