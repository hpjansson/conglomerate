/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-website.c
 *
 * Support for Norman Walsh's "Website" format
 *
 * Copyright (C) 2003 David Malcolm
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
#include "cong-plugin.h"
#include "cong-error-dialog.h"
#include "cong-document.h"
#include "cong-graph.h"
#include "cong-dialog.h"
#include "cong-primary-window.h"
#include "cong-util.h"
#include "cong-vfs.h"

#include "cong-fake-plugin-hooks.h"

#if 0
gboolean cong_transform_easy(const gchar *filename_src_xml, 
			     const gchar *filename_dst_xml, 
			     const gchar *filename_stylesheet,
			     gchar **standard_error,
			     GError **error)
{
	gchar *command_line;
	gchar *standard_output;
	gint exit_status;

	g_return_val_if_fail (filename_src_xml, FALSE);
	g_return_val_if_fail (filename_dst_xml, FALSE);
	g_return_val_if_fail (filename_stylesheet, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	command_line = g_strdup_printf("xsltproc --output %s %s %s", filename_dst_xml, filename_stylesheet, filename_src_xml);
	
	if (g_spawn_command_line_sync(command_line,
				      &standard_output,
				      standard_error,
				      &exit_status,
				      error)) {
		g_free(command_line);
		g_free(standard_output);
		
		if (exit_status) {
			/* Some kind of error occurred: */
			if (error) {
				*error = g_error_new_literal(g_quark_from_string("cong"),exit_status,*standard_error);
			}
			return FALSE;
		}

		return TRUE;

	} else {
		g_free(command_line);
		g_free(standard_output);
		return FALSE;
	}
}


typedef struct WebsiteContent
{
	struct WebsiteContent *parent_content;
	GList *list_of_child; /* inherits dst_dir attribute; maybe some others? */

	gchar *src_page;
	gchar *dst_dir;
	gchar *dst_filename;

} WebsiteContent;

typedef struct GenerateWebsiteWorkspace
{
	CongPrimaryWindow *primary_window;
	CongDocument *doc;

	gchar *src_dir;
	gchar *build_dir;
	gchar *xsl_dir;

	gchar *xsl_file; /* full path */

	GnomeVFSURI *src_uri;
	GnomeVFSURI *build_uri;
	GnomeVFSURI *xsl_uri;

	WebsiteContent *toc;
	GList *list_of_notoc; /* list of WebsiteContent ptrs */

	CongDependencyGraph *graph;

} GenerateWebsiteWorkspace;

static gboolean generate_html(CongDependencyNode *node, GError **error);

typedef struct CongDependencyNodeTransformWebsiteContent
{
	CongDependencyNodeFromFile base;

	GenerateWebsiteWorkspace *workspace;
	GnomeVFSURI *src_uri;
	WebsiteContent *content;
} CongDependencyNodeTransformWebsiteContent;

struct CongDependencyNodeClass klass_transform_website_content = 
{
	cong_dependency_node_from_file_is_up_to_date,
	generate_html
};



static void parse_config(GenerateWebsiteWorkspace *workspace, xmlNodePtr node) {
	g_message("parse_config");
}
static void parse_copyright(GenerateWebsiteWorkspace *workspace, xmlNodePtr node) {
	g_message("parse_copyright");
}
static void parse_headlink(GenerateWebsiteWorkspace *workspace, xmlNodePtr node) {
	g_message("parse_headlink");
}
static void parse_style(GenerateWebsiteWorkspace *workspace, xmlNodePtr node) {
	g_message("parse_style");
}

/**
   Extracts page, filename and dir params from a <toc>, <tocentry>, or <notoc>:
 */

WebsiteContent* website_content_new(xmlNodePtr node, WebsiteContent *parent_content)
{
	WebsiteContent *content;

	g_return_val_if_fail(node, NULL);

	content = g_new0(WebsiteContent,1);
	content->parent_content = parent_content;
	if (parent_content) {
		parent_content->list_of_child = g_list_append(parent_content->list_of_child,  content);
	}
	content->src_page = cong_node_get_attribute(node, "page");
	content->dst_dir = cong_node_get_attribute(node, "dir");
	content->dst_filename =cong_node_get_attribute(node, "filename");

	g_message("got page \"%s\"->\"%s\"", content->src_page, content->dst_filename); 

	return content;
}

static void parse_tocentry(GenerateWebsiteWorkspace *workspace, WebsiteContent *parent_content, xmlNodePtr node) {
	xmlNodePtr child;
	WebsiteContent *content;

	g_message("parse_tocentry");

	g_assert(workspace);
	g_assert(parent_content);
	g_assert(node);

	content = website_content_new(node, parent_content);

	for (child=node->children; child; child=child->next) {

		if (cong_node_is_element(child, NULL, "tocentry")) {
			parse_tocentry(workspace, content, child);
		}
	}
}
static void parse_toc(GenerateWebsiteWorkspace *workspace, xmlNodePtr node) {
	xmlNodePtr child;
	WebsiteContent *toc;

	g_message("parse_toc");

	g_assert(NULL==workspace->toc); /* for now */

	toc = website_content_new(node, NULL);
	workspace->toc = toc;

	for (child=node->children; child; child=child->next) {
		if (cong_node_is_element(child, NULL, "tocentry")) {
			parse_tocentry(workspace, toc, child);
		}
	}
}

static void parse_notoc(GenerateWebsiteWorkspace *workspace, xmlNodePtr node) {
	WebsiteContent *content;

	g_message("parse_notoc");

	content = website_content_new(node, NULL);
	workspace->list_of_notoc = g_list_append(workspace->list_of_notoc, content);
}


static void traverse_layout(GenerateWebsiteWorkspace *workspace, xmlNodePtr node) {
	CongNodePtr child;

	g_assert(node);

	g_message("traverse_layout <%s>", node->name);

	for (child=node->children; child; child=child->next) {
		if (cong_node_is_element(child, NULL, "config")) {
			parse_config(workspace, child);
		} else if (cong_node_is_element(child, NULL, "copyright")) {
			parse_copyright(workspace, child);
		} else if (cong_node_is_element(child, NULL, "headlink")) {
			parse_headlink(workspace, child);
		} else if (cong_node_is_element(child, NULL, "style")) {
			parse_style(workspace, child);
		} else if (cong_node_is_element(child, NULL, "toc")) {
			parse_toc(workspace, child);
		} else if (cong_node_is_element(child, NULL, "notoc")) {
			parse_notoc(workspace, child);
		}
	}	
}

static void traverse_dom_tree(GenerateWebsiteWorkspace *workspace, xmlNodePtr node) {
	CongNodePtr child;

	g_assert(node);

	g_message("traverse_dom_tree <%s>", node->name);

	for (child=node->children; child; child=child->next) {
		if (cong_node_is_element(child, NULL, "layout") ){
			traverse_layout(workspace, child);
		}
	}	
}

/* Evaluate dir attribute, inheriting if necessary: */
const CongXMLChar* content_get_dir(WebsiteContent *content)
{
	g_return_val_if_fail(content, NULL);

	if (content->dst_dir) {
		return content->dst_dir; /* does it simply override parent content?  What about nesting? */
	} else {
		if (content->parent_content) {
			return content_get_dir(content->parent_content);
		} else {
			return NULL;
		}
	}
}

static GnomeVFSURI *content_make_dst_uri(WebsiteContent *content, GenerateWebsiteWorkspace *workspace)
{
	const CongXMLChar *dst_dir;
	GnomeVFSURI *path_uri;
	GnomeVFSURI *result_uri;

	g_assert(content); 
	g_assert(workspace);

	dst_dir = content_get_dir(content);

	if (dst_dir) {
		path_uri = gnome_vfs_uri_append_path(workspace->build_uri, dst_dir);
		
	} else {
		path_uri = workspace->build_uri;
		gnome_vfs_uri_ref(workspace->build_uri);
	}

	result_uri = gnome_vfs_uri_append_path(path_uri, content->dst_filename);
	gnome_vfs_uri_unref(path_uri);

	return result_uri;
}

static GnomeVFSURI *content_make_src_uri(WebsiteContent *content, GenerateWebsiteWorkspace *workspace)
{
	g_assert(content); 
	g_assert(workspace);

	return gnome_vfs_uri_append_file_name(workspace->src_uri, content->src_page);
}

static gboolean generate_html(CongDependencyNode *node, GError **error)
{
	CongDependencyNodeTransformWebsiteContent *node_from_content = (CongDependencyNodeTransformWebsiteContent*)(node);
#if 0
	WebsiteContent *content;
#endif
	gchar *output_filename;
	gchar *input_filename;
	gchar *standard_error;
	gboolean result;

	g_return_val_if_fail (node, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	output_filename = cong_vfs_get_local_path_from_uri (node_from_content->base.uri);
	input_filename = cong_vfs_get_local_path_from_uri (node_from_content->src_uri);

	result = cong_transform_easy(input_filename,
				     output_filename,
				     node_from_content->workspace->xsl_file,
				     &standard_error,
				     error);

	g_message(standard_error);
	g_free(standard_error);

	g_free(output_filename);
	g_free(input_filename);
	
	return result;
}

static CongDependencyNode *make_cong_node_from_website_content(WebsiteContent *content, GenerateWebsiteWorkspace *workspace)
{
	CongDependencyNodeTransformWebsiteContent *new_node;
	CongDependencyNode *src_file;
	GnomeVFSURI *dst_uri;
	GnomeVFSURI *src_uri;

	g_assert(content); 
	g_assert(workspace);

	g_message("making CongDependencyNode for %s", content->src_page);

	dst_uri = content_make_dst_uri(content, workspace);
	src_uri = content_make_src_uri(content, workspace);

	new_node = g_new0(CongDependencyNodeTransformWebsiteContent,1);
	construct_dependency_node_from_file(CONG_DEPENDENCY_NODE_FROM_FILE(new_node), &klass_transform_website_content, dst_uri);
	new_node->workspace = workspace;
	new_node->src_uri = src_uri;

	src_file = cong_dependency_node_new_from_file(src_uri);

	cong_dependency_node_add_dependency(CONG_DEPENDENCY_NODE(new_node), src_file);
	/* FIXME: should add dependency on the XSL file as well, really */
	/* FIXME: should add dependency on the autolayout.xml file too */

	gnome_vfs_uri_unref(dst_uri);

	return CONG_DEPENDENCY_NODE(new_node);
}

static void build_node_graph_recurse_content(GenerateWebsiteWorkspace *workspace, WebsiteContent *content)
{
	GList *iter;
	CongDependencyNode *node;

	g_assert(content); 
	g_assert(workspace);

	node = make_cong_node_from_website_content(content, workspace);
	cong_dependency_graph_add_ultimate_node(workspace->graph, node);	

	/* Recurse children: */
	for (iter = content->list_of_child; iter; iter=iter->next) {
		build_node_graph_recurse_content(workspace, (WebsiteContent*)(iter->data));
	}	
}

static void build_node_graph(GenerateWebsiteWorkspace *workspace)
{
	GList *iter;

	build_node_graph_recurse_content(workspace, workspace->toc);

	for (iter=workspace->list_of_notoc; iter; iter=iter->next) {
		build_node_graph_recurse_content(workspace, (WebsiteContent*)(iter->data));
	}
}


static gboolean doc_filter(CongServiceDocTool *tool, CongDocument *doc, gpointer user_data)
{
	const CongXMLChar* dtd_public_id = cong_document_get_dtd_public_identifier(doc);
	
	if (NULL==dtd_public_id) {
		return FALSE;
	}

	g_message(dtd_public_id);

	/* Only relevant for Doctype with public ID = "" */
	if (0==strcmp(dtd_public_id,"-//Norman Walsh//DTD Website Layout V2.4.0//EN")) {
		return TRUE;
	}

	return FALSE;
}

static void on_error_details(gpointer data)
{
	GenerateWebsiteWorkspace *workspace = (GenerateWebsiteWorkspace*)data;
	g_assert(workspace);
	
	
}


static void action_callback(CongServiceDocTool *tool, CongPrimaryWindow *primary_window, gpointer user_data)
{
	GenerateWebsiteWorkspace *workspace = g_new0(GenerateWebsiteWorkspace,1);
	GError *error = NULL;

	workspace->primary_window = primary_window;
	workspace->doc = cong_primary_window_get_document(primary_window);

	/* FIXME: Do dialog here: */
	workspace->src_dir = g_strdup("/home/david/coding/website-experiment/src"); /* for now */
	workspace->build_dir = g_strdup("/home/david/coding/website-experiment/build");
	workspace->xsl_dir = g_strdup("/usr/share/xml/website-2.4.1/xsl");

/*  	workspace->xsl_file = "/usr/share/xml/website-2.4.1/xsl/chunk-website.xsl"; */
/*   	workspace->xsl_file = "/usr/share/xml/website-2.4.1/xsl/chunk-tabular.xsl"; */
/*  	workspace->xsl_file = "/usr/share/xml/website-2.4.1/xsl/tabular.xsl"; */
/*  	workspace->xsl_file = "/usr/share/xml/website-2.4.1/xsl/website.xsl"; */
  	workspace->xsl_file = "/home/david/coding/website-experiment/src/custom-stylesheet.xsl";

	workspace->src_uri = gnome_vfs_uri_new(workspace->src_dir);
	workspace->build_uri = gnome_vfs_uri_new(workspace->build_dir);
	workspace->xsl_uri  = gnome_vfs_uri_new(workspace->xsl_dir);

	/* Traverse the DOM tree for layout.xml, building up useful information: */
	traverse_dom_tree(workspace, (xmlNodePtr)cong_document_get_xml(workspace->doc));

	/* Init the dependency node graph: */
	workspace->graph = cong_dependency_graph_new();

	/* Add the autolayout dependency */

	/* FIXME */

	/* Build up the set of nodes, with dependencies: */
	build_node_graph(workspace);

	/* Query to save any unsaved dependencies: */
	/* FIXME */
#if 0
	cong_dependency_graph_query_unsaved(workspace->graph, 
					    _("Generating website"),
					    cong_primary_window_get_toplevel(primary_window));
#endif

	/* Build the nodes: */
	if (cong_dependency_graph_process(workspace->graph, &error)) {
		GtkDialog *dialog = cong_dialog_information_alert_new(cong_primary_window_get_toplevel(primary_window),
								      _("Finished generating website."));
		gtk_dialog_run(dialog);
		gtk_widget_destroy(GTK_WIDGET(dialog));		
	} else {
		GtkDialog* dialog = cong_error_dialog_new_with_convenience(cong_primary_window_get_toplevel(primary_window),
									   _("Conglomerate did not finish generating the website"), 
									   _("One of the stages reported an error."), 
									   _("Click on the \"Details\" button for more information"),
									   _("_Details"),
									   on_error_details,
									   workspace);
		cong_error_dialog_run(dialog);
		gtk_widget_destroy(GTK_WIDGET(dialog));
	}

	/* FIXME: memory leaks here! */
	g_free(workspace);
}
#endif

 /* would be exposed as "plugin_register"? */
gboolean plugin_website_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	/* Disabled this plugin as it's too broken to be fixed.  See Bugzilla #113001 */
#if 0
	cong_plugin_register_doc_tool(plugin, 
				      _("Generate website"),
				      _("Generates a collection of HTML pages from the website XML description"),
				      "generate-website",
				      _("Generate _Website"),
				      _("Generates a collection of HTML pages from the website XML description"),
				      _("Generates a collection of HTML pages from the website XML description"),
				      doc_filter,
				      action_callback,
				      NULL);
#endif


	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean plugin_website_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
