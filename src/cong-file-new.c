/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-file-new.c
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
#include "cong-plugin.h"
#include "cong-app.h"

enum
{
	NEWDOCTYPELIST_NAME_COLUMN,
	NEWDOCTYPELIST_DESCRIPTION_COLUMN,
	NEWDOCTYPELIST_FACTORY_COLUMN,
	NEWDOCTYPELIST_N_COLUMNS
};

struct CongNewFileAssistant
{
	GtkWidget *window;
	GnomeDruid *druid;
	GtkWidget *first_page;
	GtkWidget *second_page;
	GtkWidget *final_page;
	GnomeDruidPage *previous_page;
	GtkWidget *type_selection_widget;
	GdkPixbuf *logo;
	GdkPixbuf *watermark;
	GdkPixbuf *top_watermark;

	GtkListStore *list_store;
	GtkWidget *list_view;

	/* Mapping from factories to pages; the "first" page for each factory, actually the third page within the druid */
	GHashTable *hash_table_of_factory_to_page;
};

static void
set_pixbuf (GtkTreeViewColumn *tree_column,
	    GtkCellRenderer   *cell,
	    GtkTreeModel      *model,
	    GtkTreeIter       *iter,
	    gpointer           user_data)
{
	GdkPixbuf *pixbuf = NULL;
	CongDocumentFactory *factory;
	
       	gtk_tree_model_get (model, iter, 
			    NEWDOCTYPELIST_FACTORY_COLUMN, &factory,
			    -1);
	if (NULL==factory) {
		return;
	}

	pixbuf = cong_document_factory_get_icon(factory);

	g_object_set (GTK_CELL_RENDERER (cell), "pixbuf", pixbuf, NULL);
	
	if (pixbuf) {
		g_object_unref (pixbuf);
	}
}

gboolean first_page_of_factory_back(GnomeDruidPage *druidpage,
				    GtkWidget *widget,
				    gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;

	gnome_druid_set_page(GNOME_DRUID(assistant->druid), GNOME_DRUID_PAGE(assistant->second_page));

	return TRUE;
}

gboolean last_page_of_factory_next(GnomeDruidPage *druidpage,
				   GtkWidget *widget,
				   gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;

	/* Store page, so that the final page's back button takes us back here: */
	assistant->previous_page = druidpage;
	gnome_druid_set_page(GNOME_DRUID(assistant->druid), GNOME_DRUID_PAGE(assistant->final_page));

	return TRUE;
}

GnomeDruidPageStandard *cong_new_file_assistant_new_page(CongNewFileAssistant *assistant, 
							 CongDocumentFactory *document_factory, 
							 gboolean is_first_of_factory,
							 gboolean is_last_of_factory)
{
	GnomeDruidPageStandard *page;
	gchar *title;

	g_return_val_if_fail(assistant, NULL);
	g_return_val_if_fail(document_factory, NULL);

	title = g_strdup_printf(_("Creating a new file (%s)"), 
				cong_functionality_get_name(CONG_FUNCTIONALITY(document_factory)));

	page = GNOME_DRUID_PAGE_STANDARD(gnome_druid_page_standard_new_with_vals(title,
										 assistant->logo,
										 assistant->top_watermark));

	g_free(title);

	gnome_druid_append_page(assistant->druid, GNOME_DRUID_PAGE(page));
	gtk_widget_show(GTK_WIDGET(page));

	if (is_first_of_factory) {
		/* add to mapping from factories to pages: */
		g_hash_table_insert(assistant->hash_table_of_factory_to_page, document_factory, page);

		/* "Back" should take us to the druid's second page: */
		g_signal_connect(GTK_OBJECT(page), "back", G_CALLBACK(first_page_of_factory_back), assistant);
	}

	if (is_last_of_factory) {
		/* "Forward" should take us to the druid's final page: */
		g_signal_connect(GTK_OBJECT(page), "next", G_CALLBACK(last_page_of_factory_next), assistant);
	}

	return page;
}

void cong_new_file_assistant_set_page(CongNewFileAssistant *assistant, GnomeDruidPage *page)
{
	g_return_if_fail(assistant);
	g_return_if_fail(page);


	gnome_druid_set_page(GNOME_DRUID(assistant->druid), page);
	
}

GtkWindow *cong_new_file_assistant_get_toplevel(CongNewFileAssistant *assistant)
{
	g_return_if_fail(assistant);

	return GTK_WINDOW(assistant->window);
}

static void add_factory_callback(CongDocumentFactory *factory, gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;
	
	GtkTreeIter iter;
	gtk_list_store_append (assistant->list_store, &iter);  /* Acquire an iterator */
			
	gtk_list_store_set (assistant->list_store, &iter,
			    NEWDOCTYPELIST_NAME_COLUMN, cong_functionality_get_name(CONG_FUNCTIONALITY(factory)),
			    NEWDOCTYPELIST_DESCRIPTION_COLUMN, cong_functionality_get_description(CONG_FUNCTIONALITY(factory)),
			    NEWDOCTYPELIST_FACTORY_COLUMN, factory,
			    -1);

}

GtkWidget *make_type_selection_widget(CongNewFileAssistant *assistant)
{
	/* FIXME:  didn't work very well when I added a scrolled window */
#if 0
	GtkWidget *scrolled_window;
#endif
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	assistant->list_store = gtk_list_store_new (NEWDOCTYPELIST_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);

	assistant->list_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (assistant->list_store));

#if 0
	/* The view now holds a reference.  We can get rid of our own
	 * reference */
	g_object_unref (G_OBJECT (assistant->list_store));
#endif

	/* Populate the list based on the plugins: */
	cong_plugin_manager_for_each_document_factory(cong_app_singleton()->plugin_manager, add_factory_callback, assistant);


	/* The "File Type" column: */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("File Type"));

	/* Add a pixbuf-renderer to the column: */
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
 	gtk_tree_view_column_set_cell_data_func (column, renderer, set_pixbuf, NULL, NULL);

	/* Add a text renderer to the column: */
	renderer = gtk_cell_renderer_text_new ();		
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column,
					    renderer,
					    "text", NEWDOCTYPELIST_NAME_COLUMN,
					    NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (assistant->list_view), column);

	/* The "Description" column: */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Description"), renderer,
							   "text", NEWDOCTYPELIST_DESCRIPTION_COLUMN,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (assistant->list_view), column);

	gtk_widget_show(assistant->list_view);

#if 1
	return assistant->list_view;
#else
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), list_view);
	gtk_widget_show(scrolled_window);
	return scrolled_window;
#endif
}

void second_page_prepare(GnomeDruidPage *druidpage,
			 GtkWidget *widget,
			 gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;

	g_message("second_page_prepare");

	gnome_druid_set_buttons_sensitive(GNOME_DRUID(assistant->druid),
					  TRUE, /* gboolean back_sensitive, */
					  FALSE, /* gboolean next_sensitive, */
					  TRUE, /* gboolean cancel_sensitive, */
					  TRUE); /* gboolean help_sensitive */

}

gboolean second_page_next(GnomeDruidPage *druidpage,
			  GtkWidget *widget,
			  gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;
	GtkTreeSelection* tree_selection;
	GtkTreeModel *tree_model;
	GtkTreeIter iter;

	g_message("second_page_next");

	/* Get selected factory */
	tree_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(assistant->list_view));

	if (gtk_tree_selection_get_selected (tree_selection,
                                             &tree_model,
                                             &iter) ) {
		CongDocumentFactory *factory;		
		GnomeDruidPage *appropriate_third_page;

		gtk_tree_model_get(tree_model,
				   &iter,
				   NEWDOCTYPELIST_FACTORY_COLUMN,
				   &factory,
				   -1);

		/* Find the relevant page for this factory within the druid: */
		appropriate_third_page = g_hash_table_lookup(assistant->hash_table_of_factory_to_page,
							     factory);

		if (appropriate_third_page) {
			cong_new_file_assistant_set_page(assistant, GNOME_DRUID_PAGE(appropriate_third_page));
		} else {
			/* what to do in this case? */
			g_message("haven't got a third page registered for factory \"%s\"", 
				  cong_functionality_get_name(CONG_FUNCTIONALITY(factory)));

			/* go to final page: */
			assistant->previous_page = druidpage;
			gnome_druid_set_page(GNOME_DRUID(assistant->druid), GNOME_DRUID_PAGE(assistant->final_page));
		}

#if 0
#error		
		cong_document_factory_invoke_action_callback(factory, assistant);
#endif


		/* Don't advance to the next page; this should have been done already */
		return TRUE;
	} else {
		/* Nothing selected; don't advance to the next page */
		return TRUE;
	}

}

gboolean final_page_back(GnomeDruidPage *druidpage,
			 GtkWidget *widget,
			 gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;

	/* go back to the stored previous page: */
	gnome_druid_set_page(GNOME_DRUID(assistant->druid), GNOME_DRUID_PAGE(assistant->previous_page));

	return TRUE;
}

gboolean final_page_finish(GnomeDruidPage *druidpage,
			   GtkWidget *widget,
			   gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;
	GtkTreeSelection* tree_selection;
	GtkTreeModel *tree_model;
	GtkTreeIter iter;

	g_message("final_page_finish");

	/* Get selected factory */
	tree_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(assistant->list_view));

	if (gtk_tree_selection_get_selected (tree_selection,
                                             &tree_model,
                                             &iter) ) {
		CongDocumentFactory *factory;		

		gtk_tree_model_get(tree_model,
				   &iter,
				   NEWDOCTYPELIST_FACTORY_COLUMN,
				   &factory,
				   -1);

		/* Invoke the factory: */
		cong_document_factory_invoke_action_callback(factory, assistant);

		/* Close the assistant: */
		gtk_widget_destroy(assistant->window);

		/* FIXME: this leaks memory */

	} else {
		g_warning("No selected factory");
	}


	return TRUE;
}

static void add_pages_for_factory_callback(CongDocumentFactory *factory, gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;

	cong_document_factory_invoke_page_creation_callback(factory, assistant);
}

void new_document(GtkWindow *parent_window)
{
	CongNewFileAssistant *assistant;

	assistant = g_new0(CongNewFileAssistant,1);

	/* FIXME:  what if no document factories found? */

	assistant->druid = GNOME_DRUID(gnome_druid_new_with_window(_("Creating a new file"),
								   parent_window,
								   TRUE,
								   &assistant->window));

	assistant->first_page = gnome_druid_page_edge_new_with_vals(GNOME_EDGE_START,
								   TRUE,
								   _("Creating a new file"),
								   _("This assistant will guide you through creating a new file.\n\nVarious types of file are available, and Conglomerate may be able to supply some of the \"boilerplate\" content for you.\n\nWe hope that in future versions of Conglomerate you will be able to create \"template documents\" to add to this system."),
								   assistant->logo,
								   assistant->watermark,
								   assistant->top_watermark);
	gnome_druid_append_page(assistant->druid, GNOME_DRUID_PAGE(assistant->first_page));
	gtk_widget_show(GTK_WIDGET(assistant->first_page));

	assistant->second_page = gnome_druid_page_standard_new_with_vals("Creating a new file",
							      assistant->logo,
							      assistant->top_watermark);

	assistant->type_selection_widget = make_type_selection_widget(assistant);
	gnome_druid_page_standard_append_item(GNOME_DRUID_PAGE_STANDARD(assistant->second_page),
					      _("What type of file would you like to create?"),
					      assistant->type_selection_widget,
					      _("Highlight one of the items in the list and select \"Forward\" to continue"));
	gnome_druid_append_page(assistant->druid, GNOME_DRUID_PAGE(assistant->second_page));
	gtk_widget_show(GTK_WIDGET(assistant->second_page));

	g_signal_connect(GTK_OBJECT(assistant->second_page), "prepare", G_CALLBACK(second_page_prepare), assistant);
	g_signal_connect(GTK_OBJECT(assistant->second_page), "next", G_CALLBACK(second_page_next), assistant);

	assistant->final_page = gnome_druid_page_edge_new_with_vals(GNOME_EDGE_FINISH,
								    TRUE,
								    _("Creating a new file"),
								    _("Conglomerate now has the information it needs to create the document.  Press \"Apply\" to create it.  You will need to save the document if you wish to keep it."),
								    assistant->logo,
								    assistant->watermark,
								    assistant->top_watermark);
	gnome_druid_append_page(assistant->druid, GNOME_DRUID_PAGE(assistant->final_page));
	gtk_widget_show(GTK_WIDGET(assistant->final_page));
	g_signal_connect(GTK_OBJECT(assistant->final_page), "back", G_CALLBACK(final_page_back), assistant);
	g_signal_connect(GTK_OBJECT(assistant->final_page), "finish", G_CALLBACK(final_page_finish), assistant);

	/* 
	   There doesn't seem to be a way to delete pages from a druid.  Hence we have to create the GUI for all of the factories
	   at the creation of the druid, rather than "on-demand".
	 */
	assistant->hash_table_of_factory_to_page = g_hash_table_new(g_direct_hash, g_direct_equal);
	cong_plugin_manager_for_each_document_factory(cong_app_singleton()->plugin_manager, add_pages_for_factory_callback, assistant);

	gtk_widget_show(GTK_WIDGET(assistant->window));

#if 0
	CONG_DO_UNIMPLEMENTED_DIALOG("The selected menu item has not yet been implemented.");
#endif

	/* FIXME:  need to sort out memory leaks */
}
