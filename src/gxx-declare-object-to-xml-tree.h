/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
  gxx-declare-object-to-xml-tree.h

  Defines macros that declare functions for converting various kinds of in-memory representations into an XML DOM tree.

  Designed to work with libxml2 and GLib

  Copyright (C) 2003 David Malcolm, Licensed under the LGPL
 */

/* Get rid of all gxx macros: */
#include "gxx-undefine-shared-macros.h"

#define GXX_STRUCT_BEGIN_ELEMENT(xml_name, type_name, fn_name_frag) \
xmlNodePtr gxx_generated_object_to_xml_tree_fn_##fn_name_frag (const type_name *inst, xmlDocPtr xml_doc);

#define GXX_STRUCT_BEGIN_ELEMENT_WITH_CONSTRUCTOR(xml_name, type_name, fn_name_frag) \
xmlNodePtr gxx_generated_object_to_xml_tree_fn_##fn_name_frag (const type_name *inst, xmlDocPtr xml_doc);

#define GXX_STRUCT_END_ELEMENT()

#define GXX_STRUCT_ATTRIBUTE_INT(attr_name, member_name, is_required)
#define GXX_STRUCT_ATTRIBUTE_STRING(attr_name, member_name, is_required, default_value)
#define GXX_STRUCT_ATTRIBUTE_ENUM(attr_name, member_name, is_required, default_value, enum_mapping)
#define GXX_STRUCT_ATTRIBUTE_BOOLEAN(attr_name, member_name, is_required, default_value)
#define GXX_STRUCT_UNIQUE_CHILD_PTR_TO_STRUCT(child_name, member_name, fn_name_frag, is_required)
#define GXX_STRUCT_HASH_TABLE_OF_CHILDREN_WITH_PCDATA(child_name, hashing_attribute_name, hashing_attribute_ns_uri, hash_table_member_name)
