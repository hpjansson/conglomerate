/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-plugin-manager.h
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

#ifndef __CONG_PLUGIN_MANAGER_H__
#define __CONG_PLUGIN_MANAGER_H__

#include "cong-plugin.h"

G_BEGIN_DECLS

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

void
cong_plugin_manager_for_each_service (CongPluginManager *plugin_manager, 
				      void 
				      (*callback) (CongService *service,
						   gpointer user_data),
				      gpointer user_data);

void
cong_plugin_manager_for_each_service_of_type (CongPluginManager *plugin_manager, 
					      GType type,
					      void 
					      (*callback) (CongService *service,
							   gpointer user_data),
					      gpointer user_data);

CongService*
cong_plugin_manager_locate_service_by_id (CongPluginManager *plugin_manager, 
					  GType type,
					  const gchar *service_id);

void
cong_plugin_manager_for_each_service_of_type (CongPluginManager *plugin_manager, 
					      GType type,
					      void 
					      (*callback) (CongService *service,
							   gpointer user_data),
					      gpointer user_data);

void cong_plugin_manager_for_each_document_factory(CongPluginManager *plugin_manager, void (*callback)(CongServiceDocumentFactory *factory, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_importer(CongPluginManager *plugin_manager, void (*callback)(CongServiceImporter *importer, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_exporter(CongPluginManager *plugin_manager, void (*callback)(CongServiceExporter *exporter, gpointer user_data), gpointer user_data);
#if ENABLE_PRINTING
void cong_plugin_manager_for_each_print_method (CongPluginManager *plugin_manager, 
						void 
						(*callback) (CongServicePrintMethod *print_method, 
							     gpointer user_data), 
						gpointer user_data);
#endif
/*void cong_plugin_manager_for_each_thumbnailer(CongPluginManager *plugin_manager, void (*callback)(CongServiceThumbnailer *thumbnailer, gpointer user_data), gpointer user_data);*/
void cong_plugin_manager_for_each_doc_tool(CongPluginManager *plugin_manager, void (*callback)(CongServiceDocTool *doc_tool, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_node_tool(CongPluginManager *plugin_manager, void (*callback)(CongServiceNodeTool *node_tool, gpointer user_data), gpointer user_data);
void cong_plugin_manager_for_each_custom_property_dialog(CongPluginManager *plugin_manager, void (*callback)(CongServiceNodePropertyDialog *custom_property_dialog, gpointer user_data), gpointer user_data);

CongServiceNodePropertyDialog*
cong_plugin_manager_locate_custom_property_dialog_by_id (CongPluginManager *plugin_manager, 
							 const gchar *plugin_id);

CongServiceEditorNodeFactory*
cong_plugin_manager_locate_editor_node_factory_by_id (CongPluginManager *plugin_manager,
						      const gchar *plugin_id);



G_END_DECLS

#endif



