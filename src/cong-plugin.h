/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-plugin.h
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

#ifndef __CONG_PLUGIN_H__
#define __CONG_PLUGIN_H__

#include "cong-document.h"
#include "cong-editor-widget.h"
#include "cong-editor-node-element.h"

G_BEGIN_DECLS

/* PLUGIN INTERFACE: 
   These types are fully opaque, to try to minimise ABI issues.
*/
typedef struct CongPlugin CongPlugin;
typedef struct CongPluginManager CongPluginManager;

typedef struct CongService CongService;
#define CONG_SERVICE(x) ((CongService*)(x))

/* The following are all castable to CongService: */
typedef struct CongServiceDocumentFactory CongServiceDocumentFactory;
typedef struct CongServiceImporter CongServiceImporter;
typedef struct CongServiceExporter CongServiceExporter;
#if ENABLE_PRINTING
typedef struct CongServicePrintMethod CongServicePrintMethod;
#endif
typedef struct CongServiceThumbnailer CongServiceThumbnailer;
typedef struct CongServiceEditorNodeFactory CongServiceEditorNodeFactory;

typedef struct CongServiceTool CongServiceTool;
#define CONG_TOOL(x) ((CongServiceTool*)(x))
typedef struct CongServiceDocTool CongServiceDocTool;
typedef struct CongServiceNodeTool CongServiceNodeTool;

typedef struct CongServiceNodePropertyDialog CongServiceNodePropertyDialog;

/* The File->New GUI: */
typedef struct CongNewFileAssistant CongNewFileAssistant;


/* Function pointers to be exposed by .so/.dll files: */
typedef gboolean 
(*CongPluginCallbackInit)(CongPlugin *plugin); /* exposed as "plugin_init"? */

typedef gboolean 
(*CongPluginCallbackUninit)(CongPlugin *plugin); /* exposed as "plugin_uninit"? */

typedef gboolean 
(*CongPluginCallbackRegister)(CongPlugin *plugin); /* exposed as "plugin_register"? */

typedef gboolean 
(*CongPluginCallbackConfigure)(CongPlugin *plugin);  /* exposed as "plugin_configure"? legitimate for it not to be present */

/* Function pointers that are registered by plugins: */
typedef void 
(*CongServiceDocumentFactoryPageCreationCallback) (CongServiceDocumentFactory *factory, 
						   CongNewFileAssistant *assistant, 
						   gpointer user_data);

typedef void 
(*CongServiceDocumentFactoryActionCallback) (CongServiceDocumentFactory *factory, 
					     CongNewFileAssistant *assistant, 
					     gpointer user_data);

typedef gboolean
(*CongServiceImporterMimeFilter) (CongServiceImporter *importer, 
				  const gchar *mime_type, 
				  gpointer user_data);
typedef void 
(*CongServiceImporterActionCallback) (CongServiceImporter *importer, 
				      const gchar *uri, 
				      const gchar *mime_type, 
				      gpointer user_data, 
				      GtkWindow *toplevel_window);

typedef gboolean 
(*CongServiceExporterDocumentFilter) (CongServiceExporter *exporter, 
				      CongDocument *doc, 
				      gpointer user_data);

typedef void 
(*CongServiceExporterActionCallback) (CongServiceExporter *exporter, 
				      CongDocument *doc, 
				      const gchar *uri, 
				      gpointer user_data, 
				      GtkWindow *toplevel_window);

typedef gboolean 
(*CongServiceDocToolFilter) (CongServiceDocTool *doc_tool, 
			     CongDocument *doc, 
			     gpointer user_data);
typedef void 
(*CongServiceDocToolActionCallback) (CongServiceDocTool *doc_tool, 
				     CongPrimaryWindow *primary_window, 
				     gpointer user_data);
typedef gboolean 
(*CongServiceNodeToolFilter) (CongServiceNodeTool *node_tool, 
			      CongDocument *doc, 
			      CongNodePtr node,
			      gpointer user_data);
typedef void 
(*CongServiceNodeToolActionCallback) (CongServiceNodeTool *tool, 
				      CongPrimaryWindow *primary_window, 
				      CongNodePtr node,
				      gpointer user_data);

typedef GtkWidget* 
(*CongCustomPropertyFactoryMethod) (CongServiceNodePropertyDialog *custom_property_dialog, 
				    CongDocument *doc, 
				    CongNodePtr node);

#if ENABLE_PRINTING
typedef gboolean 
(*CongServicePrintMethodDocumentFilter) (CongServicePrintMethod *print_method, 
					 CongDocument *doc, 
					 gpointer user_data);

typedef void 
(*CongServicePrintMethodActionCallback) (CongServicePrintMethod *print_method, 
					 CongDocument *doc, 
					 GnomePrintContext *gpc, 
					 gpointer user_data, 
					 GtkWindow *toplevel_window);
#endif

typedef CongEditorNodeElement* 
(*CongEditorNodeFactoryMethod) (CongServiceEditorNodeFactory *plugin_editor_node_factory, 
				CongEditorWidget3 *editor_widget, 
				CongTraversalNode *traversal_node, 
				gpointer user_data);

/* 
   CongPluginManager
*/
CongPluginManager*
cong_plugin_manager_new (void);

CongPlugin*
cong_plugin_manager_register (CongPluginManager *plugin_manager, 
			      const gchar *id,
			      CongPluginCallbackRegister register_callback,
			      CongPluginCallbackConfigure configure_callback);

void 
cong_plugin_manager_unregister (CongPluginManager *plugin_manager, 
				CongPlugin *plugin);

void
cong_plugin_manager_for_each_plugin (CongPluginManager *plugin_manager, 
				     void 
				     (*callback) (CongPlugin *plugin, 
						  gpointer user_data),
				     gpointer user_data);

void cong_plugin_manager_for_each_document_factory(CongPluginManager *plugin_manager, void (*callback)(CongServiceDocumentFactory *factory, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_importer(CongPluginManager *plugin_manager, void (*callback)(CongServiceImporter *importer, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_exporter(CongPluginManager *plugin_manager, void (*callback)(CongServiceExporter *exporter, gpointer user_data), gpointer user_data);
#if ENABLE_PRINTING
void cong_plugin_manager_for_each_printmethod(CongPluginManager *plugin_manager, void (*callback)(CongServicePrintMethod *print_method, gpointer user_data), gpointer user_data);
#endif
void cong_plugin_manager_for_each_thumbnailer(CongPluginManager *plugin_manager, void (*callback)(CongServiceThumbnailer *thumbnailer, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_doc_tool(CongPluginManager *plugin_manager, void (*callback)(CongServiceDocTool *doc_tool, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_node_tool(CongPluginManager *plugin_manager, void (*callback)(CongServiceNodeTool *node_tool, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_custom_property_dialog(CongPluginManager *plugin_manager, void (*callback)(CongServiceNodePropertyDialog *custom_property_dialog, gpointer user_data), gpointer user_data);

CongServiceNodePropertyDialog*
cong_plugin_manager_locate_custom_property_dialog_by_id (CongPluginManager *plugin_manager, 
							 const gchar *plugin_id);

CongServiceEditorNodeFactory*
cong_plugin_manager_locate_editor_node_factory_by_id (CongPluginManager *plugin_manager,
						      const gchar *plugin_id);


/* 
   CongPlugin 

   These are manufactured by the CongPluginManager and passed to the registration/unregistration hooks exposed by the .so/.dll files.

   There are various methods to allow plugins to register their service with the app.
*/
CongServiceDocumentFactory*
cong_plugin_register_document_factory (CongPlugin *plugin, 
				       const gchar *name, 
				       const gchar *description,
				       const gchar *id,
				       CongServiceDocumentFactoryPageCreationCallback page_creation_callback,
				       CongServiceDocumentFactoryActionCallback action_callback,
				       const gchar *icon,
				       gpointer user_data);
CongServiceImporter*
cong_plugin_register_importer(CongPlugin *plugin, 
			      const gchar *name, 
			      const gchar *description,
			      const gchar *id,
			      CongServiceImporterMimeFilter mime_filter,
			      CongServiceImporterActionCallback action_callback,
			      gpointer user_data);
CongServiceExporter *cong_plugin_register_exporter(CongPlugin *plugin, 
					    const gchar *name, 
					    const gchar *description,
					    const gchar *id,
					    CongServiceExporterDocumentFilter doc_filter,
					    CongServiceExporterActionCallback action_callback,
					    gpointer user_data);
#if ENABLE_PRINTING
CongServicePrintMethod *cong_plugin_register_print_method(CongPlugin *plugin, 
					    const gchar *name, 
					    const gchar *description,
					    const gchar *id,
					    CongServicePrintMethodDocumentFilter doc_filter,
					    CongServicePrintMethodActionCallback action_callback,
					    gpointer user_data);
#endif
CongServiceEditorNodeFactory *cong_plugin_register_editor_node_factory(CongPlugin *plugin, 
								      const gchar *name, 
								      const gchar *description,
								      const gchar *plugin_id,
								      CongEditorNodeFactoryMethod factory_method,
								      gpointer user_data);
CongServiceDocTool*
cong_plugin_register_doc_tool(CongPlugin *plugin,
			      const gchar *name, 
			      const gchar *description,
			      const gchar *service_id,
			      const gchar *menu_text,
			      const gchar *tooltip_text,
			      const gchar *tooltip_further_text,
			      CongServiceDocToolFilter doc_filter,
			      CongServiceDocToolActionCallback action_callback,
			      gpointer user_data);

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
				gpointer user_data);

CongServiceNodePropertyDialog*
cong_plugin_register_custom_property_dialog (CongPlugin *plugin,
					     const gchar *name, 
					     const gchar *description,
					     const gchar *service_id,
					     CongCustomPropertyFactoryMethod factory_method,
					     gpointer user_data);

/* 
   Utility to reduce the number of strings needing translation (addressing bug #124780); this
   is a wrapper around cong_plugin_register_custom_property_dialog 
*/
CongServiceNodePropertyDialog*
cong_plugin_register_custom_property_dialog_for_element (CongPlugin *plugin,
							 const gchar *element_name,
							 const gchar *service_id,
							 CongCustomPropertyFactoryMethod factory_method,
							 gpointer user_data);

void cong_plugin_for_each_document_factory(CongPlugin *plugin, void (*callback)(CongServiceDocumentFactory *factory, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_importer(CongPlugin *plugin, void (*callback)(CongServiceImporter *importer, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_exporter(CongPlugin *plugin, void (*callback)(CongServiceExporter *exporter, gpointer user_data), gpointer user_data);
#if ENABLE_PRINTING
void cong_plugin_for_each_print_method(CongPlugin *plugin, void (*callback)(CongServicePrintMethod *print_method, gpointer user_data), gpointer user_data);
#endif
void cong_plugin_for_each_thumbnailer(CongPlugin *plugin, void (*callback)(CongServiceThumbnailer *thumbnailer, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_doc_tool(CongPlugin *plugin, void (*callback)(CongServiceDocTool *doc_tool, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_node_tool(CongPlugin *plugin, void (*callback)(CongServiceNodeTool *node_tool, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_custom_property_dialog(CongPlugin *plugin, void (*callback)(CongServiceNodePropertyDialog *custom_property_dialog, gpointer user_data), gpointer user_data);



gchar* cong_plugin_get_gconf_namespace(CongPlugin *plugin);

/**
 * cong_plugin_get_gconf_key
 * @plugin:
 * @local_part:
 * 
 * Convert a "local" GConf key for this plugin to a GConf key with a full-path.
 * e.g. converts "enable-fubar" to "/apps/conglomerate/plugins/docbook/enable-fubar"
 *  
 * Caller must delete returned string.
 * 
 * Returns:
 */
gchar* cong_plugin_get_gconf_key(CongPlugin *plugin, const gchar *local_part);


const gchar* cong_service_get_name(CongService *functionality);
const gchar* cong_service_get_description(CongService *functionality);

gchar* cong_service_get_gconf_namespace(CongService* functionality);

/**
 * cong_plugin_service_get_gconf_key
 * @service:
 * @local_part:
 * 
 * Convert a "local" GConf key for this plugin to a GConf key with a full-path.
 * e.g. converts "enable-fubar" to "/apps/conglomerate/plugins/docbook/enable-fubar"
 *   
 * Caller must delete returned string.
 * 
 * Returns:
 */
gchar* cong_service_get_gconf_key(CongService *functionality, const gchar *local_part);

void cong_document_factory_invoke_page_creation_callback(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant);
void cong_document_factory_invoke_action_callback(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant);
GdkPixbuf *cong_document_factory_get_icon(CongServiceDocumentFactory *factory);

gboolean cong_importer_supports_mime_type(CongServiceImporter *importer, const gchar *mime_type);
void cong_importer_invoke(CongServiceImporter *importer, const gchar *filename, const gchar *mime_type, GtkWindow *toplevel_window);

gboolean cong_exporter_supports_document(CongServiceExporter *exporter, CongDocument *doc);
void cong_exporter_invoke(CongServiceExporter *exporter, CongDocument *doc, const gchar *uri, GtkWindow *toplevel_window);
gchar *cong_exporter_get_preferred_uri(CongServiceExporter *exporter);
void cong_exporter_set_preferred_uri(CongServiceExporter *exporter, const gchar *uri);

#if ENABLE_PRINTING
gboolean cong_print_method_supports_document(CongServicePrintMethod *print_method, CongDocument *doc);
void cong_print_method_invoke(CongServicePrintMethod *print_method, CongDocument *doc, GnomePrintContext *gpc, GtkWindow *toplevel_window);
#endif

const gchar*
cong_tool_get_menu_text(CongServiceTool *tool);

const gchar*
cong_tool_get_tip_text(CongServiceTool *tool);

const gchar*
cong_tool_get_tip_further_text(CongServiceTool *tool);

gboolean 
cong_doc_tool_supports_document (CongServiceDocTool *doc_tool, 
				 CongDocument *doc);
void 
cong_doc_tool_invoke (CongServiceDocTool *doc_tool, 
		      CongPrimaryWindow *primary_window);

gboolean 
cong_node_tool_supports_node (CongServiceNodeTool *node_tool, 
			      CongDocument *doc,
			      CongNodePtr node);
void 
cong_node_tool_invoke (CongServiceNodeTool *node_tool, 
		       CongPrimaryWindow *primary_window,
		       CongNodePtr node);

/* Helpful functions for implementing plugins; the paren_window arg is used in case we need to pop up an error dialog: */
CongDocument* 
cong_ui_new_document_from_manufactured_xml(xmlDocPtr xml_doc,
					   GtkWindow *parent_window);
CongDocument*
cong_ui_new_document_from_imported_xml(xmlDocPtr xml_doc,
				       GtkWindow *parent_window);

xmlDocPtr cong_ui_transform_doc(CongDocument *doc,
				const gchar *stylesheet_filename,
				GtkWindow *toplevel_window);

void cong_ui_transform_doc_to_uri(CongDocument *doc,
				  const gchar *stylesheet_filename,
				  const gchar *uri,
				  GtkWindow *toplevel_window);

gboolean cong_ui_load_imported_file_content(const gchar *uri,
					    char** buffer,
					    GnomeVFSFileSize* size,
					    GtkWindow *parent_window);

void cong_ui_append_advanced_node_properties_page(GtkNotebook *notebook,
						  CongDocument *doc, 
						  CongNodePtr node);

/* The DocumentFactory objects all create pages within one big Druid; the booleans provide hints to make
   navigation easier */
GnomeDruidPageStandard *cong_new_file_assistant_new_page(CongNewFileAssistant *assistant, 
							 CongServiceDocumentFactory *document_factory, 
							 gboolean is_first_of_factory,
							 gboolean is_last_of_factory);
void cong_new_file_assistant_set_page(CongNewFileAssistant *assistant, GnomeDruidPage *page);

/* Method to get toplevel window of the assistant; useful when displaying error dialogs */
GtkWindow *cong_new_file_assistant_get_toplevel(CongNewFileAssistant *assistant);

CongEditorNodeElement*
cong_plugin_editor_node_factory_invoke (CongServiceEditorNodeFactory *plugin_editor_node_factory,
					CongEditorWidget3 *editor_widget, 
					CongTraversalNode *traversal_node);


GtkWidget *cong_custom_property_dialog_make(CongServiceNodePropertyDialog *custom_property_dialog,
					    CongDocument *doc,
					    CongNodePtr node);
G_END_DECLS

#endif
