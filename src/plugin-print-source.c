/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-print-source.c
 *
 * Plugin for printing XML in source form
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
#include "cong-plugin.h"
#include "cong-error-dialog.h"
#include "cong-progress-checklist.h"
#include "cong-document.h"
#include "cong-app.h"
#include "cong-util.h"
#include "cong-attribute-editor.h"

#include "cong-fake-plugin-hooks.h"
#include "cong-service-print-method.h"

#if ENABLE_PRINTING
gboolean 
source_print_method_document_filter (CongServicePrintMethod *print_method, 
				     CongDocument *doc, 
				     gpointer user_data)
{
	g_return_val_if_fail (print_method, FALSE);
	g_return_val_if_fail (doc, FALSE);

	/* This plugin can print every document.  */
	return TRUE;
}

void 
source_print_method_action_callback (CongServicePrintMethod *print_method, 
				     CongDocument *doc, 
				     GnomePrintContext *gpc, 
				     gpointer user_data, 
				     GtkWindow *toplevel_window)
{
#if 0
	GtkWidget *progress_checklist_dialog;
	CongProgressChecklist *progress_checklist;
	xmlDocPtr fo_doc;
#endif
	
	g_message("source_print_method_action_callback");

	/* I'd like to use GtkSourceView for printing.  
	   Unfortunately, the GtkSourceView API currently likes to create its own GnomePrintJob, and then use the GnomePrintContext from this.

	   What should the Conglomerate printing plugin API be?  Or should we have more than one (perhaps as subclasses) to allow for this kind of thing?
	*/

	CONG_DO_UNIMPLEMENTED_DIALOG_WITH_BUGZILLA_ID (toplevel_window, _("Print XML source"), 123138);
}
#endif /* #if ENABLE_PRINTING */


/* would be exposed as "plugin_register"? */
gboolean 
plugin_print_source_plugin_register (CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);
	
#if ENABLE_PRINTING
	cong_plugin_register_print_method (plugin, 
					   _("Print XML source"),
					   "",
					   "source-print",
					   source_print_method_document_filter,
					   source_print_method_action_callback,
					   NULL);
#endif

	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean 
plugin_print_source_plugin_configure (CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
