/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-language.h
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

#ifndef __CONG_LANGUAGE_H__
#define __CONG_LANGUAGE_H__

G_BEGIN_DECLS

typedef struct CongPerLanguageData CongPerLanguageData;

struct CongPerLanguageData
{
	GHashTable *hash_table;
};

CongPerLanguageData*
cong_per_language_data_new (GDestroyNotify value_destroy_func);

void
cong_per_language_data_free (CongPerLanguageData *per_language);

/* get value for the current language */
gpointer
cong_per_language_get_data (CongPerLanguageData *per_language);

gpointer
cong_per_language_get_data_for_lang (CongPerLanguageData *per_language,
				     const gchar *language);

void
cong_per_language_set_data_for_lang (CongPerLanguageData *per_language,
				     const gchar *language,
				     gpointer data);

void
cong_per_language_for_each (CongPerLanguageData *per_language,
			    void (*callback) (gpointer data, 
					      const gchar *language, 
					      gpointer user_data),
			    gpointer user_data);

/* Create from the direct children of a parent node; any children that match the given element are used
   to populate the container, based on their xml:lang value */
CongPerLanguageData*
cong_per_language_data_new_from_xml (xmlDocPtr xml_doc, 
				     CongNodePtr parent_node,
				     const gchar *ns_uri,
				     const gchar *element_name,
				     gpointer (make_data_callback) (xmlDocPtr xml_doc, CongNodePtr node),
				     GDestroyNotify value_destroy_func);

void
cong_per_language_data_to_xml (CongPerLanguageData *per_language,
			       CongNodePtr parent_node,
			       const gchar *ns_uri,
			       const gchar *element_name,
			       CongNodePtr (make_node_callback) (gpointer data));


G_END_DECLS

#endif



