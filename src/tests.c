/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * tests.c
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

#if !NEW_XML_IMPLEMENTATION
void add_node_recursive(TTREE *tt, GtkTreeStore *store, GtkTreeIter *parent_iter)
{
	GtkTreeIter child_iter;  /* Child iter  */

	gtk_tree_store_append (store, &child_iter, parent_iter);  /* Acquire a child iterator */

	gtk_tree_store_set (store, &child_iter,
			    0, tt->data,
			    -1); 
  
	/* Recurse over children: */
	{
		TTREE *child_tt = tt->child;
		while (child_tt != NULL) {
			add_node_recursive(child_tt, store, &child_iter);
			child_tt=child_tt->next;
		}
	}
 
}

GtkWidget* do_ttree_test(TTREE* tt)
{
	GtkTreeStore *store = gtk_tree_store_new (1, G_TYPE_STRING);
	GtkWidget *tree;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	g_return_val_if_fail(tt, NULL);
 
	add_node_recursive(tt,store,NULL);

	tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Author",
							   renderer,
							   "text", 0,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
	
	gtk_widget_show(tree);
	
	return tree;
}
#endif /* #if !NEW_XML_IMPLEMENTATION */
