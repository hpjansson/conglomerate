/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-attribute-wrapper_radio_button.h
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

#ifndef __CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON_H__
#define __CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON_H__

#include "cong-attribute-wrapper.h"

G_BEGIN_DECLS

typedef struct CongAttributeWrapperRadioButton CongAttributeWrapperRadioButton;
typedef struct CongAttributeWrapperRadioButtonClass CongAttributeWrapperRadioButtonClass;
typedef struct CongAttributeWrapperRadioButtonDetails CongAttributeWrapperRadioButtonDetails;

#define CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON_TYPE	      (cong_attribute_wrapper_radio_button_get_type ())
#define CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON_TYPE, CongAttributeWrapperRadioButton)
#define CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON_TYPE, CongAttributeWrapperRadioButtonClass)
#define IS_CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_ATTRIBUTE_WRAPPER_RADIO_BUTTON_TYPE)

struct CongAttributeWrapperRadioButton
{
	CongAttributeWrapper attribute_wrapper;

	CongAttributeWrapperRadioButtonDetails *private;
};

struct CongAttributeWrapperRadioButtonClass
{
	CongAttributeWrapperClass klass;
};

GType
cong_attribute_wrapper_radio_button_get_type (void);

/* it's legitimate for attr to be NULL */
CongAttributeWrapperRadioButton*
cong_attribute_wrapper_radio_button_construct (CongAttributeWrapperRadioButton *attribute_wrapper_radio_button,
					       CongDocument *doc,
					       CongNodePtr node,
					       xmlNs *namespace,
					       const gchar *attribute_name,
					       xmlAttributePtr attr,
					       GtkRadioButton *radio_button,
					       const gchar *attribute_value);

CongAttributeWrapperRadioButton*
cong_attribute_wrapper_radio_button_new ( CongDocument *doc,
					  CongNodePtr node,
					  xmlNs *namespace,
					  const gchar *attribute_name,
					  xmlAttributePtr attr,
					  GtkRadioButton *radio_button,
					  const gchar *attribute_value);

G_END_DECLS

#endif
