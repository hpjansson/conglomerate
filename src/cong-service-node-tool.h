/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-node-tool.h
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

#ifndef __CONG_SERVICE_NODE_TOOL_H__
#define __CONG_SERVICE_NODE_TOOL_H__

#include "cong-plugin.h"

G_BEGIN_DECLS

#define CONG_SERVICE_NODE_TOOL_TYPE	  (cong_service_node_tool_get_type ())
#define CONG_SERVICE_NODE_TOOL(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_SERVICE_NODE_TOOL_TYPE, CongServiceNodeTool)
#define CONG_SERVICE_NODE_TOOL_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_SERVICE_NODE_TOOL_TYPE, CongServiceNodeToolClass)
#define IS_CONG_SERVICE_NODE_TOOL(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_SERVICE_NODE_TOOL_TYPE)
CONG_DECLARE_CLASS (CongServiceNodeTool, cong_service_node_tool, CongServiceTool)

enum NodeToolFilterResult
{
	NODE_TOOL_HIDDEN = 3,
	NODE_TOOL_INSENSITIVE = 4,
	NODE_TOOL_AVAILABLE = 5
};

typedef enum NodeToolFilterResult
(*CongServiceNodeToolFilter) (CongServiceNodeTool *node_tool, 
			      CongDocument *doc, 
			      CongNodePtr node,
			      gpointer user_data);
typedef void 
(*CongServiceNodeToolActionCallback) (CongServiceNodeTool *tool, 
				      CongDocument *doc, 
				      CongNodePtr node,
				      GtkWindow *parent_window,
				      gpointer user_data);

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
				  gpointer user_data);

CongServiceNodeTool*
cong_plugin_register_node_tool (CongPlugin *plugin,
				const gchar *name, 
				const gchar *description,
				const gchar *service_id,
				const gchar *menu_text,
				const gchar *tooltip_text,
				const gchar *tooltip_further_text,
				CongServiceNodeToolFilter node_filter,
				CongServiceNodeToolActionCallback action_callback,
				gpointer user_data);

enum NodeToolFilterResult
cong_service_node_tool_filter_node (CongServiceNodeTool *node_tool, 
				    CongDocument *doc,
				    CongNodePtr node);
void
cong_service_node_tool_invoke (CongServiceNodeTool *node_tool, 
			       CongDocument *doc,
			       CongNodePtr node,
			       GtkWindow *parent_window);

void
cong_plugin_for_each_node_tool (CongPlugin *plugin, 
				void 
				(*callback) (CongServiceNodeTool *node_tool, 
					     gpointer user_data), 
				gpointer user_data);

G_END_DECLS

#endif



