/*
  gxx-declare-object-from-xml-tree.h

  Defines macros that declare functions for creating various kinds of in-memory representations from an XML DOM tree.

  Designed to work with libxml2 and GLib

  Copyright (C) 2003 David Malcolm, Licensed under the LGPL
 */

/* Get rid of all gxx macros: */
#include "gxx-undefine-shared-macros.h"

#define GXX_STRUCT_BEGIN_ELEMENT(xml_name, type_name, fn_name_frag) \
type_name *gxx_generated_object_from_xml_tree_fn_##fn_name_frag (xmlNodePtr xml_node);

#define GXX_STRUCT_BEGIN_ELEMENT_WITH_CONSTRUCTOR(xml_name, type_name, fn_name_frag) \
type_name *gxx_generated_object_from_xml_tree_fn_##fn_name_frag (xmlNodePtr xml_node);

#define GXX_STRUCT_END_ELEMENT()

#define GXX_STRUCT_ATTRIBUTE_STRING(attr_name, member_name, is_required, default_value)
#define GXX_STRUCT_ATTRIBUTE_ENUM(attr_name, member_name, is_required, default_value, enum_mapping)
#define GXX_STRUCT_UNIQUE_CHILD_PTR_TO_STRUCT(child_name, member_name, fn_name_frag, is_required)
