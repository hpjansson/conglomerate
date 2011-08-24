/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-attribute-wrapper-check-button.c
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
#include "cong-attribute-wrapper-check-button.h"
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

/* Internal types: */
struct _CongAttributeWrapperCheckButtonDetails
{
	GtkCheckButton *check_button;
	gchar *attribute_value_unchecked;
	gchar *attribute_value_checked;

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
	    CongAttributeWrapperCheckButton *attribute_wrapper);

static void
do_refresh (CongAttributeWrapperCheckButton *attribute_wrapper_check_button);

static gboolean 
should_button_be_active (CongAttributeWrapperCheckButton *attribute_wrapper);

/* Exported function definitions: */
G_DEFINE_TYPE(CongAttributeWrapperCheckButton,
              cong_attribute_wrapper_check_button,
              CONG_ATTRIBUTE_WRAPPER_TYPE);

static void
cong_attribute_wrapper_check_button_class_init (CongAttributeWrapperCheckButtonClass *klass)
{
	CongAttributeWrapperClass *wrapper_klass = CONG_ATTRIBUTE_WRAPPER_CLASS (klass);

	wrapper_klass->set_attribute_handler = set_attribute_handler;
	wrapper_klass->remove_attribute_handler = remove_attribute_handler;

	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;
}

static void
cong_attribute_wrapper_check_button_init (CongAttributeWrapperCheckButton *attribute_wrapper)
{
	attribute_wrapper->private = g_new0(CongAttributeWrapperCheckButtonDetails,1);
}

/**
 * cong_attribute_wrapper_check_button_construct:
 * @attribute_wrapper:
 * @doc:
 * @node:
 * @ns_ptr:
 * @attribute_name:
 * @check_button:
 * @attribute_value_unchecked:
 * @attribute_value_checked:
 *
 * TODO: Write me
 * Returns:
 */
CongAttributeWrapperCheckButton*
cong_attribute_wrapper_check_button_construct (CongAttributeWrapperCheckButton *attribute_wrapper,
					       CongDocument *doc,
					       CongNodePtr node,
					       xmlNs *ns_ptr,
					       const gchar *attribute_name,
					       GtkCheckButton *check_button,
					       const gchar *attribute_value_unchecked,
					       const gchar *attribute_value_checked)
{
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_WRAPPER_CHECK_BUTTON(attribute_wrapper), NULL);
	
	cong_attribute_wrapper_construct (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper),
					  doc,
					  node,
					  ns_ptr,
					  attribute_name);


	PRIVATE(attribute_wrapper)->check_button = check_button;
	PRIVATE(attribute_wrapper)->attribute_value_unchecked = g_strdup (attribute_value_unchecked);	
	PRIVATE(attribute_wrapper)->attribute_value_checked = g_strdup (attribute_value_checked);	

	PRIVATE(attribute_wrapper)->handler_id_toggled = g_signal_connect (G_OBJECT (check_button),
									   "toggled",
									   G_CALLBACK (on_toggled),
									   attribute_wrapper);

	do_refresh (attribute_wrapper);
	
	return attribute_wrapper;
}

/**
 * cong_attribute_wrapper_check_button_new:
 * @doc:
 * @node:
 * @ns_ptr:
 * @attribute_name:
 * @check_button:
 * @attribute_value_unchecked:
 * @attribute_value_checked:
 *
 * TODO: Write me
 * Returns:
 */
CongAttributeWrapperCheckButton*
cong_attribute_wrapper_check_button_new ( CongDocument *doc,
					  CongNodePtr node,
					  xmlNs *ns_ptr,
					  const gchar *attribute_name,
					  GtkCheckButton *check_button,
					  const gchar *attribute_value_unchecked,
					  const gchar *attribute_value_checked)
{
	return cong_attribute_wrapper_check_button_construct (g_object_new (CONG_ATTRIBUTE_WRAPPER_CHECK_BUTTON_TYPE, NULL),
							      doc,
							      node,
							      ns_ptr,
							      attribute_name,
							      check_button,
							      attribute_value_unchecked,
							      attribute_value_checked);

}

/* Internal function definitions: */
static void
set_attribute_handler (CongAttributeWrapper *attribute_wrapper)
{
	do_refresh (CONG_ATTRIBUTE_WRAPPER_CHECK_BUTTON(attribute_wrapper));
}

static void
remove_attribute_handler (CongAttributeWrapper *attribute_wrapper)
{
	do_refresh (CONG_ATTRIBUTE_WRAPPER_CHECK_BUTTON(attribute_wrapper));
}

static void
finalize (GObject *object)
{
	CongAttributeWrapperCheckButton *attribute_wrapper_check_button = CONG_ATTRIBUTE_WRAPPER_CHECK_BUTTON(object);
	
	g_free (attribute_wrapper_check_button->private);
	attribute_wrapper_check_button->private = NULL;
	
	G_OBJECT_CLASS (cong_attribute_wrapper_check_button_parent_class)->finalize (object);
}

static void
dispose (GObject *object)
{
	CongAttributeWrapperCheckButton *attribute_wrapper = CONG_ATTRIBUTE_WRAPPER_CHECK_BUTTON(object);

	if (PRIVATE(attribute_wrapper)->attribute_value_unchecked) {
		g_free (PRIVATE(attribute_wrapper)->attribute_value_unchecked);
		PRIVATE(attribute_wrapper)->attribute_value_unchecked = NULL;
	}
	if (PRIVATE(attribute_wrapper)->attribute_value_checked) {
		g_free (PRIVATE(attribute_wrapper)->attribute_value_checked);
		PRIVATE(attribute_wrapper)->attribute_value_checked = NULL;
	}
	
	G_OBJECT_CLASS (cong_attribute_wrapper_check_button_parent_class)->dispose (object);
}

static void
on_toggled (GtkToggleButton *toggle_button,
	    CongAttributeWrapperCheckButton *attribute_wrapper)
{
	if (gtk_toggle_button_get_active (toggle_button)) {
		cong_attribute_wrapper_set_value (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper),
						  PRIVATE(attribute_wrapper)->attribute_value_checked);
	} else {
		cong_attribute_wrapper_set_value (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper),
						  PRIVATE(attribute_wrapper)->attribute_value_unchecked);
	}
}

static void
do_refresh (CongAttributeWrapperCheckButton *attribute_wrapper)
{
	gboolean should_be_active = should_button_be_active (attribute_wrapper);
		
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (PRIVATE(attribute_wrapper)->check_button))!=should_be_active) {
		g_signal_handler_block ( G_OBJECT(PRIVATE(attribute_wrapper)->check_button),
					 PRIVATE(attribute_wrapper)->handler_id_toggled);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (PRIVATE(attribute_wrapper)->check_button),
					      should_be_active);
		g_signal_handler_unblock ( G_OBJECT(PRIVATE(attribute_wrapper)->check_button),
					   PRIVATE(attribute_wrapper)->handler_id_toggled);
	}
}

static gboolean 
should_button_be_active (CongAttributeWrapperCheckButton *attribute_wrapper)
{
	gchar *attribute_value = cong_attribute_wrapper_get_attribute_value (CONG_ATTRIBUTE_WRAPPER(attribute_wrapper));
	gboolean retval = FALSE;

	if (attribute_value) {
		if (0==strcmp(attribute_value, 
			      PRIVATE(attribute_wrapper)->attribute_value_checked)) {
			retval = TRUE;
		}
		g_free (attribute_value);
	}
	return retval;
}
