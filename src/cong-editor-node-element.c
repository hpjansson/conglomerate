/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node_element.c
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
#include "cong-editor-node-element.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"
#include "cong-document.h"

#undef PRIVATE
#define PRIVATE(x) ((x)->private)

struct CongEditorNodeElementDetails
{
	int dummy;
};

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeElement, 
			cong_editor_node_element,
			CongEditorNode,
			CONG_EDITOR_NODE_TYPE );

static void
cong_editor_node_element_class_init (CongEditorNodeElementClass *klass)
{
}

static void
cong_editor_node_element_instance_init (CongEditorNodeElement *node_element)
{
	node_element->private = g_new0(CongEditorNodeElementDetails,1);
}

/**
 * cong_editor_node_element_construct:
 * @editor_node_element:
 * @widget:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNodeElement*
cong_editor_node_element_construct (CongEditorNodeElement *editor_node_element,
				    CongEditorWidget3* editor_widget,
				    CongTraversalNode *traversal_node)
{
	cong_editor_node_construct (CONG_EDITOR_NODE (editor_node_element),
				    editor_widget,
				    traversal_node);

	return editor_node_element;
}

/**
 * cong_editor_node_element_get_dispspec_element:
 * @editor_node_element:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspecElement*
cong_editor_node_element_get_dispspec_element (CongEditorNodeElement *editor_node_element)
{
	CongDocument *doc;
	CongDispspecElement *ds_element;

	doc = cong_editor_node_get_document (CONG_EDITOR_NODE(editor_node_element));
	ds_element = cong_document_get_dispspec_element_for_node (doc, 
								  cong_editor_node_get_node( CONG_EDITOR_NODE(editor_node_element)));

	return ds_element;
}
