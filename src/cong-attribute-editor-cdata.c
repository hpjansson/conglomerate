/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-attribute-editor-cdata.c
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
#include "cong-attribute-editor-cdata.h"

#define PRIVATE(x) ((x)->private)

struct CongAttributeEditorCDATADetails
{
	GtkBox *hbox;
	GtkEntry *entry;
	GtkButton *add_btn;
	GtkButton *delete_btn;	
};

static void
on_set_attribute (CongDocument *doc, 
		  CongNodePtr node, 
		  const xmlChar *name, 
		  const xmlChar *value, 
		  CongAttributeEditorCDATA *attribute_editor_cdata);
static void
on_remove_attribute (CongDocument *doc, 
		     CongNodePtr node, 
		     const xmlChar *name,
		     CongAttributeEditorCDATA *attribute_editor_cdata);
static void
on_text_entry_changed (GtkEditable *editable,
		       CongAttributeEditorCDATA *attribute_editor_cdata);
static void
on_add_button (GtkButton *button,
	       CongAttributeEditorCDATA *attribute_editor_cdata);
static void
on_delete_button (GtkButton *button,
		  CongAttributeEditorCDATA *attribute_editor_cdata);
static void
do_refresh (CongAttributeEditorCDATA *attribute_editor_cdata);


/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongAttributeEditorCDATA, 
			cong_attribute_editor_cdata,
			CongAttributeEditor,
			CONG_ATTRIBUTE_EDITOR_TYPE);

static void
cong_attribute_editor_cdata_class_init (CongAttributeEditorCDATAClass *klass)
{
}

static void
cong_attribute_editor_cdata_instance_init (CongAttributeEditorCDATA *area)
{
	area->private = g_new0(CongAttributeEditorCDATADetails,1);
}

CongAttributeEditor*
cong_attribute_editor_cdata_construct (CongAttributeEditorCDATA *attribute_editor_cdata,
				       CongDocument *doc,
				       CongNodePtr node,
				       const gchar *attribute_name,
				       xmlAttributePtr attr)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR_CDATA(attribute_editor_cdata), NULL);

#if 1
	cong_attribute_editor_construct (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata),
					 doc,
					 node,
					 attribute_name,
					 attr);

	/* Build widgetry: */
	PRIVATE(attribute_editor_cdata)->hbox = GTK_BOX(gtk_hbox_new (FALSE, 6));
	PRIVATE(attribute_editor_cdata)->entry = GTK_ENTRY(gtk_entry_new ());
	PRIVATE(attribute_editor_cdata)->add_btn = GTK_BUTTON(gtk_button_new_from_stock (GTK_STOCK_ADD));
	PRIVATE(attribute_editor_cdata)->delete_btn = GTK_BUTTON(gtk_button_new_from_stock (GTK_STOCK_DELETE));

	gtk_box_pack_end(PRIVATE(attribute_editor_cdata)->hbox, GTK_WIDGET(PRIVATE(attribute_editor_cdata)->delete_btn), FALSE, FALSE, 0);
	gtk_box_pack_end(PRIVATE(attribute_editor_cdata)->hbox, GTK_WIDGET(PRIVATE(attribute_editor_cdata)->entry), TRUE, TRUE, 0);

	gtk_box_pack_end(PRIVATE(attribute_editor_cdata)->hbox, GTK_WIDGET(PRIVATE(attribute_editor_cdata)->add_btn), FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER(attribute_editor_cdata),
			   GTK_WIDGET(PRIVATE(attribute_editor_cdata)->hbox));

	do_refresh (attribute_editor_cdata);

	gtk_widget_show (GTK_WIDGET(PRIVATE(attribute_editor_cdata)->hbox));

	/* FIXME: disconnect this: */
	g_signal_connect_after (G_OBJECT(doc),
				"node_set_attribute",
				G_CALLBACK(on_set_attribute),
				attribute_editor_cdata);
	/* FIXME: disconnect this: */
	g_signal_connect_after (G_OBJECT(doc),
				"node_remove_attribute",
				G_CALLBACK(on_remove_attribute),
				attribute_editor_cdata);

	g_signal_connect_after (G_OBJECT(PRIVATE(attribute_editor_cdata)->entry),
				"changed",
				G_CALLBACK(on_text_entry_changed),
				attribute_editor_cdata);
	g_signal_connect_after (G_OBJECT(PRIVATE(attribute_editor_cdata)->add_btn),
				"clicked",
				G_CALLBACK(on_add_button),
				attribute_editor_cdata);
	g_signal_connect_after (G_OBJECT(PRIVATE(attribute_editor_cdata)->delete_btn),
				"clicked",
				G_CALLBACK(on_delete_button),
				attribute_editor_cdata);
#else
	{
		GtkWidget *label = gtk_label_new("fubar");

		g_assert (GTK_IS_HBOX (attribute_editor_cdata));
		
		gtk_box_pack_start (GTK_BOX(attribute_editor_cdata),
				    label,
				    FALSE,
				    FALSE,
				    0);

		gtk_widget_show (label);
	}
#endif

	return CONG_ATTRIBUTE_EDITOR (attribute_editor_cdata);
}

GtkWidget*
cong_attribute_editor_cdata_new (CongDocument *doc,
				 CongNodePtr node,
				 const gchar *attribute_name,
				 xmlAttributePtr attr)
{
	return GTK_WIDGET( cong_attribute_editor_cdata_construct
			   (g_object_new (CONG_ATTRIBUTE_EDITOR_CDATA_TYPE, NULL),
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
		  CongAttributeEditorCDATA *attribute_editor_cdata)
{
	do_refresh (attribute_editor_cdata);
}

static void
on_remove_attribute (CongDocument *doc, 
		     CongNodePtr node, 
		     const xmlChar *name,
		     CongAttributeEditorCDATA *attribute_editor_cdata)
{
	do_refresh (attribute_editor_cdata);
}

static void
on_text_entry_changed (GtkEditable *editable,
		       CongAttributeEditorCDATA *attribute_editor_cdata)
{
	CongDocument *doc = cong_attribute_editor_get_document (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));
	CongNodePtr node = cong_attribute_editor_get_node (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));
	const gchar *attribute_name = cong_attribute_editor_get_attribute_name (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));

	cong_document_begin_edit (doc);
	
	cong_document_node_set_attribute (doc, 
					  node, 
					  attribute_name,
					  gtk_entry_get_text (GTK_ENTRY(PRIVATE(attribute_editor_cdata)->entry)));
	
	cong_document_end_edit (doc);
}

static void
on_add_button (GtkButton *button,
	       CongAttributeEditorCDATA *attribute_editor_cdata)
{
	CongDocument *doc = cong_attribute_editor_get_document (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));
	CongNodePtr node = cong_attribute_editor_get_node (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));
	const gchar *attribute_name = cong_attribute_editor_get_attribute_name (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));

	cong_document_begin_edit (cong_attribute_editor_get_document (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata)));
	
	cong_document_node_set_attribute (doc, 
					  node, 
					  attribute_name,
					  "");
	
	cong_document_end_edit (cong_attribute_editor_get_document (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata)));
}

static void
on_delete_button (GtkButton *button,
		  CongAttributeEditorCDATA *attribute_editor_cdata)
{
	CongDocument *doc = cong_attribute_editor_get_document (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));
	CongNodePtr node = cong_attribute_editor_get_node (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));
	const gchar *attribute_name = cong_attribute_editor_get_attribute_name (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));

	cong_document_begin_edit (doc);
	
	cong_document_node_remove_attribute (doc,
					     node, 
					     attribute_name);
	
	cong_document_end_edit (doc);
}


static void
do_refresh (CongAttributeEditorCDATA *attribute_editor_cdata)
{
	gchar *attr_value = cong_attribute_editor_get_attribute_value (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));
	
	if (attr_value) {
		gtk_entry_set_text (GTK_ENTRY (PRIVATE(attribute_editor_cdata)->entry),
				    attr_value);

		g_free (attr_value);

		gtk_widget_show (GTK_WIDGET(PRIVATE(attribute_editor_cdata)->entry));
		gtk_widget_hide (GTK_WIDGET(PRIVATE(attribute_editor_cdata)->add_btn));
		gtk_widget_show (GTK_WIDGET(PRIVATE(attribute_editor_cdata)->delete_btn));
	} else {
		gtk_widget_hide (GTK_WIDGET(PRIVATE(attribute_editor_cdata)->entry));
		gtk_widget_show (GTK_WIDGET(PRIVATE(attribute_editor_cdata)->add_btn));
		gtk_widget_hide (GTK_WIDGET(PRIVATE(attribute_editor_cdata)->delete_btn));
	}
}
