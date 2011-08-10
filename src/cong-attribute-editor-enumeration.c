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

struct _CongAttributeEditorEnumerationDetails
{
	GtkWidget *combo_box;
	GPtrArray *combo_array;
	guint handler_id_changed;
};

/* Internal function declarations: */
static void
finalize (GObject *object);

static void
dispose (GObject *object);

static void
set_attribute_handler (CongAttributeEditor *attribute_editor);
static void
remove_attribute_handler (CongAttributeEditor *attribute_editor);
static void
do_refresh (CongAttributeEditorEnumeration *attribute_editor_enumeration);

static void
on_selection_changed (GtkComboBox *combo_box,
		      CongAttributeEditorEnumeration *attribute_editor_enumeration);

/* Exported function definitions: */
G_DEFINE_TYPE(CongAttributeEditorEnumeration,
              cong_attribute_editor_enumeration,
              CONG_ATTRIBUTE_EDITOR_TYPE);

static void
cong_attribute_editor_enumeration_class_init (CongAttributeEditorEnumerationClass *klass)
{
	CongAttributeEditorClass *editor_klass = CONG_ATTRIBUTE_EDITOR_CLASS (klass);

	editor_klass->set_attribute_handler = set_attribute_handler;
	editor_klass->remove_attribute_handler = remove_attribute_handler;

	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;
}

static void
cong_attribute_editor_enumeration_init (CongAttributeEditorEnumeration *area)
{
	area->private = g_new0(CongAttributeEditorEnumerationDetails,1);
}

/**
 * cong_attribute_editor_enumeration_construct:
 * @attribute_editor_enumeration:
 * @doc: Document to which the node belongs.
 * @node: Node containing the attribute.
 * @ns_ptr:
 * @attribute_name: Name of the attribute.
 * @attr: Pointer to the attribute.
 *
 * Constructor for the widget.
 *
 * Returns:
 */
CongAttributeEditor*
cong_attribute_editor_enumeration_construct (CongAttributeEditorEnumeration *attribute_editor_enumeration,
					     CongDocument *doc,
					     CongNodePtr node,
					     xmlNs *ns_ptr,
					     const gchar *attribute_name,
					     xmlAttributePtr attr)
{
	xmlEnumerationPtr enum_ptr;

	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR_ENUMERATION(attribute_editor_enumeration), NULL);

	cong_attribute_editor_construct (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration),
					 doc,
					 node,
					 ns_ptr,
					 attribute_name);

	/* Build widgetry: */
	PRIVATE(attribute_editor_enumeration)->combo_box = gtk_combo_box_new_text ();
	PRIVATE(attribute_editor_enumeration)->combo_array = g_ptr_array_new ();
	
	/*
	 * We have one more element in the combo box than we do in the "combo array"
	 * since the first item indicates that the attribute is not set.
	 */
	gtk_combo_box_append_text (GTK_COMBO_BOX (PRIVATE(attribute_editor_enumeration)->combo_box),
				   _("(unspecified)"));
	for (enum_ptr=attr->tree; enum_ptr; enum_ptr=enum_ptr->next) {
		/*
		 * We do a deep copy of the items in the attr list. This is because
		 * I have seen bug # 122028, which is about storing xmlAttributePtr when
		 * the DTD may change. For this case I think it is probably over-kill,
		 * in that the widget would need to be torn down and re-built if the
		 * DTD were to change, but I have left it in just in case.
		 */
		gchar *text = g_strdup ((gchar*)enum_ptr->name);
		gtk_combo_box_append_text (GTK_COMBO_BOX (PRIVATE(attribute_editor_enumeration)->combo_box),
					   text);
		g_ptr_array_add (PRIVATE(attribute_editor_enumeration)->combo_array,
				 (gpointer)text );
	}
	
	PRIVATE(attribute_editor_enumeration)->handler_id_changed = g_signal_connect_after (G_OBJECT(PRIVATE(attribute_editor_enumeration)->combo_box),
											    "changed",
											    G_CALLBACK(on_selection_changed),
											    attribute_editor_enumeration);
	
	gtk_container_add (GTK_CONTAINER(attribute_editor_enumeration),
			   GTK_WIDGET(PRIVATE(attribute_editor_enumeration)->combo_box));

	/* not sure whether gtk_show_all() should be after the refresh call */
	do_refresh (attribute_editor_enumeration);
	gtk_widget_show_all (PRIVATE(attribute_editor_enumeration)->combo_box);

	return CONG_ATTRIBUTE_EDITOR (attribute_editor_enumeration);
}

/**
 * cong_attribute_editor_enumeration_new:
 * @doc: Document to which the node belongs.
 * @node: Node containing the attribute.
 * @ns_ptr:
 * @attribute_name: Name of the attribute.
 * @attr: Pointer to the attribute.
 *
 * Creates a widget that allows a user to create, edit, and delete
 * the value of an attribute (@attr) of type
 * XML_ATTRIBUTE_ENUMERATION.
 *
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
finalize (GObject *object)
{
	CongAttributeEditorEnumeration *attribute_editor_enumeration = CONG_ATTRIBUTE_EDITOR_ENUMERATION(object);

	g_free (PRIVATE(attribute_editor_enumeration));
	PRIVATE(attribute_editor_enumeration) = NULL;
	
	G_OBJECT_CLASS (cong_attribute_editor_enumeration_parent_class)->finalize (object);
}

static void
dispose (GObject *object)
{
	CongAttributeEditorEnumeration *attribute_editor_enumeration = CONG_ATTRIBUTE_EDITOR_ENUMERATION(object);

	if (PRIVATE(attribute_editor_enumeration)->combo_array) {
		/* free the array of enumerated values and the values themselves */
		g_ptr_array_free (PRIVATE(attribute_editor_enumeration)->combo_array, 1);
		PRIVATE(attribute_editor_enumeration)->combo_array = NULL;
	}

	G_OBJECT_CLASS (cong_attribute_editor_enumeration_parent_class)->dispose (object);
}

static void
set_attribute_handler (CongAttributeEditor *attribute_editor)
{
	do_refresh (CONG_ATTRIBUTE_EDITOR_ENUMERATION(attribute_editor));
}

static void
remove_attribute_handler (CongAttributeEditor *attribute_editor)
{
	do_refresh (CONG_ATTRIBUTE_EDITOR_ENUMERATION(attribute_editor));
}

static void
do_refresh (CongAttributeEditorEnumeration *attribute_editor_enumeration)
{
	gchar *attr_value;
	guint enum_pos = 0;

	/* what is the current attribute setting? */
	attr_value = cong_attribute_editor_get_attribute_value (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration));

	/* only loop if the attribute is defined */
	if (NULL!=attr_value) {
		GPtrArray *gparray = PRIVATE(attribute_editor_enumeration)->combo_array;
		guint ctr;
		for (ctr=0; ctr<gparray->len && enum_pos==0; ctr++) {
			/*
			 * Is this the value of the current attribute?
			 * If so we need to add on one to account for the first
			 * element of the combo array not being a member of the combo_array
			 */
			if (cong_util_attribute_value_equality (attr_value,
								(gchar*)g_ptr_array_index (gparray,ctr)))
				enum_pos = ctr + 1;
		}
		g_free (attr_value);
	}

	/*
	 * set the widget, taking care to block/unblock the changed signal
	 */
	g_signal_handler_block (G_OBJECT(PRIVATE(attribute_editor_enumeration)->combo_box),
				PRIVATE(attribute_editor_enumeration)->handler_id_changed);
	gtk_combo_box_set_active (GTK_COMBO_BOX(PRIVATE(attribute_editor_enumeration)->combo_box),
				  enum_pos);
	g_signal_handler_unblock (G_OBJECT(PRIVATE(attribute_editor_enumeration)->combo_box),
				  PRIVATE(attribute_editor_enumeration)->handler_id_changed);
}

static void
on_selection_changed (GtkComboBox *combo_box,
		      CongAttributeEditorEnumeration *attribute_editor_enumeration)
{
	gint selected;

	selected = gtk_combo_box_get_active (combo_box);
	if (selected) {
		/* note the -1 offset between the combo box and combo array indexes */
		gchar *newval = (gchar *)g_ptr_array_index (PRIVATE(attribute_editor_enumeration)->combo_array,
							    selected-1);
		cong_attribute_editor_try_set_value (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration),
						     newval);
	} else {
		/* this deletes the attribute */
		cong_attribute_editor_try_set_value (CONG_ATTRIBUTE_EDITOR(attribute_editor_enumeration),
						     NULL);
	}
}

