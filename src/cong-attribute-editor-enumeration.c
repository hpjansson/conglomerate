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
#include "cong-eel.h"
#include "cong-command.h"
#include "cong-util.h"

#define PRIVATE(x) ((x)->private)

struct CongAttributeEditorENUMERATIONDetails
{
	GtkWidget *combo_box;
	xmlAttributePtr attr_ptr;

	guint handler_id_changed;
};

/* Internal function declarations: */
static void
set_attribute_handler (CongAttributeEditor *attribute_editor);
static void
remove_attribute_handler (CongAttributeEditor *attribute_editor);

static void
on_combo_box_changed (GtkComboBox *combo_box,
		      CongAttributeEditorENUMERATION *attribute_editor_enumeration);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongAttributeEditorENUMERATION, 
			cong_attribute_editor_enumeration,
			CongAttributeEditor,
			CONG_ATTRIBUTE_EDITOR_TYPE);

static void
cong_attribute_editor_enumeration_class_init (CongAttributeEditorENUMERATIONClass *klass)
{
	CongAttributeEditorClass *editor_klass = CONG_ATTRIBUTE_EDITOR_CLASS (klass);

	editor_klass->set_attribute_handler = set_attribute_handler;
	editor_klass->remove_attribute_handler = remove_attribute_handler;
}

static void
cong_attribute_editor_enumeration_instance_init (CongAttributeEditorENUMERATION *area)
{
	area->private = g_new0(CongAttributeEditorENUMERATIONDetails,1);
}

/**
 * cong_attribute_editor_enumeration_construct:
 * @attribute_editor_enumeration:
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
cong_attribute_editor_enumeration_construct (CongAttributeEditorENUMERATION *attribute_editor_enumeration,
					     CongDocument *doc,
					     CongNodePtr node,
					     xmlNs *ns_ptr,
					     const gchar *attribute_name,
					     xmlAttributePtr attr)
{
	xmlEnumerationPtr enum_ptr;
	gchar *attr_value;
	guint enum_ctr = 0, enum_pos = 0;

	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR_ENUMERATION(attribute_editor_enumeration), NULL);

	cong_attribute_editor_construct (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration),
					 doc,
					 node,
					 ns_ptr,
					 attribute_name,
					 attr);

	/*
	 * We store the xmlAttributePtr in the object so that we can access the values
	 * from the on_combo_box_changed handler. An alternative implementation would
	 * be to create a list of the enumerated values and store that in the object,
	 * but that involves more work for a small speed gain
	 */
	PRIVATE(attribute_editor_enumeration)->attr_ptr = attr;

	/* what is the current attribute setting? */
	attr_value = cong_attribute_editor_get_attribute_value (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration));

	/* Build widgetry: */
	PRIVATE(attribute_editor_enumeration)->combo_box = gtk_combo_box_new_text();
	
	gtk_combo_box_append_text (GTK_COMBO_BOX (PRIVATE(attribute_editor_enumeration)->combo_box),
				   _("(unspecified)"));

	for (enum_ptr=attr->tree, enum_ctr=1; enum_ptr; enum_ptr=enum_ptr->next, enum_ctr++) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (PRIVATE(attribute_editor_enumeration)->combo_box),
					   enum_ptr->name);

		/* is this the value of the current attribute? */
		if (cong_util_attribute_value_equality (attr_value,enum_ptr->name))
			enum_pos = enum_ctr;
	}
	
	/* Set the initial value of the combo box (defaults to unspecified if not found) */
	gtk_combo_box_set_active (GTK_COMBO_BOX(PRIVATE(attribute_editor_enumeration)->combo_box),
				  enum_pos);

	PRIVATE(attribute_editor_enumeration)->handler_id_changed = g_signal_connect_after (G_OBJECT(PRIVATE(attribute_editor_enumeration)->combo_box),
											    "changed",
											    G_CALLBACK(on_combo_box_changed),
											    attribute_editor_enumeration);
	
	gtk_widget_show_all (PRIVATE(attribute_editor_enumeration)->combo_box);

	gtk_container_add (GTK_CONTAINER(attribute_editor_enumeration),
			   GTK_WIDGET(PRIVATE(attribute_editor_enumeration)->combo_box));

	if (attr_value) {
		g_free (attr_value);
	}

	return CONG_ATTRIBUTE_EDITOR (attribute_editor_enumeration);
}

/**
 * cong_attribute_editor_enumeration_new:
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
cong_attribute_editor_enumeration_new (CongDocument *doc,
				       CongNodePtr node,
				       xmlNs *ns_ptr,
				       const gchar *attribute_name,
				       xmlAttributePtr attr)
{
	return GTK_WIDGET( cong_attribute_editor_enumeration_construct
			   (g_object_new (CONG_ATTRIBUTE_EDITOR_ENUMERATION_TYPE, NULL),
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
	/* FIXME */
}

static void
remove_attribute_handler (CongAttributeEditor *attribute_editor)
{
	CongAttributeEditorENUMERATION *attribute_editor_enumeration = CONG_ATTRIBUTE_EDITOR_ENUMERATION (attribute_editor);
	
	/* The initial item is the "unspecified" one: */
	gtk_combo_box_set_active (GTK_COMBO_BOX(PRIVATE(attribute_editor_enumeration)->combo_box),
				  0);
}

static void
on_combo_box_changed (GtkComboBox *combo_box,
		      CongAttributeEditorENUMERATION *attribute_editor_enumeration)
{
	xmlEnumerationPtr enum_ptr;
	gint selected, ctr;

	selected = gtk_combo_box_get_active (combo_box);
	if (selected) {
		enum_ptr = PRIVATE(attribute_editor_enumeration)->attr_ptr->tree;
		for (ctr=1; ctr<selected; ctr++) {
			enum_ptr = enum_ptr->next;
		}
		cong_attribute_editor_try_set_value (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration),
						     enum_ptr->name);
	} else {
		/* this deletes the attribute */
		cong_attribute_editor_try_set_value (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration),
						     NULL);
	}
}

