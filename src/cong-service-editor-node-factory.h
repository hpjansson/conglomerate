/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-editor-node-factory.h
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

#ifndef __CONG_SERVICE_EDITOR_NODE_FACTORY_H__
#define __CONG_SERVICE_EDITOR_NODE_FACTORY_H__

#include "cong-service.h"
#include "cong-document.h"
#include "cong-editor-widget.h"
#include "cong-editor-node-element.h"

G_BEGIN_DECLS

#define CONG_SERVICE_EDITOR_NODE_FACTORY_TYPE	  (cong_service_editor_node_factory_get_type ())
#define CONG_SERVICE_EDITOR_NODE_FACTORY(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_SERVICE_EDITOR_NODE_FACTORY_TYPE, CongServiceEditorNodeFactory)
#define CONG_SERVICE_EDITOR_NODE_FACTORY_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_SERVICE_EDITOR_NODE_FACTORY_TYPE, CongServiceEditorNodeFactoryClass)
#define IS_CONG_SERVICE_EDITOR_NODE_FACTORY(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_SERVICE_EDITOR_NODE_FACTORY_TYPE)
CONG_DECLARE_CLASS (CongServiceEditorNodeFactory, cong_service_editor_node_factory, CongService)

typedef CongEditorNodeElement* 
(*CongEditorNodeFactoryMethod) (CongServiceEditorNodeFactory *plugin_editor_node_factory, 
				CongEditorWidget3 *editor_widget, 
				CongTraversalNode *traversal_node, 
				gpointer user_data);

CongServiceEditorNodeFactory*
cong_service_editor_node_factory_construct (CongServiceEditorNodeFactory *editor_node_factory,
					    const gchar *name, 
					    const gchar *description,
					    const gchar *plugin_id,
					    CongEditorNodeFactoryMethod factory_method,
					    gpointer user_data);

CongServiceEditorNodeFactory*
cong_plugin_register_editor_node_factory (CongPlugin *plugin, 
					  const gchar *name, 
					  const gchar *description,
					  const gchar *plugin_id,
					  CongEditorNodeFactoryMethod factory_method,
					  gpointer user_data);

CongEditorNodeElement*
cong_plugin_editor_node_factory_invoke (CongServiceEditorNodeFactory *plugin_editor_node_factory,
					CongEditorWidget3 *editor_widget, 
					CongTraversalNode *traversal_node);



G_END_DECLS

#endif



