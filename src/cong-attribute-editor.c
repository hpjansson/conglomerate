/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-attribute-editor.c
 *
 * Copyright (C) 2003 David Malcolm
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
#include "cong-attribute-editor.h"

/*
   FIXME
  We need to remove callbacks from document when widgets are deleted!!!
  This probably requires us to have fully-fledged GtkWidget subclasses!
*/

/* Internal types: */
struct CongAttributeEditorDetails
{
	CongDocument *doc;
	CongNodePtr node;
	const gchar *attribute_name;

	GtkWidget *hbox;
	GtkWidget *value;
	GtkWidget *add_btn;
	GtkWidget *delete_btn;	
};



/* Internal function declarations: */

/* CDATA support: */
static void
on_set_attribute_cdata (CongDocument *doc, 
			CongNodePtr node, 
			const xmlChar *name, 
			const xmlChar *value, 
			gpointer user_data);
static void
on_remove_attribute_cdata (CongDocument *doc, 
			   CongNodePtr node, 
			   const xmlChar *name,
			   gpointer user_data);
static void
on_text_entry_changed_cdata (GtkEditable *editable,
			     gpointer user_data);
static void
on_add_button_cdata (GtkButton *button,
		     gpointer user_data);
static void
on_delete_button_cdata (GtkButton *button,
			gpointer user_data);
static void
refresh_cdata_editor (struct CongAttributeEditorDetails *details);

/* ID support: */
/* IDREF support: */
/* IDREFS support: */
/* ENTITY support: */
/* ENTITIES support: */
/* NMTOKEN support: */
/* NMTOKENS support: */
/* ENUMERATION support: */
static void
on_set_attribute_enumeration (CongDocument *doc, 
			      CongNodePtr node, 
			      const xmlChar *name, 
			      const xmlChar *value, 
			      gpointer user_data);
static void
on_remove_attribute_enumeration (CongDocument *doc, 
				 CongNodePtr node, 
				 const xmlChar *name,
				 gpointer user_data);
static void
on_enumeration_option_menu_changed (GtkOptionMenu *option_menu,
				    gpointer user_data);
/* NOTATION support: */

/* Exported function definitions: */
GtkWidget*
cong_attribute_editor_new (CongDocument *doc,
			   CongNodePtr node,
			   xmlAttributePtr attr)
{
	g_return_val_if_fail (doc, NULL);
	g_return_val_if_fail (node, NULL);
	g_return_val_if_fail (attr, NULL);

	switch (attr->atype) {
	default: g_assert_not_reached();
	case XML_ATTRIBUTE_CDATA:
		return cong_attribute_editor_cdata_new (doc,
							node,
							attr->name);
	case XML_ATTRIBUTE_ID:
		/* FIXME: extend NMTOKEN thing */
		return gtk_label_new("ID");
		
	case XML_ATTRIBUTE_IDREF:
		/* FIXME: use a combobox? */
		return gtk_label_new("IDREF");
		
	case XML_ATTRIBUTE_IDREFS:
		/* FIXME: use a list view plus buttons? */
		return gtk_label_new("IDREFS");
		
	case XML_ATTRIBUTE_ENTITY:
		/* FIXME : use a combobox? */
		return gtk_label_new("ENTITY");
		
	case XML_ATTRIBUTE_ENTITIES:
		/* FIXME : use a list view? */
		return gtk_label_new("ENTITIES");
		
	case XML_ATTRIBUTE_NMTOKEN:
		/* FIXME: should use a text entry; similar to the raw editor thing, but with validation */
		return gtk_label_new("NMTOKEN");
		
	case XML_ATTRIBUTE_NMTOKENS:
		/* FIXME: use a list view, with buttons to add and delete? */
		return gtk_label_new("NMTOKENS");
		
	case XML_ATTRIBUTE_ENUMERATION:
		{
			GtkWidget *option_menu = gtk_option_menu_new();
			GtkWidget *menu = gtk_menu_new();
			xmlEnumerationPtr enum_ptr;

			GtkWidget *menu_item = gtk_menu_item_new_with_label(enum_ptr->name);
			
			gtk_menu_shell_append (GTK_MENU_SHELL(menu), 
					       gtk_menu_item_new_with_label(_("(unspecified)")));
			
			for (enum_ptr=attr->tree; enum_ptr; enum_ptr=enum_ptr->next) {
				GtkWidget *menu_item = gtk_menu_item_new_with_label(enum_ptr->name);

				g_object_set_data (G_OBJECT(menu_item),
						   "attr_value",
						   enum_ptr->name);

				gtk_menu_shell_append(GTK_MENU_SHELL(menu), 
						      menu_item);
			}
			
			gtk_option_menu_set_menu(GTK_OPTION_MENU(option_menu), 
						 menu);

			g_object_set_data (G_OBJECT(option_menu),
					   "doc",
					   doc);
			g_object_set_data (G_OBJECT(option_menu),
					   "node",
					   node);
			g_object_set_data (G_OBJECT(option_menu),
					   "attr",
					   attr);

			g_signal_connect_after (G_OBJECT(doc),
						"node_set_attribute",
						G_CALLBACK(on_set_attribute_enumeration),
						option_menu);
			g_signal_connect_after (G_OBJECT(doc),
						"node_remove_attribute",
						G_CALLBACK(on_remove_attribute_enumeration),
						option_menu);

			g_signal_connect_after (G_OBJECT(option_menu),
						"changed",
						G_CALLBACK(on_enumeration_option_menu_changed),
						NULL);

			return option_menu;
		}
		
	case XML_ATTRIBUTE_NOTATION:
		/* FIXME: some kind of list view? */
		return gtk_label_new("NOTATION");
	}

	/* FIXME: need to set up callbacks: when someone else modifies attrs, and when this widget modifies attr! Also need to set initial value! */

	g_assert_not_reached();	
}

GtkWidget*
cong_attribute_editor_cdata_new (CongDocument *doc,
				 CongNodePtr node,
				 const gchar *attribute_name)
{
#if 0
	struct CongAttributeEditorDetails* details;

	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);
	g_return_val_if_fail(attribute_name, NULL);

	details = g_new0(struct CongAttributeEditorDetails,1);

	details->doc = doc;
	details->node = node;
	details->attribute_name = attribute_name;

	details->hbox = gtk_hbox_new (FALSE, 6);
	details->value = gtk_entry_new ();
	details->add_btn = gtk_button_new_from_stock (GTK_STOCK_ADD);
	details->delete_btn = gtk_button_new_from_stock (GTK_STOCK_DELETE);
	
	gtk_box_pack_start(GTK_BOX(details->hbox), details->value, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(details->hbox), details->add_btn, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(details->hbox), details->delete_btn, FALSE, FALSE, 0);

	refresh_cdata_editor (details);

	g_signal_connect_after (G_OBJECT(doc),
				"node_set_attribute",
				G_CALLBACK(on_set_attribute_cdata),
				details);
	g_signal_connect_after (G_OBJECT(doc),
				"node_remove_attribute",
				G_CALLBACK(on_remove_attribute_cdata),
				details);

	g_signal_connect_after (G_OBJECT(details->value),
				"changed",
				G_CALLBACK(on_text_entry_changed_cdata),
				details);
	g_signal_connect_after (G_OBJECT(details->add_btn),
				"clicked",
				G_CALLBACK(on_add_button_cdata),
				details);
	g_signal_connect_after (G_OBJECT(details->delete_btn),
				"clicked",
				G_CALLBACK(on_delete_button_cdata),
				details);

	return details->hbox;
#else
	GtkWidget *widget;
	gchar *msg = g_strdup_printf(_("This will be an editor for CDATA attribute \"%s\""), attribute_name);
	widget = gtk_label_new(msg);

	g_free(msg);

	return widget;
#endif
	
}





GtkWidget*
create_cdata_editor (GladeXML *xml,
		     gchar *func_name,
		     gchar *name,
		     gchar *string1,
		     gchar *string2,
		     gint int1,
		     gint int2,
		     gpointer user_data)
{
	GtkWidget *custom_widget;

#if 1
	/* for some reason, the string1 stuff is coming through in func_name on my machine: */
	custom_widget = cong_attribute_editor_cdata_new (global_glade_doc_ptr, 
							 global_glade_node_ptr, 
							 func_name);
#else
	custom_widget = gtk_label_new(g_strdup_printf("custom widget \"%s\" \"%s\" \"%s\" \"%s\" %i %i", func_name, name, string1, string2, int1, int2)); /* for now */

	gtk_widget_show_all(custom_widget);
#endif

	return custom_widget;
}

CongDocument *global_glade_doc_ptr = NULL;
CongNodePtr global_glade_node_ptr = NULL;


/* Internal function definitions: */
/* CDATA support: */
static void
on_set_attribute_cdata (CongDocument *doc, 
			CongNodePtr node, 
			const xmlChar *name, 
			const xmlChar *value, 
			gpointer user_data)
{
	refresh_cdata_editor (user_data);
}

static void
on_remove_attribute_cdata (CongDocument *doc, 
			   CongNodePtr node, 
			   const xmlChar *name,
			   gpointer user_data)
{
	refresh_cdata_editor (user_data);
}

static void
on_text_entry_changed_cdata (GtkEditable *editable,
			     gpointer user_data)
{
	struct CongAttributeEditorDetails *details = user_data;
	
	cong_document_begin_edit (details->doc);
	
	cong_document_node_set_attribute (details->doc, 
					  details->node, 
					  details->attribute_name,
					  gtk_entry_get_text (GTK_ENTRY(details->value)));
	
	cong_document_end_edit (details->doc);
}

static void
on_add_button_cdata (GtkButton *button,
		     gpointer user_data)
{
	struct CongAttributeEditorDetails *details = user_data;
	
	cong_document_begin_edit (details->doc);
	
	cong_document_node_set_attribute (details->doc, 
					  details->node, 
					  details->attribute_name,
					  "");
	
	cong_document_end_edit (details->doc);
}

static void
on_delete_button_cdata (GtkButton *button,
			gpointer user_data)
{
	struct CongAttributeEditorDetails *details = user_data;
	
	cong_document_begin_edit (details->doc);
	
	cong_document_node_remove_attribute (details->doc, 
					     details->node, 
					     details->attribute_name);
	
	cong_document_end_edit (details->doc);
}


static void
refresh_cdata_editor (struct CongAttributeEditorDetails *details)
{
	gchar *attr_value = cong_node_get_attribute (details->node, 
						     details->attribute_name);
	
	if (attr_value) {
		gtk_entry_set_text (GTK_ENTRY (details->value),
				    attr_value);

		g_free (attr_value);

		gtk_widget_show (details->value);
		gtk_widget_hide (details->add_btn);
		gtk_widget_show (details->delete_btn);
	} else {
		gtk_widget_hide (details->value);
		gtk_widget_show (details->add_btn);
		gtk_widget_hide (details->delete_btn);
	}
}

/* ID support: */
/* IDREF support: */
/* IDREFS support: */
/* ENTITY support: */
/* ENTITIES support: */
/* NMTOKEN support: */
/* NMTOKENS support: */

/* ENUMERATION support: */
static void
on_set_attribute_enumeration (CongDocument *doc, 
			      CongNodePtr node, 
			      const xmlChar *name, 
			      const xmlChar *value, 
			      gpointer user_data)
{
	/* FIXME */
}

static void
on_remove_attribute_enumeration (CongDocument *doc, 
				 CongNodePtr node, 
				 const xmlChar *name,
				 gpointer user_data)
		     
{
	GtkOptionMenu *option_menu = GTK_OPTION_MENU(user_data);

	if (doc == g_object_get_data (G_OBJECT(option_menu), "doc")) {
		if (node == g_object_get_data (G_OBJECT(option_menu), "node")) {
			xmlAttributePtr attr = g_object_get_data (G_OBJECT(option_menu), "attr");
			g_assert (attr);

			if (0 == strcmp(name, attr->name)) {
				/* The initial item is the "unspecified" one: */
				gtk_option_menu_set_history (option_menu,
							     0);
			}
		}
	}
}

static void
on_enumeration_option_menu_changed (GtkOptionMenu *option_menu,
				    gpointer user_data)
{
	CongDocument *doc = g_object_get_data (G_OBJECT(option_menu), "doc");
	CongNodePtr node = g_object_get_data (G_OBJECT(option_menu), "node");
	xmlAttributePtr attr = g_object_get_data (G_OBJECT(option_menu), "attr");
	GtkMenuItem *selected_menu_item;
	const gchar *new_attr_value;

	g_assert (doc);
	g_assert (node);
	g_assert (attr);

	selected_menu_item = cong_eel_option_menu_get_selected_menu_item (option_menu);
	new_attr_value = g_object_get_data (G_OBJECT(selected_menu_item),
					    "attr_value");

	cong_document_begin_edit (doc);

	if (new_attr_value) {
		cong_document_node_set_attribute (doc, 
						  node, 
						  attr->name, 
						  new_attr_value);
	} else {
		cong_document_node_remove_attribute (doc, 
						     node, 
						     attr->name);
	}

	cong_document_end_edit (doc);
}

/* NOTATION support: */
