/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-traversal-node.c
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
#include "cong-traversal-node.h"
#include "cong-document-traversal.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"
#include "cong-marshal.h"

#define PRIVATE(x) ((x)->private)

struct CongTraversalNodeDetails
{
	CongDocumentTraversal *doc_traversal;
	CongNodePtr xml_node;
	CongTraversalNode *traversal_parent;
};

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongTraversalNode, 
			cong_traversal_node,
			GObject,
			G_TYPE_OBJECT );

static void
cong_traversal_node_class_init (CongTraversalNodeClass *klass)
{
}

static void
cong_traversal_node_instance_init (CongTraversalNode *traversal_node)
{
	traversal_node->private = g_new0(CongTraversalNodeDetails,1);
}

CongTraversalNode*
cong_traversal_node_construct (CongTraversalNode *traversal_node,
			       CongDocumentTraversal *doc_traversal,
			       CongNodePtr xml_node,
			       CongTraversalNode *traversal_parent)
{
	g_return_val_if_fail (traversal_node, NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT_TRAVERSAL(doc_traversal), NULL);
	g_return_val_if_fail (xml_node, NULL);

	PRIVATE(traversal_node)->doc_traversal = doc_traversal;
	PRIVATE(traversal_node)->xml_node = xml_node;
	PRIVATE(traversal_node)->traversal_parent = traversal_parent;

	return traversal_node;
}

CongTraversalNode*
cong_traversal_node_new (CongDocumentTraversal *doc_traversal,
			 CongNodePtr xml_node,
			 CongTraversalNode *traversal_parent)
{
	return cong_traversal_node_construct (g_object_new (CONG_TRAVERSAL_NODE_TYPE, NULL),
					      doc_traversal,
					      xml_node,
					      traversal_parent);
}


CongDocumentTraversal*
cong_traversal_node_get_document_traversal (CongTraversalNode *traversal_node)
{
	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE(traversal_node), NULL);

	return PRIVATE (traversal_node)->doc_traversal;
}

CongDocument*
cong_traversal_node_get_document (CongTraversalNode *traversal_node)
{
	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE(traversal_node), NULL);

	return cong_document_traversal_get_document (PRIVATE (traversal_node)->doc_traversal);
}

CongNodePtr
cong_traversal_node_get_node (CongTraversalNode *traversal_node)
{
	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE(traversal_node), NULL);

	return PRIVATE (traversal_node)->xml_node;
}

CongTraversalNode*
cong_traversal_node_get_parent (CongTraversalNode *traversal_node)
{
	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE(traversal_node), NULL);

	return PRIVATE (traversal_node)->traversal_parent;
}

