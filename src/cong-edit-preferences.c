/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-edit-preferences.c
 *
 * Copyright (C) 2004 David Malcolm
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
#include "cong-util.h"
#include "cong-app.h"
#include "cong-dispspec-registry.h"
#include "cong-dispspec.h"
#include "cong-glade.h"

#include "cong-plugin-manager.h"
#include "cong-plugin.h"
#include "cong-service.h"

typedef struct CongPreferencesDialogDetails CongPreferencesDialogDetails;

struct CongPreferencesDialogDetails
{
	GladeXML *xml;
};

static void 
populate_dispspec_tree (CongPreferencesDialogDetails *dialog_details);

static void 
populate_plugin_tree (CongPreferencesDialogDetails *dialog_details);

static void
on_preferences_dialog_close (GtkButton *button,
                   CongPreferencesDialogDetails *dialog_details);

static gboolean
on_preferences_dialog_destroy (GtkWidget *widget,
			       CongPreferencesDialogDetails *dialog_details);


/**
 * cong_ui_hook_edit_preferences:
 * @toplevel_window:
 *
 * Opens the Preferences dialog for Conglomerate
 */
void
cong_ui_hook_edit_preferences (GtkWindow *toplevel_window)
{
	GtkDialog *dialog;
	GtkButton *close;
	CongPreferencesDialogDetails *dialog_details;

	dialog_details = g_new0 (CongPreferencesDialogDetails, 1);

	dialog_details->xml = cong_util_load_glade_file ("conglomerate/glade/cong-preferences.glade", 
							 "preferences_dialog",
							 NULL,
							 NULL);
	dialog = GTK_DIALOG (glade_xml_get_widget (dialog_details->xml, "preferences_dialog"));

	/* Get the dialog close button and connect to callback */
	close = GTK_BUTTON (glade_xml_get_widget (dialog_details->xml, "closebutton1"));
	g_signal_connect (G_OBJECT (close),
			  "clicked",
			  G_CALLBACK (on_preferences_dialog_close),
			  dialog_details);

	populate_dispspec_tree (dialog_details);
	populate_plugin_tree (dialog_details);

	/* Cleanup handler: */
	g_signal_connect (G_OBJECT (dialog),
			  "destroy",
			  G_CALLBACK (on_preferences_dialog_destroy),
			  dialog_details);

	gtk_window_set_transient_for (GTK_WINDOW (dialog), 
				      toplevel_window);

	gtk_widget_show (GTK_WIDGET (dialog));
}

enum
{
	DOCTYPELIST_NAME_COLUMN,
	DOCTYPELIST_N_COLUMNS
};

static void 
populate_dispspec_tree (CongPreferencesDialogDetails *dialog_details)
{
	GtkTreeView* tree_view;
	GtkListStore *store;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	g_assert (dialog_details);

	tree_view = GTK_TREE_VIEW (glade_xml_get_widget (dialog_details->xml, "treeview_dispspecs"));

	store = gtk_list_store_new (DOCTYPELIST_N_COLUMNS, G_TYPE_STRING);

	gtk_tree_view_set_model (tree_view,
				 GTK_TREE_MODEL (store));

	/* The view now holds a reference.  We can get rid of our own
	 * reference */
	g_object_unref (G_OBJECT (store));

	/* Populate the store based on the ds-registry: */
	{
		CongDispspecRegistry *registry = cong_app_get_dispspec_registry (cong_app_singleton ());
		int i;

		for (i=0; i<cong_dispspec_registry_get_num (registry); i++) {
			const CongDispspec* ds = cong_dispspec_registry_get(registry,i);

			gchar *text;
			
			GtkTreeIter iter;
			gtk_list_store_append (store, &iter);  /* Acquire an iterator */

			text = g_strdup_printf ("<big><b>%s</b></big>\n%s", 
						cong_dispspec_get_name(ds),
						cong_dispspec_get_description(ds));
			gtk_list_store_set (store, &iter,
					    DOCTYPELIST_NAME_COLUMN, text,
					    -1);
			
			g_free (text);
		}
	}

	renderer = gtk_cell_renderer_text_new ();

	column = gtk_tree_view_column_new_with_attributes (_("Name"), renderer,
							   "markup", DOCTYPELIST_NAME_COLUMN,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), 
				     column);

}

enum
{
	PLUGINLIST_NAME_COLUMN,
	PLUGINLIST_DESCRIPTION_COLUMN,
	PLUGINLIST_N_COLUMNS
};

struct add_service_data
{
	GtkTreeStore *store;
	GtkTreeIter *plugin_iter;
};

static void
add_service (CongService *service,
	     gpointer user_data)
{
	struct add_service_data *add_service_data = (struct add_service_data *)user_data;
	GtkTreeIter service_iter;

	gtk_tree_store_append (add_service_data->store, &service_iter, add_service_data->plugin_iter);  /* Acquire an iterator */
	
	gtk_tree_store_set (add_service_data->store, &service_iter,
			    PLUGINLIST_NAME_COLUMN, cong_service_get_id (service),
			    PLUGINLIST_DESCRIPTION_COLUMN, cong_service_get_name (service),
			    -1);
}

static void
add_plugin (CongPlugin *plugin,
	    gpointer user_data)
{
	GtkTreeStore *store = GTK_TREE_STORE (user_data);
	GtkTreeIter plugin_iter;
	struct add_service_data add_service_data;
	
	add_service_data.store = store;
	add_service_data.plugin_iter = &plugin_iter;

	gtk_tree_store_append (store, &plugin_iter, NULL);  /* Acquire an iterator */
	
	gtk_tree_store_set (store, &plugin_iter,
			    PLUGINLIST_NAME_COLUMN, cong_plugin_get_id (plugin),
			    -1);

	cong_plugin_for_each_service (plugin, 
				      add_service,
				      &add_service_data);
}

static void 
populate_plugin_tree (CongPreferencesDialogDetails *dialog_details)
{
	GtkTreeView* tree_view;
	GtkTreeStore *store;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	g_assert (dialog_details);

	tree_view = GTK_TREE_VIEW (glade_xml_get_widget (dialog_details->xml, "treeview_plugins"));

	store = gtk_tree_store_new (PLUGINLIST_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);

	gtk_tree_view_set_model (tree_view,
				 GTK_TREE_MODEL (store));

	/* The view now holds a reference.  We can get rid of our own
	 * reference */
	g_object_unref (G_OBJECT (store));

	/* Populate the store based on the plugin manager: */
	{
		cong_plugin_manager_for_each_plugin (cong_app_get_plugin_manager (cong_app_singleton ()),
						     add_plugin,
						     store);
	}

	renderer = gtk_cell_renderer_text_new ();

	column = gtk_tree_view_column_new_with_attributes (_("Name"), renderer,
							   "text", PLUGINLIST_NAME_COLUMN,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), 
				     column);

	column = gtk_tree_view_column_new_with_attributes (_("Description"), renderer,
							   "text", PLUGINLIST_DESCRIPTION_COLUMN,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), 
				     column);
}

static void
on_preferences_dialog_close (GtkButton *button,
			     CongPreferencesDialogDetails *dialog_details)
{
	GtkWidget *dialog;

	g_assert (dialog_details);

	dialog = glade_xml_get_widget (dialog_details->xml, "preferences_dialog");
	gtk_widget_destroy (dialog);
}

static gboolean
on_preferences_dialog_destroy (GtkWidget *widget,
			       CongPreferencesDialogDetails *dialog_details)
{
	g_assert (dialog_details);

	g_object_unref (G_OBJECT (dialog_details->xml));
	g_free (dialog_details);

	return FALSE;
}
