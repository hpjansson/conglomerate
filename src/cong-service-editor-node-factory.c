/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-editor-node-factory.c
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
#include "cong-service-editor-node-factory.h"
#include "cong-traversal-node.h"

struct CongServiceEditorNodeFactoryPrivate
{
	CongEditorNodeFactoryMethod make_node;
	gpointer user_data;
};

CONG_DEFINE_CLASS (CongServiceEditorNodeFactory, cong_service_editor_node_factory, CONG_SERVICE_EDITOR_NODE_FACTORY, CongService, CONG_SERVICE_TYPE)

/**
 * cong_service_editor_node_factory_construct:
 * @editor_node_factory:
 * @name: 
 * @description:
 * @plugin_id:
 * @factory_method:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceEditorNodeFactory*
cong_service_editor_node_factory_construct (CongServiceEditorNodeFactory *editor_node_factory,
					    const gchar *name, 
					    const gchar *description,
					    const gchar *plugin_id,
					    CongEditorNodeFactoryMethod factory_method,
					    gpointer user_data)
{
	g_return_val_if_fail (IS_CONG_SERVICE_EDITOR_NODE_FACTORY (editor_node_factory), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (plugin_id, NULL);
	g_return_val_if_fail (factory_method, NULL);

	cong_service_construct (CONG_SERVICE (editor_node_factory),
				name,
				description,
				plugin_id);
	PRIVATE (editor_node_factory)->make_node = factory_method;
	PRIVATE (editor_node_factory)->user_data = user_data;

	return editor_node_factory;
}

/**
 * cong_plugin_editor_node_factory_invoke:
 * @plugin_editor_node_factory:
 * @editor_widget: 
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNodeElement*
cong_plugin_editor_node_factory_invoke (CongServiceEditorNodeFactory *service_editor_node_factory,
					CongEditorWidget3 *editor_widget, 
					CongTraversalNode *traversal_node)
{
	g_return_val_if_fail (IS_CONG_SERVICE_EDITOR_NODE_FACTORY (service_editor_node_factory), NULL);
	g_return_val_if_fail (editor_widget, NULL);
	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE (traversal_node), NULL);

	g_assert (PRIVATE (service_editor_node_factory)->make_node);

	return PRIVATE (service_editor_node_factory)->make_node (service_editor_node_factory, 
								 editor_widget, 
								 traversal_node,
								 PRIVATE (service_editor_node_factory)->user_data);
}
