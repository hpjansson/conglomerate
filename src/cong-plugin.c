/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-plugin.c
 *
 * Copyright (C) 2002 David Malcolm
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
#include "cong-error-dialog.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-dispspec-registry.h"
#include "cong-primary-window.h"
#include "cong-app.h"
#include "cong-util.h"
#include "cong-vfs.h"
#include "cong-traversal-node.h"

#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>

/* Private definitions of the types: */
struct CongPluginManager
{
	GList *list_of_plugin; /* ptrs of type CongPlugin */
};

struct CongPlugin
{
	gchar *plugin_id;

	CongPluginCallbackConfigure configure_callback;

	GList *list_of_document_factory; /* ptrs of type CongServiceDocumentFactory */
	GList *list_of_importer; /* ptrs of type CongServiceImporter */
	GList *list_of_exporter; /* ptrs of type CongServiceExporter */
#if ENABLE_PRINTING
	GList *list_of_print_method; /* ptrs of type CongServicePrintMethod */
#endif
	GList *list_of_thumbnailer; /* ptrs of type CongThumnbailer */
	GList *list_of_editor_element; /* ptrs of type CongPluginEditorElement */
	GList *list_of_editor_node_factory; /* ptrs of type CongServiceEditorNodeFactory */
	GList *list_of_doc_tool; /* ptrs of type CongServiceDocTool */
	GList *list_of_node_tool; /* ptrs of type CongServiceNodeTool */
	GList *list_of_property_dialog; /* ptrs of type CongServiceNodePropertyDialog */
};

struct CongService
{
	CongPlugin *plugin;
	gchar *name;
	gchar *description;
	gchar *service_id;
};

struct CongServiceDocumentFactory
{
	CongService service; /* base class */

	CongServiceDocumentFactoryPageCreationCallback page_creation_callback;
	CongServiceDocumentFactoryActionCallback action_callback;

	gchar *icon;
	GdkPixbuf *icon16;

	gpointer user_data;
};

struct CongServiceImporter
{
	CongService service; /* base class */

	CongServiceImporterMimeFilter mime_filter;
	CongServiceImporterActionCallback action_callback;
	gpointer user_data;
};

struct CongServiceExporter
{
	CongService service; /* base class */

	CongServiceExporterDocumentFilter doc_filter;
	CongServiceExporterActionCallback action_callback;
	gpointer user_data;
};

#if ENABLE_PRINTING
struct CongServicePrintMethod
{
	CongService service; /* base class */

	CongServicePrintMethodDocumentFilter doc_filter;
	CongServicePrintMethodActionCallback action_callback;
	gpointer user_data;
};
#endif

struct CongServiceThumbnailer
{
	CongService service; /* base class */
};

struct CongServiceEditorNodeFactory
{
	CongService service; /* base class */
	CongEditorNodeFactoryMethod make_node;
	gpointer user_data;
};

struct CongServiceTool
{
	CongService service; /* base class */
	const gchar *menu_text;
	const gchar *tooltip_text;
	const gchar *tooltip_further_text;
	gpointer user_data;
};

struct CongServiceDocTool
{
	CongServiceTool tool; /* base class */
	CongServiceDocToolFilter doc_filter;
	CongServiceDocToolActionCallback action_callback;
};

struct CongServiceNodeTool
{
	CongServiceTool tool; /* base class */
	CongServiceNodeToolFilter node_filter;
	CongServiceNodeToolActionCallback action_callback;
};

struct CongServiceNodePropertyDialog
{
	CongService service; /* base class */
	CongCustomPropertyFactoryMethod factory_method;
	gpointer user_data;
};


/* Implementation of CongPluginManager: */
CongPluginManager *cong_plugin_manager_new(void)
{
	CongPluginManager *manager;

	manager = g_new0(CongPluginManager,1);
	
	return manager;
}

CongPlugin *cong_plugin_manager_register(CongPluginManager *plugin_manager,
					 const gchar *id,
					 CongPluginCallbackRegister register_callback,
					 CongPluginCallbackConfigure configure_callback)
{
	CongPlugin *plugin;

	g_return_val_if_fail(plugin_manager, NULL);
	g_return_val_if_fail(id, NULL);
	g_return_val_if_fail(register_callback, NULL);
	/* the configure callback is allowed to be NULL */

	plugin = g_new0(CongPlugin,1);

	plugin->plugin_id = g_strdup(id);
	plugin->configure_callback = configure_callback;

	/* Add to list of plugins: */
	plugin_manager->list_of_plugin = g_list_append(plugin_manager->list_of_plugin, plugin);

	(*register_callback)(plugin);

	return plugin;
}

#if 0
void cong_plugin_unregister_document_factory(CongPlugin *plugin, 
					     CongServiceDocumentFactory *factory);

void cong_plugin_unregister_importer(CongPlugin *plugin, 
				     CongServiceImporter *importer);

void cong_plugin_unregister_exporter(CongPlugin *plugin, 
				     CongServiceExporter *exporter);
#endif

void cong_plugin_manager_for_each_document_factory(CongPluginManager *plugin_manager, void (*callback)(CongServiceDocumentFactory *factory, gpointer user_data), gpointer user_data)
{
	GList *iter;
	g_return_if_fail(plugin_manager);
	g_return_if_fail(callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_document_factory(iter->data, callback, user_data);
	}
}


void cong_plugin_manager_for_each_importer(CongPluginManager *plugin_manager, void (*callback)(CongServiceImporter *importer, gpointer user_data), gpointer user_data)
{
	GList *iter;
	g_return_if_fail(plugin_manager);
	g_return_if_fail(callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_importer(iter->data, callback, user_data);
	}
}

void cong_plugin_manager_for_each_exporter(CongPluginManager *plugin_manager, void (*callback)(CongServiceExporter *exporter, gpointer user_data), gpointer user_data)
{
	GList *iter;
	g_return_if_fail(plugin_manager);
	g_return_if_fail(callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_exporter(iter->data, callback, user_data);
	}
}


#if ENABLE_PRINTING
void cong_plugin_manager_for_each_print_method(CongPluginManager *plugin_manager, void (*callback)(CongServicePrintMethod *print_method, gpointer user_data), gpointer user_data)
{
	GList *iter;
	g_return_if_fail(plugin_manager);
	g_return_if_fail(callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_print_method(iter->data, callback, user_data);
	}
}
#endif

void cong_plugin_manager_for_each_doc_tool(CongPluginManager *plugin_manager, void (*callback)(CongServiceDocTool *tool, gpointer user_data), gpointer user_data)
{
	GList *iter;
	g_return_if_fail(plugin_manager);
	g_return_if_fail(callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_doc_tool(iter->data, callback, user_data);
	}
}

void cong_plugin_manager_for_each_node_tool(CongPluginManager *plugin_manager, void (*callback)(CongServiceNodeTool *tool, gpointer user_data), gpointer user_data)
{
	GList *iter;
	g_return_if_fail(plugin_manager);
	g_return_if_fail(callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_node_tool(iter->data, callback, user_data);
	}
}

CongServiceNodePropertyDialog *cong_plugin_manager_locate_custom_property_dialog_by_id(CongPluginManager *plugin_manager, const gchar *plugin_id)
{
	GList *plugin_iter;

	g_return_val_if_fail(plugin_manager, NULL);
	g_return_val_if_fail(plugin_id, NULL);

	for (plugin_iter=plugin_manager->list_of_plugin; plugin_iter; plugin_iter = plugin_iter->next) {
		GList *factory_iter;

		for (factory_iter = ((CongPlugin*)plugin_iter->data)->list_of_property_dialog; factory_iter; factory_iter=factory_iter->next) {
			CongServiceNodePropertyDialog* factory = factory_iter->data;
			if (0==strcmp(plugin_id, CONG_SERVICE(factory)->service_id)) {
				return factory;
			}
		}
	}
	
	return NULL;
}

CongServiceEditorNodeFactory*
cong_plugin_manager_locate_editor_node_factory_by_id (CongPluginManager *plugin_manager,
						      const gchar *plugin_id)
{
	GList *plugin_iter;

	g_return_val_if_fail(plugin_manager, NULL);
	g_return_val_if_fail(plugin_id, NULL);

	for (plugin_iter=plugin_manager->list_of_plugin; plugin_iter; plugin_iter = plugin_iter->next) {
		GList *factory_iter;

		for (factory_iter = ((CongPlugin*)plugin_iter->data)->list_of_editor_node_factory; factory_iter; factory_iter=factory_iter->next) {
			CongServiceEditorNodeFactory* factory = factory_iter->data;
			if (0==strcmp(plugin_id, CONG_SERVICE(factory)->service_id)) {
				return factory;
			}
		}
	}
	
	return NULL;
}


/* Implementation of CongPlugin: */
CongServiceDocumentFactory *cong_plugin_register_document_factory(CongPlugin *plugin, 
							   const gchar *name, 
							   const gchar *description,
							   const gchar *id,
							   CongServiceDocumentFactoryPageCreationCallback page_creation_callback,
							   CongServiceDocumentFactoryActionCallback action_callback,
							   const gchar *icon,
							   gpointer user_data)
{
	CongServiceDocumentFactory *factory;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(id, NULL);
	g_return_val_if_fail(page_creation_callback, NULL);
	g_return_val_if_fail(action_callback, NULL);
	/* icon is allowed to be NULL */

	factory = g_new0(CongServiceDocumentFactory,1);

	factory->service.plugin = plugin;
	factory->service.name = g_strdup(name);
	factory->service.description = g_strdup(description);
	factory->service.service_id = g_strdup(id);
	factory->page_creation_callback = page_creation_callback;
	factory->action_callback = action_callback;
	if (icon) {
		factory->icon = g_strdup(icon);
		factory->icon16 = cong_util_load_icon(icon);
	}
	factory->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_document_factory = g_list_append(plugin->list_of_document_factory, factory);

	return factory;
}

CongServiceImporter *cong_plugin_register_importer(CongPlugin *plugin, 
					    const gchar *name, 
					    const gchar *description,
					    const gchar *id,
					    CongServiceImporterMimeFilter mime_filter,
					    CongServiceImporterActionCallback action_callback,
					    gpointer user_data)
{
	CongServiceImporter *importer;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(id, NULL);
	g_return_val_if_fail(mime_filter, NULL);
	g_return_val_if_fail(action_callback, NULL);

        importer = g_new0(CongServiceImporter,1);

	importer->service.plugin = plugin;
	importer->service.name = g_strdup(name);
	importer->service.description = g_strdup(description);
	importer->service.service_id = g_strdup(id);
	importer->mime_filter = mime_filter;
	importer->action_callback = action_callback;
	importer->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_importer = g_list_append(plugin->list_of_importer, importer);

	return importer;
}

CongServiceExporter *cong_plugin_register_exporter(CongPlugin *plugin, 
					    const gchar *name, 
					    const gchar *description,
					    const gchar *id,
					    CongServiceExporterDocumentFilter doc_filter,
					    CongServiceExporterActionCallback action_callback,
					    gpointer user_data)
{
	CongServiceExporter *exporter;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(id, NULL);
	g_return_val_if_fail(doc_filter, NULL);
	g_return_val_if_fail(action_callback, NULL);

        exporter = g_new0(CongServiceExporter,1);

	exporter->service.plugin = plugin;
	exporter->service.name = g_strdup(name);
	exporter->service.description = g_strdup(description);
	exporter->service.service_id = g_strdup(id);
	exporter->doc_filter = doc_filter;
	exporter->action_callback = action_callback;
	exporter->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_exporter = g_list_append(plugin->list_of_exporter, exporter);

	return exporter;
}

#if ENABLE_PRINTING
CongServicePrintMethod *cong_plugin_register_print_method(CongPlugin *plugin, 
					    const gchar *name, 
					    const gchar *description,
					    const gchar *id,
					    CongServicePrintMethodDocumentFilter doc_filter,
					    CongServicePrintMethodActionCallback action_callback,
					    gpointer user_data)
{
	CongServicePrintMethod *print_method;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(id, NULL);
	g_return_val_if_fail(doc_filter, NULL);
	g_return_val_if_fail(action_callback, NULL);

        print_method = g_new0(CongServicePrintMethod,1);

	print_method->service.plugin = plugin;
	print_method->service.name = g_strdup(name);
	print_method->service.description = g_strdup(description);
	print_method->service.service_id = g_strdup(id);
	print_method->doc_filter = doc_filter;
	print_method->action_callback = action_callback;
	print_method->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_print_method = g_list_append(plugin->list_of_print_method, print_method);

	return print_method;
}
#endif

CongServiceEditorNodeFactory *cong_plugin_register_editor_node_factory(CongPlugin *plugin, 
								      const gchar *name, 
								      const gchar *description,
								      const gchar *plugin_id,
								      CongEditorNodeFactoryMethod factory_method,
								      gpointer user_data)
{
	CongServiceEditorNodeFactory *editor_node_factory;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(plugin_id, NULL);
	g_return_val_if_fail(factory_method, NULL);

        editor_node_factory = g_new0(CongServiceEditorNodeFactory,1);

	editor_node_factory->service.plugin = plugin;
	editor_node_factory->service.name = g_strdup(name);
	editor_node_factory->service.description = g_strdup(description);
	editor_node_factory->service.service_id = g_strdup(plugin_id);
	editor_node_factory->make_node = factory_method;
	editor_node_factory->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_editor_node_factory = g_list_append(plugin->list_of_editor_node_factory, editor_node_factory);

	return editor_node_factory;
}

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
			       gpointer user_data)
{
	CongServiceDocTool *tool;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(service_id, NULL);

        tool = g_new0(CongServiceDocTool,1);

	tool->tool.service.plugin = plugin;
	tool->tool.service.name = g_strdup(name);
	tool->tool.service.description = g_strdup(description);
	tool->tool.service.service_id = g_strdup(service_id);
	tool->tool.menu_text = g_strdup(menu_text);
	tool->tool.tooltip_text = g_strdup(tooltip_text);
	tool->tool.tooltip_further_text = g_strdup(tooltip_further_text);
	tool->tool.user_data = user_data;
	tool->doc_filter = doc_filter;
	tool->action_callback = action_callback;

	/* Add to plugin's list: */
	plugin->list_of_doc_tool = g_list_append(plugin->list_of_doc_tool, tool);

	return tool;

}

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
				gpointer user_data)
{
	CongServiceNodeTool *tool;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(service_id, NULL);

        tool = g_new0(CongServiceNodeTool,1);

	tool->tool.service.plugin = plugin;
	tool->tool.service.name = g_strdup(name);
	tool->tool.service.description = g_strdup(description);
	tool->tool.service.service_id = g_strdup(service_id);
	tool->tool.menu_text = g_strdup(menu_text);
	tool->tool.tooltip_text = g_strdup(tooltip_text);
	tool->tool.tooltip_further_text = g_strdup(tooltip_further_text);
	tool->tool.user_data = user_data;
	tool->node_filter = node_filter;
	tool->action_callback = action_callback;

	/* Add to plugin's list: */
	plugin->list_of_node_tool = g_list_append(plugin->list_of_node_tool, tool);

	return tool;

}

CongServiceNodePropertyDialog*
cong_plugin_register_custom_property_dialog (CongPlugin *plugin,
					     const gchar *name, 
					     const gchar *description,
					     const gchar *service_id,
					     CongCustomPropertyFactoryMethod factory_method,
					     gpointer user_data)
{
	CongServiceNodePropertyDialog *property_dialog;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(service_id, NULL);

        property_dialog = g_new0(CongServiceNodePropertyDialog,1);

	property_dialog->service.plugin = plugin;
	property_dialog->service.name = g_strdup(name);
	property_dialog->service.description = g_strdup(description);
	property_dialog->service.service_id = g_strdup(service_id);
	property_dialog->factory_method = factory_method;
	property_dialog->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_property_dialog = g_list_append(plugin->list_of_property_dialog, property_dialog);

	return property_dialog;

}

CongServiceNodePropertyDialog*
cong_plugin_register_custom_property_dialog_for_element (CongPlugin *plugin,
							 const gchar *element_name,
							 const gchar *service_id,
							 CongCustomPropertyFactoryMethod factory_method,
							 gpointer user_data)
{
	CongServiceNodePropertyDialog *property_dialog;
	gchar *name;
	gchar *description;	

	g_return_val_if_fail (plugin, NULL);
	g_return_val_if_fail (element_name, NULL);
	g_return_val_if_fail (service_id, NULL);

	/* Generate a user-visible name for this plugin property dialog (property dialog for an XML element)*/
	name = g_strdup_printf (_("<%s> property dialog"), element_name);

	/* Generate a user-visible description for this plugin (property dialog for an XML element)*/
	description = g_strdup_printf (_("Provides a Properties dialog for the <%s> element"), element_name);

	property_dialog = cong_plugin_register_custom_property_dialog (plugin,
								       name, 
								       description,
								       service_id,
								       factory_method,
								       user_data);
	g_free (name);
	g_free (description);
	
	return property_dialog;
}

void cong_plugin_for_each_document_factory(CongPlugin *plugin, void (*callback)(CongServiceDocumentFactory *factory, gpointer user_data), gpointer user_data)
{
	g_return_if_fail(plugin);
	g_return_if_fail(callback);

	g_list_foreach(plugin->list_of_document_factory, (GFunc)callback, user_data);
}

void cong_plugin_for_each_importer(CongPlugin *plugin, void (*callback)(CongServiceImporter *importer, gpointer user_data), gpointer user_data)
{
	g_return_if_fail(plugin);
	g_return_if_fail(callback);

	g_list_foreach(plugin->list_of_importer, (GFunc)callback, user_data);
}

void cong_plugin_for_each_exporter(CongPlugin *plugin, void (*callback)(CongServiceExporter *exporter, gpointer user_data), gpointer user_data)
{
	g_return_if_fail(plugin);
	g_return_if_fail(callback);

	g_list_foreach(plugin->list_of_exporter, (GFunc)callback, user_data);
}

#if ENABLE_PRINTING
void cong_plugin_for_each_print_method(CongPlugin *plugin, void (*callback)(CongServicePrintMethod *print_method, gpointer user_data), gpointer user_data)
{
	g_return_if_fail(plugin);
	g_return_if_fail(callback);

	g_list_foreach(plugin->list_of_print_method, (GFunc)callback, user_data);
}
#endif

void cong_plugin_for_each_doc_tool(CongPlugin *plugin, void (*callback)(CongServiceDocTool *doc_tool, gpointer user_data), gpointer user_data)
{
	g_return_if_fail(plugin);
	g_return_if_fail(callback);

	g_list_foreach(plugin->list_of_doc_tool, (GFunc)callback, user_data);
}

void cong_plugin_for_each_node_tool(CongPlugin *plugin, void (*callback)(CongServiceNodeTool *node_tool, gpointer user_data), gpointer user_data)
{
	g_return_if_fail(plugin);
	g_return_if_fail(callback);

	g_list_foreach(plugin->list_of_node_tool, (GFunc)callback, user_data);
}

gchar* cong_plugin_get_gconf_namespace(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, NULL);

	g_assert(plugin->plugin_id);

	return g_strdup_printf( (CONG_GCONF_PATH "plugins/%s"), plugin->plugin_id);
}

gchar* cong_plugin_get_gconf_key(CongPlugin *plugin, const gchar *local_part)
{
	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(local_part, NULL);

	g_assert(plugin->plugin_id);

	return g_strdup_printf( (CONG_GCONF_PATH "plugins/%s/%s"), plugin->plugin_id, local_part);
}

/* Implementation of CongService: */
const gchar* cong_service_get_name(CongService *service)
{
	g_return_val_if_fail(service, NULL);
	
	return service->name;
}

const gchar* cong_service_get_description(CongService *service)
{
	g_return_val_if_fail(service, NULL);

	return service->description;
}

gchar* cong_service_get_gconf_namespace(CongService* service)
{
	gchar *plugin_namespace;
	gchar *result;

	g_return_val_if_fail(service, NULL);

	g_assert(service->plugin);
	g_assert(service->service_id);

	plugin_namespace = cong_plugin_get_gconf_namespace(service->plugin);

	result = g_strdup_printf("%s/%s", plugin_namespace, service->service_id);

	g_free(plugin_namespace);

	return result;
}

gchar* cong_service_get_gconf_key(CongService *service, const gchar *local_part)
{
	gchar *scoped_local_part;
	gchar *service_path;

	g_return_val_if_fail(service, NULL);
	g_return_val_if_fail(local_part, NULL);

	g_assert(service->plugin);
	g_assert(service->service_id);

	scoped_local_part =  g_strdup_printf("%s/%s", service->service_id, local_part);

	service_path = cong_plugin_get_gconf_key(service->plugin, scoped_local_part);

	g_free(scoped_local_part);

	return service_path;
}

/* Implementation of CongServiceDocumentFactory: */
void cong_document_factory_invoke_page_creation_callback(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant)
{
	g_return_if_fail(factory);
	g_return_if_fail(assistant);

#if 0
	g_message("page creation for document factory \"%s\"", cong_service_get_name(CONG_SERVICE(factory)));
#endif

	g_assert(factory->page_creation_callback);

	factory->page_creation_callback(factory, assistant, factory->user_data);
}

void cong_document_factory_invoke_action_callback(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant)
{
	g_return_if_fail(factory);
	g_return_if_fail(assistant);

#if 0
	g_message("invoking action for document factory \"%s\"", cong_service_get_name(CONG_SERVICE(factory)));
#endif

	g_assert(factory->action_callback);

	factory->action_callback(factory, assistant, factory->user_data);
}

GdkPixbuf *cong_document_factory_get_icon(CongServiceDocumentFactory *factory)
{
	g_return_val_if_fail(factory, NULL);

	if (factory->icon16) {
		g_object_ref(G_OBJECT(factory->icon16));
		return factory->icon16;
	} else {
		return NULL;
	}
}

/* Implementation of CongServiceImporter: */
gboolean cong_importer_supports_mime_type(CongServiceImporter *importer, const gchar *mime_type)
{
	g_return_val_if_fail(importer, FALSE);
	g_return_val_if_fail(mime_type, FALSE);

	g_assert(importer->mime_filter);

	return importer->mime_filter(importer, mime_type, importer->user_data);

}

void cong_importer_invoke(CongServiceImporter *importer, const gchar *filename, const gchar *mime_type, GtkWindow *toplevel_window)
{
	g_return_if_fail(importer);
	g_return_if_fail(filename);
	g_return_if_fail(mime_type);
	
	g_assert(importer->action_callback);

	return importer->action_callback(importer, filename, mime_type, importer->user_data, toplevel_window);
}

/* Implementation of CongServiceExporter: */
gboolean cong_exporter_supports_document(CongServiceExporter *exporter, CongDocument *doc)
{
	g_return_val_if_fail(exporter, FALSE);
	g_return_val_if_fail(doc, FALSE);

	g_assert(exporter->doc_filter);

	return exporter->doc_filter(exporter, doc, exporter->user_data);
}

void cong_exporter_invoke(CongServiceExporter *exporter, CongDocument *doc, const gchar *uri, GtkWindow *toplevel_window)
{
	g_return_if_fail(exporter);
	g_return_if_fail(doc);
	g_return_if_fail(uri);
	
	g_assert(exporter->action_callback);

	return exporter->action_callback(exporter, doc, uri, exporter->user_data, toplevel_window);
}

gchar *cong_exporter_get_preferred_uri(CongServiceExporter *exporter)
{
	gchar *gconf_key;
	gchar *preferred_uri;

	g_return_val_if_fail(exporter, NULL);

	gconf_key = cong_service_get_gconf_key(CONG_SERVICE(exporter), "preferred-uri");
	
	preferred_uri = gconf_client_get_string( cong_app_get_gconf_client (cong_app_singleton()),
						 gconf_key,
						 NULL);

	g_free(gconf_key);

	return preferred_uri;
}

void cong_exporter_set_preferred_uri(CongServiceExporter *exporter, const gchar *uri)
{
	gchar *gconf_key;

	g_return_if_fail(exporter);
	g_return_if_fail(uri);

	gconf_key = cong_service_get_gconf_key(CONG_SERVICE(exporter), "preferred-uri");

	gconf_client_set_string( cong_app_get_gconf_client (cong_app_singleton()),
				 gconf_key,
				 uri,
				NULL);

	g_free(gconf_key);
}

#if ENABLE_PRINTING
/* Implementation of CongServicePrintMethod: */
gboolean cong_print_method_supports_document(CongServicePrintMethod *print_method, CongDocument *doc)
{
	g_return_val_if_fail(print_method, FALSE);
	g_return_val_if_fail(doc, FALSE);

	g_assert(print_method->doc_filter);

	return print_method->doc_filter(print_method, doc, print_method->user_data);
}

void cong_print_method_invoke(CongServicePrintMethod *print_method, CongDocument *doc, GnomePrintContext *gpc, GtkWindow *toplevel_window)
{
	g_return_if_fail(print_method);
	g_return_if_fail(doc);
	g_return_if_fail(gpc);
	
	g_assert(print_method->action_callback);

	return print_method->action_callback(print_method, doc, gpc, print_method->user_data, toplevel_window);
}
#endif

gboolean 
cong_doc_tool_supports_document (CongServiceDocTool *tool, 
				 CongDocument *doc)
{
	g_return_val_if_fail(tool, FALSE);
	g_return_val_if_fail(doc, FALSE);

	g_assert(tool->doc_filter);
	return tool->doc_filter(tool, doc, tool->tool.user_data);
}

void 
cong_doc_tool_invoke (CongServiceDocTool *tool, 
		      CongPrimaryWindow *primary_window)
{
	g_return_if_fail(tool);

	g_assert(tool->action_callback);
	return tool->action_callback(tool, primary_window, tool->tool.user_data);
}

gboolean 
cong_node_tool_supports_node (CongServiceNodeTool *tool, 
			      CongDocument *doc,
			      CongNodePtr node)
{
	g_return_val_if_fail(tool, FALSE);
	g_return_val_if_fail(doc, FALSE);
	g_return_val_if_fail(node, FALSE);

	g_assert(tool->node_filter);
	return tool->node_filter(tool, doc, node, tool->tool.user_data);
}

void 
cong_node_tool_invoke (CongServiceNodeTool *tool, 
		       CongPrimaryWindow *primary_window,
		       CongNodePtr node)
{
	g_return_if_fail(tool);
	g_return_if_fail(node);

	g_assert(tool->action_callback);
	return tool->action_callback(tool, primary_window, node, tool->tool.user_data);
}

const gchar *cong_tool_get_menu_text(CongServiceTool *tool)
{
	g_return_val_if_fail(tool, NULL);

	return tool->menu_text;
}

const gchar *cong_tool_get_tip_text(CongServiceTool *tool)
{
	g_return_val_if_fail(tool, NULL);

	return tool->tooltip_text;
}

const gchar *cong_tool_get_tip_further_text(CongServiceTool *tool)
{
	g_return_val_if_fail(tool, NULL);

	return tool->tooltip_further_text;
}

CongDocument*
cong_ui_new_document_from_manufactured_xml(xmlDocPtr xml_doc,
					   GtkWindow *parent_window)
{
	CongDocument *cong_doc;
	CongDispspec *ds;
	
	g_return_val_if_fail (xml_doc, NULL);

	ds = cong_dispspec_registry_get_appropriate_dispspec (cong_app_get_dispspec_registry (cong_app_singleton()), 
							      xml_doc,
							      NULL);

	if (ds == NULL) {
		ds = query_for_forced_dispspec (_("Conglomerate could not find display information for the new file"), 
						xml_doc, 
						parent_window,
						NULL);

		if (NULL == ds) {
			xmlFreeDoc(xml_doc);
			return NULL;
		}
	}
	
	g_assert(xml_doc);
	g_assert(ds);

	cong_doc = cong_document_new_from_xmldoc(xml_doc, ds, NULL);

	cong_primary_window_new(cong_doc);
	g_object_unref(G_OBJECT(cong_doc));

	return cong_doc;
}

CongDocument*
cong_ui_new_document_from_imported_xml(xmlDocPtr xml_doc,
				       GtkWindow *parent_window)
{
	CongDocument *cong_doc;
	CongDispspec *ds;
	
	g_return_val_if_fail (xml_doc, NULL);

	ds = cong_dispspec_registry_get_appropriate_dispspec (cong_app_get_dispspec_registry (cong_app_singleton()), 
							      xml_doc,
							      NULL);

	if (ds == NULL) {
		ds = query_for_forced_dispspec (_("Conglomerate could not find display information for the new file"), 
						xml_doc, 
						parent_window,
						NULL);

		if (NULL == ds) {
			xmlFreeDoc(xml_doc);
			return NULL;
		}
	}
	
	g_assert(xml_doc);
	g_assert(ds);

	cong_doc = cong_document_new_from_xmldoc(xml_doc, ds, NULL);

	cong_primary_window_new(cong_doc);
	g_object_unref(G_OBJECT(cong_doc));

	return cong_doc;
}

xmlDocPtr cong_ui_transform_doc(CongDocument *doc,
				const gchar *stylesheet_filename,
				GtkWindow *toplevel_window)
{
	xsltStylesheetPtr xsl;
	xmlDocPtr input_clone;
	xmlDocPtr result;

	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(stylesheet_filename, NULL);

	xsl = xsltParseStylesheetFile(stylesheet_filename);

	if (NULL==xsl) {
		gchar *why_failed = g_strdup_printf(_("There was a problem reading the stylesheet file \"%s\""),stylesheet_filename);

		GtkDialog* dialog = cong_error_dialog_new(toplevel_window,
							  _("Conglomerate could not transform the document"),
							  why_failed,
							  "FIXME");
	
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		return NULL;
	}

	/* DHM 14/11/2002:  document nodes seemed to being corrupted when applying the stylesheet.
	   So We now work with a clone of the document
	*/
	input_clone = xmlCopyDoc(cong_document_get_xml(doc), TRUE);
	g_assert(input_clone);

	result = xsltApplyStylesheet(xsl, input_clone, NULL);
	g_assert(result);

	xmlFreeDoc(input_clone);

	if (result->children==NULL) {
		gchar *why_failed = g_strdup_printf(_("There was a problem applying the stylesheet file"));

		GtkDialog* dialog = cong_error_dialog_new(toplevel_window,
							  _("Conglomerate could not transform the document"),
							  why_failed,
							  "FIXME");
	
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		return NULL;
	}

	xsltFreeStylesheet(xsl);

	return result;
}

void cong_ui_transform_doc_to_uri(CongDocument *doc,
				  const gchar *stylesheet_filename,
				  const gchar *string_uri,
				  GtkWindow *toplevel_window)
{
	xmlDocPtr doc_ptr;
	GnomeVFSURI *vfs_uri;
	GnomeVFSResult vfs_result;
	GnomeVFSFileSize file_size;

	g_return_if_fail(doc);
	g_return_if_fail(stylesheet_filename);
	g_return_if_fail(string_uri);

	/* FIXME:  need some kind of feedback e.g. a busy cursor */

	doc_ptr = cong_ui_transform_doc(doc,
					stylesheet_filename,
					toplevel_window);

	if (doc_ptr) {
		vfs_uri = gnome_vfs_uri_new(string_uri);
	
		vfs_result = cong_vfs_save_xml_to_uri (doc_ptr, 
						       vfs_uri,	
						       &file_size);
		
		if (vfs_result != GNOME_VFS_OK) {
			GtkDialog* dialog = cong_error_dialog_new_from_file_save_failure(toplevel_window,
											 string_uri, 
											 vfs_result, 
											 &file_size);
			
			cong_error_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(GTK_WIDGET(dialog));
		}
		
		gnome_vfs_uri_unref(vfs_uri);

		xmlFreeDoc(doc_ptr);
	}
}

/* Handy methods for "Import" methods; doing the necessary UI hooks: */
gboolean cong_ui_load_imported_file_content(const gchar *string_uri,
					    char** buffer,
					    GnomeVFSFileSize* size,
					    GtkWindow *parent_window)
{
	GnomeVFSResult vfs_result;

	g_return_val_if_fail(string_uri, FALSE);
	g_return_val_if_fail(buffer, FALSE);
	g_return_val_if_fail(size, FALSE);

	vfs_result = cong_vfs_new_buffer_from_file(string_uri, buffer, size);
	
	if (vfs_result!=GNOME_VFS_OK) {
		GtkDialog* dialog = cong_error_dialog_new_from_file_open_failure_with_vfs_result (parent_window,
												  string_uri, 
												  vfs_result);
		
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		
		return FALSE;
	}
	
	g_assert(*buffer);

	return TRUE;
}

CongEditorNodeElement*
cong_plugin_editor_node_factory_invoke (CongServiceEditorNodeFactory *plugin_editor_node_factory,
					CongEditorWidget3 *editor_widget, 
					CongTraversalNode *traversal_node)
{
	g_return_val_if_fail(plugin_editor_node_factory, NULL);
	g_return_val_if_fail(editor_widget, NULL);
	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE (traversal_node), NULL);

	g_assert(plugin_editor_node_factory->make_node);

	return plugin_editor_node_factory->make_node (plugin_editor_node_factory, 
						      editor_widget, 
						      traversal_node,
						      plugin_editor_node_factory->user_data);
}

GtkWidget *cong_custom_property_dialog_make(CongServiceNodePropertyDialog *custom_property_dialog,
					    CongDocument *doc,
					    CongNodePtr node)
{
	g_return_val_if_fail(custom_property_dialog, NULL);
	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);

	g_assert(custom_property_dialog->factory_method);

	return custom_property_dialog->factory_method(custom_property_dialog, doc, node);
}
