/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-doc-tool.c
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
#include "cong-service-doc-tool.h"

struct CongServiceDocToolPrivate
{
	CongServiceDocToolFilter doc_filter;
	CongServiceDocToolActionCallback action_callback;
};

CONG_DEFINE_CLASS (CongServiceDocTool, cong_service_doc_tool, CONG_SERVICE_DOC_TOOL, CongServiceTool, CONG_SERVICE_TOOL_TYPE)

/**
 * cong_service_doc_tool_construct:
 * @tool:
 * @name:
 * @description:
 * @service_id:
 * @menu_text:
 * @tooltip_text:
 * @tooltip_further_text:
 * @doc_filter:
 * @action_callback:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceDocTool*
cong_service_doc_tool_construct (CongServiceDocTool *tool,
				 const gchar *name, 
				 const gchar *description,
				 const gchar *service_id,
				 const gchar *menu_text,
				 const gchar *tooltip_text,
				 const gchar *tooltip_further_text,
				 CongServiceDocToolFilter doc_filter,
				 CongServiceDocToolActionCallback action_callback,
				 gpointer user_data)
{
	g_return_val_if_fail (IS_CONG_SERVICE_DOC_TOOL (tool), NULL);
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
	PRIVATE (tool)->doc_filter = doc_filter;
	PRIVATE (tool)->action_callback = action_callback;

	return tool;

}

/**
 * cong_doc_tool_supports_document:
 * @doc_tool:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
cong_doc_tool_supports_document (CongServiceDocTool *tool, 
				 CongDocument *doc)
{
	g_return_val_if_fail (tool, FALSE);
	g_return_val_if_fail (doc, FALSE);

	g_assert(PRIVATE (tool)->doc_filter);

	return PRIVATE (tool)->doc_filter (tool, 
					   doc, 
					   cong_service_tool_get_user_data (CONG_SERVICE_TOOL (tool)));
}

/**
 * cong_doc_tool_invoke:
 * @doc_tool:
 * @primary_window:
 *
 * TODO: Write me
 */
void 
cong_doc_tool_invoke (CongServiceDocTool *tool, 
		      CongPrimaryWindow *primary_window)
{
	g_return_if_fail (tool);

	g_assert(PRIVATE (tool)->action_callback);

	return PRIVATE (tool)->action_callback (tool, 
						primary_window, 
						cong_service_tool_get_user_data (CONG_SERVICE_TOOL (tool)));
}

