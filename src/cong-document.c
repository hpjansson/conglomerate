/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"

#define TEST_VIEW 0

GtkWidget *cong_test_view_new(CongDocument *doc);

struct CongDocument
{
	xmlDocPtr xml_doc;

	CongDispspec *ds;

	gchar *url;

	GList *views; /* a list of CongView* */

	/* cursor and selections are now properties of the document: */
	CongCursor curs;
	CongSelection selection;

	gboolean modified; /* has the document been modified since it was last loaded/saved? */

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

	doc = g_new0(struct CongDocument,1);

	doc->xml_doc = xml_doc;
	doc->ds = ds;
	doc->url = g_strdup(url);

	#if TEST_VIEW
	{
		GtkWindow *window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
		GtkWidget *test_view = cong_test_view_new(doc);
		gtk_container_add(GTK_CONTAINER(window), test_view);
		gtk_widget_show(GTK_WIDGET(window));		
	}
	#endif

	cong_cursor_init(&doc->curs);
	cong_selection_init(&doc->selection);

	doc->curs.set = 0;
	doc->curs.xed = 0;
	doc->curs.w = 0;
	doc->selection.xed = 0;

#if 1
	cong_location_nullify(&doc->selection.loc0);
	cong_location_nullify(&doc->selection.loc1);
#else
	doc->selection.t0 = doc->selection.t1 = 0;
#endif

	return doc;
}

void
cong_document_delete(CongDocument *doc)
{
	g_return_if_fail(doc);

	g_assert(doc->views == NULL); /* There must not be any views left referencing this document */

	cong_cursor_uninit(&doc->curs);

	xmlFreeDoc(doc->xml_doc);

	if (doc->url) {
		g_free(doc->url);
	}
	
	g_free(doc);
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

void
cong_document_save(CongDocument *doc, const char* filename)
{
	xmlChar* mem;
	int size;

	GnomeVFSFileSize file_size;
	GnomeVFSFileSize written_size;

	GnomeVFSURI *file_uri;
	GnomeVFSResult vfs_result;
	GnomeVFSHandle* vfs_handle;

	g_return_if_fail(doc);
	g_return_if_fail(filename);

	/* Dump to a memory buffer. then write out buffer to GnomeVFS: */

	xmlDocDumpMemory(doc->xml_doc,
			 &mem,
			 &size);

	g_assert(mem);
	g_assert(size>0);

	file_size = size;

	file_uri = gnome_vfs_uri_new(filename);

	vfs_result = gnome_vfs_create_uri(&vfs_handle,
					  file_uri,
					  GNOME_VFS_OPEN_WRITE,
					  FALSE,
					  0644
					);

	if (vfs_result != GNOME_VFS_OK) {
		GtkDialog* dialog = cong_error_dialog_new_file_save_failed(file_uri, vfs_result, NULL);
			
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));

		gnome_vfs_uri_unref(file_uri);

		return;
	}

	vfs_result = gnome_vfs_write(vfs_handle,
				     mem,
				     file_size,
				     &written_size);

	if (vfs_result != GNOME_VFS_OK) {
		GtkDialog* dialog = cong_error_dialog_new_file_save_failed(file_uri, vfs_result, &file_size);
			
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));

		gnome_vfs_uri_unref(file_uri);

		gnome_vfs_close(vfs_handle);

		return;
	}

	g_assert(file_size == written_size);

	vfs_result = gnome_vfs_close(vfs_handle);

	if (vfs_result != GNOME_VFS_OK) {
		GtkDialog* dialog = cong_error_dialog_new_file_save_failed(file_uri, vfs_result, &file_size);
			
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));

		gnome_vfs_uri_unref(file_uri);

		return;
	}
	
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

#define DEBUG_MVC 1

void cong_document_coarse_update(CongDocument *doc)
{
	GList *iter;

	g_return_if_fail(doc);

	#if DEBUG_MVC
	g_message("cong_document_coarse_update\n");
	#endif

	doc->curs.w = NULL; /* should this be part of the editor_view ? */

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

	/* This is a special case, in that doc is allowed to be NULL (to handle the clipboard) */

	g_return_if_fail(node);

	#if DEBUG_MVC
	g_message("cong_document_node_make_orphan\n");
	#endif

	/* Make the change: */
	cong_node_make_orphan(node);

	if (doc) {
		/* Notify listeners: */
		for (iter = doc->views; iter; iter = g_list_next(iter) ) {
			CongView *view = CONG_VIEW(iter->data);
			
			g_assert(view->klass);
			g_assert(view->klass->on_document_node_make_orphan);
			view->klass->on_document_node_make_orphan(view, node);
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

	/* Make the change: */
	cong_node_add_after(node, older_sibling);

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_document_node_add_after);
		view->klass->on_document_node_add_after(view, node, older_sibling);
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

	/* Make the change: */
	cong_node_add_before(node, younger_sibling);

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_document_node_add_before);
		view->klass->on_document_node_add_before(view, node, younger_sibling);
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

	/* Make the change: */
	cong_node_set_parent(node, adoptive_parent);

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_document_node_set_parent);
		view->klass->on_document_node_set_parent(view, node, adoptive_parent);
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

	/* Make the change: */
	cong_node_set_text(node, new_content);

	/* Notify listeners: */
	for (iter = doc->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		g_assert(view->klass->on_document_node_set_text);
		view->klass->on_document_node_set_text(view, node, new_content);
	}

	cong_document_set_modified(doc, TRUE);
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
}

void cong_document_unregister_view(CongDocument *doc, CongView *view)
{
	g_return_if_fail(doc);
	g_return_if_fail(view);

	doc->views = g_list_remove(doc->views, view); 
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
