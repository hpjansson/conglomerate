/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-html.c
 *
 * HTML plugin
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

#include "cong-fake-plugin-hooks.h"

#define XHTML_NS_URI ("http://www.w3.org/1999/xhtml")

static const gchar *promotable_html_elements[] = {"h2", "h3", "h4", "h5", "h6"};
static const gchar *demotable_html_elements[] = {"h1", "h2", "h3", "h4", "h5"};

static enum NodeToolFilterResult 
node_filter_browse_url (CongServiceNodeTool *node_tool, 
			CongDocument *doc, 
			CongNodePtr node,
			gpointer user_data)
{
	if (cong_node_is_element (node,
				  XHTML_NS_URI,
				  "a")) {
		return NODE_TOOL_AVAILABLE;
	} else {
		return NODE_TOOL_HIDDEN;
	}
}

static void 
action_callback_browse_url (CongServiceNodeTool *tool, 
			    CongDocument *doc, 
			    CongNodePtr node,
			    GtkWindow *parent_window,
			    gpointer user_data)
{
	CONG_DO_UNIMPLEMENTED_DIALOG (parent_window,
				      "browse URL");
}

static enum NodeToolFilterResult
node_filter_promote (CongServiceNodeTool *node_tool, 
		     CongDocument *doc, 
		     CongNodePtr node,
		     gpointer user_data)
{
	if (cong_node_is_element_from_set (node, 
					   XHTML_NS_URI,
					   promotable_html_elements,
					   G_N_ELEMENTS (promotable_html_elements),
					   NULL)) {
		return NODE_TOOL_AVAILABLE;
	} else {
		return NODE_TOOL_HIDDEN;
	}
}

static void 
action_callback_promote (CongServiceNodeTool *tool, 
			 CongDocument *doc, 
			 CongNodePtr node,
			 GtkWindow *parent_window,
			 gpointer user_data)
{
	CONG_DO_UNIMPLEMENTED_DIALOG (parent_window,
				      "promote HTML");
}

static enum NodeToolFilterResult
node_filter_demote (CongServiceNodeTool *node_tool, 
		    CongDocument *doc, 
		    CongNodePtr node,
		    gpointer user_data)
{
	if (cong_node_is_element_from_set (node, 
					   XHTML_NS_URI,
					   demotable_html_elements,
					   G_N_ELEMENTS (demotable_html_elements),
					   NULL)) {
		return NODE_TOOL_AVAILABLE;
	} else {
		return NODE_TOOL_HIDDEN;
	}
}

static void 
action_callback_demote (CongServiceNodeTool *tool, 
			CongDocument *doc,
			CongNodePtr node,
			GtkWindow *parent_window,
			gpointer user_data)
{
	CONG_DO_UNIMPLEMENTED_DIALOG (parent_window,
				      "demote HTML");
}


/* would be exposed as "plugin_register"? */
/**
 * plugin_html_plugin_register:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_html_plugin_register (CongPlugin *plugin)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), FALSE);

	/* Register your services here: */
#if 1
	cong_plugin_register_node_tool (plugin,
					_("Open Link in Browser"), 
					"",
					"html-browse-to-url",
					_("Open Link in Browser"),
					NULL,
					NULL,
					node_filter_browse_url,
					action_callback_browse_url,
					NULL);

	cong_plugin_register_node_tool (plugin,
					_("Promote"), 
					"",
					"html-promote",
					_("Promote"),
					NULL,
					NULL,
					node_filter_promote,
					action_callback_promote,
					NULL);

	cong_plugin_register_node_tool (plugin,
					_("Demote"),
					"",
					"html-demote",
					_("Demote"),
					NULL,
					NULL,
					node_filter_demote,
					action_callback_demote,
					NULL);
#endif
	
	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
/**
 * plugin_html_plugin_configure:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_html_plugin_configure (CongPlugin *plugin)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), FALSE);

	return TRUE;
}
