/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-file-print.c
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
#include "cong-service-print-method.h"
#include "cong-plugin-manager.h"

#if ENABLE_PRINTING
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprintui/gnome-print-job-preview.h>

typedef struct CongPrintDialogDetails
{
	CongDocument *doc;
	gboolean got_any_print_methods;

	GtkDialog *dialog;
	GtkOptionMenu *select_print_method_option_menu;
	GtkWidget *select_print_method_menu;
} CongPrintDialogDetails;

static CongServicePrintMethod* get_selected_print_method(CongPrintDialogDetails *dialog_details)
{
	GtkMenuItem* selected_menu_item;

	g_return_val_if_fail(dialog_details, NULL);

	/* Which plugin has been selected? */
	selected_menu_item = cong_eel_option_menu_get_selected_menu_item (dialog_details->select_print_method_option_menu);
			
	if (selected_menu_item) {
		return (CongServicePrintMethod*)g_object_get_data(G_OBJECT(selected_menu_item),
							"print_method");
	} else {
		return NULL;
	}
}

static void add_print_method_to_menu(CongServicePrintMethod *print_method, gpointer user_data)
{
	CongPrintDialogDetails *dialog_details = (CongPrintDialogDetails*)user_data;

	if (cong_print_method_supports_document(print_method, dialog_details->doc)) {
		GtkWidget *menu = dialog_details->select_print_method_menu;
		GtkMenuItem *menu_item = GTK_MENU_ITEM(gtk_menu_item_new_with_label( cong_service_get_name(CONG_SERVICE(print_method))));

		gtk_menu_shell_append(GTK_MENU_SHELL(menu),
				      GTK_WIDGET(menu_item));
		
		g_object_set_data(G_OBJECT(menu_item),
				  "print_method",
				  print_method);

		dialog_details->got_any_print_methods = TRUE;
	}
}

#if 0
static void monitor_print_method(CongPrintDialogDetails *dialog_details)
{
	CongServicePrintMethod* print_method = get_selected_print_method(dialog_details);
	g_assert(dialog_details);
	g_assert(print_method);

#if 0
	/* start monitoring GConf for selected plugin */
	{
		gchar *print_method_namespace  = cong_service_get_gconf_namespace(CONG_SERVICE(print_method));

		/* g_message("adding notification for \"%s\"", print_method_namespace); */
		dialog_details->connection_id = gconf_client_notify_add(the_globals.gconf_client,
									print_method_namespace,
									gconf_notify_func,
									dialog_details,
									NULL,
									NULL);
		g_free(print_method_namespace);
	}

	/* get value and set up entry accordingly */
	{
		gchar *uri = cong_print_method_get_preferred_uri(print_method);

		if (uri) {
			gtk_entry_set_text(dialog_details->filename_entry, uri);
			g_free(uri);
		} else {
			gtk_entry_set_text(dialog_details->filename_entry, "");
		}
	}
#endif
}
#endif

static void on_print_method_selection_changed(GtkOptionMenu *optionmenu,
					  gpointer user_data)
{
#if 0
	GtkWidget* menu = gtk_option_menu_get_menu(optionmenu);
	CongPrintDialogDetails *details = user_data;
#endif

	g_message("on_print_method_selection_changed");

#if 0
	/* Stop monitoring GConf for old plugin: */
	gconf_client_notify_remove(the_globals.gconf_client,
				   details->connection_id);

	/* Monitor new plugin; set up stuff accordingly: */
	monitor_print_method(details);
#endif
}

#if 0
static void on_select_filename_button_clicked(GtkButton *button,
					      gpointer user_data)
{
	gchar *print_uri;
	gchar *new_uri;
	CongPrintDialogDetails *details = user_data;
	CongServicePrintMethod* print_method = get_selected_print_method(details);
	g_assert(print_method);

	/* 
	   FIXME: ought to set up MIME-filters according to the selected print_method plugin.
	   Needs a decent file-selector API for this...
	*/
	print_uri = cong_print_method_get_preferred_uri(print_method);
	
	new_uri = cong_get_file_name("Select file to print to",
				     print_uri,
				     GTK_WINDOW(details->dialog));

	if (new_uri) {
		cong_print_method_set_preferred_uri(print_method, new_uri);
		g_free(new_uri);
	}
	
	if (print_uri) {
		g_free(print_uri);
	}
}
#endif

static GtkWidget*
cong_document_print_dialog_new (CongDocument *doc, 
				GtkWindow *parent_window)
{
	xmlDocPtr xml_doc;
	GtkWidget *dialog;
	CongDialogContent *content;
	CongDialogCategory *general_category;
	CongDialogCategory *print_method_category;
	gchar *title, *filename;
	CongPrintDialogDetails *dialog_details;
	

	g_return_val_if_fail(doc, NULL);

	xml_doc = cong_document_get_xml(doc);

	filename = cong_document_get_filename(doc);

	title = g_strdup_printf(_("Print \"%s\""), filename);

	dialog_details = g_new0(CongPrintDialogDetails,1);

	dialog_details->doc = doc;

	dialog = gtk_dialog_new_with_buttons(_("Print"),
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

	/* Set up print_method selection option menu: */
	{
		dialog_details->select_print_method_option_menu = GTK_OPTION_MENU(gtk_option_menu_new());
		dialog_details->select_print_method_menu = gtk_menu_new();
		gtk_option_menu_set_menu(dialog_details->select_print_method_option_menu,
					 dialog_details->select_print_method_menu);
		
		cong_plugin_manager_for_each_print_method (cong_app_get_plugin_manager (cong_app_singleton ()), 
							   add_print_method_to_menu, 
							   dialog_details);
		gtk_option_menu_set_history(dialog_details->select_print_method_option_menu,0);
	}

	cong_dialog_category_add_field (general_category, 
					_("Print Method:"), 
					GTK_WIDGET(dialog_details->select_print_method_option_menu),
					TRUE);

#if 0
	monitor_print_method(dialog_details);
#endif

	print_method_category = cong_dialog_content_add_category(content, _("Print Options"));

	g_signal_connect(dialog_details->select_print_method_option_menu,
			 "changed",
			 G_CALLBACK(on_print_method_selection_changed),
			 dialog_details);

#if 0
	cong_dialog_category_add_field(print_method_category, _("Name"), make_uneditable_text(cong_dispspec_get_name(ds)));
	cong_dialog_category_add_field(print_method_category, _("Description"), make_uneditable_text(cong_dispspec_get_description(ds)));
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
do_ui_file_print(CongDocument *doc,
		 GtkWindow *toplevel_window,
		 gboolean is_preview)
{
	GtkWidget *dialog;
	gint result;
	CongPrintDialogDetails *dialog_details;

	g_return_if_fail(doc);

	dialog = cong_document_print_dialog_new(doc,
						toplevel_window);


	dialog_details = g_object_get_data( G_OBJECT(dialog),
					    "dialog_details");
	g_assert(dialog_details);

	if (dialog_details->got_any_print_methods) {
		gtk_widget_show_all(dialog);

		result = gtk_dialog_run(GTK_DIALOG(dialog));


		
		switch (result) {
		default: /* Do nothing; dialog was cancelled */
			break;
			
		case GTK_RESPONSE_OK:
			{
				CongServicePrintMethod* print_method;
				
				/* Which plugin has been selected? */
				print_method = get_selected_print_method(dialog_details);
				if (print_method) {
					GnomePrintJob *gpj;
					GnomePrintContext *gpc;
					GtkWidget *preview_widget;
					
					g_message("Print_Method invoked: \"%s\"", cong_service_get_name(CONG_SERVICE(print_method)));
					
					gpj = gnome_print_job_new (NULL); /* FIXME: provide an option to use a non-default config */
					gpc = gnome_print_job_get_context (gpj);
					
					cong_print_method_invoke(print_method, 
								 doc, 
								 gpc,
								 toplevel_window);
					
					gnome_print_job_close (gpj);
					
					preview_widget = gnome_print_job_preview_new (gpj, _("Print Preview"));
					gtk_widget_show(preview_widget);
				}
			}
			break;
		}

	} else {
		/* No print methods were found that can handle this document: */
		gchar *filename = cong_document_get_filename(doc);
		gchar *what_failed = g_strdup_printf(_("Conglomerate cannot print \"%s\""), filename);
		GtkDialog* error_dialog = cong_error_dialog_new(toplevel_window,
								what_failed, 
								_("None of Conglomerate's plugins know how to print files of that type."),
								"");

		gtk_dialog_run(error_dialog);
		gtk_widget_destroy(GTK_WIDGET(error_dialog));
	}

	gtk_widget_destroy(dialog);
	g_free(dialog_details);
}


/**
 * cong_ui_hook_file_print_preview:
 * @doc:
 * @toplevel_window:
 *
 * TODO: Write me
 */
void
cong_ui_hook_file_print_preview (CongDocument *doc,
				 GtkWindow *toplevel_window)
{
	do_ui_file_print(doc,
			 toplevel_window,
			 TRUE);
}

/**
 * cong_ui_hook_file_print:
 * @doc:
 * @toplevel_window:
 *
 * TODO: Write me
 */
void
cong_ui_hook_file_print (CongDocument *doc,
			 GtkWindow *toplevel_window)
{
	do_ui_file_print(doc,
			 toplevel_window,
			 FALSE);
}

#endif /* #if ENABLE_PRINTING */
