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
#include "cong-command.h"
#include "cong-attribute-editor-cdata.h"

#define PRIVATE(x) ((x)->private)

struct _CongAttributeEditorCDATADetails
{
	GtkBox *hbox;
	GtkEntry *entry;
	GtkButton *add_btn;
	GtkButton *delete_btn;	
	
	gulong handler_id_changed;
};

static void
set_attribute_handler (CongAttributeEditor *attribute_editor);
static void
remove_attribute_handler (CongAttributeEditor *attribute_editor);
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
	CongAttributeEditorClass *editor_klass = CONG_ATTRIBUTE_EDITOR_CLASS (klass);

	editor_klass->set_attribute_handler = set_attribute_handler;
	editor_klass->remove_attribute_handler = remove_attribute_handler;
}

static void
cong_attribute_editor_cdata_instance_init (CongAttributeEditorCDATA *area)
{
	area->private = g_new0(CongAttributeEditorCDATADetails,1);
}

/**
 * cong_attribute_editor_cdata_construct:
 * @attribute_editor_cdata:
 * @doc:
 * @node:
 * @ns_ptr:
 * @attribute_name:
 * @attr:
 *
 * TODO: Write me
 * Returns:
 */
CongAttributeEditor*
cong_attribute_editor_cdata_construct (CongAttributeEditorCDATA *attribute_editor_cdata,
				       CongDocument *doc,
				       CongNodePtr node,
				       xmlNs *ns_ptr,
				       const gchar *attribute_name,
				       xmlAttributePtr attr)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR_CDATA(attribute_editor_cdata), NULL);

#if 1
	cong_attribute_editor_construct (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata),
					 doc,
					 node,
					 ns_ptr,
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

	PRIVATE(attribute_editor_cdata)->handler_id_changed = g_signal_connect_after (G_OBJECT(PRIVATE(attribute_editor_cdata)->entry),
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

/**
 * cong_attribute_editor_cdata_new:
 * @doc:
 * @node:
 * @ns_ptr:
 * @attribute_name:
 * @attr:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget*
cong_attribute_editor_cdata_new (CongDocument *doc,
				 CongNodePtr node,
				 xmlNs *ns_ptr,
				 const gchar *attribute_name,
				 xmlAttributePtr attr)
{
	return GTK_WIDGET( cong_attribute_editor_cdata_construct
			   (g_object_new (CONG_ATTRIBUTE_EDITOR_CDATA_TYPE, NULL),
			    doc,
			    node,
			    ns_ptr,
			    attribute_name,
			    attr));			   
}

/* Internal function definitions: */
static void
set_attribute_handler (CongAttributeEditor *attribute_editor)
{
	do_refresh (CONG_ATTRIBUTE_EDITOR_CDATA(attribute_editor));
}

static void
remove_attribute_handler (CongAttributeEditor *attribute_editor)
{
	do_refresh (CONG_ATTRIBUTE_EDITOR_CDATA(attribute_editor));
}

static void
on_text_entry_changed (GtkEditable *editable,
		       CongAttributeEditorCDATA *attribute_editor_cdata)
{
	const gchar *value;

        value = gtk_entry_get_text (GTK_ENTRY(PRIVATE(attribute_editor_cdata)->entry));

        cong_attribute_editor_try_set_value (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata), value);
}

static void
on_add_button (GtkButton *button,
	       CongAttributeEditorCDATA *attribute_editor_cdata)
{
	CongDocument *doc = cong_attribute_editor_get_document (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));
	CongNodePtr node = cong_attribute_editor_get_node (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));
	const gchar *attribute_name = cong_attribute_editor_get_attribute_name (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));
	xmlNs *ns_ptr = cong_attribute_editor_get_ns (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));

	gchar *desc = g_strdup_printf ( _("Add attribute \"%s\""), attribute_name);

	CongCommand *cmd = cong_document_begin_command (doc,
							desc,
							NULL);

	g_free (desc);

	cong_command_add_node_set_attribute (cmd,
					     node,
					     ns_ptr,
					     attribute_name,
					     "");
	cong_document_end_command (doc,
				    cmd);
}

static void
on_delete_button (GtkButton *button,
		  CongAttributeEditorCDATA *attribute_editor_cdata)
{
	CongDocument *doc = cong_attribute_editor_get_document (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));
	CongNodePtr node = cong_attribute_editor_get_node (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));
	const gchar *attribute_name = cong_attribute_editor_get_attribute_name (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));
	xmlNs *ns_ptr = cong_attribute_editor_get_ns (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));

	gchar *desc = g_strdup_printf ( _("Delete attribute \"%s\""), attribute_name);

	CongCommand *cmd = cong_document_begin_command (doc,
							desc,
							NULL);

	g_free (desc);

	cong_command_add_node_remove_attribute (cmd,
						node,
						ns_ptr,
						attribute_name);
	cong_document_end_command (doc,
				   cmd);
}


static void
do_refresh (CongAttributeEditorCDATA *attribute_editor_cdata)
{
	gchar *attr_value = cong_attribute_editor_get_attribute_value (CONG_ATTRIBUTE_EDITOR(attribute_editor_cdata));
	
	if (attr_value) {
		g_signal_handler_block ( G_OBJECT(PRIVATE(attribute_editor_cdata)->entry),
					 PRIVATE(attribute_editor_cdata)->handler_id_changed);
		gtk_entry_set_text (GTK_ENTRY (PRIVATE(attribute_editor_cdata)->entry),
				    attr_value);
		g_signal_handler_unblock ( G_OBJECT(PRIVATE(attribute_editor_cdata)->entry),
					 PRIVATE(attribute_editor_cdata)->handler_id_changed);

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

