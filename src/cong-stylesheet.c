/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-stylesheet.c
 *
 * Copyright (C) 2004 David Malcolm
 *
 * Conglomerate is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Conglomerate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#include "global.h"
#include "cong-stylesheet.h"
#include "cong-primary-window.h"
#include "cong-app.h"
#include "cong-error-dialog.h"
#include "cong-document.h"
#include "cong-vfs.h"
#include "cong-util.h"

#include <libxml/globals.h>
#include <libxml/catalog.h>

#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>

/**
 * cong_utils_get_norman_walsh_stylesheet_path:
 *
 *  Try to locate Norman Walsh's stylesheets for DocBook using the local catalog to find them.
 *
 * Returns:  a string containing the path (which the caller must delete), or NULL
 */
gchar*
cong_utils_get_norman_walsh_stylesheet_path(void)
{
       /* This should be changed if another catalog is in use, i guess */
       xmlChar *resolved_path = NULL;

       resolved_path = xmlCatalogResolveURI ((const xmlChar*)"http://docbook.sourceforge.net/release/xsl/current/");

       g_message ("Norman Walsh XSL path: %s", resolved_path);

       return cong_util_dup_and_free_xml_string (resolved_path);
}

/**
 * cong_utils_get_norman_walsh_stylesheet:
 * @stylesheet_relative_path:
 *
 * TODO: Write me
 * Returns:
 */
gchar*
cong_utils_get_norman_walsh_stylesheet(const gchar *stylesheet_relative_path)
{
	gchar *path;
	gchar *result;

	g_return_val_if_fail(stylesheet_relative_path, NULL);

	path = cong_utils_get_norman_walsh_stylesheet_path();

	result = g_strdup_printf("%s%s", path, stylesheet_relative_path);

	g_free (path);

	return result;
}

CongStylesheetParameter*
cong_stylesheet_parameter_new (const gchar *name,
			       const gchar *value)
{
	CongStylesheetParameter *param;

	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (value, NULL);
	
	param = g_new0 (CongStylesheetParameter,1);
	param->name = g_strdup (name);
	param->value = g_strdup (value);
	return param;
}

void
cong_stylesheet_parameter_free (CongStylesheetParameter *parameter)
{
	g_return_if_fail (parameter);
	
	g_free (parameter->name);
	g_free (parameter->value);
	g_free (parameter);	
}


void
cong_stylesheet_parameter_list_free (GList *list_of_parameters)
{
	GList *iter;

	for (iter = list_of_parameters; iter; iter=iter->next) {
		cong_stylesheet_parameter_free ((CongStylesheetParameter*)iter->data);
	}

	g_list_free (list_of_parameters);
}

void
cong_stylesheet_parameter_list_debug (GList *list_of_parameters)
{
	GList *iter;

	g_message ("Stylesheet parameters:");

	for (iter = list_of_parameters; iter; iter=iter->next) {
		CongStylesheetParameter *param = (CongStylesheetParameter*)iter->data;

		g_message ("\"%s\"->\"%s\"", param->name, param->value);
	}

}

static char**
make_libxslt_params (GList *list_of_parameters)
{
	int num_params = g_list_length (list_of_parameters);
	char **result = g_malloc0 (sizeof(char*) * ((num_params*2)+1));
	int i;
	GList *iter;

	for (i=0, iter=list_of_parameters; iter; iter=iter->next, i++) {
		CongStylesheetParameter *param = (CongStylesheetParameter*)iter->data;

		/* shallow copy */
		result[(2*i)] = param->name;
		result[(2*i)+1] = param->value;
	}
	result[(2*i)] = NULL;

	return result;
}

static void
free_libxslt_params (char **libxslt_params)
{
	/* it was only a shallow copy */
	g_free (libxslt_params);
}

static void
set_window_cursor_helper (gpointer primary_window, gpointer cursor)
{
	gdk_window_set_cursor (((CongPrimaryWindow *)primary_window)->window->window,
			       (GdkCursor *)cursor );
}

static void
set_window_cursor (GdkCursor *cursor)
{
	/*
	 * It seems that we need the gdk_flush() to ensure the
	 * change happens before calling something processor-intensive
	 */
	g_list_foreach ( cong_app_singleton()->primary_windows,
			 set_window_cursor_helper,
			 cursor );
	gdk_flush();
}

/**
 * cong_ui_transform_doc:
 * @doc:
 * @stylesheet_filename:
 * @list_of_parameters: a #GList of #CongStylesheetParameter
 * @toplevel_window:
 *
 * TODO: Write me
 *
 * The routine sets all primary windows of Conglomerate to have a busy cursor whilst
 * the processing occurs. It will not pick up any dialog, or other, windows
 * that may have been created by Conglomerate. At the moment this is not a problem,
 * but it may be in the future. It may be possible to use GTK (or GDK) functions
 * to fid all the windows that are currently controlled by Conglomerate, and
 * so avoid this issue.
 *
 * Returns:
 */
xmlDocPtr 
cong_ui_transform_doc(CongDocument *doc,
		      const gchar *stylesheet_filename,
		      GList *list_of_parameters,
		      GtkWindow *toplevel_window)
{
	xsltStylesheetPtr xsl;
	xmlDocPtr input_clone;
	xmlDocPtr result;
	char **libxslt_params;
	GdkCursor *busy_cursor;

	g_return_val_if_fail (doc, NULL);
	g_return_val_if_fail (stylesheet_filename, NULL);

	xsl = xsltParseStylesheetFile((const xmlChar*)stylesheet_filename);

	if (NULL==xsl) {
		/*
		 * Since we can not assume that these are the DocBook stylesheets we can not
		 * be too precise with the error message. The message below assumes that whatever stylesheet
		 * was used was accessed via the XML catalog mechanism, hence the note about the catalog
		 * file. It would be nice to make the URL an actual link but that is a lot more work.
		 */
		gchar *why_failed = g_strdup_printf (_("There was a problem reading the stylesheet file \"%s\""),stylesheet_filename);
		GtkDialog* dialog = cong_error_dialog_new (toplevel_window,
							   _("Conglomerate could not transform the document"),
							   why_failed,
							   _("Since Conglomerate could not read the stylesheet it is possible that your XML catalog file is set up incorrectly. See http://www.xmlsoft.org/catalog.html for more information on catalogs."));

		cong_error_dialog_run (GTK_DIALOG(dialog));
		gtk_widget_destroy (GTK_WIDGET(dialog));
		return NULL;
	}

	/* DHM 14/11/2002:  document nodes seemed to being corrupted when applying the stylesheet.
	   So We now work with a clone of the document
	*/
	input_clone = xmlCopyDoc(cong_document_get_xml(doc), TRUE);
	g_assert(input_clone);

	cong_stylesheet_parameter_list_debug (list_of_parameters);
	libxslt_params = make_libxslt_params (list_of_parameters);

	busy_cursor = gdk_cursor_new(GDK_WATCH);
	set_window_cursor (busy_cursor);

	result = xsltApplyStylesheet(xsl, input_clone, (const char**)libxslt_params);

	set_window_cursor (NULL);
	gdk_cursor_unref (busy_cursor);

	/* g_assert(result); */
	if (result!=NULL && result->children==NULL) {
		/* is this necessary? */
		xmlFreeDoc(result);
		result = NULL;
	}
	if (result==NULL) {
		gchar *why_failed = g_strdup_printf (_("There was a problem applying the stylesheet file \"%s\" to \"%s\""),
						     stylesheet_filename,
						     cong_document_get_filename(doc));
#ifdef SILLY_IDEA
		if (list_of_parameters!=NULL) {
			/*
			 * add the stylesheet parameters to the dialog. This is probably a bad idea if the list 
			 * contains more than a few items. Two problems when there are a large number of 
			 * parameters are:
			 *   1 - the loop below is inefficient in this situation
			 *   2 - the dialog will be too large to read effectively
			 *
			 * could use the libxslt_params array rather than the list_of_parameters since this
			 * can be used by g_strjoinv() and other related functions.
			 */
			GList *iter;
			CongStylesheetParameter *param;
			gchar *temp = g_strconcat (why_failed, _("\n\nThe following stylesheet parameters were used:\n"));
			g_free (why_failed);
			why_failed = temp;

			for (iter=list_of_parameters; iter; iter=iter->next) {
				param = (CongStylesheetParameter *)iter->data;
				temp = g_strdup_printf ("%s  %s\t%s\n", why_failed, param->name, param->value);
				g_free (why_failed);
				why_failed = temp;
			}
		}
#endif
		GtkDialog* dialog = cong_error_dialog_new (toplevel_window,
							   _("Conglomerate could not transform the document"),
							   why_failed,
							   "");
	
		cong_error_dialog_run (GTK_DIALOG(dialog));
		gtk_widget_destroy (GTK_WIDGET(dialog));
	}

	xmlFreeDoc(input_clone);
	free_libxslt_params (libxslt_params);
	xsltFreeStylesheet(xsl);

	return result;
}

/**
 * cong_ui_transform_doc_to_uri:
 * @doc:
 * @stylesheet_filename:
 * @list_of_parameters: a #GList of #CongStylesheetParameter
 * @file:
 * @toplevel_window:
 *
 * Applies the stylesheet (@stylesheet_filename) to the
 * document (@doc) and saves the output to @file.
 * 
 * At present, it is assumed that a dialog window will be
 * created if there is an error in the processing (e.g.
 * unable to find the stylesheet or write to the given URI).
 * No dialog is created on success (so that the calling routine
 * can customise such a dialog, or not have one).
 *
 * It also relies on the #cong_ui_transform_doc() routines for
 * providing feed-back to the user that something is happening.
 *
 * Returns: true on success, false on failure.
 */
gboolean
cong_ui_transform_doc_to_uri(CongDocument *doc,
			     const gchar *stylesheet_filename,
			     GList *list_of_parameters,
			     GFile *file,
			     GtkWindow *toplevel_window)
{
	xmlDocPtr doc_ptr;
	gboolean result;
	gsize file_size;
	GError *error = NULL;

	g_return_val_if_fail (doc, 0);
	g_return_val_if_fail (stylesheet_filename, 0);
	g_return_val_if_fail (file, 0);

	doc_ptr = cong_ui_transform_doc(doc,
					stylesheet_filename,
					list_of_parameters,
					toplevel_window);
	if (doc_ptr == NULL) {
		return 0;
	}

	result = cong_vfs_save_xml_to_file (doc_ptr,
	                                    file,
	                                    &file_size,
	                                    &error);
		
	if (!result) {
		GtkDialog* dialog = cong_error_dialog_new_from_file_save_failure(toplevel_window,
										 file,
										 error,
										 &file_size);
			
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		return 0;
	}
		
	xmlFreeDoc(doc_ptr);
	return 1;
}


