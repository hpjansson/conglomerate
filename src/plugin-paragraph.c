/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-paragraph.c
 *
 * Plugin for list support
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

#include "cong-eel.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-font.h"

#include "cong-fake-plugin-hooks.h"

#include "plugin-paragraph-node-element-paragraph.h"

static CongEditorNodeElement*  
manufacture_editor_node_paragraph (CongServiceEditorNodeFactory *plugin_editor_node_factory, 
				   CongEditorWidget3 *editor_widget, 
				   CongTraversalNode *traversal_node,
				   gpointer user_data)
{
#if 0
	g_message("manufacture_editor_node_paragraph");
#endif

	return CONG_EDITOR_NODE_ELEMENT( cong_editor_node_element_paragraph_new (editor_widget,
										 traversal_node));
}

/* would be exposed as "plugin_register"? */
/**
 * plugin_paragraph_plugin_register:
 * @plugin:
 *
 * TODO: Write me
 */
gboolean 
plugin_paragraph_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);
	
	cong_plugin_register_editor_node_factory(plugin, 
						 _("Paragraph"), 
						 _("An editor node for visualising a paragraph"),
						 "paragraph",
						 manufacture_editor_node_paragraph,
						 NULL);
	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
/**
 * plugin_paragraph_plugin_configure:
 * @plugin:
 *
 * TODO: Write me
 */
gboolean 
plugin_paragraph_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
