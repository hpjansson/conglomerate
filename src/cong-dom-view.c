/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
  Testbed for working with MVC
 */

#include <gtk/gtk.h>
#include "global.h"
#include "cong-document.h"
#include "cong-view.h"

#define DEBUG_DOM_VIEW 0

enum {
	DOMVIEW_COLUMN_TEXT,
	DOMVIEW_COLUMN_NODE,
	DOMVIEW_NUM_COLUMNS
};

#define CONG_DOM_VIEW(x) ((CongDOMView*)(x))

typedef struct CongDOMView
{
	CongView view;

	struct CongDOMViewDetails *private;
} CongDOMView;

typedef struct CongDOMViewDetails
{

	GtkScrolledWindow *scrolled_window;
	GtkTreeStore *tree_store;
	GtkTreeView *tree_view;

} CongDOMViewDetails;

struct search_struct
{
	CongNodePtr node;

	gboolean found_it;	
	
	GtkTreeIter *tree_iter;
};

static gboolean search_for_node(GtkTreeModel *model,
				GtkTreePath *path,
				GtkTreeIter *iter,
				gpointer data)
{
	struct search_struct* search = (struct search_struct*)data;
	CongNodePtr this_node;
	
	g_assert(model);
	g_assert(path);
	g_assert(iter);
	g_assert(search);

	gtk_tree_model_get(model,
			   iter,
			   DOMVIEW_COLUMN_NODE,
			   &this_node,
			   -1);

	if (this_node==search->node) {
		search->found_it = TRUE;
		*search->tree_iter = *iter;
		return TRUE;
	} else {
		return FALSE;
	}

}

static gboolean get_iter_for_node(CongDOMViewDetails *details, CongNodePtr node, GtkTreeIter* tree_iter)
{
	/* FIXME: this is O(n), it ought to be O(1), by adding some kind of search structure */
	struct search_struct search;
	
	search.node = node;
	search.found_it = FALSE;
	search.tree_iter = tree_iter;

	gtk_tree_model_foreach(GTK_TREE_MODEL(details->tree_store),
			       search_for_node,
			       &search);

	return search.found_it;
}

static void populate_tree_store_recursive(CongDOMViewDetails *details, CongNodePtr node, GtkTreeIter* tree_iter)
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
		cleaned_text = cong_util_cleanup_text(node->content);
		text = g_strdup_printf("Text: \"%s\"", cleaned_text);
		g_free(cleaned_text);
		break; 
		
	case CONG_NODE_TYPE_COMMENT:
		cleaned_text = cong_util_cleanup_text(node->content);
		text = g_strdup_printf("Comment: \"%s\"", cleaned_text);
		g_free(cleaned_text);
		break;
	}

	g_assert(text);

	gtk_tree_store_set (details->tree_store, 
			    tree_iter,
			    DOMVIEW_COLUMN_TEXT, text,
			    DOMVIEW_COLUMN_NODE, node,
			    -1);

	g_free(text);


	for (node_iter = cong_node_first_child(node); node_iter; node_iter = node_iter->next) {
		GtkTreeIter child_tree_iter;
		gtk_tree_store_append(details->tree_store, &child_tree_iter, tree_iter);

		populate_tree_store_recursive(details, node_iter, &child_tree_iter);
	}	
}

/* Prototypes of the handler functions: */
static void on_document_coarse_update(CongView *view);
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent);
static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling);
static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling);
static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content);
static void on_selection_change(CongView *view);
static void on_cursor_change(CongView *view);

/* Definitions of the handler functions: */
static void on_document_coarse_update(CongView *view)
{
	CongDOMView *test_view;

	g_return_if_fail(view);

	#if DEBUG_DOM_VIEW
	g_message("CongDOMView - on_document_coarse_update\n");
	#endif

	test_view = CONG_DOM_VIEW(view);

	/* Ignore for now */
}

static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent)
{
	CongDOMView *test_view;
	GtkTreeIter tree_iter;

	g_return_if_fail(view);
	g_return_if_fail(node);

	#if DEBUG_DOM_VIEW
	g_message("CongDOMView - on_document_node_make_orphan\n");
	#endif

	if (before_event) {
		return;
	}

	test_view = CONG_DOM_VIEW(view);

	if ( get_iter_for_node(test_view->private, node, &tree_iter) ) {

		/* Remove this branch of the tree: */
		gtk_tree_store_remove(test_view->private->tree_store, &tree_iter);

	}
}

static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling)
{
	CongDOMView *test_view;
	GtkTreeIter tree_iter_sibling;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(older_sibling);

	#if DEBUG_DOM_VIEW
	g_message("CongDOMView - on_document_node_add_after\n");
	#endif

	if (before_event) {
		return;
	}

	test_view = CONG_DOM_VIEW(view);

	if ( get_iter_for_node(test_view->private, older_sibling, &tree_iter_sibling) ) {

		if ( get_iter_for_node(test_view->private, older_sibling->parent, &tree_iter_parent) ) {

			GtkTreeIter new_tree_iter;
			gtk_tree_store_insert_after(test_view->private->tree_store, &new_tree_iter, &tree_iter_parent, &tree_iter_sibling);

			populate_tree_store_recursive(test_view->private, node, &new_tree_iter);
		}
	}
}

static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling)
{
	CongDOMView *test_view;
	GtkTreeIter tree_iter_sibling;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);

	#if DEBUG_DOM_VIEW
	g_message("CongDOMView - on_document_node_add_before\n");
	#endif

	if (before_event) {
		return;
	}

	test_view = CONG_DOM_VIEW(view);

	if ( get_iter_for_node(test_view->private, younger_sibling, &tree_iter_sibling) ) {

		if ( get_iter_for_node(test_view->private, younger_sibling->parent, &tree_iter_parent) ) {
		
			GtkTreeIter new_tree_iter;
			gtk_tree_store_insert_before(test_view->private->tree_store, &new_tree_iter, &tree_iter_parent, &tree_iter_sibling);

			populate_tree_store_recursive(test_view->private, node, &new_tree_iter);
		}
	}
}

static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent)
{
	CongDOMView *test_view;
	GtkTreeIter tree_iter_node;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);

	#if DEBUG_DOM_VIEW
	g_message("CongDOMView - on_document_node_set_parent\n");
	#endif

	if (before_event) {
		return;
	}

	test_view = CONG_DOM_VIEW(view);

	if ( get_iter_for_node(test_view->private, node, &tree_iter_node) ) {
		/* Remove this branch of the tree: */
		gtk_tree_store_remove(test_view->private->tree_store, &tree_iter_node);
	}

	if ( get_iter_for_node(test_view->private, adoptive_parent, &tree_iter_parent) ) {
		GtkTreeIter new_tree_iter;
		gtk_tree_store_append(test_view->private->tree_store, &new_tree_iter, &tree_iter_parent);

		populate_tree_store_recursive(test_view->private, node, &new_tree_iter);
	}
}

static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content)
{
	CongDOMView *test_view;
	GtkTreeIter tree_iter;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_DOM_VIEW
	g_message("CongDOMView - on_document_node_set_text\n");
	#endif

	if (before_event) {
		return;
	}

	test_view = CONG_DOM_VIEW(view);

	if ( get_iter_for_node(test_view->private, node, &tree_iter) ) {
		gchar *text = NULL;
		gchar *cleaned_text = NULL;

		g_assert(cong_node_type(node) == CONG_NODE_TYPE_TEXT);

		cleaned_text = cong_util_cleanup_text(node->content);
		text = g_strdup_printf("Text: \"%s\"", cleaned_text);
		g_free(cleaned_text);
		
		g_assert(text);

		gtk_tree_store_set(test_view->private->tree_store, 
				   &tree_iter,
				   DOMVIEW_COLUMN_TEXT, text,
				   -1);

		g_free(text);
	}
}

static void on_selection_change(CongView *view)
{
}

static void on_cursor_change(CongView *view)
{
}

GtkWidget *cong_dom_view_new(CongDocument *doc)
{
	CongDOMViewDetails *details;
	CongDOMView *view;
	GtkCellRenderer *renderer;
 	GtkTreeViewColumn *column;
	GtkTreeIter root_iter;

	g_return_val_if_fail(doc, NULL);

	view = g_new0(CongDOMView,1);
	details = g_new0(CongDOMViewDetails,1);
	
	view->private = details;
	
	view->view.doc = doc;
	view->view.klass = g_new0(CongViewClass,1);
	view->view.klass->on_document_coarse_update = on_document_coarse_update;
	view->view.klass->on_document_node_make_orphan = on_document_node_make_orphan;
	view->view.klass->on_document_node_add_after = on_document_node_add_after;
	view->view.klass->on_document_node_add_before = on_document_node_add_before;
	view->view.klass->on_document_node_set_parent = on_document_node_set_parent;
	view->view.klass->on_document_node_set_text = on_document_node_set_text;
	view->view.klass->on_selection_change = on_selection_change;
	view->view.klass->on_cursor_change = on_cursor_change;

	cong_document_register_view( doc, CONG_VIEW(view) );
	
	details->scrolled_window = GTK_SCROLLED_WINDOW( gtk_scrolled_window_new(NULL, NULL) );
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(details->scrolled_window), 
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(GTK_WIDGET(details->scrolled_window), 100, 50);

        details->tree_store = gtk_tree_store_new (DOMVIEW_NUM_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
	details->tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model (GTK_TREE_MODEL(details->tree_store)));

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(details->scrolled_window), GTK_WIDGET(details->tree_view));

	renderer = gtk_cell_renderer_text_new ();

	/* Create a column, associating the "text" attribute of the
	 * cell_renderer to the first column of the model */
	column = gtk_tree_view_column_new_with_attributes ("Element", renderer,
							   "text", DOMVIEW_COLUMN_TEXT,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (details->tree_view), column);

	gtk_widget_show(GTK_WIDGET(details->tree_view));
	gtk_widget_show(GTK_WIDGET(details->scrolled_window));

	/* Populate the tree: */
	gtk_tree_store_append (details->tree_store, &root_iter, NULL);  /* Acquire a top-level iterator */

	populate_tree_store_recursive(details, cong_document_get_root(doc), &root_iter);

	return GTK_WIDGET(details->scrolled_window);	
}


