/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-child-policy-inline.h
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

#ifndef __CONG_EDITOR_CHILD_POLICY_INLINE_H__
#define __CONG_EDITOR_CHILD_POLICY_INLINE_H__

#include "cong-editor-child-policy.h"
#include "cong-editor-area-flow-holder-inlines.h"

G_BEGIN_DECLS

typedef struct _CongEditorChildPolicyInline CongEditorChildPolicyInline;
typedef struct _CongEditorChildPolicyInlineClass CongEditorChildPolicyInlineClass;
typedef struct _CongEditorChildPolicyInlineDetails CongEditorChildPolicyInlineDetails;

#define CONG_EDITOR_CHILD_POLICY_INLINE_TYPE	      (cong_editor_child_policy_inline_get_type ())
#define CONG_EDITOR_CHILD_POLICY_INLINE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_CHILD_POLICY_INLINE_TYPE, CongEditorChildPolicyInline)
#define CONG_EDITOR_CHILD_POLICY_INLINE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_CHILD_POLICY_INLINE_TYPE, CongEditorChildPolicyInlineClass)
#define IS_CONG_EDITOR_CHILD_POLICY_INLINE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_CHILD_POLICY_INLINE_TYPE)

struct _CongEditorChildPolicyInline
{
	CongEditorChildPolicy child_policy;

	CongEditorChildPolicyInlineDetails *private;
};

struct _CongEditorChildPolicyInlineClass
{
	CongEditorChildPolicyClass klass;
};

GType
cong_editor_child_policy_inline_get_type (void);

CongEditorChildPolicy*
cong_editor_child_policy_inline_construct (CongEditorChildPolicyInline *child_policy_inline,
					   CongEditorNode *editor_node,
					   CongEditorAreaFlowHolderInlines *inlines);

/**
 * A child policy represnting a node within an inline flow e.g. a span tag.  Responds to child changes by notifying the flow that it needs to regenerate the lines.
 */
CongEditorChildPolicy*
cong_editor_child_policy_inline_new (CongEditorNode *editor_node,
				     CongEditorAreaFlowHolderInlines *inlines);

G_END_DECLS

#endif
