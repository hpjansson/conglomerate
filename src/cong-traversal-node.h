/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-traversal-node.h
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

#ifndef __CONG_TRAVERSAL_NODE_H__
#define __CONG_TRAVERSAL_NODE_H__

#include "cong-document.h"

G_BEGIN_DECLS

#define DEBUG_TRAVERSAL_NODE_LIFETIMES 0

typedef struct CongTraversalNodeDetails CongTraversalNodeDetails;

#define CONG_TRAVERSAL_NODE_TYPE	      (cong_traversal_node_get_type ())
#define CONG_TRAVERSAL_NODE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_TRAVERSAL_NODE_TYPE, CongTraversalNode)
#define CONG_TRAVERSAL_NODE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_TRAVERSAL_NODE_TYPE, CongTraversalNodeClass)
#define IS_CONG_TRAVERSAL_NODE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_TRAVERSAL_NODE_TYPE)

/**
 * CongTraversalNode:
 *
 * A GObject subclass, managed by #CongDocument.  There is a single #CongTraversalNode for each time an xml node is reached in a depth
 * first traversal of the document tree, with the proviso that only the "correct" child of an entity ref node is reached (the correct entity
 * declaration, rather than that decl, plus all following siblings, as is reached in a naive implementation).
 *
 * Since they are GObjects, you can listen to signals, etc.
 *
 * The #CongDocument manages these, and destroys them when they are no longer needed.  If an xml node changes position, its old traversal nodes are destroyed,
 * and new ones are created, rather than trying to move the traversal node somehow.
 *
 */
struct CongTraversalNode
{
	GObject object;

	CongTraversalNodeDetails *private;
};

struct CongTraversalNodeClass
{
	GObjectClass klass;
};

GType
cong_traversal_node_get_type (void);

CongTraversalNode*
cong_traversal_node_construct (CongTraversalNode *area,
			       CongDocumentTraversal *doc_traversal,
			       CongNodePtr xml_node,
			       CongTraversalNode *traversal_parent);

CongTraversalNode*
cong_traversal_node_new (CongDocumentTraversal *doc_traversal,
			 CongNodePtr xml_node,
			 CongTraversalNode *traversal_parent);

CongDocumentTraversal*
cong_traversal_node_get_document_traversal (CongTraversalNode *traversal_node);

CongDocument*
cong_traversal_node_get_document (CongTraversalNode *traversal_node);

CongNodePtr
cong_traversal_node_get_node (CongTraversalNode *traversal_node);

CongTraversalNode*
cong_traversal_node_get_parent (CongTraversalNode *traversal_node);

typedef void (*CongTraversalNodeCallback) (CongTraversalNode *traversal_node,
					   gpointer user_data);

void
cong_traversal_node_for_each_child (CongTraversalNode *traversal_node,
				    CongTraversalNodeCallback callback,
				    gpointer user_data);

CongTraversalNode*
cong_traversal_node_get_first_child (CongTraversalNode *traversal_node);

CongTraversalNode*
cong_traversal_node_get_prev (CongTraversalNode *traversal_node);

CongTraversalNode*
cong_traversal_node_get_next (CongTraversalNode *traversal_node);

gboolean
cong_traversal_node_is_referenced_entity_decl (CongTraversalNode *traversal_node);

G_END_DECLS

#endif
