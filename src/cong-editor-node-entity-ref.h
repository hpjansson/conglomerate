/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node-entity-ref.h
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

#ifndef __CONG_EDITOR_NODE_ENTITY_REF_H__
#define __CONG_EDITOR_NODE_ENTITY_REF_H__

#include "cong-editor-node.h"

G_BEGIN_DECLS

#define CONG_EDITOR_NODE_ENTITY_REF_TYPE	      (cong_editor_node_entity_ref_get_type ())
#define CONG_EDITOR_NODE_ENTITY_REF(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_NODE_ENTITY_REF_TYPE, CongEditorNodeEntityRef)
#define CONG_EDITOR_NODE_ENTITY_REF_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_NODE_ENTITY_REF_TYPE, CongEditorNodeEntityRefClass)
#define IS_CONG_EDITOR_NODE_ENTITY_REF(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_NODE_ENTITY_REF_TYPE)

typedef struct CongEditorNodeEntityRef CongEditorNodeEntityRef;
typedef struct CongEditorNodeEntityRefClass CongEditorNodeEntityRefClass;
typedef struct CongEditorNodeEntityRefDetails CongEditorNodeEntityRefDetails;

struct CongEditorNodeEntityRef
{
	CongEditorNode node;

	CongEditorNodeEntityRefDetails *private;
};

struct CongEditorNodeEntityRefClass
{
	CongEditorNodeClass klass;

	/* Methods? */
};

GType
cong_editor_node_entity_ref_get_type (void);

CongEditorNodeEntityRef*
cong_editor_node_entity_ref_construct (CongEditorNodeEntityRef *editor_node_entity_ref,
				       CongEditorWidget3* widget,
				       CongTraversalNode *traversal_node);

CongEditorNode*
cong_editor_node_entity_ref_new (CongEditorWidget3* widget,
				 CongTraversalNode *traversal_node);

G_END_DECLS

#endif
