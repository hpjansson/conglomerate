/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include "global.h"
#include "cong-dispspec.h"
#include "cong-document.h"


/*
  EDITOR POPUP CODE:
 */
/* 
   The popup menu widget (and some items) have a pointer to the CongDocument set as a user property named "doc":
*/
static gint editor_popup_callback_item_selected(GtkWidget *widget, CongDispspecElement *element)
{
	CongNodePtr new_element;
	CongNodePtr r;

	CongDocument *doc;
	CongSelection *selection;
	CongCursor *cursor;

	g_return_val_if_fail(element, TRUE);

	doc = g_object_get_data(G_OBJECT(widget),
				"doc");
	g_assert(doc);

	selection = cong_document_get_selection(doc);
	cursor = cong_document_get_cursor(doc);

	/* GREP FOR MVC */


#ifndef RELEASE
	printf("Inserting tag (%s).\n", tag->data);
#endif

	new_element = cong_node_new_element(cong_dispspec_element_tagname(element));
#if USE_CONG_EDITOR_WIDGET
	if (!cong_selection_reparent_all(selection, doc, new_element)) {
		cong_node_free(new_element);
	}
#else
	g_assert(cursor->xed);

	if (selection->loc0.tt_loc == cursor->xed->x)
	{
		r = cong_selection_reparent_all(selection, doc, new_element);
		if (r) {
			cursor->xed->x = r;
		} else {
			cong_node_free(new_element);
		}
	}
	else if (!cong_selection_reparent_all(selection, doc, new_element)) {
		cong_node_free(new_element);
	}

	xed_redraw(cursor->xed);
	cursor->xed = NULL;
#endif

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

void editor_popup_show(GtkWidget *widget, GdkEventButton *bevent)
{
	gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL, bevent->button,
								 bevent->time);
	
	return;
}

void editor_popup_init(CongDocument *doc)
{
	if (the_globals.popup) gtk_widget_destroy(the_globals.popup);
	the_globals.popup = gtk_menu_new();

	g_object_set_data(G_OBJECT(the_globals.popup),
			  "doc",
			  doc);

	gtk_menu_set_title(GTK_MENU(the_globals.popup), "Editing menu");
}

static void editor_popup_callback_tag_remove_inner(GtkWidget *widget, CongDocument *doc)
{
	CongNodePtr n0;

	CongDispspec *ds = cong_document_get_dispspec(doc);
	CongCursor *cursor = cong_document_get_cursor(doc);
	
	if (!cong_location_exists(&cursor->location)) return;

	n0 = xml_inner_span_element(ds, cursor->location.tt_loc);

	/* GREP FOR MVC */

	if (n0) cong_document_tag_remove(doc, n0);
#if !USE_CONG_EDITOR_WIDGET
	if (cursor->xed) xed_redraw(cursor->xed);
#endif
}

static void editor_popup_callback_tag_remove_outer(GtkWidget *widget, CongDocument *doc)
{
	CongNodePtr n0;

	CongDispspec *ds = cong_document_get_dispspec(doc);
	CongCursor *cursor = cong_document_get_cursor(doc);
	
	if (!cong_location_exists(&cursor->location)) return;
	
	n0 = xml_outer_span_element(ds, cursor->location.tt_loc);

	/* GREP FOR MVC */

	if (n0) cong_document_tag_remove(doc, n0);
#if !USE_CONG_EDITOR_WIDGET
	if (cursor->xed) xed_redraw(cursor->xed);
#endif
}

static gint editor_popup_callback_cut(GtkWidget *widget, CongDocument *doc)
{
	g_assert(doc);
	
	cong_document_cut(doc);
	return TRUE;
}

static gint editor_popup_callback_copy(GtkWidget *widget, CongDocument *doc)
{
	g_assert(doc);
	
	cong_document_copy(doc);
	return TRUE;
}

static gint editor_popup_callback_paste(GtkWidget *widget, CongDocument *doc)
{
	g_assert(doc);
	
	cong_document_paste(doc, widget);
	return TRUE;
}


void editor_popup_build(CongDocument *doc)
{
	GtkWidget *item, *w0;
	CongDispspec *dispspec;
	CongDispspecElement *n0;
	CongCursor *cursor;
	
	g_return_if_fail(doc);

	dispspec = cong_document_get_dispspec(doc);
	cursor = cong_document_get_cursor(doc);

	if (the_globals.popup) gtk_widget_destroy(the_globals.popup);
	editor_popup_init(doc);
	
#ifndef RELEASE
	printf("Building menu.\n");
#endif
	
	/* Fixed editing tools */

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT,
						  NULL); 
	gtk_menu_append(GTK_MENU(the_globals.popup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(editor_popup_callback_cut), doc);
	gtk_widget_show(item);

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY,
						  NULL); 
	gtk_menu_append(GTK_MENU(the_globals.popup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(editor_popup_callback_copy), doc);
	gtk_widget_show(item);
	
	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE,
						  NULL);
	gtk_menu_append(GTK_MENU(the_globals.popup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(editor_popup_callback_paste), doc);
	gtk_widget_show(item);

	if (/* cursor->set && */ cursor->location.tt_loc && xml_inner_span_element(dispspec, cursor->location.tt_loc))
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
				   GTK_SIGNAL_FUNC(editor_popup_callback_tag_remove_inner), doc);
		gtk_widget_show(item);
		
		item = gtk_menu_item_new_with_label("Remove outer tag");
		gtk_menu_append(GTK_MENU(the_globals.popup), item);
		gtk_signal_connect(GTK_OBJECT(item), "activate",
				   GTK_SIGNAL_FUNC(editor_popup_callback_tag_remove_outer), doc);
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
	for (n0 = cong_dispspec_get_first_element(dispspec); n0; n0 = cong_dispspec_element_next(n0))
	{
		if (cong_dispspec_element_is_span(n0))
		{
			item = gtk_menu_item_new_with_label(cong_dispspec_element_username(n0));
			gtk_menu_append(GTK_MENU(the_globals.popup), item);
			
			gtk_signal_connect(GTK_OBJECT(item), "activate",
					   GTK_SIGNAL_FUNC(editor_popup_callback_item_selected), n0);
			
			g_object_set_data(G_OBJECT(item),
					  "doc",
					  doc);
			gtk_widget_show(item);
		}
	}

}

/*
  TREE POPUP CODE:
 */
/* the treeview widget has the userdata "cong_tree_view" set on it */
gint tree_popup_show(GtkWidget *widget, GdkEvent *event)
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
					
					menu = tree_popup_init(cong_tree_view, tt);
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
static GtkWidget* add_item_to_popup(GtkMenu *menu,
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

	return item;
}

static GtkWidget* add_stock_item_to_popup(GtkMenu *menu,
				    const gchar *stock_id,
				    gint (*func)(GtkWidget *widget, CongNodePtr tag),
				    CongTreeView *cong_tree_view,
				    CongNodePtr x)
{
	GtkWidget *item = gtk_image_menu_item_new_from_stock(stock_id,
							     NULL); 
	gtk_menu_append(menu, item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(func), x);
	g_object_set_data(G_OBJECT(item),
			  "cong_tree_view",
			  cong_tree_view);
	gtk_widget_show(item);

	return item;
}


GtkWidget *structural_tag_popup_init(CongDispspec *ds, gint (*callback)(GtkWidget *widget, CongNodePtr tag),
				     CongTreeView *cong_tree_view,
				     CongNodePtr x)
{
	GtkWidget *popup, *item;
	CongDispspecElement *n0;
  
	popup = gtk_menu_new();
	gtk_menu_set_title(GTK_MENU(popup), "Sub-element menu");
	
	/* Window -> vbox -> buttons */
	for (n0 = cong_dispspec_get_first_element(ds); n0; n0 = cong_dispspec_element_next(n0)) {
		if (cong_dispspec_element_is_structural(n0)) {
			item = add_item_to_popup(GTK_MENU(popup),
						 cong_dispspec_element_username(n0),
						 callback,
						 cong_tree_view,
						 x);
			g_object_set_data(G_OBJECT(item),
					  "label",
					  cong_dispspec_element_tagname(n0));

		}
	}
	gtk_widget_show(popup);
	return popup;
}


GtkWidget* tree_popup_init(CongTreeView *cong_tree_view, CongNodePtr x)
{
	GtkMenu *tpopup;
	GtkWidget *item, *w0, *sub_popup;
	CongDocument *doc;
	CongDispspec *ds;

	g_assert(cong_tree_view);				
	doc = CONG_VIEW(cong_tree_view)->doc;
	ds = cong_document_get_dispspec(doc);

	tpopup = GTK_MENU(gtk_menu_new());
	gtk_menu_set_title(GTK_MENU(tpopup), "Structure menu");


	add_stock_item_to_popup(tpopup,
				GTK_STOCK_PROPERTIES,
				tree_properties,
				cong_tree_view,
				x);

	item = gtk_menu_item_new();
	w0 = gtk_hseparator_new();
	gtk_container_add(GTK_CONTAINER(item), w0);
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_widget_set_sensitive(item, 0);
	gtk_widget_show(w0);
	gtk_widget_show(item);

	add_stock_item_to_popup(tpopup,
				GTK_STOCK_CUT,
				tree_cut,
				cong_tree_view,
				x);
	add_stock_item_to_popup(tpopup,
				GTK_STOCK_COPY,
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

	item = gtk_menu_item_new();
	w0 = gtk_hseparator_new();
	gtk_container_add(GTK_CONTAINER(item), w0);
	gtk_menu_append(GTK_MENU(tpopup), item);
	gtk_widget_set_sensitive(item, 0);
	gtk_widget_show(w0);
	gtk_widget_show(item);

	item = add_item_to_popup(tpopup,
				 "New sub-element",
				 NULL,
				 cong_tree_view,
				 x);
	
	sub_popup = structural_tag_popup_init(ds, tree_new_sub_element, cong_tree_view, x);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub_popup);

	
	item = add_item_to_popup(tpopup,
				 "New sibling",
				 NULL,
				 cong_tree_view,
				 x);

	sub_popup = structural_tag_popup_init(ds, tree_new_sibling, cong_tree_view, x);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub_popup);

	return GTK_WIDGET(tpopup);
}
