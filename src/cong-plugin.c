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

	GList *list_of_document_factory; /* ptrs of type CongDocumentFactory */
	GList *list_of_importer; /* ptrs of type CongImporter */
	GList *list_of_exporter; /* ptrs of type CongExporter */
#if ENABLE_PRINTING
	GList *list_of_print_method; /* ptrs of type CongPrintMethod */
#endif
	GList *list_of_thumbnailer; /* ptrs of type CongThumnbailer */
	GList *list_of_editor_element; /* ptrs of type CongPluginEditorElement */
	GList *list_of_editor_node_factory; /* ptrs of type CongPluginEditorNodeFactory */
	GList *list_of_tool; /* ptrs of type CongTool */
	GList *list_of_property_dialog; /* ptrs of type CongCustomPropertyDialog */
};

struct CongFunctionality
{
	CongPlugin *plugin;
	gchar *name;
	gchar *description;
	gchar *functionality_id;
};

struct CongDocumentFactory
{
	CongFunctionality functionality; /* base class */

	CongDocumentFactoryPageCreationCallback page_creation_callback;
	CongDocumentFactoryActionCallback action_callback;

	gchar *icon;
	GdkPixbuf *icon16;

	gpointer user_data;
};

struct CongImporter
{
	CongFunctionality functionality; /* base class */

	CongImporterMimeFilter mime_filter;
	CongImporterActionCallback action_callback;
	gpointer user_data;
};

struct CongExporter
{
	CongFunctionality functionality; /* base class */

	CongExporterDocumentFilter doc_filter;
	CongExporterActionCallback action_callback;
	gpointer user_data;
};

#if ENABLE_PRINTING
struct CongPrintMethod
{
	CongFunctionality functionality; /* base class */

	CongPrintMethodDocumentFilter doc_filter;
	CongPrintMethodActionCallback action_callback;
	gpointer user_data;
};
#endif

struct CongThumbnailer
{
	CongFunctionality functionality; /* base class */
};

struct CongPluginEditorElement
{
	CongFunctionality functionality; /* base class */
	CongEditorElementFactoryMethod make_element;
	gpointer user_data;
};

struct CongPluginEditorNodeFactory
{
	CongFunctionality functionality; /* base class */
	CongEditorNodeFactoryMethod make_node;
	gpointer user_data;
};

struct CongTool
{
	CongFunctionality functionality; /* base class */
	const gchar *menu_text;
	const gchar *tooltip_text;
	const gchar *tooltip_further_text;
	CongToolDocumentFilter doc_filter;
	CongToolActionCallback action_callback;
	gpointer user_data;
};

struct CongCustomPropertyDialog
{
	CongFunctionality functionality; /* base class */
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
					     CongDocumentFactory *factory);

void cong_plugin_unregister_importer(CongPlugin *plugin, 
				     CongImporter *importer);

void cong_plugin_unregister_exporter(CongPlugin *plugin, 
				     CongExporter *exporter);
#endif

void cong_plugin_manager_for_each_document_factory(CongPluginManager *plugin_manager, void (*callback)(CongDocumentFactory *factory, gpointer user_data), gpointer user_data)
{
	GList *iter;
	g_return_if_fail(plugin_manager);
	g_return_if_fail(callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_document_factory(iter->data, callback, user_data);
	}
}


void cong_plugin_manager_for_each_importer(CongPluginManager *plugin_manager, void (*callback)(CongImporter *importer, gpointer user_data), gpointer user_data)
{
	GList *iter;
	g_return_if_fail(plugin_manager);
	g_return_if_fail(callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_importer(iter->data, callback, user_data);
	}
}

void cong_plugin_manager_for_each_exporter(CongPluginManager *plugin_manager, void (*callback)(CongExporter *exporter, gpointer user_data), gpointer user_data)
{
	GList *iter;
	g_return_if_fail(plugin_manager);
	g_return_if_fail(callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_exporter(iter->data, callback, user_data);
	}
}


#if ENABLE_PRINTING
void cong_plugin_manager_for_each_print_method(CongPluginManager *plugin_manager, void (*callback)(CongPrintMethod *print_method, gpointer user_data), gpointer user_data)
{
	GList *iter;
	g_return_if_fail(plugin_manager);
	g_return_if_fail(callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_print_method(iter->data, callback, user_data);
	}
}
#endif

void cong_plugin_manager_for_each_tool(CongPluginManager *plugin_manager, void (*callback)(CongTool *tool, gpointer user_data), gpointer user_data)
{
	GList *iter;
	g_return_if_fail(plugin_manager);
	g_return_if_fail(callback);

	for (iter=plugin_manager->list_of_plugin; iter; iter = iter->next) {
		cong_plugin_for_each_tool(iter->data, callback, user_data);
	}
}

CongCustomPropertyDialog *cong_plugin_manager_locate_custom_property_dialog_by_id(CongPluginManager *plugin_manager, const gchar *plugin_id)
{
	GList *plugin_iter;

	g_return_val_if_fail(plugin_manager, NULL);
	g_return_val_if_fail(plugin_id, NULL);

	for (plugin_iter=plugin_manager->list_of_plugin; plugin_iter; plugin_iter = plugin_iter->next) {
		GList *factory_iter;

		for (factory_iter = ((CongPlugin*)plugin_iter->data)->list_of_property_dialog; factory_iter; factory_iter=factory_iter->next) {
			CongCustomPropertyDialog* factory = factory_iter->data;
			if (0==strcmp(plugin_id, CONG_FUNCTIONALITY(factory)->functionality_id)) {
				return factory;
			}
		}
	}
	
	return NULL;
}

CongPluginEditorNodeFactory*
cong_plugin_manager_locate_editor_node_factory_by_id (CongPluginManager *plugin_manager,
						      const gchar *plugin_id)
{
	GList *plugin_iter;

	g_return_val_if_fail(plugin_manager, NULL);
	g_return_val_if_fail(plugin_id, NULL);

	for (plugin_iter=plugin_manager->list_of_plugin; plugin_iter; plugin_iter = plugin_iter->next) {
		GList *factory_iter;

		for (factory_iter = ((CongPlugin*)plugin_iter->data)->list_of_editor_node_factory; factory_iter; factory_iter=factory_iter->next) {
			CongPluginEditorNodeFactory* factory = factory_iter->data;
			if (0==strcmp(plugin_id, CONG_FUNCTIONALITY(factory)->functionality_id)) {
				return factory;
			}
		}
	}
	
	return NULL;
}


/* Implementation of CongPlugin: */
CongDocumentFactory *cong_plugin_register_document_factory(CongPlugin *plugin, 
							   const gchar *name, 
							   const gchar *description,
							   const gchar *id,
							   CongDocumentFactoryPageCreationCallback page_creation_callback,
							   CongDocumentFactoryActionCallback action_callback,
							   const gchar *icon,
							   gpointer user_data)
{
	CongDocumentFactory *factory;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(id, NULL);
	g_return_val_if_fail(page_creation_callback, NULL);
	g_return_val_if_fail(action_callback, NULL);
	/* icon is allowed to be NULL */

	factory = g_new0(CongDocumentFactory,1);

	factory->functionality.plugin = plugin;
	factory->functionality.name = g_strdup(name);
	factory->functionality.description = g_strdup(description);
	factory->functionality.functionality_id = g_strdup(id);
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

CongImporter *cong_plugin_register_importer(CongPlugin *plugin, 
					    const gchar *name, 
					    const gchar *description,
					    const gchar *id,
					    CongImporterMimeFilter mime_filter,
					    CongImporterActionCallback action_callback,
					    gpointer user_data)
{
	CongImporter *importer;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(id, NULL);
	g_return_val_if_fail(mime_filter, NULL);
	g_return_val_if_fail(action_callback, NULL);

        importer = g_new0(CongImporter,1);

	importer->functionality.plugin = plugin;
	importer->functionality.name = g_strdup(name);
	importer->functionality.description = g_strdup(description);
	importer->functionality.functionality_id = g_strdup(id);
	importer->mime_filter = mime_filter;
	importer->action_callback = action_callback;
	importer->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_importer = g_list_append(plugin->list_of_importer, importer);

	return importer;
}

CongExporter *cong_plugin_register_exporter(CongPlugin *plugin, 
					    const gchar *name, 
					    const gchar *description,
					    const gchar *id,
					    CongExporterDocumentFilter doc_filter,
					    CongExporterActionCallback action_callback,
					    gpointer user_data)
{
	CongExporter *exporter;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(id, NULL);
	g_return_val_if_fail(doc_filter, NULL);
	g_return_val_if_fail(action_callback, NULL);

        exporter = g_new0(CongExporter,1);

	exporter->functionality.plugin = plugin;
	exporter->functionality.name = g_strdup(name);
	exporter->functionality.description = g_strdup(description);
	exporter->functionality.functionality_id = g_strdup(id);
	exporter->doc_filter = doc_filter;
	exporter->action_callback = action_callback;
	exporter->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_exporter = g_list_append(plugin->list_of_exporter, exporter);

	return exporter;
}

#if ENABLE_PRINTING
CongPrintMethod *cong_plugin_register_print_method(CongPlugin *plugin, 
					    const gchar *name, 
					    const gchar *description,
					    const gchar *id,
					    CongPrintMethodDocumentFilter doc_filter,
					    CongPrintMethodActionCallback action_callback,
					    gpointer user_data)
{
	CongPrintMethod *print_method;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(id, NULL);
	g_return_val_if_fail(doc_filter, NULL);
	g_return_val_if_fail(action_callback, NULL);

        print_method = g_new0(CongPrintMethod,1);

	print_method->functionality.plugin = plugin;
	print_method->functionality.name = g_strdup(name);
	print_method->functionality.description = g_strdup(description);
	print_method->functionality.functionality_id = g_strdup(id);
	print_method->doc_filter = doc_filter;
	print_method->action_callback = action_callback;
	print_method->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_print_method = g_list_append(plugin->list_of_print_method, print_method);

	return print_method;
}
#endif

CongPluginEditorElement *cong_plugin_register_editor_element(CongPlugin *plugin, 
							     const gchar *name, 
							     const gchar *description,
							     const gchar *id,
							     CongEditorElementFactoryMethod factory_method,
							     gpointer user_data)
{
	CongPluginEditorElement *editor_element_factory;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(id, NULL);

        editor_element_factory = g_new0(CongPluginEditorElement,1);

	editor_element_factory->functionality.plugin = plugin;
	editor_element_factory->functionality.name = g_strdup(name);
	editor_element_factory->functionality.description = g_strdup(description);
	editor_element_factory->functionality.functionality_id = g_strdup(id);
	editor_element_factory->make_element = factory_method;
	editor_element_factory->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_editor_element = g_list_append(plugin->list_of_editor_element, editor_element_factory);

	return editor_element_factory;
}

CongPluginEditorNodeFactory *cong_plugin_register_editor_node_factory(CongPlugin *plugin, 
								      const gchar *name, 
								      const gchar *description,
								      const gchar *plugin_id,
								      CongEditorNodeFactoryMethod factory_method,
								      gpointer user_data)
{
	CongPluginEditorNodeFactory *editor_node_factory;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(plugin_id, NULL);
	g_return_val_if_fail(factory_method, NULL);

        editor_node_factory = g_new0(CongPluginEditorNodeFactory,1);

	editor_node_factory->functionality.plugin = plugin;
	editor_node_factory->functionality.name = g_strdup(name);
	editor_node_factory->functionality.description = g_strdup(description);
	editor_node_factory->functionality.functionality_id = g_strdup(plugin_id);
	editor_node_factory->make_node = factory_method;
	editor_node_factory->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_editor_node_factory = g_list_append(plugin->list_of_editor_node_factory, editor_node_factory);

	return editor_node_factory;
}

CongTool *cong_plugin_register_tool(CongPlugin *plugin,
				    const gchar *name, 
				    const gchar *description,
				    const gchar *functionality_id,
				    const gchar *menu_text,
				    const gchar *tooltip_text,
				    const gchar *tooltip_further_text,
				    CongToolDocumentFilter doc_filter,
				    CongToolActionCallback action_callback,
				    gpointer user_data)
{
	CongTool *tool;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(functionality_id, NULL);

        tool = g_new0(CongTool,1);

	tool->functionality.plugin = plugin;
	tool->functionality.name = g_strdup(name);
	tool->functionality.description = g_strdup(description);
	tool->functionality.functionality_id = g_strdup(functionality_id);
	tool->menu_text = g_strdup(menu_text);
	tool->tooltip_text = g_strdup(tooltip_text);
	tool->tooltip_further_text = g_strdup(tooltip_further_text);
	tool->doc_filter = doc_filter;
	tool->action_callback = action_callback;
	tool->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_tool = g_list_append(plugin->list_of_tool, tool);

	return tool;

}

CongCustomPropertyDialog *cong_plugin_register_custom_property_dialog(CongPlugin *plugin,
								      const gchar *name, 
								      const gchar *description,
								      const gchar *functionality_id,
								      CongCustomPropertyFactoryMethod factory_method,
								      gpointer user_data)
{
	CongCustomPropertyDialog *property_dialog;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(functionality_id, NULL);

        property_dialog = g_new0(CongCustomPropertyDialog,1);

	property_dialog->functionality.plugin = plugin;
	property_dialog->functionality.name = g_strdup(name);
	property_dialog->functionality.description = g_strdup(description);
	property_dialog->functionality.functionality_id = g_strdup(functionality_id);
	property_dialog->factory_method = factory_method;
	property_dialog->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_property_dialog = g_list_append(plugin->list_of_property_dialog, property_dialog);

	return property_dialog;

}

void cong_plugin_for_each_document_factory(CongPlugin *plugin, void (*callback)(CongDocumentFactory *factory, gpointer user_data), gpointer user_data)
{
	g_return_if_fail(plugin);
	g_return_if_fail(callback);

	g_list_foreach(plugin->list_of_document_factory, (GFunc)callback, user_data);
}

void cong_plugin_for_each_importer(CongPlugin *plugin, void (*callback)(CongImporter *importer, gpointer user_data), gpointer user_data)
{
	g_return_if_fail(plugin);
	g_return_if_fail(callback);

	g_list_foreach(plugin->list_of_importer, (GFunc)callback, user_data);
}

void cong_plugin_for_each_exporter(CongPlugin *plugin, void (*callback)(CongExporter *exporter, gpointer user_data), gpointer user_data)
{
	g_return_if_fail(plugin);
	g_return_if_fail(callback);

	g_list_foreach(plugin->list_of_exporter, (GFunc)callback, user_data);
}

#if ENABLE_PRINTING
void cong_plugin_for_each_print_method(CongPlugin *plugin, void (*callback)(CongPrintMethod *print_method, gpointer user_data), gpointer user_data)
{
	g_return_if_fail(plugin);
	g_return_if_fail(callback);

	g_list_foreach(plugin->list_of_print_method, (GFunc)callback, user_data);
}
#endif

void cong_plugin_for_each_tool(CongPlugin *plugin, void (*callback)(CongTool *tool, gpointer user_data), gpointer user_data)
{
	g_return_if_fail(plugin);
	g_return_if_fail(callback);

	g_list_foreach(plugin->list_of_tool, (GFunc)callback, user_data);
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

/* Implementation of CongFunctionality: */
const gchar* cong_functionality_get_name(CongFunctionality *functionality)
{
	g_return_val_if_fail(functionality, NULL);
	
	return functionality->name;
}

const gchar* cong_functionality_get_description(CongFunctionality *functionality)
{
	g_return_val_if_fail(functionality, NULL);

	return functionality->description;
}

gchar* cong_functionality_get_gconf_namespace(CongFunctionality* functionality)
{
	gchar *plugin_namespace;
	gchar *result;

	g_return_val_if_fail(functionality, NULL);

	g_assert(functionality->plugin);
	g_assert(functionality->functionality_id);

	plugin_namespace = cong_plugin_get_gconf_namespace(functionality->plugin);

	result = g_strdup_printf("%s/%s", plugin_namespace, functionality->functionality_id);

	g_free(plugin_namespace);

	return result;
}

gchar* cong_functionality_get_gconf_key(CongFunctionality *functionality, const gchar *local_part)
{
	gchar *scoped_local_part;
	gchar *functionality_path;

	g_return_val_if_fail(functionality, NULL);
	g_return_val_if_fail(local_part, NULL);

	g_assert(functionality->plugin);
	g_assert(functionality->functionality_id);

	scoped_local_part =  g_strdup_printf("%s/%s", functionality->functionality_id, local_part);

	functionality_path = cong_plugin_get_gconf_key(functionality->plugin, scoped_local_part);

	g_free(scoped_local_part);

	return functionality_path;
}

/* Implementation of CongDocumentFactory: */
void cong_document_factory_invoke_page_creation_callback(CongDocumentFactory *factory, CongNewFileAssistant *assistant)
{
	g_return_if_fail(factory);
	g_return_if_fail(assistant);

#if 0
	g_message("page creation for document factory \"%s\"", cong_functionality_get_name(CONG_FUNCTIONALITY(factory)));
#endif

	g_assert(factory->page_creation_callback);

	factory->page_creation_callback(factory, assistant, factory->user_data);
}

void cong_document_factory_invoke_action_callback(CongDocumentFactory *factory, CongNewFileAssistant *assistant)
{
	g_return_if_fail(factory);
	g_return_if_fail(assistant);

#if 0
	g_message("invoking action for document factory \"%s\"", cong_functionality_get_name(CONG_FUNCTIONALITY(factory)));
#endif

	g_assert(factory->action_callback);

	factory->action_callback(factory, assistant, factory->user_data);
}

GdkPixbuf *cong_document_factory_get_icon(CongDocumentFactory *factory)
{
	g_return_val_if_fail(factory, NULL);

	if (factory->icon16) {
		g_object_ref(G_OBJECT(factory->icon16));
		return factory->icon16;
	} else {
		return NULL;
	}
}

/* Implementation of CongImporter: */
gboolean cong_importer_supports_mime_type(CongImporter *importer, const gchar *mime_type)
{
	g_return_val_if_fail(importer, FALSE);
	g_return_val_if_fail(mime_type, FALSE);

	g_assert(importer->mime_filter);

	return importer->mime_filter(importer, mime_type, importer->user_data);

}

void cong_importer_invoke(CongImporter *importer, const gchar *filename, const gchar *mime_type, GtkWindow *toplevel_window)
{
	g_return_if_fail(importer);
	g_return_if_fail(filename);
	g_return_if_fail(mime_type);
	
	g_assert(importer->action_callback);

	return importer->action_callback(importer, filename, mime_type, importer->user_data, toplevel_window);
}

/* Implementation of CongExporter: */
gboolean cong_exporter_supports_document(CongExporter *exporter, CongDocument *doc)
{
	g_return_val_if_fail(exporter, FALSE);
	g_return_val_if_fail(doc, FALSE);

	g_assert(exporter->doc_filter);

	return exporter->doc_filter(exporter, doc, exporter->user_data);
}

void cong_exporter_invoke(CongExporter *exporter, CongDocument *doc, const gchar *uri, GtkWindow *toplevel_window)
{
	g_return_if_fail(exporter);
	g_return_if_fail(doc);
	g_return_if_fail(uri);
	
	g_assert(exporter->action_callback);

	return exporter->action_callback(exporter, doc, uri, exporter->user_data, toplevel_window);
}

gchar *cong_exporter_get_preferred_uri(CongExporter *exporter)
{
	gchar *gconf_key;
	gchar *preferred_uri;

	g_return_val_if_fail(exporter, NULL);

	gconf_key = cong_functionality_get_gconf_key(CONG_FUNCTIONALITY(exporter), "preferred-uri");
	
	preferred_uri = gconf_client_get_string(cong_app_singleton()->gconf_client,
						gconf_key,
						NULL);

	g_free(gconf_key);

	return preferred_uri;
}

void cong_exporter_set_preferred_uri(CongExporter *exporter, const gchar *uri)
{
	gchar *gconf_key;

	g_return_if_fail(exporter);
	g_return_if_fail(uri);

	gconf_key = cong_functionality_get_gconf_key(CONG_FUNCTIONALITY(exporter), "preferred-uri");

	gconf_client_set_string(cong_app_singleton()->gconf_client,
				gconf_key,
				uri,
				NULL);

	g_free(gconf_key);
}

#if ENABLE_PRINTING
/* Implementation of CongPrintMethod: */
gboolean cong_print_method_supports_document(CongPrintMethod *print_method, CongDocument *doc)
{
	g_return_val_if_fail(print_method, FALSE);
	g_return_val_if_fail(doc, FALSE);

	g_assert(print_method->doc_filter);

	return print_method->doc_filter(print_method, doc, print_method->user_data);
}

void cong_print_method_invoke(CongPrintMethod *print_method, CongDocument *doc, GnomePrintContext *gpc, GtkWindow *toplevel_window)
{
	g_return_if_fail(print_method);
	g_return_if_fail(doc);
	g_return_if_fail(gpc);
	
	g_assert(print_method->action_callback);

	return print_method->action_callback(print_method, doc, gpc, print_method->user_data, toplevel_window);
}
#endif

gboolean cong_tool_supports_document(CongTool *tool, CongDocument *doc)
{
	g_return_val_if_fail(tool, FALSE);
	g_return_val_if_fail(doc, FALSE);

	g_assert(tool->doc_filter);
	return tool->doc_filter(tool, doc, tool->user_data);
}

void cong_tool_invoke(CongTool *tool, CongPrimaryWindow *primary_window)
{
	g_return_if_fail(tool);

	g_assert(tool->action_callback);
	return tool->action_callback(tool, primary_window, tool->user_data);
}

const gchar *cong_tool_get_menu_text(CongTool *tool)
{
	g_return_val_if_fail(tool, NULL);

	return tool->menu_text;
}

const gchar *cong_tool_get_tip_text(CongTool *tool)
{
	g_return_val_if_fail(tool, NULL);

	return tool->tooltip_text;
}

const gchar *cong_tool_get_tip_further_text(CongTool *tool)
{
	g_return_val_if_fail(tool, NULL);

	return tool->tooltip_further_text;
}

void cong_ui_new_document_from_manufactured_xml(xmlDocPtr xml_doc,
						GtkWindow *parent_window)
{
	CongDocument *cong_doc;
	CongDispspec *ds;
	
	g_return_if_fail(xml_doc);

	ds = cong_dispspec_registry_get_appropriate_dispspec(cong_app_singleton()->ds_registry, xml_doc);

	if (ds == NULL) {
		ds = query_for_forced_dispspec(_("Conglomerate could not find display information for the new file"), 
					       xml_doc, 
					       parent_window);

		if (NULL == ds) {
			xmlFreeDoc(xml_doc);
			return;
		}
	}
	
	g_assert(xml_doc);
	g_assert(ds);

	cong_doc = cong_document_new_from_xmldoc(xml_doc, ds, NULL);

	cong_primary_window_new(cong_doc);
	g_object_unref(G_OBJECT(cong_doc));
}

void cong_ui_new_document_from_imported_xml(xmlDocPtr xml_doc,
					    GtkWindow *parent_window)
{
	CongDocument *cong_doc;
	CongDispspec *ds;
	
	g_return_if_fail(xml_doc);

	ds = cong_dispspec_registry_get_appropriate_dispspec(cong_app_singleton()->ds_registry, xml_doc);

	if (ds == NULL) {
		ds = query_for_forced_dispspec(_("Conglomerate could not find display information for the new file"), 
					       xml_doc, 
					       parent_window);

		if (NULL == ds) {
			xmlFreeDoc(xml_doc);
			return;
		}
	}
	
	g_assert(xml_doc);
	g_assert(ds);

	cong_doc = cong_document_new_from_xmldoc(xml_doc, ds, NULL);

	cong_primary_window_new(cong_doc);
	g_object_unref(G_OBJECT(cong_doc));
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
				  const gchar *uri,
				  GtkWindow *toplevel_window)
{
	xmlDocPtr doc_ptr;
	GnomeVFSURI *file_uri;
	GnomeVFSResult vfs_result;
	GnomeVFSFileSize file_size;

	g_return_if_fail(doc);
	g_return_if_fail(stylesheet_filename);
	g_return_if_fail(uri);

	/* FIXME:  need some kind of feedback e.g. a busy cursor */

	doc_ptr = cong_ui_transform_doc(doc,
					stylesheet_filename,
					toplevel_window);

	if (doc_ptr) {
		file_uri = gnome_vfs_uri_new(uri);
	
		vfs_result = cong_xml_save_to_vfs(doc_ptr, 
						  file_uri,	
						  &file_size);
		
		if (vfs_result != GNOME_VFS_OK) {
			GtkDialog* dialog = cong_error_dialog_new_from_file_save_failure(toplevel_window,
											 file_uri, 
											 vfs_result, 
											 &file_size);
			
			cong_error_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(GTK_WIDGET(dialog));
		}
		
		gnome_vfs_uri_unref(file_uri);

		xmlFreeDoc(doc_ptr);
	}
}

/* Handy methods for "Import" methods; doing the necessary UI hooks: */
gboolean cong_ui_load_imported_file_content(const gchar *uri,
					    char** buffer,
					    GnomeVFSFileSize* size,
					    GtkWindow *parent_window)
{
	GnomeVFSResult vfs_result;

	g_return_val_if_fail(uri, FALSE);
	g_return_val_if_fail(buffer, FALSE);
	g_return_val_if_fail(size, FALSE);

	vfs_result = cong_vfs_new_buffer_from_file(uri, buffer, size);
	
	if (vfs_result!=GNOME_VFS_OK) {
		GnomeVFSURI* file_uri = gnome_vfs_uri_new(uri);
		GtkDialog* dialog = cong_error_dialog_new_from_file_open_failure_with_vfs_result(parent_window,
												 file_uri, 
												 vfs_result);
		
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		
		gnome_vfs_uri_unref(file_uri);
		
		return FALSE;
	}
	
	g_assert(*buffer);

	return TRUE;
}

CongElementEditor *cong_plugin_element_editor_new(CongEditorWidget2 *editor_widget, 
						  CongNodePtr node, 
						  CongDispspecElement *element)
{
	CongElementEditor *element_editor;
	gchar *message;
	const gchar *plugin_id = cong_dispspec_element_get_editor_plugin_id(element);

#if 1
	GList *plugin_iter;

	for (plugin_iter=cong_app_singleton()->plugin_manager->list_of_plugin; plugin_iter; plugin_iter = plugin_iter->next) {
		GList *editor_element_iter;
		CongPlugin *plugin = plugin_iter->data;		

		g_assert(plugin);

		for (editor_element_iter=plugin->list_of_editor_element; editor_element_iter; editor_element_iter = editor_element_iter->next) {
			CongPluginEditorElement *plugin_editor_element = editor_element_iter->data;

			g_assert(plugin_editor_element);

			if (0==strcmp(CONG_FUNCTIONALITY(plugin_editor_element)->functionality_id, plugin_id)) {
				g_assert(plugin_editor_element->make_element);
				return plugin_editor_element->make_element(plugin_editor_element, editor_widget, node, plugin_editor_element->user_data);
			}
		}
	}

	/* Handle the "plugin not found" case: */
	{
#if 1
		return cong_section_head_editor_new(editor_widget, node);
#else
		message = g_strdup_printf(_("Unrecognised plugin (id=\"%s\")"), cong_dispspec_element_get_editor_plugin_id(element));
		element_editor = cong_dummy_element_editor_new(editor_widget, node, message);

		g_free(message);
#endif
	}
#else	
	message = g_strdup_printf("Plugin (id=\"%s\")", cong_dispspec_element_get_plugin_id(element));
	element_editor = cong_dummy_element_editor_new(editor_widget, node, message);

	g_free(message);
#endif

	return element_editor;

}

CongEditorNodeElement*
cong_plugin_editor_node_factory_invoke (CongPluginEditorNodeFactory *plugin_editor_node_factory,
					CongEditorWidget3 *editor_widget, 
					CongNodePtr node,
					CongEditorNode *traversal_parent)
{
	g_return_val_if_fail(plugin_editor_node_factory, NULL);
	g_return_val_if_fail(editor_widget, NULL);
	g_return_val_if_fail(node, NULL);

	g_assert(plugin_editor_node_factory->make_node);

	return plugin_editor_node_factory->make_node (plugin_editor_node_factory, 
						      editor_widget, 
						      node, 
						      traversal_parent,
						      plugin_editor_node_factory->user_data);
}

GtkWidget *cong_custom_property_dialog_make(CongCustomPropertyDialog *custom_property_dialog,
					    CongDocument *doc,
					    CongNodePtr node)
{
	g_return_val_if_fail(custom_property_dialog, NULL);
	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);

	g_assert(custom_property_dialog->factory_method);

	return custom_property_dialog->factory_method(custom_property_dialog, doc, node);
}
