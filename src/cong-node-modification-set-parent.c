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
	CongNodePtr adoptive_parent;
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

CongNodeModificationSetParent*
cong_node_modification_set_parent_construct (CongNodeModificationSetParent *node_modification_set_parent,
					     CongDocument *doc,
					     CongNodePtr node,
					     CongNodePtr adoptive_parent)
{
	cong_node_modification_construct (CONG_NODE_MODIFICATION(node_modification_set_parent),
					  doc,
					  node);

	PRIVATE(node_modification_set_parent)->adoptive_parent = adoptive_parent;
	cong_document_node_ref (doc, adoptive_parent);

	return node_modification_set_parent;
}

CongModification*
cong_node_modification_set_parent_new (CongDocument *doc,
				       CongNodePtr node,
				       CongNodePtr adoptive_parent)
{
	return CONG_MODIFICATION(cong_node_modification_set_parent_construct (CONG_NODE_MODIFICATION_SET_PARENT(g_object_new (CONG_NODE_MODIFICATION_SET_PARENT_TYPE, NULL)),
									      doc,
									      node,
									      adoptive_parent));
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
	if (PRIVATE(node_modification_set_parent)->adoptive_parent) {
		cong_document_node_unref (cong_modification_get_document(CONG_MODIFICATION(node_modification_set_parent)),
					  PRIVATE(node_modification_set_parent)->adoptive_parent);
		PRIVATE(node_modification_set_parent)->adoptive_parent = NULL;
	}

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
undo (CongModification *modification)
{
	CongNodeModificationSetParent *node_modification_set_parent = CONG_NODE_MODIFICATION_SET_PARENT (modification);

	/* FIXME: handle undo */
}

static void
redo (CongModification *modification)
{
	CongNodeModificationSetParent *node_modification_set_parent = CONG_NODE_MODIFICATION_SET_PARENT (modification);

	/* FIXME: handle redo */
}
