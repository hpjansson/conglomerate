/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-file-import.c
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
 * Based on code by Hans Petter Jansson <hpj@ximian.com>
 */

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "global.h"
#include "../config.h"
#include "cong-dispspec.h"
#include "cong-document.h"
#include "cong-error-dialog.h"
#include "cong-dialog.h"
#include "cong-plugin.h"
#include "cong-app.h"

struct add_importer_to_list_data
{
	const gchar *mime_type;
	GList **list_head;
};

static void add_importer_to_list(CongImporter *importer, gpointer user_data)
{
	struct add_importer_to_list_data *data = user_data;
	if (cong_importer_supports_mime_type(importer, data->mime_type) ) {
		*data->list_head = g_list_append(*data->list_head, importer);
	}
}

void
cong_ui_file_import(GtkWindow *toplevel_window)
{
	gchar *filename;

	filename = cong_get_file_name(_("Import file..."), 
				      NULL, 
				      toplevel_window);

	if (filename) {
		const char* mime_type = gnome_vfs_mime_type_from_name(filename);
		GList *list_of_valid = NULL; 
		struct add_importer_to_list_data data;
		CongImporter *importer = NULL;

		g_message("Got mimetype: \"%s\"",mime_type);

		data.mime_type = mime_type;
		data.list_head = &list_of_valid;


		/* Construct a list of importers that can handle this mimetype: */
		cong_plugin_manager_for_each_importer(cong_app_singleton()->plugin_manager, add_importer_to_list, &data);

		/* OK:  there are three cases:
		   (i) if no importers can handle this mimetype; then tell the user and give them the option of cancelling or forcing the use of a plugin (with a dialog to choose)
		   (ii) if exactly one importer can handle the mimetype, then use it
		   (iii) if more than one importer can handle it, then present the user with a choice dialog.
		*/
		if (NULL==list_of_valid) {
			GtkDialog* dialog;
			gchar *what_failed;
			gchar *why_failed;
			gchar *suggestions;

			/* FIXME:  eventually provide a convenience dialog to force things */

			what_failed = g_strdup_printf(_("Could not import the file."));
			why_failed = g_strdup_printf(_("None of Conglomerate's plugins know how to load files of type \"%s\""), gnome_vfs_mime_get_description(mime_type));
			suggestions = g_strdup_printf(_("FIXME"));
			dialog = cong_error_dialog_new(toplevel_window,
						       what_failed,
						       why_failed,
						       suggestions);
			cong_error_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(GTK_WIDGET(dialog));
			
			g_list_free(list_of_valid);

			g_free(filename);			
			return;
		}

		g_assert(list_of_valid);

		if (list_of_valid->next) {
			/* There's more than one valid importer... */
			CONG_DO_UNIMPLEMENTED_DIALOG(toplevel_window, _("More than one importer can handle that file type; the selection dialog has yet to be implemented.  You will have to use the first one that the plugin manager found."));

			importer = list_of_valid->data;
		} else {
			importer = list_of_valid->data;
		}

		g_list_free(list_of_valid);

		g_assert(importer);
		
		cong_importer_invoke(importer, filename, mime_type, toplevel_window);

		g_free(filename);
	}

}

