/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-file-save.c
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
 *          Hans Petter Jansson <hpj@ximian.com>
 */

#include <gtk/gtk.h>

#include "global.h"

#include "cong-document.h"
#include "cong-primary-window.h"
#include "cong-file-selection.h"
#include "cong-ui-hooks.h"
#include "cong-dialog.h"

/**
 * toolbar_callback_save:
 * @w:
 * @data:
 *
 * TODO: Write me
 * Returns:
 */
gint 
toolbar_callback_save(GtkWidget *w, gpointer data)
{
	CongPrimaryWindow *primary_window = data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	return save_document(doc, cong_primary_window_get_toplevel(primary_window));
}


/**
 * save_document_as:
 * @doc:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
gint 
save_document_as(CongDocument *doc, GtkWindow *parent_window)
{
	GFile *current_doc;
	GFile *new_doc;

	g_return_val_if_fail(doc, FALSE);
	g_return_val_if_fail(parent_window, FALSE);

	current_doc = cong_document_get_file(doc);
	
	new_doc = cong_get_file_name(_("Save XML as..."),
	                             current_doc,
	                             parent_window,
	                             CONG_FILE_CHOOSER_ACTION_SAVE,
	                             cong_file_selection_make_xml_filter_list ());

	if (!new_doc) {
		return TRUE;
	}

	if (g_file_query_exists (new_doc, NULL)) {

	        GFileInfo  *info;
		gboolean   writable;
		GtkWidget  *dialog;
		GtkWidget  *content;
		gchar      *reason;
		const char *filename;
	
		info = g_file_query_info (new_doc,
	                                  G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE "," G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
	                                  G_FILE_QUERY_INFO_NONE,
	                                  NULL,
	                                  NULL);

		if (info) {

			filename = g_file_info_get_display_name(info);
			writable = g_file_info_get_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);

			if (!writable) {
			    reason = g_strdup_printf (_("The file \"%s\" is read-only.\n"), filename);

			} else {
			    reason = g_strdup_printf (_("A file named \"%s\" already exists.\n"), filename);
			}

			dialog = gtk_dialog_new_with_buttons(NULL,
			     parent_window,
			     GTK_DIALOG_MODAL,
			     GTK_STOCK_CANCEL,
			     GTK_RESPONSE_CANCEL,
			     _("_Replace"),
			     GTK_RESPONSE_OK,
			     NULL);

			gtk_dialog_set_default_response(GTK_DIALOG(dialog),
			     GTK_RESPONSE_OK);


			content = cong_alert_content_new(GTK_STOCK_DIALOG_WARNING,
						     reason,
						     _("Do you want to replace it with the "
						     "one you are saving?"),
						     NULL);

			gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), content);
			gtk_widget_show_all(dialog);

			if (gtk_dialog_run (GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
			    cong_document_save(doc, new_doc, parent_window);
			}

			gtk_widget_destroy (dialog);
			g_free (reason);

			g_object_unref(info);

	         }
		 
	} else {
	        cong_document_save(doc, new_doc, parent_window);
	}

	/* Add recent entry */
	{
		CongPrimaryWindow *primary_window = cong_document_get_primary_window(doc);
		char *uri = g_file_get_uri(new_doc);
		gtk_recent_manager_add_item (primary_window->recent_manager, uri);
		g_free(uri);
	}

	g_object_unref(new_doc);
	
	return TRUE;
}

/**
 * save_document:
 * @doc:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
gint 
save_document(CongDocument *doc, GtkWindow *parent_window) 
{
	GFile *doc_name;

	g_return_val_if_fail(doc, FALSE);

	doc_name = cong_document_get_file(doc);

	if (!doc_name) {
		return save_document_as(doc, parent_window);
	}

	cong_document_save(doc, doc_name, parent_window);

	return TRUE;
}


