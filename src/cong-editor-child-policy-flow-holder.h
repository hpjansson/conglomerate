/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-child-policy-flow-holder.h
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

#ifndef __CONG_EDITOR_CHILD_POLICY_FLOW_HOLDER_H__
#define __CONG_EDITOR_CHILD_POLICY_FLOW_HOLDER_H__

#include "cong-editor-child-policy.h"
#include "cong-editor-area-flow-holder.h"

G_BEGIN_DECLS

typedef struct CongEditorChildPolicyFlowHolder CongEditorChildPolicyFlowHolder;
typedef struct CongEditorChildPolicyFlowHolderClass CongEditorChildPolicyFlowHolderClass;
typedef struct CongEditorChildPolicyFlowHolderDetails CongEditorChildPolicyFlowHolderDetails;

#define CONG_EDITOR_CHILD_POLICY_FLOW_HOLDER_TYPE	      (cong_editor_child_policy_flow_holder_get_type ())
#define CONG_EDITOR_CHILD_POLICY_FLOW_HOLDER(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_CHILD_POLICY_FLOW_HOLDER_TYPE, CongEditorChildPolicyFlowHolder)
#define CONG_EDITOR_CHILD_POLICY_FLOW_HOLDER_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_CHILD_POLICY_FLOW_HOLDER_TYPE, CongEditorChildPolicyFlowHolderClass)
#define IS_CONG_EDITOR_CHILD_POLICY_FLOW_HOLDER(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_CHILD_POLICY_FLOW_HOLDER_TYPE)

struct CongEditorChildPolicyFlowHolder
{
	CongEditorChildPolicy child_policy;

	CongEditorChildPolicyFlowHolderDetails *private;
};

struct CongEditorChildPolicyFlowHolderClass
{
	CongEditorChildPolicyClass klass;
};

GType
cong_editor_child_policy_flow_holder_get_type (void);

CongEditorChildPolicy*
cong_editor_child_policy_flow_holder_construct (CongEditorChildPolicyFlowHolder *child_policy_flow_holder,
						CongEditorNode *editor_node,
						CongEditorAreaFlowHolder *flow_holder);

CongEditorChildPolicy*
cong_editor_child_policy_flow_holder_new (CongEditorNode *editor_node,
					  CongEditorAreaFlowHolder *flow_holder);

G_END_DECLS

#endif
