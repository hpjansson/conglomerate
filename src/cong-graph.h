/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-graph.h
 *
 * This file is licensed under the LGPL
 * Copyright (C) 2003 David Malcolm
 */

#ifndef __CONG_GRAPH_H
#define __CONG_GRAPH_H

G_BEGIN_DECLS

typedef struct _CongDependencyNode CongDependencyNode;
typedef struct _CongDependencyNodeClass CongDependencyNodeClass;
typedef struct _CongDependencyNodeFromFile CongDependencyNodeFromFile;
typedef struct _CongDependencyGraph CongDependencyGraph;
typedef struct _CongBuildProcess CongBuildProcess;

#define CONG_DEPENDENCY_NODE(x) ((CongDependencyNode*)(x))
#define CONG_DEPENDENCY_NODE_FROM_FILE(x) ((CongDependencyNodeFromFile*)(x))

struct _CongDependencyNode
{
	const CongDependencyNodeClass *klass;

	gchar *debug_name;

	/* FIXME: ultimately these should be GObjects, I think */
	/* FIXME: should generalise all of this into an editable graph of nodes, and allow in-memory modification */

	/* Last known modification time? */	

	GList *list_of_upstream; /* These are cong targets that affect this target */
	GList *list_of_downstream; /* These are cong targets; the targets affected by this target */
};

struct _CongDependencyNodeClass
{
	/* A rule for building a target */
	gboolean (*is_up_to_date)(CongDependencyNode *target); 
	gboolean (*generate) (CongDependencyNode *target, GError **error); /* can be NULL to indicate a pure dependency? */
};

struct _CongDependencyNodeFromFile
{
	CongDependencyNode base;

	GFile *file; /* in a subclass, to allow in-memory modification */
};

struct _CongDependencyGraph
{
	GList *list_of_ultimate_targets; /* These are CongDependencyNodes */
};

struct _CongBuildProcess
{
	/* mapping from targets to modification times? */
};

/* Manufacture a "pure" dependency from a GFile: */
CongDependencyNode *cong_dependency_node_new_from_file(GFile *file);
void cong_dependency_node_add_dependency(CongDependencyNode *downstream, CongDependencyNode *upstream);

void construct_dependency_node_from_file(CongDependencyNodeFromFile *node, const CongDependencyNodeClass *klass, GFile *uri);

CongDependencyGraph *cong_dependency_graph_new(void);
void cong_dependency_graph_add_ultimate_target(CongDependencyGraph *graph, CongDependencyNode *target);
gboolean cong_dependency_graph_process(CongDependencyGraph *graph, GError **error);

gboolean cong_dependency_node_from_file_is_up_to_date(CongDependencyNode *target); 

G_END_DECLS

#endif
