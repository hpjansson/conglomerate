/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node.c
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
#include "cong-editor-node.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorNodeDetails
{
	CongEditorWidget3 *widget;

	/* An editor node applies to a specific document node; there is a 1-1 mapping: */
	CongNodePtr node;

#if 0
	CongEditorArea *primary_area;
	CongEditorArea *inner_area;
#endif
};

CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_node, generate_area);


/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNode, 
			cong_editor_node,
			GObject,
			G_TYPE_OBJECT );

static void
cong_editor_node_class_init (CongEditorNodeClass *klass)
{
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_node,
					      generate_area);
}

static void
cong_editor_node_instance_init (CongEditorNode *node)
{
	node->private = g_new0(CongEditorNodeDetails,1);
}

CongEditorNode*
cong_editor_node_construct (CongEditorNode *editor_node,
			    CongEditorWidget3* editor_widget,
			    CongNodePtr node)
{
	PRIVATE(editor_node)->widget = editor_widget;
	PRIVATE(editor_node)->node = node;
}


CongEditorWidget3*
cong_editor_node_get_widget (CongEditorNode *editor_node)
{
	g_return_val_if_fail (editor_node, NULL);

	return PRIVATE(editor_node)->widget;
}

CongDocument*
cong_editor_node_get_document (CongEditorNode *editor_node)
{
	g_return_val_if_fail (editor_node, NULL);
	
	return cong_editor_widget3_get_document( cong_editor_node_get_widget(editor_node));
}

CongNodePtr
cong_editor_node_get_node (CongEditorNode *editor_node)
{
	g_return_val_if_fail (editor_node, NULL);

	return PRIVATE(editor_node)->node;
}

#if 0
CongEditorArea*
cong_editor_node_get_area (CongEditorNode *editor_node)
{
#error
}
#endif

CongEditorArea*
cong_editor_node_generate_area (CongEditorNode *editor_node)
{
	g_return_if_fail (editor_node);
	
	return CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_NODE_CLASS,
						       editor_node,
						       generate_area, 
						       (editor_node));
}
