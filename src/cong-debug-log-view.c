/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
  Testbed for working with MVC.

  Has not yet had strings marked for translation.
 */

#include <gtk/gtk.h>
#include "global.h"
#include "cong-document.h"
#include "cong-view.h"
#include "cong-util.h"

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

static void debug_log_view_details_add_message (CongDebugLogViewDetails *details,
						const gchar *description, 
						gboolean before_event,
						const gchar *node_id, 
						const gchar *extra_info)
{
	GtkTreeIter iter;

	/* Append to the end of the list: */
	gtk_list_store_append (details->list_store,
			       &iter);

	gtk_list_store_set (details->list_store,
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

#if 0
static void debug_log_view_add_message (CongDebugLogView *debug_log_view,
					const gchar *description, 
					gboolean before_event,
					const gchar *node_id, 
					const gchar *extra_info)
{
	CongDebugLogViewDetails* details;

	g_return_if_fail(debug_log_view);
	g_return_if_fail(description);

	details = debug_log_view->private;
	g_assert(details);

	debug_log_view_details_add_message(details,
					   description, 
					   before_event,
					   node_id, 
					   extra_info);
}
#endif

/**
 * log_begin_edit:
 * @details:
 * @before_event:
 *
 * TODO: Write me
 */

void 
log_begin_edit (CongDebugLogViewDetails *details,
		gboolean before_event)
{
	debug_log_view_details_add_message(details, "Begin edit", before_event, "", "");
}

/**
 * log_end_edit:
 * @details:
 * @before_event:
 *
 * TODO: Write me
 */
void 
log_end_edit (CongDebugLogViewDetails *details,
	      gboolean before_event)
{
	debug_log_view_details_add_message(details, "End edit", before_event, "", "");
}

/**
 * log_make_orphan:
 * @details:
 * @before_event:
 * @node:
 *
 * TODO: Write me
 */
void 
log_make_orphan (CongDebugLogViewDetails *details, 
		 gboolean before_event, 
		 CongNodePtr node)
{ 
	debug_log_view_details_add_message(details, "Make orphan", before_event, "", "");
}

/**
 * log_add_after:
 * @details:
 * @before_event:
 * @node:
 * @older_sibling:
 *
 * TODO: Write me
 */
void 
log_add_after (CongDebugLogViewDetails *details, 
	       gboolean before_event, 
	       CongNodePtr node, 
	       CongNodePtr older_sibling) 
{ 
	debug_log_view_details_add_message(details, "Add node after existing child", before_event, "", "");
}

/**
 * log_add_before:
 * @details:
 * @before_event:
 * @node:
 * @younger_sibling:
 *
 * TODO: Write me
 */
void 
log_add_before (CongDebugLogViewDetails *details, 
		gboolean before_event, 
		CongNodePtr node, 
		CongNodePtr younger_sibling) 
{ 
	debug_log_view_details_add_message(details, "Add node before existing child", before_event, "", "");
}

/**
 * log_set_parent:
 * @details:
 * @before_event:
 * @node:
 * @adoptive_parent:
 *
 * TODO: Write me
 */
void 
log_set_parent (CongDebugLogViewDetails *details, 
		gboolean before_event, 
		CongNodePtr node, 
		CongNodePtr adoptive_parent,
		gboolean add_to_end) 
{ 
	debug_log_view_details_add_message(details, "Set parent", before_event, "", "");
}

/**
 * log_set_text:
 * @details:
 * @before_event:
 * @node:
 * @new_content:
 *
 * TODO: Write me
 */
void 
log_set_text (CongDebugLogViewDetails *details, 
	      gboolean before_event, 
	      CongNodePtr node, 
	      const xmlChar *new_content)
{ 
	gchar *node_name = cong_node_get_path(node);
	gchar *cleaned_text = cong_util_cleanup_text(new_content);

	debug_log_view_details_add_message(details, "Set text", before_event, node_name, cleaned_text);

	g_free (node_name);
	g_free (cleaned_text);
}

/**
 * log_set_attribute:
 * @details:
 * @before_event:
 * @node:
 * @ns_ptr:
 * @name:
 * @value:
 *
 * TODO: Write me
 */
void 
log_set_attribute (CongDebugLogViewDetails *details, 
		   gboolean before_event, 
		   CongNodePtr node, 
		   xmlNs *ns_ptr, 
		   const xmlChar *name, 
		   const xmlChar *value) 
{ 
	gchar *qualified_name = cong_util_get_qualified_attribute_name(ns_ptr, name);
	gchar *node_name = cong_node_get_path(node);
	gchar *extra_info = g_strdup_printf("%s=\"%s\"", qualified_name, value);

	debug_log_view_details_add_message(details, "Set attribute", before_event, node_name, extra_info);

	g_free(qualified_name);
	g_free(node_name);
	g_free(extra_info);
}

/**
 * log_remove_attribute:
 * @details:
 * @before_event:
 * @node:
 * @ns_ptr:
 * @name:
 *
 * TODO: Write me
 */
void 
log_remove_attribute (CongDebugLogViewDetails *details, 
		      gboolean before_event, 
		      CongNodePtr node, 
		      xmlNs *ns_ptr, 
		      const xmlChar *name)
{ 
	gchar *qualified_name = cong_util_get_qualified_attribute_name(ns_ptr, name);
	gchar *node_name = cong_node_get_path(node);

	debug_log_view_details_add_message(details, 
					   "Remove attribute", 
					   before_event, node_name, qualified_name);

	g_free(node_name);
	g_free(qualified_name);
}

/**
 * log_selection_change:
 * @details:
 * @before_event:
 *
 * TODO: Write me
 */
void 
log_selection_change (CongDebugLogViewDetails *details, 
		      gboolean before_event) 
{ 
	debug_log_view_details_add_message(details, "Selection change", before_event, "", "");
}

/**
 * log_cursor_change:
 * @details:
 * @before_event:
 *
 * TODO: Write me
 */
void 
log_cursor_change (CongDebugLogViewDetails *details, 
		   gboolean before_event)
{ 
	debug_log_view_details_add_message(details, "Cursor change", before_event, "", "");
}	    


/* Prototypes of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent);
static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling);
static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling);
static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent, gboolean add_to_end);
static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content);
static void on_document_node_set_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name, const xmlChar *value);
static void on_document_node_remove_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name);
static void on_selection_change(CongView *view);
static void on_cursor_change(CongView *view);

/* Definitions of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent)
{
	CongDebugLogView *debug_log_view;
	CongDebugLogViewDetails* details;

	g_return_if_fail(view);
	g_return_if_fail(node);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);
	details = debug_log_view->private;
	g_assert(details);

	log_make_orphan (details, before_event, node);
}

static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling)
{
	CongDebugLogView *debug_log_view;
	CongDebugLogViewDetails* details;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(older_sibling);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);
	details = debug_log_view->private;
	g_assert(details);

	log_add_after (details, before_event, node, older_sibling);

}

static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling)
{
	CongDebugLogView *debug_log_view;
	CongDebugLogViewDetails* details;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);
	details = debug_log_view->private;
	g_assert(details);

	log_add_before (details, before_event, node, younger_sibling);
}

static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent, gboolean add_to_end)
{
	CongDebugLogView *debug_log_view;
	CongDebugLogViewDetails* details;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);
	details = debug_log_view->private;
	g_assert(details);

	log_set_parent (details, before_event, node, adoptive_parent, add_to_end);
}

static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content)
{
	CongDebugLogView *debug_log_view;
	CongDebugLogViewDetails* details;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);
	details = debug_log_view->private;
	g_assert(details);

	log_set_text (details, before_event, node, new_content);
}

static void on_document_node_set_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name, const xmlChar *value)
{
	CongDebugLogView *debug_log_view;
	CongDebugLogViewDetails* details;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(name);
	g_return_if_fail(value);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);
	details = debug_log_view->private;
	g_assert(details);

	log_set_attribute (details, before_event, node, ns_ptr, name, value);
}

static void on_document_node_remove_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name)
{
	CongDebugLogView *debug_log_view;
	CongDebugLogViewDetails* details;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(name);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);
	details = debug_log_view->private;
	g_assert(details);

	log_remove_attribute (details, before_event, node, ns_ptr, name);
}

static void on_selection_change(CongView *view)
{
	CongDebugLogView *debug_log_view;
	CongDebugLogViewDetails* details;

	g_return_if_fail(view);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);
	details = debug_log_view->private;
	g_assert(details);

	log_selection_change (details, FALSE);
}

static void on_cursor_change(CongView *view)
{
	CongDebugLogView *debug_log_view;
	CongDebugLogViewDetails* details;

	g_return_if_fail(view);

	debug_log_view = CONG_DEBUG_LOG_VIEW(view);
	details = debug_log_view->private;
	g_assert(details);

	log_cursor_change (details, FALSE);
}


/* Building the widgets: */
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

static void add_columns (GtkTreeView *tree_view)
{
	/* Create columns, associating the "text" attribute of the
	 * cell_renderer to each column of the model */
	add_text_column(tree_view,
			"Message",
			DEBUGLOGVIEW_COLUMN_MESSAGE_NAME);
	add_text_column(tree_view,
			"Before/After",
			DEBUGLOGVIEW_COLUMN_BEFOREORAFTER);
	add_text_column(tree_view,
			"Node",
			DEBUGLOGVIEW_COLUMN_NODE);
	add_text_column(tree_view,
			"Additional Info",
			DEBUGLOGVIEW_COLUMN_EXTRA_INFO);

}
/**
 * make_window_etc:
 *
 * TODO: Write me
 */
CongDebugLogViewDetails *
make_window_etc(void)
{
	CongDebugLogViewDetails *details;

	details = g_new0(CongDebugLogViewDetails,1);

	details->scrolled_window = GTK_SCROLLED_WINDOW( gtk_scrolled_window_new(NULL, NULL) );
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(details->scrolled_window), 
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(GTK_WIDGET(details->scrolled_window), 100, 50);

        details->list_store = gtk_list_store_new (DEBUGLOGVIEW_NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	details->tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model (GTK_TREE_MODEL(details->list_store)));

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(details->scrolled_window), GTK_WIDGET(details->tree_view));

	add_columns (GTK_TREE_VIEW (details->tree_view));

	gtk_widget_show(GTK_WIDGET(details->tree_view));
	gtk_widget_show(GTK_WIDGET(details->scrolled_window));

	return details;
}

/**
 * cong_debug_message_log_view_new:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget *
cong_debug_message_log_view_new(CongDocument *doc)
{
	CongDebugLogViewDetails *details;
	CongDebugLogView *view;

	g_return_val_if_fail(doc, NULL);

	details = make_window_etc();

	view = g_new0(CongDebugLogView,1);
	
	view->private = details;
	
	view->view.doc = doc;
	view->view.klass = g_new0(CongViewClass,1);
	view->view.klass->on_document_node_make_orphan = on_document_node_make_orphan;
	view->view.klass->on_document_node_add_after = on_document_node_add_after;
	view->view.klass->on_document_node_add_before = on_document_node_add_before;
	view->view.klass->on_document_node_set_parent = on_document_node_set_parent;
	view->view.klass->on_document_node_set_text = on_document_node_set_text;
	view->view.klass->on_document_node_set_attribute = on_document_node_set_attribute;
	view->view.klass->on_document_node_remove_attribute = on_document_node_remove_attribute;
	view->view.klass->on_selection_change = on_selection_change;
	view->view.klass->on_cursor_change = on_cursor_change;

	cong_document_register_view( doc, CONG_VIEW(view) );

	return GTK_WIDGET(details->scrolled_window);	
}

/* Signal handling callbacks: */
/* Callbacks attached before the default handler: */
static void on_signal_begin_edit_notify_before (CongDocument *doc,
					 gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_begin_edit (details, TRUE);
}

static void on_signal_end_edit_notify_before (CongDocument *doc,
				       gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_end_edit (details, TRUE);
}

static void on_signal_make_orphan_notify_before (CongDocument *doc, 
					  CongNodePtr node, 
					  gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_make_orphan (details, TRUE, node);
}

static void on_signal_add_after_notify_before (CongDocument *doc, 
					CongNodePtr node, 
					CongNodePtr older_sibling, 
					gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_add_after (details, TRUE, node, older_sibling);
}

static void on_signal_add_before_notify_before (CongDocument *doc, 
					 CongNodePtr node, 
					 CongNodePtr younger_sibling, 
					 gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_add_before (details, TRUE, node, younger_sibling);
}

static void on_signal_set_parent_notify_before (CongDocument *doc, 
					 CongNodePtr node, 
					 CongNodePtr adoptive_parent, 
						gboolean add_to_end,
					 gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_set_parent (details, TRUE, node, adoptive_parent, add_to_end);
}

static void on_signal_set_text_notify_before (CongDocument *doc, 
				       CongNodePtr node, 
				       const xmlChar *new_content, 
				       gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_set_text (details, TRUE, node, new_content);
}

static void on_signal_set_attribute_notify_before (CongDocument *doc, 
						   CongNodePtr node, 
						   xmlNs *ns_ptr,
						   const xmlChar *name, 
						   const xmlChar *value, 
						   gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_set_attribute (details, TRUE, node, ns_ptr, name, value);
}

static void on_signal_remove_attribute_notify_before (CongDocument *doc, 
						      CongNodePtr node, 
						      xmlNs *ns_ptr,
						      const xmlChar *name, 
						      gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_remove_attribute (details, TRUE, node, ns_ptr, name);
}

static void on_signal_selection_change_notify_before (CongDocument *doc, 
					       gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_selection_change (details, TRUE);
}

static void on_signal_cursor_change_notify_before (CongDocument *doc, 
					    gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_cursor_change (details, TRUE);
}


/* Callbacks attached after the default handler: */
static void on_signal_begin_edit_notify_after (CongDocument *doc,
					 gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_begin_edit (details, FALSE);
}

static void on_signal_end_edit_notify_after (CongDocument *doc,
				       gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_end_edit (details, FALSE);
}

static void on_signal_make_orphan_notify_after (CongDocument *doc, 
					 CongNodePtr node, 
					 gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 
	
	log_make_orphan (details, FALSE, node);
}

static void on_signal_add_after_notify_after (CongDocument *doc, 
					CongNodePtr node, 
					CongNodePtr older_sibling, 
					gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_add_after (details, FALSE, node, older_sibling);
}

static void on_signal_add_before_notify_after (CongDocument *doc, 
					 CongNodePtr node, 
					 CongNodePtr younger_sibling, 
					 gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_add_before (details, FALSE, node, younger_sibling);
}

static void on_signal_set_parent_notify_after (CongDocument *doc, 
					 CongNodePtr node, 
					 CongNodePtr adoptive_parent, 
					       gboolean add_to_end,
					 gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_set_parent (details, FALSE, node, adoptive_parent, add_to_end);
}

static void on_signal_set_text_notify_after (CongDocument *doc, 
				       CongNodePtr node, 
				       const xmlChar *new_content, 
				       gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_set_text (details, FALSE, node, new_content);
}

static void on_signal_set_attribute_notify_after (CongDocument *doc, 
						  CongNodePtr node, 
						  xmlNs *ns_ptr,
						  const xmlChar *name, 
						  const xmlChar *value, 
						  gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_set_attribute (details, FALSE, node, ns_ptr, name, value);
}

static void on_signal_remove_attribute_notify_after (CongDocument *doc, 
						     CongNodePtr node, 
						     xmlNs *ns_ptr,
						     const xmlChar *name, 
						     gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_remove_attribute (details, FALSE, node, ns_ptr, name);
}

static void on_signal_selection_change_notify_after (CongDocument *doc, 
					       gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_selection_change (details, FALSE);
}

static void on_signal_cursor_change_notify_after (CongDocument *doc, 
					    gpointer user_data) 
{ 
	CongDebugLogViewDetails *details = (CongDebugLogViewDetails*)user_data; 

	log_cursor_change (details, FALSE);
}

/**
 * cong_debug_signal_log_view_new:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget *
cong_debug_signal_log_view_new(CongDocument *doc)
{
	CongDebugLogViewDetails *details;

	g_return_val_if_fail(doc, NULL);

	details = make_window_etc();

	/* attach signal handlers to document for notification before change happens: */
	g_signal_connect (G_OBJECT(doc), "begin_edit", G_CALLBACK(on_signal_begin_edit_notify_before), details);
	g_signal_connect (G_OBJECT(doc), "end_edit", G_CALLBACK(on_signal_end_edit_notify_before), details);
	g_signal_connect (G_OBJECT(doc), "node_make_orphan", G_CALLBACK(on_signal_make_orphan_notify_before), details);
	g_signal_connect (G_OBJECT(doc), "node_add_after", G_CALLBACK(on_signal_add_after_notify_before), details);
	g_signal_connect (G_OBJECT(doc), "node_add_before", G_CALLBACK(on_signal_add_before_notify_before), details);
	g_signal_connect (G_OBJECT(doc), "node_set_parent", G_CALLBACK(on_signal_set_parent_notify_before), details);
	g_signal_connect (G_OBJECT(doc), "node_set_text", G_CALLBACK(on_signal_set_text_notify_before), details);
	g_signal_connect (G_OBJECT(doc), "node_set_attribute", G_CALLBACK(on_signal_set_attribute_notify_before), details);
	g_signal_connect (G_OBJECT(doc), "node_remove_attribute", G_CALLBACK(on_signal_remove_attribute_notify_before), details);
	g_signal_connect (G_OBJECT(doc), "selection_change", G_CALLBACK(on_signal_selection_change_notify_before), details);
	g_signal_connect (G_OBJECT(doc), "cursor_change", G_CALLBACK(on_signal_cursor_change_notify_before), details);

	/* attach signal handlers to document for notification after change happens: */
	g_signal_connect_after (G_OBJECT(doc), "begin_edit", G_CALLBACK(on_signal_begin_edit_notify_after), details);
	g_signal_connect_after (G_OBJECT(doc), "end_edit", G_CALLBACK(on_signal_end_edit_notify_after), details);
	g_signal_connect_after (G_OBJECT(doc), "node_make_orphan", G_CALLBACK(on_signal_make_orphan_notify_after), details);
	g_signal_connect_after (G_OBJECT(doc), "node_add_after", G_CALLBACK(on_signal_add_after_notify_after), details);
	g_signal_connect_after (G_OBJECT(doc), "node_add_before", G_CALLBACK(on_signal_add_before_notify_after), details);
	g_signal_connect_after (G_OBJECT(doc), "node_set_parent", G_CALLBACK(on_signal_set_parent_notify_after), details);
	g_signal_connect_after (G_OBJECT(doc), "node_set_text", G_CALLBACK(on_signal_set_text_notify_after), details);
	g_signal_connect_after (G_OBJECT(doc), "node_set_attribute", G_CALLBACK(on_signal_set_attribute_notify_after), details);
	g_signal_connect_after (G_OBJECT(doc), "node_remove_attribute", G_CALLBACK(on_signal_remove_attribute_notify_after), details);
	g_signal_connect_after (G_OBJECT(doc), "selection_change", G_CALLBACK(on_signal_selection_change_notify_after), details);
	g_signal_connect_after (G_OBJECT(doc), "cursor_change", G_CALLBACK(on_signal_cursor_change_notify_after), details);

	return GTK_WIDGET(details->scrolled_window);
}
