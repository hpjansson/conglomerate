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

struct validation_attempt
{
};

static void on_validation_error(void *ctx,
				const char *msg,
				...)
{
	gchar *str;

	CONG_GET_VAR_STR(msg,str)

	g_message("on_validation_error: \"%s\"", str);

	g_free(str);

}

static void on_validation_warning(void *ctx,
				  const char *msg,
				  ...)
{
	gchar *str;

	CONG_GET_VAR_STR(msg,str)

	g_message("on_validation_warning: \"%s\"", str);

	g_free(str);
}

static void on_details(gpointer data)
{
	CONG_DO_UNIMPLEMENTED_DIALOG_WITH_BUGZILLA_ID(NULL, "Can't supply details about validation failure", 113758);
}

static gboolean doc_filter(CongServiceDocTool *tool, CongDocument *doc, gpointer user_data)
{
	/* Always appropriate: */
	return TRUE;
}

static void action_callback(CongServiceDocTool *tool, CongPrimaryWindow *primary_window, gpointer user_data)
{
	CongDocument *doc = cong_primary_window_get_document(primary_window);
	xmlDocPtr xml_doc = cong_document_get_xml(doc);
#if 0
	GtkWidget *window;
	GtkWidget *log_view;
#endif

	int result;

	xmlValidCtxt ctxt; /* Is there a better way of manipulating this? */

	g_message("action_callback");

	if ((xml_doc->intSubset == NULL) && (xml_doc->extSubset == NULL)) {
		gchar *what_failed = g_strdup_printf(_("The document \"%s\" cannot be checked for validity."), cong_document_get_filename(doc));
		gchar* why_failed = g_strdup_printf("The document does not identify what set of rules it is supposed to comply with (known as a \"document type\").");
		GtkDialog * dialog;
			
		dialog = cong_error_dialog_new(cong_primary_window_get_toplevel(primary_window),
					       what_failed,
					       why_failed,
					       _("In a future version of Conglomerate we may have a feature that lets you attach a document type to a document."));
		
		g_free(what_failed);
		g_free(why_failed);

		cong_error_dialog_run(dialog);
		gtk_widget_destroy(GTK_WIDGET(dialog));
		return;
	}

	ctxt.error = on_validation_error;
	ctxt.warning = on_validation_warning;

	result = xmlValidateDocument(&ctxt,
				     xml_doc);

	switch (result) {
	default: g_assert_not_reached();
	case 0: /* Failure: */
		{
			gchar *what_failed = g_strdup_printf(_("The document \"%s\" is not valid."), cong_document_get_filename(doc));
			gchar* why_failed = g_strdup_printf("FIXME");
			GtkDialog * dialog;

			dialog = cong_error_dialog_new_with_convenience(cong_primary_window_get_toplevel(primary_window),
									what_failed,
									why_failed,
									_("Click on the \"Details\" button for more information."),
									_("_Details"),
									on_details,
									NULL);

			g_free(what_failed);
			g_free(why_failed);

			cong_error_dialog_run(dialog);
			gtk_widget_destroy(GTK_WIDGET(dialog));
		}
		return;

	case 1: /* Success: */
		{
			gchar *message = g_strdup_printf(_("The document \"%s\" is valid."), cong_document_get_filename(doc));
			GtkDialog *dialog = cong_dialog_information_alert_new(cong_primary_window_get_toplevel(primary_window),
									      message);

			g_free(message);

			gtk_dialog_run(dialog);
			gtk_widget_destroy(GTK_WIDGET(dialog));
		}
		return;
	}
	
}

 /* would be exposed as "plugin_register"? */
gboolean plugin_validate_plugin_register(CongPlugin *plugin)
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

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean plugin_validate_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
