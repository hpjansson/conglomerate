/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-attribute-editor-nmtoken.c
 *
 * Copyright (C) 2004 David Malcolm
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
 * Authors: Douglas Burke <dburke@cfa.harvard.edu>
 * Based on code by David Malcolm <david@davemalcolm.demon.co.uk>
 */

#include "global.h"
#include "cong-command.h"
#include "cong-attribute-editor-nmtoken.h"
#include "cong-primary-window.h"
#include "cong-error-dialog.h"

#define PRIVATE(x) ((x)->private)

struct _CongAttributeEditorNMTOKENDetails
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
		       CongAttributeEditorNMTOKEN *attribute_editor_nmtoken);
static void
on_add_button (GtkButton *button,
	       CongAttributeEditorNMTOKEN *attribute_editor_nmtoken);
static void
on_delete_button (GtkButton *button,
		  CongAttributeEditorNMTOKEN *attribute_editor_nmtoken);
static void
do_refresh (CongAttributeEditorNMTOKEN *attribute_editor_nmtoken);


/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongAttributeEditorNMTOKEN, 
			cong_attribute_editor_nmtoken,
			CongAttributeEditor,
			CONG_ATTRIBUTE_EDITOR_TYPE);

static void
cong_attribute_editor_nmtoken_class_init (CongAttributeEditorNMTOKENClass *klass)
{
	CongAttributeEditorClass *editor_klass = CONG_ATTRIBUTE_EDITOR_CLASS (klass);

	editor_klass->set_attribute_handler = set_attribute_handler;
	editor_klass->remove_attribute_handler = remove_attribute_handler;
}

static void
cong_attribute_editor_nmtoken_instance_init (CongAttributeEditorNMTOKEN *area)
{
	area->private = g_new0(CongAttributeEditorNMTOKENDetails,1);
}

/**
 * cong_attribute_editor_nmtoken_construct:
 * @attribute_editor_nmtoken:
 * @doc:
 * @node:
 * @ns_ptr:
 * @attribute_name:
 * @attr:
 *
 * Constructror called by #cong_attribute_editor_nmtoken_new()
 *
 * Returns:
 */
CongAttributeEditor*
cong_attribute_editor_nmtoken_construct (CongAttributeEditorNMTOKEN *attribute_editor_nmtoken,
					 CongDocument *doc,
					 CongNodePtr node,
					 xmlNs *ns_ptr,
					 const gchar *attribute_name,
					 xmlAttributePtr attr)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR_NMTOKEN(attribute_editor_nmtoken), NULL);

	cong_attribute_editor_construct (CONG_ATTRIBUTE_EDITOR(attribute_editor_nmtoken),
					 doc,
					 node,
					 ns_ptr,
					 attribute_name,
					 attr);

	/* Build widgetry: */
	PRIVATE(attribute_editor_nmtoken)->hbox = GTK_BOX(gtk_hbox_new (FALSE, 6));
	PRIVATE(attribute_editor_nmtoken)->entry = GTK_ENTRY(gtk_entry_new ());
	PRIVATE(attribute_editor_nmtoken)->add_btn = GTK_BUTTON(gtk_button_new_from_stock (GTK_STOCK_ADD));
	PRIVATE(attribute_editor_nmtoken)->delete_btn = GTK_BUTTON(gtk_button_new_from_stock (GTK_STOCK_DELETE));

	gtk_box_pack_end(PRIVATE(attribute_editor_nmtoken)->hbox, GTK_WIDGET(PRIVATE(attribute_editor_nmtoken)->delete_btn), FALSE, FALSE, 0);
	gtk_box_pack_end(PRIVATE(attribute_editor_nmtoken)->hbox, GTK_WIDGET(PRIVATE(attribute_editor_nmtoken)->entry), TRUE, TRUE, 0);

	gtk_box_pack_end(PRIVATE(attribute_editor_nmtoken)->hbox, GTK_WIDGET(PRIVATE(attribute_editor_nmtoken)->add_btn), FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER(attribute_editor_nmtoken),
			   GTK_WIDGET(PRIVATE(attribute_editor_nmtoken)->hbox));

	do_refresh (attribute_editor_nmtoken);

	gtk_widget_show (GTK_WIDGET(PRIVATE(attribute_editor_nmtoken)->hbox));

	PRIVATE(attribute_editor_nmtoken)->handler_id_changed = g_signal_connect_after (G_OBJECT(PRIVATE(attribute_editor_nmtoken)->entry),
    				"changed",
				G_CALLBACK(on_text_entry_changed),
				attribute_editor_nmtoken);
	g_signal_connect_after (G_OBJECT(PRIVATE(attribute_editor_nmtoken)->add_btn),
				"clicked",
				G_CALLBACK(on_add_button),
				attribute_editor_nmtoken);
	g_signal_connect_after (G_OBJECT(PRIVATE(attribute_editor_nmtoken)->delete_btn),
				"clicked",
				G_CALLBACK(on_delete_button),
				attribute_editor_nmtoken);

	return CONG_ATTRIBUTE_EDITOR (attribute_editor_nmtoken);
}

/**
 * cong_attribute_editor_nmtoken_new:
 * @doc:
 * @node:
 * @ns_ptr:
 * @attribute_name:
 * @attr:
 *
 * Creates a #GtkWidget that is used to display/edit the
 * supplied attribute (with name given by the attribute_name
 * argument). This attribute should be an XML name token.
 *
 * At the moment this is a striaght copy of the #cong_attribute_editor_cdata_new
 * function. I think it would make more sense if this were a sub-class of
 * the "CDATA" object (perhaps with a minor name change to make it less CDATA-specific)?
 *
 * Should there be any user-visible indication that the item is a NMTOKEN
 * rather than CDATA?
 *
 * Returns:
 */
GtkWidget*
cong_attribute_editor_nmtoken_new (CongDocument *doc,
				   CongNodePtr node,
				   xmlNs *ns_ptr,
				   const gchar *attribute_name,
				   xmlAttributePtr attr)
{
	return GTK_WIDGET( cong_attribute_editor_nmtoken_construct
			   (g_object_new (CONG_ATTRIBUTE_EDITOR_NMTOKEN_TYPE, NULL),
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
	do_refresh (CONG_ATTRIBUTE_EDITOR_NMTOKEN(attribute_editor));
}

static void
remove_attribute_handler (CongAttributeEditor *attribute_editor)
{
	do_refresh (CONG_ATTRIBUTE_EDITOR_NMTOKEN(attribute_editor));
}

static void
on_text_entry_changed (GtkEditable *editable,
		       CongAttributeEditorNMTOKEN *attribute_editor_nmtoken)
{
	const gchar *value;

        value = gtk_entry_get_text (GTK_ENTRY(PRIVATE(attribute_editor_nmtoken)->entry));

	/* need to perform validation here */
	if ( xmlValidateNmtokenValue(value) ) {
		cong_attribute_editor_try_set_value (CONG_ATTRIBUTE_EDITOR(attribute_editor_nmtoken), value);

	} else {
		/* are there mem leaks here */
		gchar *what_failed = g_strdup_printf (_("Unable to set attribute \"%s\" to \"%s\""),
						      cong_attribute_editor_get_attribute_name (CONG_ATTRIBUTE_EDITOR(attribute_editor_nmtoken)),
						      value);
		CongDocument *doc = cong_attribute_editor_get_document (CONG_ATTRIBUTE_EDITOR(attribute_editor_nmtoken));
		GtkWindow *parent_window;
		GtkDialog *dialog;

		const gchar *curr_value;

		parent_window = cong_primary_window_get_toplevel (cong_document_get_primary_window (doc));
		dialog = cong_error_dialog_new (parent_window,
						what_failed,
						_("The attribute value must be an XML name token."),
						_("XML name tokens can only contain alphanumeric and/or ideographic characters and the punctuation marks _,-, ., and :. It can not contain whitespace characters."));


		cong_error_dialog_run (dialog);
		gtk_widget_destroy (GTK_WIDGET(dialog));

		/* 
		 * restore the previous value: should this be done *before* the dialog is called
		 *
		 * I think we want to block the signals, although I guess other parts of the
		 * code could now think that the attribute value has changed?
		 */
		curr_value = cong_attribute_editor_get_attribute_value (CONG_ATTRIBUTE_EDITOR(attribute_editor_nmtoken));
		g_signal_handler_block ( G_OBJECT(PRIVATE(attribute_editor_nmtoken)->entry),
					 PRIVATE(attribute_editor_nmtoken)->handler_id_changed);
		gtk_entry_set_text (GTK_ENTRY(PRIVATE(attribute_editor_nmtoken)->entry), curr_value);
		g_signal_handler_unblock ( G_OBJECT(PRIVATE(attribute_editor_nmtoken)->entry),
					 PRIVATE(attribute_editor_nmtoken)->handler_id_changed);

		return;
	}
}

static void
on_add_button (GtkButton *button,
	       CongAttributeEditorNMTOKEN *attribute_editor_nmtoken)
{
	CongDocument *doc = cong_attribute_editor_get_document (CONG_ATTRIBUTE_EDITOR(attribute_editor_nmtoken));
	CongNodePtr node = cong_attribute_editor_get_node (CONG_ATTRIBUTE_EDITOR(attribute_editor_nmtoken));
	const gchar *attribute_name = cong_attribute_editor_get_attribute_name (CONG_ATTRIBUTE_EDITOR(attribute_editor_nmtoken));
	xmlNs *ns_ptr = cong_attribute_editor_get_ns (CONG_ATTRIBUTE_EDITOR(attribute_editor_nmtoken));

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
		  CongAttributeEditorNMTOKEN *attribute_editor_nmtoken)
{
	CongDocument *doc = cong_attribute_editor_get_document (CONG_ATTRIBUTE_EDITOR(attribute_editor_nmtoken));
	CongNodePtr node = cong_attribute_editor_get_node (CONG_ATTRIBUTE_EDITOR(attribute_editor_nmtoken));
	const gchar *attribute_name = cong_attribute_editor_get_attribute_name (CONG_ATTRIBUTE_EDITOR(attribute_editor_nmtoken));
	xmlNs *ns_ptr = cong_attribute_editor_get_ns (CONG_ATTRIBUTE_EDITOR(attribute_editor_nmtoken));

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
do_refresh (CongAttributeEditorNMTOKEN *attribute_editor_nmtoken)
{
	gchar *attr_value = cong_attribute_editor_get_attribute_value (CONG_ATTRIBUTE_EDITOR(attribute_editor_nmtoken));
	
	if (attr_value) {
		g_signal_handler_block ( G_OBJECT(PRIVATE(attribute_editor_nmtoken)->entry),
					 PRIVATE(attribute_editor_nmtoken)->handler_id_changed);
		gtk_entry_set_text (GTK_ENTRY (PRIVATE(attribute_editor_nmtoken)->entry),
				    attr_value);
		g_signal_handler_unblock ( G_OBJECT(PRIVATE(attribute_editor_nmtoken)->entry),
					 PRIVATE(attribute_editor_nmtoken)->handler_id_changed);

		g_free (attr_value);

		gtk_widget_show (GTK_WIDGET(PRIVATE(attribute_editor_nmtoken)->entry));
		gtk_widget_hide (GTK_WIDGET(PRIVATE(attribute_editor_nmtoken)->add_btn));
		gtk_widget_show (GTK_WIDGET(PRIVATE(attribute_editor_nmtoken)->delete_btn));
	} else {
		gtk_widget_hide (GTK_WIDGET(PRIVATE(attribute_editor_nmtoken)->entry));
		gtk_widget_show (GTK_WIDGET(PRIVATE(attribute_editor_nmtoken)->add_btn));
		gtk_widget_hide (GTK_WIDGET(PRIVATE(attribute_editor_nmtoken)->delete_btn));
	}
}
