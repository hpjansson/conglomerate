/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node.h
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

#ifndef __CONG_EDITOR_NODE_H__
#define __CONG_EDITOR_NODE_H__

#include "cong-editor-area.h"

G_BEGIN_DECLS

#define CONG_EDITOR_NODE_TYPE	      (cong_editor_node_get_type ())
#define CONG_EDITOR_NODE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_NODE_TYPE, CongEditorNode)
#define CONG_EDITOR_NODE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_NODE_TYPE, CongEditorNodeClass)
#define IS_CONG_EDITOR_NODE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_NODE_TYPE)

typedef struct CongEditorNode CongEditorNode;
typedef struct CongEditorNodeClass CongEditorNodeClass;
typedef struct CongEditorNodeDetails CongEditorNodeDetails;

struct CongEditorNode
{
	GObject object;

	CongEditorNodeDetails *private;
};

struct CongEditorNodeClass
{
	GObjectClass klass;

	/* Methods? */

	/* Simplistic interface for now: */
	CongEditorArea* (*generate_area) (CongEditorNode *editor_node);
};

GType
cong_editor_node_get_type (void);

CongEditorNode*
cong_editor_node_construct (CongEditorNode *editor_node,
			    CongEditorWidget3* widget,
			    CongNodePtr node);

CongEditorWidget3*
cong_editor_node_get_widget (CongEditorNode *editor_node);

CongDocument*
cong_editor_node_get_document (CongEditorNode *editor_node);

CongNodePtr
cong_editor_node_get_node (CongEditorNode *editor_node);

/* 
   Simplistic node->area interface (1-1 for now).
*/
#if 0
CongEditorArea*
cong_editor_node_get_primary_area (CongEditorNode *editor_node);

CongEditorArea*
cong_editor_node_get_inner_area (CongEditorNode *editor_node);
#endif

CongEditorArea*
cong_editor_node_generate_area (CongEditorNode *editor_node);


G_END_DECLS

#endif
