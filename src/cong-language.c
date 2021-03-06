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
#include "cong-util.h"

CongPerLanguageData*
cong_per_language_data_new (GDestroyNotify value_destroy_func)
{
	CongPerLanguageData *result;

	result = g_new0 (CongPerLanguageData, 1);

	result->hash_table = g_hash_table_new_full (cong_str_or_null_hash,
						    cong_str_or_null_equal,
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
	const char * const *lang;
	gpointer result;

	g_return_val_if_fail (per_language, NULL);

	for (lang = g_get_language_names(); *lang; lang++) {
		result = g_hash_table_lookup (per_language->hash_table,
					      *lang);
		if (result) {
			return result;
		}
	}	

	result = g_hash_table_lookup (per_language->hash_table,
				      NULL);
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
								     g_strdup ((const gchar*)lang),
								     data);
				xmlFree (lang);
			} else {
				cong_per_language_set_data_for_lang (per_lang,
								     NULL,
								     data);
			}
		}
	}
	
	return per_lang;
}

struct data_to_xml_data
{
	CongNodePtr parent_node;
	CongNodePtr (*make_node_callback) (xmlDocPtr xml_doc,
					   gpointer data);
};

static void
data_to_xml_cb (gpointer key,
		gpointer value,
		gpointer user_data)
{
	struct data_to_xml_data *cb_data = user_data;
	CongNodePtr new_node;

	g_assert (cb_data->parent_node);
	g_assert (cb_data->make_node_callback);

	new_node = cb_data->make_node_callback (cb_data->parent_node->doc,
						value);

	xmlAddChild (cb_data->parent_node, 
		     new_node);

	if (key) {
		xmlNsPtr ns;

		ns = xmlSearchNsByHref (cb_data->parent_node->doc, 
					new_node, 
					XML_XML_NAMESPACE);
		if (ns == NULL) {
			g_warning ("FIXME couldn't find namespace %s ", XML_XML_NAMESPACE);
		}
		xmlSetNsProp (new_node,
			      ns,
			      (const xmlChar*)"lang", 
			      key);
	}

}

void
cong_per_language_data_to_xml (CongPerLanguageData *per_language,
			       CongNodePtr parent_node,
			       CongNodePtr (make_node_callback) (xmlDocPtr xml_doc,
								 gpointer data))
{
	struct data_to_xml_data cb_data;
	g_return_if_fail (per_language);
	g_return_if_fail (parent_node);
	
	cb_data.parent_node = parent_node;
	cb_data.make_node_callback = make_node_callback;

	g_hash_table_foreach (per_language->hash_table,
			      data_to_xml_cb,
			      &cb_data);
}

