/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-debug.c
 *
 * Debug plugin, for helping track down Conglomerate bugs
 *
 * Copyright (C) 2005 David Malcolm
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

#include "cong-primary-window.h"

#include "cong-fake-plugin-hooks.h"

#if ENABLE_DEBUG_PLUGIN
static gboolean 
dump_area_tree_doc_filter (CongServiceDocTool *tool, 
			   CongDocument *doc, 
			   gpointer user_data)
{
	/* Only available for documents using the editor widget */
	CongPrimaryWindow *primary_window = cong_document_get_primary_window(doc);

	return primary_window->cong_editor_widget3 != NULL;
}


static void
dump_area_tree_action_callback (CongServiceDocTool *tool, 
				CongPrimaryWindow *primary_window, 
				gpointer user_data)
{
	CongEditorWidget3 *editor_widget;
	xmlDocPtr xml_doc;
	g_return_if_fail (primary_window);

	editor_widget = CONG_EDITOR_WIDGET3 (primary_window->cong_editor_widget3);
	xml_doc = cong_editor_widget_debug_dump_area_tree (editor_widget);
	
	cong_ui_new_document_from_manufactured_xml(xml_doc,
						   cong_primary_window_get_toplevel (primary_window));
}
#endif

/* would be exposed as "plugin_register"? */
/**
 * plugin_debug_plugin_register:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_debug_plugin_register (CongPlugin *plugin)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), FALSE);

#if ENABLE_DEBUG_PLUGIN
	cong_plugin_register_doc_tool(plugin, 
				      "Dump area tree",
				      "Generates a debug dump of the CongEditorArea tree of the CongEditorWidget3 as another XML document.",
				      "dump-area-tree",
				      "Debug: _Dump Area Tree",
				      NULL,
				      NULL,
				      dump_area_tree_doc_filter,
				      dump_area_tree_action_callback,
				      NULL);
#endif
	
	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
/**
 * plugin_debug_plugin_configure:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_debug_plugin_configure (CongPlugin *plugin)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), FALSE);

	return TRUE;
}
