/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-dispspec-element.h
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

#ifndef __CONG_DISPSPEC_ELEMENT_H__
#define __CONG_DISPSPEC_ELEMENT_H__

/* FIXME: make these private eventually */
struct CongDispspecElementHeaderInfo
{
	gchar *xpath; /* if present, this is the XPath to use when determining the title of the tag */
	gchar *tagname; /* if xpath not present, then look for this tag below the main tag (deprecated) */
};

struct CongDispspecElement
{
	/* URI of namespace, or NULL: */
	gchar *ns_uri;

	/* Local name within namespace (if any); must be non-NULL */
	gchar *local_name;

	GHashTable *hash_of_language_to_user_name;
	GHashTable *hash_of_language_to_short_desc;

	gchar *icon_name;
	GdkPixbuf *icon16;
	enum CongWhitespaceHandling whitespace;

	enum CongElementType type;
	gboolean collapseto;

#if NEW_LOOK
	GdkColor col_array[CONG_DISPSPEC_GC_USAGE_NUM];
	GdkGC* gc_array[CONG_DISPSPEC_GC_USAGE_NUM];
#else
	GdkColor col;
	GdkGC* gc;
#endif

	CongDispspecElementHeaderInfo *header_info;

	gchar *editor_service_id;
	gchar *property_dialog_service_id;

	GHashTable *key_value_hash;

	struct CongDispspecElement* next;	
};

G_BEGIN_DECLS

/** 
 * cong_dispspec_element_new:
 * @ns_uri: the URI of the namespace, or NULL
 * @local_name: the local name for the new element; must be non-NULL
 * @type:
 * @autogenerate_username:  if TRUE, then generate a sane user-visible name for the element,
 * using "header capitalisation"
 *
 * Constructs a new #CongDispspecElement, initialising fields to sane defaults.
 *
 * Returns: a freshly allocated #CongDispspecElement 
 *
 **/
CongDispspecElement*
cong_dispspec_element_new (const gchar* ns_uri, 
			   const gchar* local_name,
			   enum CongElementType type,
			   gboolean autogenerate_username);

/* Destruction  */
void 
cong_dispspec_element_destroy (CongDispspecElement *element); 


/** 
 * cong_dispspec_element_get_ns_uri:
 * @element: the element in question
 * 
 * Returns: the namespace URI for this kind of element, or NULL if none
 *
 */
const gchar*
cong_dispspec_element_get_ns_uri (CongDispspecElement *element); 

/**
 * cong_dispspec_element_get_local_name:
 * @element: the element in question
 * 
 * Returns: the local name (relative to its namespace, if any) for this kind of element
 *
 */
const char*
cong_dispspec_element_get_local_name (CongDispspecElement *element);

/** Get the name in a user-friendly form */
const char*
cong_dispspec_element_username(CongDispspecElement *element);

/** Get a short user-friendly description of the element */
const gchar*
cong_dispspec_element_get_description(CongDispspecElement *element);

/**
 *  Get a pixbuf (if any) for this dispspec; caller is repsonsible for unrefing the pixbuf
 */
GdkPixbuf*
cong_dispspec_element_get_icon(CongDispspecElement *element);

/**
 * cong_dispspec_element_get_value_for_key:
 *
 * @key: the key
 * @element:  the dispspec element
 * 
 * Dispspec elements support a list of key/value string pairs; this is intended as a mechanism
 * to allow plugins to have arbitrary data whilst having a DTD for xds files.
 * 
 * Returns:  the value, if found, or NULL if not present.
 */
const gchar*
cong_dispspec_element_get_value_for_key (const gchar *key, 
					 const CongDispspecElement *element);

const char*
cong_dispspec_element_name_name_get(CongDispspecElement* element);

CongDispspecElement*
cong_dispspec_element_next(CongDispspecElement* element);

enum CongElementType
cong_dispspec_element_type(CongDispspecElement *element);


/**
 * cong_dispspec_element_get_whitespace:
 * @element:  The element of the display spec
 *
 * Get the #CongWhitespaceHandling behaviour for this element
 * Returns:  
 */
enum CongWhitespaceHandling
cong_dispspec_element_get_whitespace (CongDispspecElement *element);

/**
 * cong_dispspec_element_set_whitespace:
 * @element:  The element of the display spec
 * @whitespace: The new value for whitespace handling
 *
 * Set the #CongWhitespaceHandling behaviour for this element
 */
void
cong_dispspec_element_set_whitespace (CongDispspecElement *element,
				      enum CongWhitespaceHandling whitespace);

gboolean
cong_dispspec_element_collapseto(CongDispspecElement *element);

gboolean
cong_dispspec_element_is_structural(CongDispspecElement *element);

gboolean
cong_dispspec_element_is_span(CongDispspecElement *element);

unsigned int
cong_dispspec_element_color(CongDispspecElement *element);

#if NEW_LOOK
GdkGC*
cong_dispspec_element_gc(CongDispspecElement *element, enum CongDispspecGCUsage usage);

const GdkColor*
cong_dispspec_element_col(CongDispspecElement *element, enum CongDispspecGCUsage usage);
#else
GdkGC*
cong_dispspec_element_gc(CongDispspecElement *element);

const GdkColor*
cong_dispspec_element_col(CongDispspecElement *element);
#endif

CongDispspecElementHeaderInfo*
cong_dispspec_element_header_info(CongDispspecElement *element);

/*
  caller must free result
 */
gchar*
cong_dispspec_element_header_info_get_xpath_expression (CongDispspecElementHeaderInfo* header_info);

gchar*
cong_dispspec_element_get_title(CongDispspecElement *element, CongNodePtr x);

gchar*
cong_dispspec_element_get_section_header_text(CongDispspecElement *element, CongNodePtr x);

CongFont*
cong_dispspec_element_get_font(CongDispspecElement *element, enum CongFontRole role);

const gchar*
cong_dispspec_element_get_editor_service_id(CongDispspecElement *element);

const gchar*
cong_dispspec_element_get_property_dialog_service_id(CongDispspecElement *element);

/**
 * cong_dispspec_element_to_xml:
 *
 * @element:  the dispspec element we are adding
 * @xml_doc:  the XML document we are creating
 *
 * Create an element tag suitable for adding to an element-list tag within an xds XML document.
 *
 * Returns: a newly-created #xmlNodePtr
 */
xmlNodePtr
cong_dispspec_element_to_xml (const CongDispspecElement *element,
			      xmlDocPtr xml_doc);

/**
 * cong_dispspec_element_from_xml:
 *
 * @xml_element:  the element within the XML document
 *
 * Create an #CongDispspecElement from an xds XML representation
 *
 * Returns: a newly-created #CongDispspecElement
 *
 */
CongDispspecElement*
cong_dispspec_element_from_xml (xmlNodePtr xml_element);


G_END_DECLS

#endif
