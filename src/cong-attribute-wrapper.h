/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-attribute-wrapper.h
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

#ifndef __CONG_ATTRIBUTE_WRAPPER_H__
#define __CONG_ATTRIBUTE_WRAPPER_H__

#include "cong-document.h"

G_BEGIN_DECLS

typedef struct CongAttributeWrapper CongAttributeWrapper;
typedef struct CongAttributeWrapperClass CongAttributeWrapperClass;
typedef struct CongAttributeWrapperDetails CongAttributeWrapperDetails;

#define CONG_ATTRIBUTE_WRAPPER_TYPE	      (cong_attribute_wrapper_get_type ())
#define CONG_ATTRIBUTE_WRAPPER(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_ATTRIBUTE_WRAPPER_TYPE, CongAttributeWrapper)
#define CONG_ATTRIBUTE_WRAPPER_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_ATTRIBUTE_WRAPPER_TYPE, CongAttributeWrapperClass)
#define IS_CONG_ATTRIBUTE_WRAPPER(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_ATTRIBUTE_WRAPPER_TYPE)

struct CongAttributeWrapper
{
	GObject base_class;

	CongAttributeWrapperDetails *private;
};

struct CongAttributeWrapperClass
{
	GObjectClass klass;

	void (*set_attribute_handler) (CongAttributeWrapper *attribute_wrapper);
	void (*remove_attribute_handler) (CongAttributeWrapper *attribute_wrapper);
};

GType
cong_attribute_wrapper_get_type (void);

/* it's legitimate for attr to be NULL */
CongAttributeWrapper*
cong_attribute_wrapper_construct (CongAttributeWrapper *attribute_wrapper,
				  CongDocument *doc,
				  CongNodePtr node,
				  xmlNs *ns_ptr,
				  const gchar *attribute_name,
				  xmlAttributePtr attr);

CongDocument*
cong_attribute_wrapper_get_document (CongAttributeWrapper *attribute_wrapper);

CongNodePtr
cong_attribute_wrapper_get_node (CongAttributeWrapper *attribute_wrapper);

/* Result can be NULL */
xmlAttributePtr
cong_attribute_wrapper_get_attribute (CongAttributeWrapper *attribute_wrapper);

xmlNs *
cong_attribute_wrapper_get_ns (CongAttributeWrapper *attribute_wrapper);

const gchar*
cong_attribute_wrapper_get_attribute_name (CongAttributeWrapper *attribute_wrapper);

gchar*
cong_attribute_wrapper_get_attribute_value (CongAttributeWrapper *attribute_wrapper);

void
cong_attribute_wrapper_set_value (CongAttributeWrapper *attribute_wrapper,
				  const gchar *new_value);

void
cong_attribute_wrapper_remove_value (CongAttributeWrapper *attribute_wrapper);

void
cong_attribute_wrapper_bind_to_widget (CongAttributeWrapper* wrapper, GtkWidget *widget);

G_END_DECLS

#endif
