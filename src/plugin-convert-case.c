/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-convert-case.c
 *
 * Plugin to export with lowercase tags.
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

#include "global.h"
#include "cong-plugin.h"
#include "cong-error-dialog.h"
#include "cong-document.h"
#include "cong-vfs.h"

#include "cong-fake-plugin-hooks.h"

static gboolean convert_case_exporter_document_filter(CongServiceExporter *exporter, CongDocument *doc, gpointer user_data)
{
	g_return_val_if_fail(exporter, FALSE);
	g_return_val_if_fail(doc, FALSE);

	return FALSE;
}

static void visit_node(xmlNodePtr node) {
	xmlNodePtr child;

	/* Converts tags and attributes to lower case: */
	if (node->type==XML_ELEMENT_NODE || node->type==XML_ATTRIBUTE_NODE) {
		if (node->name) {
			gchar *new_name = g_utf8_strdown (node->name, -1);

			g_message("Converting <%s> to <%s>", node->name, new_name);
			xmlFree ((char*)node->name);
			node->name = new_name;
		}
	}

	for (child=node->children;child;child=child->next) {
		visit_node(child);
	}
}

static void convert_case_exporter_action_callback(CongServiceExporter *exporter, CongDocument *doc, const gchar *uri, gpointer user_data, GtkWindow *toplevel_window)
{
	xmlDocPtr doc_ptr;

	g_return_if_fail(exporter);
	g_return_if_fail(doc);
	g_return_if_fail(uri);

	g_message("convert_case_exporter_action_callback");

	/* Clone the xml_doc, then traverse it, recursively doing case-conversions: */
	doc_ptr = xmlCopyDoc(cong_document_get_xml(doc), TRUE);
	g_assert(doc_ptr);

	/* Action goes here; this could be refactored: */
	{
		xmlNodePtr node;

		for (node=doc_ptr->children;node;node=node->next) {
			visit_node(node);
		}
	}

	/* Save the result: */
	if (doc_ptr) {
		GnomeVFSURI *file_uri = gnome_vfs_uri_new(uri);
		GnomeVFSResult vfs_result;
		GnomeVFSFileSize file_size;
	
		vfs_result = cong_vfs_save_xml_to_uri (doc_ptr, 
						       file_uri,	
						       &file_size);
		
		if (vfs_result != GNOME_VFS_OK) {
			GtkDialog* dialog = cong_error_dialog_new_from_file_save_failure(toplevel_window,
											 uri, 
											 vfs_result, 
											 &file_size);
			
			cong_error_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(GTK_WIDGET(dialog));
		}
		
		gnome_vfs_uri_unref(file_uri);

		xmlFreeDoc(doc_ptr);
	}
}

 /* would be exposed as "plugin_register"? */
gboolean plugin_convert_case_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);
	
	cong_plugin_register_exporter(plugin, 
				      _("Export as \"lower case\" XML"),
				      _("Exports as an XML file, converting all the tags and attributes in the document to lowercase.  Useful when working with legacy SGML documents."),
				      "convert-case-export",
				      convert_case_exporter_document_filter,
				      convert_case_exporter_action_callback,
				      NULL);
	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean plugin_convert_case_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
