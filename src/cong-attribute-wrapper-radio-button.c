/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-attribute-wrapper-radio-button.c
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
#include "cong-attribute-wrapper-radio-button.h"
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

/* Internal types: */
struct CongAttributeWrapperRadioButtonDetails
{
	GtkRadioButton *radio_button;
	gchar *attribute_value;	

	gulong handler_id_toggled;
};

/* Internal function declarations: */
static void
set_attribute_handler (CongAttributeWrapper *attribute_wrapper);
static void
remove_attribute_handler (CongAttributeWrapper *attribute_wrapper);

static void
finalize (GObject *object);

static void
dispose (GObject *object);

static void
on_toggled (GtkToggleButton *togglebutton,
	    CongAttributeWrapperRadioButton *attribute_wrapper);

static void
do_refresh (CongAttributeWrapperRadioButton *attribute_wrapper_radio_button);

static gboolean 
should_button_be_active (CongAttributeWrapperRadioButton *attribute_wrapper);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongAttributeWrapperRadioButton, 
			cong_attribute_wrapper_radio_button,
			CongAttributeWrapper,
			CONG_ATTRIBUTE_WRAPPER_TYPE);

static void
cong_attribute_wrapper_radio_button_class_init (CongAttributeWrapperRadioButtonClass *klass)
{
	CongAttributeWrapperClass *wrapper_klass = CONG_ATTRIBUTE_WRAPPER_CLASS (klass);

	wrapper_klass->set_attribute_handler = set_attribute_handler;
	wrapper_klass->remove_attribute_handler = remove_attribute_handler;

	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;
}

static void
cong_attribute_wrapper_radio_button_instance_init (CongAttributeWrapperRadioButton *attribute_wrapper)
{
	attribute_wrapper->private = g_new0(CongAttributeWrapperRadioButtonDetails,1);
}

CongAttributeWrapperRadioButton*
cong_attribute_wrapper_radio_button_construct (CongAttributeWrapperRadioButton *attribute_wrapper,
					       CongDocument *doc,
					       CongNodePtr node,
					       xmlNs *ns_ptr,
					       const gchar *attribute_name,
					       xmlAttributePtr attr,
					       GtkRadioButton *radio_button,
					       const gchar *attribute_value)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON(attribute_wrapper), NULL);
	
	cong_attribute_wrapper_construct (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper),
					  doc,
					  node,
					  ns_ptr,
					  attribute_name,
					  attr);


	PRIVATE(attribute_wrapper)->radio_button = radio_button;
	PRIVATE(attribute_wrapper)->attribute_value = g_strdup (attribute_value);	

	PRIVATE(attribute_wrapper)->handler_id_toggled = g_signal_connect (G_OBJECT (radio_button),
									   "toggled",
									   G_CALLBACK (on_toggled),
									   attribute_wrapper);	
	do_refresh (attribute_wrapper);
	
	return attribute_wrapper;
}

CongAttributeWrapperRadioButton*
cong_attribute_wrapper_radio_button_new ( CongDocument *doc,
					  CongNodePtr node,
					  xmlNs *ns_ptr,
					  const gchar *attribute_name,
					  xmlAttributePtr attr,
					  GtkRadioButton *radio_button,
					  const gchar *attribute_value)
{
	return cong_attribute_wrapper_radio_button_construct (g_object_new (CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON_TYPE, NULL),
							      doc,
							      node,
							      ns_ptr,
							      attribute_name,
							      attr,
							      radio_button,
							      attribute_value);

}

/* Internal function definitions: */
static void
set_attribute_handler (CongAttributeWrapper *attribute_wrapper)
{
	do_refresh (CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON(attribute_wrapper));
}

static void
remove_attribute_handler (CongAttributeWrapper *attribute_wrapper)
{
	do_refresh (CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON(attribute_wrapper));
}

static void
finalize (GObject *object)
{
#if 0
	CongAttributeWrapperRadioButton *attribute_wrapper_radio_button = CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON(object);
	
	g_free (attribute_wrapper->private);
	attribute_wrapper->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
#endif
}

static void
dispose (GObject *object)
{
#if 0
	CongAttributeWrapperRadioButton *attribute_wrapper = CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON(object);

	if (PRIVATE(attribute_wrapper)->doc) {
	
		g_signal_handler_disconnect (G_OBJECT (PRIVATE(attribute_wrapper)->doc),
					     PRIVATE(attribute_wrapper)->handler_id_node_set_attribute);
		g_signal_handler_disconnect (G_OBJECT (PRIVATE(attribute_wrapper)->doc),
					     PRIVATE(attribute_wrapper)->handler_id_node_remove_attribute);
		
		g_object_unref (G_OBJECT (PRIVATE(attribute_wrapper)->doc));
		PRIVATE(attribute_wrapper)->doc = NULL;
		
		g_free (PRIVATE(attribute_wrapper)->attribute_name);
	}
		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
#endif
}

static void
on_toggled (GtkToggleButton *toggle_button,
	    CongAttributeWrapperRadioButton *attribute_wrapper)
{
	if (gtk_toggle_button_get_active (toggle_button)) {
		cong_attribute_wrapper_set_value (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper),
						  PRIVATE(attribute_wrapper)->attribute_value);
	}
}

static void
do_refresh (CongAttributeWrapperRadioButton *attribute_wrapper)
{
	gboolean should_be_active = should_button_be_active (attribute_wrapper);
		
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (PRIVATE(attribute_wrapper)->radio_button))!=should_be_active) {
		g_signal_handler_block ( G_OBJECT(PRIVATE(attribute_wrapper)->radio_button),
					 PRIVATE(attribute_wrapper)->handler_id_toggled);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (PRIVATE(attribute_wrapper)->radio_button),
					      should_be_active);
		g_signal_handler_unblock ( G_OBJECT(PRIVATE(attribute_wrapper)->radio_button),
					   PRIVATE(attribute_wrapper)->handler_id_toggled);
		
	}
}

static gboolean 
should_button_be_active (CongAttributeWrapperRadioButton *attribute_wrapper)
{
	gchar *attribute_value = cong_attribute_wrapper_get_attribute_value (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper));

	if (attribute_value) {

		if (0==strcmp(attribute_value, 
			      PRIVATE(attribute_wrapper)->attribute_value)) {
			
			g_free (attribute_value);
			return TRUE;			
		} else {
			g_free (attribute_value);
			return FALSE;
		}
	} else {
		return (0==strcmp("", PRIVATE(attribute_wrapper)->attribute_value));
	}
}
