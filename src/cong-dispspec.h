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

CongDispspec* cong_dispspec_new_from_ds_file(const char *name);
GnomeVFSResult cong_dispspec_new_from_xds_file(GnomeVFSURI *uri, CongDispspec** ds);
CongDispspec* cong_dispspec_new_from_xds_buffer(const char *buffer, size_t size);
CongDispspec* cong_dispspec_new_from_xml_file(xmlDocPtr doc);
void cong_dispspec_delete(CongDispspec *dispspec);

const gchar*
cong_dispspec_get_name(const CongDispspec *ds);

const gchar*
cong_dispspec_get_description(const CongDispspec *ds);

#if 0
char *cong_dispspec_name_name_get(CongDispspec *ds, TTREE *t);
#endif

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

gboolean cong_dispspec_element_structural(CongDispspec *ds, const char *name);
gboolean cong_dispspec_element_collapse(CongDispspec *ds, const char *name);
gboolean cong_dispspec_element_span(CongDispspec *ds, const char *name);
gboolean cong_dispspec_element_insert(CongDispspec *ds, const char *name);

enum CongElementType
cong_dispspec_type(CongDispspec *ds, const char* tagname);

/* New API for getting at elements within a dispspec */
CongDispspecElement*
cong_dispspec_lookup_element(const CongDispspec *ds, const char* tagname);

CongDispspecElement*
cong_dispspec_lookup_node(const CongDispspec *ds, CongNodePtr node);

CongDispspecElement*
cong_dispspec_get_first_element(CongDispspec *ds);

/* Will return NULL if no such tag exists */
CongDispspecElement*
cong_dispspec_get_paragraph(CongDispspec *ds);

/** Get the tagname in a parser-friendly form */
const char*
cong_dispspec_element_tagname(CongDispspecElement* element);

/** Get the name in a user-friendly form */
const char*
cong_dispspec_element_username(CongDispspecElement* element);

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
cong_dispspec_element_get_section_header_text(CongDispspecElement *element, CongNodePtr x);

CongFont*
cong_dispspec_element_get_font(CongDispspecElement *element, enum CongFontRole role);



G_END_DECLS

#endif



