/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-node-modification-add-before.c
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
#include "cong-node-modification-add-before.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongNodeModificationAddBeforeDetails
{
	/* Redo info: */
	CongNodePtr new_parent;
	CongNodePtr new_younger_sibling;

	/* Undo info: */
	CongNodePtr former_parent;
	CongNodePtr former_younger_sibling;
};

/* Internal function declarations: */
static void
finalize (GObject *object);

static void
dispose (GObject *object);

static void
undo (CongModification *modification);

static void
redo (CongModification *modification);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongNodeModificationAddBefore, 
			cong_node_modification_add_before,
			CongNodeModification,
			CONG_NODE_MODIFICATION_TYPE );

static void
cong_node_modification_add_before_class_init (CongNodeModificationAddBeforeClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	CONG_MODIFICATION_CLASS (klass)->undo = undo;
	CONG_MODIFICATION_CLASS (klass)->redo = redo;
}

static void
cong_node_modification_add_before_instance_init (CongNodeModificationAddBefore *node)
{
	node->private = g_new0(CongNodeModificationAddBeforeDetails,1);
}

/**
 * cong_node_modification_add_before_construct:
 * @node_modification_add_before:
 * @doc:
 * @node:
 * @younger_sibling:
 *
 * TODO: Write me
 * Returns:
 */
CongNodeModificationAddBefore*
cong_node_modification_add_before_construct (CongNodeModificationAddBefore *node_modification_add_before,
					     CongDocument *doc,
					     CongNodePtr node,
					     CongNodePtr younger_sibling)
{
	cong_node_modification_construct (CONG_NODE_MODIFICATION(node_modification_add_before),
					  doc,
					  node);

	PRIVATE(node_modification_add_before)->new_younger_sibling = younger_sibling;
	cong_document_node_ref (doc, younger_sibling);

	if (younger_sibling->parent) {
		PRIVATE(node_modification_add_before)->new_parent = younger_sibling->parent;
		cong_document_node_ref (doc, PRIVATE(node_modification_add_before)->new_parent);
	}

	if (node->parent) {
		PRIVATE(node_modification_add_before)->former_parent = node->parent;
		cong_document_node_ref (doc, PRIVATE(node_modification_add_before)->former_parent);		
	}
	if (node->next) {
		PRIVATE(node_modification_add_before)->former_younger_sibling = node->next;
		cong_document_node_ref (doc, PRIVATE(node_modification_add_before)->former_younger_sibling);		
	}

	return node_modification_add_before;
}

/**
 * cong_node_modification_add_before_new:
 * @doc:
 * @node:
 * @younger_sibling:
 *
 * TODO: Write me
 * Returns:
 */
CongModification*
cong_node_modification_add_before_new (CongDocument *doc,
				      CongNodePtr node,
				      CongNodePtr younger_sibling)
{
	return CONG_MODIFICATION(cong_node_modification_add_before_construct (CONG_NODE_MODIFICATION_ADD_BEFORE(g_object_new (CONG_NODE_MODIFICATION_ADD_BEFORE_TYPE, NULL)),
									     doc,
									     node,
									     younger_sibling));
}

static void
finalize (GObject *object)
{
	CongNodeModificationAddBefore *node_modification_add_before = CONG_NODE_MODIFICATION_ADD_BEFORE (object);

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongNodeModificationAddBefore::finalize");
#endif

	g_free (node_modification_add_before->private);
	node_modification_add_before->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);

}

static void
dispose (GObject *object)
{
	CongNodeModificationAddBefore *node_modification_add_before = CONG_NODE_MODIFICATION_ADD_BEFORE (object);
	CongDocument *doc = cong_modification_get_document(CONG_MODIFICATION(node_modification_add_before));

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongNodeModificationAddBefore::dispose");
#endif

	g_assert (node_modification_add_before->private);
	
	/* Cleanup: */
	if (PRIVATE(node_modification_add_before)->new_younger_sibling) {
		
		cong_document_node_unref (doc,
					  PRIVATE(node_modification_add_before)->new_younger_sibling);
		PRIVATE(node_modification_add_before)->new_younger_sibling = NULL;
	}
	if (PRIVATE(node_modification_add_before)->new_parent) {
		cong_document_node_unref (doc, 
					  PRIVATE(node_modification_add_before)->new_parent);
		PRIVATE(node_modification_add_before)->new_parent = NULL;
	}
	if (PRIVATE(node_modification_add_before)->former_parent) {
		cong_document_node_unref (doc, 
					  PRIVATE(node_modification_add_before)->former_parent);		
		PRIVATE(node_modification_add_before)->former_parent = NULL;
	}
	if (PRIVATE(node_modification_add_before)->former_younger_sibling) {
		cong_document_node_unref (doc, 
					PRIVATE(node_modification_add_before)->former_younger_sibling);		
		PRIVATE(node_modification_add_before)->former_younger_sibling = NULL;
	}

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
undo (CongModification *modification)
{
	CongNodeModificationAddBefore *node_modification_add_before = CONG_NODE_MODIFICATION_ADD_BEFORE (modification);
	CongDocument *doc = cong_modification_get_document (modification);
	CongNodePtr node = cong_node_modification_get_node (CONG_NODE_MODIFICATION(modification));

	g_assert (node->parent == PRIVATE(node_modification_add_before)->new_parent);
	g_assert (node->next == PRIVATE(node_modification_add_before)->new_younger_sibling);

	cong_document_begin_edit (doc);

	if (PRIVATE(node_modification_add_before)->former_parent) {
		if (PRIVATE(node_modification_add_before)->former_younger_sibling) {
			cong_document_private_node_add_before (doc, 
							       node, 
							       PRIVATE(node_modification_add_before)->former_younger_sibling);
		} else {
			cong_document_private_node_set_parent (doc, 
							       node, 
							       PRIVATE(node_modification_add_before)->former_parent,
							       TRUE);
		}
	} else {
		cong_document_private_node_make_orphan (doc, 
							node);
	}

	cong_document_end_edit (doc);

	g_assert (node->parent == PRIVATE(node_modification_add_before)->former_parent);
	g_assert (node->next == PRIVATE(node_modification_add_before)->former_younger_sibling);
}

static void
redo (CongModification *modification)
{
	CongNodeModificationAddBefore *node_modification_add_before = CONG_NODE_MODIFICATION_ADD_BEFORE (modification);
	CongDocument *doc = cong_modification_get_document (modification);
	CongNodePtr node = cong_node_modification_get_node (CONG_NODE_MODIFICATION(modification));

	g_assert (node->parent == PRIVATE(node_modification_add_before)->former_parent);
	g_assert (node->next == PRIVATE(node_modification_add_before)->former_younger_sibling);
	
	cong_document_begin_edit (doc);

	if (PRIVATE(node_modification_add_before)->new_younger_sibling) {
		cong_document_private_node_add_before (doc, 
						       node, 
						       PRIVATE(node_modification_add_before)->new_younger_sibling);
	} else {
		cong_document_private_node_set_parent (doc, 
						       node, 
						       PRIVATE(node_modification_add_before)->new_parent,
						       TRUE);
	}

	cong_document_end_edit (doc);

	g_assert (node->parent == PRIVATE(node_modification_add_before)->new_parent);
	g_assert (node->next == PRIVATE(node_modification_add_before)->new_younger_sibling);
}

