/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-plugin-manager.c
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
#include "cong-plugin-manager.h"
#include "cong-plugin.h"

struct CongPluginManager
{
	GList *list_of_plugin; /* ptrs of type CongPlugin */
};

/* Implementation of CongPluginManager: */
CongPluginManager *cong_plugin_manager_new(void)
{
	CongPluginManager *manager;

	manager = g_new0 (CongPluginManager,1);
	
	return manager;
}

CongPlugin*
cong_plugin_manager_register (CongPluginManager *plugin_manager,
			      const gchar *plugin_id,
			      CongPluginCallbackRegister register_callback,
			      CongPluginCallbackConfigure configure_callback)
{
	CongPlugin *plugin;

	g_return_val_if_fail (plugin_manager, NULL);
	g_return_val_if_fail (plugin_id, NULL);
	g_return_val_if_fail (register_callback, NULL);
	/* the configure callback is allowed to be NULL */

	plugin = cong_plugin_construct (g_object_new (CONG_PLUGIN_TYPE, NULL),
					plugin_id,
					register_callback,
					configure_callback);

	/* Add to list of plugins: */
	plugin_manager->list_of_plugin = g_list_append (plugin_manager->list_of_plugin, 
							plugin);

	return plugin;
}

void
cong_plugin_manager_for_each_plugin (CongPluginManager *plugin_manager, 
				     void 
				     (*callback) (CongPlugin *plugin, 
						  gpointer user_data),
				     gpointer user_data)
{
	GList *iter;

	g_return_if_fail (plugin_manager);
	g_return_if_fail (callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		(*callback) (CONG_PLUGIN (iter->data),
			     user_data);
	}
}

void 
cong_plugin_manager_for_each_document_factory (CongPluginManager *plugin_manager, 
					       void 
					       (*callback) (CongServiceDocumentFactory *factory, 
							    gpointer user_data), 
					       gpointer user_data)
{
	GList *iter;
	g_return_if_fail(plugin_manager);
	g_return_if_fail (callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_document_factory(iter->data, callback, user_data);
	}
}

void 
cong_plugin_manager_for_each_importer (CongPluginManager *plugin_manager, 
				       void 
				       (*callback) (CongServiceImporter *importer, 
						    gpointer user_data), 
				       gpointer user_data)
{
	GList *iter;
	g_return_if_fail (plugin_manager);
	g_return_if_fail (callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_importer(iter->data, callback, user_data);
	}
}

void 
cong_plugin_manager_for_each_exporter (CongPluginManager *plugin_manager, 
				       void 
				       (*callback) (CongServiceExporter *exporter, 
						    gpointer user_data), 
				       gpointer user_data)
{
	GList *iter;
	g_return_if_fail (plugin_manager);
	g_return_if_fail (callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_exporter(iter->data, callback, user_data);
	}
}


#if ENABLE_PRINTING
void cong_plugin_manager_for_each_print_method (CongPluginManager *plugin_manager, 
						void 
						(*callback) (CongServicePrintMethod *print_method, 
							     gpointer user_data), 
						gpointer user_data)
{
	GList *iter;
	g_return_if_fail (plugin_manager);
	g_return_if_fail (callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_print_method(iter->data, callback, user_data);
	}
}
#endif

void cong_plugin_manager_for_each_doc_tool(CongPluginManager *plugin_manager, void (*callback)(CongServiceDocTool *tool, gpointer user_data), gpointer user_data)
{
	GList *iter;
	g_return_if_fail (plugin_manager);
	g_return_if_fail (callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_doc_tool(iter->data, callback, user_data);
	}
}

void cong_plugin_manager_for_each_node_tool(CongPluginManager *plugin_manager, void (*callback)(CongServiceNodeTool *tool, gpointer user_data), gpointer user_data)
{
	GList *iter;
	g_return_if_fail (plugin_manager);
	g_return_if_fail (callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_node_tool(iter->data, callback, user_data);
	}
}

CongServiceNodePropertyDialog*
cong_plugin_manager_locate_custom_property_dialog_by_id (CongPluginManager *plugin_manager, 
							 const gchar *service_id)
{
	GList *plugin_iter;

	g_return_val_if_fail (plugin_manager, NULL);
	g_return_val_if_fail (service_id, NULL);

	for (plugin_iter=plugin_manager->list_of_plugin; plugin_iter; plugin_iter = plugin_iter->next) {
		CongServiceNodePropertyDialog* dialog = cong_plugin_locate_custom_property_dialog_by_id (CONG_PLUGIN (plugin_iter->data),
													  service_id);
		if (dialog) {
			return dialog;
		}
	}

	return NULL;
}

CongServiceEditorNodeFactory*
cong_plugin_manager_locate_editor_node_factory_by_id (CongPluginManager *plugin_manager,
						      const gchar *service_id)
{
	GList *plugin_iter;

	g_return_val_if_fail (plugin_manager, NULL);
	g_return_val_if_fail (service_id, NULL);

	for (plugin_iter=plugin_manager->list_of_plugin; plugin_iter; plugin_iter = plugin_iter->next) {
		CongServiceEditorNodeFactory* factory = cong_plugin_locate_editor_node_factory_by_id (CONG_PLUGIN (plugin_iter->data),
												      service_id);
		if (factory) {
			return factory;
		}
	}
	
	return NULL;
}

