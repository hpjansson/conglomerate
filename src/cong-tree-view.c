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
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-view.h"

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

#define CONG_TREE_VIEW(x) ((CongTreeView*)(x))

enum
{
	CONG_OVERVIEW_TREEMODEL_TITLE_COLUMN,
	CONG_OVERVIEW_TREEMODEL_NODE_COLUMN,
	CONG_OVERVIEW_TREEMODEL_DOC_COLUMN,
	CONG_OVERVIEW_TREEMODEL_FOREGROUND_COLOR_COLUMN,
	CONG_OVERVIEW_TREEMODEL_BACKGROUND_COLOR_COLUMN,
	CONG_OVERVIEW_TREEMODEL_N_COLUMNS
};


struct CongTreeView
{
	CongView view;

	struct CongTreeViewDetails *private;
};

typedef struct CongTreeViewDetails
{
	/* the treeview widget has the userdata "cong_tree_view" set on it */
	GtkTreeView *gtk_tree_view;
	GtkTreeStore *gtk_tree_store;
} CongTreeViewDetails;

static gboolean should_appear_in_overview(CongTreeView *cong_tree_view,
				       CongNodePtr node);
static void populate_tree_store_recursive(CongTreeView *cong_tree_view,
					  CongNodePtr node, 
					  GtkTreeIter* node_iter);

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
			   CONG_OVERVIEW_TREEMODEL_NODE_COLUMN,
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

static gboolean get_iter_for_node(CongTreeViewDetails *tree_view_details, CongNodePtr node, GtkTreeIter* tree_iter)
{
	/* FIXME: this is O(n), it ought to be O(1), by adding some kind of search structure */
	struct search_struct search;
	
	search.node = node;
	search.found_it = FALSE;
	search.tree_iter = tree_iter;

	gtk_tree_model_foreach(GTK_TREE_MODEL(tree_view_details->gtk_tree_store),
			       search_for_node,
			       &search);

	return search.found_it;
}

/* the treeview widget has the userdata "cong_tree_view" set on it */
static gint tree_popup_show(GtkWidget *widget, GdkEvent *event)
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

				tree_model = GTK_TREE_MODEL(cong_tree_view_get_tree_store(cong_tree_view));
#if 0
				gchar* msg = gtk_tree_path_to_string(path);
				printf("right-click on path \"%s\"\n",msg);
				g_free(msg);
#endif
		    
				if ( gtk_tree_model_get_iter(tree_model, &iter, path) ) {
					CongNodePtr node;
					GtkWidget* menu;
					CongDocument* doc = cong_view_get_document(CONG_VIEW(cong_tree_view));
					
					gtk_tree_model_get(tree_model, &iter, CONG_OVERVIEW_TREEMODEL_NODE_COLUMN, &node, -1);
					gtk_tree_model_get(tree_model, &iter, CONG_OVERVIEW_TREEMODEL_DOC_COLUMN, &doc, -1);
					
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

/* Prototypes of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr former_parent);
static void on_document_node_add_after(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr older_sibling);
static void on_document_node_add_before(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr younger_sibling);
static void on_document_node_set_parent(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
static void on_document_node_set_text(CongView *view, gboolean before_change, CongNodePtr node, const xmlChar *new_content);
static void on_document_node_set_attribute(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *name, const xmlChar *value);
static void on_document_node_remove_attribute(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *name);
static void on_selection_change(CongView *view);
static void on_cursor_change(CongView *view);

/* Definitions of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr former_parent)
{
	CongTreeView *cong_tree_view;
	GtkTreeIter tree_iter;

	g_return_if_fail(view);
	g_return_if_fail(node);

	#if DEBUG_TREE_VIEW
	CONG_TREE_VIEW_DEBUG_MSG1("CongTreeView - on_document_node_make_orphan\n");
	#endif

	if (before_change) {
		return;
	}

	cong_tree_view = CONG_TREE_VIEW(view);

	if ( get_iter_for_node(cong_tree_view->private, node, &tree_iter) ) {
		
		/* Remove this branch of the tree: */
		gtk_tree_store_remove(cong_tree_view->private->gtk_tree_store, &tree_iter);
		
	}
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

	if (should_appear_in_overview(cong_tree_view, node)) {

		if ( get_iter_for_node(cong_tree_view->private, older_sibling->parent, &tree_iter_parent) ) {
	
			/* Perhaps the older sibling isn't in the overview; find youngest older sibling that actually appears in view: */
			while (!should_appear_in_overview(cong_tree_view, older_sibling)) {
				older_sibling = older_sibling->prev;
				
				if (NULL==older_sibling) {
					/* None of the siblings should appear in view; add as first child of parent: */
					/* FIXME: what if parent shouldn't appear? */
					GtkTreeIter new_tree_iter;
					gtk_tree_store_prepend(cong_tree_view->private->gtk_tree_store, &new_tree_iter, &tree_iter_parent);
				
					populate_tree_store_recursive(cong_tree_view, node, &new_tree_iter);					
					return;
				}
			}

			g_assert(should_appear_in_overview(cong_tree_view, older_sibling));
			
			if ( get_iter_for_node(cong_tree_view->private, older_sibling, &tree_iter_sibling) ) {
				
				GtkTreeIter new_tree_iter;
				gtk_tree_store_insert_after(cong_tree_view->private->gtk_tree_store, &new_tree_iter, &tree_iter_parent, &tree_iter_sibling);
				
				populate_tree_store_recursive(cong_tree_view, node, &new_tree_iter);
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

	if (should_appear_in_overview(cong_tree_view, node)) {

		if ( get_iter_for_node(cong_tree_view->private, younger_sibling->parent, &tree_iter_parent) ) {

			/* Perhaps the younger sibling isn't in the overview; find oldest younger sibling that actually appears in view: */
			while (!should_appear_in_overview(cong_tree_view, younger_sibling)) {
				younger_sibling = younger_sibling->next;
				
				if (NULL==younger_sibling) {
					/* None of the siblings should appear in view; add as final child of parent: */
					/* FIXME: what if parent shouldn't appear? */
					GtkTreeIter new_tree_iter;
					gtk_tree_store_append(cong_tree_view->private->gtk_tree_store, &new_tree_iter, &tree_iter_parent);
				
					populate_tree_store_recursive(cong_tree_view, node, &new_tree_iter);					
					return;
				}
			}

			g_assert(should_appear_in_overview(cong_tree_view, younger_sibling));			

			if ( get_iter_for_node(cong_tree_view->private, younger_sibling, &tree_iter_sibling) ) {
				
				GtkTreeIter new_tree_iter;
				gtk_tree_store_insert_before(cong_tree_view->private->gtk_tree_store, &new_tree_iter, &tree_iter_parent, &tree_iter_sibling);
				
				populate_tree_store_recursive(cong_tree_view, node, &new_tree_iter);
			}
		}
	}
}

static void on_document_node_set_parent(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr adoptive_parent)
{
	CongTreeView *cong_tree_view;
	GtkTreeIter tree_iter_node;
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

	if (should_appear_in_overview(cong_tree_view, node)) {
		if ( get_iter_for_node(cong_tree_view->private, node, &tree_iter_node) ) {
			/* Remove this branch of the tree: */
			gtk_tree_store_remove(cong_tree_view->private->gtk_tree_store, &tree_iter_node);
		}
		
		if ( get_iter_for_node(cong_tree_view->private, adoptive_parent, &tree_iter_parent) ) {
			GtkTreeIter new_tree_iter;
			gtk_tree_store_append(cong_tree_view->private->gtk_tree_store, &new_tree_iter, &tree_iter_parent);
			
			populate_tree_store_recursive(cong_tree_view, node, &new_tree_iter);
		}
	}

}

static void on_document_node_set_text(CongView *view, gboolean before_change, CongNodePtr node, const xmlChar *new_content)
{
	CongTreeView *cong_tree_view;
	GtkTreeIter tree_iter;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_TREE_VIEW
	CONG_TREE_VIEW_DEBUG_MSG1("CongTreeView - on_document_node_set_text\n");
	#endif

	cong_tree_view = CONG_TREE_VIEW(view);

	/* FIXME: Potentially we need to regenerate all text in the tree, due to XPath concerns... */
}

static void on_document_node_set_attribute(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *name, const xmlChar *value)
{
	/* UNWRITTEN */
}

static void on_document_node_remove_attribute(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *name)
{
	/* UNWRITTEN */
}

static void on_selection_change(CongView *view)
{
}

static void on_cursor_change(CongView *view)
{
}

CongNodePtr tree_editor_elements_skip(CongNodePtr x, CongDispspec *ds)
{
	for ( ; x; x = cong_node_next(x))
	{
		enum CongNodeType node_type = cong_node_type(x);
		const gchar *xmlns = cong_node_xmlns(x);
		const gchar *name = xml_frag_name_nice(x);
		CongDispspecElement* element = cong_dispspec_lookup_element(ds, xmlns, name);

		if (element) {
			if (node_type == CONG_NODE_TYPE_ELEMENT && cong_dispspec_element_is_structural(element)) {
				return(cong_node_prev(x));
			}

			if (CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE==cong_dispspec_element_type(element)) {
				return(cong_node_prev(x));
			}
		}
	}

	return(x);
}

static void
set_pixbuf (GtkTreeViewColumn *tree_column,
	    GtkCellRenderer   *cell,
	    GtkTreeModel      *model,
	    GtkTreeIter       *iter,
	    gpointer           user_data)
{
	CongTreeView *cong_tree_view = user_data;	
	CongNodePtr node;
	enum CongNodeType node_type;
	GdkPixbuf *pixbuf = NULL;
	
       	gtk_tree_model_get (model, iter, 
			    CONG_OVERVIEW_TREEMODEL_NODE_COLUMN, &node, 
			    -1);
	if (NULL==node) {
		return;
	}

	node_type = cong_node_type(node);

	switch (node_type) {
	default: g_assert_not_reached();
	case CONG_NODE_TYPE_UNKNOWN:
		{
			/* Empty for now */
		}
		break;

	case CONG_NODE_TYPE_ELEMENT:
		{
			CongDispspecElement *element;

			element = cong_document_get_dispspec_element_for_node(cong_view_get_document(CONG_VIEW(cong_tree_view)), node);

			if (element) {
				pixbuf = cong_dispspec_element_get_icon(element);
			}
		}
		break;

	case CONG_NODE_TYPE_DOCUMENT:
		{
			/* Empty for now; FIXME: should get icon from mime-type */
		}
		break;
	}

	g_object_set (GTK_CELL_RENDERER (cell), "pixbuf", pixbuf, NULL);
	if (pixbuf) {
		g_object_unref (pixbuf);
	}


}

static gboolean should_appear_in_overview(CongTreeView *cong_tree_view,
				       CongNodePtr node)
{
	enum CongNodeType node_type;

	g_return_val_if_fail(cong_tree_view, FALSE);
	g_return_val_if_fail(node, FALSE);

	node_type = cong_node_type(node);

	switch (node_type) {
	default: g_assert_not_reached();

	case CONG_NODE_TYPE_UNKNOWN:
		return TRUE;

	case CONG_NODE_TYPE_ELEMENT:
		{
			CongDispspecElement* element = cong_dispspec_lookup_node(cong_view_get_dispspec(CONG_VIEW(cong_tree_view)), node);
			
			if (element) {
				switch (cong_dispspec_element_type(element)) {
				default: g_assert_not_reached();
				case CONG_ELEMENT_TYPE_STRUCTURAL: 
					return TRUE;

				case CONG_ELEMENT_TYPE_SPAN:
				case CONG_ELEMENT_TYPE_INSERT:						
				case CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE:
					return FALSE;
						
				case CONG_ELEMENT_TYPE_PARAGRAPH:
					return FALSE;
						
				case CONG_ELEMENT_TYPE_PLUGIN:
					return FALSE;
						
				case CONG_ELEMENT_TYPE_UNKNOWN:
					return TRUE;

				}
			} else {
				/* Unknown elements should be added: */
				return TRUE;
			}
		}
	case CONG_NODE_TYPE_ATTRIBUTE:
	case CONG_NODE_TYPE_TEXT:
	case CONG_NODE_TYPE_CDATA_SECTION:
	case CONG_NODE_TYPE_ENTITY_REF:
	case CONG_NODE_TYPE_ENTITY_NODE:
	case CONG_NODE_TYPE_PI:
	case CONG_NODE_TYPE_COMMENT:
		return FALSE;

	case CONG_NODE_TYPE_DOCUMENT:
		return TRUE;

	case CONG_NODE_TYPE_DOCUMENT_TYPE:
	case CONG_NODE_TYPE_DOCUMENT_FRAG:
	case CONG_NODE_TYPE_NOTATION:
	case CONG_NODE_TYPE_HTML_DOCUMENT:
	case CONG_NODE_TYPE_DTD:
	case CONG_NODE_TYPE_ELEMENT_DECL:
	case CONG_NODE_TYPE_ATRRIBUTE_DECL:
	case CONG_NODE_TYPE_ENTITY_DECL:
	case CONG_NODE_TYPE_NAMESPACE_DECL:
	case CONG_NODE_TYPE_XINCLUDE_START:
	case CONG_NODE_TYPE_XINCLUDE_END:
		return FALSE;
	}
}

static void populate_tree_store_recursive(CongTreeView *cong_tree_view,
					  CongNodePtr node, 
					  GtkTreeIter* tree_iter)
{
	CongDocument *doc;
	CongDispspec *ds;
	enum CongNodeType node_type;
	CongNodePtr child_node;

	g_return_if_fail(cong_tree_view);
	g_return_if_fail(node);
	g_return_if_fail(tree_iter);

	g_assert(should_appear_in_overview(cong_tree_view,node));

	doc = cong_view_get_document(CONG_VIEW(cong_tree_view));
	ds = cong_view_get_dispspec(CONG_VIEW(cong_tree_view));

	gtk_tree_store_set (cong_tree_view->private->gtk_tree_store, 
			    tree_iter,
			    CONG_OVERVIEW_TREEMODEL_NODE_COLUMN, node,
			    CONG_OVERVIEW_TREEMODEL_DOC_COLUMN, doc,
			    -1);

	node_type = cong_node_type(node);

	switch (node_type) {
	default: g_assert_not_reached();
	case CONG_NODE_TYPE_UNKNOWN:
		{
			gtk_tree_store_set (cong_tree_view->private->gtk_tree_store, 
					    tree_iter,
					    CONG_OVERVIEW_TREEMODEL_TITLE_COLUMN, _("Unknown data"),
					    -1);
		}
		break;

	case CONG_NODE_TYPE_ELEMENT:
		{
			CongDispspecElement *element;
			GdkPixbuf *pixbuf;
			gchar *text;

			element = cong_dispspec_lookup_node(ds, node);

			if (element) {
				const GdkColor *col;
				gchar *col_string;

				text = cong_dispspec_element_get_section_header_text(element, node);
      	
				gtk_tree_store_set (cong_tree_view->private->gtk_tree_store, 
						    tree_iter,
						    CONG_OVERVIEW_TREEMODEL_TITLE_COLUMN, text,
						    -1);
				g_free(text);
				/* FIXME:  this will fail to update when the text is edited */
#if NEW_LOOK
				col = cong_dispspec_element_col(element, CONG_DISPSPEC_GC_USAGE_TEXT);
				/* We hope this will contrast well against white */
#else
				col = cong_dispspec_element_col(element);
#endif

				col_string = get_col_string(col);
				
				gtk_tree_store_set (cong_tree_view->private->gtk_tree_store, 
						    tree_iter,
						    CONG_OVERVIEW_TREEMODEL_FOREGROUND_COLOR_COLUMN, col_string,
						    -1);
				
				g_free(col_string);
				
			} else {
				/* Use red for "tag not found" errors: */ 
				gchar *text;

				if (cong_node_xmlns(node)) {
					text= g_strdup_printf("<%s:%s>", cong_node_xmlns(node), node->name);
				} else {
					text= g_strdup_printf("<%s>", node->name);
				}

				gtk_tree_store_set (cong_tree_view->private->gtk_tree_store, 
						    tree_iter,
						    CONG_OVERVIEW_TREEMODEL_TITLE_COLUMN, text,
						    CONG_OVERVIEW_TREEMODEL_FOREGROUND_COLOR_COLUMN, "#ff0000", 
						    -1);
				g_free(text);
			}
		}
		break;

	case CONG_NODE_TYPE_DOCUMENT:
		{
			gchar *filename = cong_document_get_filename(doc);

			gtk_tree_store_set (cong_tree_view->private->gtk_tree_store, tree_iter,
					    CONG_OVERVIEW_TREEMODEL_TITLE_COLUMN, filename,
					    /* CONG_OVERVIEW_TREEMODEL_COLOR_COLUMN, g_strdup_printf("#305050"), */
					    -1);
			/* FIXME: What colour should the Document node be? */
			
			g_free(filename);
		}
		break;
	}
		
	for (child_node=node->children; child_node; child_node=child_node->next) {
		if (should_appear_in_overview(cong_tree_view,
					   child_node)) {
			GtkTreeIter child_tree_iter;
			gtk_tree_store_append (cong_tree_view->private->gtk_tree_store, &child_tree_iter, tree_iter);

			populate_tree_store_recursive(cong_tree_view,
						      child_node, 
						      &child_tree_iter);
		}
	}
}

CongTreeView *cong_tree_view_new(CongDocument *doc)
{
	CongTreeView *cong_tree_view;
	CongTreeViewDetails *details;

 	GtkTreeViewColumn *column;
 	GtkCellRenderer *renderer; 

	GtkTreeIter root_iter;

	g_return_val_if_fail(doc, NULL);

	cong_tree_view = g_new0(CongTreeView, 1);
	details = g_new0(CongTreeViewDetails,1);
	
	cong_tree_view->private = details;

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

	cong_document_register_view( doc, CONG_VIEW(cong_tree_view) );

        details->gtk_tree_store = gtk_tree_store_new (CONG_OVERVIEW_TREEMODEL_N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING);

	details->gtk_tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model (GTK_TREE_MODEL(details->gtk_tree_store)));

	g_object_set_data(G_OBJECT(details->gtk_tree_view),
			  "cong_tree_view",
			  cong_tree_view);

	/* A single-columned treeview: */
	column = gtk_tree_view_column_new();

	/* Add a pixbuf-renderer to the column: */
	renderer = gtk_cell_renderer_pixbuf_new ();		
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
 	gtk_tree_view_column_set_cell_data_func (column, renderer, set_pixbuf, cong_tree_view, NULL);

	/* Add a text renderer to the column: */
	renderer = gtk_cell_renderer_text_new ();		
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column,
					    renderer,
					    "text", CONG_OVERVIEW_TREEMODEL_TITLE_COLUMN,
					    "foreground", CONG_OVERVIEW_TREEMODEL_FOREGROUND_COLOR_COLUMN,
					    "background", CONG_OVERVIEW_TREEMODEL_BACKGROUND_COLOR_COLUMN,
					    NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (details->gtk_tree_view), column);

 	/* Wire up the context-menu callback */
 	gtk_signal_connect_object(GTK_OBJECT(details->gtk_tree_view), "event",
 				  (GtkSignalFunc) tree_popup_show, details->gtk_tree_view);

#if 0
	cong_tree_view_populate_tree(cong_tree_view);
#else
	gtk_tree_store_append (details->gtk_tree_store, &root_iter, NULL);  /* Acquire a top-level iterator */
	populate_tree_store_recursive(cong_tree_view, (CongNodePtr)cong_document_get_xml(doc), &root_iter);
#endif

	gtk_widget_show(GTK_WIDGET(details->gtk_tree_view));

	return cong_tree_view;
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

	return GTK_WIDGET(tree_view->private->gtk_tree_view);
}

GtkTreeStore* cong_tree_view_get_tree_store(CongTreeView *tree_view)
{
	g_return_val_if_fail(tree_view, NULL);

	return tree_view->private->gtk_tree_store;
}
