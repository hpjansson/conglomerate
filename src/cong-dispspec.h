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

GdkGC* generate_gc_for_col(const GdkColor *col);

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

/**
 *  Get a pixbuf (if any) for this dispspec; caller is repsonsible for unrefing the pixbuf
 */
GdkPixbuf*
cong_dispspec_get_icon(const CongDispspec *ds);

/* Getting at elements within a dispspec */
CongDispspecElement*
cong_dispspec_lookup_element (const CongDispspec *ds, 
			      const gchar* ns_uri, 
			      const gchar* local_name);

CongDispspecElement*
cong_dispspec_lookup_node (const CongDispspec *ds, 
			   CongNodePtr node);

enum CongElementType
cong_dispspec_type (CongDispspec *ds, 
		    const gchar* ns_uri, 
		    const gchar* local_name);

CongDispspecElement*
cong_dispspec_get_first_element(CongDispspec *ds);

/* Manipulating a dispspec: */
void cong_dispspec_add_element (CongDispspec* ds, 
				CongDispspecElement* element);

/* Caution: this is o(n) */
guint
cong_dispspec_get_num_elements (CongDispspec *ds);

/* Caution: this is o(n) */
CongDispspecElement*
cong_dispspec_get_element (CongDispspec *ds,
			   guint index);

/* Various functions that may get deprecated at some point: */
#if NEW_LOOK
GdkGC *cong_dispspec_gc_get(CongDispspec *ds, CongNodePtr x, enum CongDispspecGCUsage usage);
#else
#if 0
GdkGC *cong_dispspec_name_gc_get(CongDispspec *ds, TTREE *t, int tog);
#endif
GdkGC *cong_dispspec_gc_get(CongDispspec *ds, CongNodePtr x, int tog);
#endif
const char *cong_dispspec_name_get(CongDispspec *ds, CongNodePtr x);

#if 1
gboolean cong_dispspec_element_structural(CongDispspec *ds, const gchar *ns_uri, const gchar *local_name);
gboolean cong_dispspec_element_collapse(CongDispspec *ds, const gchar *ns_uri, const gchar *local_name);
gboolean cong_dispspec_element_span(CongDispspec *ds, const gchar *ns_uri, const gchar *local_name);
gboolean cong_dispspec_element_insert(CongDispspec *ds, const gchar *ns_uri, const gchar *local_name);
#endif


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

xmlNodePtr
cong_dispspec_get_template(const CongDispspec *ds);

/*
 * cong_dispspec_calculate_coverage:
 *
 * @ds:  The #CongDispspec to use
 * @xml_doc:  An xml document to be analysed
 *
 * Utility function for choosing between different dispspecs to use with a document.
 *
 * Returns:  The fraction of the element nodes in the document which are covered by the dispspec (between 0 and 1)
 *
 */
gdouble
cong_dispspec_calculate_coverage (const CongDispspec *ds,
				  xmlDocPtr xml_doc);

G_END_DECLS

#endif



