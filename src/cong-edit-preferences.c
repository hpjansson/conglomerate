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

typedef struct CongPreferencesDialogDetails CongPreferencesDialogDetails;

struct CongPreferencesDialogDetails
{
	GladeXML *xml;
};

static void 
populate_dispspec_tree (CongPreferencesDialogDetails *dialog_details);

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
	CongPreferencesDialogDetails *dialog_details;

	dialog_details = g_new0 (CongPreferencesDialogDetails, 1);

	dialog_details->xml = cong_util_load_glade_file ("conglomerate/glade/cong-preferences.glade", 
							 "preferences_dialog",
							 NULL,
							 NULL);
	dialog = GTK_DIALOG (glade_xml_get_widget (dialog_details->xml, "preferences_dialog"));

	populate_dispspec_tree (dialog_details);

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
	DOCTYPELIST_DESCRIPTION_COLUMN,
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

	store = gtk_list_store_new (DOCTYPELIST_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);

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
			
			GtkTreeIter iter;
			gtk_list_store_append (store, &iter);  /* Acquire an iterator */
			
			gtk_list_store_set (store, &iter,
					    DOCTYPELIST_NAME_COLUMN, cong_dispspec_get_name(ds),
					    DOCTYPELIST_DESCRIPTION_COLUMN, cong_dispspec_get_description(ds),
					    -1);
		}
	}

	renderer = gtk_cell_renderer_text_new ();

	column = gtk_tree_view_column_new_with_attributes (_("Name"), renderer,
							   "text", DOCTYPELIST_NAME_COLUMN,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), 
				     column);

	column = gtk_tree_view_column_new_with_attributes (_("Description"), renderer,
							   "text", DOCTYPELIST_DESCRIPTION_COLUMN,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), 
				     column);
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
