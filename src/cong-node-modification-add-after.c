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
	CongNodePtr older_sibling;
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

CongNodeModificationAddAfter*
cong_node_modification_add_after_construct (CongNodeModificationAddAfter *node_modification_add_after,
					    CongDocument *doc,
					    CongNodePtr node,
					    CongNodePtr older_sibling)
{
	cong_node_modification_construct (CONG_NODE_MODIFICATION(node_modification_add_after),
					  doc,
					  node);

	PRIVATE(node_modification_add_after)->older_sibling = older_sibling;
	cong_document_node_ref (doc, older_sibling);

	return node_modification_add_after;
}

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

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongNodeModificationAddAfter::dispose");
#endif

	g_assert (node_modification_add_after->private);
	
	/* Cleanup: */
	if (PRIVATE(node_modification_add_after)->older_sibling) {
		
		cong_document_node_unref (cong_modification_get_document(CONG_MODIFICATION(node_modification_add_after)),
					  PRIVATE(node_modification_add_after)->older_sibling);
		PRIVATE(node_modification_add_after)->older_sibling = NULL;
	}

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
undo (CongModification *modification)
{
	CongNodeModificationAddAfter *node_modification_add_after = CONG_NODE_MODIFICATION_ADD_AFTER (modification);

	/* FIXME: handle undo */
	g_assert_not_reached();
}

static void
redo (CongModification *modification)
{
	CongNodeModificationAddAfter *node_modification_add_after = CONG_NODE_MODIFICATION_ADD_AFTER (modification);

	/* FIXME: handle redo */
	g_assert_not_reached();
}

