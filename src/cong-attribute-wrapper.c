/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-attribute-wrapper.c
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
#include "cong-attribute-wrapper.h"
#include "cong-eel.h"
#include "cong-command.h"
#include "cong-util.h"

#define PRIVATE(x) ((x)->private)

/* Internal types: */
struct _CongAttributeWrapperDetails
{
	CongDocument *doc;
	CongNodePtr node;
	gchar *attribute_name;
	xmlNs *ns_ptr;

	gulong handler_id_node_set_attribute;
	gulong handler_id_node_remove_attribute;
};

/* Internal function declarations: */
static void
finalize (GObject *object);

static void
dispose (GObject *object);

static void
on_set_attribute (CongDocument *doc, 
		  CongNodePtr node, 
		  const xmlNs *ns_ptr,
		  const gchar *name, 
		  const gchar *value, 
		  CongAttributeWrapper *attribute_wrapper);
static void
on_remove_attribute (CongDocument *doc, 
		     CongNodePtr node, 
		     const xmlNs *ns_ptr,
		     const gchar *name,
		     CongAttributeWrapper *attribute_wrapper);

/* Exported function definitions: */
G_DEFINE_TYPE(CongAttributeWrapper,
              cong_attribute_wrapper,
              G_TYPE_OBJECT);

CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_attribute_wrapper, set_attribute_handler);
CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_attribute_wrapper, remove_attribute_handler);

static void
cong_attribute_wrapper_class_init (CongAttributeWrapperClass *klass)
{
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_attribute_wrapper,
					      set_attribute_handler);
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_attribute_wrapper,
					      remove_attribute_handler);

	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;
}

static void
cong_attribute_wrapper_init (CongAttributeWrapper *attribute_wrapper)
{
	attribute_wrapper->private = g_new0(CongAttributeWrapperDetails,1);
}

/**
 * cong_attribute_wrapper_construct:
 * @attribute_wrapper:
 * @doc:
 * @node:
 * @ns_ptr:
 * @attribute_name:
 *
 * TODO: Write me
 * Returns:
 */
CongAttributeWrapper*
cong_attribute_wrapper_construct (CongAttributeWrapper *attribute_wrapper,
				  CongDocument *doc,
				  CongNodePtr node,
				  xmlNs *ns_ptr,
				  const gchar *attribute_name)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_WRAPPER(attribute_wrapper), NULL);

	PRIVATE(attribute_wrapper)->doc = doc;
	g_object_ref(doc);

	PRIVATE(attribute_wrapper)->node = node;
	PRIVATE(attribute_wrapper)->attribute_name = g_strdup(attribute_name);

	PRIVATE(attribute_wrapper)->ns_ptr = ns_ptr;

	PRIVATE(attribute_wrapper)->handler_id_node_set_attribute = g_signal_connect_after (G_OBJECT(doc),
											   "node_set_attribute",
											   G_CALLBACK(on_set_attribute),
											   attribute_wrapper);

	PRIVATE(attribute_wrapper)->handler_id_node_remove_attribute = g_signal_connect_after (G_OBJECT(doc),
											      "node_remove_attribute",
											      G_CALLBACK(on_remove_attribute),
											      attribute_wrapper);
	
	return CONG_ATTRIBUTE_WRAPPER (attribute_wrapper);
}

/**
 * cong_attribute_wrapper_get_document:
 * @attribute_wrapper:
 *
 * TODO: Write me
 * Returns:
 */
CongDocument*
cong_attribute_wrapper_get_document (CongAttributeWrapper *attribute_wrapper)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_WRAPPER(attribute_wrapper), NULL);

	return PRIVATE(attribute_wrapper)->doc; 
}

/**
 * cong_attribute_wrapper_get_node:
 * @attribute_wrapper:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr
cong_attribute_wrapper_get_node (CongAttributeWrapper *attribute_wrapper)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_WRAPPER(attribute_wrapper), NULL);

	return PRIVATE(attribute_wrapper)->node;
}

/**
 * cong_attribute_wrapper_get_ns:
 * @attribute_wrapper:
 *
 * TODO: Write me
 * Returns:
 */
xmlNs *
cong_attribute_wrapper_get_ns (CongAttributeWrapper *attribute_wrapper)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_WRAPPER(attribute_wrapper), NULL);

	return PRIVATE(attribute_wrapper)->ns_ptr;	
}

/**
 * cong_attribute_wrapper_get_attribute_name:
 * @attribute_wrapper:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_attribute_wrapper_get_attribute_name (CongAttributeWrapper *attribute_wrapper)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_WRAPPER(attribute_wrapper), NULL);

	return PRIVATE(attribute_wrapper)->attribute_name;
}

/**
 * cong_attribute_wrapper_get_attribute_value:
 * @attribute_wrapper:
 *
 * Returns the content of the attribute represented by @attribute_wrapper.
 *
 * Returns: The content of the attribute, to be freed by the caller.
 *          Can be NULL.
 */

gchar*
cong_attribute_wrapper_get_attribute_value (CongAttributeWrapper *attribute_wrapper)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_WRAPPER(attribute_wrapper), NULL);

	return cong_node_get_attribute (PRIVATE(attribute_wrapper)->node, 
					PRIVATE(attribute_wrapper)->ns_ptr,
					PRIVATE(attribute_wrapper)->attribute_name);
}

/**
 * cong_attribute_wrapper_set_value:
 * @attribute_wrapper:
 * @new_value:
 *
 * TODO: Write me
 */
void
cong_attribute_wrapper_set_value (CongAttributeWrapper *attribute_wrapper,
				  const gchar *new_value)
{
	CongDocument *doc = cong_attribute_wrapper_get_document (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper));
	CongNodePtr node = cong_attribute_wrapper_get_node (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper));
	const gchar *attribute_name = cong_attribute_wrapper_get_attribute_name (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper));
	xmlNs *ns_ptr = cong_attribute_wrapper_get_ns (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper));

	gchar *desc = g_strdup_printf ( _("Set attribute \"%s\" to \"%s\""), attribute_name, new_value);

	CongCommand *cmd = cong_document_begin_command (doc,
							desc,
							NULL);

	g_free (desc);

	cong_command_add_node_set_attribute (cmd,
					     node,
					     ns_ptr,
					     attribute_name,
					     new_value);
	cong_document_end_command (doc,
				   cmd);
}

/**
 * cong_attribute_wrapper_remove_value:
 * @attribute_wrapper:
 *
 * TODO: Write me
 */
void
cong_attribute_wrapper_remove_value (CongAttributeWrapper *attribute_wrapper)
{
	CongDocument *doc = cong_attribute_wrapper_get_document (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper));
	CongNodePtr node = cong_attribute_wrapper_get_node (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper));
	const gchar *attribute_name = cong_attribute_wrapper_get_attribute_name (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper));
	xmlNs *ns_ptr = cong_attribute_wrapper_get_ns (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper));
	
	gchar *desc = g_strdup_printf ( _("Delete attribute \"%s\""), attribute_name);

	CongCommand *cmd = cong_document_begin_command (doc,
							desc,
							NULL);

	g_free (desc);

	cong_command_add_node_remove_attribute (cmd,
						node,
						ns_ptr,
						attribute_name);
	cong_document_end_command (doc,
				   cmd);
}

/* Internal function definitions: */
static void
finalize (GObject *object)
{
	CongAttributeWrapper *attribute_wrapper = CONG_ATTRIBUTE_WRAPPER(object);
	
	g_free (attribute_wrapper->private);
	attribute_wrapper->private = NULL;
	
	G_OBJECT_CLASS (cong_attribute_wrapper_parent_class)->finalize (object);
}

static void
dispose (GObject *object)
{
	CongAttributeWrapper *attribute_wrapper = CONG_ATTRIBUTE_WRAPPER(object);

	if (PRIVATE(attribute_wrapper)->doc) {
	
		g_signal_handler_disconnect (G_OBJECT (PRIVATE(attribute_wrapper)->doc),
					     PRIVATE(attribute_wrapper)->handler_id_node_set_attribute);
		g_signal_handler_disconnect (G_OBJECT (PRIVATE(attribute_wrapper)->doc),
					     PRIVATE(attribute_wrapper)->handler_id_node_remove_attribute);
		
		g_object_unref (G_OBJECT (PRIVATE(attribute_wrapper)->doc));
		PRIVATE(attribute_wrapper)->doc = NULL;
		
		g_free (PRIVATE(attribute_wrapper)->attribute_name);
	}
		
	G_OBJECT_CLASS (cong_attribute_wrapper_parent_class)->finalize (object);
}

static void
on_set_attribute (CongDocument *doc, 
		  CongNodePtr node, 
		  const xmlNs *ns_ptr,
		  const gchar *name, 
		  const gchar *value, 
		  CongAttributeWrapper *attribute_wrapper)
{
	g_return_if_fail (IS_CONG_ATTRIBUTE_WRAPPER(attribute_wrapper));

	if (node == cong_attribute_wrapper_get_node (attribute_wrapper)) {
		if (0 == strcmp(name, cong_attribute_wrapper_get_attribute_name (attribute_wrapper)) &&
		    cong_util_ns_equality (ns_ptr, cong_attribute_wrapper_get_ns (attribute_wrapper))) {
			CONG_EEL_CALL_METHOD (CONG_ATTRIBUTE_WRAPPER_CLASS,
					      attribute_wrapper,
					      set_attribute_handler, 
					      (attribute_wrapper));
		}
	}
}

static void
on_remove_attribute (CongDocument *doc, 
		     CongNodePtr node, 
		     const xmlNs *ns_ptr,
		     const gchar *name,
		     CongAttributeWrapper *attribute_wrapper)
{
	g_return_if_fail (IS_CONG_ATTRIBUTE_WRAPPER(attribute_wrapper));

	if (node == cong_attribute_wrapper_get_node (attribute_wrapper)) {
		if (0 == strcmp(name, cong_attribute_wrapper_get_attribute_name (attribute_wrapper)) &&
		    cong_util_ns_equality (ns_ptr, cong_attribute_wrapper_get_ns (attribute_wrapper))) {
			CONG_EEL_CALL_METHOD (CONG_ATTRIBUTE_WRAPPER_CLASS,
					      attribute_wrapper,
					      remove_attribute_handler,
					      (attribute_wrapper));
		}
	}
}

/**
 * cong_attribute_wrapper_bind_to_widget:
 * @wrapper:
 * @widget:
 *
 * TODO: Write me
 */

void
cong_attribute_wrapper_bind_to_widget (CongAttributeWrapper* wrapper,
			GtkWidget *widget)
{
     g_return_if_fail (IS_CONG_ATTRIBUTE_WRAPPER (wrapper));

     /* Leak it for now? Or attach as generic data and attach a signal handler to the widget's destroy signal? */
}
