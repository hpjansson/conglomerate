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

/* Private definitions of the types: */
struct CongPluginManager
{
	GList *list_of_plugin; /* ptrs of type CongPlugin */
};

struct CongPlugin
{
	gchar *name;
	gchar *description;

	CongPluginCallbackConfigure configure_callback;

	GList *list_of_document_factory; /* ptrs of type CongDocumentFactory */
	GList *list_of_importer; /* ptrs of type CongImporter */
	GList *list_of_exporter; /* ptrs of type CongExporter */
	GList *list_of_print_method; /* ptrs of type CongPrintMethod */
	GList *list_of_thumbnailer; /* ptrs of type CongThumnbailer */
};

struct CongFunctionality
{
	gchar *name;
	gchar *description;
};

struct CongDocumentFactory
{
	CongFunctionality functionality; /* base class */

	CongDocumentFactoryPageCreationCallback page_creation_callback;
	CongDocumentFactoryActionCallback action_callback;
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

	CongExporterFpiFilter fpi_filter;
	CongExporterActionCallback action_callback;
	gpointer user_data;
};

struct CongPrintMethod
{
	CongFunctionality functionality; /* base class */
};

struct CongThumbnailer
{
	CongFunctionality functionality; /* base class */
};


/* Implementation of CongPluginManager: */
CongPluginManager *cong_plugin_manager_new(void)
{
	CongPluginManager *manager;

	manager = g_new0(CongPluginManager,1);
	
	return manager;
}

CongPlugin *cong_plugin_manager_register(CongPluginManager *plugin_manager, 
					 CongPluginCallbackRegister register_callback,
					 CongPluginCallbackConfigure configure_callback)
{
	CongPlugin *plugin;

	g_return_val_if_fail(plugin_manager, NULL);
	g_return_val_if_fail(register_callback, NULL);
	/* the configure callback is allowed to be NULL */

	plugin = g_new0(CongPlugin,1);

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


/* Implementation of CongPlugin: */
CongDocumentFactory *cong_plugin_register_document_factory(CongPlugin *plugin, 
							   const gchar *name, 
							   const gchar *description,
							   CongDocumentFactoryPageCreationCallback page_creation_callback,
							   CongDocumentFactoryActionCallback action_callback,
							   gpointer user_data)
{
	CongDocumentFactory *factory;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(page_creation_callback, NULL);
	g_return_val_if_fail(action_callback, NULL);

	factory = g_new0(CongDocumentFactory,1);

	factory->functionality.name = g_strdup(name);
	factory->functionality.description = g_strdup(description);
	factory->page_creation_callback = page_creation_callback;
	factory->action_callback = action_callback;
	factory->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_document_factory = g_list_append(plugin->list_of_document_factory, factory);

	return factory;
}

CongImporter *cong_plugin_register_importer(CongPlugin *plugin, 
					    const gchar *name, 
					    const gchar *description,
					    CongImporterMimeFilter mime_filter,
					    CongImporterActionCallback action_callback,
					    gpointer user_data)
{
	CongImporter *importer;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(mime_filter, NULL);
	g_return_val_if_fail(action_callback, NULL);

        importer = g_new0(CongImporter,1);

	importer->functionality.name = g_strdup(name);
	importer->functionality.description = g_strdup(description);
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
					    CongExporterFpiFilter fpi_filter,
					    CongExporterActionCallback action_callback,
					    gpointer user_data)
{
	CongExporter *exporter;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(fpi_filter, NULL);
	g_return_val_if_fail(action_callback, NULL);

        exporter = g_new0(CongExporter,1);

	exporter->functionality.name = g_strdup(name);
	exporter->functionality.description = g_strdup(description);
	exporter->fpi_filter = fpi_filter;
	exporter->action_callback = action_callback;
	exporter->user_data = user_data;

	/* Add to plugin's list: */
	plugin->list_of_exporter = g_list_append(plugin->list_of_exporter, exporter);

	return exporter;
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


/* Implementation of CongDocumentFactory: */
void cong_document_factory_invoke_page_creation_callback(CongDocumentFactory *factory, CongNewFileAssistant *assistant)
{
	g_return_if_fail(factory);
	g_return_if_fail(assistant);

	g_message("page creation for document factory \"%s\"", cong_functionality_get_name(CONG_FUNCTIONALITY(factory)));

	g_assert(factory->page_creation_callback);

	factory->page_creation_callback(factory, assistant, factory->user_data);
}

void cong_document_factory_invoke_action_callback(CongDocumentFactory *factory, CongNewFileAssistant *assistant)
{
	g_return_if_fail(factory);
	g_return_if_fail(assistant);

	g_message("invoking action for document factory \"%s\"", cong_functionality_get_name(CONG_FUNCTIONALITY(factory)));

	g_assert(factory->action_callback);

	factory->action_callback(factory, assistant, factory->user_data);
}

/* Implementation of CongImporter: */
gboolean cong_importer_supports_mime_type(CongImporter *importer, const gchar *mime_type)
{
	g_return_val_if_fail(importer, FALSE);
	g_return_val_if_fail(mime_type, FALSE);

	g_assert(importer->mime_filter);

	return importer->mime_filter(importer, mime_type, importer->user_data);

}

void cong_importer_invoke(CongImporter *importer, const gchar *filename, const gchar *mime_type)
{
	g_return_if_fail(importer);
	g_return_if_fail(filename);
	g_return_if_fail(mime_type);
	
	g_assert(importer->action_callback);

	return importer->action_callback(importer, filename, mime_type, importer->user_data);
}

void cong_ui_new_document_from_manufactured_xml(xmlDocPtr xml_doc)
{
	CongDocument *cong_doc;
	CongDispspec *ds;
	
	g_return_if_fail(xml_doc);

	ds = get_appropriate_dispspec(xml_doc);

	if (ds == NULL) {
		GtkDialog* dialog;
		gchar *what_failed;
		gchar *why_failed;
		gchar *suggestions;
		
		what_failed = g_strdup_printf("Conglomerate could not create the file");
		why_failed = g_strdup_printf("Conglomerate could not find display information for the new file");
		suggestions = g_strdup_printf("There may a problem with your installation, or a bug in the importer");
		
		dialog = cong_error_dialog_new(what_failed,
					       why_failed,
					       suggestions);
		g_free(what_failed);
		g_free(why_failed);
		g_free(suggestions);		
	
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		
		xmlFreeDoc(xml_doc);
		return;
	}
	
	g_assert(xml_doc);
	g_assert(ds);

	cong_doc = cong_document_new_from_xmldoc(xml_doc, ds, NULL);

	cong_primary_window_new(cong_doc);
}

void cong_ui_new_document_from_imported_xml(xmlDocPtr xml_doc)
{
	CongDocument *cong_doc;
	CongDispspec *ds;
	
	g_return_if_fail(xml_doc);

	ds = get_appropriate_dispspec(xml_doc);

	if (ds == NULL) {
		GtkDialog* dialog;
		gchar *what_failed;
		gchar *why_failed;
		gchar *suggestions;
		
		what_failed = g_strdup_printf("Conglomerate could not import the file");
		why_failed = g_strdup_printf("Conglomerate could not find display information for the new file");
		suggestions = g_strdup_printf("There may a problem with your installation, or a bug in the importer");
		
		dialog = cong_error_dialog_new(what_failed,
					       why_failed,
					       suggestions);
		g_free(what_failed);
		g_free(why_failed);
		g_free(suggestions);		
	
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		
		xmlFreeDoc(xml_doc);
		return;
	}
	
	g_assert(xml_doc);
	g_assert(ds);

	cong_doc = cong_document_new_from_xmldoc(xml_doc, ds, NULL);

	cong_primary_window_new(cong_doc);
}

/* Handy methods for "Import" methods; doing the necessary UI hooks: */
gboolean cong_ui_load_imported_file_content(const gchar *uri,
					    char** buffer,
					    GnomeVFSFileSize* size)
{
	GnomeVFSResult vfs_result;

	g_return_val_if_fail(uri, FALSE);
	g_return_val_if_fail(buffer, FALSE);
	g_return_val_if_fail(size, FALSE);

	vfs_result = cong_vfs_new_buffer_from_file(uri, buffer, size);
	
	if (vfs_result!=GNOME_VFS_OK) {
		GnomeVFSURI* file_uri = gnome_vfs_uri_new(uri);
		GtkDialog* dialog = cong_error_dialog_new_file_open_failed_from_vfs_result(file_uri, vfs_result);
		
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		
		gnome_vfs_uri_unref(file_uri);
		
		return FALSE;
	}
	
	g_assert(*buffer);

	return TRUE;
}

