/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-file-export.c
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
#include "cong-eel.h"
#include "cong-app.h"
#include "cong-file-selection.h"
#include "cong-service-exporter.h"
#include "cong-plugin-manager.h"

typedef struct CongExportDialogDetails
{
	CongDocument *doc;

	GtkDialog *dialog;
	GtkEntry *filename_entry;
	GtkWidget *select_exporter_menu;
	GtkOptionMenu *select_exporter_option_menu;
	GtkLabel *description;
	guint connection_id;

	gboolean got_any_exporters;

} CongExportDialogDetails;

static CongServiceExporter* get_selected_exporter(CongExportDialogDetails *dialog_details)
{
	GtkMenuItem* selected_menu_item;

	g_return_val_if_fail(dialog_details, NULL);

	/* Which plugin has been selected? */
	selected_menu_item = cong_eel_option_menu_get_selected_menu_item (dialog_details->select_exporter_option_menu);
			
	if (selected_menu_item) {
		return (CongServiceExporter*)g_object_get_data(G_OBJECT(selected_menu_item),
							"exporter");
	} else {
		return NULL;
	}
}

static void add_exporter_to_menu(CongServiceExporter *exporter, gpointer user_data)
{
	CongExportDialogDetails *dialog_details = (CongExportDialogDetails*)user_data;

	if (cong_exporter_supports_document(exporter, dialog_details->doc)) {

		GtkWidget *menu = dialog_details->select_exporter_menu;

		GtkMenuItem *menu_item = GTK_MENU_ITEM(gtk_menu_item_new_with_label( cong_service_get_name(CONG_SERVICE(exporter))));

		gtk_menu_shell_append(GTK_MENU_SHELL(menu),
				      GTK_WIDGET(menu_item));
		
		g_object_set_data(G_OBJECT(menu_item),
				  "exporter",
				  exporter);

		dialog_details->got_any_exporters = TRUE;
	}
}

static void gconf_notify_func(GConfClient *client,
			      guint connection_id,
			      GConfEntry *entry,
			      gpointer user_data)
{
	CongExportDialogDetails *details = user_data;
	CongServiceExporter* exporter = get_selected_exporter(details);

	g_message("gconf_notify_function");

	if (exporter) {
		gchar *uri = cong_exporter_get_preferred_uri(exporter);
	
		gtk_entry_set_text(details->filename_entry, uri);
		
		g_free(uri);
	}
}

static void monitor_exporter(CongExportDialogDetails *dialog_details)
{
	CongServiceExporter* exporter = get_selected_exporter(dialog_details);
	g_assert(dialog_details);
	g_assert(exporter);

	/* start monitoring GConf for selected plugin */
	{
		gchar *exporter_namespace  = cong_service_get_gconf_namespace(CONG_SERVICE(exporter));

		/* g_message("adding notification for \"%s\"", exporter_namespace); */
		dialog_details->connection_id = gconf_client_notify_add(cong_app_get_gconf_client (cong_app_singleton()),
									exporter_namespace,
									gconf_notify_func,
									dialog_details,
									NULL,
									NULL);
		g_free(exporter_namespace);
	}

	/* get value and set up entry accordingly */
	{
		gchar *uri = cong_exporter_get_preferred_uri(exporter);

		if (uri) {
			gtk_entry_set_text(dialog_details->filename_entry, uri);
			g_free(uri);
		} else {
			gtk_entry_set_text(dialog_details->filename_entry, "");
		}
	}
}

static void setup_description(CongExportDialogDetails *dialog_details)
{
	CongServiceExporter* exporter = get_selected_exporter(dialog_details);
	const gchar *desc;
	gchar * text;

	g_assert(dialog_details);
	g_assert(exporter);

	desc = cong_service_get_description(CONG_SERVICE(exporter));

	text = g_strdup_printf("<small>%s</small>", 
                                       (desc ? desc : _("(No description available)")));
	gtk_label_set_markup(dialog_details->description, text);
	g_free(text);
}

static void on_exporter_selection_changed(GtkOptionMenu *optionmenu,
					  gpointer user_data)
{
	CongExportDialogDetails *details = user_data;

	g_message("on_exporter_selection_changed");

	/* Stop monitoring GConf for old plugin: */
	gconf_client_notify_remove(cong_app_get_gconf_client (cong_app_singleton()),
				   details->connection_id);

	/* Monitor new plugin; set up stuff accordingly: */
	monitor_exporter(details);

	setup_description(details);
}

static void on_select_filename_button_clicked(GtkButton *button,
					      gpointer user_data)
{
	gchar *export_uri;
	gchar *new_uri;
	CongExportDialogDetails *details = user_data;
	CongServiceExporter* exporter = get_selected_exporter(details);
	g_assert(exporter);

	/* 
	   FIXME: ought to set up MIME-filters according to the selected exporter plugin.
	   Needs a decent file-selector API for this...
	*/
	export_uri = cong_exporter_get_preferred_uri(exporter);
	
	new_uri = cong_get_file_name(_("Select file to export to"),
				     export_uri,
				     GTK_WINDOW(details->dialog),
				     CONG_FILE_CHOOSER_ACTION_SAVE,
				     NULL /* For now */);

	if (new_uri) {
		cong_exporter_set_preferred_uri(exporter, new_uri);
		g_free(new_uri);
	}
	
	if (export_uri) {
		g_free(export_uri);
	}
}

static GtkWidget *cong_document_export_dialog_new(CongDocument *doc, 
						  GtkWindow *parent_window)
{
	xmlDocPtr xml_doc;
	GtkWidget *dialog;
	CongDialogContent *content;
	CongDialogCategory *general_category;
	CongDialogCategory *exporter_category;
	gchar *filename, *title;
	GtkWidget *hbox, *select_filename_button;
	CongExportDialogDetails *dialog_details;
	

	g_return_val_if_fail(doc, NULL);

	xml_doc = cong_document_get_xml(doc);

	filename = cong_document_get_filename(doc);

	title = g_strdup_printf(_("Export \"%s\""), filename);

	dialog_details = g_new0(CongExportDialogDetails,1);
	dialog_details->doc = doc;

	dialog = gtk_dialog_new_with_buttons(_("Export"),
					     parent_window,
					     0,
					     GTK_STOCK_CANCEL,
					     GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	dialog_details->dialog = GTK_DIALOG(dialog);

	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	gtk_container_set_border_width(GTK_CONTAINER(dialog), 12);

	content = cong_dialog_content_new(FALSE);
	general_category = cong_dialog_content_add_category(content, _("General"));

	/* Set up exporter selection option menu: */
	{
		dialog_details->select_exporter_option_menu = GTK_OPTION_MENU(gtk_option_menu_new());
		dialog_details->select_exporter_menu = gtk_menu_new();
		gtk_option_menu_set_menu(dialog_details->select_exporter_option_menu,
					 dialog_details->select_exporter_menu);
		
		cong_plugin_manager_for_each_exporter(cong_app_get_plugin_manager (cong_app_singleton()), add_exporter_to_menu, dialog_details);

		gtk_option_menu_set_history(dialog_details->select_exporter_option_menu,0);
	}

	dialog_details->description = GTK_LABEL(gtk_label_new(NULL));
	gtk_label_set_line_wrap(dialog_details->description, TRUE);

	/* Set up filename entry widgetry: */
	{
		dialog_details->filename_entry = GTK_ENTRY(gtk_entry_new());
		gtk_entry_set_editable(dialog_details->filename_entry, FALSE);		

		select_filename_button = gtk_button_new_from_stock(GTK_STOCK_SAVE_AS);

		g_signal_connect(G_OBJECT(select_filename_button),
				 "clicked",
				 G_CALLBACK(on_select_filename_button_clicked),
				 dialog_details
				 );
		
		hbox = gtk_hbox_new(FALSE, 6);
		gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(dialog_details->filename_entry), TRUE, TRUE,0);
		gtk_box_pack_start(GTK_BOX(hbox), select_filename_button, FALSE, FALSE,0);
	}

	cong_dialog_category_add_field(general_category, _("Exporter:"), GTK_WIDGET(dialog_details->select_exporter_option_menu), FALSE);
	cong_dialog_category_add_field(general_category, "", GTK_WIDGET(dialog_details->description), FALSE);
	cong_dialog_category_add_field(general_category, _("File:"), hbox, TRUE);

	if (dialog_details->got_any_exporters) {
		monitor_exporter(dialog_details);
		setup_description(dialog_details);
	}

	exporter_category = cong_dialog_content_add_category(content, _("Export Options"));

	g_signal_connect(dialog_details->select_exporter_option_menu,
			 "changed",
			 G_CALLBACK(on_exporter_selection_changed),
			 dialog_details);

#if 0
	cong_dialog_category_add_field(exporter_category, _("Name"), make_uneditable_text(cong_dispspec_get_name(ds)));
	cong_dialog_category_add_field(exporter_category, _("Description"), make_uneditable_text(cong_dispspec_get_description(ds)));
#endif

	g_free(title);
	g_free(filename);

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   cong_dialog_content_get_widget(content));

	g_object_set_data(G_OBJECT(dialog),
			  "dialog_details",
			  dialog_details);

	return dialog;
}

/**
 * cong_ui_hook_file_export:
 * @doc:
 * @toplevel_window:
 *
 * TODO: Write me
 */
void
cong_ui_hook_file_export (CongDocument *doc,
			  GtkWindow *toplevel_window)
{
	GtkWidget *dialog;
	gint result;
	CongExportDialogDetails *dialog_details;

	g_return_if_fail(doc);

	dialog = cong_document_export_dialog_new(doc,
						 toplevel_window);

	dialog_details = g_object_get_data( G_OBJECT(dialog),
					    "dialog_details");
	g_assert(dialog_details);

	if (dialog_details->got_any_exporters) {
		gtk_widget_show_all(dialog);

		result = gtk_dialog_run(GTK_DIALOG(dialog));


		switch (result) {
		default: /* Do nothing; dialog was cancelled */
			break;
			
		case GTK_RESPONSE_OK:
			{
				CongServiceExporter* exporter;
				
				/* Which plugin has been selected? */
				exporter = get_selected_exporter(dialog_details);
				if (exporter) {
					gchar *export_uri = cong_exporter_get_preferred_uri(exporter);
					g_message("Exporter invoked: \"%s\" to \"%s\"", cong_service_get_name(CONG_SERVICE(exporter)), export_uri);
					
					cong_exporter_invoke(exporter, 
							     doc, 
							     export_uri,
							     toplevel_window);
					
					g_free(export_uri);
					
				}
			}
			break;
		}

		/* FIXME: Somewhat hackish cleanup: */
		gconf_client_notify_remove(cong_app_get_gconf_client(cong_app_singleton()),
					   dialog_details->connection_id);
	} else {
		/* There are no plugins which can handle this document: */
		gchar *filename = cong_document_get_filename(doc);
		gchar *what_failed = g_strdup_printf(_("Conglomerate cannot export \"%s\""), filename);
		GtkDialog* error_dialog = cong_error_dialog_new(toplevel_window,
								what_failed, 
								_("None of Conglomerate's plugins know how to export files of that type."),
								"");

		gtk_dialog_run(error_dialog);
		gtk_widget_destroy(GTK_WIDGET(error_dialog));
	}

	/* FIXME: Somewhat hackish cleanup: */
	gtk_widget_destroy(dialog);

	g_free(dialog_details);
}
