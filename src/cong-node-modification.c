/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-node-modification.c
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
#include "cong-node-modification.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongNodeModificationDetails
{
	CongNodePtr node;
};

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongNodeModification, 
			cong_node_modification,
			CongModification,
			CONG_MODIFICATION_TYPE );

static void
cong_node_modification_class_init (CongNodeModificationClass *klass)
{
}

static void
cong_node_modification_instance_init (CongNodeModification *node)
{
	node->private = g_new0(CongNodeModificationDetails,1);
}

/**
 * cong_node_modification_construct:
 * @node_modification:
 * @doc:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongNodeModification*
cong_node_modification_construct (CongNodeModification *node_modification,
				  CongDocument *doc,
				  CongNodePtr node)
{
	cong_modification_construct (CONG_MODIFICATION(node_modification),
				     doc);

	PRIVATE(node_modification)->node = node;

	cong_document_node_ref (doc,
				node);

	return node_modification;
}

/**
 * cong_node_modification_get_node:
 * @node_modification:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr 
cong_node_modification_get_node (CongNodeModification *node_modification)
{
	g_return_val_if_fail (IS_CONG_NODE_MODIFICATION (node_modification), NULL);

	return PRIVATE(node_modification)->node;
}
