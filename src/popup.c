/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include "global.h"
#include "cong-dispspec.h"
#include "cong-document.h"
#include "cong-view.h"
#include "cong-app.h"

static GtkMenuItem* make_menu_item(const gchar *label,
				   const gchar *tip,
				   GdkPixbuf *pixbuf);


static GtkMenuItem* make_menu_item_for_dispspec_element(CongDispspecElement *element);

static GtkWidget *add_menu_separator(GtkMenu *menu);

static GtkWidget* add_item_to_popup(GtkMenu *menu,
				    GtkMenuItem *item, 
				    CongDocument *doc,
				    gint (*func)(GtkWidget *widget, CongNodePtr tag),
				    CongNodePtr callback_data,
				    GtkWindow *parent_window);

static GtkWidget* span_tag_removal_popup_init(CongDispspec *ds, 
					      CongCursor *cursor, 
					      gint (*callback)(GtkWidget *widget, CongNodePtr node_ptr),
					      CongDocument *doc, 
					      GList *list,
					      GtkWindow *parent_window);

static gint editor_popup_callback_remove_span_tag(GtkWidget *widget, 
						  CongNodePtr node_ptr);

static GtkWidget *structural_tag_popup_init(CongDocument *doc,
					    gint (*callback)(GtkWidget *widget, CongNodePtr tag),
					    CongNodePtr x, 
					    GList *list,
					    GtkWindow *parent_window);

static GList *sort_menu(GList *list_of_dispspec_element);


static GtkWidget *new_sibling_structural_tag_popup_init(CongDocument *doc,
							gint (*callback)(GtkWidget *widget, CongNodePtr tag),
							CongNodePtr x,
							GtkWindow *parent_window);

static GtkWidget *new_sub_element_structural_tag_popup_init(CongDocument *doc,
							    gint (*callback)(GtkWidget *widget, CongNodePtr tag),
							    CongNodePtr x,
							    GtkWindow *parent_window);

static GtkMenuItem* make_menu_item(const gchar *label,
				   const gchar *tip,
				   GdkPixbuf *pixbuf)
{
	GtkWidget *item;

	g_return_val_if_fail(label, NULL);

	item = gtk_image_menu_item_new_with_label (label);

	if (pixbuf) {
		GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
		gtk_widget_show(image);
	}

	if (tip) {
		gtk_tooltips_set_tip(the_app.tooltips,
				     GTK_WIDGET(item),
				     tip,
				     tip);
	}

	return GTK_MENU_ITEM(item);
}

static GtkMenuItem* make_menu_item_for_dispspec_element(CongDispspecElement *element)
{
	GtkMenuItem *item;
	GdkPixbuf *pixbuf;
	const gchar *tip;

	g_return_val_if_fail(element, NULL);

	pixbuf = cong_dispspec_element_get_icon(element);
	tip = cong_dispspec_element_get_description(element);

	if (NULL==tip) {
		tip = _("(no description available)");
	}
	
	item = make_menu_item(cong_dispspec_element_username(element),
			      tip,
			      pixbuf);
	if (pixbuf) {
		g_object_unref(G_OBJECT(pixbuf));
	}

	return item;
}

static GtkWidget *add_menu_separator(GtkMenu *menu)
{
	GtkWidget *item = gtk_menu_item_new();
	GtkWidget *w0 = gtk_hseparator_new();
	gtk_container_add(GTK_CONTAINER(item), w0);
	gtk_menu_append(menu, item);
	gtk_widget_set_sensitive(item, 0);
	gtk_widget_show(w0);
	gtk_widget_show(item);

	return item;
}



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

	new_element = cong_node_new_element_from_dispspec(element, doc);
	if (!cong_selection_reparent_all(selection, doc, new_element)) {
		cong_node_free(new_element);
	}

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
	if (the_app.popup) gtk_widget_destroy(the_app.popup);
	the_app.popup = gtk_menu_new();

	g_object_set_data(G_OBJECT(the_app.popup),
			  "doc",
			  doc);

	gtk_menu_set_title(GTK_MENU(the_app.popup), "Editing menu");
}

static gint editor_popup_callback_remove_span_tag(GtkWidget *widget, CongNodePtr node_ptr)
{ 
	CongDocument *doc = (CongDocument*)(g_object_get_data(G_OBJECT(widget), "document"));
	CongCursor *cursor = cong_document_get_cursor(doc);

	cong_document_tag_remove(doc, node_ptr);
}

static gint editor_popup_callback_cut(GtkWidget *widget, CongDocument *doc)
{
	g_assert(doc);
	
	cong_document_cut_selection(doc);
	return TRUE;
}

static gint editor_popup_callback_copy(GtkWidget *widget, CongDocument *doc)
{
	g_assert(doc);
	
	cong_document_copy_selection(doc);
	return TRUE;
}

static gint editor_popup_callback_paste(GtkWidget *widget, CongDocument *doc)
{
	g_assert(doc);
	
	cong_document_paste_selection(doc, widget);
	return TRUE;
}


void editor_popup_build(CongDocument *doc, GtkWindow *parent_window)
{
	GtkWidget *item, *w0, *sub_popup;
	CongDispspec *dispspec;
	CongDispspecElement *dispspec_element;
	CongCursor *cursor;
	GList *span_tags_list;
	
	g_return_if_fail(doc);

	dispspec = cong_document_get_dispspec(doc);
	cursor = cong_document_get_cursor(doc);

	if (the_app.popup) gtk_widget_destroy(the_app.popup);
	editor_popup_init(doc);
	
#ifndef RELEASE
	printf("Building menu.\n");
#endif
	
	/* Fixed editing tools */

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT,
						  NULL); 
	gtk_menu_append(GTK_MENU(the_app.popup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(editor_popup_callback_cut), doc);
	gtk_widget_show(item);

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY,
						  NULL); 
	gtk_menu_append(GTK_MENU(the_app.popup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(editor_popup_callback_copy), doc);
	gtk_widget_show(item);
	
	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE,
						  NULL);
	gtk_menu_append(GTK_MENU(the_app.popup), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(editor_popup_callback_paste), doc);
	gtk_widget_show(item);

	span_tags_list = xml_all_present_span_elements(dispspec, cursor->location.node);

	if (span_tags_list != NULL)
	{
		item = gtk_menu_item_new();
		w0 = gtk_hseparator_new();
		gtk_container_add(GTK_CONTAINER(item), w0);
		gtk_menu_append(GTK_MENU(the_app.popup), item);
		gtk_widget_set_sensitive(item, 0);
		gtk_widget_show(w0);
		gtk_widget_show(item);
		
		item = add_item_to_popup(GTK_MENU(the_app.popup), 
					 make_menu_item(_("Remove span tag"), 
							NULL, /* FIXME: we ought to have a tip for this */
							NULL), /* FIXME: we ought to have a icon for this */
					 doc, 
					 NULL,
					 NULL,
					 parent_window);		

		sub_popup = span_tag_removal_popup_init(dispspec, 
							cursor, 
							editor_popup_callback_remove_span_tag, 
							doc, 
							span_tags_list,
							parent_window);
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub_popup);

	}
	
	g_list_free(span_tags_list);
	
	
	add_menu_separator(GTK_MENU(the_app.popup));

	/* Build list of dynamic tag insertion tools */
	/*  build the list of valid inline tags here */
	span_tags_list = xml_all_valid_span_elements(dispspec, cursor->location.node);
	span_tags_list = sort_menu(span_tags_list);

	if (span_tags_list) {
		GList *iter;

		for (iter=span_tags_list; iter; iter=iter->next) {
			CongDispspecElement *dispspec_element = (CongDispspecElement *)iter->data;

			item = GTK_WIDGET(make_menu_item_for_dispspec_element(dispspec_element));
			/* FIXME: perhaps we should composite an "add" icon to the element's icon? */

			gtk_menu_append(GTK_MENU(the_app.popup), item);

			gtk_signal_connect(GTK_OBJECT(item), "activate",
					   GTK_SIGNAL_FUNC(editor_popup_callback_item_selected), dispspec_element);
			
			g_object_set_data(G_OBJECT(item),
					  "doc",
					  doc);
			gtk_widget_show(item);
		}
	}

	g_list_free(span_tags_list);
}

/*
  TREE POPUP CODE:
 */

/* the popup items have the data "document" set on them: */
static GtkWidget *add_item_to_popup(GtkMenu *menu,
				    GtkMenuItem *item, 
				    CongDocument *doc,
				    gint (*func)(GtkWidget *widget, CongNodePtr tag),
				    CongNodePtr callback_data,
				    GtkWindow *parent_window)
{
	g_return_val_if_fail(menu, NULL);
	g_return_val_if_fail(item, NULL);
	g_return_val_if_fail(doc, NULL);

	if (func != NULL) {
		gtk_signal_connect(GTK_OBJECT(item), 
				   "activate",
				   GTK_SIGNAL_FUNC(func), 
				   callback_data);
	}

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(item));
	g_object_set_data(G_OBJECT(item),
			  "document",
			  doc);
	g_object_set_data(G_OBJECT(item),
			  "parent_window",
			  parent_window);
	gtk_widget_show(GTK_WIDGET(item));

	return GTK_WIDGET(item);
}

static GtkWidget* add_stock_item_to_popup(GtkMenu *menu,
					  const gchar *stock_id,
					  CongDocument *doc,
					  gint (*func)(GtkWidget *widget, CongNodePtr tag),
					  CongNodePtr node,
					  GtkWindow *parent_window)

{
	GtkWidget *item = gtk_image_menu_item_new_from_stock(stock_id,
							     NULL); 
	gtk_menu_append(menu, item);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(func), node);
	g_object_set_data(G_OBJECT(item),
			  "document",
			  doc);
	g_object_set_data(G_OBJECT(item),
			  "parent_window",
			  parent_window);
	gtk_widget_show(item);

	return item;
}

static GtkWidget* span_tag_removal_popup_init(CongDispspec *ds, 
					      CongCursor *cursor, 
					      gint (*callback)(GtkWidget *widget, CongNodePtr node_ptr),
					      CongDocument *doc, 
					      GList *list,
					      GtkWindow *parent_window) 
{
	GtkWidget *popup;
	GList *current;

	popup = gtk_menu_new();
	gtk_menu_set_title(GTK_MENU(popup), "Span tag menu");

	for (current = g_list_last(list); current; current = g_list_previous(current)) {
		
		CongNodePtr node = (CongNodePtr)(current->data);
		CongDispspecElement *element = cong_dispspec_lookup_node(ds, node);

		GtkWidget *item = add_item_to_popup(GTK_MENU(popup),
						    make_menu_item_for_dispspec_element(element), /* FIXME: should we composite a deletion icon onto the pixbuf? */
						    doc,
						    callback,
						    node,
						    parent_window);

		g_object_set_data(G_OBJECT(item),
				  "document",
				  doc);
	}

	gtk_widget_show(popup);
	return popup;
}

static gint my_compare_func(gconstpointer a, gconstpointer b)
{
	CongDispspecElement *elem_a;
	CongDispspecElement *elem_b;
	const gchar *name_a;
	const gchar *name_b;
	gchar *folded_a;
	gchar *folded_b;
	gint result;

	elem_a = (CongDispspecElement*)a;
	elem_b = (CongDispspecElement*)b;

	name_a = cong_dispspec_element_username(elem_a);
	if (NULL==name_a) {
		name_a = cong_dispspec_element_tagname(elem_a);
	}
	name_b = cong_dispspec_element_username(elem_b);
	if (NULL==name_b) {
		name_b = cong_dispspec_element_tagname(elem_b);
	}

	g_assert(name_a);
	g_assert(name_b);

	/* g_message("comparing \"%s\" and \"%s\"", name_a, name_b); */

	folded_a = g_utf8_casefold(name_a,-1);
	folded_b = g_utf8_casefold(name_b,-1);
	result = g_utf8_collate(folded_a, folded_b);

	g_free(folded_a);
	g_free(folded_b);

	return result;
}


static GList *sort_menu(GList *list_of_dispspec_element)
{
	/* Sort the list into alphabetical order or user-visible names: */
	return g_list_sort(list_of_dispspec_element, my_compare_func);
}
					 
static GtkWidget *new_sibling_structural_tag_popup_init(CongDocument *doc, 
							gint (*callback)(GtkWidget *widget, CongNodePtr tag),
							CongNodePtr x,
							GtkWindow *parent_window) 
{
	CongDispspec *ds;
	GList *list;
	GtkWidget *popup;

	g_return_val_if_fail(doc, NULL);

	ds = cong_document_get_dispspec(doc);

	list = xml_get_valid_next_sibling(ds, x, CONG_ELEMENT_TYPE_STRUCTURAL);

	list = sort_menu(list);

	popup = structural_tag_popup_init(doc, 
					  callback, 
					  x, 
					  list,
					  parent_window);
	
	g_list_free(list);

	return popup;
}
					 
static GtkWidget *new_sub_element_structural_tag_popup_init(CongDocument *doc,
							    gint (*callback)(GtkWidget *widget, CongNodePtr tag),
							    CongNodePtr x,
							    GtkWindow *parent_window) 
{
	CongDispspec *ds;
	GList *list;
	GtkWidget *popup;

	g_return_val_if_fail(doc, NULL);

	ds = cong_document_get_dispspec(doc);

	list = xml_get_valid_children(ds, x, CONG_ELEMENT_TYPE_STRUCTURAL);

	list = sort_menu(list);

	popup = structural_tag_popup_init(doc, 
					  callback, 
					  x, 
					  list,
					  parent_window);
	
	g_list_free(list);

	return popup;
}



static GtkWidget *structural_tag_popup_init(CongDocument *doc,
					    gint (*callback)(GtkWidget *widget, CongNodePtr tag),
					    CongNodePtr node, 
					    GList *list,
					    GtkWindow *parent_window)
{
	CongDispspec *ds;
	GtkWidget *popup, *item;
	GList *current;
/* 	int i; */

	g_return_val_if_fail(doc, NULL);

	ds = cong_document_get_dispspec(doc);

	popup = gtk_menu_new();
	gtk_menu_set_title(GTK_MENU(popup), "Sub menu");
	
	for (current = g_list_first(list); current; current = g_list_next(current)) {

		CongDispspecElement *element = (CongDispspecElement *)(current->data);
		
		item = add_item_to_popup(GTK_MENU(popup),
					 make_menu_item_for_dispspec_element(element),
					 doc,
					 callback,
					 node,
					 parent_window);
		g_object_set_data(G_OBJECT(item),
				  "element",
				  (gpointer)element);

	}

	gtk_widget_show(popup);
	return popup;
}

GtkWidget* cong_ui_popup_init(CongDocument *doc, 
			      CongNodePtr node,
			      GtkWindow *parent_window)
{
	GtkMenu *tpopup;
	GtkWidget *item, *w0, *sub_popup;
	CongDispspec *ds;

	g_assert(doc);				
	ds = cong_document_get_dispspec(doc);

	tpopup = GTK_MENU(gtk_menu_new());
	gtk_menu_set_title(GTK_MENU(tpopup), "Structure menu");


	/* Add "Properties" action: */
	add_stock_item_to_popup(tpopup,
				GTK_STOCK_PROPERTIES,
				doc,
				tree_properties,
				node,
				parent_window);
	
	add_menu_separator(GTK_MENU(tpopup));

	/* Add clipboard operations: */
	/* FIXME:  the clipboard stuff only currently works for elements, hence we should filter on these for now: */
	if (cong_node_type(node)==CONG_NODE_TYPE_ELEMENT) {
		add_stock_item_to_popup(tpopup,
					GTK_STOCK_CUT,
					doc,
					tree_cut,
					node,
					parent_window);
		add_stock_item_to_popup(tpopup,
					GTK_STOCK_COPY,
					doc,
					tree_copy,
					node,
					parent_window);
		add_item_to_popup(tpopup,
				  make_menu_item(_("Paste into"),
						 NULL, /* FIXME:  ought to have a tooltip */
						 NULL), /* FIXME:  ought to have an icon */
				  doc,
				  tree_paste_under,
				  node,
				  parent_window);
		add_item_to_popup(tpopup,
				  make_menu_item(_("Paste before"),
						 NULL, /* FIXME:  ought to have a tooltip */
						 NULL), /* FIXME:  ought to have an icon */
				  doc,
				  tree_paste_before,
				  node,
				  parent_window);
		add_item_to_popup(tpopup,
				  make_menu_item(_("Paste after"),
						 NULL, /* FIXME:  ought to have a tooltip */
						 NULL), /* FIXME:  ought to have an icon */
				  doc,
				  tree_paste_after,
				  node,
				  parent_window);
	}

	add_menu_separator(GTK_MENU(tpopup));

	item = add_item_to_popup(tpopup,
				 make_menu_item(_("New sub-element"),
						NULL, /* FIXME:  ought to have a tooltip */
						NULL), /* FIXME:  ought to have an icon */
				 doc,
				 NULL,
				 node,
				 parent_window);
	
	sub_popup = new_sub_element_structural_tag_popup_init(doc, 
							      tree_new_sub_element, 
							      node, 
							      parent_window);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub_popup);

	
	item = add_item_to_popup(tpopup,
				 make_menu_item(_("New sibling"),
						NULL, /* FIXME:  ought to have a tooltip */
						NULL), /* FIXME:  ought to have an icon */
				 doc,
				 NULL,
				 node,
				 parent_window);

	sub_popup = new_sibling_structural_tag_popup_init(doc, 
							  tree_new_sibling, 
							  node, 
							  parent_window);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub_popup);

	return GTK_WIDGET(tpopup);
}


/**
 * Popup a modal, blocking dialog to obtain
 * the user's choice between a list of char *.
 * Returns a new char * of the element name 
 * selected, which must be freed by caller.
 */
gchar *string_selection_dialog(gchar *title, gchar *element_description, GList *elements) 
{
	GtkWidget *window, *vbox, *label, *button, *action_area, *combo;
	GList *current;
	gint length, response;
	gchar *text;

	/*  create a new dialog */
	window = gtk_dialog_new_with_buttons(title, NULL, /* FIXME: set up the parent dialog */
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK, GTK_RESPONSE_OK,
					     NULL);

	/*  get internal vbox  */
	vbox = GTK_DIALOG(window)->vbox;
	action_area = GTK_DIALOG(window)->action_area;
       
	/*  Create dialog contents */
	label = gtk_label_new(element_description);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	
	combo = gtk_combo_new ();
	gtk_combo_set_popdown_strings(GTK_COMBO(combo), elements);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(combo)->entry), FALSE);
	

	gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 0);	

	/*  run the dialog */
	gtk_widget_show_all(GTK_WIDGET(window));
	response = gtk_dialog_run(GTK_DIALOG(window));

	if (response == GTK_RESPONSE_OK) {
		/* get combo value, and strdup since string will be destroyed with combo widget */
		text = g_strdup(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry)));
	}
	else {
		text = NULL;
	}
	
	gtk_widget_destroy(GTK_WIDGET(window));

	return text;
}
