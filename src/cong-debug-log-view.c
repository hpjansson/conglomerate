/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
  Testbed for working with MVC.

  Has not yet had strings marked for translation.
 */

#include <gtk/gtk.h>
#include "global.h"
#include "cong-document.h"
#include "cong-view.h"

#define DEBUG_DEBUG_LOG_VIEW 0

enum {
	DEBUGLOGVIEW_COLUMN_MESSAGE_NAME,
	DEBUGLOGVIEW_COLUMN_BEFOREORAFTER,
	DEBUGLOGVIEW_COLUMN_NODE,
	DEBUGLOGVIEW_COLUMN_EXTRA_INFO,
	DEBUGLOGVIEW_NUM_COLUMNS
};

#define CONG_DEBUG_LOG_VIEW(x) ((CongDebugLogView*)(x))

typedef struct CongDebugLogView
{
	CongView view;

	struct CongDebugLogViewDetails *private;
} CongDebugLogView;

typedef struct CongDebugLogViewDetails
{

	GtkScrolledWindow *scrolled_window;
	GtkListStore *list_store;
	GtkTreeView *tree_view;

} CongDebugLogViewDetails;

static void debug_low_view_add_message(CongDebugLogView *debug_log_view,
				       const gchar *description, 
				       gboolean before_event,
				       const gchar *node_id, 
				       const gchar *extra_info)
{
	GtkTreeIter iter;
	CongDebugLogViewDetails* details;

	g_return_if_fail(debug_log_view);
	g_return_if_fail(description);

	details = debug_log_view->private;
	g_assert(details);

	/* Append to the end of the list: */
	gtk_list_store_append(details->list_store,
			      &iter);

	gtk_list_store_set(details->list_store,
			   &iter,
			   DEBUGLOGVIEW_COLUMN_MESSAGE_NAME,
			   description,
			   DEBUGLOGVIEW_COLUMN_BEFOREORAFTER,
			   (before_event?"before":"after"),
			   DEBUGLOGVIEW_COLUMN_NODE,
			   node_id,
			   DEBUGLOGVIEW_COLUMN_EXTRA_INFO,
			   extra_info,
			   -1);
}

/* Prototypes of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent);
static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling);
static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling);
static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content);
static void on_selection_change(CongView *view);
static void on_cursor_change(CongView *view);

/* Definitions of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent)
{
	CongDebugLogView *debug_log_view;
	GtkTreeIter tree_iter;

	g_return_if_fail(view);
	g_return_if_fail(node);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);

	debug_low_view_add_message(debug_log_view, "Make orphan", before_event, "", "");
}

static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling)
{
	CongDebugLogView *debug_log_view;
	GtkTreeIter tree_iter_sibling;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(older_sibling);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);

	debug_low_view_add_message(debug_log_view, "Add node after existing child", before_event, "", "");

}

static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling)
{
	CongDebugLogView *debug_log_view;
	GtkTreeIter tree_iter_sibling;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);

	debug_low_view_add_message(debug_log_view, "Add node before existing child", before_event, "", "");
}

static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent)
{
	CongDebugLogView *debug_log_view;
	GtkTreeIter tree_iter_node;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);

	debug_low_view_add_message(debug_log_view, "Set parent", before_event, "", "");
}

static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content)
{
	CongDebugLogView *debug_log_view;
	GtkTreeIter tree_iter;
	gchar *cleaned_text;
	gchar *node_name;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);

	node_name = cong_node_get_path(node);
	cleaned_text = cong_util_cleanup_text(new_content);

	debug_low_view_add_message(debug_log_view, "Set text", before_event, node_name, cleaned_text);

	g_free(node_name);
	g_free(cleaned_text);
}

static void on_selection_change(CongView *view)
{
	CongDebugLogView *debug_log_view;

	g_return_if_fail(view);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);

	debug_low_view_add_message(debug_log_view, "Selection change", FALSE, "", "");
}

static void on_cursor_change(CongView *view)
{
}

static void add_text_column(GtkTreeView *tree_view,
			    const gchar *column_title,
			    gint model_column)
{
	GtkCellRenderer *renderer;
 	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (column_title, 
							   renderer,
							   "text", model_column,
							   NULL);
	gtk_tree_view_append_column (tree_view, column);
}

GtkWidget *cong_debug_log_view_new(CongDocument *doc)
{
	CongDebugLogViewDetails *details;
	CongDebugLogView *view;

	g_return_val_if_fail(doc, NULL);

	view = g_new0(CongDebugLogView,1);
	details = g_new0(CongDebugLogViewDetails,1);
	
	view->private = details;
	
	view->view.doc = doc;
	view->view.klass = g_new0(CongViewClass,1);
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

        details->list_store = gtk_list_store_new (DEBUGLOGVIEW_NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	details->tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model (GTK_TREE_MODEL(details->list_store)));

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(details->scrolled_window), GTK_WIDGET(details->tree_view));


	/* Create columns, associating the "text" attribute of the
	 * cell_renderer to each column of the model */
	add_text_column(GTK_TREE_VIEW (details->tree_view),
			"Message",
			DEBUGLOGVIEW_COLUMN_MESSAGE_NAME);
	add_text_column(GTK_TREE_VIEW (details->tree_view),
			"Before/After",
			DEBUGLOGVIEW_COLUMN_BEFOREORAFTER);
	add_text_column(GTK_TREE_VIEW (details->tree_view),
			"Node",
			DEBUGLOGVIEW_COLUMN_NODE);
	add_text_column(GTK_TREE_VIEW (details->tree_view),
			"Additional Info",
			DEBUGLOGVIEW_COLUMN_EXTRA_INFO);

	gtk_widget_show(GTK_WIDGET(details->tree_view));
	gtk_widget_show(GTK_WIDGET(details->scrolled_window));

	return GTK_WIDGET(details->scrolled_window);	
}


