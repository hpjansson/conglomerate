/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include "global.h"

void tree_coarse_update_of_view(CongTreeView *cong_tree_view)
{
#if 1
	CongDocument *doc = CONG_VIEW(cong_tree_view)->doc;

	cong_document_coarse_update(doc);
#else
	CongDocument *doc = the_globals.xv->doc;
	xmlview_destroy(FALSE);
	the_globals.xv = xmlview_new(doc);
	gtk_box_pack_start(GTK_BOX(cong_gui_get_root(&the_gui)), the_globals.xv->w, FALSE, FALSE, 0);
#endif
}

/* the popup items have the data "cong_tree_view" set on them: */

gint tree_new_sibling(GtkWidget *widget, CongNodePtr tag)
{
	CongTreeView *cong_tree_view;
	CongDocument *doc;
	CongDispspec *ds;

	CongNodePtr n0, n1, n2;
	char *s;

	cong_tree_view = g_object_get_data(G_OBJECT(widget),
					   "cong_tree_view");
	g_assert(cong_tree_view);				
	doc = CONG_VIEW(cong_tree_view)->doc;
	ds = cong_document_get_dispspec(doc);
	
	s = pick_structural_tag(ds);
	if (!s) return(TRUE);

	/* GREP FOR MVC */

	n0 = cong_node_new_text(" ");
	cong_document_node_add_after(doc, n0, tag);

	n1 = cong_node_new_element(s);
	cong_document_node_set_parent(doc, n1, tag->parent);

	n2 = cong_node_new_text(" ");
	cong_document_node_set_parent(doc, n2, n1);

	tree_coarse_update_of_view(cong_tree_view);

	return(TRUE);
}


gint tree_new_sub_element(GtkWidget *widget, CongNodePtr tag)
{
	CongTreeView *cong_tree_view;
	CongDocument *doc;
	CongDispspec *ds;

	CongNodePtr n0;
	CongNodePtr n1;
	char *s;

	cong_tree_view = g_object_get_data(G_OBJECT(widget),
					   "cong_tree_view");
	g_assert(cong_tree_view);				
	doc = CONG_VIEW(cong_tree_view)->doc;
	ds = cong_document_get_dispspec(doc);

	s = pick_structural_tag(ds);
	if (!s) return(TRUE);

	/* GREP FOR MVC */

	n0 = cong_node_new_element(s);
	cong_document_node_set_parent(doc, n0, tag);

	n1 = cong_node_new_text(" ");
	cong_document_node_set_parent(doc, n1, n0);

	n0 = cong_node_new_text(" ");
	cong_document_node_set_parent(doc, n0, tag);

	tree_coarse_update_of_view(cong_tree_view);

	return(TRUE);
}


gint tree_cut(GtkWidget *widget, CongNodePtr tag)
{
	CongTreeView *cong_tree_view;
	CongDocument *doc;
	CongDispspec *ds;

	cong_tree_view = g_object_get_data(G_OBJECT(widget),
					   "cong_tree_view");
	g_assert(cong_tree_view);				
	doc = CONG_VIEW(cong_tree_view)->doc;
	ds = cong_document_get_dispspec(doc);

	/* GREP FOR MVC */

	if (the_globals.clipboard) cong_node_recursive_delete(NULL, the_globals.clipboard);
	the_globals.clipboard = cong_node_recursive_dup(tag);
	cong_node_recursive_delete(doc, tag);

	tree_coarse_update_of_view(cong_tree_view);

	return(TRUE);
}


gint tree_copy(GtkWidget *widget, CongNodePtr tag)
{
	/* GREP FOR MVC */

	if (the_globals.clipboard) cong_node_recursive_delete(NULL, the_globals.clipboard);
	the_globals.clipboard = cong_node_recursive_dup(tag);

	return(TRUE);
}


gint tree_paste_under(GtkWidget *widget, CongNodePtr tag)
{
	CongTreeView *cong_tree_view;
	CongDocument *doc;
	CongDispspec *ds;

	CongNodePtr new_copy;

	cong_tree_view = g_object_get_data(G_OBJECT(widget),
					   "cong_tree_view");
	g_assert(cong_tree_view);				
	doc = CONG_VIEW(cong_tree_view)->doc;
	ds = cong_document_get_dispspec(doc);

	if (!the_globals.clipboard) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(tag))) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(the_globals.clipboard))) return(TRUE);

	/* GREP FOR MVC */

	new_copy = cong_node_recursive_dup(the_globals.clipboard);

	cong_document_node_set_parent(doc, new_copy,tag);

	tree_coarse_update_of_view(cong_tree_view);

	return(TRUE);
}


gint tree_paste_before(GtkWidget *widget, CongNodePtr tag)
{
	CongTreeView *cong_tree_view;
	CongDocument *doc;
	CongDispspec *ds;

	CongNodePtr new_copy;

	cong_tree_view = g_object_get_data(G_OBJECT(widget),
				"cong_tree_view");
	g_assert(cong_tree_view);				
	doc = CONG_VIEW(cong_tree_view)->doc;
	ds = cong_document_get_dispspec(doc);

	if (!the_globals.clipboard) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(tag))) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(the_globals.clipboard))) return(TRUE);

	/* GREP FOR MVC */

	new_copy = cong_node_recursive_dup(the_globals.clipboard);

	cong_document_node_add_before(doc, new_copy,tag);
	
	tree_coarse_update_of_view(cong_tree_view);

	return(TRUE);
}


gint tree_paste_after(GtkWidget *widget, CongNodePtr tag)
{
	CongTreeView *cong_tree_view;
	CongDocument *doc;
	CongDispspec *ds;

	CongNodePtr new_copy;

	cong_tree_view = g_object_get_data(G_OBJECT(widget),
				"cong_tree_view");
	g_assert(cong_tree_view);				
	doc = CONG_VIEW(cong_tree_view)->doc;
	ds = cong_document_get_dispspec(doc);

	if (!the_globals.clipboard) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(tag))) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(the_globals.clipboard))) return(TRUE);

	/* GREP FOR MVC */

	new_copy = cong_node_recursive_dup(the_globals.clipboard);

	cong_document_node_add_after(doc, new_copy,tag);
	
	tree_coarse_update_of_view(cong_tree_view);

	return(TRUE);
}

