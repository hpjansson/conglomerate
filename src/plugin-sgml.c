/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-sgml.c
 *
 * Plugin for SGML import.
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
#include "cong-parser-error.h"

#include "cong-fake-plugin-hooks.h"

gchar *cong_ui_make_what_failed_string_for_import(const gchar *uri_string)
{
	gchar *return_val = g_strdup_printf(_("Conglomerate could not import the file \"%s\""), uri_string);

	return return_val;
}

gboolean sgml_importer_mime_filter(CongServiceImporter *importer, const gchar *mime_type, gpointer user_data)
{
	g_return_val_if_fail(importer, FALSE);
	g_return_val_if_fail(mime_type, FALSE);

	if (0==strcmp(mime_type,"text/sgml")) {
		return TRUE;
	} else {
		return FALSE;
	}
}

#if 0
static gboolean on_stdout(GIOChannel *source,
			  GIOCondition condition,
			  gpointer data)
{
	g_message("on_stdout");

	return TRUE;
}

static gboolean on_stderr(GIOChannel *source,
			  GIOCondition condition,
			  gpointer data)
{
	g_message("on_stderr");

	return TRUE;
}
#endif

void sgml_importer_action_callback(CongServiceImporter *importer, const gchar *uri, const gchar *mime_type, gpointer user_data, GtkWindow *toplevel_window)
{
#if 0
	char* buffer;
	GnomeVFSFileSize size;
#endif
	xmlDocPtr xml_doc;

#if 1
	gchar *argv[3];
	gchar *standard_output;
	gchar *standard_error;
	gint exit_status;
	GError *error = NULL; 
	gboolean result;

/* 	gchar * posix_name = cong_util_get_local_path_from_uri(GnomeVFSURI *uri); */
 

	g_message("sgml_importer_action_callback");

	argv[0] = "sgml2xml";
	argv[1] = (gchar*)uri; /* FIXME: this is GnomeVFS uri path string, not a POSIX path */
	argv[2] = NULL;


	/* Spawn a subprocess that runs sgml2xml on the uri and grabs the output: */
	result = g_spawn_sync(NULL,
			      argv,
			      NULL,
			      G_SPAWN_SEARCH_PATH,
			      NULL,
			      NULL,
			      &standard_output,
			      &standard_error,
			      &exit_status,
			      &error);

	if (!result) {
		gchar *what_failed = cong_ui_make_what_failed_string_for_import(uri);
		GtkDialog *dialog = cong_error_dialog_new_from_gerror(toplevel_window,
								      what_failed,
								      _("Attempting to run the sgml2xml tool"),
								      error);
							   
		g_free(what_failed);

		cong_error_dialog_run(dialog);
		gtk_widget_destroy(GTK_WIDGET(dialog));

		return;		
	}

	g_assert(standard_output);
	g_assert(standard_error);

	if (exit_status) {
		gchar *what_failed = cong_ui_make_what_failed_string_for_import(uri);


		GtkDialog* dialog = cong_error_dialog_new_from_shell_command_failure_with_argv(toplevel_window,
											       what_failed,
											       exit_status,
											       standard_error,
											       (const gchar**)argv);
		g_free(what_failed);

		cong_error_dialog_run(dialog);
		gtk_widget_destroy(GTK_WIDGET(dialog));

		g_free(standard_output);
		g_free(standard_error);

		return;		
	}

	g_message(standard_error);

	/* Attempt to parse the stdout into an xmlDocPtr: */
	{
		xml_doc = cong_ui_parse_buffer (standard_output, 
						strlen(standard_output), 
						uri, 
						toplevel_window);
	}

	g_free(standard_output);
	g_free(standard_error);

	if (xml_doc) {
		/* Attempt to create a UI window for the new document: */
		cong_ui_new_document_from_imported_xml(xml_doc,
						       toplevel_window);
	}

#else

	CONG_DO_UNIMPLEMENTED_DIALOG_WITH_BUGZILLA_ID(toplevel_window, _("Importing SGML"), 108465);
#endif
}


 /* would be exposed as "plugin_register"? */
gboolean plugin_sgml_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);
	
	cong_plugin_register_importer(plugin, 
				      _("Import SGML"), 
				      _("Import an SGML file, converting to XML."),
				      "sgml-import",
				      sgml_importer_mime_filter,
				      sgml_importer_action_callback,
				      NULL);
	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean plugin_sgml_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
