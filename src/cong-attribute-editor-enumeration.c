/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-attribute-editor-enumeration.c
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
#include "cong-attribute-editor-enumeration.h"

#define PRIVATE(x) ((x)->private)

struct CongAttributeEditorENUMERATIONDetails
{
	GtkWidget *option_menu;
	GtkWidget *menu;
};

static void
on_set_attribute (CongDocument *doc, 
		  CongNodePtr node, 
		  const xmlChar *name, 
		  const xmlChar *value, 
		  CongAttributeEditorENUMERATION *attribute_editor_enumeration);
static void
on_remove_attribute (CongDocument *doc, 
		     CongNodePtr node, 
		     const xmlChar *name,
		     CongAttributeEditorENUMERATION *attribute_editor_enumeration);
static void
on_option_menu_changed (GtkOptionMenu *option_menu,
			CongAttributeEditorENUMERATION *attribute_editor_enumeration);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongAttributeEditorENUMERATION, 
			cong_attribute_editor_enumeration,
			CongAttributeEditor,
			CONG_ATTRIBUTE_EDITOR_TYPE);

static void
cong_attribute_editor_enumeration_class_init (CongAttributeEditorENUMERATIONClass *klass)
{
}

static void
cong_attribute_editor_enumeration_instance_init (CongAttributeEditorENUMERATION *area)
{
	area->private = g_new0(CongAttributeEditorENUMERATIONDetails,1);
}

CongAttributeEditor*
cong_attribute_editor_enumeration_construct (CongAttributeEditorENUMERATION *attribute_editor_enumeration,
				       CongDocument *doc,
				       CongNodePtr node,
				       const gchar *attribute_name,
				       xmlAttributePtr attr)
{
	xmlEnumerationPtr enum_ptr;

	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR_ENUMERATION(attribute_editor_enumeration), NULL);

	cong_attribute_editor_construct (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration),
					 doc,
					 node,
					 attribute_name,
					 attr);
	/* Build widgetry: */
	PRIVATE(attribute_editor_enumeration)->option_menu = gtk_option_menu_new();
	PRIVATE(attribute_editor_enumeration)->menu = gtk_menu_new();

	gtk_menu_shell_append (GTK_MENU_SHELL(PRIVATE(attribute_editor_enumeration)->menu), 
			       gtk_menu_item_new_with_label(_("(unspecified)")));
	
	for (enum_ptr=attr->tree; enum_ptr; enum_ptr=enum_ptr->next) {
		GtkWidget *menu_item = gtk_menu_item_new_with_label(enum_ptr->name);
		
		g_object_set_data (G_OBJECT(menu_item),
				   "attr_value",
				   enum_ptr->name);
		
		gtk_menu_shell_append(GTK_MENU_SHELL(PRIVATE(attribute_editor_enumeration)->menu), 
				      menu_item);
	}
	
	gtk_option_menu_set_menu(GTK_OPTION_MENU(PRIVATE(attribute_editor_enumeration)->option_menu), 
				 PRIVATE(attribute_editor_enumeration)->menu);
	
	/* Disconnect this: */
	g_signal_connect_after (G_OBJECT(doc),
				"node_set_attribute",
				G_CALLBACK(on_set_attribute),
				attribute_editor_enumeration);
	/* Disconnect this: */
	g_signal_connect_after (G_OBJECT(doc),
				"node_remove_attribute",
				G_CALLBACK(on_remove_attribute),
				attribute_editor_enumeration);
	
	g_signal_connect_after (G_OBJECT(PRIVATE(attribute_editor_enumeration)->option_menu),
				"changed",
				G_CALLBACK(on_option_menu_changed),
				attribute_editor_enumeration);
	
	gtk_widget_show_all (PRIVATE(attribute_editor_enumeration)->option_menu);

	gtk_container_add (GTK_CONTAINER(attribute_editor_enumeration),
			   GTK_WIDGET(PRIVATE(attribute_editor_enumeration)->option_menu));

	
	return CONG_ATTRIBUTE_EDITOR (attribute_editor_enumeration);
}

GtkWidget*
cong_attribute_editor_enumeration_new (CongDocument *doc,
				 CongNodePtr node,
				 const gchar *attribute_name,
				 xmlAttributePtr attr)
{
	return GTK_WIDGET( cong_attribute_editor_enumeration_construct
			   (g_object_new (CONG_ATTRIBUTE_EDITOR_ENUMERATION_TYPE, NULL),
			    doc,
			    node,
			    attribute_name,
			    attr));			   
}

/* Internal function definitions: */
static void
on_set_attribute (CongDocument *doc, 
		  CongNodePtr node, 
		  const xmlChar *name, 
		  const xmlChar *value, 
		  CongAttributeEditorENUMERATION *attribute_editor_enumeration)
{
	/* FIXME */
}

static void
on_remove_attribute (CongDocument *doc, 
		     CongNodePtr node, 
		     const xmlChar *name,
		     CongAttributeEditorENUMERATION *attribute_editor_enumeration)	     
{
	if (doc == cong_attribute_editor_get_document (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration))) {
		if (node == cong_attribute_editor_get_node (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration))) {
			xmlAttributePtr attr = cong_attribute_editor_get_attribute (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration));
			g_assert (attr);

			if (0 == strcmp(name, attr->name)) {
				/* The initial item is the "unspecified" one: */
				gtk_option_menu_set_history (GTK_OPTION_MENU(PRIVATE(attribute_editor_enumeration)->option_menu),
							     0);
			}
		}
	}
}

static void
on_option_menu_changed (GtkOptionMenu *option_menu,
			CongAttributeEditorENUMERATION *attribute_editor_enumeration)
{
	CongDocument *doc = cong_attribute_editor_get_document (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration));
	CongNodePtr node = cong_attribute_editor_get_node (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration));
	xmlAttributePtr attr = cong_attribute_editor_get_attribute (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration));

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
