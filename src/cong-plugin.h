/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-plugin.h
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

typedef struct CongFunctionality CongFunctionality;
#define CONG_FUNCTIONALITY(x) ((CongFunctionality*)(x))

/* The following are all castable to CongFunctionality: */
typedef struct CongDocumentFactory CongDocumentFactory;
typedef struct CongImporter CongImporter;
typedef struct CongExporter CongExporter;
#if ENABLE_PRINTING
typedef struct CongPrintMethod CongPrintMethod;
#endif
typedef struct CongThumbnailer CongThumbnailer;
typedef struct CongPluginEditorNodeFactory CongPluginEditorNodeFactory;

typedef struct CongTool CongTool;
#define CONG_TOOL(x) ((CongTool*)(x))
typedef struct CongDocTool CongDocTool;
typedef struct CongNodeTool CongNodeTool;

typedef struct CongCustomPropertyDialog CongCustomPropertyDialog;

/* The File->New GUI: */
typedef struct CongNewFileAssistant CongNewFileAssistant;


/* Function pointers to be exposed by .so/.dll files: */
typedef gboolean (*CongPluginCallbackInit)(CongPlugin *plugin); /* exposed as "plugin_init"? */
typedef gboolean (*CongPluginCallbackUninit)(CongPlugin *plugin); /* exposed as "plugin_uninit"? */
typedef gboolean (*CongPluginCallbackRegister)(CongPlugin *plugin); /* exposed as "plugin_register"? */
typedef gboolean (*CongPluginCallbackConfigure)(CongPlugin *plugin);  /* exposed as "plugin_configure"? legitimate for it not to be present */

/* Function pointers that are registered by plugins: */
typedef void 
(*CongDocumentFactoryPageCreationCallback) (CongDocumentFactory *factory, 
					    CongNewFileAssistant *assistant, 
					    gpointer user_data);

typedef void 
(*CongDocumentFactoryActionCallback) (CongDocumentFactory *factory, 
				      CongNewFileAssistant *assistant, 
				      gpointer user_data);

typedef gboolean
(*CongImporterMimeFilter) (CongImporter *importer, 
			   const gchar *mime_type, 
			   gpointer user_data);
typedef void 
(*CongImporterActionCallback) (CongImporter *importer, 
			       const gchar *uri, 
			       const gchar *mime_type, 
			       gpointer user_data, 
			       GtkWindow *toplevel_window);

typedef gboolean 
(*CongExporterDocumentFilter) (CongExporter *exporter, 
			       CongDocument *doc, 
			       gpointer user_data);

typedef void 
(*CongExporterActionCallback) (CongExporter *exporter, 
			       CongDocument *doc, 
			       const gchar *uri, 
			       gpointer user_data, 
			       GtkWindow *toplevel_window);

typedef gboolean 
(*CongDocToolFilter) (CongDocTool *doc_tool, 
		      CongDocument *doc, 
		      gpointer user_data);
typedef void 
(*CongDocToolActionCallback) (CongDocTool *doc_tool, 
			      CongPrimaryWindow *primary_window, 
			      gpointer user_data);
typedef gboolean 
(*CongNodeToolFilter) (CongNodeTool *node_tool, 
		       CongDocument *doc, 
		       CongNodePtr node,
		       gpointer user_data);
typedef void 
(*CongNodeToolActionCallback) (CongNodeTool *tool, 
			       CongPrimaryWindow *primary_window, 
			       CongNodePtr node,
			       gpointer user_data);

typedef GtkWidget* 
(*CongCustomPropertyFactoryMethod) (CongCustomPropertyDialog *custom_property_dialog, 
				    CongDocument *doc, 
				    CongNodePtr node);

#if ENABLE_PRINTING
typedef gboolean 
(*CongPrintMethodDocumentFilter) (CongPrintMethod *print_method, 
				  CongDocument *doc, 
				  gpointer user_data);

typedef void 
(*CongPrintMethodActionCallback) (CongPrintMethod *print_method, 
				  CongDocument *doc, 
				  GnomePrintContext *gpc, 
				  gpointer user_data, 
				  GtkWindow *toplevel_window);
#endif

typedef CongEditorNodeElement* 
(*CongEditorNodeFactoryMethod) (CongPluginEditorNodeFactory *plugin_editor_node_factory, 
				CongEditorWidget3 *editor_widget, 
				CongTraversalNode *traversal_node, 
				gpointer user_data);

/* 
   CongPluginManager
*/
CongPluginManager *cong_plugin_manager_new(void);
CongPlugin *cong_plugin_manager_register(CongPluginManager *plugin_manager, 
					 const gchar *id,
					 CongPluginCallbackRegister register_callback,
					 CongPluginCallbackConfigure configure_callback);
void cong_plugin_manager_unregister(CongPluginManager *plugin_manager, CongPlugin *plugin);
void cong_plugin_manager_for_each_plugin(CongPluginManager *plugin_manager, void (*callback)(CongPlugin *plugin, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_document_factory(CongPluginManager *plugin_manager, void (*callback)(CongDocumentFactory *factory, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_importer(CongPluginManager *plugin_manager, void (*callback)(CongImporter *importer, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_exporter(CongPluginManager *plugin_manager, void (*callback)(CongExporter *exporter, gpointer user_data), gpointer user_data);
#if ENABLE_PRINTING
void cong_plugin_manager_for_each_printmethod(CongPluginManager *plugin_manager, void (*callback)(CongPrintMethod *print_method, gpointer user_data), gpointer user_data);
#endif
void cong_plugin_manager_for_each_thumbnailer(CongPluginManager *plugin_manager, void (*callback)(CongThumbnailer *thumbnailer, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_doc_tool(CongPluginManager *plugin_manager, void (*callback)(CongDocTool *doc_tool, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_node_tool(CongPluginManager *plugin_manager, void (*callback)(CongNodeTool *node_tool, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_custom_property_dialog(CongPluginManager *plugin_manager, void (*callback)(CongCustomPropertyDialog *custom_property_dialog, gpointer user_data), gpointer user_data);

CongCustomPropertyDialog*
cong_plugin_manager_locate_custom_property_dialog_by_id (CongPluginManager *plugin_manager, 
							 const gchar *plugin_id);

CongPluginEditorNodeFactory*
cong_plugin_manager_locate_editor_node_factory_by_id (CongPluginManager *plugin_manager,
						      const gchar *plugin_id);


/* 
   CongPlugin 

   These are manufactured by the CongPluginManager and passed to the registration/unregistration hooks exposed by the .so/.dll files.

   There are various methods to allow plugins to register their functionality with the app.
*/
CongDocumentFactory *cong_plugin_register_document_factory(CongPlugin *plugin, 
							   const gchar *name, 
							   const gchar *description,
							   const gchar *id,
							   CongDocumentFactoryPageCreationCallback page_creation_callback,
							   CongDocumentFactoryActionCallback action_callback,
							   const gchar *icon,
							   gpointer user_data);
CongImporter *cong_plugin_register_importer(CongPlugin *plugin, 
					    const gchar *name, 
					    const gchar *description,
					    const gchar *id,
					    CongImporterMimeFilter mime_filter,
					    CongImporterActionCallback action_callback,
					    gpointer user_data);
CongExporter *cong_plugin_register_exporter(CongPlugin *plugin, 
					    const gchar *name, 
					    const gchar *description,
					    const gchar *id,
					    CongExporterDocumentFilter doc_filter,
					    CongExporterActionCallback action_callback,
					    gpointer user_data);
#if ENABLE_PRINTING
CongPrintMethod *cong_plugin_register_print_method(CongPlugin *plugin, 
					    const gchar *name, 
					    const gchar *description,
					    const gchar *id,
					    CongPrintMethodDocumentFilter doc_filter,
					    CongPrintMethodActionCallback action_callback,
					    gpointer user_data);
#endif
CongPluginEditorNodeFactory *cong_plugin_register_editor_node_factory(CongPlugin *plugin, 
								      const gchar *name, 
								      const gchar *description,
								      const gchar *plugin_id,
								      CongEditorNodeFactoryMethod factory_method,
								      gpointer user_data);
CongDocTool*
cong_plugin_register_doc_tool(CongPlugin *plugin,
			      const gchar *name, 
			      const gchar *description,
			      const gchar *functionality_id,
			      const gchar *menu_text,
			      const gchar *tooltip_text,
			      const gchar *tooltip_further_text,
			      CongDocToolFilter doc_filter,
			      CongDocToolActionCallback action_callback,
			      gpointer user_data);

CongNodeTool*
cong_plugin_register_node_tool (CongPlugin *plugin,
				const gchar *name, 
				const gchar *description,
				const gchar *functionality_id,
				const gchar *menu_text,
				const gchar *tooltip_text,
				const gchar *tooltip_further_text,
				CongNodeToolFilter node_filter,
				CongNodeToolActionCallback action_callback,
				gpointer user_data);

CongCustomPropertyDialog *cong_plugin_register_custom_property_dialog(CongPlugin *plugin,
								      const gchar *name, 
								      const gchar *description,
								      const gchar *functionality_id,
								      CongCustomPropertyFactoryMethod factory_method,
								      gpointer user_data);

void cong_plugin_for_each_document_factory(CongPlugin *plugin, void (*callback)(CongDocumentFactory *factory, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_importer(CongPlugin *plugin, void (*callback)(CongImporter *importer, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_exporter(CongPlugin *plugin, void (*callback)(CongExporter *exporter, gpointer user_data), gpointer user_data);
#if ENABLE_PRINTING
void cong_plugin_for_each_print_method(CongPlugin *plugin, void (*callback)(CongPrintMethod *print_method, gpointer user_data), gpointer user_data);
#endif
void cong_plugin_for_each_thumbnailer(CongPlugin *plugin, void (*callback)(CongThumbnailer *thumbnailer, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_doc_tool(CongPlugin *plugin, void (*callback)(CongDocTool *doc_tool, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_node_tool(CongPlugin *plugin, void (*callback)(CongNodeTool *node_tool, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_custom_property_dialog(CongPlugin *plugin, void (*callback)(CongCustomPropertyDialog *custom_property_dialog, gpointer user_data), gpointer user_data);



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


const gchar* cong_functionality_get_name(CongFunctionality *functionality);
const gchar* cong_functionality_get_description(CongFunctionality *functionality);

gchar* cong_functionality_get_gconf_namespace(CongFunctionality* functionality);

/**
 * cong_plugin_functionality_get_gconf_key
 * @functionality:
 * @local_part:
 * 
 * Convert a "local" GConf key for this plugin to a GConf key with a full-path.
 * e.g. converts "enable-fubar" to "/apps/conglomerate/plugins/docbook/enable-fubar"
 *   
 * Caller must delete returned string.
 * 
 * Returns:
 */
gchar* cong_functionality_get_gconf_key(CongFunctionality *functionality, const gchar *local_part);

void cong_document_factory_invoke_page_creation_callback(CongDocumentFactory *factory, CongNewFileAssistant *assistant);
void cong_document_factory_invoke_action_callback(CongDocumentFactory *factory, CongNewFileAssistant *assistant);
GdkPixbuf *cong_document_factory_get_icon(CongDocumentFactory *factory);

gboolean cong_importer_supports_mime_type(CongImporter *importer, const gchar *mime_type);
void cong_importer_invoke(CongImporter *importer, const gchar *filename, const gchar *mime_type, GtkWindow *toplevel_window);

gboolean cong_exporter_supports_document(CongExporter *exporter, CongDocument *doc);
void cong_exporter_invoke(CongExporter *exporter, CongDocument *doc, const gchar *uri, GtkWindow *toplevel_window);
gchar *cong_exporter_get_preferred_uri(CongExporter *exporter);
void cong_exporter_set_preferred_uri(CongExporter *exporter, const gchar *uri);

#if ENABLE_PRINTING
gboolean cong_print_method_supports_document(CongPrintMethod *print_method, CongDocument *doc);
void cong_print_method_invoke(CongPrintMethod *print_method, CongDocument *doc, GnomePrintContext *gpc, GtkWindow *toplevel_window);
#endif

const gchar*
cong_tool_get_menu_text(CongTool *tool);

const gchar*
cong_tool_get_tip_text(CongTool *tool);

const gchar*
cong_tool_get_tip_further_text(CongTool *tool);

gboolean 
cong_doc_tool_supports_document (CongDocTool *doc_tool, 
				 CongDocument *doc);
void 
cong_doc_tool_invoke (CongDocTool *doc_tool, 
		      CongPrimaryWindow *primary_window);

gboolean 
cong_node_tool_supports_node (CongNodeTool *node_tool, 
			      CongDocument *doc,
			      CongNodePtr node);
void 
cong_node_tool_invoke (CongNodeTool *node_tool, 
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
							 CongDocumentFactory *document_factory, 
							 gboolean is_first_of_factory,
							 gboolean is_last_of_factory);
void cong_new_file_assistant_set_page(CongNewFileAssistant *assistant, GnomeDruidPage *page);

/* Method to get toplevel window of the assistant; useful when displaying error dialogs */
GtkWindow *cong_new_file_assistant_get_toplevel(CongNewFileAssistant *assistant);

CongEditorNodeElement*
cong_plugin_editor_node_factory_invoke (CongPluginEditorNodeFactory *plugin_editor_node_factory,
					CongEditorWidget3 *editor_widget, 
					CongTraversalNode *traversal_node);


GtkWidget *cong_custom_property_dialog_make(CongCustomPropertyDialog *custom_property_dialog,
					    CongDocument *doc,
					    CongNodePtr node);
G_END_DECLS

#endif
