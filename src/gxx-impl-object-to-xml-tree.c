/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
  gxx-imp-object-to-xml-tree.c

  Implementation of support functions for converting various kinds of in-memory representations into an XML DOM tree.

  Designed to work with libxml2 and GLib

  Copyright (C) 2003 David Malcolm, Licensed under the LGPL
 */

#include <glib.h>
#include <libxml/tree.h>
#include "gxx-object-to-xml-tree.h"

void
gxx_hash_table_of_children_with_pcdata_to_xml_tree (gpointer key,
						    gpointer value,
						    gpointer user_data)
{
	GXXCallbackData_HashTableOfChildrenWithPCDATA *cb_data = (GXXCallbackData_HashTableOfChildrenWithPCDATA *)user_data;
	xmlNodePtr child_node;

	if (key) {
		g_assert (g_utf8_validate (key, -1, NULL));
	}
	g_assert (value);
	g_assert (g_utf8_validate (value, -1, NULL));

	g_assert (cb_data);
	g_assert (cb_data->xml_node);
	g_assert (cb_data->str_child_name);
	g_assert (cb_data->str_hashing_attribute_name);

	child_node = xmlNewDocNode (cb_data->xml_node->doc,
				    NULL,
				    cb_data->str_child_name,
				    value);
	xmlAddChild (cb_data->xml_node, 
		     child_node);    

	if (key) {
		xmlNsPtr ns;

		ns = xmlSearchNsByHref (cb_data->xml_node->doc, 
					child_node, 
					cb_data->str_hashing_attribute_ns_uri);
		if (ns == NULL) {
			g_warning ("FIXME couldn't find namespace %s ", cb_data->str_hashing_attribute_ns_uri);
		}
		xmlSetNsProp (child_node,
			      ns,
			      cb_data->str_hashing_attribute_name, 
			      key);
	}
}
