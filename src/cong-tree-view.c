/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-tree-view.c
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
 * Based on code by Hans Petter Jansson <hpj@ximian.com>
 */

#include "global.h"
#include "cong-tree-view.h"
#include "cong-document.h"
#include "cong-selection.h"
#include "cong-ui-hooks.h"

#define PRIVATE(tree_view) ((tree_view)->private)

#define DEBUG_TREE_VIEW 0

#if 0
#define CONG_TREE_VIEW_DEBUG_MSG1(x)       g_message((x))
#define CONG_TREE_VIEW_DEBUG_MSG2(x, a)    g_message((x), (a))
#define CONG_TREE_VIEW_DEBUG_MSG2(x, a, b) g_message((x), (a), (b))
#else
#define CONG_TREE_VIEW_DEBUG_MSG1(x)       ((void)0)
#define CONG_TREE_VIEW_DEBUG_MSG2(x, a)    ((void)0)
#define CONG_TREE_VIEW_DEBUG_MSG3(x, a, b) ((void)0)
#endif

typedef struct CongTreeViewDetails
{
	/* Creation parameters: */
	CongTreeViewNodeFilter node_filter;
	CongTreeViewNodeCreationCallback node_creation_callback;
	CongTreeViewPixbufCallback pixbuf_callback; /* can be NULL */
	gpointer user_data;

	/* the treeview widget has the userdata "cong_tree_view" set on it */
	GtkTreeView *gtk_tree_view;
	GtkTreeStore *gtk_tree_store;
	
	/* need some kind of search structure - mapping from nodes to GtkTreeIter; the GtkTreeStore class guarantees persistant validity of a GtkTreeIter provided the node remains valid */
	GTree *search_structure;

	gulong gtk_tree_view_selection_changed_handler;

} CongTreeViewDetails;

#if 0
struct search_struct
{
	CongNodePtr node;

	gboolean found_it;	
	
	GtkTreeIter *tree_iter;
};
#endif

/* Internal function prototypes: */
static void
on_widget_destroy_event (GtkWidget *widget,
			 gpointer user_data);


static gint 
tree_popup_show(GtkWidget *widget, GdkEvent *event);

static void        
on_tree_view_selection_changed (GtkTreeSelection *treeselection,
				gpointer user_data);

#if 0
static gboolean 
node_search_callback(GtkTreeModel *model,
		     GtkTreePath *path,
		     GtkTreeIter *iter,
		     gpointer data);
#endif

gint key_compare_func (gconstpointer a, 
		       gconstpointer b, 
		       gpointer user_data);
void value_destroy_func (gpointer data);

static gboolean 
get_iter_for_node(CongTreeViewDetails *tree_view_details, CongNodePtr node, GtkTreeIter* tree_iter);

static CongNodePtr
get_node_for_iter(CongTreeViewDetails *tree_view_details, GtkTreeIter* tree_iter);

static void 
regenerate_data_for_node (CongTreeView *cong_tree_view, 
			  CongNodePtr node);

static void on_document_node_make_orphan(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr former_parent);
static void on_document_node_add_after(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr older_sibling);
static void on_document_node_add_before(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr younger_sibling);
static void on_document_node_set_parent(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
static void on_document_node_set_text(CongView *view, gboolean before_change, CongNodePtr node, const xmlChar *new_content);
static void on_document_node_set_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name, const xmlChar *value);
static void on_document_node_remove_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name);
static void on_selection_change(CongView *view);
static void on_cursor_change(CongView *view);

static void
set_pixbuf (GtkTreeViewColumn *tree_column,
	    GtkCellRenderer   *cell,
	    GtkTreeModel      *model,
	    GtkTreeIter       *iter,
	    gpointer           user_data);

static void recursive_add_to_tree_store(CongTreeView *cong_tree_view,
					CongNodePtr node, 
					GtkTreeIter* tree_iter);

static void recursive_remove_from_tree_store(CongTreeView *cong_tree_view,
					     CongNodePtr node);

/* Exported function implementations: */
CongTreeView *
cong_tree_view_new (CongDocument *doc,
		    gboolean use_markup,
		    CongTreeViewNodeFilter node_filter,
		    CongTreeViewNodeCreationCallback node_creation_callback,
		    CongTreeViewPixbufCallback pixbuf_callback, /* can be NULL */
		    gpointer user_data
		    )
{
	CongTreeView *cong_tree_view;
	CongTreeViewDetails *details;

 	GtkTreeViewColumn *column;
 	GtkCellRenderer *renderer; 

	GtkTreeIter root_iter;

	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node_filter, NULL);
	g_return_val_if_fail(node_creation_callback, NULL);

	cong_tree_view = g_new0(CongTreeView, 1);
	details = g_new0(CongTreeViewDetails,1);
	
	PRIVATE(cong_tree_view) = details;
	
	details->node_filter = node_filter;
	details->node_creation_callback = node_creation_callback;
	details->pixbuf_callback = pixbuf_callback;
	details->user_data = user_data;

	details->search_structure = g_tree_new_full (key_compare_func,
						     NULL,
						     NULL,
						     value_destroy_func);
	cong_tree_view->view.doc = doc;
	cong_tree_view->view.klass = g_new0(CongViewClass,1);
	cong_tree_view->view.klass->on_document_node_make_orphan = on_document_node_make_orphan;
	cong_tree_view->view.klass->on_document_node_add_after = on_document_node_add_after;
	cong_tree_view->view.klass->on_document_node_add_before = on_document_node_add_before;
	cong_tree_view->view.klass->on_document_node_set_parent = on_document_node_set_parent;
	cong_tree_view->view.klass->on_document_node_set_text = on_document_node_set_text;
	cong_tree_view->view.klass->on_document_node_set_attribute = on_document_node_set_attribute;
	cong_tree_view->view.klass->on_document_node_remove_attribute = on_document_node_remove_attribute;
	cong_tree_view->view.klass->on_selection_change = on_selection_change;
	cong_tree_view->view.klass->on_cursor_change = on_cursor_change;

	/* Create and populate the tree store: */
	{
		details->gtk_tree_store = gtk_tree_store_new (CONG_TREE_VIEW_TREE_MODEL_N_COLUMNS, 
							      G_TYPE_POINTER, 
							      G_TYPE_POINTER, 
							      G_TYPE_STRING, 
							      G_TYPE_STRING, 
							      G_TYPE_STRING);
		
		gtk_tree_store_append (details->gtk_tree_store, &root_iter, NULL);  /* Acquire a top-level iterator */
		recursive_add_to_tree_store(cong_tree_view, (CongNodePtr)cong_document_get_xml(doc), &root_iter);
	}

	cong_document_register_view( doc, CONG_VIEW(cong_tree_view) );


	/* Create the GtkTreeView: */
	{
		details->gtk_tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model (GTK_TREE_MODEL(details->gtk_tree_store)));
		
		g_object_set_data(G_OBJECT(details->gtk_tree_view),
				  "cong_tree_view",
				  cong_tree_view);
		
		/* A single-columned treeview: */
		column = gtk_tree_view_column_new();
		
		if (pixbuf_callback) {
			/* Add a pixbuf-renderer to the column: */
			renderer = gtk_cell_renderer_pixbuf_new ();		
			gtk_tree_view_column_pack_start (column, renderer, FALSE);
			gtk_tree_view_column_set_cell_data_func (column, 
								 renderer, 
								 set_pixbuf, 
								 cong_tree_view, 
								 NULL);
		}
		
		/* Add a text renderer to the column: */
		renderer = gtk_cell_renderer_text_new ();		
		gtk_tree_view_column_pack_start (column, renderer, FALSE);
		gtk_tree_view_column_set_attributes(column,
						    renderer,
						    (use_markup ? "markup" : "text"), CONG_TREE_VIEW_TREE_MODEL_TITLE_COLUMN,
						    "foreground", CONG_TREE_VIEW_TREE_MODEL_FOREGROUND_COLOR_COLUMN,
						    "background", CONG_TREE_VIEW_TREE_MODEL_BACKGROUND_COLOR_COLUMN,
						    NULL);
		
		/* Add the column to the view. */
		gtk_tree_view_append_column (GTK_TREE_VIEW (details->gtk_tree_view), column);
		
		/* Wire up the context-menu callback */
		gtk_signal_connect_object(GTK_OBJECT(details->gtk_tree_view), 
					  "event",
					  (GtkSignalFunc) tree_popup_show, 
					  details->gtk_tree_view);

		/* Deal with selection changes: */
		details->gtk_tree_view_selection_changed_handler = g_signal_connect (G_OBJECT(gtk_tree_view_get_selection (details->gtk_tree_view)), 
										     "changed",
										     G_CALLBACK (on_tree_view_selection_changed),
										     cong_tree_view);

	}

	/* Set up for cleanup: */
	g_signal_connect (G_OBJECT (details->gtk_tree_view),
			  "destroy",
			  G_CALLBACK (on_widget_destroy_event),
			  cong_tree_view);
	
	/* Show the tree view: */
	gtk_widget_show(GTK_WIDGET(details->gtk_tree_view));
		
	return cong_tree_view;
}

void cong_tree_view_free(CongTreeView *tree_view)
{
	g_return_if_fail(tree_view);

#if 0
	g_message ("cong_tree_view_free");
#endif

	/* FIXME: should we delete the widgetry as well? */
	/* FIXME: should we unref the tree store? */

	cong_document_unregister_view( CONG_DOCUMENT (tree_view->view.doc), 
				       CONG_VIEW(tree_view) );

	g_free(tree_view->view.klass);
	g_free(tree_view);
}

GtkWidget* cong_tree_view_get_widget(CongTreeView *tree_view)
{
	g_return_val_if_fail(tree_view, NULL);

	return GTK_WIDGET(tree_view->private->gtk_tree_view);
}

gboolean
cong_tree_view_should_show_node (CongTreeView *cong_tree_view,
				 CongNodePtr node)
{
	g_return_val_if_fail (cong_tree_view, FALSE);
	g_return_val_if_fail (node, FALSE);

	g_assert(PRIVATE(cong_tree_view)->node_filter);

	return PRIVATE(cong_tree_view)->node_filter (cong_tree_view,
						     node,
						     PRIVATE(cong_tree_view)->user_data);
}

CongNodePtr
cong_tree_view_get_selected_node (CongTreeView *cong_tree_view)
{
	GtkTreeIter tree_iter;

	if (gtk_tree_selection_get_selected (gtk_tree_view_get_selection (PRIVATE(cong_tree_view)->gtk_tree_view),
					     NULL,
                                             &tree_iter)) {
		return get_node_for_iter (PRIVATE(cong_tree_view), &tree_iter);
	} else {
		return NULL;
	}
}

GtkTreeStore* 
cong_tree_view_protected_get_tree_store (CongTreeView *tree_view)
{
	g_return_val_if_fail(tree_view, NULL);

	return tree_view->private->gtk_tree_store;
}

void
cong_tree_view_protected_force_node_update (CongTreeView *tree_view,
					    CongNodePtr node)
{
	regenerate_data_for_node (tree_view, 
				  node);
}


/* Internal function implementations: */
static void
on_widget_destroy_event (GtkWidget *widget,
			 gpointer user_data)
{
	CongTreeView *cong_tree_view = user_data;

#if 0
	g_message ("on_widget_destroy_event");
#endif

	cong_tree_view_free (cong_tree_view);
}

/* the treeview widget has the userdata "cong_tree_view" set on it */
static gint 
tree_popup_show(GtkWidget *widget, GdkEvent *event)
{
	GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_toplevel(widget));

	if (event->type == GDK_BUTTON_PRESS)
	{
		GdkEventButton *bevent = (GdkEventButton *) event;
		if (bevent->button != 3) return(FALSE);
		
 		/* printf("button 3\n"); */
 		{
			GtkTreePath* path;
			if ( gtk_tree_view_get_path_at_pos( GTK_TREE_VIEW(widget),
							    bevent->x,
							    bevent->y,
							    &path,
							    NULL,
							    NULL, 
							    NULL)
			     ) { 
				CongTreeView *cong_tree_view;
				GtkTreeModel* tree_model;
				GtkTreeIter iter;

				cong_tree_view = g_object_get_data(G_OBJECT(widget),
								   "cong_tree_view");
				g_assert(cong_tree_view);

				tree_model = GTK_TREE_MODEL(cong_tree_view_protected_get_tree_store(cong_tree_view));
#if 0
				gchar* msg = gtk_tree_path_to_string(path);
				printf("right-click on path \"%s\"\n",msg);
				g_free(msg);
#endif
		    
				if ( gtk_tree_model_get_iter(tree_model, &iter, path) ) {
					CongNodePtr node;
					GtkWidget* menu;
					CongDocument* doc = cong_view_get_document(CONG_VIEW(cong_tree_view));
					
					gtk_tree_model_get(tree_model, &iter, CONG_TREE_VIEW_TREE_MODEL_NODE_COLUMN, &node, -1);
					gtk_tree_model_get(tree_model, &iter, CONG_TREE_VIEW_TREE_MODEL_DOC_COLUMN, &doc, -1);
					
#if 0
					{
						gchar *desc = cong_node_debug_description (node);
						g_message ("got node \"%s\"\n",desc);
						g_free (desc);
					}
#endif
					
					menu = cong_ui_popup_init(doc, 
								  node, 
								  parent_window);
					gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, bevent->button,
						       bevent->time);		      
				}
				
				
				gtk_tree_path_free(path);		  
			}

 		}

		return(TRUE);
	}

	return(FALSE);
}

static void        
on_tree_view_selection_changed (GtkTreeSelection *treeselection,
				gpointer user_data)
{
	CongTreeView *cong_tree_view = CONG_TREE_VIEW (user_data);
	CongDocument *doc;
	CongNodePtr node;

	g_message ("on_tree_view_selection_changed");


	doc = cong_view_get_document (CONG_VIEW (cong_tree_view));

	node = cong_tree_view_get_selected_node (cong_tree_view);


	if (node) {
		g_signal_handler_block ( G_OBJECT (gtk_tree_view_get_selection (PRIVATE(cong_tree_view)->gtk_tree_view)),
					 PRIVATE(cong_tree_view)->gtk_tree_view_selection_changed_handler);

		cong_document_select_node (doc, node);

		g_signal_handler_unblock ( G_OBJECT (gtk_tree_view_get_selection (PRIVATE(cong_tree_view)->gtk_tree_view)),
					   PRIVATE(cong_tree_view)->gtk_tree_view_selection_changed_handler);
	}

}


#if 0
static gboolean 
node_search_callback (GtkTreeModel *model,
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
			   CONG_TREE_VIEW_TREE_MODEL_NODE_COLUMN,
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
#endif


gint key_compare_func (gconstpointer a, 
		       gconstpointer b, 
		       gpointer user_data)
{
	return (gint)(a-b);
}

void value_destroy_func (gpointer data)
{
	g_free(data);
}

static gboolean 
get_iter_for_node(CongTreeViewDetails *tree_view_details, CongNodePtr node, GtkTreeIter* tree_iter)
{
#if 1
	GtkTreeIter *found_iter = g_tree_lookup (tree_view_details->search_structure,
						 node);

	if (found_iter) {
		*tree_iter = *found_iter;
		return TRUE;
	} else {
		return FALSE;
	}
#else
	/* FIXME: this is O(n), it ought to be O(1), by adding some kind of search structure */
	struct search_struct search;
	
	search.node = node;
	search.found_it = FALSE;
	search.tree_iter = tree_iter;

	gtk_tree_model_foreach(GTK_TREE_MODEL(tree_view_details->gtk_tree_store),
			       node_search_callback,
			       &search);

	return search.found_it;
#endif
}

static CongNodePtr
get_node_for_iter(CongTreeViewDetails *tree_view_details, GtkTreeIter* tree_iter)
{
	CongNodePtr node = NULL;

	gtk_tree_model_get (GTK_TREE_MODEL(tree_view_details->gtk_tree_store),
			    tree_iter,
			    CONG_TREE_VIEW_TREE_MODEL_NODE_COLUMN, &node,
			    -1);

	return node;
}

/* Definitions of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr former_parent)
{
	CongTreeView *cong_tree_view;

	g_return_if_fail(view);
	g_return_if_fail(node);

	#if DEBUG_TREE_VIEW
	CONG_TREE_VIEW_DEBUG_MSG1("CongTreeView - on_document_node_make_orphan\n");
	#endif

	if (before_change) {
		return;
	}

	cong_tree_view = CONG_TREE_VIEW(view);

	recursive_remove_from_tree_store(cong_tree_view, node);
}

static void on_document_node_add_after(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr older_sibling)
{
	CongTreeView *cong_tree_view;
	GtkTreeIter tree_iter_sibling;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(older_sibling);

	#if DEBUG_TREE_VIEW
	CONG_TREE_VIEW_DEBUG_MSG1("CongTreeView - on_document_node_add_after\n");
	#endif

	if (before_change) {
		return;
	}

	cong_tree_view = CONG_TREE_VIEW(view);

	if (cong_tree_view_should_show_node(cong_tree_view, node)) {

		if ( get_iter_for_node(PRIVATE(cong_tree_view), older_sibling->parent, &tree_iter_parent) ) {
	
			/* Perhaps the older sibling isn't in the overview; find youngest older sibling that actually appears in view: */
			while (!cong_tree_view_should_show_node(cong_tree_view, older_sibling)) {
				older_sibling = older_sibling->prev;
				
				if (NULL==older_sibling) {
					/* None of the siblings should appear in view; add as first child of parent: */
					/* FIXME: what if parent shouldn't appear? */
					GtkTreeIter new_tree_iter;
					gtk_tree_store_prepend(PRIVATE(cong_tree_view)->gtk_tree_store, &new_tree_iter, &tree_iter_parent);
				
					recursive_add_to_tree_store(cong_tree_view, node, &new_tree_iter);					
					return;
				}
			}

			g_assert(cong_tree_view_should_show_node(cong_tree_view, older_sibling));
			
			if ( get_iter_for_node(PRIVATE(cong_tree_view), older_sibling, &tree_iter_sibling) ) {
				
				GtkTreeIter new_tree_iter;
				gtk_tree_store_insert_after(PRIVATE(cong_tree_view)->gtk_tree_store, &new_tree_iter, &tree_iter_parent, &tree_iter_sibling);
				
				recursive_add_to_tree_store(cong_tree_view, node, &new_tree_iter);
			}
		}
	}
}

static void on_document_node_add_before(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr younger_sibling)
{
	CongTreeView *cong_tree_view;
	GtkTreeIter tree_iter_sibling;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);

	#if DEBUG_TREE_VIEW
	CONG_TREE_VIEW_DEBUG_MSG1("CongTreeView - on_document_node_add_before\n");
	#endif

	if (before_change) {
		return;
	}

	cong_tree_view = CONG_TREE_VIEW(view);

	if (cong_tree_view_should_show_node(cong_tree_view, node)) {

		if ( get_iter_for_node(PRIVATE(cong_tree_view), younger_sibling->parent, &tree_iter_parent) ) {

			/* Perhaps the younger sibling isn't in the overview; find oldest younger sibling that actually appears in view: */
			while (!cong_tree_view_should_show_node(cong_tree_view, younger_sibling)) {
				younger_sibling = younger_sibling->next;
				
				if (NULL==younger_sibling) {
					/* None of the siblings should appear in view; add as final child of parent: */
					/* FIXME: what if parent shouldn't appear? */
					GtkTreeIter new_tree_iter;
					gtk_tree_store_append(PRIVATE(cong_tree_view)->gtk_tree_store, &new_tree_iter, &tree_iter_parent);
				
					recursive_add_to_tree_store(cong_tree_view, node, &new_tree_iter);					
					return;
				}
			}

			g_assert(cong_tree_view_should_show_node(cong_tree_view, younger_sibling));			

			if ( get_iter_for_node(PRIVATE(cong_tree_view), younger_sibling, &tree_iter_sibling) ) {
				
				GtkTreeIter new_tree_iter;
				gtk_tree_store_insert_before(PRIVATE(cong_tree_view)->gtk_tree_store, &new_tree_iter, &tree_iter_parent, &tree_iter_sibling);
				
				recursive_add_to_tree_store(cong_tree_view, node, &new_tree_iter);
			}
		}
	}
}

static void on_document_node_set_parent(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr adoptive_parent)
{
	CongTreeView *cong_tree_view;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);

	#if DEBUG_TREE_VIEW
	CONG_TREE_VIEW_DEBUG_MSG1("CongTreeView - on_document_node_set_parent\n");
	#endif

	if (before_change) {
		return;
	}

	cong_tree_view = CONG_TREE_VIEW(view);

	if (cong_tree_view_should_show_node(cong_tree_view, node)) {
		recursive_remove_from_tree_store(cong_tree_view, node);

		if ( get_iter_for_node(PRIVATE(cong_tree_view), adoptive_parent, &tree_iter_parent) ) {
			GtkTreeIter new_tree_iter;
			gtk_tree_store_append(PRIVATE(cong_tree_view)->gtk_tree_store, &new_tree_iter, &tree_iter_parent);
			
			recursive_add_to_tree_store(cong_tree_view, node, &new_tree_iter);
		}
	}

}

static void regenerate_data_for_node(CongTreeView *cong_tree_view, CongNodePtr node)
{
	GtkTreeIter tree_iter;

	/* FIXME: Potentially we need to regenerate all text in the tree, due to XPath concerns... */

	/* For now, just regenerate the text for this node: */
	if ( get_iter_for_node(PRIVATE(cong_tree_view), node, &tree_iter) ) {
		PRIVATE(cong_tree_view)->node_creation_callback (cong_tree_view,
								 &tree_iter,
								 node,
								 PRIVATE(cong_tree_view)->user_data);
	}
}

static void on_document_node_set_text(CongView *view, gboolean before_change, CongNodePtr node, const xmlChar *new_content)
{
	CongTreeView *cong_tree_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_TREE_VIEW
	CONG_TREE_VIEW_DEBUG_MSG1("CongTreeView - on_document_node_set_text\n");
	#endif

	cong_tree_view = CONG_TREE_VIEW(view);

	if (!before_change) {
		regenerate_data_for_node(cong_tree_view, node);
	}
}

static void on_document_node_set_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name, const xmlChar *value)
{
	CongTreeView *cong_tree_view;

	g_return_if_fail(view);
	g_return_if_fail(node);

	#if DEBUG_TREE_VIEW
	CONG_TREE_VIEW_DEBUG_MSG1("CongTreeView - on_document_node_set_attribute\n");
	#endif

	cong_tree_view = CONG_TREE_VIEW(view);

	if (!before_event) {
		regenerate_data_for_node(cong_tree_view, node);
	}
}

static void on_document_node_remove_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name)
{
	CongTreeView *cong_tree_view;

	g_return_if_fail(view);
	g_return_if_fail(node);

	#if DEBUG_TREE_VIEW
	CONG_TREE_VIEW_DEBUG_MSG1("CongTreeView - on_document_node_remove_attribute\n");
	#endif

	cong_tree_view = CONG_TREE_VIEW(view);

	if (!before_event) {
		regenerate_data_for_node(cong_tree_view, node);
	}
}


static void on_selection_change(CongView *view)
{
	CongTreeView *cong_tree_view;
	CongDocument *doc;
	CongSelection *selection;
	CongNodePtr node;
	GtkTreeSelection *tree_selection;

	g_return_if_fail(view);

	#if DEBUG_TREE_VIEW
	CONG_TREE_VIEW_DEBUG_MSG1("CongTreeView - on_selection_change\n");
	#endif

	cong_tree_view = CONG_TREE_VIEW(view);

	tree_selection = gtk_tree_view_get_selection (PRIVATE(cong_tree_view)->gtk_tree_view);
	g_assert (tree_selection);
	
	doc = cong_view_get_document (view);
	selection = cong_document_get_selection (doc);
	
	node = cong_location_node (cong_selection_get_logical_start (selection));
	
	g_signal_handler_block ( G_OBJECT (gtk_tree_view_get_selection (PRIVATE(cong_tree_view)->gtk_tree_view)),
				 PRIVATE(cong_tree_view)->gtk_tree_view_selection_changed_handler);
	
	if (node) {
		GtkTreeIter tree_iter;
		
		if (get_iter_for_node(PRIVATE(cong_tree_view), node, &tree_iter)) {
			gtk_tree_selection_select_iter  (tree_selection,
							 &tree_iter);
		}
	} else {
		gtk_tree_selection_unselect_all (tree_selection);
	}
	
	g_signal_handler_unblock ( G_OBJECT (gtk_tree_view_get_selection (PRIVATE(cong_tree_view)->gtk_tree_view)),
				   PRIVATE(cong_tree_view)->gtk_tree_view_selection_changed_handler);
}

static void on_cursor_change(CongView *view)
{
}

static void
set_pixbuf (GtkTreeViewColumn *tree_column,
	    GtkCellRenderer   *cell,
	    GtkTreeModel      *model,
	    GtkTreeIter       *iter,
	    gpointer           user_data)
{
	CongTreeView *cong_tree_view = user_data;	
	CongNodePtr node = NULL;
	GdkPixbuf *pixbuf = NULL;
	
	g_assert (cong_tree_view);
	g_assert (PRIVATE(cong_tree_view)->pixbuf_callback);

       	gtk_tree_model_get (model, 
			    iter, 
			    CONG_TREE_VIEW_TREE_MODEL_NODE_COLUMN, &node, 
			    -1);
	if (NULL==node) {
		return;
	}

	pixbuf = PRIVATE(cong_tree_view)->pixbuf_callback (cong_tree_view, 
							   node, 
							   PRIVATE(cong_tree_view)->user_data);

	g_object_set (GTK_CELL_RENDERER (cell), "pixbuf", pixbuf, NULL);
	if (pixbuf) {
		g_object_unref (pixbuf);
	}
}

static void recursive_add_to_tree_store(CongTreeView *cong_tree_view,
					  CongNodePtr node, 
					  GtkTreeIter* tree_iter)
{
	CongDocument *doc;
	CongDispspec *ds;
	CongNodePtr child_node;

	g_return_if_fail(cong_tree_view);
	g_return_if_fail(node);
	g_return_if_fail(tree_iter);

	g_assert(cong_tree_view_should_show_node(cong_tree_view,node));

	doc = cong_view_get_document(CONG_VIEW(cong_tree_view));
	ds = cong_view_get_dispspec(CONG_VIEW(cong_tree_view));

	gtk_tree_store_set (PRIVATE(cong_tree_view)->gtk_tree_store, 
			    tree_iter,
			    CONG_TREE_VIEW_TREE_MODEL_NODE_COLUMN, node,
			    CONG_TREE_VIEW_TREE_MODEL_DOC_COLUMN, doc,
			    -1);

	g_assert(PRIVATE(cong_tree_view)->node_creation_callback);
	PRIVATE(cong_tree_view)->node_creation_callback (cong_tree_view,
							 tree_iter,
							 node,
							 PRIVATE(cong_tree_view)->user_data);

	/* Add to search structure: */
	{
		GtkTreeIter *copied_tree_iter = g_new(GtkTreeIter, 1);
		*copied_tree_iter = *tree_iter;

		g_tree_insert (PRIVATE(cong_tree_view)->search_structure,
			       node,
			       copied_tree_iter);
	}
		      

	/* Recurse through the children: */
	for (child_node=node->children; child_node; child_node=child_node->next) {
		if (cong_tree_view_should_show_node(cong_tree_view,
						    child_node)) {
			GtkTreeIter child_tree_iter;
			gtk_tree_store_append (PRIVATE(cong_tree_view)->gtk_tree_store, &child_tree_iter, tree_iter);

			recursive_add_to_tree_store(cong_tree_view,
						    child_node, 
						    &child_tree_iter);
		}
	}
}

static void recursive_remove_from_search_structure(CongTreeView *cong_tree_view,
						   CongNodePtr node)
{
	CongNodePtr child_node;
	
	g_tree_remove(PRIVATE(cong_tree_view)->search_structure, node);	
	
	for (child_node=node->children; child_node; child_node=child_node->next) {
		recursive_remove_from_search_structure (cong_tree_view,
							child_node);
	}
}

static void recursive_remove_from_tree_store(CongTreeView *cong_tree_view,
					     CongNodePtr node)
{
	GtkTreeIter tree_iter_node;

	g_return_if_fail(cong_tree_view);
	g_return_if_fail(node);
	
	if ( get_iter_for_node(PRIVATE(cong_tree_view), node, &tree_iter_node) ) {
		/* Remove this branch of the tree: */
		gtk_tree_store_remove(PRIVATE(cong_tree_view)->gtk_tree_store, &tree_iter_node);
	}		
	
	recursive_remove_from_search_structure (cong_tree_view,
						node);
}
