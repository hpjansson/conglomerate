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

#include "cong-editor-widget-impl.h"
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
typedef struct CongPluginEditorElement CongPluginEditorElement;
typedef struct CongPluginEditorNodeFactory CongPluginEditorNodeFactory;
typedef struct CongTool CongTool;
typedef struct CongCustomPropertyDialog CongCustomPropertyDialog;

/* The File->New GUI: */
typedef struct CongNewFileAssistant CongNewFileAssistant;


/* Function pointers to be exposed by .so/.dll files: */
typedef gboolean (*CongPluginCallbackInit)(CongPlugin *plugin); /* exposed as "plugin_init"? */
typedef gboolean (*CongPluginCallbackUninit)(CongPlugin *plugin); /* exposed as "plugin_uninit"? */
typedef gboolean (*CongPluginCallbackRegister)(CongPlugin *plugin); /* exposed as "plugin_register"? */
typedef gboolean (*CongPluginCallbackConfigure)(CongPlugin *plugin);  /* exposed as "plugin_configure"? legitimate for it not to be present */

/* Function pointers that are registered by plugins: */
typedef void (*CongDocumentFactoryPageCreationCallback)(CongDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data);
typedef void (*CongDocumentFactoryActionCallback)(CongDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data);
typedef gboolean (*CongImporterMimeFilter)(CongImporter *importer, const gchar *mime_type, gpointer user_data);
typedef void (*CongImporterActionCallback)(CongImporter *importer, const gchar *uri, const gchar *mime_type, gpointer user_data, GtkWindow *toplevel_window);
typedef gboolean (*CongExporterDocumentFilter)(CongExporter *exporter, CongDocument *doc, gpointer user_data);
typedef void (*CongExporterActionCallback)(CongExporter *exporter, CongDocument *doc, const gchar *uri, gpointer user_data, GtkWindow *toplevel_window);
typedef gboolean (*CongToolDocumentFilter)(CongTool *tool, CongDocument *doc, gpointer user_data);
typedef void (*CongToolActionCallback)(CongTool *tool, CongPrimaryWindow *primary_window, gpointer user_data);
typedef GtkWidget* (*CongCustomPropertyFactoryMethod)(CongCustomPropertyDialog *custom_property_dialog, CongDocument *doc, CongNodePtr node);

#if ENABLE_PRINTING
typedef gboolean (*CongPrintMethodDocumentFilter)(CongPrintMethod *print_method, CongDocument *doc, gpointer user_data);
typedef void (*CongPrintMethodActionCallback)(CongPrintMethod *print_method, CongDocument *doc, GnomePrintContext *gpc, gpointer user_data, GtkWindow *toplevel_window);
#endif

typedef CongElementEditor* (*CongEditorElementFactoryMethod)(CongPluginEditorElement *plugin_editor_element, CongEditorWidget2 *editor_widget, CongNodePtr node, gpointer user_data);
typedef CongEditorNodeElement* (*CongEditorNodeFactoryMethod)(CongPluginEditorNodeFactory *plugin_editor_node_factory, CongEditorWidget3 *editor_widget, CongNodePtr node, CongEditorNode *traversal_parent, gpointer user_data);

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
void cong_plugin_manager_for_each_tool(CongPluginManager *plugin_manager, void (*callback)(CongTool *tool, gpointer user_data), gpointer user_data);
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
CongPluginEditorElement *cong_plugin_register_editor_element(CongPlugin *plugin, 
							     const gchar *name, 
							     const gchar *description,
							     const gchar *plugin_id,
							     CongEditorElementFactoryMethod factory_method,
							     gpointer user_data);
CongPluginEditorNodeFactory *cong_plugin_register_editor_node_factory(CongPlugin *plugin, 
								      const gchar *name, 
								      const gchar *description,
								      const gchar *plugin_id,
								      CongEditorNodeFactoryMethod factory_method,
								      gpointer user_data);
CongTool *cong_plugin_register_tool(CongPlugin *plugin,
				    const gchar *name, 
				    const gchar *description,
				    const gchar *functionality_id,
				    const gchar *menu_text,
				    const gchar *tooltip_text,
				    const gchar *tooltip_further_text,
				    CongToolDocumentFilter doc_filter,
				    CongToolActionCallback action_callback,
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
void cong_plugin_for_each_tool(CongPlugin *plugin, void (*callback)(CongTool *tool, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_custom_property_dialog(CongPlugin *plugin, void (*callback)(CongCustomPropertyDialog *custom_property_dialog, gpointer user_data), gpointer user_data);


gchar* cong_plugin_get_gconf_namespace(CongPlugin *plugin);

/**
   Convert a "local" GConf key for this plugin to a GConf key with a full-path.
   e.g. converts "enable-fubar" to "/apps/conglomerate/plugins/docbook/enable-fubar"
   
   Caller must delete returned string.
 */
gchar* cong_plugin_get_gconf_key(CongPlugin *plugin, const gchar *local_part);


const gchar* cong_functionality_get_name(CongFunctionality *functionality);
const gchar* cong_functionality_get_description(CongFunctionality *functionality);

gchar* cong_functionality_get_gconf_namespace(CongFunctionality* functionality);

/**
   Convert a "local" GConf key for this plugin to a GConf key with a full-path.
   e.g. converts "enable-fubar" to "/apps/conglomerate/plugins/docbook/enable-fubar"
   
   Caller must delete returned string.
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
void cong_exporter_set_preferred_ui(CongExporter *exporter, const gchar *uri);

#if ENABLE_PRINTING
gboolean cong_print_method_supports_document(CongPrintMethod *print_method, CongDocument *doc);
void cong_print_method_invoke(CongPrintMethod *print_method, CongDocument *doc, GnomePrintContext *gpc, GtkWindow *toplevel_window);
#endif

gboolean cong_tool_supports_document(CongTool *tool, CongDocument *doc);
void cong_tool_invoke(CongTool *tool, CongPrimaryWindow *primary_window);
const gchar *cong_tool_get_menu_text(CongTool *tool);
const gchar *cong_tool_get_tip_text(CongTool *tool);
const gchar *cong_tool_get_tip_further_text(CongTool *tool);

/* Helpful functions for implementing plugins; the paren_window arg is used in case we need to pop up an error dialog: */
void cong_ui_new_document_from_manufactured_xml(xmlDocPtr xml_doc,
						GtkWindow *parent_window);
void cong_ui_new_document_from_imported_xml(xmlDocPtr xml_doc,
					    GtkWindow *parent_window);

xmlDocPtr cong_ui_parse_buffer(const char* buffer, 
			       GnomeVFSFileSize size, 
			       GnomeVFSURI *file_uri, 
			       GtkWindow *parent_window);

xmlDocPtr cong_ui_transform_doc(CongDocument *doc,
				const gchar *stylesheet_filename,
				GtkWindow *toplevel_window);

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

CongElementEditor *cong_plugin_element_editor_new(CongEditorWidget2 *editor_widget, 
						  CongNodePtr node, 
						  CongDispspecElement *element);

CongEditorNodeElement*
cong_plugin_editor_node_factory_invoke (CongPluginEditorNodeFactory *plugin_editor_node_factory,
					CongEditorWidget3 *editor_widget, 
					CongNodePtr node,
					CongEditorNode *traversal_parent);


GtkWidget *cong_custom_property_dialog_make(CongCustomPropertyDialog *custom_property_dialog,
					    CongDocument *doc,
					    CongNodePtr node);


G_END_DECLS

#endif
