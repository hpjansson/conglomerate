/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-node-modification-add-after.c
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
#include "cong-node-modification-add-after.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongNodeModificationAddAfterDetails
{
	/* Redo info: */
	CongNodePtr new_parent;
	CongNodePtr new_older_sibling;

	/* Undo info: */
	CongNodePtr former_parent;
	CongNodePtr former_older_sibling;
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
GNOME_CLASS_BOILERPLATE(CongNodeModificationAddAfter, 
			cong_node_modification_add_after,
			CongNodeModification,
			CONG_NODE_MODIFICATION_TYPE );

static void
cong_node_modification_add_after_class_init (CongNodeModificationAddAfterClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	CONG_MODIFICATION_CLASS (klass)->undo = undo;
	CONG_MODIFICATION_CLASS (klass)->redo = redo;
}

static void
cong_node_modification_add_after_instance_init (CongNodeModificationAddAfter *node)
{
	node->private = g_new0(CongNodeModificationAddAfterDetails,1);
}

/**
 * cong_node_modification_add_after_construct:
 * @node_modification_add_after:
 * @doc:
 * @node:
 * @older_sibling:
 *
 * TODO: Write me
 * Returns:
 */
CongNodeModificationAddAfter*
cong_node_modification_add_after_construct (CongNodeModificationAddAfter *node_modification_add_after,
					    CongDocument *doc,
					    CongNodePtr node,
					    CongNodePtr older_sibling)
{
	cong_node_modification_construct (CONG_NODE_MODIFICATION(node_modification_add_after),
					  doc,
					  node);

	PRIVATE(node_modification_add_after)->new_older_sibling = older_sibling;
	cong_document_node_ref (doc, older_sibling);

	if (older_sibling->parent) {
		PRIVATE(node_modification_add_after)->new_parent = older_sibling->parent;
		cong_document_node_ref (doc, PRIVATE(node_modification_add_after)->new_parent);
	}

	if (node->parent) {
		PRIVATE(node_modification_add_after)->former_parent = node->parent;
		cong_document_node_ref (doc, PRIVATE(node_modification_add_after)->former_parent);		
	}
	if (node->prev) {
		PRIVATE(node_modification_add_after)->former_older_sibling = node->prev;
		cong_document_node_ref (doc, PRIVATE(node_modification_add_after)->former_older_sibling);		
	}

	return node_modification_add_after;
}

/**
 * cong_node_modification_add_after_new:
 * @doc:
 * @node:
 * @older_sibling:
 *
 * TODO: Write me
 * Returns:
 */
CongModification*
cong_node_modification_add_after_new (CongDocument *doc,
				      CongNodePtr node,
				      CongNodePtr older_sibling)
{
	return CONG_MODIFICATION(cong_node_modification_add_after_construct (CONG_NODE_MODIFICATION_ADD_AFTER(g_object_new (CONG_NODE_MODIFICATION_ADD_AFTER_TYPE, NULL)),
									     doc,
									     node,
									     older_sibling));
}

static void
finalize (GObject *object)
{
	CongNodeModificationAddAfter *node_modification_add_after = CONG_NODE_MODIFICATION_ADD_AFTER (object);

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongNodeModificationAddAfter::finalize");
#endif

	g_free (node_modification_add_after->private);
	node_modification_add_after->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);

}

static void
dispose (GObject *object)
{
	CongNodeModificationAddAfter *node_modification_add_after = CONG_NODE_MODIFICATION_ADD_AFTER (object);
	CongDocument *doc = cong_modification_get_document(CONG_MODIFICATION(node_modification_add_after));

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongNodeModificationAddAfter::dispose");
#endif

	g_assert (node_modification_add_after->private);
	
	/* Cleanup: */
	if (PRIVATE(node_modification_add_after)->new_older_sibling) {
		
		cong_document_node_unref (doc,
					  PRIVATE(node_modification_add_after)->new_older_sibling);
		PRIVATE(node_modification_add_after)->new_older_sibling = NULL;
	}
	if (PRIVATE(node_modification_add_after)->new_parent) {
		cong_document_node_unref (doc, 
					  PRIVATE(node_modification_add_after)->new_parent);
		PRIVATE(node_modification_add_after)->new_parent = NULL;
	}
	if (PRIVATE(node_modification_add_after)->former_parent) {
		cong_document_node_unref (doc, 
					  PRIVATE(node_modification_add_after)->former_parent);		
		PRIVATE(node_modification_add_after)->former_parent = NULL;
	}
	if (PRIVATE(node_modification_add_after)->former_older_sibling) {
		cong_document_node_unref (doc, 
					PRIVATE(node_modification_add_after)->former_older_sibling);		
		PRIVATE(node_modification_add_after)->former_older_sibling = NULL;
	}

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
undo (CongModification *modification)
{
	CongNodeModificationAddAfter *node_modification_add_after = CONG_NODE_MODIFICATION_ADD_AFTER (modification);
	CongDocument *doc = cong_modification_get_document (modification);
	CongNodePtr node = cong_node_modification_get_node (CONG_NODE_MODIFICATION(modification));

	g_assert (node->parent == PRIVATE(node_modification_add_after)->new_parent);
	g_assert (node->prev == PRIVATE(node_modification_add_after)->new_older_sibling);

	cong_document_begin_edit (doc);

	if (PRIVATE(node_modification_add_after)->former_parent) {
		if (PRIVATE(node_modification_add_after)->former_older_sibling) {
			cong_document_private_node_add_after (doc, 
							      node, 
							      PRIVATE(node_modification_add_after)->former_older_sibling);
		} else {
			cong_document_private_node_set_parent (doc, 
							       node, 
							       PRIVATE(node_modification_add_after)->former_parent,
							       FALSE);
		}
	} else {
		cong_document_private_node_make_orphan (doc, 
							node);
	}

	g_assert (node->parent == PRIVATE(node_modification_add_after)->former_parent);
	g_assert (node->prev == PRIVATE(node_modification_add_after)->former_older_sibling);

	cong_document_end_edit (doc);
}

static void
redo (CongModification *modification)
{
	CongNodeModificationAddAfter *node_modification_add_after = CONG_NODE_MODIFICATION_ADD_AFTER (modification);
	CongDocument *doc = cong_modification_get_document (modification);
	CongNodePtr node = cong_node_modification_get_node (CONG_NODE_MODIFICATION(modification));

	g_assert (node->parent == PRIVATE(node_modification_add_after)->former_parent);
	g_assert (node->prev == PRIVATE(node_modification_add_after)->former_older_sibling);
	
	cong_document_begin_edit (doc);

	if (PRIVATE(node_modification_add_after)->new_older_sibling) {
		cong_document_private_node_add_after (doc, 
						      node, 
						      PRIVATE(node_modification_add_after)->new_older_sibling);
	} else {
		cong_document_private_node_set_parent (doc, 
						       node, 
						       PRIVATE(node_modification_add_after)->new_parent,
						       FALSE);
	}

	cong_document_end_edit (doc);

	g_assert (node->parent == PRIVATE(node_modification_add_after)->new_parent);
	g_assert (node->prev == PRIVATE(node_modification_add_after)->new_older_sibling);
}

