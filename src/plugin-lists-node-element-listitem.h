/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-lists-node-element-listitem.h
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

#ifndef __PLUGIN_LISTS_NODE_ELEMENT_LISTITEM_H__
#define __PLUGIN_LISTS_NODE_ELEMENT_LISTITEM_H__

#include "cong-editor-node-element.h"

G_BEGIN_DECLS

#define CONG_EDITOR_NODE_ELEMENT_LISTITEM_TYPE	      (cong_editor_node_element_listitem_get_type ())
#define CONG_EDITOR_NODE_ELEMENT_LISTITEM(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_NODE_ELEMENT_LISTITEM_TYPE, CongEditorNodeElementListitem)
#define CONG_EDITOR_NODE_ELEMENT_LISTITEM_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_NODE_ELEMENT_LISTITEM_TYPE, CongEditorNodeElementListitemClass)
#define IS_CONG_EDITOR_NODE_ELEMENT_LISTITEM(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_NODE_ELEMENT_LISTITEM_TYPE)

#if 1
CONG_EDITOR_NODE_DECLARE_PLUGIN_SUBCLASS(Listitem, listitem)
#else
typedef struct CongEditorNodeElementListitem CongEditorNodeElementListitem;
typedef struct CongEditorNodeElementListitemClass CongEditorNodeElementListitemClass;
typedef struct CongEditorNodeElementListitemDetails CongEditorNodeElementListitemDetails;

struct CongEditorNodeElementListitem
{
	CongEditorNodeElement editor_node_element;

	CongEditorNodeElementListitemDetails *private;
};

struct CongEditorNodeElementListitemClass
{
	CongEditorNodeElementClass klass;

	/* Methods? */
};

GType
cong_editor_node_element_listitem_get_type (void);

CongEditorNodeElementListitem*
cong_editor_node_element_listitem_construct (CongEditorNodeElementListitem *editor_node_element_listitem,
					     CongEditorWidget3 *widget,
					     CongTraversalNode *traversal_node);

CongEditorNode*
cong_editor_node_element_listitem_new (CongEditorWidget3 *widget,
				       CongTraversalNode *traversal_node);
#endif

const gchar*
cong_editor_node_element_listitem_get_label (CongEditorNodeElementListitem* listitem);


G_END_DECLS

#endif
