/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
  Useful debug view; shows a source dump of the document.
 */

#include <gtk/gtk.h>

#ifdef ENABLE_GTKSOURCEVIEW
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcelanguagesmanager.h>
#include <gtksourceview/gtksourcelanguage.h>
#endif

#include "global.h"
#include "cong-document.h"
#include "cong-view.h"
#include "cong-util.h"

#define CONG_SOURCE_VIEW(x) ((CongSourceView*)(x))

typedef struct CongSourceView
{
	CongView view;

	struct CongSourceViewDetails *private;
} CongSourceView;

typedef struct CongSourceViewDetails
{
	gboolean format;

#if 0
        GtkSourceBuffer *text_buffer;
        GtkSourceView *text_view;
#else
        GtkTextBuffer *text_buffer;
        GtkTextView *text_view;
#endif
	
	gboolean is_buffer_up_to_date;

	void (*regeneration_cb) (CongDocument *doc,
				 GtkTextBuffer *text_buffer);

} CongSourceViewDetails;

/* Internal function prototypes: */
static void
cong_source_view_free (CongSourceView *source_view);

static void
on_widget_destroy_event (GtkWidget *widget,
			 gpointer user_data);

/* Prototypes of the handler functions: */
static void on_document_begin_edit(CongView *view);
static void on_document_end_edit(CongView *view);
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent);
static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling);
static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling);
static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent, gboolean add_to_end);
static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content);
static void on_document_node_set_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name, const xmlChar *value);
static void on_document_node_remove_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name);
static void on_selection_change(CongView *view);
static void on_cursor_change(CongView *view);
static void on_document_set_dtd_ptr (CongView *view, 
				     gboolean before_change, 
				     xmlDtdPtr dtd_ptr);

#define DEBUG_SOURCE_VIEW 0

static void 
regenerate_text_buffer (CongSourceView *source_view)
{
	CongSourceViewDetails *details;

	g_return_if_fail(source_view);

	details = source_view->private;
	g_assert(details);

	g_assert (details->regeneration_cb);
	(*details->regeneration_cb) (CONG_VIEW(source_view)->doc,
				     GTK_TEXT_BUFFER(details->text_buffer));
	details->is_buffer_up_to_date = TRUE;
}


static void
regenerate_source (CongDocument *doc,
		   GtkTextBuffer *text_buffer)
{
	/* Use libxml to generate a UTF-8 string representation of the buffer: */
	xmlChar *doc_txt_ptr;
	int doc_txt_len;

	xmlDocDumpFormatMemoryEnc (cong_document_get_xml (doc),
				   &doc_txt_ptr,
				   &doc_txt_len, 
				   "UTF-8",
#if 1
				   0);
#else
				   (details->format ? 1 : 0));
#endif
	
	gtk_text_buffer_set_text (GTK_TEXT_BUFFER(text_buffer),
				  doc_txt_ptr,
				  doc_txt_len);
	
	xmlFree (doc_txt_ptr);
}

static void on_document_change(CongSourceView *source_view)
{
	CongSourceViewDetails *details;

	g_assert(source_view);

	details = source_view->private;

	/* A change has occurred to the document... */ 
	if ( cong_document_is_within_edit (cong_view_get_document (CONG_VIEW (source_view)))) {
		/* We're within a nested series of edits: flag the buffer as invalid; we will update it when the nested editing is complete */
		details->is_buffer_up_to_date = FALSE;
	} else {
		/* We're not within a nested series of edits; we must update now: */
		regenerate_text_buffer(source_view);
	}
}


/* Definitions of the handler functions: */
static void on_document_begin_edit(CongView *view)
{
	/* empty */
}

static void on_document_end_edit(CongView *view)
{
	CongSourceView *source_view;
	CongSourceViewDetails *details;

	g_assert(view);

	source_view = CONG_SOURCE_VIEW(view);
	g_assert(source_view);

	details = source_view->private;
	
	if (!details->is_buffer_up_to_date) {
		/* Then some changes occurred during the nested begin/end edit and we deferred them; regenerate the buffer now: */
		regenerate_text_buffer(source_view);
	}
}

static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent)
{
	CongSourceView *source_view;

	g_return_if_fail(view);
	g_return_if_fail(node);

	#if DEBUG_SOURCE_VIEW
	g_message("CongSourceView - on_document_node_make_orphan\n");
	#endif

	source_view = CONG_SOURCE_VIEW(view);

	if (!before_event) {
		on_document_change(source_view);
	}
}

static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling)
{
	CongSourceView *source_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(older_sibling);

	#if DEBUG_SOURCE_VIEW
	g_message("CongSourceView - on_document_node_add_after\n");
	#endif

	source_view = CONG_SOURCE_VIEW(view);

	if (!before_event) {
		on_document_change(source_view);
	}
}

static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling)
{
	CongSourceView *source_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);

	#if DEBUG_SOURCE_VIEW
	g_message("CongSourceView - on_document_node_add_before\n");
	#endif

	source_view = CONG_SOURCE_VIEW(view);

	if (!before_event) {
		on_document_change(source_view);
	}
}

static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent, gboolean add_to_end)
{
	CongSourceView *source_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);

	#if DEBUG_SOURCE_VIEW
	g_message("CongSourceView - on_document_node_set_parent\n");
	#endif

	source_view = CONG_SOURCE_VIEW(view);

	if (!before_event) {
		on_document_change(source_view);
	}
}

static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content)
{
	CongSourceView *source_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_SOURCE_VIEW
	g_message("CongSourceView - on_document_node_set_text\n");
	#endif

	source_view = CONG_SOURCE_VIEW(view);

	if (!before_event) {
		on_document_change(source_view);
	}
}

static void on_document_node_set_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name, const xmlChar *value)
{
	CongSourceView *source_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(name);
	g_return_if_fail(value);

	#if DEBUG_SOURCE_VIEW
	g_message("CongSourceView - on_document_node_set_attribute\n");
	#endif

	source_view = CONG_SOURCE_VIEW(view);

	if (!before_event) {
		on_document_change(source_view);
	}
}

static void on_document_node_remove_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name)
{
	CongSourceView *source_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(name);

	#if DEBUG_SOURCE_VIEW
	g_message("CongSourceView - on_document_node_remove_attribute\n");
	#endif

	source_view = CONG_SOURCE_VIEW(view);

	if (!before_event) {
		on_document_change(source_view);
	}
}

static void on_selection_change(CongView *view)
{
}

static void on_cursor_change(CongView *view)
{
}

static void on_document_set_dtd_ptr (CongView *view, 
				     gboolean before_event, 
				     xmlDtdPtr dtd_ptr)
{
	CongSourceView *source_view;

	g_return_if_fail(view);

	#if DEBUG_SOURCE_VIEW
	g_message("CongSourceView - on_document_set_dtd_ptr");
	#endif

	source_view = CONG_SOURCE_VIEW(view);

	if (!before_event) {
		on_document_change(source_view);
	}
}

/* 
 * cong_util_make_source_view:
 * @source_mime_type:
 * @output_text_view:
 *
 * Make a scrollable source view for a programming language, with syntax highlighting if available.
 * The view is uneditable.
 * The widget is already "shown".
 *
 * Returns:
 */
GtkWidget*
cong_util_make_source_view (const gchar *source_mime_type,
			    GtkTextView **output_text_view)
{
#ifdef ENABLE_GTKSOURCEVIEW
        GtkSourceLanguagesManager *lang_manager;
        GtkSourceLanguage *lang;
        GtkSourceBuffer *text_buffer;
        GtkSourceView *text_view;
#else
        GtkTextBuffer *text_buffer;
        GtkTextView *text_view;
#endif
	GtkScrolledWindow *scrolled_window;

	scrolled_window = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL) );
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), 
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

#ifdef ENABLE_GTKSOURCEVIEW
        lang_manager = gtk_source_languages_manager_new ();
        lang = gtk_source_languages_manager_get_language_from_mime_type (lang_manager, 
									 source_mime_type);
        text_buffer = gtk_source_buffer_new_with_language (lang);
	text_view = GTK_SOURCE_VIEW (gtk_source_view_new_with_buffer (text_buffer));
        gtk_source_buffer_set_highlight (text_buffer, 
					 TRUE);
        g_object_unref(lang_manager);
        g_object_unref(lang);
#else
        text_buffer = gtk_text_buffer_new (NULL);
        text_view = GTK_TEXT_VIEW (gtk_text_view_new_with_buffer(text_buffer));
#endif

	gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (text_view), FALSE);

	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window), 
					       GTK_WIDGET (text_view));
	gtk_widget_show (GTK_WIDGET (text_view));
	gtk_widget_show (GTK_WIDGET (scrolled_window));

	*output_text_view = GTK_TEXT_VIEW (text_view);

	return GTK_WIDGET (scrolled_window);
}

GtkWidget*
cong_source_view_new_full (CongDocument *doc,
			   const gchar *source_mime_type,
			   void (*regeneration_cb) (CongDocument *doc,
						    GtkTextBuffer *text_buffer))
{
	CongSourceViewDetails *details;
	CongSourceView *view;
	GtkWidget *widget;
		
	g_return_val_if_fail(doc, NULL);

	view = g_new0(CongSourceView,1);
	details = g_new0(CongSourceViewDetails,1);
	
	view->private = details;
	
	view->view.doc = doc;
	view->view.klass = g_new0(CongViewClass,1);
	view->view.klass->on_document_begin_edit = on_document_begin_edit;
	view->view.klass->on_document_end_edit = on_document_end_edit;
	view->view.klass->on_document_node_make_orphan = on_document_node_make_orphan;
	view->view.klass->on_document_node_add_after = on_document_node_add_after;
	view->view.klass->on_document_node_add_before = on_document_node_add_before;
	view->view.klass->on_document_node_set_parent = on_document_node_set_parent;
	view->view.klass->on_document_node_set_text = on_document_node_set_text;
	view->view.klass->on_document_node_set_attribute = on_document_node_set_attribute;
	view->view.klass->on_document_node_remove_attribute = on_document_node_remove_attribute;
	view->view.klass->on_selection_change = on_selection_change;
	view->view.klass->on_cursor_change = on_cursor_change;
	view->view.klass->on_document_set_dtd_ptr = on_document_set_dtd_ptr;

	cong_document_register_view( doc, CONG_VIEW(view) );

	details->format = FALSE;
	
	widget = cong_util_make_source_view ("text/xml",
					     &details->text_view);
	details->text_buffer = gtk_text_view_get_buffer (details->text_view);
	details->regeneration_cb = regeneration_cb;

	regenerate_text_buffer(view);

	/* Set up for cleanup: */
	g_signal_connect (G_OBJECT (details->text_view),
			  "destroy",
			  G_CALLBACK (on_widget_destroy_event),
			  view);

	return widget;
}

GtkWidget*
cong_source_view_new (CongDocument *doc)
{
	return cong_source_view_new_full (doc,
					  "text/xml",
					  regenerate_source);
}


static void
cong_source_view_free (CongSourceView *source_view)
{
	g_assert (source_view);

#if 0
	g_message ("cong_source_view_free");
#endif

	cong_document_unregister_view (source_view->view.doc, CONG_VIEW(source_view));

	/* FIXME: do we need to unref the text buffer? */

	g_free (source_view->private);
	g_free (source_view);
}

static void
on_widget_destroy_event (GtkWidget *widget,
			 gpointer user_data)
{
	CongSourceView *source_view = user_data;

#if 0
	g_message ("on_widget_destroy_event");
#endif

	cong_source_view_free (source_view);
}

