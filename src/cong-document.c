/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"
#if !NEW_XML_IMPLEMENTATION
#include "xml.h"
#endif

#define TEST_VIEW 1

GtkWidget *cong_test_view_new(CongDocument *doc);

struct _CongDocument
{
	char dummy[128];

#if NEW_XML_IMPLEMENTATION
	xmlDocPtr xml_doc;
#else
	TTREE *tt;
#endif  /* #if NEW_XML_IMPLEMENTATION */

	CongDispspec *ds;

	gchar *url;
};

#if NEW_XML_IMPLEMENTATION
CongDocument*
cong_document_new_from_xmldoc(xmlDocPtr xml_doc, CongDispspec *ds, const gchar *url)
{
	CongDocument *doc;

	g_return_val_if_fail(xml_doc!=NULL, NULL);

	doc = g_new(struct _CongDocument,1);

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

	doc = g_new(struct _CongDocument,1);

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
