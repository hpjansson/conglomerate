/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-node-tool.c
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
#include "cong-service-node-tool.h"

struct CongServiceNodeToolPrivate
{
	CongServiceNodeToolFilter node_filter;
	CongServiceNodeToolActionCallback action_callback;
};

CONG_DEFINE_CLASS (CongServiceNodeTool, cong_service_node_tool, CONG_SERVICE_NODE_TOOL, CongServiceTool, CONG_SERVICE_TOOL_TYPE)

/**
 * cong_service_node_tool_construct:
 * @tool:
 * @name: 
 * @description:
 * @service_id:
 * @menu_text:
 * @tooltip_text:
 * @tooltip_further_text:
 * @node_filter:
 * @action_callback:
 * @user_data:
 *
 * TODO: Write me
 */
CongServiceNodeTool*
cong_service_node_tool_construct (CongServiceNodeTool *tool,
				  const gchar *name, 
				  const gchar *description,
				  const gchar *service_id,
				  const gchar *menu_text,
				  const gchar *tooltip_text,
				  const gchar *tooltip_further_text,
				  CongServiceNodeToolFilter node_filter,
				  CongServiceNodeToolActionCallback action_callback,
				  gpointer user_data)
{
	g_return_val_if_fail (IS_CONG_SERVICE_NODE_TOOL (tool), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);

	cong_service_tool_construct (CONG_SERVICE_TOOL (tool),
				     name,
				     description,
				     service_id,
				     menu_text,
				     tooltip_text,
				     tooltip_further_text,
				     user_data);
	PRIVATE (tool)->node_filter = node_filter;
	PRIVATE (tool)->action_callback = action_callback;

	return tool;
}

/**
 * cong_service_node_tool_filter_node:
 * @tool:
 * @doc: 
 * @node:
 *
 * TODO: Write me
 */
enum NodeToolFilterResult
cong_service_node_tool_filter_node (CongServiceNodeTool *tool, 
				    CongDocument *doc,
				    CongNodePtr node)
{
	g_return_val_if_fail (IS_CONG_SERVICE_NODE_TOOL (tool), FALSE);
	g_return_val_if_fail (doc, FALSE);
	g_return_val_if_fail (node, FALSE);

	g_assert (PRIVATE (tool)->node_filter);
	return PRIVATE (tool)->node_filter (tool, 
					    doc, 
					    node, 
					    cong_service_tool_get_user_data (CONG_SERVICE_TOOL (tool)));
}

/**
 * cong_service_node_tool_invoke:
 * @tool:
 * @doc: 
 * @node:
 * @parent_window:
 *
 * TODO: Write me
 */
void 
cong_service_node_tool_invoke (CongServiceNodeTool *tool,
			       CongDocument *doc,
			       CongNodePtr node,
			       GtkWindow *parent_window)
{
	g_return_if_fail (IS_CONG_SERVICE_NODE_TOOL (tool));
	g_return_if_fail (node);

	g_assert (PRIVATE (tool)->action_callback);
	return PRIVATE (tool)->action_callback (tool, 
						doc,
						node, 
						parent_window,
						cong_service_tool_get_user_data (CONG_SERVICE_TOOL (tool)));
}

