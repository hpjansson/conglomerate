/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-tree-view.h
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

#ifndef __CONG_TREE_VIEW_H__
#define __CONG_TREE_VIEW_H__

G_BEGIN_DECLS

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "cong-view.h"

#define CONG_TREE_VIEW(x) ((CongTreeView*)(x))

typedef struct CongTreeView CongTreeView;

struct CongTreeView
{
	CongView view;

	struct CongTreeViewDetails *private;
};

typedef gboolean (*CongTreeViewNodeFilter) (CongTreeView *cong_tree_view,
					    CongNodePtr node,
					    gpointer user_data);

typedef void (*CongTreeViewNodeCreationCallback) (CongTreeView *cong_tree_view,
						  GtkTreeIter* tree_iter,
						  CongNodePtr node,
						  gpointer user_data);

typedef GdkPixbuf* (*CongTreeViewPixbufCallback) (CongTreeView *cong_tree_view,
						  CongNodePtr node,
						  gpointer user_data);

CongTreeView *
cong_tree_view_new (CongDocument *doc,
		    gboolean use_markup,
		    CongTreeViewNodeFilter node_filter,
		    CongTreeViewNodeCreationCallback node_creation_callback,
		    CongTreeViewPixbufCallback pixbuf_callback, /* can be NULL */
		    gpointer user_data
		    );

void 
cong_tree_view_free (CongTreeView *tree_view);

GtkWidget* 
cong_tree_view_get_widget (CongTreeView *tree_view);

gboolean
cong_tree_view_should_show_node (CongTreeView *cong_tree_view,
				 CongNodePtr node);

CongNodePtr
cong_tree_view_get_selected_node (CongTreeView *cong_tree_view);

GtkTreeStore* 
cong_tree_view_protected_get_tree_store (CongTreeView *tree_view);

enum
{
	/* Set by the base tree view: */
	CONG_TREE_VIEW_TREE_MODEL_NODE_COLUMN,
	CONG_TREE_VIEW_TREE_MODEL_DOC_COLUMN,
	
	/* To be set by derived tree views: */
	CONG_TREE_VIEW_TREE_MODEL_TITLE_COLUMN,
	CONG_TREE_VIEW_TREE_MODEL_FOREGROUND_COLOR_COLUMN,
	CONG_TREE_VIEW_TREE_MODEL_BACKGROUND_COLOR_COLUMN,
	
	CONG_TREE_VIEW_TREE_MODEL_N_COLUMNS
};

G_END_DECLS

#endif
