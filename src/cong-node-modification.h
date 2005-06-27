/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-node-modification.h
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

#ifndef __CONG_NODE_MODIFICATION_H__
#define __CONG_NODE_MODIFICATION_H__

#include "cong-modification.h"

G_BEGIN_DECLS

#define CONG_NODE_MODIFICATION_TYPE	  (cong_node_modification_get_type ())
#define CONG_NODE_MODIFICATION(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_NODE_MODIFICATION_TYPE, CongNodeModification)
#define CONG_NODE_MODIFICATION_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_NODE_MODIFICATION_TYPE, CongNodeModificationClass)
#define IS_CONG_NODE_MODIFICATION(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_NODE_MODIFICATION_TYPE)

typedef struct _CongNodeModification CongNodeModification;
typedef struct _CongNodeModificationClass CongNodeModificationClass;
typedef struct _CongNodeModificationDetails CongNodeModificationDetails;

struct _CongNodeModification
{
	CongModification base;

	CongNodeModificationDetails *private;
};

struct _CongNodeModificationClass
{
	CongModificationClass klass;

	/* Methods? */
};

GType
cong_node_modification_get_type (void);

CongNodeModification*
cong_node_modification_construct (CongNodeModification *node_modification,
				  CongDocument *doc,
				  CongNodePtr node);

CongNodePtr 
cong_node_modification_get_node (CongNodeModification *node_modification);

G_END_DECLS

#endif
