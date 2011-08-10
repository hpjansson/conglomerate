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

#define DEBUG_FILE_NEW_ASSISTANT 0

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
	GtkAssistant *druid;
	GtkWidget *first_page;
	GtkWidget *second_page;
	GtkWidget *final_page;
	GtkWidget *previous_page;
	GtkWidget *type_selection_widget;
	GdkPixbuf *logo;
	GdkPixbuf *watermark;
	GdkPixbuf *top_watermark;

	GtkListStore *list_store;
	GtkWidget *list_view;

	/* Mapping from factories to pages; the "first" page for each factory, actually the third page within the druid */
	GHashTable *hash_table_of_factory_to_page;
	/* Also the "last" page for each factory */
	GHashTable *hash_table_of_factory_to_last_page;
	GHashTable *hash_table_of_factory_to_data;
};

static void
on_type_selection_changed(GtkTreeSelection *tree_selection, CongNewFileAssistant *assistant);

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

static void
format_page(CongNewFileAssistant *assistant, GtkWidget *page)
{
	gtk_assistant_set_page_header_image(assistant->druid, page, assistant->top_watermark);
	gtk_assistant_set_page_side_image(assistant->druid, page, assistant->logo);
	gtk_widget_show(page);
}

static GtkWidget *
add_new_page(CongNewFileAssistant *assistant, const char *text, GtkAssistantPageType type)
{
	GtkWidget *page = gtk_label_new(text);
	gtk_label_set_line_wrap(GTK_LABEL(page), TRUE);
	gtk_assistant_append_page(assistant->druid, page);
	gtk_assistant_set_page_type(assistant->druid, page, type);
	gtk_assistant_set_page_title(assistant->druid, page, _("Creating a new file"));
	format_page(assistant, page);
	return page;
}

/* Build the page to look more or less like GnomeDruid would have done it */
static GtkWidget *
add_new_page_with_widget(CongNewFileAssistant *assistant,
                         const char *title,
                         const char *question,
                         const char *info,
                         GtkWidget *widget)
{
	GtkWidget *page, *top_label, *bottom_label;
	char *text;

	page = gtk_vbox_new(FALSE, 0);
	if(question) {
		top_label = gtk_label_new(question);
		g_object_set(top_label,
			     "use-markup", TRUE,
			     "use-underline", TRUE,
			     "mnemonic-widget", widget,
			     "justify", GTK_JUSTIFY_LEFT,
			     "xalign", 0.0,
			     NULL);
		gtk_box_pack_start(GTK_BOX(page), top_label, FALSE, FALSE, 0);
	}
	gtk_box_pack_start(GTK_BOX(page), widget, FALSE, FALSE, 0);
	if(info) {
		text = g_strconcat("<span size=\"small\">", info, "</span>", NULL);
		bottom_label = gtk_label_new(text);
		g_free(text);
		g_object_set(bottom_label,
			     "use-markup", TRUE,
			     "justify", GTK_JUSTIFY_LEFT,
			     "xalign", 0.0,
			     "xpad", 24,
			     NULL);
		gtk_box_pack_start(GTK_BOX(page), bottom_label, FALSE, FALSE, 0);
	}

	gtk_assistant_append_page(assistant->druid, page);
	gtk_assistant_set_page_type(assistant->druid, page, GTK_ASSISTANT_PAGE_CONTENT);
	gtk_assistant_set_page_title(assistant->druid, page, title);
	gtk_assistant_set_page_complete(assistant->druid, page, TRUE);
	format_page(assistant, page);
	return page;
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
GtkWidget *
cong_new_file_assistant_new_page(CongNewFileAssistant *assistant, 
				 CongServiceDocumentFactory *document_factory, 
                                 GtkWidget *content,
                                 const char *question,
                                 const char *info,
				 gboolean is_first_of_factory,
				 gboolean is_last_of_factory)
{
	GtkWidget *page;
	gchar *title;

	g_return_val_if_fail(assistant, NULL);
	g_return_val_if_fail(document_factory, NULL);

	title = g_strdup_printf(_("Creating a new file (%s)"), 
				cong_service_get_name(CONG_SERVICE(document_factory)));
	page = add_new_page_with_widget(assistant, title, question, info, content);
	g_free(title);

	if (is_first_of_factory) {
		/* add to mapping from factories to pages: */
		g_hash_table_insert(assistant->hash_table_of_factory_to_page, document_factory, page);
	}

	if (is_last_of_factory) {
		g_hash_table_insert(assistant->hash_table_of_factory_to_last_page, document_factory, page);
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
cong_new_file_assistant_set_page(CongNewFileAssistant *assistant, GtkWidget *page)
{
	g_return_if_fail(assistant);
	g_return_if_fail(page);

	int count;
	for(count = gtk_assistant_get_n_pages(assistant->druid); count >= 0; count--) {
		if(gtk_assistant_get_nth_page(assistant->druid, count) == page) {
			gtk_assistant_set_current_page(assistant->druid, count);
			break;
		}
	}
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

	return GTK_WINDOW(assistant->druid);
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
	GtkTreeSelection *selection;

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

	/* Listen for a selection */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(assistant->list_view));
	g_signal_connect(selection, "changed", G_CALLBACK(on_type_selection_changed), assistant);

#if 1
	return assistant->list_view;
#else
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), list_view);
	gtk_widget_show(scrolled_window);
	return scrolled_window;
#endif
}

static void
on_type_selection_changed(GtkTreeSelection *tree_selection, CongNewFileAssistant *assistant)
{
	GtkTreeModel *tree_model;
	GtkTreeIter iter;
	int n_pages, count;

#if DEBUG_FILE_NEW_ASSISTANT
	g_message("on_type_selection_changed");
#endif
	/* Hide all factory pages (all except 0, 1 and last) */
	n_pages = gtk_assistant_get_n_pages(assistant->druid);
	for(count = 2; count < n_pages - 1; count++) {
		GtkWidget *page = gtk_assistant_get_nth_page(assistant->druid, count);
		gtk_widget_hide_all(page);
	}

	/* Get selected factory */
	if (gtk_tree_selection_get_selected (tree_selection,
                                             &tree_model,
                                             &iter) ) {
		CongServiceDocumentFactory *factory;		
		GtkWidget *appropriate_third_page, *factory_last_page;

		gtk_tree_model_get(tree_model,
				   &iter,
				   NEWDOCTYPELIST_FACTORY_COLUMN,
				   &factory,
				   -1);

		/* Find the relevant page for this factory within the druid: */
		appropriate_third_page = g_hash_table_lookup(assistant->hash_table_of_factory_to_page,
							     factory);
		factory_last_page = g_hash_table_lookup(assistant->hash_table_of_factory_to_last_page,
		                                        factory);

		if (appropriate_third_page) {
			if (appropriate_third_page == factory_last_page) {
				gtk_widget_show_all(appropriate_third_page);
			} else {
				gboolean show_page = FALSE;
				for(count = 2; count < n_pages - 1; count++) {
					GtkWidget *page = gtk_assistant_get_nth_page(assistant->druid, count);
					if(page == appropriate_third_page)
						show_page = TRUE;
					if(show_page)
						gtk_widget_show_all(page);
					if(page == factory_last_page)
						show_page = FALSE;
				}
			}
		} else {
			/* what to do in this case? */
#if DEBUG_FILE_NEW_ASSISTANT
			g_message("haven't got a third page registered for factory \"%s\"", 
				  cong_service_get_name(CONG_SERVICE(factory)));
#endif
			/* go to final page, by keeping all the in-between pages hidden */
		}

#if 0
#error		
		cong_document_factory_invoke_action_callback(factory, assistant);
#endif

		/* Allow advancing to the next page */
		gtk_assistant_set_page_complete(assistant->druid, assistant->second_page, TRUE);
	} else {
		/* Nothing selected; don't advance to the next page */
		gtk_assistant_set_page_complete(assistant->druid, assistant->second_page, FALSE);
	}

}

static void
on_apply(GtkAssistant *widget, CongNewFileAssistant *assistant)
{
	GtkTreeSelection* tree_selection;
	GtkTreeModel *tree_model;
	GtkTreeIter iter;

#if DEBUG_FILE_NEW_ASSISTANT
	g_message("on_apply");
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
	} else {
		g_warning("No selected factory");
	}

	/* The assistant closes automatically */
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

	assistant->druid = GTK_ASSISTANT(gtk_assistant_new());
	gtk_widget_show (GTK_WIDGET (assistant->druid));
	g_printerr("Number of pages: %d\n", gtk_assistant_get_n_pages(assistant->druid));

	gtk_window_set_title (GTK_WINDOW (assistant->druid),
			      _("Creating a new file"));

	if (parent_window != NULL) {
		gtk_window_set_transient_for (GTK_WINDOW (assistant->druid),
					      parent_window);
	}	
	
	g_signal_connect_object (assistant->druid, "cancel",
				 G_CALLBACK (gtk_widget_destroy),
				 assistant->druid,
				 G_CONNECT_SWAPPED);
	g_signal_connect (assistant->druid, "apply",
                     G_CALLBACK (on_apply),
                     assistant);
	g_signal_connect (assistant->druid, "close",
                     G_CALLBACK (gtk_widget_destroy),
                     NULL);

	assistant->first_page = add_new_page(assistant, _("This assistant will "
		"guide you through creating a new file.\n\n"
		"Various types of file are available, and Conglomerate may be "
		"able to supply some of the \"boilerplate\" content for you.\n\n"
		"We hope that in future versions of Conglomerate you will be "
		"able to create \"template documents\" to add to this system."),
	                                     GTK_ASSISTANT_PAGE_INTRO);
	gtk_assistant_set_page_complete(assistant->druid, assistant->first_page, TRUE);

	assistant->type_selection_widget = make_type_selection_widget(assistant);
	assistant->second_page = add_new_page_with_widget(assistant,
	                                                  _("Creating a new file"),
	                                                  _("What type of file would you like to create?"),
	                                                  _("Highlight one of the items in the list and select \"Forward\" to continue"),
	                                                  assistant->type_selection_widget);
	gtk_widget_show_all(assistant->second_page);

	/* 
	   There doesn't seem to be a way to delete pages from a druid.  Hence we have to create the GUI for all of the factories
	   at the creation of the druid, rather than "on-demand".
	   FIXME: With GtkAssistant, gtk_container_remove() should work.
	 */
	assistant->hash_table_of_factory_to_page = g_hash_table_new(g_direct_hash, g_direct_equal);
	assistant->hash_table_of_factory_to_last_page = g_hash_table_new(g_direct_hash, g_direct_equal);
	assistant->hash_table_of_factory_to_data = g_hash_table_new (g_direct_hash, 
								     g_direct_equal);
	cong_plugin_manager_for_each_document_factory (cong_app_get_plugin_manager (cong_app_singleton()), 
						       add_pages_for_factory_callback, 
						       assistant);

	assistant->final_page = add_new_page(assistant, _("Conglomerate now has the "
		"information it needs to create the document.  Press \"Apply\" "
		"to create it.  You will need to save the document if you wish "
		"to keep it."), GTK_ASSISTANT_PAGE_CONFIRM);
	gtk_assistant_set_page_complete(assistant->druid, assistant->final_page, TRUE);

	gtk_assistant_set_current_page(assistant->druid, 0);
	gtk_widget_show(GTK_WIDGET(assistant->druid));

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
