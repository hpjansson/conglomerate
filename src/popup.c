/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include <ttree.h>
#include <xml.h>
#include "global.h"

#if 1
static gint popup_item_selected(GtkWidget *widget, CongDispspecElement *element)
#else
static gint popup_item_selected(GtkWidget *widget, TTREE *tag)
#endif
{
	TTREE *dummy, *n, *r;

#ifndef RELEASE
	printf("Inserting tag (%s).\n", tag->data);
#endif
	
	dummy = ttree_node_add(0, "d", 1);
	n = ttree_node_add(dummy, "tag_span", 8);
#if 1
	ttree_node_add(n, cong_dispspec_element_tagname(element), strlen(cong_dispspec_element_tagname(element))+1);
#else
	ttree_node_add(n, tag->data, tag->size);
#endif
	n->parent = 0;
	dummy->child = 0;
	ttree_branch_remove(dummy);

#if 1
	if (the_globals.selection.loc0.tt_loc == the_globals.curs.xed->x)
#else
	if (the_globals.selection.t0 == the_globals.curs.xed->x)
#endif
	{
		r = selection_reparent_all(&the_globals.selection, n);
		if (r) the_globals.curs.xed->x = r;
		else ttree_branch_remove(n);
	}
	else if (!selection_reparent_all(&the_globals.selection, n)) ttree_branch_remove(n);

	xed_redraw(the_globals.curs.xed);
	the_globals.curs.xed = 0;

	return(TRUE);
}


void popup_item_handlers_destroy(GtkWidget *widget, gpointer data)
{
	UNUSED_VAR(int sig);

}


static gint popup_deactivate(GtkWidget *widget, GdkEvent *event)
{
	UNUSED_VAR(int sig);

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
	if (cong_gui_get_popup(&the_gui)) gtk_widget_destroy(cong_gui_get_popup(&the_gui));
	cong_gui_set_popup(&the_gui,gtk_menu_new());

	gtk_menu_set_title(GTK_MENU(cong_gui_get_popup(&the_gui)), "Editing menu");
}


void popup_tag_remove_inner()
{
	TTREE *n0;

	CongDocument *doc = the_globals.xv->doc;
	CongDispspec *ds = cong_document_get_dispspec(doc);
	
#if 1
	if (!cong_location_exists(&the_globals.curs.location)) return;
#else
	if (!the_globals.curs.t) return;
#endif

	n0 = xml_inner_span_element(ds, the_globals.curs.location.tt_loc);
	if (n0) xml_tag_remove(n0);
	
	if (the_globals.curs.xed) xed_redraw(the_globals.curs.xed);
}

void popup_tag_remove_outer()
{
	TTREE *n0;

	CongDocument *doc = the_globals.xv->doc;
	CongDispspec *ds = cong_document_get_dispspec(doc);
	
#if 1
	if (!cong_location_exists(&the_globals.curs.location)) return;
#else
	if (!the_globals.curs.t) return;
#endif
	
	n0 = xml_outer_span_element(ds, the_globals.curs.location.tt_loc);
	if (n0) xml_tag_remove(n0);
	
	if (the_globals.curs.xed) xed_redraw(the_globals.curs.xed);
}


void popup_build(CongXMLEditor *xed)
{
	GtkWidget *item, *w0;
#if 1
	CongDispspecElement *n0;
#else
	TTREE *n0;
#endif
	UNUSED_VAR(TTREE *n1);

	if (cong_gui_get_popup(&the_gui)) gtk_widget_destroy(cong_gui_get_popup(&the_gui));
	popup_init();
	
#ifndef RELEASE
	printf("Building menu.\n");
#endif
	
	/* Fixed editing tools */

	item = gtk_menu_item_new_with_label("Cut");
	gtk_menu_append(GTK_MENU(cong_gui_get_popup(&the_gui)), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
										 GTK_SIGNAL_FUNC(xed_cut), xed);
	gtk_widget_show(item);

	item = gtk_menu_item_new_with_label("Copy");
	gtk_menu_append(GTK_MENU(cong_gui_get_popup(&the_gui)), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
										 GTK_SIGNAL_FUNC(xed_copy), xed);
	gtk_widget_show(item);
	
	item = gtk_menu_item_new_with_label("Paste");
	gtk_menu_append(GTK_MENU(cong_gui_get_popup(&the_gui)), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
										 GTK_SIGNAL_FUNC(xed_paste), xed);
	gtk_widget_show(item);

	if (/* the_globals.curs.set && */ the_globals.curs.location.tt_loc && xml_inner_span_element(xed->displayspec, the_globals.curs.location.tt_loc))
	{
	  item = gtk_menu_item_new();
	  w0 = gtk_hseparator_new();
	  gtk_container_add(GTK_CONTAINER(item), w0);
	  gtk_menu_append(GTK_MENU(cong_gui_get_popup(&the_gui)), item);
	  gtk_widget_set_sensitive(item, 0);
	  gtk_widget_show(w0);
	  gtk_widget_show(item);

	  item = gtk_menu_item_new_with_label("Remove inner tag");
	  gtk_menu_append(GTK_MENU(cong_gui_get_popup(&the_gui)), item);
	  gtk_signal_connect(GTK_OBJECT(item), "activate",
	  									 GTK_SIGNAL_FUNC(popup_tag_remove_inner), xed);
	  gtk_widget_show(item);

	  item = gtk_menu_item_new_with_label("Remove outer tag");
	  gtk_menu_append(GTK_MENU(cong_gui_get_popup(&the_gui)), item);
	  gtk_signal_connect(GTK_OBJECT(item), "activate",
	  									 GTK_SIGNAL_FUNC(popup_tag_remove_outer), xed);
	  gtk_widget_show(item);
	}
	
	item = gtk_menu_item_new();
	w0 = gtk_hseparator_new();
	gtk_container_add(GTK_CONTAINER(item), w0);
	gtk_menu_append(GTK_MENU(cong_gui_get_popup(&the_gui)), item);
	gtk_widget_set_sensitive(item, 0);
	gtk_widget_show(w0);
	gtk_widget_show(item);

	/* Build list of dynamic tag insertion tools */
#if 1
	for (n0 = cong_dispspec_get_first_element(xed->displayspec); n0; n0 = cong_dispspec_element_next(n0))
	{
		if (cong_dispspec_element_is_span(n0))
		{
			item = gtk_menu_item_new_with_label(cong_dispspec_element_username(n0));
			gtk_menu_append(GTK_MENU(cong_gui_get_popup(&the_gui)), item);
			
			gtk_signal_connect(GTK_OBJECT(item), "activate",
					   GTK_SIGNAL_FUNC(popup_item_selected), n0);
			
			gtk_widget_show(item);
		}
	}
#else
	for (n0 = cong_dispspec_ttree(the_globals.ds)->child; n0; n0 = n0->next)
	{
		if (ttree_branch_walk_str(n0, "type span"))
		{
			item = gtk_menu_item_new_with_label(cong_dispspec_name_name_get(n0));
			gtk_menu_append(GTK_MENU(cong_gui_get_popup(&the_gui)), item);
			
			gtk_signal_connect(GTK_OBJECT(item), "activate",
												 GTK_SIGNAL_FUNC(popup_item_selected), n0);
			
			gtk_widget_show(item);
		}
	}
#endif
}


gint tpopup_show(GtkWidget *widget, GdkEvent *event)
{
  if (event->type == GDK_BUTTON_PRESS)
	{
		GdkEventButton *bevent = (GdkEventButton *) event;
		if (bevent->button != 3) return(FALSE);
		
 		#if 1
 		printf("button 3\n");
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
 
 		    GtkTreeIter iter;
 		    GtkTreeModel* tree_model = GTK_TREE_MODEL(cong_gui_get_tree_store(&the_gui));
#if 0
 		    gchar* msg = gtk_tree_path_to_string(path);
 		    printf("right-click on path \"%s\"\n",msg);
 		    g_free(msg);
#endif
		    
 		    if ( gtk_tree_model_get_iter(tree_model, &iter, path) ) {
 		      CongNodePtr tt;
 		      GtkWidget* menu;
		      CongDocument* doc;
 
 		      gtk_tree_model_get(tree_model, &iter, TREEVIEW_NODE_COLUMN, &tt, -1);
 		      gtk_tree_model_get(tree_model, &iter, TREEVIEW_DOC_COLUMN, &doc, -1);
 
 		      printf("got node \"%s\"\n",cong_dispspec_name_get(cong_document_get_dispspec(doc), tt));
 
 		      menu = tpopup_init(tt);
 		      gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, bevent->button,
 				     bevent->time);		      
 		    }
 
 				 
 		  gtk_tree_path_free(path);		  
 		  }
 		}
#else
		gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL, bevent->button,
									 bevent->time);
		#endif
		return(TRUE);
	}

	return(FALSE);
}

#if 1
GtkWidget* tpopup_init(TTREE *x)
#else
void tpopup_init(GtkWidget *treeitem, TTREE *x)
#endif
{
	GtkWidget *item, *tpopup, *w0;

	tpopup = gtk_menu_new();
	gtk_menu_set_title(GTK_MENU(tpopup), "Structure menu");

#if 0
	gtk_signal_connect_object(GTK_OBJECT(treeitem), "event",
				  (GtkSignalFunc) tpopup_show, GTK_OBJECT(tpopup));
#endif
	
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

	#if 1
	return tpopup;
	#endif
}

