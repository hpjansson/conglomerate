/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <string.h>
#include <gtk/gtk.h>
#include "global.h"
#include "cong-dispspec.h"
#include "cong-dispspec-registry.h"
#include "cong-document.h"
#include "cong-error-dialog.h"
#include "cong-plugin.h"
#include "cong-app.h"
#include "cong-primary-window.h"
#include <unistd.h> /* for chdir */

/* Data and callback for handling the forced loading of a file, autogenerating a dispspec: */
struct force_dialog
{
	int was_forced;
	xmlDocPtr doc;
	CongDispspec *ds;
};

void force_load(gpointer data)
{
	struct force_dialog *the_dlg = (struct force_dialog*)data;

	g_assert(the_dlg);
	g_assert(the_dlg->doc);

	the_dlg->was_forced=TRUE;
	the_dlg->ds = cong_dispspec_new_generate_from_xml_file(the_dlg->doc);

	g_assert(the_dlg->ds);
}

CongDispspec* query_for_forced_dispspec(gchar *what_failed, xmlDocPtr doc, GtkWindow *parent_window)
{
	GtkDialog *dialog;
	struct force_dialog the_dlg;
	the_dlg.was_forced=FALSE;
	the_dlg.doc=doc;


	dialog = cong_error_dialog_new_with_convenience(parent_window,
							what_failed,
							_("The internal structure of the document does not match any of the types known to Conglomerate."), 
							_("You can force Conglomerate to load the document by clicking on the \"Force\" button below, but results may not be ideal."),
							_("Force"),
							force_load,
							&the_dlg);
	
	cong_error_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));

	if (the_dlg.was_forced) {
		/* We carry on, with the auto-generated dispspec: */
		return the_dlg.ds;
		/* FIXME:  This will eventually leak; the ds is never released and is never part of the global registry */
	} else {
		return NULL;
	}
}

void open_document_do(const gchar* doc_name, GtkWindow *parent_window)
{
	char *p;
	FILE *xml_f;
	CongDispspec *ds;
	CongDocument *cong_doc;
	xmlDocPtr doc = NULL;

	/* Use libxml to load the doc: */
	{
		GnomeVFSURI* file_uri = gnome_vfs_uri_new(doc_name);

		/* Load using GnomeVFS: */
		{
			char* buffer;
			GnomeVFSFileSize size;
			GnomeVFSResult vfs_result = cong_vfs_new_buffer_from_file(doc_name, &buffer, &size);

			if (vfs_result!=GNOME_VFS_OK) {
				GtkDialog* dialog = cong_error_dialog_new_from_file_open_failure_with_vfs_result(parent_window,
														 file_uri, 
														 vfs_result);
			
				cong_error_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(GTK_WIDGET(dialog));

				gnome_vfs_uri_unref(file_uri);

				return;
			}

			g_assert(buffer);

			/* Parse the file from the buffer: */
			doc = cong_ui_parse_buffer(buffer, size, file_uri, parent_window);

			g_free(buffer);
		}


		if (NULL==doc) {
			gnome_vfs_uri_unref(file_uri);
			return;
		}

		ds = cong_dispspec_registry_get_appropriate_dispspec(cong_app_singleton()->ds_registry, doc);

		if (ds==NULL) {
			gchar *what_failed;

			what_failed = cong_error_what_failed_on_file_open_failure(file_uri, FALSE);

			ds = query_for_forced_dispspec(what_failed, doc, parent_window);
			
			g_free(what_failed);

			if (NULL==ds) {
				xmlFreeDoc(doc);
				gnome_vfs_uri_unref(file_uri);
				return;
			}
		}

		gnome_vfs_uri_unref(file_uri);
		
	}

	g_assert(ds);
	cong_doc = cong_document_new_from_xmldoc(doc, ds, doc_name); /* takes ownership of doc */

	cong_node_self_test_recursive(cong_document_get_root(cong_doc));

	g_assert(cong_doc);

#if 1
	cong_primary_window_new(cong_doc);
	cong_document_unref(cong_doc);
#else
	the_globals.xv = xmlview_new(cong_doc);
	gtk_box_pack_start(GTK_BOX(cong_gui_get_root(&the_gui)), the_globals.xv->w, FALSE, FALSE, 0);
#endif

}



void open_document(GtkWindow *parent_window)
{
	char *doc_name;

	g_return_if_fail(parent_window);
	
	doc_name = cong_get_file_name(_("Select an XML document"),
				      NULL,
				      parent_window);

	if (!doc_name) {
		return;
	}

	open_document_do(doc_name, parent_window);

	g_free(doc_name);
}

gint toolbar_callback_open(GtkWidget *widget, gpointer data)
{
	CongPrimaryWindow *primary_window = data;

	open_document(cong_primary_window_get_toplevel(primary_window));

	return TRUE;
}
