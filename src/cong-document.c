/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"
#include "cong-document.h"
#include "cong-view.h"
#include "cong-error-dialog.h"
#include "cong-dispspec.h"

#define TEST_VIEW 0
#define TEST_EDITOR_VIEW 0
#define DEBUG_MVC 0

typedef void (*CongXMLSelfTestCallback)(xmlNodePtr node, const gchar *error_message);

gboolean cong_xml_selftest_node(xmlNodePtr node, CongXMLSelfTestCallback selftest_callback);
gboolean cong_xml_selftest_doc(xmlDocPtr xml_doc, CongXMLSelfTestCallback selftest_callback);

gboolean cong_xml_selftest_node(xmlNodePtr node, CongXMLSelfTestCallback selftest_callback)
{
	/* Test this node: */
	if (node->content && (node->type!=XML_ATTRIBUTE_DECL)) {
		/* g_message("testing node content\"%s\"", node->content); */
		if (!g_utf8_validate(node->content, -1, NULL)) {
			if (selftest_callback) {
				(*selftest_callback)(node, "Invalid UTF-8 data");
			}
			return FALSE;
		}
	}

	/* Recurse through children: */
	{
		xmlNodePtr iter;
		
		for (iter = node->children; iter; iter=iter->next) {
			if (!cong_xml_selftest_node(iter, selftest_callback)) {
				return FALSE;
			}
		}
	}

	/* All tests passed: */
	return TRUE;
}

gboolean cong_xml_selftest_doc(xmlDocPtr xml_doc, CongXMLSelfTestCallback selftest_callback)
{
	xmlNodePtr iter;

	g_return_val_if_fail(xml_doc, FALSE);

	/* Traverse the document: */
	for (iter = xml_doc->children; iter; iter=iter->next) {
		if (!cong_xml_selftest_node(iter, selftest_callback)) {
			return FALSE;
		}
	}
}


struct CongDocument
{
	int ref_count;

	xmlDocPtr xml_doc;

	CongDispspec *ds;

	gchar *url;

	GList *views; /* a list of CongView* */

	/* cursor and selections are now properties of the document: */
	CongCursor curs;
	CongSelection selection;

	gboolean modified; /* has the document been modified since it was last loaded/saved? */
	GTimeVal time_of_last_save;

	/* We have an SDI interface, so there should be just one primary window associated with each doc.
	   Knowing this lets us update the window title when it changes (eventually do as a signal on the document).
	*/
	CongPrimaryWindow *primary_window; 
};

CongDocument*
cong_document_new_from_xmldoc(xmlDocPtr xml_doc, CongDispspec *ds, const gchar *url)
{
	CongDocument *doc;

	g_return_val_if_fail(xml_doc!=NULL, NULL);
#if 0
	g_return_val_if_fail(cong_xml_selftest_doc(xml_doc, NULL), NULL);
#endif

	doc = g_new0(struct CongDocument,1);

	doc->ref_count=1; /* created with an initial ref_count of 1, with all this implies */
	doc->xml_doc = xml_doc;
	doc->ds = ds;
	doc->url = g_strdup(url);

	g_get_current_time(&doc->time_of_last_save);

	#if TEST_VIEW
	{
		GtkWindow *window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
		GtkWidget *test_view = cong_test_view_new(doc);
		gtk_container_add(GTK_CONTAINER(window), test_view);
		gtk_widget_show(GTK_WIDGET(window));		
	}
	#endif

	#if TEST_EDITOR_VIEW
	{
		GtkWindow *window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
		GtkWidget *test_editor_view = cong_editor_widget_new(doc);
		GtkWidget *scroller;

		scroller = gtk_scrolled_window_new(NULL, NULL);

		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), 
					       GTK_POLICY_AUTOMATIC,
					       GTK_POLICY_ALWAYS);

		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroller), 
						      test_editor_view);

		gtk_container_add(GTK_CONTAINER(window), scroller);

		gtk_widget_show_all(GTK_WIDGET(window));		
	}
	#endif

	cong_cursor_init(&doc->curs, doc);
	cong_selection_init(&doc->selection);

#if 0
	doc->curs.set = 0;
	doc->curs.w = 0;
#endif

	cong_location_nullify(&doc->selection.loc0);
	cong_location_nullify(&doc->selection.loc1);

	return doc;
}

void
cong_document_ref(CongDocument *doc)
{
	g_return_if_fail(doc);

	doc->ref_count++;
}


void
cong_document_unref(CongDocument *doc)
{
	g_return_if_fail(doc);

	if ((--doc->ref_count)==0) {
		g_assert(doc->views == NULL); /* There must not be any views left referencing this document; views are supposed to hold references to the doc */

		cong_cursor_uninit(&doc->curs);
	
		xmlFreeDoc(doc->xml_doc);

		if (doc->url) {
			g_free(doc->url);
		}
	
		g_free(doc);
	}
}

xmlDocPtr
cong_document_get_xml(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return doc->xml_doc;
}

CongNodePtr
cong_document_get_root(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return doc->xml_doc->children;

}

CongDispspec*
cong_document_get_dispspec(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return doc->ds;
}

CongDispspecElement*
cong_document_get_dispspec_element_for_node(CongDocument *doc, CongNodePtr node)
{
	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);

	return cong_dispspec_lookup_node(doc->ds, node);
}

gchar*
cong_document_get_filename(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	if (doc->url) {
		gchar *filename;
		gchar *path;
		GnomeVFSURI *uri = gnome_vfs_uri_new(doc->url);
		
		cong_error_split_uri(uri, &filename, &path);

		gnome_vfs_uri_unref(uri);

		g_free(path);
		
		return filename;

	} else {
		return g_strdup("(Untitled)");
	}
}

gchar*
cong_document_get_full_uri(CongDocument *doc) {
	g_return_val_if_fail(doc, NULL);

	if (doc->url) {
		return g_strdup(doc->url);
	}
	else {
		return NULL;
	}		    
}

gchar*
cong_document_get_parent_uri(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	if (doc->url) {
		gchar *filename;
		gchar *path;
		GnomeVFSURI *uri = gnome_vfs_uri_new(doc->url);
		
		cong_error_split_uri(uri, &filename, &path);

		gnome_vfs_uri_unref(uri);

		g_free(filename);
		
		return path;

	} else {
		return g_strdup(".");
	}
}

const CongXMLChar*
cong_document_get_dtd_public_identifier(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	g_assert(doc->xml_doc);

	if (NULL==doc->xml_doc->extSubset) {
		return NULL;
	}

	return doc->xml_doc->extSubset->ExternalID;
}

void
cong_document_save(CongDocument *doc, 
		   const char* filename, 
		   GtkWindow *toplevel_window)
{

	GnomeVFSURI *file_uri;
	GnomeVFSResult vfs_result;
	GnomeVFSFileSize file_size;

	g_return_if_fail(doc);
	g_return_if_fail(filename);

	file_uri = gnome_vfs_uri_new(filename);
	
	vfs_result = cong_xml_save_to_vfs(doc->xml_doc, 
					  file_uri,	
					  &file_size);

	if (vfs_result != GNOME_VFS_OK) {
		GtkDialog* dialog = cong_error_dialog_new_from_file_save_failure(toplevel_window,
										 file_uri, 
										 vfs_result, 
										 &file_size);
			
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));

		gnome_vfs_uri_unref(file_uri);

		return;
	}

	cong_document_set_url(doc, filename);

	cong_document_set_modified(doc, FALSE);

	g_get_current_time(&doc->time_of_last_save);

	gnome_vfs_uri_unref(file_uri);
}

gboolean
cong_document_is_modified(CongDocument *doc)
{
	g_return_val_if_fail(doc, FALSE);

	return doc->modified;
}

void
cong_document_set_modified(CongDocument *doc, gboolean modified)
{
	g_return_if_fail(doc);

	doc->modified = modified;

	/* get at primary window; set title */
	if (doc->primary_window) {
		cong_primary_window_update_title(doc->primary_window);
	}
}

void
cong_document_set_primary_window(CongDocument *doc, CongPrimaryWindow *window)
{
	g_return_if_fail(doc);
	g_return_if_fail(window);

	g_assert(doc->primary_window==NULL);
	doc->primary_window = window;
}

void 
cong_document_set_url(CongDocument *doc, const gchar *url) 
{
	g_return_if_fail(doc);

	if (doc->url) {
		g_free(doc->url);
	}
	doc->url = g_strdup(url);

	/* get at primary window; set title */
	if (doc->primary_window) {
		cong_primary_window_update_title(doc->primary_window);
	}
}

glong
cong_document_get_seconds_since_last_save_or_load(const CongDocument *doc)
{
	GTimeVal current_time;

	g_return_val_if_fail(doc, 0);

	g_get_current_time(&current_time);

	return current_time.tv_sec - doc->time_of_last_save.tv_sec;
}

void cong_document_coarse_update(CongDocument *doc)
{
	GList *iter;

	g_return_if_fail(doc);

	#if DEBUG_MVC
	g_message("cong_document_coarse_update\n");
	#endif

#if 0
	doc->curs.w = NULL; /* should this be part of the editor_view ? */
#endif

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_document_coarse_update);
		view->klass->on_document_coarse_update(view);
	}

}

void cong_document_node_make_orphan(CongDocument *doc, CongNodePtr node)
{
	GList *iter;
	CongNodePtr former_parent;

	/* This is a special case, in that doc is allowed to be NULL (to handle the clipboard) */

	g_return_if_fail(node);

	#if DEBUG_MVC
	g_message("cong_document_node_make_orphan\n");
	#endif

	former_parent = node->parent;

	if (doc) {
		/* Notify listeners: */
		for (iter = doc->views; iter; iter = g_list_next(iter) ) {
			CongView *view = CONG_VIEW(iter->data);
			
			g_assert(view->klass);
			g_assert(view->klass->on_document_node_make_orphan);
			view->klass->on_document_node_make_orphan(view, TRUE, node, former_parent);
		}
		
		cong_document_set_modified(doc, TRUE);
	}

	/* Make the change: */
	cong_node_private_make_orphan(node);

	if (doc) {
		/* Notify listeners: */
		for (iter = doc->views; iter; iter = g_list_next(iter) ) {
			CongView *view = CONG_VIEW(iter->data);
			
			g_assert(view->klass);
			g_assert(view->klass->on_document_node_make_orphan);
			view->klass->on_document_node_make_orphan(view, FALSE, node, former_parent);
		}
		
		cong_document_set_modified(doc, TRUE);
	}
}

void cong_document_node_add_after(CongDocument *doc, CongNodePtr node, CongNodePtr older_sibling)
{
	GList *iter;

	g_assert(doc);
	g_return_if_fail(doc);
	g_return_if_fail(node);

	#if DEBUG_MVC
	g_message("cong_document_node_add_after\n");
	#endif

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_document_node_add_after);
		view->klass->on_document_node_add_after(view, TRUE, node, older_sibling);
	}

	/* Make the change: */
	cong_node_private_add_after(node, older_sibling);

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_document_node_add_after);
		view->klass->on_document_node_add_after(view, FALSE, node, older_sibling);
	}

	cong_document_set_modified(doc, TRUE);
}

void cong_document_node_add_before(CongDocument *doc, CongNodePtr node, CongNodePtr younger_sibling)
{
	GList *iter;

	g_return_if_fail(doc);
	g_return_if_fail(node);

	#if DEBUG_MVC
	g_message("cong_document_node_add_before\n");
	#endif

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_document_node_add_before);
		view->klass->on_document_node_add_before(view, TRUE, node, younger_sibling);
	}

	/* Make the change: */
	cong_node_private_add_before(node, younger_sibling);

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_document_node_add_before);
		view->klass->on_document_node_add_before(view, FALSE, node, younger_sibling);
	}

	cong_document_set_modified(doc, TRUE);
}

void cong_document_node_set_parent(CongDocument *doc, CongNodePtr node, CongNodePtr adoptive_parent)
{
	GList *iter;

	g_return_if_fail(doc);
	g_return_if_fail(node);

	#if DEBUG_MVC
	g_message("cong_document_node_set_parent\n");
	#endif

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_document_node_set_parent);
		view->klass->on_document_node_set_parent(view, TRUE, node, adoptive_parent);
	}

	/* Make the change: */
	cong_node_private_set_parent(node, adoptive_parent);

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_document_node_set_parent);
		view->klass->on_document_node_set_parent(view, FALSE, node, adoptive_parent);
	}

	cong_document_set_modified(doc, TRUE);
}

void cong_document_node_set_text(CongDocument *doc, CongNodePtr node, const xmlChar *new_content)
{
	GList *iter;

	g_return_if_fail(doc);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_MVC
	g_message("cong_document_node_set_text\n");
	#endif

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_document_node_set_text);
		view->klass->on_document_node_set_text(view, TRUE, node, new_content);
	}

	/* Make the change: */
	cong_node_private_set_text(node, new_content);

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_document_node_set_text);
		view->klass->on_document_node_set_text(view, FALSE, node, new_content);
	}

	cong_document_set_modified(doc, TRUE);
}

void cong_document_on_selection_change(CongDocument *doc)
{
	GList *iter;

	g_return_if_fail(doc);

	#if DEBUG_MVC
	g_message("cong_document_on_selection_change\n");
	#endif

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_selection_change);
		view->klass->on_selection_change(view);
	}
}

void cong_document_on_cursor_change(CongDocument *doc)
{
	GList *iter;

	g_return_if_fail(doc);

	#if DEBUG_MVC
	g_message("cong_document_node_on_cursor_change\n");
	#endif

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_cursor_change);
		view->klass->on_cursor_change(view);
	}
}

void cong_document_tag_remove(CongDocument *doc, CongNodePtr x)
{
	GList *iter;

	g_return_if_fail(doc);
	g_return_if_fail(x);

	#if DEBUG_MVC
	g_message("cong_document_tag_remove\n");
	#endif

	xml_tag_remove(doc, x); /* this is now a compound operation */
}

void cong_document_register_view(CongDocument *doc, CongView *view)
{
	g_return_if_fail(doc);
	g_return_if_fail(view);

	doc->views = g_list_prepend(doc->views, view);
	cong_document_ref(doc);
}

void cong_document_unregister_view(CongDocument *doc, CongView *view)
{
	g_return_if_fail(doc);
	g_return_if_fail(view);

	doc->views = g_list_remove(doc->views, view); 
	cong_document_unref(doc);
}


CongCursor* cong_document_get_cursor(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return &doc->curs;
}

CongSelection* cong_document_get_selection(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return &doc->selection;
}

PangoLanguage*
cong_document_get_language_for_node(CongDocument *doc, 
				    CongNodePtr node)
{
	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);

	return NULL; /* for now */
}

void cong_document_delete_selection(CongDocument *doc)
{
	CongNodePtr t;
	CongSelection *selection;
	CongCursor *curs;

	g_return_if_fail(doc);

#if 0
	CONG_DO_UNIMPLEMENTED_DIALOG(NULL,
				     "Deletion of selection");
#else
	cong_selection_delete(cong_document_get_selection(doc), doc);
#endif
}
