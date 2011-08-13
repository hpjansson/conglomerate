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
#include "cong-source-layout.h"

/* Private definitions of the types: */

struct CongPluginPrivate
{
	gchar *plugin_id;

	CongPluginCallbackConfigure configure_callback;

	GList *list_of_service; /* ptrs of type CongService */
};

CONG_DEFINE_CLASS (CongPlugin, cong_plugin, CONG_PLUGIN, GObject, G_TYPE_OBJECT)

#if 0
struct CongServiceThumbnailerPrivate
{
	CongService service; /* base class */
};
#endif


#if 0
void cong_plugin_unregister_document_factory(CongPlugin *plugin, 
					     CongServiceDocumentFactory *factory);

void cong_plugin_unregister_importer(CongPlugin *plugin, 
				     CongServiceImporter *importer);

void cong_plugin_unregister_exporter(CongPlugin *plugin, 
				     CongServiceExporter *exporter);
#endif



/* Implementation of CongPlugin: */
/**
 * cong_plugin_construct:
 * @plugin:
 * @plugin_id:
 * @register_callback:
 * @configure_callback:
 *
 * TODO: Write me
 * Returns:
 */
CongPlugin*
cong_plugin_construct (CongPlugin *plugin,
		       const gchar *plugin_id,
		       CongPluginCallbackRegister register_callback,
		       CongPluginCallbackConfigure configure_callback)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (plugin_id, NULL);
	g_return_val_if_fail (register_callback, NULL);
	/* the configure callback is allowed to be NULL */

	PRIVATE (plugin)->plugin_id = g_strdup (plugin_id);
	PRIVATE (plugin)->configure_callback = configure_callback;

	(*register_callback)(plugin);

	return plugin;
}

/**
 * cong_plugin_add_service:
 * @plugin:
 * @service:
 *
 * TODO: Write me
 */
void
cong_plugin_add_service (CongPlugin *plugin,
			 CongService *service)
{
	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (IS_CONG_SERVICE (service));

	/* Add to plugin's list: */
	PRIVATE (plugin)->list_of_service = g_list_append (PRIVATE (plugin)->list_of_service, 
							   service);
}

/**
 * cong_plugin_for_each_service:
 * @plugin:
 * @callback:
 * @user_data:
 *
 * TODO: Write me
 */
void
cong_plugin_for_each_service (CongPlugin *plugin, 
			      void 
			      (*callback) (CongService *service,
					   gpointer user_data),
			      gpointer user_data)
{
	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (callback);

	g_list_foreach(PRIVATE(plugin)->list_of_service, (GFunc)callback, user_data);	
}

/**
 * cong_plugin_for_each_service_of_type:
 * @plugin:
 * @type:
 * @callback:
 * @user_data:
 *
 * TODO: Write me
 */
void
cong_plugin_for_each_service_of_type (CongPlugin *plugin, 
				      GType type,
				      void 
				      (*callback) (CongService *service,
						   gpointer user_data),
				      gpointer user_data)
{
	GList *iter;

	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (callback);

	for (iter=PRIVATE(plugin)->list_of_service;iter;iter=iter->next) {
		CongService *service = CONG_SERVICE (iter->data);
		if (g_type_check_instance_is_a ((GTypeInstance*)service, type)) {
			(*callback)(service, 
				    user_data);
		}
	}
}

/**
 * cong_plugin_locate_service_by_id:
 * @plugin:
 * @type:
 * @service_id:
 *
 * TODO: Write me
 * Returns:
 */
CongService*
cong_plugin_locate_service_by_id (CongPlugin *plugin, 
				  GType type,
				  const gchar *service_id)
{
	GList *iter;

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (service_id, NULL);

	for (iter=PRIVATE(plugin)->list_of_service;iter;iter=iter->next) {
		CongService *service = CONG_SERVICE (iter->data);

		if (g_type_check_instance_is_a ((GTypeInstance*)service, type)) {
			/* g_message ("searching for \"%s\" found \"%s\"", service_id, cong_service_get_id (service)); */
			if (0==strcmp(service_id, cong_service_get_id (service))) {
				return service;
			}
		}
	}

	return NULL;
}


typedef void
(CongServiceCallback) (CongService *service,
		       gpointer user_data);

/**
 * cong_plugin_for_each_document_factory:
 * @plugin:
 * @callback:
 * @user_data:
 *
 * TODO: Write me
 */
void 
cong_plugin_for_each_document_factory(CongPlugin *plugin, void (*callback)(CongServiceDocumentFactory *factory, gpointer user_data), gpointer user_data)
{
	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (callback);

	cong_plugin_for_each_service_of_type (plugin,
					      CONG_SERVICE_DOCUMENT_FACTORY_TYPE,
					      (CongServiceCallback*)callback,
					      user_data);
}

/**
 * cong_plugin_for_each_importer:
 * @plugin:
 * @callback:
 * @user_data:
 *
 * TODO: Write me
 */
void 
cong_plugin_for_each_importer(CongPlugin *plugin, void (*callback)(CongServiceImporter *importer, gpointer user_data), gpointer user_data)
{
	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (callback);

	cong_plugin_for_each_service_of_type (plugin, 
					      CONG_SERVICE_IMPORTER_TYPE,
					      (CongServiceCallback*)callback,
					      user_data);
}

/**
 * cong_plugin_for_each_exporter:
 * @plugin:
 * @callback:
 * @user_data:
 *
 * TODO: Write me
 */
void 
cong_plugin_for_each_exporter(CongPlugin *plugin, void (*callback)(CongServiceExporter *exporter, gpointer user_data), gpointer user_data)
{
	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (callback);

	cong_plugin_for_each_service_of_type (plugin, 
					      CONG_SERVICE_EXPORTER_TYPE,
					      (CongServiceCallback*)callback,
					      user_data);
}

#if ENABLE_PRINTING
/**
 * cong_plugin_for_each_print_method:
 * @plugin:
 * @callback:
 * @user_data:
 *
 * TODO: Write me
 */
void 
cong_plugin_for_each_print_method(CongPlugin *plugin, void (*callback)(CongServicePrintMethod *print_method, gpointer user_data), gpointer user_data)
{
	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (callback);

	cong_plugin_for_each_service_of_type (plugin, 
					      CONG_SERVICE_PRINT_METHOD_TYPE,
					      (CongServiceCallback*)callback,
					      user_data);
}
#endif

/**
 * cong_plugin_for_each_doc_tool:
 * @plugin:
 * @callback:
 * @user_data:
 *
 * TODO: Write me
 */
void 
cong_plugin_for_each_doc_tool(CongPlugin *plugin, void (*callback)(CongServiceDocTool *doc_tool, gpointer user_data), gpointer user_data)
{
	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (callback);

	cong_plugin_for_each_service_of_type (plugin, 
					      CONG_SERVICE_DOC_TOOL_TYPE,
					      (CongServiceCallback*)callback,
					      user_data);
}

/**
 * cong_plugin_for_each_node_tool:
 * @plugin:
 * @callback:
 * @user_data:
 *
 * TODO: Write me
 */
void 
cong_plugin_for_each_node_tool(CongPlugin *plugin, void (*callback)(CongServiceNodeTool *node_tool, gpointer user_data), gpointer user_data)
{
	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (callback);

	cong_plugin_for_each_service_of_type (plugin, 
					      CONG_SERVICE_NODE_TOOL_TYPE,
					      (CongServiceCallback*)callback,
					      user_data);
}


#if 1
/**
 * cong_plugin_locate_custom_property_dialog_by_id:
 * @plugin:
 * @service_id:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceNodePropertyDialog*
cong_plugin_locate_custom_property_dialog_by_id (CongPlugin *plugin, 
						 const gchar *service_id)
{	
	CongService *service;

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (service_id, NULL);

	service = cong_plugin_locate_service_by_id (plugin, 
						    CONG_SERVICE_NODE_PROPERTY_DIALOG_TYPE,
						    service_id);

	if (service) {
		return CONG_SERVICE_NODE_PROPERTY_DIALOG (service);
	} else {
		return NULL;
	}
}

/**
 * cong_plugin_locate_editor_node_factory_by_id:
 * @plugin:
 * @service_id:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceEditorNodeFactory*
cong_plugin_locate_editor_node_factory_by_id (CongPlugin *plugin,
					      const gchar *service_id)
{
	CongService *service;

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (service_id, NULL);

	service = cong_plugin_locate_service_by_id (plugin, 
						    CONG_SERVICE_EDITOR_NODE_FACTORY_TYPE,
						    service_id);

	if (service) {
		return CONG_SERVICE_EDITOR_NODE_FACTORY (service);
	} else {
		return NULL;
	}

}
#else
/**
 * cong_plugin_locate_custom_property_dialog_by_id:
 * @plugin:
 * @service_id:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceNodePropertyDialog*
cong_plugin_locate_custom_property_dialog_by_id (CongPlugin *plugin, 
						 const gchar *service_id)
{	
	GList *dialog_iter;

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (service_id, NULL);

	for (dialog_iter = PRIVATE (plugin)->list_of_property_dialog; dialog_iter; dialog_iter=dialog_iter->next) {
		CongServiceNodePropertyDialog* dialog = CONG_SERVICE_NODE_PROPERTY_DIALOG (dialog_iter->data);
		g_assert (dialog);
		if (0==strcmp(service_id, cong_service_get_id (CONG_SERVICE(dialog)))) {
			return dialog;
		}
	}
	
	return NULL;
}

/**
 * cong_plugin_locate_editor_node_factory_by_id:
 * @plugin:
 * @service_id:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceEditorNodeFactory*
cong_plugin_locate_editor_node_factory_by_id (CongPlugin *plugin,
					      const gchar *service_id)
{
	GList *factory_iter;
	
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (service_id, NULL);

	for (factory_iter = PRIVATE (plugin)->list_of_editor_node_factory; factory_iter; factory_iter=factory_iter->next) {
		CongServiceEditorNodeFactory* factory = CONG_SERVICE_EDITOR_NODE_FACTORY (factory_iter->data);
		g_assert (factory);
		if (0==strcmp(service_id, cong_service_get_id (CONG_SERVICE(factory)))) {
			return factory;
		}
	}

	return NULL;
}
#endif

/**
 * cong_plugin_get_id:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
const gchar* 
cong_plugin_get_id (CongPlugin *plugin)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);

	return PRIVATE (plugin)->plugin_id;
}

/**
 * cong_plugin_get_gconf_namespace:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gchar* 
cong_plugin_get_gconf_namespace(CongPlugin *plugin)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);

	g_assert(PRIVATE (plugin)->plugin_id);

	return g_strdup_printf( (CONG_GCONF_PATH "plugins/%s"), PRIVATE (plugin)->plugin_id);
}

gchar* 
cong_plugin_get_gconf_key(CongPlugin *plugin, const gchar *local_part)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (local_part, NULL);

	g_assert(PRIVATE (plugin)->plugin_id);

	return g_strdup_printf( (CONG_GCONF_PATH "plugins/%s/%s"), PRIVATE (plugin)->plugin_id, local_part);
}

/**
 * cong_ui_new_document_from_manufactured_xml:
 * @xml_doc:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
CongDocument*
cong_ui_new_document_from_manufactured_xml (xmlDocPtr xml_doc,
					    GtkWindow *parent_window)
{
	CongDocument *cong_doc;
	CongDispspec *ds;
	
	g_return_val_if_fail (xml_doc, NULL);

	ds = cong_dispspec_registry_get_appropriate_dispspec (cong_app_get_dispspec_registry (cong_app_singleton()), 
							      xml_doc,
							      NULL);

	#if 0
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
	#endif
	
	g_assert(xml_doc);

	cong_doc = cong_document_new_from_xmldoc(xml_doc, ds, NULL);

	/* Clean up the xml_doc; eventually there might be a user preference for this */
	{
		CongCommand *command;
		CongSourceCleanupOptions options;

		/* FIXME: get user preferences about preferred XML layout */
		options.use_tabs = TRUE;
		options.wrap_text = TRUE;
		options.num_text_columns = 80;
		
		command = cong_document_begin_command (cong_doc,
						       "",
						       NULL);
		
		cong_command_add_cleanup_source (command,
						 &options);
		
		cong_document_end_preprocessor_command (cong_doc,
							command);
	}

	cong_primary_window_new(cong_doc);
	g_object_unref(G_OBJECT(cong_doc));

	return cong_doc;
}

/**
 * cong_ui_new_document_from_imported_xml:
 * @xml_doc:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
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

/* Handy methods for "Import" methods; doing the necessary UI hooks: */
/**
 * cong_ui_load_imported_file_content:
 * @uri:
 * @buffer:
 * @size:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
cong_ui_load_imported_file_content(GFile *file,
				   char** buffer,
				   gsize* size,
				   GtkWindow *parent_window)
{
	gboolean result;
	GError *error = NULL;

	g_return_val_if_fail (file, FALSE);
	g_return_val_if_fail (buffer, FALSE);
	g_return_val_if_fail (size, FALSE);

	result = cong_vfs_new_buffer_from_file(file, buffer, size, &error);
	
	if (!result) {
		GtkDialog* dialog = cong_error_dialog_new_from_file_open_failure_with_gerror (parent_window,
		                                                                              file,
		                                                                              error);
		
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		
		return FALSE;
	}
	
	g_assert(*buffer);

	return TRUE;
}

/* Registration methods for various services: */
/**
 * cong_plugin_register_document_factory:
 * @plugin:
 * @name:
 * @description:
 * @id:
 * @page_creation_callback:
 * @action_callback:
 * @icon:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceDocumentFactory*
cong_plugin_register_document_factory (CongPlugin *plugin, 
				       const gchar *name, 
				       const gchar *description,
				       const gchar *id,
				       CongServiceDocumentFactoryPageCreationCallback page_creation_callback,
				       CongServiceDocumentFactoryActionCallback action_callback,
				       const gchar *icon,
				       gpointer user_data)
{
	CongServiceDocumentFactory* factory;

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (id, NULL);
	g_return_val_if_fail (page_creation_callback, NULL);
	g_return_val_if_fail (action_callback, NULL);
	/* icon is allowed to be NULL */

	factory = cong_service_document_factory_construct (g_object_new (CONG_SERVICE_DOCUMENT_FACTORY_TYPE, NULL),
							   name, 
							   description,
							   id,
							   page_creation_callback,
							   action_callback,
							   icon,
							   user_data);

	cong_plugin_add_service (plugin,
				 CONG_SERVICE (factory));

	return factory;
}

/**
 * cong_plugin_register_doc_tool:
 * @plugin:
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

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);

        tool = cong_service_doc_tool_construct (g_object_new (CONG_SERVICE_DOC_TOOL_TYPE, NULL),
						name,
						description,
						service_id,
						menu_text,
						tooltip_text,
						tooltip_further_text,
						doc_filter,
						action_callback,
						user_data);

	cong_plugin_add_service (plugin,
				 CONG_SERVICE (tool));

	return tool;

}

/**
 * cong_plugin_register_node_tool:
 * @plugin:
 * @name:
 * @description:
 * @service_id:
 * @menu_text:
 * @tooltip_text:
 * @tooltip_further_text:
 * @node_filter:
 * @action_callback:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
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

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);

        tool = cong_service_node_tool_construct (g_object_new (CONG_SERVICE_NODE_TOOL_TYPE, NULL),
						 name,
						 description,
						 service_id,
						 menu_text,
						 tooltip_text,
						 tooltip_further_text,
						 node_filter,
						 action_callback,
						 user_data);

	cong_plugin_add_service (plugin,
				 CONG_SERVICE (tool));

	return tool;

}

/**
 * cong_plugin_register_editor_node_factory:
 * @plugin:
 * @name:
 * @description:
 * @plugin_id:
 * @factory_method:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceEditorNodeFactory *
cong_plugin_register_editor_node_factory(CongPlugin *plugin, 
					 const gchar *name, 
					 const gchar *description,
					 const gchar *service_id,
					 CongEditorNodeFactoryMethod factory_method,
					 gpointer user_data)
{
	CongServiceEditorNodeFactory *editor_node_factory;

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);
	g_return_val_if_fail (factory_method, NULL);

        editor_node_factory = cong_service_editor_node_factory_construct (g_object_new (CONG_SERVICE_EDITOR_NODE_FACTORY_TYPE, NULL),
									  name,
									  description,
									  service_id,
									  factory_method,
									  user_data);

	cong_plugin_add_service (plugin,
				 CONG_SERVICE (editor_node_factory));

	return editor_node_factory;
}

/**
 * cong_plugin_register_exporter:
 * @plugin:
 * @name:
 * @description:
 * @id:
 * @doc_filter:
 * @options_widget_callback:
 * @action_callback:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceExporter*
cong_plugin_register_exporter (CongPlugin *plugin, 
			       const gchar *name, 
			       const gchar *description,
			       const gchar *service_id,
			       CongServiceExporterDocumentFilter doc_filter,
			       CongServiceExporterOptionsWidgetCallback options_widget_callback,
			       CongServiceExporterActionCallback action_callback,
			       gpointer user_data)
{
	CongServiceExporter *exporter;

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);
	g_return_val_if_fail (doc_filter, NULL);
	g_return_val_if_fail (action_callback, NULL);

        exporter = cong_service_exporter_construct (g_object_new (CONG_SERVICE_EXPORTER_TYPE, NULL),
						    name,
						    description,
						    service_id,
						    doc_filter,
						    options_widget_callback,
						    action_callback,
						    user_data);

	cong_plugin_add_service (plugin,
				 CONG_SERVICE (exporter));

	return exporter;
}

/**
 * cong_plugin_register_importer:
 * @plugin:
 * @name:
 * @description:
 * @id:
 * @filter_factory_callback:
 * @options_widget_callback:
 * @action_callback:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceImporter*
cong_plugin_register_importer (CongPlugin *plugin, 
			       const gchar *name, 
			       const gchar *description,
			       const gchar *service_id,	
			       CongServiceImporterMakeFilterCallback filter_factory_callback,
			       CongServiceImporterOptionsWidgetCallback options_widget_callback,
			       CongServiceImporterActionCallback action_callback,
			       gpointer user_data)
{
	CongServiceImporter *importer;

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);
	g_return_val_if_fail (filter_factory_callback, NULL);
	g_return_val_if_fail (action_callback, NULL);

        importer = cong_service_importer_construct (g_object_new (CONG_SERVICE_IMPORTER_TYPE, NULL),
						    name,
						    description,
						    service_id,
						    filter_factory_callback,
						    options_widget_callback,
						    action_callback,
						    user_data);

	cong_plugin_add_service (plugin,
				 CONG_SERVICE (importer));

	return importer;
}

/**
 * cong_plugin_register_custom_property_dialog:
 * @plugin:
 * @name:
 * @description:
 * @service_id:
 * @factory_method:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceNodePropertyDialog*
cong_plugin_register_custom_property_dialog (CongPlugin *plugin,
					     const gchar *name, 
					     const gchar *description,
					     const gchar *service_id,
					     CongCustomPropertyFactoryMethod factory_method,
					     gpointer user_data)
{
	CongServiceNodePropertyDialog *property_dialog;

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);

        property_dialog = cong_service_node_property_dialog_construct (g_object_new (CONG_SERVICE_NODE_PROPERTY_DIALOG_TYPE, NULL),
								       name,
								       description,
								       service_id,
								       factory_method,
								       user_data);

	cong_plugin_add_service (plugin,
				 CONG_SERVICE (property_dialog));

	return property_dialog;

}

/**
 * cong_plugin_register_custom_property_dialog_for_element:
 * @plugin:
 * @element_name:
 * @service_id:
 * @factory_method:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
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

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
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

/**
 * cong_plugin_register_custom_property_page:
 * @plugin:
 * @name:
 * @description:
 * @service_id:
 * @factory_method:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceNodePropertyPage*
cong_plugin_register_custom_property_page (CongPlugin *plugin,
					   const gchar *name, 
					   const gchar *description,
					   const gchar *service_id,
					   CongCustomPropertyPageFactoryMethod factory_method,
					   gpointer user_data)
{
	CongServiceNodePropertyPage *property_page;

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);

        property_page = cong_service_node_property_page_construct (g_object_new (CONG_SERVICE_NODE_PROPERTY_PAGE_TYPE, NULL),
								   name,
								   description,
								   service_id,
								   factory_method,
								   user_data);

	cong_plugin_add_service (plugin,
				 CONG_SERVICE (property_page));

	return property_page;
}

/**
 * cong_plugin_register_custom_property_page_for_element:
 * @plugin:
 * @element_name:
 * @service_id:
 * @factory_method:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceNodePropertyPage*
cong_plugin_register_custom_property_page_for_element (CongPlugin *plugin,
						       const gchar *element_name,
						       const gchar *service_id,
						       CongCustomPropertyPageFactoryMethod factory_method,
						       gpointer user_data)
{
	CongServiceNodePropertyPage *property_page;
	gchar *name;
	gchar *description;	

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (element_name, NULL);
	g_return_val_if_fail (service_id, NULL);

	/* Generate a user-visible name for this plugin property page */
#if 1
	name = g_strdup (element_name);
#else
	name = g_strdup_printf (_("<%s> property page"), element_name);
#endif

	/* Generate a user-visible description for this plugin */
	description = g_strdup_printf (_("Provides a Properties page for the <%s> element"), element_name);

	property_page = cong_plugin_register_custom_property_page (plugin,
								   name, 
								   description,
								   service_id,
								   factory_method,
								   user_data);
	g_free (name);
	g_free (description);
	
	return property_page;
}


#if ENABLE_PRINTING
/**
 * cong_plugin_register_print_method:
 * @plugin:
 * @name:
 * @description:
 * @id:
 * @doc_filter:
 * @action_callback:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServicePrintMethod*
cong_plugin_register_print_method (CongPlugin *plugin, 
				   const gchar *name, 
				   const gchar *description,
				   const gchar *service_id,
				   CongServicePrintMethodDocumentFilter doc_filter,
				   CongServicePrintMethodActionCallback action_callback,
				   gpointer user_data)
{
	CongServicePrintMethod *print_method;

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);
	g_return_val_if_fail (doc_filter, NULL);
	g_return_val_if_fail (action_callback, NULL);

        print_method = cong_service_print_method_construct (g_object_new (CONG_SERVICE_PRINT_METHOD_TYPE, NULL),
							    name,
							    description,
							    service_id,
							    doc_filter,
							    action_callback,
							    user_data);

	cong_plugin_add_service (plugin,
				 CONG_SERVICE (print_method));

	return print_method;
}
#endif
