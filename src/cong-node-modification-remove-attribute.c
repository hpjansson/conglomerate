/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-node-modification-remove-attribute.c
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
#include "cong-node-modification-remove-attribute.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongNodeModificationRemoveAttributeDetails
{
	gchar *name;
	gchar *old_value; /* can be NULL */
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
GNOME_CLASS_BOILERPLATE(CongNodeModificationRemoveAttribute, 
			cong_node_modification_remove_attribute,
			CongNodeModification,
			CONG_NODE_MODIFICATION_TYPE );

static void
cong_node_modification_remove_attribute_class_init (CongNodeModificationRemoveAttributeClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	CONG_MODIFICATION_CLASS (klass)->undo = undo;
	CONG_MODIFICATION_CLASS (klass)->redo = redo;
}

static void
cong_node_modification_remove_attribute_instance_init (CongNodeModificationRemoveAttribute *node)
{
	node->private = g_new0(CongNodeModificationRemoveAttributeDetails,1);
}

CongNodeModificationRemoveAttribute*
cong_node_modification_remove_attribute_construct (CongNodeModificationRemoveAttribute *node_modification_remove_attribute,
						   CongDocument *doc,
						   CongNodePtr node,
						   const gchar *name)
{
	cong_node_modification_construct (CONG_NODE_MODIFICATION(node_modification_remove_attribute),
					  doc,
					  node);

	PRIVATE(node_modification_remove_attribute)->name = g_strdup (name);
	PRIVATE(node_modification_remove_attribute)->old_value = cong_node_get_attribute (node, name);

	return node_modification_remove_attribute;
}

CongModification*
cong_node_modification_remove_attribute_new (CongDocument *doc,
					     CongNodePtr node,
					     const gchar *name)
{
	return CONG_MODIFICATION(cong_node_modification_remove_attribute_construct (CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE(g_object_new (CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE_TYPE, NULL)),
										    doc,
										    node,
										    name));
}

static void
finalize (GObject *object)
{
	CongNodeModificationRemoveAttribute *node_modification_remove_attribute = CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE (object);

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongNodeModificationRemoveAttribute::finalize");
#endif

	g_free (node_modification_remove_attribute->private);
	node_modification_remove_attribute->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);

}

static void
dispose (GObject *object)
{
	CongNodeModificationRemoveAttribute *node_modification_remove_attribute = CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE (object);

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongNodeModificationRemoveAttribute::dispose");
#endif

	g_assert (node_modification_remove_attribute->private);
	
	/* Cleanup: */
	if (PRIVATE(node_modification_remove_attribute)->name) {
		g_free (PRIVATE(node_modification_remove_attribute)->name);
		PRIVATE(node_modification_remove_attribute)->name = NULL;
	}
	if (PRIVATE(node_modification_remove_attribute)->old_value) {
		g_free (PRIVATE(node_modification_remove_attribute)->old_value);
		PRIVATE(node_modification_remove_attribute)->old_value = NULL;
	}

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
undo (CongModification *modification)
{
	CongNodeModificationRemoveAttribute *node_modification_remove_attribute = CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE (modification);

	CongDocument *doc = cong_modification_get_document (modification);
	CongNodePtr node = cong_node_modification_get_node (CONG_NODE_MODIFICATION(modification));

	cong_document_begin_edit (doc);
	
	if (PRIVATE(node_modification_remove_attribute)->old_value) {
		cong_document_node_set_attribute (doc,
						  node,
						  PRIVATE(node_modification_remove_attribute)->name,
						  PRIVATE(node_modification_remove_attribute)->old_value);
	}
	
	cong_document_end_edit (doc);
}

static void
redo (CongModification *modification)
{
	CongNodeModificationRemoveAttribute *node_modification_remove_attribute = CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE (modification);

	CongDocument *doc = cong_modification_get_document (modification);
	CongNodePtr node = cong_node_modification_get_node (CONG_NODE_MODIFICATION(modification));

	cong_document_begin_edit (doc);
	
	if (PRIVATE(node_modification_remove_attribute)->old_value) {
		cong_document_node_remove_attribute (doc,
						     node,
						     PRIVATE(node_modification_remove_attribute)->name);
	}
	
	cong_document_end_edit (doc);
}

