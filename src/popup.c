#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include <ttree.h>
#include <xml.h>
#include "global.h"


static gint popup_item_selected(GtkWidget *widget, TTREE *tag)
{
	TTREE *dummy, *n, *r;

#ifndef RELEASE
	printf("Inserting tag (%s).\n", tag->data);
#endif
	
	dummy = ttree_node_add(0, "d", 1);
	n = ttree_node_add(dummy, "tag_span", 8);
	ttree_node_add(n, tag->data, tag->size);
	n->parent = 0;
	dummy->child = 0;
	ttree_branch_remove(dummy);

	if (selection.t0 == curs.xed->x)
	{
		r = selection_reparent_all(n);
		if (r) curs.xed->x = r;
		else ttree_branch_remove(n);
	}
	else if (!selection_reparent_all(n)) ttree_branch_remove(n);

	xed_redraw(curs.xed);
	curs.xed = 0;

	return(TRUE);
}


void popup_item_handlers_destroy(GtkWidget *widget, gpointer data)
{
	int sig;

}


static gint popup_deactivate(GtkWidget *widget, GdkEvent *event)
{
	int sig;

#ifndef RELEASE
  printf("Menu deactivated.\n");
#endif
	
#if 0
	
#if 1
	gtk_container_foreach(GTK_CONTAINER(widget), popup_item_handlers_destroy, 0);
	gtk_signal_handlers_destroy(GTK_OBJECT(widget));
#else
	sig = gtk_signal_lookup("activate", GTK_MENUITEM);
	gtk_signal_handler_block(GTK_OBJECT(widget), sig);
#endif
	gtk_widget_destroy(widget);

#ifndef RELEASE
  printf("Menu destroyed.\n");
#endif
	popup_init();

#endif	
	
	return(FALSE);
}


void popup_show(GtkWidget *widget, GdkEventButton *bevent)
{
	gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL, bevent->button,
								 bevent->time);
	
	return;
}


void popup_init()
{
	if (popup) gtk_widget_destroy(popup);
  popup = gtk_menu_new();

	gtk_menu_set_title(GTK_MENU(popup), "Editing menu");
}


void popup_tag_remove_inner()
{
	TTREE *n0;
	
  if (!curs.t) return;

	n0 = xml_inner_span_element(curs.t);
	if (n0) xml_tag_remove(n0);
	
	if (curs.xed) xed_redraw(curs.xed);
}

void popup_tag_remove_outer()
{
	TTREE *n0;
	
  if (!curs.t) return;
	
	n0 = xml_outer_span_element(curs.t);
	if (n0) xml_tag_remove(n0);
	
	if (curs.xed) xed_redraw(curs.xed);
}


void popup_build(struct xed *xed)
{
	GtkWidget *item, *w0;
	TTREE *n0, *n1;

	if (popup) gtk_widget_destroy(popup);
	popup_init();
	
#ifndef RELEASE
	printf("Building menu.\n");
#endif
	
	/* Fixed editing tools */

	item = gtk_menu_item_new_with_label("Cut");
	gtk_menu_append(GTK_MENU(popup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
										 GTK_SIGNAL_FUNC(xed_cut), xed);
	gtk_widget_show(item);

	item = gtk_menu_item_new_with_label("Copy");
	gtk_menu_append(GTK_MENU(popup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
										 GTK_SIGNAL_FUNC(xed_copy), xed);
	gtk_widget_show(item);
	
	item = gtk_menu_item_new_with_label("Paste");
	gtk_menu_append(GTK_MENU(popup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
										 GTK_SIGNAL_FUNC(xed_paste), xed);
	gtk_widget_show(item);

	if (/* curs.set && */ curs.t && xml_inner_span_element(curs.t))
	{
	  item = gtk_menu_item_new();
	  w0 = gtk_hseparator_new();
	  gtk_container_add(GTK_CONTAINER(item), w0);
	  gtk_menu_append(GTK_MENU(popup), item);
	  gtk_widget_set_sensitive(item, 0);
	  gtk_widget_show(w0);
	  gtk_widget_show(item);

	  item = gtk_menu_item_new_with_label("Remove inner tag");
	  gtk_menu_append(GTK_MENU(popup), item);
	  gtk_signal_connect(GTK_OBJECT(item), "activate",
	  									 GTK_SIGNAL_FUNC(popup_tag_remove_inner), xed);
	  gtk_widget_show(item);

	  item = gtk_menu_item_new_with_label("Remove outer tag");
	  gtk_menu_append(GTK_MENU(popup), item);
	  gtk_signal_connect(GTK_OBJECT(item), "activate",
	  									 GTK_SIGNAL_FUNC(popup_tag_remove_outer), xed);
	  gtk_widget_show(item);
	}
	
	item = gtk_menu_item_new();
	w0 = gtk_hseparator_new();
	gtk_container_add(GTK_CONTAINER(item), w0);
	gtk_menu_append(GTK_MENU(popup), item);
	gtk_widget_set_sensitive(item, 0);
	gtk_widget_show(w0);
	gtk_widget_show(item);

	/* Build list of dynamic tag insertion tools */

	for (n0 = ds_global->child; n0; n0 = n0->next)
	{
		if (ttree_branch_walk_str(n0, "type span"))
		{
			item = gtk_menu_item_new_with_label(ds_name_name_get(n0));
			gtk_menu_append(GTK_MENU(popup), item);
			
			gtk_signal_connect(GTK_OBJECT(item), "activate",
												 GTK_SIGNAL_FUNC(popup_item_selected), n0);
			
			gtk_widget_show(item);
		}
	}
}


gint tpopup_show(GtkWidget *widget, GdkEvent *event)
{
  if (event->type == GDK_BUTTON_PRESS)
	{
		GdkEventButton *bevent = (GdkEventButton *) event;
		if (bevent->button != 3) return(FALSE);
		
		gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL, bevent->button,
									 bevent->time);
		return(TRUE);
	}

	return(FALSE);
}


void tpopup_init(GtkWidget *treeitem, TTREE *x)
{
	GtkWidget *item, *tpopup, *w0;

	tpopup = gtk_menu_new();
	gtk_menu_set_title(GTK_MENU(tpopup), "Structure menu");

  gtk_signal_connect_object(GTK_OBJECT(treeitem), "event",
														(GtkSignalFunc) tpopup_show, GTK_OBJECT(tpopup));
	
	/* Fixed editing tools */

	item = gtk_menu_item_new_with_label("Cut");
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
										 GTK_SIGNAL_FUNC(tree_cut), x);

	gtk_widget_show(item);

	item = gtk_menu_item_new_with_label("Copy");
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
										 GTK_SIGNAL_FUNC(tree_copy), x);

	gtk_widget_show(item);

	item = gtk_menu_item_new_with_label("Paste into");
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
										 GTK_SIGNAL_FUNC(tree_paste_under), x);

	gtk_widget_show(item);

	item = gtk_menu_item_new_with_label("Paste before");
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
										 GTK_SIGNAL_FUNC(tree_paste_before), x);

	gtk_widget_show(item);

	item = gtk_menu_item_new_with_label("Paste after");
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
										 GTK_SIGNAL_FUNC(tree_paste_after), x);

	gtk_widget_show(item);

	item = gtk_menu_item_new();
	w0 = gtk_hseparator_new();
	gtk_container_add(GTK_CONTAINER(item), w0);
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_widget_set_sensitive(item, 0);
	gtk_widget_show(w0);
	gtk_widget_show(item);

	item = gtk_menu_item_new_with_label("New sub-element");
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
										 GTK_SIGNAL_FUNC(tree_new_sub_element), x);

	gtk_widget_show(item);

	item = gtk_menu_item_new_with_label("New sibling");
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
										 GTK_SIGNAL_FUNC(tree_new_sibling), x);
	gtk_widget_show(item);
}

