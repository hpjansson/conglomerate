/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"

#define TEST_VIEW 1

GtkWidget *cong_test_view_new(CongDocument *doc);

struct CongDocument
{
	char dummy[128];

#if NEW_XML_IMPLEMENTATION
	xmlDocPtr xml_doc;
#else
	TTREE *tt;
#endif  /* #if NEW_XML_IMPLEMENTATION */

	CongDispspec *ds;

	gchar *url;

	GList *views; /* a list of CongView* */
};

#if NEW_XML_IMPLEMENTATION
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

	return doc;
}
#else
CongDocument*
cong_document_new_from_ttree(TTREE *tt, CongDispspec *ds, const gchar *url)
{
	CongDocument *doc;

	g_return_val_if_fail(tt!=NULL, NULL);

	doc = g_new(struct CongDocument,1);

	doc->tt = tt;
	doc->ds = ds;
	doc->url = g_strdup(url);

	return doc;
}
#endif  /* #if NEW_XML_IMPLEMENTATION */

void
cong_document_delete(CongDocument *doc)
{
	g_return_if_fail(doc);

#if NEW_XML_IMPLEMENTATION
	g_assert(0);
#else
	g_assert(doc->tt);

	ttree_branch_remove(doc->tt);
#endif  /* #if NEW_XML_IMPLEMENTATION */

	if (doc->url) {
		g_free(doc->url);
	}
	
	g_free(doc);
}

CongNodePtr
cong_document_get_root(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

#if NEW_XML_IMPLEMENTATION
	return doc->xml_doc->children;
#else
	return doc->tt->child;
#endif /* #if NEW_XML_IMPLEMENTATION */

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

void
cong_document_save(CongDocument *doc, const char* filename)
{
#if NEW_XML_IMPLEMENTATION
	xmlChar* mem;
	int size;

	GnomeVFSFileSize file_size;
	GnomeVFSFileSize written_size;

	GnomeVFSURI *file_uri;
	GnomeVFSResult vfs_result;
	GnomeVFSHandle* vfs_handle;

#else
	FILE *xml_f;
#endif

	g_return_if_fail(doc);
	g_return_if_fail(filename);

#if NEW_XML_IMPLEMENTATION
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

#else
	xml_f = fopen(filename, "wt");
	if (!xml_f) return;

	xml_t_to_f(cong_document_get_root(doc), xml_f);
	fclose(xml_f);
#endif  /* #if NEW_XML_IMPLEMENTATION */
}

#if NEW_XML_IMPLEMENTATION
#define DEBUG_MVC 1
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
			view->klass->on_document_node_make_orphan(view, node);
		}
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
		view->klass->on_document_node_add_after(view, node, older_sibling);
	}
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
		view->klass->on_document_node_add_before(view, node, younger_sibling);
	}
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
		view->klass->on_document_node_set_parent(view, node, adoptive_parent);
	}
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
		view->klass->on_document_node_set_text(view, node, new_content);
	}
}
#endif

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

	doc->views = g_list_prepend(doc->views,view);
}

void cong_document_unregister_view(CongDocument *doc, CongView *view)
{
	g_return_if_fail(doc);
	g_return_if_fail(view);

	g_assert(0); /* FIXME:  unwritten */
}


