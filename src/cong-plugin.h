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

#include "cong-object.h"

#include "cong-service-doc-tool.h"
#include "cong-service-document-factory.h"
#include "cong-service-editor-node-factory.h"
#include "cong-service-exporter.h"
#include "cong-service-importer.h"
#include "cong-service-node-property-dialog.h"
#include "cong-service-node-tool.h"
#include "cong-service-print-method.h"
#include "cong-service-tool.h"

G_BEGIN_DECLS

#define CONG_PLUGIN_TYPE	  (cong_plugin_get_type ())
#define CONG_PLUGIN(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_PLUGIN_TYPE, CongPlugin)
#define CONG_PLUGIN_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_PLUGIN_TYPE, CongPluginClass)
#define IS_CONG_PLUGIN(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_PLUGIN_TYPE)

CONG_DECLARE_CLASS (CongPlugin, cong_plugin, GObject)

/* typedef struct CongServiceThumbnailer CongServiceThumbnailer; */


/* Function pointers to be exposed by .so/.dll files: */
typedef gboolean 
(*CongPluginCallbackInit)(CongPlugin *plugin); /* exposed as "plugin_init"? */

typedef gboolean 
(*CongPluginCallbackUninit)(CongPlugin *plugin); /* exposed as "plugin_uninit"? */

typedef gboolean 
(*CongPluginCallbackRegister)(CongPlugin *plugin); /* exposed as "plugin_register"? */

typedef gboolean 
(*CongPluginCallbackConfigure)(CongPlugin *plugin);  /* exposed as "plugin_configure"? legitimate for it not to be present */


/* 
   CongPlugin 

   These are manufactured by the CongPluginManager and passed to the registration/unregistration hooks exposed by the .so/.dll files.

   There are various methods to allow plugins to register their service with the app.
*/
CongPlugin*
cong_plugin_construct (CongPlugin *plugin,
		       const gchar *plugin_id,
		       CongPluginCallbackRegister register_callback,
		       CongPluginCallbackConfigure configure_callback);

void
cong_plugin_add_service (CongPlugin *plugin,
			 CongService *service);

void
cong_plugin_for_each_service (CongPlugin *plugin, 
			      void 
			      (*callback) (CongService *service,
					   gpointer user_data),
			      gpointer user_data);

void
cong_plugin_for_each_service_of_type (CongPlugin *plugin, 
				      GType type,
				      void 
				      (*callback) (CongService *service,
						   gpointer user_data),
				      gpointer user_data);

CongService*
cong_plugin_locate_service_by_id (CongPlugin *plugin, 
				  GType type,
				  const gchar *service_id);

CongServiceNodePropertyDialog*
cong_plugin_locate_custom_property_dialog_by_id (CongPlugin *plugin, 
						 const gchar *service_id);
CongServiceEditorNodeFactory*
cong_plugin_locate_editor_node_factory_by_id (CongPlugin *plugin,
					      const gchar *service_id);



/* void cong_plugin_for_each_thumbnailer(CongPlugin *plugin, void (*callback)(CongServiceThumbnailer *thumbnailer, gpointer user_data), gpointer user_data); */

const gchar* 
cong_plugin_get_id (CongPlugin *plugin);

gchar* 
cong_plugin_get_gconf_namespace (CongPlugin *plugin);

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
gchar* 
cong_plugin_get_gconf_key (CongPlugin *plugin, 
			   const gchar *local_part);



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


G_END_DECLS

#endif
