/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include "global.h"

void tree_coarse_update_of_view()
{
	CongDocument *doc = the_globals.xv->doc;
	xmlview_destroy(FALSE);
	the_globals.xv = xmlview_new(doc);
	gtk_box_pack_start(GTK_BOX(cong_gui_get_root(&the_gui)), the_globals.xv->w, FALSE, FALSE, 0);
}

gint tree_new_sibling(GtkWidget *widget, CongNodePtr tag)
{
	CongNodePtr n0, n1, n2;
	char *s;

	CongDocument *doc = the_globals.xv->doc;
	CongDispspec *ds = cong_document_get_dispspec(doc);
	
	s = pick_structural_tag(ds);
	if (!s) return(TRUE);

	/* GREP FOR MVC */

#if NEW_XML_IMPLEMENTATION
	n0 = cong_node_new_text(" ");
	cong_document_node_add_after(doc, n0, tag);

	n1 = cong_node_new_element(s);
	cong_document_node_set_parent(doc, n1, tag->parent);

	n2 = cong_node_new_text(" ");
	cong_document_node_set_parent(doc, n2, n1);
#else
	n0 = ttree_node_add(tag->parent, "data", 4);
	ttree_node_add(n0, " ", 1);

	n1 = ttree_node_add(tag->parent, "tag_span", 8);
	n2 = ttree_node_add(n1, s, strlen(s));
	n2 = ttree_node_add(n2, "data", 4);
	ttree_node_add(n2, " ", 1);

	if (n0->prev) n0->prev->next = 0;
	if (tag->next) tag->next->prev = n1;
	n1->next = tag->next;
	n0->prev = tag;
	tag->next = n0;
#endif

	tree_coarse_update_of_view();

	return(TRUE);
}


gint tree_new_sub_element(GtkWidget *widget, CongNodePtr tag)
{
	CongNodePtr n0;
	CongNodePtr n1;
	char *s;

	CongDocument *doc = the_globals.xv->doc;
	CongDispspec *ds = cong_document_get_dispspec(doc);

	s = pick_structural_tag(ds);
	if (!s) return(TRUE);

	/* GREP FOR MVC */

#if NEW_XML_IMPLEMENTATION
	n0 = cong_node_new_element(s);
	cong_document_node_set_parent(doc, n0, tag);

	n1 = cong_node_new_text(" ");
	cong_document_node_set_parent(doc, n1, n0);

	n0 = cong_node_new_text(" ");
	cong_document_node_set_parent(doc, n0, tag);
#else	
	n0 = ttree_node_add(tag->child, "tag_span", 8);
	n1 = ttree_node_add(n0, s, strlen(s));
	n1 = ttree_node_add(n1, "data", 4);
	ttree_node_add(n1, " ", 1);
	
	n0 = ttree_node_add(tag->child, "data", 4);
	ttree_node_add(n0, " ", 1);
#endif

	tree_coarse_update_of_view();

	return(TRUE);
}


gint tree_cut(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc = the_globals.xv->doc;

	/* GREP FOR MVC */

	if (the_globals.clipboard) cong_node_recursive_delete(NULL, the_globals.clipboard);
	the_globals.clipboard = cong_node_recursive_dup(tag);
	cong_node_recursive_delete(doc, tag);

	tree_coarse_update_of_view();

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
	CongDocument *doc = the_globals.xv->doc;
	CongDispspec *ds = cong_document_get_dispspec(doc);
#if NEW_XML_IMPLEMENTATION
	CongNodePtr new_copy;
#endif

	if (!the_globals.clipboard) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(tag))) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(the_globals.clipboard))) return(TRUE);

	/* GREP FOR MVC */

#if NEW_XML_IMPLEMENTATION
	new_copy = cong_node_recursive_dup(the_globals.clipboard);

	cong_document_node_set_parent(doc, new_copy,tag);
#else
	
	the_globals.clipboard->prev = 0;
	the_globals.clipboard->next = tag->child->child;

	if (tag->child->child)
		tag->child->child->prev = the_globals.clipboard;
	
	tag->child->child = the_globals.clipboard;
	the_globals.clipboard->parent = tag->child;
	
	the_globals.clipboard = cong_node_recursive_dup(the_globals.clipboard);
#endif

	tree_coarse_update_of_view();

	return(TRUE);
}


gint tree_paste_before(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc = the_globals.xv->doc;
	CongDispspec *ds = cong_document_get_dispspec(doc);
#if NEW_XML_IMPLEMENTATION
	CongNodePtr new_copy;
#endif

	if (!the_globals.clipboard) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(tag))) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(the_globals.clipboard))) return(TRUE);

	/* GREP FOR MVC */

#if NEW_XML_IMPLEMENTATION
	new_copy = cong_node_recursive_dup(the_globals.clipboard);

	cong_document_node_add_before(doc, new_copy,tag);
#else
	if (tag->prev)
	{
		tag->prev->next = the_globals.clipboard;
		the_globals.clipboard->prev = tag->prev;
		tag->prev = the_globals.clipboard;
	}
	else if (tag->parent)
	{
		the_globals.clipboard->prev = 0;
		tag->parent->child = the_globals.clipboard;
	}

	the_globals.clipboard->next = tag;
	the_globals.clipboard->parent = tag->parent;

	the_globals.clipboard = ttree_branch_dup(the_globals.clipboard);
#endif
	
	tree_coarse_update_of_view();

	return(TRUE);
}


gint tree_paste_after(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc = the_globals.xv->doc;
	CongDispspec *ds = cong_document_get_dispspec(doc);
#if NEW_XML_IMPLEMENTATION
	CongNodePtr new_copy;
#endif

	if (!the_globals.clipboard) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(tag))) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(the_globals.clipboard))) return(TRUE);

	/* GREP FOR MVC */

#if NEW_XML_IMPLEMENTATION
	new_copy = cong_node_recursive_dup(the_globals.clipboard);

	cong_document_node_add_after(doc, new_copy,tag);
#else

	if (tag->next)
	{
		tag->next->prev = the_globals.clipboard;
	}
	
	the_globals.clipboard->next = tag->next;
	tag->next = the_globals.clipboard;
	the_globals.clipboard->prev = tag;
	the_globals.clipboard->parent = tag->parent;
	
	the_globals.clipboard = ttree_branch_dup(the_globals.clipboard);
#endif
	
	tree_coarse_update_of_view();

	return(TRUE);
}


