/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include "global.h"

/* 
   The popup menu widget (and some items) have a pointer to the xed widget set as a user property named "xed".
*/

static gint popup_item_selected(GtkWidget *widget, CongDispspecElement *element)
{
	CongNodePtr new_element;
	CongNodePtr r;

	CongXMLEditor *xed;
	CongDocument *doc;
	CongSelection *selection;
	CongCursor *cursor;

	g_return_val_if_fail(element, TRUE);

	xed = g_object_get_data(G_OBJECT(widget),
				"xed");
	g_assert(xed);

	doc = xed->doc;
	selection = cong_document_get_selection(doc);
	cursor = cong_document_get_cursor(doc);

	/* GREP FOR MVC */


#ifndef RELEASE
	printf("Inserting tag (%s).\n", tag->data);
#endif

	new_element = cong_node_new_element(cong_dispspec_element_tagname(element));

	g_assert(cursor->xed);

	if (selection->loc0.tt_loc == cursor->xed->x)
	{
		r = cong_selection_reparent_all(selection, new_element);
		if (r) {
			cursor->xed->x = r;
		} else {
			cong_node_free(new_element);
		}
	}
	else if (!cong_selection_reparent_all(selection, new_element)) {
		cong_node_free(new_element);
	}

	xed_redraw(cursor->xed);
	cursor->xed = NULL;

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

void popup_init(CongXMLEditor *xed)
{
	if (the_globals.popup) gtk_widget_destroy(the_globals.popup);
	the_globals.popup = gtk_menu_new();

	g_object_set_data(G_OBJECT(the_globals.popup),
			  "xed",
			  xed);

	gtk_menu_set_title(GTK_MENU(the_globals.popup), "Editing menu");
}

void popup_tag_remove_inner(GtkWidget *widget, CongXMLEditor *xed)
{
	CongNodePtr n0;

	CongDocument *doc = xed->doc;
	CongDispspec *ds = cong_document_get_dispspec(doc);
	CongCursor *cursor = cong_document_get_cursor(doc);
	
	if (!cong_location_exists(&cursor->location)) return;

	n0 = xml_inner_span_element(ds, cursor->location.tt_loc);

	/* GREP FOR MVC */

	if (n0) cong_document_tag_remove(doc, n0);
	if (cursor->xed) xed_redraw(cursor->xed);
}

void popup_tag_remove_outer(GtkWidget *widget, CongXMLEditor *xed)
{
	CongNodePtr n0;

	CongDocument *doc = xed->doc;
	CongDispspec *ds = cong_document_get_dispspec(doc);
	CongCursor *cursor = cong_document_get_cursor(doc);
	
	if (!cong_location_exists(&cursor->location)) return;
	
	n0 = xml_outer_span_element(ds, cursor->location.tt_loc);

	/* GREP FOR MVC */

	if (n0) cong_document_tag_remove(doc, n0);
	if (cursor->xed) xed_redraw(cursor->xed);
}

void popup_build(CongXMLEditor *xed)
{
	GtkWidget *item, *w0;
	CongDispspecElement *n0;
	CongCursor *cursor;
	
	g_return_if_fail(xed);

	g_assert(xed->doc);
	cursor = cong_document_get_cursor(xed->doc);

	if (the_globals.popup) gtk_widget_destroy(the_globals.popup);
	popup_init(xed);
	
#ifndef RELEASE
	printf("Building menu.\n");
#endif
	
	/* Fixed editing tools */

	item = gtk_menu_item_new_with_label("Cut");
	gtk_menu_append(GTK_MENU(the_globals.popup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(xed_cut), xed);
	gtk_widget_show(item);

	item = gtk_menu_item_new_with_label("Copy");
	gtk_menu_append(GTK_MENU(the_globals.popup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(xed_copy), xed);
	gtk_widget_show(item);
	
	item = gtk_menu_item_new_with_label("Paste");
	gtk_menu_append(GTK_MENU(the_globals.popup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(xed_paste), xed);
	gtk_widget_show(item);

	if (/* cursor->set && */ cursor->location.tt_loc && xml_inner_span_element(xed->displayspec, cursor->location.tt_loc))
	{
		item = gtk_menu_item_new();
		w0 = gtk_hseparator_new();
		gtk_container_add(GTK_CONTAINER(item), w0);
		gtk_menu_append(GTK_MENU(the_globals.popup), item);
		gtk_widget_set_sensitive(item, 0);
		gtk_widget_show(w0);
		gtk_widget_show(item);
		
		item = gtk_menu_item_new_with_label("Remove inner tag");
		gtk_menu_append(GTK_MENU(the_globals.popup), item);
		gtk_signal_connect(GTK_OBJECT(item), "activate",
				   GTK_SIGNAL_FUNC(popup_tag_remove_inner), xed);
		gtk_widget_show(item);
		
		item = gtk_menu_item_new_with_label("Remove outer tag");
		gtk_menu_append(GTK_MENU(the_globals.popup), item);
		gtk_signal_connect(GTK_OBJECT(item), "activate",
				   GTK_SIGNAL_FUNC(popup_tag_remove_outer), xed);
		gtk_widget_show(item);
	}
	
	item = gtk_menu_item_new();
	w0 = gtk_hseparator_new();
	gtk_container_add(GTK_CONTAINER(item), w0);
	gtk_menu_append(GTK_MENU(the_globals.popup), item);
	gtk_widget_set_sensitive(item, 0);
	gtk_widget_show(w0);
	gtk_widget_show(item);

	/* Build list of dynamic tag insertion tools */
	for (n0 = cong_dispspec_get_first_element(xed->displayspec); n0; n0 = cong_dispspec_element_next(n0))
	{
		if (cong_dispspec_element_is_span(n0))
		{
			item = gtk_menu_item_new_with_label(cong_dispspec_element_username(n0));
			gtk_menu_append(GTK_MENU(the_globals.popup), item);
			
			gtk_signal_connect(GTK_OBJECT(item), "activate",
					   GTK_SIGNAL_FUNC(popup_item_selected), n0);
			
			g_object_set_data(G_OBJECT(item),
					  "xed",
					  xed);
			gtk_widget_show(item);
		}
	}

}

/* the treeview widget has the userdata "cong_tree_view" set on it */
gint tpopup_show(GtkWidget *widget, GdkEvent *event)
{
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
					CongNodePtr tt;
					GtkWidget* menu;
					CongDocument* doc;
					
					gtk_tree_model_get(tree_model, &iter, TREEVIEW_NODE_COLUMN, &tt, -1);
					gtk_tree_model_get(tree_model, &iter, TREEVIEW_DOC_COLUMN, &doc, -1);
					
					printf("got node \"%s\"\n",cong_dispspec_name_get(cong_document_get_dispspec(doc), tt));
					
					menu = tpopup_init(cong_tree_view, tt);
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

/* the popup items have the data "cong_tree_view" set on them: */
void add_item_to_popup(GtkMenu *menu,
		       const gchar *label,
		       gint (*func)(GtkWidget *widget, CongNodePtr tag),
		       CongTreeView *cong_tree_view,
		       CongNodePtr x)
{
	GtkWidget *item = gtk_menu_item_new_with_label(label);
	gtk_menu_append(menu, item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(func), x);
	g_object_set_data(G_OBJECT(item),
			  "cong_tree_view",
			  cong_tree_view);
	gtk_widget_show(item);
}

GtkWidget* tpopup_init(CongTreeView *cong_tree_view, CongNodePtr x)
{
	GtkMenu *tpopup;
	GtkWidget *item, *w0;

	tpopup = GTK_MENU(gtk_menu_new());
	gtk_menu_set_title(GTK_MENU(tpopup), "Structure menu");
	
#if 1
	add_item_to_popup(tpopup,
			  "Cut",
			  tree_cut,
			  cong_tree_view,
			  x);
	add_item_to_popup(tpopup,
			  "Copy",
			  tree_copy,
			  cong_tree_view,
			  x);
	add_item_to_popup(tpopup,
			  "Paste into",
			  tree_paste_under,
			  cong_tree_view,
			  x);
	add_item_to_popup(tpopup,
			  "Paste before",
			  tree_paste_before,
			  cong_tree_view,
			  x);
	add_item_to_popup(tpopup,
			  "Paste after",
			  tree_paste_after,
			  cong_tree_view,
			  x);
#else
	item = gtk_menu_item_new_with_label("Cut");
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(tree_cut), x);
	g_object_set_data(G_OBJECT(item),
			  "cong_tree_view",
			  cong_tree_view);
	gtk_widget_show(item);

	item = gtk_menu_item_new_with_label("Copy");
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(tree_copy), x);
	g_object_set_data(G_OBJECT(item),
			  "cong_tree_view",
			  cong_tree_view);
	gtk_widget_show(item);

	item = gtk_menu_item_new_with_label("Paste into");
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(tree_paste_under), x);
	g_object_set_data(G_OBJECT(item),
			  "cong_tree_view",
			  cong_tree_view);
	gtk_widget_show(item);

	item = gtk_menu_item_new_with_label("Paste before");
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(tree_paste_before), x);
	g_object_set_data(G_OBJECT(item),
			  "cong_tree_view",
			  cong_tree_view);
	gtk_widget_show(item);

	item = gtk_menu_item_new_with_label("Paste after");
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(tree_paste_after), x);
	g_object_set_data(G_OBJECT(item),
			  "cong_tree_view",
			  cong_tree_view);
	gtk_widget_show(item);
#endif

	item = gtk_menu_item_new();
	w0 = gtk_hseparator_new();
	gtk_container_add(GTK_CONTAINER(item), w0);
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_widget_set_sensitive(item, 0);
	gtk_widget_show(w0);
	gtk_widget_show(item);

#if 1
	add_item_to_popup(tpopup,
			  "New sub-element",
			  tree_new_sub_element,
			  cong_tree_view,
			  x);
	add_item_to_popup(tpopup,
			  "New sibling",
			  tree_new_sibling,
			  cong_tree_view,
			  x);
#else
	item = gtk_menu_item_new_with_label("New sub-element");
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(tree_new_sub_element), x);
	g_object_set_data(G_OBJECT(item),
			  "cong_tree_view",
			  cong_tree_view);
	gtk_widget_show(item);

	item = gtk_menu_item_new_with_label("New sibling");
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(tree_new_sibling), x);
	g_object_set_data(G_OBJECT(item),
			  "cong_tree_view",
			  cong_tree_view);
	gtk_widget_show(item);
#endif

	return GTK_WIDGET(tpopup);
}
