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

/**
 * cong_traversal_node_construct:
 * @area:
 * @doc_traversal:
 * @xml_node:
 * @traversal_parent:
 *
 * TODO: Write me
 * Returns:
 */
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

/**
 * cong_traversal_node_new:
 * @doc_traversal:
 * @xml_node:
 * @traversal_parent:
 *
 * TODO: Write me
 * Returns:
 */
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

/**
 * cong_traversal_node_get_document_traversal:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongDocumentTraversal*
cong_traversal_node_get_document_traversal (CongTraversalNode *traversal_node)
{
	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE(traversal_node), NULL);

	return PRIVATE (traversal_node)->doc_traversal;
}

/**
 * cong_traversal_node_get_document:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongDocument*
cong_traversal_node_get_document (CongTraversalNode *traversal_node)
{
	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE(traversal_node), NULL);

	return cong_document_traversal_get_document (PRIVATE (traversal_node)->doc_traversal);
}

/**
 * cong_traversal_node_get_node:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr
cong_traversal_node_get_node (CongTraversalNode *traversal_node)
{
	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE(traversal_node), NULL);

	return PRIVATE (traversal_node)->xml_node;
}

/**
 * cong_traversal_node_get_parent:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongTraversalNode*
cong_traversal_node_get_parent (CongTraversalNode *traversal_node)
{
	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE(traversal_node), NULL);

	return PRIVATE (traversal_node)->traversal_parent;
}

/**
 * cong_traversal_node_get_first_child:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongTraversalNode*
cong_traversal_node_get_first_child (CongTraversalNode *traversal_node)
{
	CongNodePtr other_doc_node;

	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE(traversal_node), NULL);

	other_doc_node = cong_traversal_node_get_node(traversal_node)->children;

	if (other_doc_node) {
		return cong_document_traversal_get_traversal_node (PRIVATE (traversal_node)->doc_traversal,
								   other_doc_node,
								   traversal_node);
	} else {
		return NULL;
	}
}

/**
 * cong_traversal_node_get_prev:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongTraversalNode*
cong_traversal_node_get_prev (CongTraversalNode *traversal_node)
{
	CongNodePtr other_doc_node;

	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE(traversal_node), NULL);

	/* If we're traversing below an entity ref node, visiting an entity decl node, then don't return the siblings (which are all the other entity decls in this document */
	if (cong_traversal_node_is_referenced_entity_decl(traversal_node)) {
		return NULL;
	}

	other_doc_node = cong_traversal_node_get_node(traversal_node)->prev;

	if (other_doc_node) {
		return cong_document_traversal_get_traversal_node (PRIVATE (traversal_node)->doc_traversal,
								   other_doc_node,
								   PRIVATE (traversal_node)->traversal_parent);
	} else {
		return NULL;
	}
}

/**
 * cong_traversal_node_get_next:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongTraversalNode*
cong_traversal_node_get_next (CongTraversalNode *traversal_node)
{
	CongNodePtr other_doc_node;

	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE(traversal_node), NULL);

	/* If we're traversing below an entity ref node, visiting an entity decl node, then don't return the siblings (which are all the other entity decls in this document */
	if (cong_traversal_node_is_referenced_entity_decl(traversal_node)) {
		return NULL;
	}

	other_doc_node = cong_traversal_node_get_node(traversal_node)->next;

	if (other_doc_node) {
		return cong_document_traversal_get_traversal_node (PRIVATE (traversal_node)->doc_traversal,
								   other_doc_node,
								   PRIVATE (traversal_node)->traversal_parent);
	} else {
		return NULL;
	}
}

/**
 * cong_traversal_node_is_referenced_entity_decl:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_traversal_node_is_referenced_entity_decl (CongTraversalNode *traversal_node)
{
	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE(traversal_node), FALSE);

	if (cong_node_type (cong_traversal_node_get_node (traversal_node))==CONG_NODE_TYPE_ENTITY_DECL) {
 		if (cong_node_type (cong_traversal_node_get_node (cong_traversal_node_get_parent (traversal_node)))==CONG_NODE_TYPE_ENTITY_REF) {
			g_message ("got a referenced entity decl");
			return TRUE;
		}
	}

	return FALSE;
}

