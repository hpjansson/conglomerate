/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-validate.c
 *
 * Validation tool
 *
 * Copyright (C) 2003 David Malcolm
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
#include "cong-plugin.h"
#include "cong-document.h"
#include "cong-error-dialog.h"
#include "cong-dialog.h"
#include "cong-primary-window.h"

#include <libxml/valid.h>

#include "cong-fake-plugin-hooks.h"
#include "cong-util.h"

#define WARNING_TAG "warning"
#define ERROR_TAG   "error"

typedef struct _ValidationDetails {
  GtkTextBuffer *buffer;
} ValidationDetails;

static void on_validation_error(void * ctxt,
				const char *msg,
				...)
{
  va_list args;
  gchar *buf;
  GtkTextIter end;
   ValidationDetails *details = (ValidationDetails*) ctxt;
  va_start (args, msg);
  buf = g_strdup_vprintf (msg, args);
  va_end (args);

  gtk_text_buffer_get_end_iter (details->buffer, &end);
  gtk_text_buffer_insert_with_tags_by_name (details->buffer, &end, buf, -1, ERROR_TAG, NULL);

  g_free (buf);
}

static void on_validation_warning(void *ctx,
				  const char *msg,
				  ...)
{
  ValidationDetails *details = (ValidationDetails *) ctx;
  va_list args;
  gchar *buf;
  GtkTextIter end;
    
  va_start (args, msg);
  buf = g_strdup_vprintf (msg, args);
  va_end (args);

  gtk_text_buffer_get_end_iter (details->buffer, &end);
  gtk_text_buffer_insert (details->buffer, &end, buf, -1);
  
  g_free (buf);
}

static gboolean doc_filter(CongServiceDocTool *tool, CongDocument *doc, gpointer user_data)
{
	/* Always appropriate: */
	return TRUE;
}

static GtkTextBuffer * create_buffer (void) {
  GtkTextBuffer *buffer;
  
  buffer = gtk_text_buffer_new (NULL);
  
  gtk_text_buffer_create_tag (buffer, WARNING_TAG, NULL);
  gtk_text_buffer_create_tag (buffer, ERROR_TAG, 
			      "weight", PANGO_WEIGHT_BOLD, 
			      "weight-set", TRUE,
			      NULL);
  
  return buffer;
}

static GtkWidget *create_dialog (const gchar *doc_name, GtkWindow* parent, GtkTextBuffer *buffer) {

  GtkWidget *dialog;
  GtkWidget *text_view;
  GtkWidget *alert;
  GtkWidget *expander;
  GtkWidget *sw;
  gchar *title;
  
  dialog = gtk_dialog_new_with_buttons ("Validation failed", parent, 
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_CLOSE, GTK_RESPONSE_OK, 
					NULL);
  text_view = gtk_text_view_new_with_buffer (buffer);    

  title = g_strdup_printf ("Document \"%s\" is not valid", doc_name);
  alert = cong_alert_content_new (GTK_STOCK_DIALOG_INFO, title, NULL, NULL);
  g_free (title);

  expander = gtk_expander_new_with_mnemonic ("_Details");
  sw = gtk_scrolled_window_new (NULL, NULL);

  gtk_container_add (GTK_CONTAINER (sw), text_view);
  gtk_container_add (GTK_CONTAINER(expander), sw);  
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), alert, FALSE, FALSE, 6);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), expander, TRUE, TRUE, 6);

  gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);
  gtk_widget_set_size_request (sw, -1, 200);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), 
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_text_view_set_editable (GTK_TEXT_VIEW(text_view), FALSE);				  

  gtk_widget_show_all (GTK_DIALOG(dialog)->vbox);
  
  return dialog;
}

static void 
on_add_dtd(gpointer data)
{
	CongPrimaryWindow *primary_window = (CongPrimaryWindow *)data;
	CongDocument *doc = cong_primary_window_get_document (primary_window);

	cong_util_run_add_dtd_dialog (doc,
				      cong_primary_window_get_toplevel(primary_window));
}

static void action_callback(CongServiceDocTool *tool, CongPrimaryWindow *primary_window, gpointer user_data)
{
	CongDocument *doc = cong_primary_window_get_document(primary_window);
	xmlDocPtr xml_doc = cong_document_get_xml(doc);

	int result;
	xmlValidCtxtPtr ctxt; 
	ValidationDetails *details;
	
	if ((xml_doc->intSubset == NULL) && (xml_doc->extSubset == NULL)) {
		gchar *what_failed = g_strdup_printf(_("The document \"%s\" cannot be checked for validity."), cong_document_get_filename(doc));
		gchar* why_failed = g_strdup_printf("The document does not identify what set of rules it is supposed to comply with (known as a \"document type\").");
		GtkDialog * dialog;
			
		dialog = cong_error_dialog_new_with_convenience (cong_primary_window_get_toplevel(primary_window),
								 what_failed,
								 why_failed,
								 _("You can fix this by adding a Document Type Declaration (DTD) to the document"),
								 _("_Add a DTD"),
								 GTK_STOCK_CANCEL,
								 TRUE,
								 on_add_dtd,
								 primary_window);
		
		g_free(what_failed);
		g_free(why_failed);

		cong_error_dialog_run(dialog);
		gtk_widget_destroy(GTK_WIDGET(dialog));
		return;
	}

	details = g_new (ValidationDetails, 1);
	details->buffer = create_buffer ();	

	ctxt = xmlNewValidCtxt ();
	
	ctxt->error = on_validation_error;
	ctxt->warning = on_validation_warning;
	ctxt->userData = details;

	result = xmlValidateDocument(ctxt,
				     xml_doc);
	xmlFreeValidCtxt (ctxt);

	switch (result) {
	default: g_assert_not_reached();
	case 0: /* Failure: */
		{
		GtkWidget *dialog;
		
		dialog = create_dialog (cong_document_get_filename(doc),
					cong_primary_window_get_toplevel(primary_window),
				        details->buffer);
					
		gtk_dialog_run (GTK_DIALOG(dialog));
		gtk_widget_destroy (dialog);
		}
		break;

	case 1: /* Success: */
		{
			gchar *message = g_strdup_printf(_("The document \"%s\" is valid."), cong_document_get_filename(doc));
			GtkDialog *dialog = cong_dialog_information_alert_new(cong_primary_window_get_toplevel(primary_window),
									      message);

			g_free(message);

			gtk_dialog_run(dialog);
			gtk_widget_destroy(GTK_WIDGET(dialog));
		}
		break;
	}
    
	g_object_unref (details->buffer);
	g_free (details);

	return;
}

/**
 * plugin_validate_plugin_register: Register
 * @plugin: Plugin
 *
 * Registers validate plugin - plugin that provides menu item
 * that validates document agains DTD and show validation 
 * warining and errors.
 *
 * Returns:
 */
gboolean 
plugin_validate_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	cong_plugin_register_doc_tool(plugin, 
				      _("Validate Document"),
				      _("Checks to see if the document is \"valid\" i.e. that it matches a set of rules given by the corresponding document type or schema"),
				      "validate",
				      _("_Validate Document"),
				      _("Checks to see if the document is \"valid\" i.e. that it matches a set of rules given by the corresponding document type or schema"),
				      _("Checks to see if the document is \"valid\" i.e. that it matches a set of rules given by the corresponding document type or schema"),
				      doc_filter,
				      action_callback,
				      NULL);

	return TRUE;
}

/**
 * plugin_validate_plugin_configure: Configure validate plugin
 * @plugin: Plugin
 *
 * This function should contain all code to configure validate plugin
 *
 * Returns: 
 */
gboolean 
plugin_validate_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
