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

	CongDocumentFactoryCallback callback;
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
							   CongDocumentFactoryCallback callback,
							   gpointer user_data)
{
	CongDocumentFactory *factory;

	g_return_val_if_fail(plugin, NULL);
	g_return_val_if_fail(name, NULL);
	g_return_val_if_fail(description, NULL);
	g_return_val_if_fail(callback, NULL);

	factory = g_new0(CongDocumentFactory,1);

	factory->functionality.name = g_strdup(name);
	factory->functionality.description = g_strdup(description);
	factory->callback = callback;
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

