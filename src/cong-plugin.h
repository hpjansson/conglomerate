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

G_BEGIN_DECLS


/* 
   CongPluginManager
*/
CongPluginManager *cong_plugin_manager_new(void);
CongPlugin *cong_plugin_manager_register(CongPluginManager *plugin_manager, 
					 CongPluginCallbackRegister register_callback,
					 CongPluginCallbackConfigure configure_callback);
void cong_plugin_manager_unregister(CongPluginManager *plugin_manager, CongPlugin *plugin);
void cong_plugin_manager_for_each_plugin(CongPluginManager *plugin_manager, void (*callback)(CongPlugin *plugin, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_document_factory(CongPluginManager *plugin_manager, void (*callback)(CongDocumentFactory *factory, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_importer(CongPluginManager *plugin_manager, void (*callback)(CongImporter *importer, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_exporter(CongPluginManager *plugin_manager, void (*callback)(CongExporter *exporter, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_printmethod(CongPluginManager *plugin_manager, void (*callback)(CongPrintMethod *print_method, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_thumbnailer(CongPluginManager *plugin_manager, void (*callback)(CongThumbnailer *thumbnailer, gpointer user_data), gpointer user_data);

/* 
   CongPlugin 

   These are manufactured by the CongPluginManager and passed to the registration/unregistration hooks exposed by the .so/.dll files.

   There are various methods to allow plugins to register their functionality with the app.
*/
CongDocumentFactory *cong_plugin_register_document_factory(CongPlugin *plugin, 
							   const gchar *name, 
							   const gchar *description,
							   CongDocumentFactoryPageCreationCallback page_creation_callback,
							   CongDocumentFactoryActionCallback action_callback,
							   gpointer user_data);
CongImporter *cong_plugin_register_importer(CongPlugin *plugin, 
					    const gchar *name, 
					    const gchar *description,
					    CongImporterMimeFilter mime_filter,
					    CongImporterActionCallback action_callback,
					    gpointer user_data);
CongExporter *cong_plugin_register_exporter(CongPlugin *plugin, 
					    const gchar *name, 
					    const gchar *description,
					    CongExporterFpiFilter fip_filter,
					    CongExporterActionCallback action_callback,
					    gpointer user_data);

void cong_plugin_for_each_document_factory(CongPlugin *plugin, void (*callback)(CongDocumentFactory *factory, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_importer(CongPlugin *plugin, void (*callback)(CongImporter *importer, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_exporter(CongPlugin *plugin, void (*callback)(CongExporter *exporter, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_printmethod(CongPlugin *plugin, void (*callback)(CongPrintMethod *print_method, gpointer user_data), gpointer user_data);
void cong_plugin_for_each_thumbnailer(CongPlugin *plugin, void (*callback)(CongThumbnailer *thumbnailer, gpointer user_data), gpointer user_data);


const gchar* cong_functionality_get_name(CongFunctionality *functionality);
const gchar* cong_functionality_get_description(CongFunctionality *functionality);

void cong_document_factory_invoke_page_creation_callback(CongDocumentFactory *factory, CongNewFileAssistant *assistant);
void cong_document_factory_invoke_action_callback(CongDocumentFactory *factory, CongNewFileAssistant *assistant);

gboolean cong_importer_supports_mime_type(CongImporter *importer, const gchar *mime_type);
void cong_importer_invoke(CongImporter *importer, const gchar *filename, const gchar *mime_type);

/* Helpful functions for implementing plugins: */
void cong_ui_new_document_from_manufactured_xml(xmlDocPtr xml_doc);
void cong_ui_new_document_from_imported_xml(xmlDocPtr xml_doc);

/* The DocumentFactory objects all create pages within one big Druid; the booleans provide hints to make
   navigation easier */
GnomeDruidPageStandard *cong_new_file_assistant_new_page(CongNewFileAssistant *assistant, 
							 CongDocumentFactory *document_factory, 
							 gboolean is_first_of_factory,
							 gboolean is_last_of_factory);
void cong_new_file_assistant_set_page(CongNewFileAssistant *assistant, GnomeDruidPage *page);


/* Plugins at the moment are all compiled into the app; here are the symbols that would be dynamically extracted: */
/* plugin-docbook.c: */
gboolean plugin_docbook_plugin_register(CongPlugin *plugin);
gboolean plugin_docbook_plugin_configure(CongPlugin *plugin);

/* more plugins please! */


G_END_DECLS

#endif
