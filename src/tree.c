#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include <ttree.h>
#include <xml.h>
#include "global.h"


gint tree_new_sibling(GtkWidget *widget, TTREE *tag)
{
	TTREE *n0, *n1, *n2, *x;
  char *s;
	
  s = pick_structural_tag();
	if (!s) return(TRUE);

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

	x = xv->x;
	xmlview_destroy(FALSE);
	xv = xmlview_new(x, ds_global);
  gtk_box_pack_start(GTK_BOX(root), xv->w, FALSE, FALSE, 0);

	return(TRUE);
}


gint tree_new_sub_element(GtkWidget *widget, TTREE *tag)
{
	TTREE *n0, *n1, *x;
	char *s;

  s = pick_structural_tag();
	if (!s) return(TRUE);
	
	n0 = ttree_node_add(tag->child, "tag_span", 8);
	n1 = ttree_node_add(n0, s, strlen(s));
  n1 = ttree_node_add(n1, "data", 4);
	ttree_node_add(n1, " ", 1);
	
	n0 = ttree_node_add(tag->child, "data", 4);
	ttree_node_add(n0, " ", 1);

	x = xv->x;
	xmlview_destroy(FALSE);
	xv = xmlview_new(x, ds_global);
  gtk_box_pack_start(GTK_BOX(root), xv->w, FALSE, FALSE, 0);
	
	return(TRUE);
}


gint tree_cut(GtkWidget *widget, TTREE *tag)
{
	TTREE *x;

	if (clipboard) ttree_branch_remove(clipboard);
	clipboard = ttree_branch_dup(tag);
	ttree_branch_remove(tag);

	x = xv->x;
	xmlview_destroy(FALSE);
	xv = xmlview_new(x, ds_global);
  gtk_box_pack_start(GTK_BOX(root), xv->w, FALSE, FALSE, 0);

	return(TRUE);
}


gint tree_copy(GtkWidget *widget, TTREE *tag)
{
	if (clipboard) ttree_branch_remove(clipboard);
	clipboard = ttree_branch_dup(tag);
	
	return(TRUE);
}


gint tree_paste_under(GtkWidget *widget, TTREE *tag)
{
	TTREE *x;

	if (!clipboard) return(TRUE);
	if (!ds_element_structural(ds_global, xml_frag_name_nice(tag))) return(TRUE);
	if (!ds_element_structural(ds_global, xml_frag_name_nice(clipboard))) return(TRUE);
	
	clipboard->prev = 0;
	clipboard->next = tag->child->child;

	if (tag->child->child)
		tag->child->child->prev = clipboard;
	
	tag->child->child = clipboard;
	clipboard->parent = tag->child;
	
	clipboard = ttree_branch_dup(clipboard);

	x = xv->x;
	xmlview_destroy(FALSE);
	xv = xmlview_new(x, ds_global);
  gtk_box_pack_start(GTK_BOX(root), xv->w, FALSE, FALSE, 0);

	return(TRUE);
}


gint tree_paste_before(GtkWidget *widget, TTREE *tag)
{
	TTREE *x;
	
	if (!clipboard) return(TRUE);
	if (!ds_element_structural(ds_global, xml_frag_name_nice(tag))) return(TRUE);
	if (!ds_element_structural(ds_global, xml_frag_name_nice(clipboard))) return(TRUE);
	
	if (tag->prev)
	{
		tag->prev->next = clipboard;
		clipboard->prev = tag->prev;
		tag->prev = clipboard;
	}
	else if (tag->parent)
	{
		clipboard->prev = 0;
		tag->parent->child = clipboard;
	}

	clipboard->next = tag;
	clipboard->parent = tag->parent;

	clipboard = ttree_branch_dup(clipboard);
	
	x = xv->x;
	xmlview_destroy(FALSE);
	xv = xmlview_new(x, ds_global);
  gtk_box_pack_start(GTK_BOX(root), xv->w, FALSE, FALSE, 0);

	return(TRUE);
}


gint tree_paste_after(GtkWidget *widget, TTREE *tag)
{
	TTREE *x;
	
	if (!clipboard) return(TRUE);
	if (!ds_element_structural(ds_global, xml_frag_name_nice(tag))) return(TRUE);
	if (!ds_element_structural(ds_global, xml_frag_name_nice(clipboard))) return(TRUE);
	
	if (tag->next)
	{
		tag->next->prev = clipboard;
	}
	
	clipboard->next = tag->next;
	tag->next = clipboard;
	clipboard->prev = tag;
	clipboard->parent = tag->parent;
	
	clipboard = ttree_branch_dup(clipboard);
	
	x = xv->x;
	xmlview_destroy(FALSE);
	xv = xmlview_new(x, ds_global);
  gtk_box_pack_start(GTK_BOX(root), xv->w, FALSE, FALSE, 0);

	return(TRUE);
}


