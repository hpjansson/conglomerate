/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-paragraph-node-element-paragraph.c
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
 * Fragments of code based upon libxslt: numbers.c
 */

#include "global.h"
#include "plugin-paragraph-node-element-paragraph.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "plugin-paragraph-area-paragraph.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorNodeElementParagraphDetails
{
	int dummy;
};

static CongEditorArea*
generate_block_area (CongEditorNode *editor_node);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeElementParagraph, 
			cong_editor_node_element_paragraph,
			CongEditorNodeElement,
			CONG_EDITOR_NODE_ELEMENT_TYPE );

static void
cong_editor_node_element_paragraph_class_init (CongEditorNodeElementParagraphClass *klass)
{
	CongEditorNodeClass *node_klass = CONG_EDITOR_NODE_CLASS(klass);

	node_klass->generate_block_area = generate_block_area;
}

static void
cong_editor_node_element_paragraph_instance_init (CongEditorNodeElementParagraph *node_element_paragraph)
{
	node_element_paragraph->private = g_new0(CongEditorNodeElementParagraphDetails,1);
}

CongEditorNodeElementParagraph*
cong_editor_node_element_paragraph_construct (CongEditorNodeElementParagraph *editor_node_element_paragraph,
					      CongEditorWidget3 *editor_widget,
					      CongTraversalNode *traversal_node)
{
	cong_editor_node_element_construct (CONG_EDITOR_NODE_ELEMENT (editor_node_element_paragraph),
					    editor_widget,
					    traversal_node);

	return editor_node_element_paragraph;
}

CongEditorNode*
cong_editor_node_element_paragraph_new (CongEditorWidget3* widget,
					CongTraversalNode *traversal_node)
{
#if DEBUG_EDITOR_NODE_LIFETIMES
	g_message("cong_editor_node_element_paragraph_new(%s)", node->name);
#endif

	return CONG_EDITOR_NODE( cong_editor_node_element_paragraph_construct
				 (g_object_new (CONG_EDITOR_NODE_ELEMENT_PARAGRAPH_TYPE, NULL),
				  widget,
				  traversal_node));
}

static CongEditorArea*
generate_block_area (CongEditorNode *editor_node)
{
	CongEditorArea *new_area;

	g_return_val_if_fail (editor_node, NULL);

	new_area = cong_editor_area_paragraph_new (cong_editor_node_get_widget (editor_node));

	cong_editor_area_connect_node_signals (new_area,
					       editor_node);

	return new_area;
}
