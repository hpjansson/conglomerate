/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include "global.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-dialog.h"
#include "cong-view.h"

/* the popup items have the data "popup_data_item" set on them: */

gint tree_new_sibling(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;

	CongNodePtr text_node, new_node;
	char *label;

	label = g_object_get_data(G_OBJECT(widget),
				  "label");

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);
	
	
	/* GREP FOR MVC */

	/* Text node before new element */
	text_node = cong_node_new_text(" ");
	cong_document_node_add_after(doc, text_node, tag);

	/* New element */
	new_node = cong_node_new_element(label);
	cong_document_node_add_after(doc, new_node, text_node);

	/*  add any necessary sub elements it needs */
	xml_add_required_children(doc, new_node);

	/* Text node after new element */
	text_node = cong_node_new_text(" ");
	cong_document_node_add_after(doc, text_node, new_node);

	return(TRUE);
}


gint tree_new_sub_element(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;

	CongNodePtr text_node, new_node;
	char *label;

	label = g_object_get_data(G_OBJECT(widget),
				  "label");

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	/* GREP FOR MVC */

	/* Text node before new element */
	text_node = cong_node_new_text(" ");
	cong_document_node_set_parent(doc, text_node, tag);

	/* New element */
	new_node = cong_node_new_element(label);
	cong_document_node_set_parent(doc, new_node, tag);

	/*  add any necessary sub elements it needs */
	xml_add_required_children(doc, new_node);

	/* Text node after new element */
	text_node = cong_node_new_text(" ");
	cong_document_node_set_parent(doc, text_node, tag);

	return(TRUE);
}

gint tree_properties(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;
	CongDispspec *ds;
	GtkWidget *dialog, *vbox;
	GtkWidget *notebook;
	CongDialogContent *dialog_content;
	CongDialogCategory *category_xpath;
	CongDialogCategory *category_dtdschema;
	CongDialogCategory *category_custom;
	xmlElementPtr xml_element;
	GtkWindow *parent_window;
	gchar *xpath;

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	ds = cong_document_get_dispspec(doc);

	parent_window = g_object_get_data(G_OBJECT(widget),
					  "parent_window");

	/* FIXME: Test implementation: */
	dialog = gtk_dialog_new_with_buttons(_("Placeholder Properties Dialog"),
					     parent_window,
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_OK, GTK_RESPONSE_OK,
					     NULL);
					
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);

	vbox = GTK_DIALOG(dialog)->vbox;

	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(vbox), notebook, FALSE, FALSE, 0);	

	dialog_content = cong_dialog_content_new(TRUE);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
				 cong_dialog_content_get_widget(dialog_content),
				 gtk_label_new(_("Advanced"))
				 );

	category_xpath = cong_dialog_content_add_category(dialog_content, 
							  _("Location"));
	xpath = cong_node_get_path(tag);
	cong_dialog_category_add_field(category_xpath, _("XPath"), gtk_label_new(xpath));
	g_free(xpath);

	

	xml_element = xml_get_dtd_element(doc, tag);

	if (xml_element) {
		xmlAttributePtr attr;
		category_dtdschema = cong_dialog_content_add_category(dialog_content, 
								      _("Properties from DTD/schema"));

		for (attr=xml_element->attributes; attr; attr=attr->nexth) {
			switch (attr->atype) {
			default: g_assert_not_reached();
			case XML_ATTRIBUTE_CDATA:
				cong_dialog_category_add_field(category_dtdschema, attr->name, gtk_label_new("CDATA"));
				break;
				
			case XML_ATTRIBUTE_ID:
				cong_dialog_category_add_field(category_dtdschema, attr->name, gtk_label_new("ID"));
				break;
				
			case XML_ATTRIBUTE_IDREF:
				cong_dialog_category_add_field(category_dtdschema, attr->name, gtk_label_new("IDREF"));
				break;
				
			case XML_ATTRIBUTE_IDREFS:
				cong_dialog_category_add_field(category_dtdschema, attr->name, gtk_label_new("IDREFS"));
				break;

			case XML_ATTRIBUTE_ENTITY:
				cong_dialog_category_add_field(category_dtdschema, attr->name, gtk_label_new("ENTITY"));
				break;
				
			case XML_ATTRIBUTE_ENTITIES:
				cong_dialog_category_add_field(category_dtdschema, attr->name, gtk_label_new("ENTITIES"));
				break;
				
			case XML_ATTRIBUTE_NMTOKEN:
				cong_dialog_category_add_field(category_dtdschema, attr->name, gtk_label_new("NMTOKEN"));
				break;
				
			case XML_ATTRIBUTE_NMTOKENS:
				cong_dialog_category_add_field(category_dtdschema, attr->name, gtk_label_new("NMTOKENS"));
				break;
				
			case XML_ATTRIBUTE_ENUMERATION:
				{
					GtkWidget *option_menu = gtk_option_menu_new();
					GtkWidget *menu = gtk_menu_new();
					xmlEnumerationPtr enum_ptr;

					for (enum_ptr=attr->tree; enum_ptr; enum_ptr=enum_ptr->next) {
						gtk_menu_shell_append(GTK_MENU_SHELL(menu), 
								      gtk_menu_item_new_with_label(enum_ptr->name));
					}

					gtk_option_menu_set_menu(GTK_OPTION_MENU(option_menu), 
								 menu);

					cong_dialog_category_add_field(category_dtdschema, 
								       attr->name, 
								       option_menu);
				}
				break;

			case XML_ATTRIBUTE_NOTATION:
				cong_dialog_category_add_field(category_dtdschema, attr->name, gtk_label_new("NOTATION"));
				break;
			}
			


		}
	}

	category_custom = cong_dialog_content_add_category(dialog_content, 
							   _("Custom Properties"));


	gtk_widget_show_all (dialog);

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	return TRUE;
}

gint tree_cut(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;
	CongDispspec *ds;

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	ds = cong_document_get_dispspec(doc);

	/* GREP FOR MVC */

	if (the_globals.clipboard) cong_node_recursive_delete(NULL, the_globals.clipboard);
	the_globals.clipboard = cong_node_recursive_dup(tag);
	cong_node_recursive_delete(doc, tag);

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
	CongDocument *doc;
	CongDispspec *ds;

	CongNodePtr new_copy;

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	ds = cong_document_get_dispspec(doc);

	if (!the_globals.clipboard) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(tag))) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(the_globals.clipboard))) return(TRUE);

	/* GREP FOR MVC */

	new_copy = cong_node_recursive_dup(the_globals.clipboard);

	cong_document_node_set_parent(doc, new_copy,tag);

	return(TRUE);
}


gint tree_paste_before(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;
	CongDispspec *ds;

	CongNodePtr new_copy;

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	ds = cong_document_get_dispspec(doc);

	if (!the_globals.clipboard) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(tag))) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(the_globals.clipboard))) return(TRUE);

	/* GREP FOR MVC */

	new_copy = cong_node_recursive_dup(the_globals.clipboard);

	cong_document_node_add_before(doc, new_copy,tag);
	
	return(TRUE);
}


gint tree_paste_after(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;
	CongDispspec *ds;

	CongNodePtr new_copy;

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	ds = cong_document_get_dispspec(doc);

	if (!the_globals.clipboard) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(tag))) return(TRUE);
	if (!cong_dispspec_element_structural(ds, xml_frag_name_nice(the_globals.clipboard))) return(TRUE);

	/* GREP FOR MVC */

	new_copy = cong_node_recursive_dup(the_globals.clipboard);

	cong_document_node_add_after(doc, new_copy,tag);
	
	return(TRUE);
}

