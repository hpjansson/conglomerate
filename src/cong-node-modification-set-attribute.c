/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-node-modification-set-attribute.c
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
#include "cong-node-modification-set-attribute.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongNodeModificationSetAttributeDetails
{
	gchar *ns_prefix;
	gchar *name;
	gchar *value;
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
GNOME_CLASS_BOILERPLATE(CongNodeModificationSetAttribute, 
			cong_node_modification_set_attribute,
			CongNodeModification,
			CONG_NODE_MODIFICATION_TYPE );

static void
cong_node_modification_set_attribute_class_init (CongNodeModificationSetAttributeClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	CONG_MODIFICATION_CLASS (klass)->undo = undo;
	CONG_MODIFICATION_CLASS (klass)->redo = redo;
}

static void
cong_node_modification_set_attribute_instance_init (CongNodeModificationSetAttribute *node)
{
	node->private = g_new0(CongNodeModificationSetAttributeDetails,1);
}

/**
 * cong_node_modification_set_attribute_construct:
 * @node_modification_set_attribute:
 * @doc:
 * @node:
 * @ns_ptr:
 * @name:
 * @value:
 *
 * TODO: Write me
 * Returns:
 */
CongNodeModificationSetAttribute*
cong_node_modification_set_attribute_construct (CongNodeModificationSetAttribute *node_modification_set_attribute,
						CongDocument *doc,
						CongNodePtr node,
						xmlNs *ns_ptr,
						const xmlChar *name, 
						const xmlChar *value)
{
	struct CongNodeModificationSetAttributeDetails *private;

	private = PRIVATE(node_modification_set_attribute);

	cong_node_modification_construct (CONG_NODE_MODIFICATION(node_modification_set_attribute),
					  doc,
					  node);

	if (ns_ptr != NULL) {
		private->ns_prefix = g_strdup(ns_ptr->prefix);
	} else {
		private->ns_prefix = NULL;
	}
	private->name = g_strdup (name);
	private->value = g_strdup (value);
	private->old_value = cong_node_get_attribute (node, ns_ptr, name);

	return node_modification_set_attribute;
}

/**
 * cong_node_modification_set_attribute_new:
 * @doc:
 * @node:
 * @ns_ptr:
 * @name:
 * @value:
 *
 * TODO: Write me
 * Returns:
 */
CongModification*
cong_node_modification_set_attribute_new (CongDocument *doc,
					  CongNodePtr node,
					  xmlNs *ns_ptr,
					  const xmlChar *name, 
					  const xmlChar *value)
{
	return CONG_MODIFICATION(cong_node_modification_set_attribute_construct (CONG_NODE_MODIFICATION_SET_ATTRIBUTE(g_object_new (CONG_NODE_MODIFICATION_SET_ATTRIBUTE_TYPE, NULL)),
										 doc,
										 node,
										 ns_ptr,
										 name,
										 value));
}

static void
finalize (GObject *object)
{
	CongNodeModificationSetAttribute *node_modification_set_attribute = CONG_NODE_MODIFICATION_SET_ATTRIBUTE (object);

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongNodeModificationSetAttribute::finalize");
#endif

	g_free (node_modification_set_attribute->private);
	node_modification_set_attribute->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);

}

static void
dispose (GObject *object)
{
	CongNodeModificationSetAttribute *node_modification_set_attribute = CONG_NODE_MODIFICATION_SET_ATTRIBUTE (object);
	struct CongNodeModificationSetAttributeDetails *private;

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongNodeModificationSetAttribute::dispose");
#endif

	g_assert (node_modification_set_attribute->private);

	private = PRIVATE(node_modification_set_attribute);

	/* Cleanup: */
	if (private->ns_prefix) {
		g_free (private->ns_prefix);
		private->ns_prefix = NULL;
	}

	if (private->name) {
		g_free (private->name);
		private->name = NULL;
	}
	if (private->value) {
		g_free (private->value);
		private->value = NULL;
	}
	if (private->old_value) {
		g_free (private->old_value);
		private->old_value = NULL;
	}

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
undo (CongModification *modification)
{
	struct CongNodeModificationSetAttributeDetails *private = PRIVATE(CONG_NODE_MODIFICATION_SET_ATTRIBUTE (modification));
	CongDocument *doc = cong_modification_get_document (modification);
	CongNodePtr node = cong_node_modification_get_node (CONG_NODE_MODIFICATION(modification));

	xmlNs *ns_ptr = NULL;

	if(private->ns_prefix != NULL) {
		ns_ptr = cong_node_get_ns_for_prefix(node, 
					    private->ns_prefix);
	}

	cong_document_begin_edit (doc);

	if (private->old_value) {
		cong_document_private_node_set_attribute (doc,
							  node,
							  ns_ptr,
							  private->name,
							  private->old_value);
	} else {
		cong_document_private_node_remove_attribute (doc,
							     node,
							     ns_ptr,
							     private->name);
	}
	
	cong_document_end_edit (doc);
}

static void
redo (CongModification *modification)
{
	struct CongNodeModificationSetAttributeDetails *private = PRIVATE(CONG_NODE_MODIFICATION_SET_ATTRIBUTE (modification));
	CongDocument *doc = cong_modification_get_document (modification);
	CongNodePtr node = cong_node_modification_get_node (CONG_NODE_MODIFICATION(modification));
	xmlNs *ns_ptr = NULL;

	if(private->ns_prefix != NULL) {
		ns_ptr = cong_node_get_ns_for_prefix(node, 
						private->ns_prefix);
	}

	cong_document_begin_edit (doc);
	
	cong_document_private_node_set_attribute (doc,
						  node,
						  ns_ptr,
						  private->name,
						  private->value);
	
	cong_document_end_edit (doc);
}
