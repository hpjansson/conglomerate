/*
  gxx-object-to-xml-tree.h

  Defines macros for converting various kinds of in-memory representations into an XML DOM tree.
  Might eventually have another header that does direct SAX output.

  Designed to work with libxml2 and GLib

  Copyright (C) 2003 David Malcolm, Licensed under the LGPL
 */

/* Get rid of all gxx macros: */
#include "gxx-undefine-shared-macros.h"

#define GXX_STRUCT_BEGIN_ELEMENT_WITH_CONSTRUCTOR(xml_name, type_name, fn_name_frag) \
xmlNodePtr gxx_generated_object_to_xml_tree_fn_##fn_name_frag (const type_name *inst, xmlDocPtr xml_doc) { \
  const gchar * const tag_name = xml_name; \
  xmlNodePtr xml_node; \
  g_return_val_if_fail (inst, NULL); \
  g_return_val_if_fail (xml_doc, NULL); \
  xml_node = xmlNewDocNode (xml_doc, NULL, xml_name, NULL); 

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
    xmlNewProp (xml_node, attr_name, inst->member_name); \
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
    xmlNewProp (xml_node, attr_name, attr_value); \
  }


