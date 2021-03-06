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
	GtkWidget *combo_box;
	GPtrArray *combo_array;
	GtkLabel *description;
	guint connection_id;
	GtkWidget *option_holder;
	GtkWidget *options;

	gboolean got_any_exporters;

} CongExportDialogDetails;

static CongServiceExporter* get_selected_exporter(CongExportDialogDetails *dialog_details)
{
	gint selected;
	g_return_val_if_fail (dialog_details, NULL);

	selected = gtk_combo_box_get_active( GTK_COMBO_BOX(dialog_details->combo_box) );

	/* can selected ever be -1 here? */
	if ( selected == -1 ) {
		return NULL;
	} else {
		return (CongServiceExporter*)g_ptr_array_index (dialog_details->combo_array, selected);
	}

}

static void add_exporter_to_menu(CongServiceExporter *exporter, gpointer user_data)
{
	CongExportDialogDetails *dialog_details = (CongExportDialogDetails*)user_data;

	if (cong_exporter_supports_document(exporter, dialog_details->doc)) {

		/* g_message( "adding exporter to menu - %s", cong_service_get_name(CONG_SERVICE(exporter)) ); */

		gtk_combo_box_append_text (GTK_COMBO_BOX (dialog_details->combo_box),
					   cong_service_get_name(CONG_SERVICE(exporter)) );

		g_ptr_array_add (dialog_details->combo_array, (gpointer) exporter);
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
		GFile *file = cong_exporter_get_preferred_location(exporter);
		char *uri = g_file_get_uri(file);
	
		gtk_entry_set_text(details->filename_entry, uri);
		
		g_free(uri);
		g_object_unref(file);
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
		GFile *file = cong_exporter_get_preferred_location(exporter);
		char *uri = g_file_get_uri(file);

		if (uri) {
			gtk_entry_set_text(dialog_details->filename_entry, uri);
			g_free(uri);
		} else {
			gtk_entry_set_text(dialog_details->filename_entry, "");
		}
		g_object_unref(file);
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

static void setup_options(CongExportDialogDetails *dialog_details)
{
	CongServiceExporter *exporter = get_selected_exporter(dialog_details);

	if (dialog_details->options) {
		gtk_container_remove (GTK_CONTAINER(dialog_details->option_holder),
				      dialog_details->options);
	}
	
	dialog_details->options = cong_exporter_make_options_widget (exporter, 
								     dialog_details->doc);
	
	if (dialog_details->options) {
		gtk_container_add (GTK_CONTAINER(dialog_details->option_holder),
				   dialog_details->options);
		gtk_widget_show (dialog_details->options);
	} else {
	}
}

static void on_exporter_selection_changed(GtkWidget *combo_box,
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

	setup_options(details);
}

static void on_select_filename_button_clicked(GtkButton *button,
					      gpointer user_data)
{
	GFile *export_file;
	GFile *new_file;
	CongExportDialogDetails *details = user_data;
	CongServiceExporter* exporter = get_selected_exporter(details);
	g_assert(exporter);

	/* 
	   FIXME: ought to set up MIME-filters according to the selected exporter plugin.
	   Needs a decent file-selector API for this...
	*/
	export_file = cong_exporter_get_preferred_location(exporter);
	
	new_file = cong_get_file_name(_("Select file to export to"),
				      export_file,
				      GTK_WINDOW(details->dialog),
				      CONG_FILE_CHOOSER_ACTION_SAVE,
				      NULL /* For now */);

	if (new_file) {
		cong_exporter_set_preferred_location(exporter, new_file);
		g_object_unref(new_file);
	}
	
	if (export_file) {
		g_object_unref(export_file);
	}
}

/*
 * The change to use a GtkComboBox rather than a GtkOptionMenu
 * means that a GPtrArray is used to store the exporter information,
 * rather than the old scheme of storing this information in the
 * option menu itself (or rather the GtkMenuItem). It means that
 * the CongExportDialogDetails structure now needs to be
 * explicitly cleaned up (in order to free up the GPtrArray).
 *
 * The use of the GtkComboBox probably does not preclude the old style,
 * it is just that I find this way easier.
 *
 * Doug.
 */

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
		dialog_details->combo_box = gtk_combo_box_new_text();
		dialog_details->combo_array = g_ptr_array_new ();

		cong_plugin_manager_for_each_exporter (cong_app_get_plugin_manager (cong_app_singleton()),
						       add_exporter_to_menu,
						       dialog_details);

		gtk_combo_box_set_active (GTK_COMBO_BOX(dialog_details->combo_box), 0);
		g_assert (dialog_details->got_any_exporters);
	}

	dialog_details->description = GTK_LABEL(gtk_label_new(NULL));
	gtk_label_set_line_wrap(dialog_details->description, TRUE);

	/* Set up filename entry widgetry: */
	{
		dialog_details->filename_entry = GTK_ENTRY(gtk_entry_new());
		gtk_editable_set_editable(GTK_EDITABLE(dialog_details->filename_entry), FALSE);

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

	cong_dialog_category_add_field(general_category, _("Exporter:"), GTK_WIDGET(dialog_details->combo_box), FALSE);
	cong_dialog_category_add_field(general_category, "", GTK_WIDGET(dialog_details->description), FALSE);
	cong_dialog_category_add_field(general_category, _("File:"), hbox, TRUE);

	exporter_category = cong_dialog_content_add_category(content, _("Export Options"));

	dialog_details->option_holder = gtk_vbox_new (TRUE, 0);
	cong_dialog_category_add_selflabelled_field (exporter_category, 
						     dialog_details->option_holder,
						     TRUE);

	monitor_exporter(dialog_details);
	setup_description(dialog_details);
	setup_options(dialog_details);

	g_signal_connect(dialog_details->combo_box,
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

static void
cong_document_export_dialog_delete (GtkWidget *dialog)
{
	CongExportDialogDetails *dialog_details;
	g_assert (dialog);

	dialog_details = g_object_get_data (G_OBJECT(dialog),
					    "dialog_details");
	g_assert (dialog_details);

	if (dialog_details->combo_array)
		g_ptr_array_free (dialog_details->combo_array, FALSE);

	gtk_widget_destroy (dialog);
	g_free (dialog_details);
}

/**
 * cong_ui_hook_file_export:
 * @doc:
 * @toplevel_window:
 *
 * Convert the document (@doc) to a different format. A dialog is
 * displayed that lists all the registered exporters for this document
 * type. If the user does not cancel then we check to see if a
 * filename was given; if it was not the user is returned to the
 * dialog, otherwise the chosen exporter is called.
 *
 * Open issues include:
 * how to inform the user of what is going on, and
 * whether we allow the user to stop an exporter whilst it is
 * processing.
 */
void
cong_ui_hook_file_export (CongDocument *doc,
			  GtkWindow *toplevel_window)
{
	GtkWidget *dialog;
	CongExportDialogDetails *dialog_details;
	CongServiceExporter* exporter = NULL;
	GFile *export_file = NULL;
	gint flag = 1;

	g_return_if_fail(doc);

	dialog = cong_document_export_dialog_new(doc,
						 toplevel_window);

	dialog_details = g_object_get_data( G_OBJECT(dialog),
					    "dialog_details");
	g_assert(dialog_details);

	gtk_widget_show_all (dialog);

	/*
	 * Outcome of the dialog:
	 *    - cancelled
	 *    - user selected ok, filename selected
	 *    - user selected ok, no filename selected
	 *
	 * For the last case we want to throw up a dialog saying a filename is
	 * needed and re-display the dialog, otherwise we end the loop.
	 */
	while (flag==1)
	{
		gint result = gtk_dialog_run (GTK_DIALOG(dialog));
		flag = 0;

		if (result==GTK_RESPONSE_OK) {
			exporter = get_selected_exporter (dialog_details);
			g_assert (exporter);

			export_file = cong_exporter_get_preferred_location (exporter);
			if (export_file==NULL) {
				GtkDialog* error_dialog;
				error_dialog = cong_error_dialog_new (toplevel_window,
								      _("No output file specified"), 
								      _("Please specify a file name."),
								      "");
				gtk_dialog_run (error_dialog);
				gtk_widget_destroy (GTK_WIDGET(error_dialog));
				flag = 1;
				exporter = NULL; /* in case the user decides to cancel the dialog */
			}
		}
	}

	/* FIXME: Somewhat hackish cleanup: */
	gconf_client_notify_remove(cong_app_get_gconf_client(cong_app_singleton()),
				   dialog_details->connection_id);
	cong_document_export_dialog_delete (dialog);

	/*
	 * We use the exporter variable to determine whether we continue with the processing.
	 * Note that we destroy the dialog BEFORE we do this check, so that we do not
	 * get a "hung" dialog window. The exporter should make sure that the busy cursor
	 * is set in the top-level Conglomerate windows to tell the user that something
	 * is happening. This probably needs some "UI love" to ensure it is usable.
	 *
	 * (due to the current design of cong_ui_transform_doc() in cong-plugins.c we
         *  would not get the "busy" cursor in the dialog window if it were open when
         *  we call the exporter)
	 */
	if (exporter) {
		g_assert (export_file);

		char *export_uri = g_file_get_uri(export_file);
		g_message("Exporter invoked: \"%s\" to \"%s\"",
			  cong_service_get_name(CONG_SERVICE(exporter)),
			  export_uri);
		g_free(export_uri);

		cong_exporter_invoke (exporter, 
				      doc, 
				      export_file,
				      toplevel_window);
		g_object_unref(export_file);
	}

}
