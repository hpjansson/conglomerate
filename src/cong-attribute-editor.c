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
#include "cong-attribute-wrapper.h"
#include "cong-attribute-wrapper-radio-button.h"
#include "cong-attribute-wrapper-check-button.h"
#include "cong-eel.h"
#include "cong-util.h"

#define PRIVATE(x) ((x)->private)

/* Internal types: */
struct CongAttributeEditorDetails
{
	CongDocument *doc;
	CongNodePtr node;
	xmlNs *ns_ptr;
	gchar *attribute_name;
	xmlAttributePtr attr; /* can be NULL */

	gulong handler_id_node_set_attribute;
	gulong handler_id_node_remove_attribute;
};

/* Internal function declarations: */
static void
finalize (GObject *object);

static void
dispose (GObject *object);

static void
on_set_attribute (CongDocument *doc, 
		  CongNodePtr node, 
		  const xmlNs *ns_ptr,
		  const xmlChar *name, 
		  const xmlChar *value, 
		  CongAttributeEditor *attribute_editor);
static void
on_remove_attribute (CongDocument *doc, 
		     CongNodePtr node, 
		     const xmlNs *ns_ptr, 
		     const xmlChar *name,
		     CongAttributeEditor *attribute_editor);

static void
bind_wrapper_to_widget (CongAttributeWrapper* wrapper,
			GtkWidget *widget);


/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongAttributeEditor, 
			cong_attribute_editor,
			GtkHBox,
			GTK_TYPE_HBOX);

CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_attribute_editor, set_attribute_handler);
CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_attribute_editor, remove_attribute_handler);

static void
cong_attribute_editor_class_init (CongAttributeEditorClass *klass)
{
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_attribute_editor,
					      set_attribute_handler);
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_attribute_editor,
					      remove_attribute_handler);

	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;
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
				 xmlNs *ns_ptr,
				 const gchar *attribute_name,
				 xmlAttributePtr attr)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR(attribute_editor), NULL);

	PRIVATE(attribute_editor)->doc = doc;
	g_object_ref(doc); /*FIXME: need to unref */

	PRIVATE(attribute_editor)->node = node;
	PRIVATE(attribute_editor)->attribute_name = g_strdup(attribute_name); /* FIXME: need to release */
	PRIVATE(attribute_editor)->attr = attr;

	PRIVATE(attribute_editor)->ns_ptr = ns_ptr;

	PRIVATE(attribute_editor)->handler_id_node_set_attribute = g_signal_connect_after (G_OBJECT(doc),
											   "node_set_attribute",
											   G_CALLBACK(on_set_attribute),
											   attribute_editor);

	PRIVATE(attribute_editor)->handler_id_node_remove_attribute = g_signal_connect_after (G_OBJECT(doc),
											      "node_remove_attribute",
											      G_CALLBACK(on_remove_attribute),
											      attribute_editor);
	
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

xmlNs *
cong_attribute_editor_get_ns (CongAttributeEditor *attribute_editor)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR(attribute_editor), NULL);

	return PRIVATE(attribute_editor)->ns_ptr;	
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
					PRIVATE(attribute_editor)->ns_ptr,
					PRIVATE(attribute_editor)->attribute_name);
}

GtkWidget*
cong_attribute_editor_new (CongDocument *doc,
			   CongNodePtr node,
			   xmlAttributePtr attr)
{
	xmlNs *ns_ptr;

	g_return_val_if_fail (doc, NULL);
	g_return_val_if_fail (node, NULL);
	g_return_val_if_fail (attr, NULL);

	/* ##FIXME: is this the right way? */
	if(attr->prefix != NULL) {
		ns_ptr = cong_node_get_ns_for_prefix(node, attr->prefix);
	} else {
		ns_ptr = NULL;
	}

	switch (attr->atype) {
	default: g_assert_not_reached();
	case XML_ATTRIBUTE_CDATA:
		return cong_attribute_editor_cdata_new (doc,
							node,
						        ns_ptr,
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
							      ns_ptr,
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

	/* FIXME: Should we store the namespace URI somewhere or is the prefix enough. */

	const char *local_name;
	xmlNs *ns_ptr = cong_node_get_attr_ns(global_glade_node_ptr,
					      func_name,
					      &local_name);

	custom_widget = cong_attribute_editor_cdata_new (global_glade_doc_ptr, 
							 global_glade_node_ptr, 
							 ns_ptr,
							 local_name,
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

void
cong_bind_radio_button (GtkRadioButton *radio_button,
			CongDocument *doc,
			CongNodePtr node,
			xmlNs *ns_ptr,
			const gchar *attribute_name,
			const gchar *attribute_value)
{
	CongAttributeWrapperRadioButton* wrapper = cong_attribute_wrapper_radio_button_new ( doc,
											     node,
											     ns_ptr,
											     attribute_name,
											     NULL,
											     radio_button,
											     attribute_value);
	bind_wrapper_to_widget (CONG_ATTRIBUTE_WRAPPER (wrapper),
				GTK_WIDGET (radio_button));
}


void
cong_bind_check_button (GtkCheckButton *check_button,
			CongDocument *doc,
			CongNodePtr node,
			xmlNs *ns_ptr,
			const gchar *attribute_name,
			const gchar *attribute_value_unchecked,
			const gchar *attribute_value_checked)
{
	CongAttributeWrapperCheckButton* wrapper = cong_attribute_wrapper_check_button_new ( doc,
											     node,
											     ns_ptr,
											     attribute_name,
											     NULL,
											     check_button,
											     attribute_value_unchecked,
											     attribute_value_checked);
	bind_wrapper_to_widget (CONG_ATTRIBUTE_WRAPPER (wrapper),
				GTK_WIDGET (check_button));
}


/* Internal function definitions: */
static void
finalize (GObject *object)
{
	CongAttributeEditor *attribute_editor = CONG_ATTRIBUTE_EDITOR(object);
	
	g_free (attribute_editor->private);
	attribute_editor->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
dispose (GObject *object)
{
	CongAttributeEditor *attribute_editor = CONG_ATTRIBUTE_EDITOR(object);

	if (PRIVATE(attribute_editor)->doc) {
	
		g_signal_handler_disconnect (G_OBJECT (PRIVATE(attribute_editor)->doc),
					     PRIVATE(attribute_editor)->handler_id_node_set_attribute);
		g_signal_handler_disconnect (G_OBJECT (PRIVATE(attribute_editor)->doc),
					     PRIVATE(attribute_editor)->handler_id_node_remove_attribute);
		
		g_object_unref (G_OBJECT (PRIVATE(attribute_editor)->doc));
		PRIVATE(attribute_editor)->doc = NULL;
		
		g_free (PRIVATE(attribute_editor)->attribute_name);
	}
		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
on_set_attribute (CongDocument *doc, 
		  CongNodePtr node, 
		  const xmlNs *ns_ptr, 
		  const xmlChar *name, 
		  const xmlChar *value, 
		  CongAttributeEditor *attribute_editor)
{
	g_return_if_fail (IS_CONG_ATTRIBUTE_EDITOR(attribute_editor));

	if (node == cong_attribute_editor_get_node (attribute_editor)) {
		if (0 == strcmp(name, cong_attribute_editor_get_attribute_name (attribute_editor)) &&
		    cong_util_ns_equality (ns_ptr, cong_attribute_editor_get_ns (attribute_editor))) {
			CONG_EEL_CALL_METHOD (CONG_ATTRIBUTE_EDITOR_CLASS,
					      attribute_editor,
					      set_attribute_handler, 
					      (attribute_editor));
		}
	}
}

static void
on_remove_attribute (CongDocument *doc, 
		     CongNodePtr node, 
		     const xmlNs *ns_ptr, 
		     const xmlChar *name,
		     CongAttributeEditor *attribute_editor)
{
	g_return_if_fail (IS_CONG_ATTRIBUTE_EDITOR(attribute_editor));

	if (node == cong_attribute_editor_get_node (attribute_editor)) {
		if (0 == strcmp(name, cong_attribute_editor_get_attribute_name (attribute_editor)) &&
		    cong_util_ns_equality (ns_ptr, cong_attribute_editor_get_ns (attribute_editor))) {
			CONG_EEL_CALL_METHOD (CONG_ATTRIBUTE_EDITOR_CLASS,
					      attribute_editor,
					      remove_attribute_handler,
					      (attribute_editor));
		}
	}
}

static void
bind_wrapper_to_widget (CongAttributeWrapper* wrapper,
			GtkWidget *widget)
{
     g_return_if_fail (IS_CONG_ATTRIBUTE_WRAPPER (wrapper));

     /* Leak it for now? Or attach as generic data and attach a signal handler to the widget's destroy signal? */
}
