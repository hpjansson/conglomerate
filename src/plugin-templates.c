/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-templates.c
 *
 * Plugin for Creating Stub Documents from Dispspecs
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
 * Authors: Jeff Martin <jeff@custommonkey.org>
 */

#include "global.h"
#include "cong-plugin.h"
#include "cong-app.h"
#include "cong-fake-plugin-hooks.h"
#include "cong-dispspec.h"
#include "cong-util.h"
#include "cong-vfs.h"

const gchar NAMESPACE[] = "http://www.conglomerate.org/";

typedef struct CongTemplate
{
	CongPlugin *plugin;
	gchar* dir;
} CongTemplate;

void factory_action_callback_templates(CongDocumentFactory *factory,
	CongNewFileAssistant *assistant, gpointer user_data)
{
	xmlDocPtr template;
	gchar* template_file_name;

	template_file_name = (gchar*)user_data;

	template = xmlParseFile(template_file_name);

	xmlUnsetNsProp (xmlDocGetRootElement(template),
				"name", NAMESPACE);
	xmlUnsetNsProp (xmlDocGetRootElement(template),
				"description", NAMESPACE);

	cong_ui_new_document_from_manufactured_xml(template,
		cong_new_file_assistant_get_toplevel(assistant));
}

void factory_page_creation_callback_templates(CongDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
}

static gchar** get_template_paths(CongPlugin* plugin)
{
	gchar* template_path;
	gchar** template_paths;
	gchar* gconf_key;
	GConfClient* gconf_client;

	gconf_client = cong_app_get_gconf_client(cong_app_singleton());
	gconf_key = cong_plugin_get_gconf_key(plugin, "template-path");

	template_path = gconf_client_get_string(
		gconf_client,
		gconf_key, NULL);

	if(template_path==NULL)
	{
		template_path = gnome_program_locate_file(
			cong_app_get_gnome_program(cong_app_singleton()),
			GNOME_FILE_DOMAIN_APP_DATADIR,
			"templates",
			FALSE,
			NULL);

		gconf_client_set_string(gconf_client,
			gconf_key,
			template_path,
			NULL);
	}

	template_paths = g_strsplit(template_path, ":", 0);

	g_free(gconf_key);
	g_free(template_path);

	return template_paths;
}

static xmlChar* cong_get_template_name(xmlDocPtr doc)
{
	return xmlGetNsProp(xmlDocGetRootElement(doc), "name", NAMESPACE);
}

static xmlChar* cong_get_template_description(xmlDocPtr doc)
{
	return xmlGetNsProp(xmlDocGetRootElement(doc), "description", NAMESPACE);
}

static gboolean
register_template(const gchar *rel_path,
	   GnomeVFSFileInfo *info,
	   gboolean recursing_will_loop,
	   gpointer data,
	   gboolean *recurse)
{
	xmlDocPtr doc;
	gchar* file_name;
	xmlChar *name;
	xmlChar *description;

	CongTemplate* template = (CongTemplate*)data;

	g_return_val_if_fail(template->dir, TRUE);
	g_return_val_if_fail(rel_path, TRUE);

	file_name = g_strconcat(template->dir, "/", rel_path, NULL);
	g_return_val_if_fail(file_name, TRUE);

	g_message("template file: %s", file_name);

	doc = xmlParseFile(file_name);
	name = cong_get_template_name(doc);
	description = cong_get_template_description(doc);

	if(name!=NULL && description!=NULL)
	{
		g_message("plugin: %s, %s, %s", name, description, file_name);
		cong_plugin_register_document_factory(template->plugin, 
			_(name), 
			_(description),
			name, 
			factory_page_creation_callback_templates,
			factory_action_callback_templates,
			rel_path,
			file_name);
	}else{
		g_warning("Template %s is not a valid template", file_name);
	}

	xmlFreeDoc(doc);

	return TRUE;
}

void visit_paths(gchar** paths, GnomeVFSDirectoryVisitFunc visit_path,
		void* data)
{
	int i;
	for(i = 0; paths[i]!=NULL; i++)
	{
		GnomeVFSResult vfs_result;

		((CongTemplate*)data)->dir = paths[i];

		vfs_result = gnome_vfs_directory_visit(paths[i],
			GNOME_VFS_FILE_INFO_DEFAULT,
			GNOME_VFS_DIRECTORY_VISIT_DEFAULT,
			visit_path,
			data);
	}
}

/* would be exposed as "plugin_register"? */
gboolean plugin_templates_plugin_register(CongPlugin *plugin)
{
	int i;
	gchar** template_paths;

	g_return_val_if_fail(plugin, FALSE);

	template_paths = get_template_paths(plugin);

	CongTemplate* template;
	template = g_new0(CongTemplate, sizeof(template));
	template->plugin = plugin;

	visit_paths(template_paths, register_template, template);

	g_strfreev(template_paths);

	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean plugin_templates_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
