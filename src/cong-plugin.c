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

struct CongPluginPrivate
{
	gchar *plugin_id;

	CongPluginCallbackConfigure configure_callback;

#if 1
	GList *list_of_service; /* ptrs of type CongService */
#else
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
#endif
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
			g_message ("searching for \"%s\" found \"%s\"", service_id, cong_service_get_id (service));
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

void cong_plugin_for_each_document_factory(CongPlugin *plugin, void (*callback)(CongServiceDocumentFactory *factory, gpointer user_data), gpointer user_data)
{
	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (callback);

#if 1
	cong_plugin_for_each_service_of_type (plugin, 
					      CONG_SERVICE_DOCUMENT_FACTORY_TYPE,
					      (CongServiceCallback*)callback,
					      user_data);
#else
	g_list_foreach(PRIVATE(plugin)->list_of_document_factory, (GFunc)callback, user_data);
#endif
}

void cong_plugin_for_each_importer(CongPlugin *plugin, void (*callback)(CongServiceImporter *importer, gpointer user_data), gpointer user_data)
{
	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (callback);

#if 1
	cong_plugin_for_each_service_of_type (plugin, 
					      CONG_SERVICE_IMPORTER_TYPE,
					      (CongServiceCallback*)callback,
					      user_data);
#else
	g_list_foreach(PRIVATE (plugin)->list_of_importer, (GFunc)callback, user_data);
#endif
}

void cong_plugin_for_each_exporter(CongPlugin *plugin, void (*callback)(CongServiceExporter *exporter, gpointer user_data), gpointer user_data)
{
	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (callback);

#if 1
	cong_plugin_for_each_service_of_type (plugin, 
					      CONG_SERVICE_EXPORTER_TYPE,
					      (CongServiceCallback*)callback,
					      user_data);
#else
	g_list_foreach(PRIVATE (plugin)->list_of_exporter, (GFunc)callback, user_data);
#endif
}

#if ENABLE_PRINTING
void cong_plugin_for_each_print_method(CongPlugin *plugin, void (*callback)(CongServicePrintMethod *print_method, gpointer user_data), gpointer user_data)
{
	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (callback);

#if 1
	cong_plugin_for_each_service_of_type (plugin, 
					      CONG_SERVICE_PRINT_METHOD_TYPE,
					      (CongServiceCallback*)callback,
					      user_data);
#else
	g_list_foreach(PRIVATE (plugin)->list_of_print_method, (GFunc)callback, user_data);
#endif
}
#endif

void cong_plugin_for_each_doc_tool(CongPlugin *plugin, void (*callback)(CongServiceDocTool *doc_tool, gpointer user_data), gpointer user_data)
{
	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (callback);

#if 1
	cong_plugin_for_each_service_of_type (plugin, 
					      CONG_SERVICE_DOC_TOOL_TYPE,
					      (CongServiceCallback*)callback,
					      user_data);
#else
	g_list_foreach(PRIVATE (plugin)->list_of_doc_tool, (GFunc)callback, user_data);
#endif
}

void cong_plugin_for_each_node_tool(CongPlugin *plugin, void (*callback)(CongServiceNodeTool *node_tool, gpointer user_data), gpointer user_data)
{
	g_return_if_fail (IS_CONG_PLUGIN (plugin));
	g_return_if_fail (callback);

#if 1
	cong_plugin_for_each_service_of_type (plugin, 
					      CONG_SERVICE_NODE_TOOL_TYPE,
					      (CongServiceCallback*)callback,
					      user_data);
#else
	g_list_foreach(PRIVATE (plugin)->list_of_node_tool, (GFunc)callback, user_data);
#endif
}


#if 1
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

const gchar* 
cong_plugin_get_id (CongPlugin *plugin)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);

	return PRIVATE (plugin)->plugin_id;
}

gchar* cong_plugin_get_gconf_namespace(CongPlugin *plugin)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);

	g_assert(PRIVATE (plugin)->plugin_id);

	return g_strdup_printf( (CONG_GCONF_PATH "plugins/%s"), PRIVATE (plugin)->plugin_id);
}

gchar* cong_plugin_get_gconf_key(CongPlugin *plugin, const gchar *local_part)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (local_part, NULL);

	g_assert(PRIVATE (plugin)->plugin_id);

	return g_strdup_printf( (CONG_GCONF_PATH "plugins/%s/%s"), PRIVATE (plugin)->plugin_id, local_part);
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

	g_return_val_if_fail (doc, NULL);
	g_return_val_if_fail (stylesheet_filename, NULL);

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

	g_return_if_fail (doc);
	g_return_if_fail (stylesheet_filename);
	g_return_if_fail (string_uri);

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

	g_return_val_if_fail (string_uri, FALSE);
	g_return_val_if_fail (buffer, FALSE);
	g_return_val_if_fail (size, FALSE);

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


/* Registration methods for various services: */
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

CongServiceEditorNodeFactory *cong_plugin_register_editor_node_factory(CongPlugin *plugin, 
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


CongServiceExporter*
cong_plugin_register_exporter (CongPlugin *plugin, 
			       const gchar *name, 
			       const gchar *description,
			       const gchar *service_id,
			       CongServiceExporterDocumentFilter doc_filter,
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
						    action_callback,
						    user_data);

	cong_plugin_add_service (plugin,
				 CONG_SERVICE (exporter));

	return exporter;
}

CongServiceImporter*
cong_plugin_register_importer (CongPlugin *plugin, 
			       const gchar *name, 
			       const gchar *description,
			       const gchar *service_id,
			       CongServiceImporterMimeFilter mime_filter,
			       CongServiceImporterActionCallback action_callback,
			       gpointer user_data)
{
	CongServiceImporter *importer;

	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);
	g_return_val_if_fail (mime_filter, NULL);
	g_return_val_if_fail (action_callback, NULL);

        importer = cong_service_importer_construct (g_object_new (CONG_SERVICE_IMPORTER_TYPE, NULL),
						    name,
						    description,
						    service_id,
						    mime_filter,
						    action_callback,
						    user_data);

	cong_plugin_add_service (plugin,
				 CONG_SERVICE (importer));

	return importer;
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

#if ENABLE_PRINTING
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
