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
#include "cong-plugin.h"

static void add_exporter_to_menu(CongExporter *exporter, gpointer user_data)
{
	/* FIXME: should check for an appropriate FPI */
	GtkWidget *menu = user_data;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),
			      gtk_menu_item_new_with_label( cong_functionality_get_name(CONG_FUNCTIONALITY(exporter))));

}


static void on_exporter_selection_changed(GtkOptionMenu *optionmenu,
					  gpointer user_data)
{
	GtkWidget* menu = gtk_option_menu_get_menu(optionmenu);

	g_message("on_exporter_selection_changed");
}

static GtkWidget *cong_document_export_dialog_new(CongDocument *doc, 
						  GtkWindow *parent_window)
{
	xmlDocPtr xml_doc;
	CongDispspec* ds;
	GtkWidget *dialog;
	CongDialogContent *content;
	CongDialogCategory *general_category;
	CongDialogCategory *exporter_category;
	gchar *filename, *title;
	GtkWidget *select_exporter_option_menu;
	GtkWidget *select_exporter_menu;

	g_return_val_if_fail(doc, NULL);

	xml_doc = cong_document_get_xml(doc);
	ds = cong_document_get_dispspec(doc);

	filename = cong_document_get_filename(doc);

	title = g_strdup_printf(_("Export \"%s\""), filename);

	dialog = gtk_dialog_new_with_buttons(_("Export"),
					     parent_window,
					     0,
					     GTK_STOCK_CANCEL,
					     GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	gtk_container_set_border_width(GTK_CONTAINER(dialog), 12);

	content = cong_dialog_content_new(FALSE);
	general_category = cong_dialog_content_add_category(content, _("General"));

	select_exporter_option_menu = gtk_option_menu_new();
	select_exporter_menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(select_exporter_option_menu),
				 select_exporter_menu);

	cong_plugin_manager_for_each_exporter(the_globals.plugin_manager, add_exporter_to_menu, select_exporter_menu);
	gtk_option_menu_set_history(GTK_OPTION_MENU(select_exporter_option_menu),0);

	cong_dialog_category_add_field(general_category, _("File:"), make_uneditable_text("filename"));
	cong_dialog_category_add_field(general_category, _("Exporter:"), select_exporter_option_menu);

	exporter_category = cong_dialog_content_add_category(content, _("Export Options"));

#if 0
	g_signal_connect(select_exporter_option_menu,
			 "changed"
			 on_exporter_selection_changed,
			 NULL);
#endif

#if 0
	cong_dialog_category_add_field(exporter_category, _("Name"), make_uneditable_text(cong_dispspec_get_name(ds)));
	cong_dialog_category_add_field(exporter_category, _("Description"), make_uneditable_text(cong_dispspec_get_description(ds)));
#endif

	g_free(title);
	g_free(filename);

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   cong_dialog_content_get_widget(content));

	gtk_widget_show_all(dialog);

	return dialog;
}


void
cong_ui_file_export(CongDocument *doc,
		    GtkWindow *toplevel_window)
{
	GtkWidget *dialog;

	g_return_if_fail(doc);

	dialog = cong_document_export_dialog_new(doc,
						 toplevel_window);

	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);

	/* FIXME: memory leaks */

}
