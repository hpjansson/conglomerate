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
#include "cong-vfs.h"
#include "cong-file-selection.h"

/* Data and callback for handling the forced loading of a file, autogenerating a dispspec: */
struct force_dialog
{
	int was_forced;
	xmlDocPtr doc;
	CongDispspec *ds;
	const gchar *filename_extension;
};

/**
 * force_load:
 * @data:
 *
 * TODO: Write me
 */
void 
force_load(gpointer data)
{
	struct force_dialog *the_dlg = (struct force_dialog*)data;

	g_assert(the_dlg);
	g_assert(the_dlg->doc);

	the_dlg->was_forced=TRUE;
	the_dlg->ds = cong_dispspec_new_generate_from_xml_file (the_dlg->doc,
								the_dlg->filename_extension);

	g_assert(the_dlg->ds);
}

/**
 * query_for_forced_dispspec:
 * @what_failed:
 * @doc:
 * @parent_window:
 * @filename_extension:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspec* 
query_for_forced_dispspec (gchar *what_failed, 
			   xmlDocPtr doc, 
			   GtkWindow *parent_window,
			   const gchar *filename_extension)
{
	GtkDialog *dialog;
	struct force_dialog the_dlg;
	the_dlg.was_forced = FALSE;
	the_dlg.doc = doc;
	the_dlg.filename_extension = filename_extension;


	dialog = cong_error_dialog_new_with_convenience(parent_window,
							what_failed,
							_("The internal structure of the document does not match any of the types known to Conglomerate."), 
							_("You can force Conglomerate to load the document by clicking on the \"Force\" button below, but results may not be ideal."),
							_("Force"),
							GTK_STOCK_CANCEL,
							TRUE,
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

/**
 * get_filename_extension:
 * @file:
 *
 * TODO: Write me
 * Returns:
 */
gchar*
get_filename_extension (GFile *file)
{
	gchar *result = NULL;
	gchar *short_name;
	gchar *separator;

	g_return_val_if_fail (file, NULL);

	short_name = g_file_get_basename(file);

	separator = g_strrstr (short_name, ".");

	if (separator) {
		result = g_strdup (separator+1);
	}

	g_free (short_name);

	return result;
}

/**
 * open_document_do:
 * @doc_name:
 * @parent_window:
 *
 * TODO: Write me
 */
void 
open_document_do (GFile* doc_name,
		  GtkWindow *parent_window)
{
	CongDispspec *ds;
	CongDocument *cong_doc;
	xmlDocPtr doc = NULL;

	doc = cong_vfs_load_xml_from_file (doc_name, parent_window);

	if (NULL==doc) {
		return;
	}

	/* Use libxml to load the doc: */
	{
		gchar *filename_extension;

		filename_extension = get_filename_extension (doc_name);

		ds = cong_dispspec_registry_get_appropriate_dispspec (cong_app_get_dispspec_registry (cong_app_singleton()), 
								      doc,
								      filename_extension);

		/* Disable this for now: we'll make it valid to have a NULL dispspec in this case for now: */
		#if 0
		if (ds==NULL) {
			gchar *what_failed;

			what_failed = cong_error_what_failed_on_file_open_failure(doc_name, FALSE);

			ds = query_for_forced_dispspec (what_failed, 
							doc, 
							parent_window,
							filename_extension);
			
			g_free(what_failed);

			if (NULL==ds) {
				xmlFreeDoc(doc);
				gnome_vfs_uri_unref(file_uri);

				if (filename_extension) {
					g_free (filename_extension);
				}
				return;
			}
		}
		#endif

		if (filename_extension) {
			g_free (filename_extension);
		}		
	}

	cong_doc = cong_document_new_from_xmldoc(doc, ds, doc_name); /* takes ownership of doc */

	cong_node_self_test_recursive (cong_document_get_root_element (cong_doc));

	g_assert(cong_doc);

	cong_primary_window_new(cong_doc);

	/* now we have a primary window we can set the "Save" menu item */
	{
		CongPrimaryWindow *primary_window = cong_document_get_primary_window(cong_doc);
		g_assert (primary_window);
		cong_primary_window_action_set_sensitive (primary_window, "Save", FALSE);
	}
	
	/* Add recent entry */
	{
		CongPrimaryWindow *primary_window = cong_document_get_primary_window(cong_doc);
		char *uri = g_file_get_uri(doc_name);
		gtk_recent_manager_add_item (primary_window->recent_manager, uri);
		g_free(uri);
	}

	g_object_unref( G_OBJECT(cong_doc));
}

/**
 * open_document:
 * @parent_window:
 *
 * TODO: Write me
 */
void 
open_document(GtkWindow *parent_window)
{
	GFile *doc_name;

	g_return_if_fail(parent_window);
	
	doc_name = cong_get_file_name(_("Select an XML document"),
				      NULL,
				      parent_window,
				      CONG_FILE_CHOOSER_ACTION_OPEN,
				      cong_file_selection_make_xml_filter_list());
	if (!doc_name) {
		return;
	}

	open_document_do(doc_name, parent_window);

	g_object_unref(doc_name);
}

/**
 * toolbar_callback_open:
 * @widget:
 * @data:
 *
 * TODO: Write me
 * Returns:
 */
gint 
toolbar_callback_open(GtkWidget *widget, gpointer data)
{
	CongPrimaryWindow *primary_window = data;

	open_document(cong_primary_window_get_toplevel(primary_window));

	return TRUE;
}
