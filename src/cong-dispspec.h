/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-dispspec.h
 *
 * Copyright (C) 2002 David Malcolm
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

#ifndef __CONG_DISPSPEC_H__
#define __CONG_DISPSPEC_H__

G_BEGIN_DECLS

enum CongDocumentModelType
{
	CONG_DOCUMENT_MODE_TYPE_DTD,
	CONG_DOCUMENT_MODE_TYPE_W3C_XML_SCHEMA,
	CONG_DOCUMENT_MODE_TYPE_RELAX_NG_SCHEMA,

	NUM_CONG_DOCUMENT_MODEL_TYPES
};

typedef struct CongSerialisationFormat CongSerialisationFormat;
typedef struct CongExternalDocumentModel CongExternalDocumentModel;

/*******************************
   cong_dispspec stuff: 
*******************************/

/* Barebones constructor: */
CongDispspec* cong_dispspec_new(void);

/* Constructors that use the standard format: */
CongDispspec* cong_dispspec_new_from_ds_file(const char *name);
GnomeVFSResult cong_dispspec_new_from_xds_file(GnomeVFSURI *uri, CongDispspec** ds);
CongDispspec* cong_dispspec_new_from_xds_buffer(const char *buffer, size_t size);

/* Constructors that try to generate from another format: */
CongDispspec* 
cong_dispspec_new_generate_from_xml_file (xmlDocPtr doc,
					  const gchar *extension);
CongDispspec* 
cong_dispspec_new_generate_from_dtd (xmlDtdPtr dtd, 
				     const gchar *name, 
				     const gchar *description);

/* Destruction: */
void
cong_dispspec_delete (CongDispspec *dispspec);

/**
 *  Routine to manufacture a XML representation of the dispspec.  Can be saved to disk, used as a CongDocument etc
 */
xmlDocPtr
cong_dispspec_make_xml (CongDispspec *dispspec);

/* Data for the dispspec: */
const gchar*
cong_dispspec_get_name(const CongDispspec *ds);

const gchar*
cong_dispspec_get_description(const CongDispspec *ds);

guint
cong_dispspec_get_num_serialisation_formats (const CongDispspec *ds);

const CongSerialisationFormat*
cong_dispspec_get_serialisation_format (const CongDispspec *ds,
					guint index);

/* Returns NULL if it can't find a serialisation format with that extension */
const CongSerialisationFormat*
cong_dispspec_lookup_filename_extension (const CongDispspec *ds,
					 const gchar *extension);

/* Returns whether the dispspec uses that extension */
gboolean
cong_dispspec_matches_filename_extension (const CongDispspec *ds,
					  const gchar *extension);


const CongExternalDocumentModel*
cong_dispspec_get_external_document_model (const CongDispspec *ds,
					   enum CongDocumentModelType model_type);

xmlNodePtr
cong_dispspec_get_template(const CongDispspec *ds);

/**
 *  Get a pixbuf (if any) for this dispspec; caller is repsonsible for unrefing the pixbuf
 */
GdkPixbuf*
cong_dispspec_get_icon(const CongDispspec *ds);

/* Getting at elements within a dispspec */
CongDispspecElement*
cong_dispspec_lookup_element(const CongDispspec *ds, const gchar* xmlns, const gchar* tagname);

CongDispspecElement*
cong_dispspec_lookup_node(const CongDispspec *ds, CongNodePtr node);

enum CongElementType
cong_dispspec_type(CongDispspec *ds, const gchar* xmlns, const gchar* tagname);

CongDispspecElement*
cong_dispspec_get_first_element(CongDispspec *ds);

/* Will return NULL if no such tag exists */
CongDispspecElement*
cong_dispspec_get_paragraph(CongDispspec *ds);

/* Manipulating a dispspec: */
void cong_dispspec_add_element (CongDispspec* ds, 
				CongDispspecElement* element);

/* Various functions that may get deprecated at some point: */
#if NEW_LOOK
enum CongDispspecGCUsage
{
	CONG_DISPSPEC_GC_USAGE_BOLD_LINE,
	CONG_DISPSPEC_GC_USAGE_DIM_LINE,
	CONG_DISPSPEC_GC_USAGE_BACKGROUND,
	CONG_DISPSPEC_GC_USAGE_TEXT,

	CONG_DISPSPEC_GC_USAGE_NUM
};
GdkGC *cong_dispspec_gc_get(CongDispspec *ds, CongNodePtr x, enum CongDispspecGCUsage usage);
#else
#if 0
GdkGC *cong_dispspec_name_gc_get(CongDispspec *ds, TTREE *t, int tog);
#endif
GdkGC *cong_dispspec_gc_get(CongDispspec *ds, CongNodePtr x, int tog);
#endif
const char *cong_dispspec_name_get(CongDispspec *ds, CongNodePtr x);

#if 1
gboolean cong_dispspec_element_structural(CongDispspec *ds, const gchar *xmlns, const gchar *name);
gboolean cong_dispspec_element_collapse(CongDispspec *ds, const gchar *xmlns, const gchar *name);
gboolean cong_dispspec_element_span(CongDispspec *ds, const gchar *xmlns, const gchar *name);
gboolean cong_dispspec_element_insert(CongDispspec *ds, const gchar *xmlns, const gchar *name);
#endif


/*******************************
   cong_dispspec_element stuff: 
*******************************/

/* Construction  */
CongDispspecElement*
cong_dispspec_element_new (const gchar* xmlns, 
			   const gchar* tagname, 
			   enum CongElementType type,
			   gboolean autogenerate_username);

/* Destruction  */
void 
cong_dispspec_element_destroy (CongDispspecElement *element); 


/** Get the namespace prefix (if any) */
const gchar*
cong_dispspec_element_get_xmlns(CongDispspecElement *element); 

/** Get the tagname in a parser-friendly form */
const char*
cong_dispspec_element_tagname(CongDispspecElement *element);

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

const char*
cong_dispspec_element_name_name_get(CongDispspecElement* element);

CongDispspecElement*
cong_dispspec_element_next(CongDispspecElement* element);

enum CongElementType
cong_dispspec_element_type(CongDispspecElement *element);

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

gchar*
cong_dispspec_element_get_title(CongDispspecElement *element, CongNodePtr x);

gchar*
cong_dispspec_element_get_section_header_text(CongDispspecElement *element, CongNodePtr x);

CongFont*
cong_dispspec_element_get_font(CongDispspecElement *element, enum CongFontRole role);

const gchar*
cong_dispspec_element_get_editor_plugin_id(CongDispspecElement *element);

const gchar*
cong_dispspec_element_get_property_dialog_plugin_id(CongDispspecElement *element);

/* Serialisation format stuff: */
const gchar*
cong_serialisation_format_get_extension (const CongSerialisationFormat* format);


/* Document model stuff: */
/* e.g. for DocBook 4.1.2: "-//OASIS//DTD DocBook XML V4.1.2//EN" */
const gchar*
cong_external_document_model_get_public_id (const CongExternalDocumentModel* model);

/* e.g. for DocBook 4.1.2: "http://www.oasis-open.org/docbook/xml/4.1.2/docbookx.dtd" */
const gchar*
cong_external_document_model_get_system_id (const CongExternalDocumentModel* model);

G_END_DECLS

#endif



