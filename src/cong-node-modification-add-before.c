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
	CongNodePtr younger_sibling;
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

CongNodeModificationAddBefore*
cong_node_modification_add_before_construct (CongNodeModificationAddBefore *node_modification_add_before,
					    CongDocument *doc,
					    CongNodePtr node,
					    CongNodePtr younger_sibling)
{
	cong_node_modification_construct (CONG_NODE_MODIFICATION(node_modification_add_before),
					  doc,
					  node);

	PRIVATE(node_modification_add_before)->younger_sibling = younger_sibling;
	cong_document_node_ref (doc, younger_sibling);

	return node_modification_add_before;
}

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

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongNodeModificationAddBefore::dispose");
#endif

	g_assert (node_modification_add_before->private);
	
	/* Cleanup: */
	if (PRIVATE(node_modification_add_before)->younger_sibling) {
		
		cong_document_node_unref (cong_modification_get_document(CONG_MODIFICATION(node_modification_add_before)),
					  PRIVATE(node_modification_add_before)->younger_sibling);
		PRIVATE(node_modification_add_before)->younger_sibling = NULL;
	}

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
undo (CongModification *modification)
{
	CongNodeModificationAddBefore *node_modification_add_before = CONG_NODE_MODIFICATION_ADD_BEFORE (modification);

	/* FIXME: handle undo */
	g_assert_not_reached();
}

static void
redo (CongModification *modification)
{
	CongNodeModificationAddBefore *node_modification_add_before = CONG_NODE_MODIFICATION_ADD_BEFORE (modification);

	/* FIXME: handle redo */
	g_assert_not_reached();
}

