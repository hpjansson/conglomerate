/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-node-modification-set-parent.c
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
#include "cong-node-modification-set-parent.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongNodeModificationSetParentDetails
{
	/* Undo info: */
	CongNodePtr former_parent;
	CongNodePtr former_older_sibling;

	/* Undo info: */
	CongNodePtr new_parent;
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
GNOME_CLASS_BOILERPLATE(CongNodeModificationSetParent, 
			cong_node_modification_set_parent,
			CongNodeModification,
			CONG_NODE_MODIFICATION_TYPE );

static void
cong_node_modification_set_parent_class_init (CongNodeModificationSetParentClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	CONG_MODIFICATION_CLASS (klass)->undo = undo;
	CONG_MODIFICATION_CLASS (klass)->redo = redo;
}

static void
cong_node_modification_set_parent_instance_init (CongNodeModificationSetParent *node)
{
	node->private = g_new0(CongNodeModificationSetParentDetails,1);
}

/**
 * cong_node_modification_set_parent_construct:
 * @node_modification_set_parent:
 * @doc:
 * @node:
 * @adoptive_parent:
 *
 * TODO: Write me
 * Returns:
 */
CongNodeModificationSetParent*
cong_node_modification_set_parent_construct (CongNodeModificationSetParent *node_modification_set_parent,
					     CongDocument *doc,
					     CongNodePtr node,
					     CongNodePtr new_parent)
{
	cong_node_modification_construct (CONG_NODE_MODIFICATION(node_modification_set_parent),
					  doc,
					  node);

	PRIVATE(node_modification_set_parent)->new_parent = new_parent;
	cong_document_node_ref (doc, new_parent);

	if (node->parent) {
		PRIVATE(node_modification_set_parent)->former_parent = node->parent;
		cong_document_node_ref (doc, PRIVATE(node_modification_set_parent)->former_parent);		
	}
	if (node->prev) {
		PRIVATE(node_modification_set_parent)->former_older_sibling = node->prev;
		cong_document_node_ref (doc, PRIVATE(node_modification_set_parent)->former_older_sibling);
	}

	return node_modification_set_parent;
}

/**
 * cong_node_modification_set_parent_new:
 * @doc:
 * @node:
 * @adoptive_parent:
 *
 * TODO: Write me
 * Returns:
 */
CongModification*
cong_node_modification_set_parent_new (CongDocument *doc,
				       CongNodePtr node,
				       CongNodePtr new_parent)
{
	return CONG_MODIFICATION(cong_node_modification_set_parent_construct (CONG_NODE_MODIFICATION_SET_PARENT(g_object_new (CONG_NODE_MODIFICATION_SET_PARENT_TYPE, NULL)),
									      doc,
									      node,
									      new_parent));
}

static void
finalize (GObject *object)
{
	CongNodeModificationSetParent *node_modification_set_parent = CONG_NODE_MODIFICATION_SET_PARENT (object);

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongNodeModificationSetParent::finalize");
#endif

	g_free (node_modification_set_parent->private);
	node_modification_set_parent->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);

}

static void
dispose (GObject *object)
{
	CongNodeModificationSetParent *node_modification_set_parent = CONG_NODE_MODIFICATION_SET_PARENT (object);

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongNodeModificationSetParent::dispose");
#endif

	g_assert (node_modification_set_parent->private);
	
	/* Cleanup: */
	if (PRIVATE(node_modification_set_parent)->new_parent) {
		cong_document_node_unref (cong_modification_get_document(CONG_MODIFICATION(node_modification_set_parent)),
					  PRIVATE(node_modification_set_parent)->new_parent);
		PRIVATE(node_modification_set_parent)->new_parent = NULL;
	}

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
undo (CongModification *modification)
{
	CongNodeModificationSetParent *node_modification_set_parent = CONG_NODE_MODIFICATION_SET_PARENT (modification);
	CongDocument *doc = cong_modification_get_document (modification);
	CongNodePtr node = cong_node_modification_get_node (CONG_NODE_MODIFICATION(modification));

	g_assert (node->next == NULL);
	g_assert (node->parent == PRIVATE(node_modification_set_parent)->new_parent);

	cong_document_begin_edit (doc);

	if (PRIVATE(node_modification_set_parent)->former_parent) {
		if (PRIVATE(node_modification_set_parent)->former_older_sibling) {
			cong_document_private_node_add_after (doc, 
							      node, 
							      PRIVATE(node_modification_set_parent)->former_older_sibling);
		} else {
			cong_document_private_node_set_parent (doc, 
							       node, 
							       PRIVATE(node_modification_set_parent)->former_parent,
							       TRUE); /* add to end */
		}
	} else {
		cong_document_private_node_make_orphan (doc, 
							node);
	}

	cong_document_end_edit (doc);

	g_assert (node->prev == PRIVATE(node_modification_set_parent)->former_older_sibling);
	g_assert (node->parent == PRIVATE(node_modification_set_parent)->former_parent);
}

static void
redo (CongModification *modification)
{
	CongNodeModificationSetParent *node_modification_set_parent = CONG_NODE_MODIFICATION_SET_PARENT (modification);
	CongDocument *doc = cong_modification_get_document (modification);
	CongNodePtr node = cong_node_modification_get_node (CONG_NODE_MODIFICATION(modification));

	g_assert (node->prev == PRIVATE(node_modification_set_parent)->former_older_sibling);
	g_assert (node->parent == PRIVATE(node_modification_set_parent)->former_parent);

	cong_document_begin_edit (doc);

	cong_document_private_node_set_parent (doc, 
					       node,
					       PRIVATE(node_modification_set_parent)->new_parent,
					       TRUE); /* add to end */

	cong_document_end_edit (doc);

	g_assert (node->next == NULL);
	g_assert (node->parent == PRIVATE(node_modification_set_parent)->new_parent);
}
