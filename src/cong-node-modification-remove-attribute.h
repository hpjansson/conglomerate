/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-node-modification-remove-attribute.h
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

#ifndef __CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE_H__
#define __CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE_H__

#include "cong-node-modification.h"

G_BEGIN_DECLS

#define CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE_TYPE	  (cong_node_modification_remove_attribute_get_type ())
#define CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE_TYPE, CongNodeModificationRemoveAttribute)
#define CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE_TYPE, CongNodeModificationRemoveAttributeClass)
#define IS_CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_NODE_MODIFICATION_REMOVE_ATTRIBUTE_TYPE)

typedef struct CongNodeModificationRemoveAttribute CongNodeModificationRemoveAttribute;
typedef struct CongNodeModificationRemoveAttributeClass CongNodeModificationRemoveAttributeClass;
typedef struct CongNodeModificationRemoveAttributeDetails CongNodeModificationRemoveAttributeDetails;


struct CongNodeModificationRemoveAttribute
{
	CongNodeModification base;

	CongNodeModificationRemoveAttributeDetails *private;
};

struct CongNodeModificationRemoveAttributeClass
{
	CongNodeModificationClass klass;

	/* Methods? */
};

GType
cong_node_modification_remove_attribute_get_type (void);

CongNodeModificationRemoveAttribute*
cong_node_modification_remove_attribute_construct (CongNodeModificationRemoveAttribute *node_modification_remove_attribute,
						   CongDocument *doc,
						   CongNodePtr node,
						   xmlNs *ns_ptr,
						   const gchar *name);

CongModification*
cong_node_modification_remove_attribute_new (CongDocument *doc,
					     CongNodePtr node,
					     xmlNs *ns_ptr,
					     const gchar *name);

G_END_DECLS

#endif
