/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-tool.c
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
#include "cong-service-tool.h"

struct CongServiceToolPrivate
{
	const gchar *menu_text;
	const gchar *tooltip_text;
	const gchar *tooltip_further_text;
	gpointer user_data;
};

CONG_DEFINE_CLASS (CongServiceTool, cong_service_tool, CONG_SERVICE_TOOL, CongService, CONG_SERVICE_TYPE)

/**
 * cong_service_tool_construct:
 * @tool:
 * @name:
 * @description:
 * @service_id:
 * @menu_text:
 * @tooltip_text:
 * @tooltip_further_text:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceTool*
cong_service_tool_construct (CongServiceTool *tool,
			     const gchar *name, 
			     const gchar *description,
			     const gchar *service_id,
			     const gchar *menu_text,
			     const gchar *tooltip_text,
			     const gchar *tooltip_further_text,
			     gpointer user_data)
{
	g_return_val_if_fail (IS_CONG_SERVICE_TOOL (tool), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);

	cong_service_construct (CONG_SERVICE (tool),
				name,
				description,
				service_id);

	PRIVATE (tool)->menu_text = g_strdup (menu_text);
	PRIVATE (tool)->tooltip_text = g_strdup (tooltip_text);
	PRIVATE (tool)->tooltip_further_text = g_strdup (tooltip_further_text);
	PRIVATE (tool)->user_data = user_data;

	return tool;

}

/**
 * cong_service_tool_get_menu_text:
 * @tool:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_service_tool_get_menu_text (CongServiceTool *tool)
{
	g_return_val_if_fail (IS_CONG_SERVICE_TOOL (tool), NULL);

	return PRIVATE (tool)->menu_text;
}

/**
 * cong_service_tool_get_tip_text:
 * @tool:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_service_tool_get_tip_text (CongServiceTool *tool)
{
	g_return_val_if_fail (IS_CONG_SERVICE_TOOL (tool), NULL);

	return PRIVATE (tool)->tooltip_text;
}

/**
 * cong_service_tool_get_tip_further_text:
 * @tool:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_service_tool_get_tip_further_text (CongServiceTool *tool)
{
	g_return_val_if_fail (IS_CONG_SERVICE_TOOL (tool), NULL);

	return PRIVATE (tool)->tooltip_further_text;
}

/**
 * cong_service_tool_get_user_data:
 * @tool:
 *
 * TODO: Write me
 * Returns:
 */
gpointer
cong_service_tool_get_user_data (CongServiceTool *tool)
{
	g_return_val_if_fail (IS_CONG_SERVICE_TOOL (tool), NULL);

	return PRIVATE (tool)->user_data;
}
