/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * new_doc.c
 *
 * Copyright (C) 2002 David Malcolm
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
#include <string.h>

enum
{
	NEWDOCTYPELIST_NAME_COLUMN,
	NEWDOCTYPELIST_DESCRIPTION_COLUMN,
	NEWDOCTYPELIST_FACTORY_COLUMN,
	NEWDOCTYPELIST_N_COLUMNS
};

struct add_factory_callback_data
{
	GtkListStore *store;
};

static void add_factory_callback(CongDocumentFactory *factory, gpointer user_data)
{
	struct add_factory_callback_data *data = user_data;
	
	GtkTreeIter iter;
	gtk_list_store_append (data->store, &iter);  /* Acquire an iterator */
			
	gtk_list_store_set (data->store, &iter,
			    NEWDOCTYPELIST_NAME_COLUMN, cong_functionality_get_name(CONG_FUNCTIONALITY(factory)),
			    NEWDOCTYPELIST_DESCRIPTION_COLUMN, cong_functionality_get_description(CONG_FUNCTIONALITY(factory)),
			    NEWDOCTYPELIST_FACTORY_COLUMN, factory,
			    -1);

}

GtkWidget *make_type_selection_widget(void)
{
	/* FIXME:  didn't work very well when I added a scrolled window */
#if 0
	GtkWidget *scrolled_window;
#endif
	GtkWidget *list_view;
	GtkListStore *store;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	store = gtk_list_store_new (NEWDOCTYPELIST_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);

	list_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

	/* The view now holds a reference.  We can get rid of our own
	 * reference */
	g_object_unref (G_OBJECT (store));

#if 1
	/* Populate the list based on the plugins: */
	{
		struct add_factory_callback_data user_data;
		user_data.store = store;
		cong_plugin_manager_for_each_document_factory(the_globals.plugin_manager, add_factory_callback, &user_data);
	}
#else
	/* Populate the store based on the ds-registry: */
	{
		CongDispspecRegistry* registry = the_globals.ds_registry;
		int i;

		for (i=0;i<cong_dispspec_registry_get_num(registry);i++) {
			const CongDispspec* ds = cong_dispspec_registry_get(registry,i);
			
			GtkTreeIter iter;
			gtk_list_store_append (store, &iter);  /* Acquire an iterator */
			
			gtk_list_store_set (store, &iter,
					    NEWDOCTYPELIST_NAME_COLUMN, cong_dispspec_get_name(ds),
					    NEWDOCTYPELIST_DESCRIPTION_COLUMN, "the quick brown fox jumps over the lazy dog\nhere is some more text\nand some more", /* cong_dispspec_get_description(ds), */
					    NEWDOCTYPELIST_FACTORY_COLUMN, NULL,
					    -1);
		}
	}
#endif

	renderer = gtk_cell_renderer_text_new ();

	column = gtk_tree_view_column_new_with_attributes ("File Type", renderer,
							   "text", NEWDOCTYPELIST_NAME_COLUMN,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (list_view), column);

	column = gtk_tree_view_column_new_with_attributes ("Description", renderer,
							   "text", NEWDOCTYPELIST_DESCRIPTION_COLUMN,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (list_view), column);

	gtk_widget_show(list_view);

#if 1
	return list_view;
#else
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), list_view);
	gtk_widget_show(scrolled_window);
	return scrolled_window;
#endif
}

void new_document(GtkWindow *parent_window)
{
	GtkWidget *window;
	GnomeDruid *druid;
	GtkWidget *first_page;
	GtkWidget *second_page;
	GtkWidget *type_selection_widget;
	GdkPixbuf *logo = NULL;
	GdkPixbuf *watermark = NULL;
	GdkPixbuf *top_watermark = NULL;

	/* FIXME:  what if no document factories found? */

	druid = GNOME_DRUID(gnome_druid_new_with_window("Creating a new file",
							parent_window,
							TRUE,
							&window));

	first_page = gnome_druid_page_edge_new_with_vals(GNOME_EDGE_START,
							 TRUE,
							 "Creating a new file",
							 "This assistant will guide you through creating a new file.\n\nVarious types of file are available, and Conglomerate may be able to supply some of the \"boilerplate\" content for you.\n\nWe hope that in future versions of Conglomerate you will be able to create \"template documents\" to add to this system.",
							 logo,
							 watermark,
							 top_watermark);
	gnome_druid_append_page(druid, GNOME_DRUID_PAGE(first_page));
	gtk_widget_show(GTK_WIDGET(first_page));

	second_page = gnome_druid_page_standard_new_with_vals("Creating a new file",
							      logo,
							      top_watermark);

	type_selection_widget = make_type_selection_widget();
	gnome_druid_page_standard_append_item(GNOME_DRUID_PAGE_STANDARD(second_page),
					      "What type of file would you like to create?",
					      type_selection_widget,
					      "Highlight one of the items in the list and select \"Forward\" to continue");
	gnome_druid_append_page(druid, GNOME_DRUID_PAGE(second_page));
	gtk_widget_show(GTK_WIDGET(second_page));

	gtk_widget_show(GTK_WIDGET(window));

#if 0
	CONG_DO_UNIMPLEMENTED_DIALOG("The selected menu item has not yet been implemented.");
#endif
}

#if 0

GtkWidget *newdoc;

int capsula_mode = 0;

char *capsula_str = "capsula";
char *extracta_str = "extracta";
char *selecta_str = "selecta";
char *informa_str = "informa";
char *analiza_str = "analiza";




static gint new_document_from_template(GtkWidget *w, char *type)
{
	gtk_widget_destroy(newdoc);

#ifndef RELEASE
	fputs(type, stdout);
	printf("\n");
#endif	

	if (capsula_mode && !strcmp(type, "capsula"))
	{
/*		
		add_capsula_template();
 */
	}
	else
	{
	  xmlview_destroy(TRUE);
/*		
	  get_template(type);
 */
	}

	gtk_widget_set_sensitive(cong_gui_get_button_submit(&the_gui), TRUE);

	if (!strcmp(type, "capsula"))
	{
/*		
		vect_win_open();
 */
		capsula_mode = 1;
	}
	else capsula_mode = 0;

	return(TRUE);
}


int gui_window_new_document_make()
{
	GtkWidget *w0, *w1;

  newdoc = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(newdoc), 10);
  gtk_widget_set_usize(GTK_WIDGET(newdoc), 240, 120);
  gtk_window_set_title(GTK_WINDOW(newdoc), "Select document class");
  gtk_window_set_position(GTK_WINDOW(newdoc), GTK_WIN_POS_MOUSE);
	gtk_window_set_modal(GTK_WINDOW(newdoc), 1);
	gtk_window_set_policy(GTK_WINDOW(newdoc), 0, 0, 1);

  /* --- Window -> vbox --- */

  w0 = gtk_vbox_new(TRUE, 1);
  gtk_container_add(GTK_CONTAINER(newdoc), w0);
  gtk_widget_show(w0);

	/* Window -> vbox -> buttons */

	w1 = gtk_button_new_with_label("Capsula");
	gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
	gtk_widget_show(w1);
  gtk_signal_connect(GTK_OBJECT(w1), "clicked", (GtkSignalFunc) new_document_from_template, (gpointer) capsula_str);

#if 0	
	w1 = gtk_button_new_with_label("Extracta");
	gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
	gtk_widget_show(w1);
  gtk_signal_connect(GTK_OBJECT(w1), "clicked", (GtkSignalFunc) new_document_from_template, (gpointer) extracta_str);

	w1 = gtk_button_new_with_label("Informa");
	gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
	gtk_widget_show(w1);
  gtk_signal_connect(GTK_OBJECT(w1), "clicked", (GtkSignalFunc) new_document_from_template, (gpointer) informa_str);

	w1 = gtk_button_new_with_label("Selecta");
	gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
	gtk_widget_show(w1);
  gtk_signal_connect(GTK_OBJECT(w1), "clicked", (GtkSignalFunc) new_document_from_template, (gpointer) selecta_str);

	w1 = gtk_button_new_with_label("Analiza");
	gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
	gtk_widget_show(w1);
  gtk_signal_connect(GTK_OBJECT(w1), "clicked", (GtkSignalFunc) new_document_from_template, (gpointer) analiza_str);
#endif
	
	gtk_widget_show(newdoc);
	return(1);
}
#endif
