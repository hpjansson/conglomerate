/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* 
 * cong-dispspec-element-gxx.h
 *  
 * This file will be included multiple times, with different results.
 */

GXX_STRUCT_BEGIN_ELEMENT_WITH_CONSTRUCTOR("element", CongDispspecElement, dispspec_element)
     GXX_STRUCT_ATTRIBUTE_ENUM("type", type, TRUE, CONG_ELEMENT_TYPE_UNKNOWN, type_numeration)
     GXX_STRUCT_ATTRIBUTE_STRING("nsURI", ns_uri, FALSE, "")
     GXX_STRUCT_ATTRIBUTE_STRING("localName", local_name, TRUE, "")
     GXX_STRUCT_ATTRIBUTE_ENUM("whitespace", whitespace, FALSE, CONG_WHITESPACE_NORMALIZE, whitespace_numeration)
     GXX_STRUCT_ATTRIBUTE_STRING("icon", icon_name, FALSE, "")

     GXX_STRUCT_HASH_TABLE_OF_CHILDREN_WITH_PCDATA("name", "lang", XML_XML_NAMESPACE, hash_of_language_to_user_name)
     GXX_STRUCT_HASH_TABLE_OF_CHILDREN_WITH_PCDATA("short-desc", "lang", XML_XML_NAMESPACE, hash_of_language_to_short_desc)

     GXX_STRUCT_UNIQUE_CHILD_PTR_TO_STRUCT("header-info", header_info, header_info, FALSE)
GXX_STRUCT_END_ELEMENT()

GXX_STRUCT_BEGIN_ELEMENT("header-info", CongDispspecElementHeaderInfo, header_info)
     GXX_STRUCT_ATTRIBUTE_STRING("xpath", xpath, FALSE, "")
     GXX_STRUCT_ATTRIBUTE_STRING("tag", tagname, FALSE, "")
GXX_STRUCT_END_ELEMENT()

