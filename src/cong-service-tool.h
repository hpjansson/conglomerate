/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-tool.h
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

#ifndef __CONG_SERVICE_TOOL_H__
#define __CONG_SERVICE_TOOL_H__

#include "cong-service.h"

G_BEGIN_DECLS

#define CONG_SERVICE_TOOL_TYPE	  (cong_service_tool_get_type ())
#define CONG_SERVICE_TOOL(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_SERVICE_TOOL_TYPE, CongServiceTool)
#define CONG_SERVICE_TOOL_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_SERVICE_TOOL_TYPE, CongServiceToolClass)
#define IS_CONG_SERVICE_TOOL(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_SERVICE_TOOL_TYPE)
CONG_DECLARE_CLASS (CongServiceTool, cong_service_tool, CongService)

CongServiceTool*
cong_service_tool_construct (CongServiceTool *tool,
			     const gchar *name, 
			     const gchar *description,
			     const gchar *service_id,
			     const gchar *menu_text,
			     const gchar *tooltip_text,
			     const gchar *tooltip_further_text,
			     gpointer user_data);

const gchar*
cong_service_tool_get_menu_text (CongServiceTool *tool);

const gchar*
cong_service_tool_get_tip_text (CongServiceTool *tool);

const gchar*
cong_service_tool_get_tip_further_text (CongServiceTool *tool);

gpointer
cong_service_tool_get_user_data (CongServiceTool *tool);

G_END_DECLS

#endif



