/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-cleanup-source.c
 *
 * Tools for cleaning up the XML source of the document
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
#include "cong-fake-plugin-hooks.h"
#include "cong-dispspec.h"
#include "cong-dispspec-element.h"
#include "cong-command.h"
#include "cong-util.h"
#include "cong-source-layout.h"

static gboolean doc_filter(CongServiceDocTool *tool, CongDocument *doc, gpointer user_data)
{
	/* Always appropriate: */
	return TRUE;
}


static void action_callback(CongServiceDocTool *tool, CongPrimaryWindow *primary_window, gpointer user_data)
{
	CongSourceCleanupOptions options;

	/* FIXME: present a dialog box; use GConf etc */
	options.use_tabs = TRUE;
	options.wrap_text = TRUE;
	options.num_text_columns = 80;

	cong_util_cleanup_source(cong_primary_window_get_document(primary_window), &options);
}

/* would be exposed as "plugin_register"? */
/**
 * plugin_cleanup_source_plugin_register:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_cleanup_source_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	cong_plugin_register_doc_tool(plugin, 
				      _("Cleanup XML Source"),
				      _("Cleans up the XML source of the document, so that it is easy to read in a code editor."),
				      "cleanup",
				      _("_Clean the XML Source"),
				      _("Cleans up the XML source of the document, so that it is easy to read in a code editor."),
				      _("Cleans up the XML source of the document, so that it is easy to read in a code editor."),
				      doc_filter,
				      action_callback,
				      NULL);

	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
/**
 * plugin_cleanup_source_plugin_configure:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_cleanup_source_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
