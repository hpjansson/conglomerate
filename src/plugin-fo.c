/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-fo.c
 *
 * Plugin for XSL-FO support
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
#include "cong-error-dialog.h"
#include "fo.h"
#include "cong-progress-checklist.h"
#include "cong-document.h"
#include <glade/glade.h>

#include "cong-fake-plugin-hooks.h"

#define FO_NS_URI ("http://www.w3.org/1999/XSL/Format")

static gboolean is_fo(CongDocument *doc) 
{
	CongNodePtr root;

	g_return_val_if_fail(doc, FALSE);

	root = cong_document_get_root(doc);

	if (cong_node_is_element (root, FO_NS_URI, "root")) {
		return TRUE;
	}

	return FALSE;
}


gboolean fo_pdf_exporter_document_filter(CongServiceExporter *exporter, CongDocument *doc, gpointer user_data)
{
	g_return_val_if_fail(exporter, FALSE);
	g_return_val_if_fail(doc, FALSE);

	return is_fo(doc);
}

void fo_pdf_exporter_action_callback(CongServiceExporter *exporter, CongDocument *doc, const gchar *uri, gpointer user_data, GtkWindow *toplevel_window)
{
	g_message("fo_pdf_exporter_action_callback");

	g_return_if_fail(exporter);
	g_return_if_fail(doc);
	g_return_if_fail(uri);

	
	CONG_DO_UNIMPLEMENTED_DIALOG_WITH_BUGZILLA_ID(toplevel_window, _("Converting XSL Formatting Objects to PDF"), 108467);
	/* FIXME: ultimately we probably want to use xmlroff to do this stage */
}

#if (ENABLE_PRINTING && ENABLE_LIBFO)
gboolean fo_print_method_document_filter(CongServicePrintMethod *print_method, CongDocument *doc, gpointer user_data)
{
	g_return_val_if_fail(print_method, FALSE);
	g_return_val_if_fail(doc, FALSE);

	return is_fo(doc);
}

void fo_print_method_action_callback(CongServicePrintMethod *print_method, CongDocument *doc, GnomePrintContext *gpc, gpointer user_data, GtkWindow *toplevel_window)
{
	g_message("fo_print_method_action_callback");

	cong_util_print_xslfo(toplevel_window, gpc, cong_document_get_xml(doc));
}
#endif /* #if ENABLE_PRINTING */

 /* would be exposed as "plugin_register"? */
gboolean plugin_fo_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	cong_plugin_register_exporter(plugin, 
				      _("Export XSL-FO as PDF"), 
				      _("Generate a PDF file from the XSL:FO information"),
				      "fo-pdf-export",
				      fo_pdf_exporter_document_filter,
				      fo_pdf_exporter_action_callback,
				      NULL);

#if (ENABLE_PRINTING && ENABLE_LIBFO)
	cong_plugin_register_print_method(plugin, 
					  _("Print XSL:FO"),
					  "",
					  "fo-print",
					  fo_print_method_document_filter,
					  fo_print_method_action_callback,
					  NULL);
#endif
	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean plugin_fo_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
