/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-graph.c
 *
 * This file is licensed under the LGPL
 * Copyright (C) 2003 David Malcolm
 */

#include "global.h"
#include "cong-graph.h"

const CongDependencyNodeClass klass_pure_dependency = {
	cong_dependency_node_from_file_is_up_to_date,
	NULL
};

/**
 * cong_dependency_node_new_from_file:
 * @uri:
 *
 * TODO: Write me
 *
 * Returns: the new #CongDependencyNode
 */
CongDependencyNode *
cong_dependency_node_new_from_file(GnomeVFSURI *uri)
{
	CongDependencyNodeFromFile *node;

	node = g_new0(CongDependencyNodeFromFile,1);

	construct_dependency_node_from_file(node, &klass_pure_dependency, uri);

	return CONG_DEPENDENCY_NODE(node);
}

/**
 * construct_dependency_node_from_file:
 * @node:
 * @klass:
 * @uri:
 * 
 * TODO: Write me
 */
void 
construct_dependency_node_from_file(CongDependencyNodeFromFile *node, const CongDependencyNodeClass *klass, GnomeVFSURI *uri)
{
	node->base.klass = klass;
	node->base.debug_name = gnome_vfs_uri_to_string(uri, GNOME_VFS_URI_HIDE_PASSWORD);	
	
	node->uri = uri;
	gnome_vfs_uri_ref(uri);
}

/**
 * cong_dependency_node_add_dependency:
 * @downstream:
 * @upstream:

 * TODO: Write me
 */
void 
cong_dependency_node_add_dependency(CongDependencyNode *downstream, CongDependencyNode *upstream)
{
	g_return_if_fail(downstream);
	g_return_if_fail(upstream);

	downstream->list_of_upstream = g_list_append(downstream->list_of_upstream, upstream);
	upstream->list_of_downstream = g_list_append(upstream->list_of_downstream, downstream);
}

/**
 * cong_dependency_graph_new:
 *
 * Returns: a new #CongDependencyGraph
 */
CongDependencyGraph *
cong_dependency_graph_new(void)
{
	return g_new0(CongDependencyGraph,1);
}

/**
 * cong_dependency_graph_add_ultimate_node:
 * @graph:
 * @node:
 *
 * TODO: Write me
 */
void 
cong_dependency_graph_add_ultimate_node(CongDependencyGraph *graph, CongDependencyNode *node)
{
	g_return_if_fail(graph);
	g_return_if_fail(node);

	graph->list_of_ultimate_targets = g_list_append(graph->list_of_ultimate_targets, node);
}

/**
 * brute_force_build:
 * @node:
 * @error:
 *
 * TODO: Write me
 */
static gboolean 
brute_force_build(CongDependencyNode* node, GError **error)
{
	GList *iter;

	g_return_val_if_fail (node, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	
	g_message("Evaluating \"%s\"", node->debug_name);

	/* Build upstream nodes.  We hope there are no loops. */
	/* For now, the brute force approach. Just plough through entire build process, doing everything (possibly more than once!) */
	for (iter = node->list_of_upstream; iter; iter=iter->next) {
		if (!brute_force_build((CongDependencyNode*)(iter->data), error)) {
			g_assert (error == NULL || *error != NULL);
			return FALSE;
		}
	}	

	/* Build this node: */
	g_assert(node->klass);
	if (node->klass->generate) {
		if (!node->klass->generate(node, error)) {
			return FALSE;
		}
	} else {
		g_message("(node \"%s\" is a pure input; nothing to be done)", node->debug_name);
	}

	return TRUE;	
}

/**
 * cong_dependency_graph_process:
 * @graph:
 * @error:
 *
 * TODO: Write me
 *
 * Returns:
 */
gboolean 
cong_dependency_graph_process(CongDependencyGraph *graph, GError **error)
{
	GList *iter;

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (graph, FALSE);

	/* For now, the brute force approach. Just plough through entire build process, doing everything (possibly more than once!) */
	for (iter = graph->list_of_ultimate_targets; iter; iter=iter->next) {
		if (!brute_force_build((CongDependencyNode*)(iter->data), error)) {
			g_assert (error == NULL || *error != NULL);
			return FALSE;
		}
	}	

	return TRUE;
}

/**
 * cong_dependency_node_from_file_is_up_to_date:
 * @target: a #CongDependencyNode
 *
 * This function is not currently implemented.
 *
 * Returns: FALSE
 */
gboolean
cong_dependency_node_from_file_is_up_to_date(CongDependencyNode *target)
{
	return FALSE; /* for now */
}

