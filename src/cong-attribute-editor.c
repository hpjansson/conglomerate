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
#include "cong-attribute-editor-cdata.h"
#include "cong-attribute-editor-enumeration.h"

#define PRIVATE(x) ((x)->private)

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
	xmlAttributePtr attr; /* can be NULL */
};

/* Internal function declarations: */

/* CDATA support: */
/* ID support: */
/* IDREF support: */
/* IDREFS support: */
/* ENTITY support: */
/* ENTITIES support: */
/* NMTOKEN support: */
/* NMTOKENS support: */
/* ENUMERATION support: */
/* NOTATION support: */

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongAttributeEditor, 
			cong_attribute_editor,
			GtkHBox,
			GTK_TYPE_HBOX);

static void
cong_attribute_editor_class_init (CongAttributeEditorClass *klass)
{
}

static void
cong_attribute_editor_instance_init (CongAttributeEditor *attribute_editor)
{
	attribute_editor->private = g_new0(CongAttributeEditorDetails,1);
}

CongAttributeEditor*
cong_attribute_editor_construct (CongAttributeEditor *attribute_editor,
				 CongDocument *doc,
				 CongNodePtr node,
				 const gchar *attribute_name,
				 xmlAttributePtr attr)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR(attribute_editor), NULL);

	PRIVATE(attribute_editor)->doc = doc;
	g_object_ref(doc); /*FIXME: need to unref */

	PRIVATE(attribute_editor)->node = node;
	PRIVATE(attribute_editor)->attribute_name = g_strdup(attribute_name); /* FIXME: need to release */
	PRIVATE(attribute_editor)->attr = attr;

	return CONG_ATTRIBUTE_EDITOR (attribute_editor);
}

CongDocument*
cong_attribute_editor_get_document (CongAttributeEditor *attribute_editor)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR(attribute_editor), NULL);

	return PRIVATE(attribute_editor)->doc; 
}

CongNodePtr
cong_attribute_editor_get_node (CongAttributeEditor *attribute_editor)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR(attribute_editor), NULL);

	return PRIVATE(attribute_editor)->node;
}

xmlAttributePtr
cong_attribute_editor_get_attribute (CongAttributeEditor *attribute_editor)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR(attribute_editor), NULL);

	return PRIVATE(attribute_editor)->attr;
}

const gchar*
cong_attribute_editor_get_attribute_name (CongAttributeEditor *attribute_editor)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR(attribute_editor), NULL);

	return PRIVATE(attribute_editor)->attribute_name;
}

gchar*
cong_attribute_editor_get_attribute_value (CongAttributeEditor *attribute_editor)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR(attribute_editor), NULL);

	return cong_node_get_attribute (PRIVATE(attribute_editor)->node, 
					PRIVATE(attribute_editor)->attribute_name);
}

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
							attr->name,
							attr);
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
		return cong_attribute_editor_enumeration_new (doc,
							      node,
							      attr->name,
							      attr);
	case XML_ATTRIBUTE_NOTATION:
		/* FIXME: some kind of text entry? */
		return gtk_label_new("NOTATION");
	}

	/* FIXME: need to set up callbacks: when someone else modifies attrs, and when this widget modifies attr! Also need to set initial value! */

	g_assert_not_reached();	
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
							 func_name,
							 NULL);
#else
	custom_widget = gtk_label_new(g_strdup_printf("custom widget \"%s\" \"%s\" \"%s\" \"%s\" %i %i", func_name, name, string1, string2, int1, int2)); /* for now */

	gtk_widget_show_all(custom_widget);
#endif

	gtk_widget_show (custom_widget);

	return custom_widget;
}

CongDocument *global_glade_doc_ptr = NULL;
CongNodePtr global_glade_node_ptr = NULL;


/* Internal function definitions: */
/* ID support: */
/* IDREF support: */
/* IDREFS support: */
/* ENTITY support: */
/* ENTITIES support: */
/* NMTOKEN support: */
/* NMTOKENS support: */

/* ENUMERATION support: */

/* NOTATION support: */
