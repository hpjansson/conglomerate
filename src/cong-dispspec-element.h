/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-dispspec-element.h
 *
 * Copyright (C) 2004 David Malcolm
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

G_BEGIN_DECLS

typedef enum 
{
	CONG_ELEMENT_TYPE_STRUCTURAL,
	CONG_ELEMENT_TYPE_SPAN,
	CONG_ELEMENT_TYPE_INSERT,

	CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE,

	/* Other types?  Table? Plugin widget/Bonobo control? */

	CONG_ELEMENT_TYPE_PLUGIN,

	CONG_ELEMENT_TYPE_UNKNOWN,

	CONG_ELEMENT_TYPE_ALL
} CongElementType;

CongDispspecElement*
cong_dispspec_element_new (const gchar* ns_uri, 
			   const gchar* local_name,
			   CongElementType type,
			   gboolean autogenerate_username);

/* Destruction  */
void 
cong_dispspec_element_destroy (CongDispspecElement *element); 


const gchar*
cong_dispspec_element_get_ns_uri (CongDispspecElement *element); 

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

const gchar*
cong_dispspec_element_get_value_for_key (const gchar *key, 
					 const CongDispspecElement *element);

const char*
cong_dispspec_element_name_name_get(CongDispspecElement* element);

CongElementType
cong_dispspec_element_type(CongDispspecElement *element);


CongWhitespaceHandling
cong_dispspec_element_get_whitespace (CongDispspecElement *element);

void
cong_dispspec_element_set_whitespace (CongDispspecElement *element,
				      CongWhitespaceHandling whitespace);

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
cong_dispspec_element_gc(CongDispspecElement *element, CongDispspecGCUsage usage);

const GdkColor*
cong_dispspec_element_col(CongDispspecElement *element, CongDispspecGCUsage usage);
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
cong_dispspec_element_get_font(CongDispspecElement *element, CongFontRole role);

const gchar*
cong_dispspec_element_get_editor_service_id(CongDispspecElement *element);

const gchar*
cong_dispspec_element_get_property_dialog_service_id(CongDispspecElement *element);

xmlNodePtr
cong_dispspec_element_to_xml (const CongDispspecElement *element,
			      xmlDocPtr xml_doc);

CongDispspecElement*
cong_dispspec_element_from_xml (xmlNodePtr xml_element);

CongElementDescription*
cong_dispspec_element_make_element_description (const CongDispspecElement *ds_element);

gboolean
cong_dispspec_element_should_spellcheck (const CongDispspecElement *ds_element);

G_END_DECLS

#endif
