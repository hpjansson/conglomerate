/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
  Testbed for working with MVC
 */

#include <gtk/gtk.h>
#include "global.h"

enum {
	TESTVIEW_COLUMN_TEXT,
	TESTVIEW_COLUMN_NODE,
	TESTVIEW_NUM_COLUMNS
};

typedef struct CongTestViewDetails
{

	GtkScrolledWindow *scrolled_window;
	GtkTreeStore *tree_store;
	GtkTreeView *tree_view;

} CongTestViewDetails;

#if NEW_XML_IMPLEMENTATION
gchar* cleanup_text(xmlChar *text) {
	gchar *buffer = g_malloc(strlen(text)*3); /* for safety's sake */
	gchar *dst = buffer;

	/* FIXME: audit for character set issues */

	while (*text) {
		switch (*text) {
		default:
			*(dst++) = *text;
			break;
		case '\n':
			*(dst++) = '\\';
			*(dst++) = 'n';
			break;
		case '\t':
			*(dst++) = '\\';
			*(dst++) = 't';
			break;
		}

		text++;
	}

	*(dst++) = '\0';

	return buffer;
}

void populate_tree_store_recursive(CongTestViewDetails *details, CongNodePtr node, GtkTreeIter* tree_iter)
{
	CongNodePtr node_iter;
	gchar *text = NULL;
	gchar *cleaned_text = NULL;

	switch (cong_node_type(node)) {
	default: g_assert(0);
	case CONG_NODE_TYPE_UNKNOWN: 
		text = g_strdup("UNKNOWN");
		break;

	case CONG_NODE_TYPE_ELEMENT: 
		text = g_strdup_printf("<%s>", cong_node_name(node));
		break;

	case CONG_NODE_TYPE_TEXT:
		cleaned_text = cleanup_text(node->content);
		text = g_strdup_printf("Text: \"%s\"", cleaned_text);
		g_free(cleaned_text);
		break; 
		
	case CONG_NODE_TYPE_COMMENT:
		cleaned_text = cleanup_text(node->content);
		text = g_strdup_printf("Comment: \"%s\"", cleaned_text);
		g_free(cleaned_text);
		break;
	}

	g_assert(text);

	gtk_tree_store_set (details->tree_store, 
			    tree_iter,
			    TESTVIEW_COLUMN_TEXT, text,
			    TESTVIEW_COLUMN_NODE, node,
			    -1);

	g_free(text);


	for (node_iter = cong_node_first_child(node); node_iter; node_iter = node_iter->next) {
		GtkTreeIter child_tree_iter;
		gtk_tree_store_append(details->tree_store, &child_tree_iter, tree_iter);

		populate_tree_store_recursive(details, node_iter, &child_tree_iter);
	}	
}
#endif

GtkWidget *cong_test_view_new(CongDocument *doc)
{
	CongTestViewDetails *details;
	GtkCellRenderer *renderer;
 	GtkTreeViewColumn *column;
	GtkTreeIter root_iter;

	g_return_val_if_fail(doc, NULL);

	details = g_new0(CongTestViewDetails,1);

	details->scrolled_window = GTK_SCROLLED_WINDOW( gtk_scrolled_window_new(NULL, NULL) );
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(details->scrolled_window), 
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(GTK_WIDGET(details->scrolled_window), 100, 50);

        details->tree_store = gtk_tree_store_new (TESTVIEW_NUM_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
	details->tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model (GTK_TREE_MODEL(details->tree_store)));

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(details->scrolled_window), GTK_WIDGET(details->tree_view));

	renderer = gtk_cell_renderer_text_new ();

	/* Create a column, associating the "text" attribute of the
	 * cell_renderer to the first column of the model */
	column = gtk_tree_view_column_new_with_attributes ("Element", renderer,
							   "text", TESTVIEW_COLUMN_TEXT,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (details->tree_view), column);

	gtk_widget_show(GTK_WIDGET(details->tree_view));
	gtk_widget_show(GTK_WIDGET(details->scrolled_window));

	/* Populate the tree: */
	gtk_tree_store_append (details->tree_store, &root_iter, NULL);  /* Acquire a top-level iterator */

#if NEW_XML_IMPLEMENTATION
	populate_tree_store_recursive(details, cong_document_get_root(doc), &root_iter);
#endif

	return GTK_WIDGET(details->scrolled_window);	
}


