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
#include "cong-app.h"
#include "cong-eel.h"
#include <glade/glade-xml.h>
#include "cong-vfs.h"
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#include "cong-file-selection.h"
#include "cong-service-importer.h"
#include "cong-plugin-manager.h"
#include <libgnomevfs/gnome-vfs-mime-handlers.h>

CongServiceImporter*
cong_file_import_dialog_run (GtkWindow *toplevel_window,
			     const gchar *filename,
			     const gchar* mime_type,
			     GList *list_of_valid_importers);

struct add_importer_to_list_data
{
	const gchar *mime_type;
	GList **list_head;
};

static void add_importer_to_list(CongServiceImporter *importer, gpointer user_data)
{
	struct add_importer_to_list_data *data = user_data;
	if (cong_importer_supports_mime_type(importer, data->mime_type) ) {
		*data->list_head = g_list_append(*data->list_head, importer);
	}
}

void
cong_ui_hook_file_import (GtkWindow *toplevel_window)
{
	gchar *filename;

	filename = cong_get_file_name(_("Import file..."), 
				      NULL, 
				      toplevel_window,
				      CONG_FILE_CHOOSER_ACTION_OPEN);

	if (filename) {
		char* mime_type = gnome_vfs_get_mime_type (filename);
		GList *list_of_valid = NULL; 
		struct add_importer_to_list_data data;

		g_message("Got mimetype: \"%s\"",mime_type);

		data.mime_type = mime_type;
		data.list_head = &list_of_valid;


		/* Construct a list of importers that can handle this mimetype: */
		cong_plugin_manager_for_each_importer( cong_app_get_plugin_manager (cong_app_singleton()), 
						       add_importer_to_list, 
						       &data);

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

			g_free (mime_type);			
			g_free (filename);			
			return;
		}

		g_assert(list_of_valid);

		if (list_of_valid->next) {
			/* There's more than one valid importer... */
			CongServiceImporter *importer= cong_file_import_dialog_run (toplevel_window,
									     filename,
									     mime_type,
									     list_of_valid);
			if (importer) {
				cong_importer_invoke (importer, 
						      filename, 
						      mime_type, 
						      toplevel_window);
			}
			
		} else {
			CongServiceImporter *importer = list_of_valid->data;
			g_assert(importer);

			cong_importer_invoke (importer, 
					      filename, 
					      mime_type, 
					      toplevel_window);
		}

		g_list_free(list_of_valid);
		g_free (mime_type);
		g_free(filename);
	}

}

CongServiceImporter*
cong_file_import_dialog_run (GtkWindow *toplevel_window,
			     const gchar *filename,
			     const gchar* mime_type,
			     GList *list_of_valid_importers)
{
	CongServiceImporter* result = NULL;
	gchar* glade_filename;
	GladeXML *xml;
	GtkOptionMenu *select_importer;
	GtkDialog *dialog;

	glade_filename = gnome_program_locate_file( cong_app_get_gnome_program (cong_app_singleton()),
						    GNOME_FILE_DOMAIN_APP_DATADIR,
						    "glade/cong-file-import.glade",
						    FALSE,
						    NULL);
	
	xml = glade_xml_new(glade_filename, NULL, NULL);
	glade_xml_signal_autoconnect(xml);

	g_free(glade_filename);

	select_importer = GTK_OPTION_MENU(glade_xml_get_widget(xml, "optionmenu_select_importer"));

	g_object_ref (select_importer);

	/* Populate the option menu: */
	{
		GList *iter;
		GtkMenu *menu;

		menu = GTK_MENU (gtk_menu_new ());

		for (iter=list_of_valid_importers; iter; iter=iter->next) {
			CongServiceImporter *importer = iter->data;
			GtkWidget *menu_item;

			g_assert (importer);

			menu_item = gtk_menu_item_new_with_label (cong_service_get_name (CONG_SERVICE (importer)));
			
			gtk_widget_show (menu_item);

			g_object_set_data (G_OBJECT(menu_item),
					   "importer",
					   importer);
			
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), 
					      menu_item);
		}

		gtk_option_menu_set_menu (select_importer,
					  GTK_WIDGET (menu));

		gtk_widget_show (GTK_WIDGET(menu));
	}	

	dialog = GTK_DIALOG(glade_xml_get_widget(xml, "file_import_dialog"));

	switch (gtk_dialog_run (dialog)) {
	default: /* do nothing - closed by window manager? */
	case GTK_RESPONSE_NONE:
	case GTK_RESPONSE_CANCEL: 
		/* do nothing */
		g_message("import cancelled");
		break;
		
	case GTK_RESPONSE_OK:
		{
			GtkMenuItem* menu_item = cong_eel_option_menu_get_selected_menu_item (select_importer);

			g_assert (menu_item);

			result = g_object_get_data (G_OBJECT (menu_item),
						    "importer");
		}
		break;
	}

	gtk_widget_destroy (GTK_WIDGET (dialog));
	g_object_unref (G_OBJECT (xml));

	return result;
}
