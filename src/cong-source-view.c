/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
  Useful debug view; shows a source dump of the document.
 */

#include <gtk/gtk.h>
#include "global.h"
#include "cong-document.h"
#include "cong-view.h"

#define CONG_SOURCE_VIEW(x) ((CongSourceView*)(x))

typedef struct CongSourceView
{
	CongView view;

	struct CongSourceViewDetails *private;
} CongSourceView;

typedef struct CongSourceViewDetails
{
	GtkScrolledWindow *scrolled_window;
	
	GtkTextBuffer *text_buffer;
	GtkTextView *text_view;

} CongSourceViewDetails;

/* Prototypes of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent);
static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling);
static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling);
static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content);
static void on_selection_change(CongView *view);
static void on_cursor_change(CongView *view);

#define DEBUG_SOURCE_VIEW 1

void regenerate_text_buffer(CongSourceView *source_view)
{
	CongSourceViewDetails *details;

	g_return_if_fail(source_view);

	details = source_view->private;
	g_assert(details);

#if 1
	/* Use libxml to generate a UTF-8 string representation of the buffer: */
	{
		xmlChar *doc_txt_ptr;
		int doc_txt_len;

		xmlDocDumpFormatMemoryEnc(cong_document_get_xml(CONG_VIEW(source_view)->doc), 
					  &doc_txt_ptr,
					  &doc_txt_len, 
					  "UTF-8",
					  1 /*  int format */);

		gtk_text_buffer_set_text(details->text_buffer,
					 doc_txt_ptr,
					 doc_txt_len);

		xmlFree(doc_txt_ptr);
	}

#else
	gtk_text_buffer_set_text(details->text_buffer,
				 "fubar",
				 -1);
#endif


}

/* Definitions of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent)
{
	CongSourceView *source_view;
	GtkTreeIter tree_iter;

	g_return_if_fail(view);
	g_return_if_fail(node);

	#if DEBUG_SOURCE_VIEW
	g_message("CongSourceView - on_document_node_make_orphan\n");
	#endif

	source_view = CONG_SOURCE_VIEW(view);

	if (!before_event) {
		regenerate_text_buffer(source_view);
	}
}

static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling)
{
	CongSourceView *source_view;
	GtkTreeIter tree_iter_sibling;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(older_sibling);

	#if DEBUG_SOURCE_VIEW
	g_message("CongSourceView - on_document_node_add_after\n");
	#endif

	source_view = CONG_SOURCE_VIEW(view);

	if (!before_event) {
		regenerate_text_buffer(source_view);
	}
}

static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling)
{
	CongSourceView *source_view;
	GtkTreeIter tree_iter_sibling;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);

	#if DEBUG_SOURCE_VIEW
	g_message("CongSourceView - on_document_node_add_before\n");
	#endif

	source_view = CONG_SOURCE_VIEW(view);

	if (!before_event) {
		regenerate_text_buffer(source_view);
	}
}

static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent)
{
	CongSourceView *source_view;
	GtkTreeIter tree_iter_node;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);

	#if DEBUG_SOURCE_VIEW
	g_message("CongSourceView - on_document_node_set_parent\n");
	#endif

	source_view = CONG_SOURCE_VIEW(view);

	if (!before_event) {
		regenerate_text_buffer(source_view);
	}
}

static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content)
{
	CongSourceView *source_view;
	GtkTreeIter tree_iter;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_SOURCE_VIEW
	g_message("CongSourceView - on_document_node_set_text\n");
	#endif

	source_view = CONG_SOURCE_VIEW(view);

	if (!before_event) {
		regenerate_text_buffer(source_view);
	}
}

static void on_selection_change(CongView *view)
{
}

static void on_cursor_change(CongView *view)
{
}

GtkWidget *cong_source_view_new(CongDocument *doc)
{
	CongSourceViewDetails *details;
	CongSourceView *view;
	GtkCellRenderer *renderer;
 	GtkTreeViewColumn *column;
	GtkTreeIter root_iter;

	g_return_val_if_fail(doc, NULL);

	view = g_new0(CongSourceView,1);
	details = g_new0(CongSourceViewDetails,1);
	
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

        details->text_buffer = gtk_text_buffer_new(NULL);
	details->text_view = GTK_TEXT_VIEW(gtk_text_view_new_with_buffer(details->text_buffer));

	gtk_text_view_set_editable(details->text_view, FALSE);
	gtk_text_view_set_cursor_visible(details->text_view, FALSE);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(details->scrolled_window), GTK_WIDGET(details->text_view));

	regenerate_text_buffer(view);

	gtk_widget_show(GTK_WIDGET(details->text_view));
	gtk_widget_show(GTK_WIDGET(details->scrolled_window));

	return GTK_WIDGET(details->scrolled_window);	
}


