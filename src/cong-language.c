/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-language.c
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

#include "global.h"
#include "cong-language.h"
#include "cong-app.h"

CongPerLanguageData*
cong_per_language_data_new (GDestroyNotify value_destroy_func)
{
	CongPerLanguageData *result;

	result = g_new0 (CongPerLanguageData, 1);

	result->hash_table = g_hash_table_new_full (g_str_hash,
						    g_str_equal,
						    g_free,
						    value_destroy_func);
	return result;
}

void
cong_per_language_data_free (CongPerLanguageData *per_language)
{
	g_return_if_fail (per_language);

	g_hash_table_destroy (per_language->hash_table);
	g_free (per_language);	
}


/* get value for the current language */
gpointer
cong_per_language_get_data (CongPerLanguageData *per_language)
{
	const GList *lang_iter;
	gpointer result;

	g_return_val_if_fail (per_language, NULL);

	for (lang_iter = cong_app_get_language_list (cong_app_singleton ()); lang_iter; lang_iter=lang_iter->next) {
		const gchar *lang = (const gchar*)lang_iter->data;
		
 		result = g_hash_table_lookup (per_language->hash_table,
					      lang);
		if (result) {
			return result;
		}
	}	

	result = g_hash_table_lookup (per_language->hash_table,
				      "");
	if (result) {
		return result;
	}

	return NULL;
}

gpointer
cong_per_language_get_data_for_lang (CongPerLanguageData *per_language,
				     const gchar *language)
{
	g_return_val_if_fail (per_language, NULL);

	return g_hash_table_lookup (per_language->hash_table,
				    language);

}

void
cong_per_language_set_data_for_lang (CongPerLanguageData *per_language,
				     const gchar *language,
				     gpointer data)
{
	g_return_if_fail (per_language);
	g_return_if_fail (language); /* must be non-NULL or the hash table code crashes */

	return g_hash_table_insert (per_language->hash_table,
				    g_strdup (language),
				    data);
}

void
cong_per_language_for_each (CongPerLanguageData *per_language,
			    void (*callback) (gpointer data, 
					      const gchar *language, 
					      gpointer user_data),
			    gpointer user_data)
{
	g_assert (0);
}

/* Create from the direct children of a parent node; any children that match the given element are used
   to populate the container, based on their xml:lang value */
CongPerLanguageData*
cong_per_language_data_new_from_xml (xmlDocPtr xml_doc, 
				     CongNodePtr parent_node,
				     const gchar *ns_uri,
				     const gchar *element_name,
				     gpointer (make_data_callback) (xmlDocPtr xml_doc, CongNodePtr node),
				     GDestroyNotify value_destroy_func)
{
	CongPerLanguageData *per_lang;
	CongNodePtr child_iter;

	g_return_val_if_fail (xml_doc, NULL);
	g_return_val_if_fail (parent_node, NULL);
	g_return_val_if_fail (element_name, NULL);
	g_return_val_if_fail (make_data_callback, NULL);	

	per_lang  = cong_per_language_data_new (value_destroy_func);

	for (child_iter=parent_node->children; child_iter; child_iter=child_iter->next) {
		if (cong_node_is_element (child_iter,
					  ns_uri,
					  element_name)) {			
			gpointer data = (*make_data_callback) (xml_doc, child_iter);
			xmlChar *lang = xmlGetNsProp (child_iter, BAD_CAST "lang", XML_XML_NAMESPACE);

			if (lang) {
				cong_per_language_set_data_for_lang (per_lang,
								     g_strdup (lang),
								     data);
				xmlFree (lang);
			} else {
				cong_per_language_set_data_for_lang (per_lang,
								     g_strdup (""),
								     data);
			}
		}
	}
	
	return per_lang;
}

void
cong_per_language_data_to_xml (CongPerLanguageData *per_language,
			       CongNodePtr parent_node,
			       const gchar *ns_uri,
			       const gchar *element_name,
			       CongNodePtr (make_node_callback) (gpointer data))
{
	g_assert (0);
}

