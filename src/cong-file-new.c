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
#include "cong-service-document-factory.h"
#include "cong-app.h"

#include "cong-primary-window.h"
#include "cong-plugin-manager.h"

#define DEBUG_FILE_NEW_ASSISTANT 1

typedef struct PerFactoryData PerFactoryData;

/* Data stored per-factory by each assistant */
struct PerFactoryData
{
	gpointer data;
	void (*free_func) (gpointer data);
};

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
	GHashTable *hash_table_of_factory_to_data;
};

static void
set_pixbuf (GtkTreeViewColumn *tree_column,
	    GtkCellRenderer   *cell,
	    GtkTreeModel      *model,
	    GtkTreeIter       *iter,
	    gpointer           user_data)
{
	GdkPixbuf *pixbuf = NULL;
	CongServiceDocumentFactory *factory;
	
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

/**
 * first_page_of_factory_back:
 * @druidpage:
 * @widget:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
first_page_of_factory_back(GnomeDruidPage *druidpage,
			   GtkWidget *widget,
			   gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;

	gnome_druid_set_page(GNOME_DRUID(assistant->druid), GNOME_DRUID_PAGE(assistant->second_page));

	return TRUE;
}

/**
 * last_page_of_factory_next:
 * @druidpage:
 * @widget:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
last_page_of_factory_next(GnomeDruidPage *druidpage,
			  GtkWidget *widget,
			  gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;

	/* Store page, so that the final page's back button takes us back here: */
	assistant->previous_page = druidpage;
	gnome_druid_set_page(GNOME_DRUID(assistant->druid), GNOME_DRUID_PAGE(assistant->final_page));

	return TRUE;
}

/**
 * cong_new_file_assistant_new_page:
 * @assistant:
 * @document_factory:
 * @is_first_of_factory:
 * @is_last_of_factory:
 *
 * TODO: Write me
 * Returns:
 */
GnomeDruidPageStandard *
cong_new_file_assistant_new_page(CongNewFileAssistant *assistant, 
				 CongServiceDocumentFactory *document_factory, 
				 gboolean is_first_of_factory,
				 gboolean is_last_of_factory)
{
	GnomeDruidPageStandard *page;
	gchar *title;

	g_return_val_if_fail(assistant, NULL);
	g_return_val_if_fail(document_factory, NULL);

	title = g_strdup_printf(_("Creating a new file (%s)"), 
				cong_service_get_name(CONG_SERVICE(document_factory)));

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

/**
 * cong_new_file_assistant_set_page:
 * @assistant:
 * @page:
 *
 * TODO: Write me
 */
void 
cong_new_file_assistant_set_page(CongNewFileAssistant *assistant, GnomeDruidPage *page)
{
	g_return_if_fail(assistant);
	g_return_if_fail(page);


	gnome_druid_set_page(GNOME_DRUID(assistant->druid), page);
	
}

/**
 * cong_new_file_assistant_get_toplevel:
 * @assistant:
 *
 * TODO: Write me
 * Returns:
 */
GtkWindow *
cong_new_file_assistant_get_toplevel(CongNewFileAssistant *assistant)
{
	g_return_val_if_fail (assistant, NULL);

	return GTK_WINDOW(assistant->window);
}

/**
 * cong_new_file_assistant_set_data_for_factory:
 * @assistant:
 * @document_factory:
 * @factory_data:
 * @free_func:
 *
 * TODO: Write me
 */
void
cong_new_file_assistant_set_data_for_factory (CongNewFileAssistant *assistant,
					      CongServiceDocumentFactory *document_factory,
					      gpointer factory_data,
					      void (*free_func) (gpointer factory_data))
{
	PerFactoryData *per_factory_data;

	g_return_if_fail (assistant);
	g_return_if_fail (IS_CONG_SERVICE_DOCUMENT_FACTORY (document_factory));


	/* Add space for the per-factory data: */
	per_factory_data = g_hash_table_lookup (assistant->hash_table_of_factory_to_data, 
						document_factory);
	g_assert (per_factory_data);

	/* Free existing data if necessary: */
	if (per_factory_data->data) {
		if (per_factory_data->free_func) {
			(per_factory_data->free_func) (per_factory_data->data);
		}
	}

	per_factory_data->data = factory_data;
	per_factory_data->free_func = free_func;
}

/**
 * cong_new_file_assistant_get_data_for_factory:
 * @assistant:
 * @document_factory:
 *
 * TODO: Write me
 * Returns:
 */
gpointer
cong_new_file_assistant_get_data_for_factory (CongNewFileAssistant *assistant,
					      CongServiceDocumentFactory *document_factory)
{
	PerFactoryData *per_factory_data;

	g_return_val_if_fail (assistant, NULL);
	g_return_val_if_fail (IS_CONG_SERVICE_DOCUMENT_FACTORY (document_factory), NULL);

	per_factory_data = g_hash_table_lookup (assistant->hash_table_of_factory_to_data, 
						document_factory);
	g_assert (per_factory_data);

	return per_factory_data->data;
}

/* Internal functions: */
static void add_factory_callback(CongServiceDocumentFactory *factory, gpointer user_data)
{	
	CongNewFileAssistant *assistant = user_data;
	GtkTreeIter iter;

	g_assert (IS_CONG_SERVICE_DOCUMENT_FACTORY (factory));

	gtk_list_store_append (assistant->list_store, &iter);  /* Acquire an iterator */
			
	gtk_list_store_set (assistant->list_store, &iter,
			    NEWDOCTYPELIST_NAME_COLUMN, cong_service_get_name(CONG_SERVICE(factory)),
			    NEWDOCTYPELIST_DESCRIPTION_COLUMN, cong_service_get_description(CONG_SERVICE(factory)),
			    NEWDOCTYPELIST_FACTORY_COLUMN, factory,
			    -1);

}

/**
 * make_type_selection_widget:
 * @assistant:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget *
make_type_selection_widget(CongNewFileAssistant *assistant)
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
	cong_plugin_manager_for_each_document_factory (cong_app_get_plugin_manager (cong_app_singleton()), 
						       add_factory_callback, 
						       assistant);

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

/**
 * second_page_prepare:
 * @druidpage:
 * @widget:
 * @user_data:
 *
 * TODO: Write me
 */
void 
second_page_prepare(GnomeDruidPage *druidpage,
		    GtkWidget *widget,
		    gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;

#if DEBUG_FILE_NEW_ASSISTANT
	g_message("second_page_prepare");
#endif

	gnome_druid_set_buttons_sensitive(GNOME_DRUID(assistant->druid),
					  TRUE, /* gboolean back_sensitive, */
					  FALSE, /* gboolean next_sensitive, */
					  TRUE, /* gboolean cancel_sensitive, */
					  TRUE); /* gboolean help_sensitive */

}

/**
 * second_page_next:
 * @druidpage:
 * @widget:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
second_page_next(GnomeDruidPage *druidpage,
		 GtkWidget *widget,
		 gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;
	GtkTreeSelection* tree_selection;
	GtkTreeModel *tree_model;
	GtkTreeIter iter;

#if DEBUG_FILE_NEW_ASSISTANT
	g_message("second_page_next");
#endif

	/* Get selected factory */
	tree_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(assistant->list_view));

	if (gtk_tree_selection_get_selected (tree_selection,
                                             &tree_model,
                                             &iter) ) {
		CongServiceDocumentFactory *factory;		
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
#if DEBUG_FILE_NEW_ASSISTANT
			g_message("haven't got a third page registered for factory \"%s\"", 
				  cong_service_get_name(CONG_SERVICE(factory)));
#endif
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

/**
 * final_page_back:
 * @druidpage:
 * @widget:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
final_page_back(GnomeDruidPage *druidpage,
		GtkWidget *widget,
		gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;

	/* go back to the stored previous page: */
	gnome_druid_set_page(GNOME_DRUID(assistant->druid), GNOME_DRUID_PAGE(assistant->previous_page));

	return TRUE;
}

/**
 * final_page_finish:
 * @druidpage:
 * @widget:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
final_page_finish(GnomeDruidPage *druidpage,
		  GtkWidget *widget,
		  gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;
	GtkTreeSelection* tree_selection;
	GtkTreeModel *tree_model;
	GtkTreeIter iter;

#if DEBUG_FILE_NEW_ASSISTANT
	g_message("final_page_finish");
#endif

	/* Get selected factory */
	tree_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(assistant->list_view));

	if (gtk_tree_selection_get_selected (tree_selection,
                                             &tree_model,
                                             &iter) ) {
		CongServiceDocumentFactory *factory;		

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

static void
add_pages_for_factory_callback (CongServiceDocumentFactory *factory, 
				gpointer user_data)
{
	CongNewFileAssistant *assistant = user_data;
	PerFactoryData *per_factory_data; 


	/* Add space for the per-factory data: */
	per_factory_data = g_new0 (PerFactoryData, 1);

	g_hash_table_insert (assistant->hash_table_of_factory_to_data, 
			     factory, 
			     per_factory_data);
	/* FIXME: we currently don't clean this up, or call the free func */

	/* Invoke the callback to create pages: */
	cong_document_factory_invoke_page_creation_callback(factory, assistant);
}

/**
 * new_document:
 * @parent_window:
 *
 * TODO: Write me
 */
void 
new_document(GtkWindow *parent_window)
{
	CongNewFileAssistant *assistant;

	assistant = g_new0(CongNewFileAssistant,1);

	/* FIXME:  what if no document factories found? */

	/* We don't use gnome_druid_new_with_window since this shows the window, and we've got lots of construction work to do before that should happen: */
	assistant->druid = GNOME_DRUID (gnome_druid_new ());
	gtk_widget_show (GTK_WIDGET (assistant->druid));

	assistant->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title (GTK_WINDOW (assistant->window), 
			      _("Creating a new file"));

	if (parent_window != NULL) {
		gtk_window_set_transient_for (GTK_WINDOW (assistant->window),
					      parent_window);
	}	
	
	gtk_container_add (GTK_CONTAINER (assistant->window), 
			   GTK_WIDGET (assistant->druid));
	
	g_signal_connect_object (assistant->druid, "cancel",
				 G_CALLBACK (gtk_widget_destroy),
				 assistant->window,
				 G_CONNECT_SWAPPED);
	
	/* When the druid gets destroyed so does the window */
	g_signal_connect_object (assistant->druid, "destroy",
				 G_CALLBACK (gtk_widget_destroy),
				 assistant->window,
				 G_CONNECT_SWAPPED);

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
	assistant->hash_table_of_factory_to_data = g_hash_table_new (g_direct_hash, 
								     g_direct_equal);
	cong_plugin_manager_for_each_document_factory (cong_app_get_plugin_manager (cong_app_singleton()), 
						       add_pages_for_factory_callback, 
						       assistant);

	gtk_widget_show(GTK_WIDGET(assistant->window));

#if 0
	CONG_DO_UNIMPLEMENTED_DIALOG("The selected menu item has not yet been implemented.");
#endif

	/* FIXME:  need to sort out memory leaks */
}

/**
 * toolbar_callback_new:
 * @w:
 * @data:
 *
 * TODO: Write me
 * Returns:
 */
gint 
toolbar_callback_new(GtkWidget *widget, gpointer data)
{
	CongPrimaryWindow *primary_window = data;

	new_document(cong_primary_window_get_toplevel(primary_window));

	return TRUE;
}
