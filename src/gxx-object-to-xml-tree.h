/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
  gxx-object-to-xml-tree.h

  Defines macros for converting various kinds of in-memory representations into an XML DOM tree.
  Might eventually have another header that does direct SAX output.

  Designed to work with libxml2 and GLib

  Copyright (C) 2003 David Malcolm, Licensed under the LGPL
 */

/* Get rid of all gxx macros: */
#include "gxx-undefine-shared-macros.h"

G_BEGIN_DECLS

/* Types and prototypes used by the generated code: */
void
gxx_hash_table_of_children_with_pcdata_to_xml_tree (gpointer key,
						    gpointer value,
						    gpointer user_data);

typedef struct _GXXCallbackData_HashTableOfChildrenWithPCDATA GXXCallbackData_HashTableOfChildrenWithPCDATA;

struct _GXXCallbackData_HashTableOfChildrenWithPCDATA
{
	xmlNodePtr xml_node;
	const gchar *str_child_name;
	const gchar *str_hashing_attribute_name;
	const gchar *str_hashing_attribute_ns_uri;
};

/* Macro definitions: */
#define GXX_STRUCT_BEGIN_ELEMENT(xml_name, type_name, fn_name_frag) \
xmlNodePtr gxx_generated_object_to_xml_tree_fn_##fn_name_frag (const type_name *inst, xmlDocPtr xml_doc) { \
  const gchar * const tag_name = xml_name; \
  xmlNodePtr xml_node; \
  g_return_val_if_fail (inst, NULL); \
  g_return_val_if_fail (xml_doc, NULL); \
  xml_node = xmlNewDocNode (xml_doc, NULL, (const xmlChar*)xml_name, NULL); 

#define GXX_STRUCT_BEGIN_ELEMENT_WITH_CONSTRUCTOR(xml_name, type_name, fn_name_frag) \
xmlNodePtr gxx_generated_object_to_xml_tree_fn_##fn_name_frag (const type_name *inst, xmlDocPtr xml_doc) { \
  const gchar * const tag_name = xml_name; \
  xmlNodePtr xml_node; \
  g_return_val_if_fail (inst, NULL); \
  g_return_val_if_fail (xml_doc, NULL); \
  xml_node = xmlNewDocNode (xml_doc, NULL, (const xmlChar*)xml_name, NULL); 

#define GXX_STRUCT_END_ELEMENT() \
  return xml_node; \
}

#define GXX_STRUCT_ATTRIBUTE_INT(attr_name, member_name, is_required) \
  { \
    gchar *attr_value = g_strdup_printf ("%i", inst->member_name); \
    xmlNewProp (xml_node, attr_name, attr_value); \
    g_free (attr_value); \
  }

#define GXX_STRUCT_ATTRIBUTE_STRING(attr_name, member_name, is_required, default_value) \
  if (inst->member_name) { \
	  xmlNewProp (xml_node, (const xmlChar*)attr_name, (const xmlChar*)inst->member_name); \
  } else { \
    if (is_required) { \
      g_warning ("NULL string for attribute \"%s\" within <%s>", attr_name, tag_name); \
    } \
  }

#define GXX_STRUCT_ATTRIBUTE_ENUM(attr_name, member_name, is_required, default_value, enum_mapping) \
  { \
    const gchar *attr_value = cong_enum_mapping_lookup_string (enum_mapping, \
							       sizeof(enum_mapping)/sizeof(CongEnumMapping), \
							       inst->member_name); \
    xmlNewProp (xml_node, (const xmlChar*)attr_name, (const xmlChar*)attr_value); \
  }

#define GXX_STRUCT_ATTRIBUTE_BOOLEAN(attr_name, member_name, is_required, default_value) \
  { \
    xmlNewProp (xml_node, (const xmlChar*)attr_name, (const xmlChar*)cong_util_bool_to_string (inst->member_name)); \
  }

#define GXX_STRUCT_UNIQUE_CHILD_PTR_TO_STRUCT(child_name, member_name, fn_name_frag, is_required) \
  { \
    if (inst->member_name) { \
		xmlAddChild (xml_node, \
			     gxx_generated_object_to_xml_tree_fn_##fn_name_frag (inst->member_name, xml_doc)); \
    } else { \
      if (is_required) { \
			   g_warning ("missing child <%s> within <%s>", child_name, tag_name); \
      } \
    } \
  }

#define GXX_STRUCT_HASH_TABLE_OF_CHILDREN_WITH_PCDATA(child_name, hashing_attribute_name, hashing_attribute_ns_uri, hash_table_member_name) \
  { \
    GXXCallbackData_HashTableOfChildrenWithPCDATA cb_data; \
    cb_data.xml_node = xml_node; \
    cb_data.str_child_name = child_name; \
    cb_data.str_hashing_attribute_name = hashing_attribute_name; \
    cb_data.str_hashing_attribute_ns_uri = hashing_attribute_ns_uri; \
    g_hash_table_foreach (inst->hash_table_member_name, \
			  gxx_hash_table_of_children_with_pcdata_to_xml_tree, \
			  &cb_data); \
  }

G_END_DECLS
