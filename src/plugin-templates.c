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
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

const gchar NAMESPACE[] = "http://www.conglomerate.org/";

typedef struct CongTemplate
{
	CongPlugin *plugin;
	gchar* dir;
} CongTemplate;

static xmlXPathContextPtr create_xpath_context(xmlDocPtr doc)
{
	xmlXPathContextPtr xpathCtx; 
	xpathCtx = xmlXPathNewContext(doc);
	xmlXPathRegisterNs(xpathCtx, "cong", NAMESPACE);
	return xpathCtx;
}

static void remove_template_def(xmlDocPtr template)
{
	xmlXPathContextPtr xpathCtx; 
	xmlXPathObjectPtr xpathObj; 
	xmlNodePtr def;

	xpathCtx = create_xpath_context(template);

	g_assert(xpathCtx);

	xpathObj = xmlXPathEvalExpression("/*/cong:template", xpathCtx);

	g_assert(xpathObj);

	def = xpathObj->nodesetval->nodeTab[0];
	xmlReplaceNode(def, def->next);

	xmlXPathFreeObject(xpathObj);
	xmlXPathFreeContext(xpathCtx); 
}

void factory_action_callback_templates(CongServiceDocumentFactory *factory,
	CongNewFileAssistant *assistant, gpointer user_data)
{
	xmlDocPtr template;
	gchar* template_file_name;

	template_file_name = (gchar*)user_data;

	template = xmlParseFile(template_file_name);

	remove_template_def(template);

	cong_ui_new_document_from_manufactured_xml(template,
		cong_new_file_assistant_get_toplevel(assistant));
}

void factory_page_creation_callback_templates(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
}

static GSList* get_template_paths(CongPlugin* plugin)
{
	GSList* template_paths;
	gchar* gconf_key;
	GConfClient* gconf_client;

	gconf_client = cong_app_get_gconf_client(cong_app_singleton());
	gconf_key = cong_plugin_get_gconf_key(plugin, "template-paths");

	g_message("key: %s", gconf_key);

	template_paths = gconf_client_get_list(gconf_client,
		gconf_key, GCONF_VALUE_STRING, NULL);

	g_free(gconf_key);

	return template_paths;
}

static xmlChar* value_from_template(xmlDocPtr doc, xmlChar* path)
{
	xmlXPathContextPtr xpathCtx; 
	xmlXPathObjectPtr xpathObj; 
	xmlChar* value;

	xpathCtx = create_xpath_context(doc);

	g_assert(xpathCtx);

	xpathObj = xmlXPathEvalExpression(path, xpathCtx);

	g_assert(xpathObj);

	value = g_strdup(xpathObj->stringval);

	xmlXPathFreeObject(xpathObj);
	xmlXPathFreeContext(xpathCtx); 

	return value;
}

static xmlChar* cong_get_template_name(xmlDocPtr doc)
{
	return value_from_template(doc,
		"string(/*/cong:template/cong:name)");
}

static xmlChar* cong_get_template_description(xmlDocPtr doc)
{
	return value_from_template(doc,
		"string(/*/cong:template/cong:description)");
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

	g_message("template: %s, %s", name, description);

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

	xmlFree(name);
	xmlFree(description);
	xmlFreeDoc(doc);

	return TRUE;
}

static void visit_paths(GSList* paths, GnomeVFSDirectoryVisitFunc visit_path,
		void* data)
{
	GSList* path;
	path = paths;

	for(path = paths; path != NULL; path = g_slist_next(path))
	{
		GnomeVFSResult vfs_result;

		gchar* absolute_path;
		absolute_path = gnome_vfs_expand_initial_tilde(path->data);

		g_message("loading templates from %s", absolute_path);

		((CongTemplate*)data)->dir = absolute_path;

		vfs_result = gnome_vfs_directory_visit(absolute_path,
			GNOME_VFS_FILE_INFO_DEFAULT,
			GNOME_VFS_DIRECTORY_VISIT_DEFAULT,
			visit_path,
			data);
	}

}

/* would be exposed as "plugin_register"? */
gboolean plugin_templates_plugin_register(CongPlugin *plugin)
{
	GSList* template_paths;
	CongTemplate* template;

	g_return_val_if_fail(plugin, FALSE);

	template_paths = get_template_paths(plugin);

	template = g_new0(CongTemplate, sizeof(template));
	template->plugin = plugin;

	visit_paths(template_paths, register_template, template);

	g_slist_free(template_paths);

	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean plugin_templates_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
