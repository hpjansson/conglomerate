/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-tree-view.c
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
 * Based on code by Hans Petter Jansson <hpj@ximian.com>
 */

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "global.h"

#define CONG_TREE_VIEW(x) ((CongTreeView*)(x))

struct CongTreeView
{
	CongView view;

	/* the treeview widget has the userdata "cong_tree_view" set on it */
	GtkTreeView *gtk_tree_view;
	GtkTreeStore *gtk_tree_store;
};

void cong_tree_view_populate_tree(CongTreeView *tree_view);
void cong_tree_view_recursive_populate(CongDocument *doc, 
				       CongNodePtr x, 
				       gboolean collapsed, 
				       GtkTreeStore* store, 
				       GtkTreeIter* parent_iter);

struct search_struct
{
	CongNodePtr node;

	gboolean found_it;	
	
	GtkTreeIter *tree_iter;
};

static gboolean search_for_node(GtkTreeModel *model,
				GtkTreePath *path,
				GtkTreeIter *iter,
				gpointer data)
{
	struct search_struct* search = (struct search_struct*)data;
	CongNodePtr this_node;
	
	g_assert(model);
	g_assert(path);
	g_assert(iter);
	g_assert(search);

	gtk_tree_model_get(model,
			   iter,
			   TREEVIEW_NODE_COLUMN,
			   &this_node,
			   -1);

	if (this_node==search->node) {
		search->found_it = TRUE;
		*search->tree_iter = *iter;
		return TRUE;
	} else {
		return FALSE;
	}

}

static gboolean get_iter_for_node(CongTreeView *tree_view, CongNodePtr node, GtkTreeIter* tree_iter)
{
	/* FIXME: this is O(n), it ought to be O(1), by adding some kind of search structure */
	struct search_struct search;
	
	search.node = node;
	search.found_it = FALSE;
	search.tree_iter = tree_iter;

	gtk_tree_model_foreach(GTK_TREE_MODEL(tree_view->gtk_tree_store),
			       search_for_node,
			       &search);

	return search.found_it;
}

/* Prototypes of the handler functions: */
static void on_document_coarse_update(CongView *view);
static void on_document_node_make_orphan(CongView *view, CongNodePtr node);
static void on_document_node_add_after(CongView *view, CongNodePtr node, CongNodePtr older_sibling);
static void on_document_node_add_before(CongView *view, CongNodePtr node, CongNodePtr younger_sibling);
static void on_document_node_set_parent(CongView *view, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
static void on_document_node_set_text(CongView *view, CongNodePtr node, const xmlChar *new_content);

#define DEBUG_TREE_VIEW 1

/* Definitions of the handler functions: */
static void on_document_coarse_update(CongView *view)
{
	CongTreeView *tree_view;
	GtkTreeIter tree_iter;

	g_return_if_fail(view);

	#if DEBUG_TREE_VIEW
	g_message("CongTreeView - on_document_coarse_update\n");
	#endif

	tree_view = CONG_TREE_VIEW(view);

#if 0
	/* Ignore this for now: */
#else
	/* Empty and then repopulate the tree store: */
	gtk_tree_store_clear(tree_view->gtk_tree_store);
	cong_tree_view_populate_tree(tree_view);
#endif

}

static void on_document_node_make_orphan(CongView *view, CongNodePtr node)
{
	CongTreeView *tree_view;
	GtkTreeIter tree_iter;

	g_return_if_fail(view);
	g_return_if_fail(node);

	#if DEBUG_TREE_VIEW
	g_message("CongTreeView - on_document_node_make_orphan\n");
	#endif

	tree_view = CONG_TREE_VIEW(view);

	if ( get_iter_for_node(tree_view, node, &tree_iter) ) {

		/* Remove this branch of the tree: */
		gtk_tree_store_remove(tree_view->gtk_tree_store, &tree_iter);

	}
}

static void on_document_node_add_after(CongView *view, CongNodePtr node, CongNodePtr older_sibling)
{
	CongTreeView *tree_view;
	GtkTreeIter tree_iter_sibling;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(older_sibling);

	#if DEBUG_TREE_VIEW
	g_message("CongTreeView - on_document_node_add_after\n");
	#endif

	tree_view = CONG_TREE_VIEW(view);

	if ( get_iter_for_node(tree_view, older_sibling, &tree_iter_sibling) ) {

		if ( get_iter_for_node(tree_view, older_sibling->parent, &tree_iter_parent) ) {

#if 0
			GtkTreeIter new_tree_iter;
			gtk_tree_store_insert_after(tree_view->tree_store, &new_tree_iter, &tree_iter_parent, &tree_iter_sibling);
#endif

#if 0
			/* FIXME: this doesn't insert it in the correct place: */
			cong_tree_view_recursive_populate(CONG_VIEW(tree_view)->doc, 
							  node, 
							  TRUE, /* gboolean collapsed, */
							  tree_view->tree_store, 
							  tree_iter_parent);
#endif
		}
	}
}

static void on_document_node_add_before(CongView *view, CongNodePtr node, CongNodePtr younger_sibling)
{
	CongTreeView *tree_view;
	GtkTreeIter tree_iter_sibling;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);

	#if DEBUG_TREE_VIEW
	g_message("CongTreeView - on_document_node_add_before\n");
	#endif

	tree_view = CONG_TREE_VIEW(view);

}

static void on_document_node_set_parent(CongView *view, CongNodePtr node, CongNodePtr adoptive_parent)
{
	CongTreeView *tree_view;
	GtkTreeIter tree_iter_node;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);

	#if DEBUG_TREE_VIEW
	g_message("CongTreeView - on_document_node_set_parent\n");
	#endif

	tree_view = CONG_TREE_VIEW(view);

}

static void on_document_node_set_text(CongView *view, CongNodePtr node, const xmlChar *new_content)
{
	CongTreeView *tree_view;
	GtkTreeIter tree_iter;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_TREE_VIEW
	g_message("CongTreeView - on_document_node_set_text\n");
	#endif

	tree_view = CONG_TREE_VIEW(view);

}

CongNodePtr tree_editor_elements_skip(CongNodePtr x, CongDispspec *ds)
{
	for ( ; x; x = cong_node_next(x))
	{
		enum CongNodeType node_type = cong_node_type(x);
		const char *name = xml_frag_name_nice(x);

		if (node_type == CONG_NODE_TYPE_ELEMENT && cong_dispspec_element_structural(ds, name))
		{
			return(cong_node_prev(x));
		}

		if (CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE==cong_dispspec_type(ds, name)) {
			return(cong_node_prev(x));
		}
	}

	return(x);
}

void cong_tree_view_recursive_populate(CongDocument *doc, 
				       CongNodePtr x, 
				       gboolean collapsed, 
				       GtkTreeStore* store, 
				       GtkTreeIter* parent_iter)
{
	CongDispspec *ds;

	CongNodePtr x_orig;

	GtkTreeIter new_tree_iter;

	CongDispspecElement *element;

	gchar *text;

	ds = cong_document_get_dispspec(doc);

	element = cong_dispspec_lookup_element(ds, cong_node_name(x));

	text = cong_dispspec_get_section_header_text(ds, x);
      	
	gtk_tree_store_append (store, &new_tree_iter, parent_iter);

	gtk_tree_store_set (store, &new_tree_iter,
			    TREEVIEW_TITLE_COLUMN, text,
			    TREEVIEW_NODE_COLUMN, x,
			    TREEVIEW_DOC_COLUMN, doc,
			    -1);

	g_free(text);
	/* FIXME:  this will fail to update when the text is edited */

	if (element) {
#if NEW_LOOK
		const GdkColor *col = cong_dispspec_element_col(element, CONG_DISPSPEC_GC_USAGE_TEXT);
		/* We hope this will contrast well against white */
#else
		const GdkColor *col = cong_dispspec_element_col(element);
#endif

		gchar *col_string = get_col_string(col);

		gtk_tree_store_set (store, &new_tree_iter,
				    TREEVIEW_FOREGROUND_COLOR_COLUMN, col_string,
				    -1);

		g_free(col_string);

		/* Experimental attempt to show background colour; looks ugly */
#if 0 /* NEW_LOOK */
		col_string = get_col_string( cong_dispspec_element_col(element, CONG_DISPSPEC_GC_USAGE_BACKGROUND) );
		gtk_tree_store_set (store, &new_tree_iter,
				    TREEVIEW_BACKGROUND_COLOR_COLUMN, col_string,
				    -1);

		g_free(col_string);
#endif

	} else {
		/* Use red for "tag not found" errors: */ 
		gtk_tree_store_set (store, &new_tree_iter,
				    TREEVIEW_FOREGROUND_COLOR_COLUMN, "#ff0000", 
				    -1);
	}

	x_orig = x;
	
	x = cong_node_first_child(x);
	if (!x) return;

	for ( ; x; )
	{
		enum CongNodeType node_type = cong_node_type(x);
		const char *name = xml_frag_name_nice(x);

		/* g_message("Examining frag %s\n",name); */

		if (node_type == CONG_NODE_TYPE_ELEMENT)
		{
			if (cong_dispspec_element_structural(ds, name))
			{
				if (cong_dispspec_element_collapse(ds, name))
				{
					cong_tree_view_recursive_populate(doc, x, TRUE, store, &new_tree_iter);
				}
				else
				{
					/* New structural element */
					cong_tree_view_recursive_populate(doc, x, FALSE, store, &new_tree_iter);
				}
			}
			else if (cong_dispspec_element_span(ds, name) ||
				 cong_dispspec_element_insert(ds, name))
			{
				/* New editor window */
				x = tree_editor_elements_skip(x, ds);
			}
		}
		else if (node_type == CONG_NODE_TYPE_TEXT)
		{
			/* New editor window */
			x = tree_editor_elements_skip(x, ds);
		}

		if (x) {
			x = cong_node_next(x);
		}
	}

	if (!collapsed) {
		/* FIXME: Expand the node: */
	}

}


void cong_tree_view_populate_tree(CongTreeView *tree_view)
{
	CongDocument *doc;
	CongDispspec *displayspec;

	gchar* filename;
	GtkTreeIter root_iter;

	CongNodePtr x;

	g_assert(tree_view);

	doc = tree_view->view.doc;
	displayspec = cong_document_get_dispspec(doc);
	
	filename = cong_document_get_filename(doc);

	gtk_tree_store_append (tree_view->gtk_tree_store, &root_iter, NULL);  /* Acquire a top-level iterator */
	gtk_tree_store_set (tree_view->gtk_tree_store, &root_iter,
			    TREEVIEW_TITLE_COLUMN, filename,
			    TREEVIEW_NODE_COLUMN, cong_document_get_root(doc),
			    TREEVIEW_DOC_COLUMN, doc,
			    /* TREEVIEW_COLOR_COLUMN, g_strdup_printf("#305050"), */
			    -1);
	/* FIXME: What colour should the Document node be? */

	g_free(filename);

	x = cong_document_get_root(doc);

	for ( ; x; x = cong_node_next(x))
	{
		enum CongNodeType type = cong_node_type(x);

		const char *name = xml_frag_name_nice(x);

		g_message("examining frag \"%s\", type = %s\n", name, cong_node_type_description(type));
		
		if (type == CONG_NODE_TYPE_ELEMENT && cong_dispspec_element_structural(displayspec, name))
		{
			/* New element */
			cong_tree_view_recursive_populate(doc, x, FALSE, tree_view->gtk_tree_store, &root_iter);
		}
	}

}

CongTreeView *cong_tree_view_new(CongDocument *doc)
{
	CongTreeView *tree_view;

 	GtkTreeViewColumn *column;
 	GtkCellRenderer *renderer; 

	g_return_val_if_fail(doc, NULL);

	tree_view = g_new0(CongTreeView, 1);

	tree_view->view.doc = doc;
	tree_view->view.klass = g_new0(CongViewClass,1);
	tree_view->view.klass->on_document_coarse_update = on_document_coarse_update;
	tree_view->view.klass->on_document_node_make_orphan = on_document_node_make_orphan;
	tree_view->view.klass->on_document_node_add_after = on_document_node_add_after;
	tree_view->view.klass->on_document_node_add_before = on_document_node_add_before;
	tree_view->view.klass->on_document_node_set_parent = on_document_node_set_parent;
	tree_view->view.klass->on_document_node_set_text = on_document_node_set_text;

	cong_document_register_view( doc, CONG_VIEW(tree_view) );

        tree_view->gtk_tree_store = gtk_tree_store_new (TREEVIEW_N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING);

	tree_view->gtk_tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model (GTK_TREE_MODEL(tree_view->gtk_tree_store)));

	g_object_set_data(G_OBJECT(tree_view->gtk_tree_view),
			  "cong_tree_view",
			  tree_view);

	renderer = gtk_cell_renderer_text_new ();

	/* Create a column, associating the "text" attribute of the
	 * cell_renderer to the first column of the model */
	column = gtk_tree_view_column_new_with_attributes ("Element", renderer,
							   "text", TREEVIEW_TITLE_COLUMN,
							   "foreground", TREEVIEW_FOREGROUND_COLOR_COLUMN,
							   "background", TREEVIEW_BACKGROUND_COLOR_COLUMN,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view->gtk_tree_view), column);

 	/* Wire up the context-menu callback */
 	gtk_signal_connect_object(GTK_OBJECT(tree_view->gtk_tree_view), "event",
 				  (GtkSignalFunc) tpopup_show, tree_view->gtk_tree_view);

	cong_tree_view_populate_tree(tree_view);

	gtk_widget_show(GTK_WIDGET(tree_view->gtk_tree_view));

	return tree_view;
}

void cong_tree_view_free(CongTreeView *tree_view)
{
	g_return_if_fail(tree_view);

	/* FIXME: should we delete the widgetry as well? */
	/* FIXME: should we unref the tree store? */

	cong_document_unregister_view( tree_view->view.doc, CONG_VIEW(tree_view) );

	g_free(tree_view->view.klass);
	g_free(tree_view);
}

GtkWidget* cong_tree_view_get_widget(CongTreeView *tree_view)
{
	g_return_val_if_fail(tree_view, NULL);

	return GTK_WIDGET(tree_view->gtk_tree_view);
}

GtkTreeStore* cong_tree_view_get_tree_store(CongTreeView *tree_view)
{
	g_return_val_if_fail(tree_view, NULL);

	return tree_view->gtk_tree_store;
}
