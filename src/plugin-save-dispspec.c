/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-save-dispec.c
 *
 * Serializes a display spec to xml
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
 * Authors: Jeff Martin <jeff@custommonkey.org>
 */

#include "global.h"
#include "cong-plugin.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-app.h"
#include "cong-primary-window.h"
#include "cong-file-selection.h"

static gboolean doc_filter(CongServiceDocTool *tool, CongDocument *doc, gpointer user_data)
{
	/* Always appropriate: */
	return TRUE;
}

gchar* change_to_xds(gchar* filename)
{
	gchar **split_strings;
	gchar *old_doc_name;
	gchar *result;

	split_strings = g_strsplit (filename, ".", 2);
	
	old_doc_name = split_strings[0];

	if (old_doc_name) {
		result = g_strconcat (old_doc_name, ".xds", NULL);
	} else {
		result = g_strconcat (filename, ".xds", NULL);
	}
	
	g_strfreev (split_strings);

	return result;
}

static void save_dispspec(CongServiceDocTool *tool, CongPrimaryWindow *primary_window, gpointer user_data)
{
	gchar *new_doc_name;
	gchar *old_doc_name;
	CongDocument *doc;
	xmlDocPtr xml;

	doc = cong_primary_window_get_document(primary_window);

	old_doc_name = change_to_xds (cong_document_get_filename(doc));

	new_doc_name = cong_get_file_name("Save Display Specification",
					  old_doc_name, 
					  cong_primary_window_get_toplevel (primary_window),
					  CONG_FILE_CHOOSER_ACTION_SAVE);

	if (!new_doc_name) {
		g_free (old_doc_name);
		return;
	}

	xml = cong_dispspec_make_xml(cong_document_get_dispspec(doc));
	xmlCreateIntSubset(xml, "dispspec", NULL, "dispspec.dtd");
	xmlSaveFormatFile(new_doc_name, xml, TRUE);

	/* FIXME: does this leak xml? */

	g_free(new_doc_name);
	g_free(old_doc_name);

}

static void edit_dispspec(CongServiceDocTool *tool, CongPrimaryWindow *primary_window, gpointer user_data)
{
	CongDocument *doc;
	xmlDocPtr xml;
#if 0
	CongDispspec *dispspec;
	gchar *filename;
#endif

	doc = cong_primary_window_get_document(primary_window);

	xml = cong_dispspec_make_xml(cong_document_get_dispspec(doc));
	xmlCreateIntSubset(xml, "dispspec", NULL, "dispspec.dtd");
#if 1
	cong_ui_new_document_from_imported_xml(xml,
					       cong_primary_window_get_toplevel(primary_window));
#else
	dispspec = cong_dispspec_registry_get_appropriate_dispspec(
			cong_app_singleton()->ds_registry, xml);

	filename = change_to_xds (cong_document_get_filename(doc));

	doc = cong_document_new_from_xmldoc (xml, dispspec, filename);

	cong_primary_window_new(doc);

	g_free(filename);
#endif

}



 /* would be exposed as "plugin_register"? */
gboolean plugin_save_dispspec_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	cong_plugin_register_doc_tool(plugin, 
				      _("Dump Display Spec"),
				      _("Writes the current display spec into a display spec xml file. This file can then be customized by the conglomerate user"),
				      "save_dispspec",
				      _("_Dump Display Spec"),
				      _("Writes the current display spec into a display spec xml file. This file can then be customized by the conglomerate user"),
				      _("Writes the current display spec into a display spec xml file. This file can then be customized by the conglomerate user"),
				      doc_filter,
				      save_dispspec,
				      NULL);
	
	cong_plugin_register_doc_tool(plugin, 
				      _("Edit Display Spec"),
				      _("Opens the current display spec as a conglomerate document."),
				      "edit_dispspec",
				      _("_Edit Display Spec"),
				      _("Opens the current display spec as a conglomerate document."),
				      _("Opens the current display spec as a conglomerate document."),
				      doc_filter,
				      edit_dispspec,
				      NULL);
	

	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean plugin_save_dispspec_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
