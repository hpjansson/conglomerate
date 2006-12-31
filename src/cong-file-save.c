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
#include <libgnomevfs/gnome-vfs.h>

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
	char *current_doc_name;
	char *new_doc_name;
	GnomeVFSURI *uri;

	g_return_val_if_fail(doc, FALSE);
	g_return_val_if_fail(parent_window, FALSE);

	current_doc_name = cong_document_get_full_uri(doc);
	
	new_doc_name = cong_get_file_name(_("Save XML as..."), 
					  current_doc_name,
					  parent_window,
					  CONG_FILE_CHOOSER_ACTION_SAVE,
					  cong_file_selection_make_xml_filter_list ());
	if (current_doc_name) {
		g_free(current_doc_name);
	}

	if (!new_doc_name) {
		return TRUE;
	}
	
	uri = gnome_vfs_uri_new (new_doc_name);
	
	if (gnome_vfs_uri_exists (uri)) {

	        GnomeVFSFileInfo  *info;
		gboolean          writable;
		GtkWidget         *dialog;
		GtkWidget         *content;
		gchar             *reason;
	
        	info = gnome_vfs_file_info_new ();

		if (gnome_vfs_get_file_info (new_doc_name,
					     info, 
					     GNOME_VFS_FILE_INFO_FOLLOW_LINKS | 
					     GNOME_VFS_FILE_INFO_GET_ACCESS_RIGHTS) == GNOME_VFS_OK) {

			if (info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_ACCESS) {
				
	  	    		    writable = info->permissions & GNOME_VFS_PERM_ACCESS_WRITABLE;
				
				    if (!writable) {		    		    
					    reason = g_strdup_printf (_("The file \"%s\" is read-only.\n"), new_doc_name);
	
				    } else {	
					    reason = g_strdup_printf (_("A file named \"%s\" already exists.\n"), new_doc_name);
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
					    cong_document_save(doc, new_doc_name, parent_window);	    
				    }
				    
				    gtk_widget_destroy (dialog);
				    g_free (reason);
			}

	         }

		gnome_vfs_file_info_unref (info);
		 
	} else {
	        cong_document_save(doc, new_doc_name, parent_window);
	}

	/* Add recent entry */
	{
		CongPrimaryWindow *primary_window = cong_document_get_primary_window(doc);
		gtk_recent_manager_add_item (primary_window->recent_manager, new_doc_name);
	}

	g_free(new_doc_name);
	gnome_vfs_uri_unref (uri);
	
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
	gchar *doc_name;

	g_return_val_if_fail(doc, FALSE);

	doc_name = cong_document_get_full_uri(doc);

	if (!doc_name) {
		return save_document_as(doc, parent_window);
	}

	cong_document_save(doc, doc_name, parent_window);
	
	g_free(doc_name);

	return TRUE;
}


