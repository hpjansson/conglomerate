/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
  gxx-object-from-xml-tree.h

  Defines macros for creating various kinds of in-memory representations from an XML DOM tree.
  Might eventually have another header that does direct SAX input.

  Designed to work with libxml2 and GLib

  Copyright (C) 2003 David Malcolm, Licensed under the LGPL
 */

/* Get rid of all gxx macros: */
#include "gxx-undefine-shared-macros.h"

#define GXX_STRUCT_BEGIN_ELEMENT(xml_name, type_name, fn_name_frag) \
type_name *gxx_generated_object_from_xml_tree_fn_##fn_name_frag (xmlNodePtr xml_node) { \
  const gchar * const tag_name = xml_name; \
  type_name *inst; \
  g_return_val_if_fail (xml_node, NULL); \
  inst = g_new0 (type_name, 1);

#define GXX_STRUCT_BEGIN_ELEMENT_WITH_CONSTRUCTOR(xml_name, type_name, fn_name_frag) \
type_name *gxx_generated_object_from_xml_tree_fn_##fn_name_frag (xmlNodePtr xml_node) { \
  const gchar * const tag_name = xml_name; \
  type_name *inst; \
  g_return_val_if_fail (xml_node, NULL); \
  inst = gxx_callback_construct_##fn_name_frag(); \

#define GXX_STRUCT_END_ELEMENT() \
  return inst; \
}

#define GXX_STRUCT_ATTRIBUTE_STRING(attr_name, member_name, is_required, default_value) \
  { \
    xmlChar *xml_value = xmlGetProp (xml_node, attr_name); \
    if (xml_value) { \
      if (inst->member_name) { \
        g_free (inst->member_name); \
      } \
      inst->member_name = g_strdup (xml_value); \
      xmlFree (xml_value); \
    } else {\
      if (is_required) { \
        g_warning ("Missing attribute \"%s\" within <%s>", attr_name, tag_name); \
      } \
    } \
  }

#define GXX_STRUCT_ATTRIBUTE_ENUM(attr_name, member_name, is_required, default_value, enum_mapping) \
  { \
    xmlChar *prop = xmlGetProp (xml_node, attr_name); \
    if (prop) { \
      inst->member_name = cong_enum_mapping_lookup (enum_mapping, \
						    sizeof(enum_mapping)/sizeof(CongEnumMapping), \
						    prop, \
						    default_value); \
      xmlFree (prop); \
    } else { \
      if (is_required) { \
        g_warning ("Missing attribute \"%s\" within <%s>", attr_name, tag_name); \
        inst->member_name = default_value; \
      } \
    } \
  }

#define GXX_STRUCT_UNIQUE_CHILD_PTR_TO_STRUCT(child_name, member_name, fn_name_frag, is_required) \
  { \
    xmlNodePtr child; \
    for (child = xml_node->children; child; child=child->next) { \
      if (0==strcmp(child->name,child_name)) { \
	inst->member_name = gxx_generated_object_from_xml_tree_fn_##fn_name_frag (child); \
      } \
    } \
    if (is_required) { \
     if (NULL==inst->member_name) { \
       g_warning("Missing child <%s> within <%s>", child_name, tag_name); \
     } \
    } \
  }

#define GXX_STRUCT_HASH_TABLE_OF_CHILDREN_WITH_PCDATA(child_name, hashing_attribute_name, hashing_attribute_ns_uri, hash_table_member_name) \
  { \
    xmlNodePtr child; \
    gboolean is_required = FALSE; /* for now */ \
    for (child = xml_node->children; child; child=child->next) { \
      if (0==strcmp(child->name,child_name)) { \
        gchar *pcdata = xmlNodeListGetString(xml_node->doc, child->xmlChildrenNode, 1); \
	if (pcdata) { \
          gchar *hash_attr = xmlGetNsProp (child, hashing_attribute_name, hashing_attribute_ns_uri); \
          if (hash_attr) { \
            g_hash_table_insert (inst->hash_table_member_name, \
				 g_strdup (hash_attr), \
				 g_strdup (pcdata)); \
	    g_free (hash_attr); \
	  } else { \
            if (is_required) { \
                g_warning("Missing attribute \"%s\" within <%s>", hashing_attribute_name, child_name); \
              } else { \
                g_hash_table_insert (inst->hash_table_member_name, \
				     NULL, \
				     g_strdup (pcdata)); \
              } \
	  } \
          g_free (pcdata); \
	} else { \
	  g_warning("Missing PCDATA within <%s>", child_name); \
	} \
      } \
    } \
  }
