/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-doc-tool.h
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

#ifndef __CONG_SERVICE_DOC_TOOL_H__
#define __CONG_SERVICE_DOC_TOOL_H__

#include "cong-service-tool.h"

G_BEGIN_DECLS

#define CONG_SERVICE_DOC_TOOL_TYPE	  (cong_service_doc_tool_get_type ())
#define CONG_SERVICE_DOC_TOOL(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_SERVICE_DOC_TOOL_TYPE, CongServiceDocTool)
#define CONG_SERVICE_DOC_TOOL_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_SERVICE_DOC_TOOL_TYPE, CongServiceDocToolClass)
#define IS_CONG_SERVICE_DOC_TOOL(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_SERVICE_DOC_TOOL_TYPE)
CONG_DECLARE_CLASS (CongServiceDocTool, cong_service_doc_tool, CongServiceTool)

typedef gboolean 
(*CongServiceDocToolFilter) (CongServiceDocTool *doc_tool, 
			     CongDocument *doc, 
			     gpointer user_data);
typedef void 
(*CongServiceDocToolActionCallback) (CongServiceDocTool *doc_tool, 
				     CongPrimaryWindow *primary_window, 
				     gpointer user_data);

CongServiceDocTool*
cong_plugin_register_doc_tool (CongPlugin *plugin,
			       const gchar *name, 
			       const gchar *description,
			       const gchar *service_id,
			       const gchar *menu_text,
			       const gchar *tooltip_text,
			       const gchar *tooltip_further_text,
			       CongServiceDocToolFilter doc_filter,
			       CongServiceDocToolActionCallback action_callback,
			       gpointer user_data);

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
				 gpointer user_data);

gboolean 
cong_doc_tool_supports_document (CongServiceDocTool *doc_tool, 
				 CongDocument *doc);
void 
cong_doc_tool_invoke (CongServiceDocTool *doc_tool, 
		      CongPrimaryWindow *primary_window);

void 
cong_plugin_for_each_doc_tool (CongPlugin *plugin, 
			       void 
			       (*callback) (CongServiceDocTool *doc_tool, 
					    gpointer user_data), 
			       gpointer user_data);

G_END_DECLS

#endif



