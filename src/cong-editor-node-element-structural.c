/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node-element-structural.c
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
#include "cong-editor-node-element-structural.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "cong-editor-area-border.h"
#include "cong-editor-area-structural-tag.h"
#include "cong-dispspec-element.h"

#undef PRIVATE
#define PRIVATE(x) ((x)->private)

struct CongEditorNodeElementStructuralDetails
{
	int dummy;
};

CONG_EDITOR_NODE_DECLARE_HOOKS

/* Declarations of the CongEditorArea event handlers: */

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeElementStructural, 
			cong_editor_node_element_structural,
			CongEditorNodeElement,
			CONG_EDITOR_NODE_ELEMENT_TYPE );

static void
cong_editor_node_element_structural_class_init (CongEditorNodeElementStructuralClass *klass)
{
	CONG_EDITOR_NODE_CONNECT_HOOKS
}

static void
cong_editor_node_element_structural_instance_init (CongEditorNodeElementStructural *node_element_structural)
{
	node_element_structural->private = g_new0(CongEditorNodeElementStructuralDetails,1);
}

/**
 * cong_editor_node_element_structural_construct:
 * @editor_node_element_structural:
 * @widget:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNodeElementStructural*
cong_editor_node_element_structural_construct (CongEditorNodeElementStructural *editor_node_element_structural,
					       CongEditorWidget3* editor_widget,
					       CongTraversalNode *traversal_node)
{
	cong_editor_node_element_construct (CONG_EDITOR_NODE_ELEMENT (editor_node_element_structural),
					    editor_widget,
					    traversal_node);

	return editor_node_element_structural;
}

/**
 * cong_editor_node_element_structural_new:
 * @widget:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_node_element_structural_new (CongEditorWidget3* widget,
					 CongTraversalNode *traversal_node)
{
#if DEBUG_EDITOR_NODE_LIFETIMES
	g_message("cong_editor_node_element_structural_new(%s)", node->name);
#endif

	return CONG_EDITOR_NODE( cong_editor_node_element_structural_construct
				 (g_object_new (CONG_EDITOR_NODE_ELEMENT_STRUCTURAL_TYPE, NULL),
				  widget,
				  traversal_node));
}

#if 1
static void 
create_areas (CongEditorNode *editor_node,
	      const CongAreaCreationInfo *creation_info)
{
	CongEditorArea *block_area;
	CongDispspecElement *ds_element;
	GdkPixbuf *pixbuf;
	gchar *title_text;

	g_return_if_fail (editor_node);

	ds_element = cong_editor_node_element_get_dispspec_element (CONG_EDITOR_NODE_ELEMENT (editor_node));
	pixbuf = cong_dispspec_element_get_icon (ds_element);
	title_text = cong_dispspec_element_get_section_header_text (ds_element,
								    cong_editor_node_get_node (editor_node));

	block_area = cong_editor_area_structural_tag_new (cong_editor_node_get_widget (editor_node),
							  ds_element,
							  pixbuf,
							  title_text);
	if (pixbuf) {
		g_object_unref (G_OBJECT(pixbuf));
	}
	g_free (title_text);

	cong_editor_area_connect_node_signals (block_area,
					       editor_node);

	cong_editor_node_create_block_area (editor_node,
					    creation_info,
					    block_area,
					    TRUE);
}

CONG_EDITOR_NODE_DEFINE_BLOCK_AREA_REGENERATION_HOOK

#else
static CongEditorArea*
generate_block_area (CongEditorNode *editor_node)
{
#if 0
	CongEditorArea *outer_area;
#endif
	CongEditorArea *inner_area;
	CongDispspecElement *ds_element;
	GdkPixbuf *pixbuf;
	gchar *title_text;

	g_return_val_if_fail (editor_node, NULL);

	ds_element = cong_editor_node_element_get_dispspec_element (CONG_EDITOR_NODE_ELEMENT(editor_node));

	pixbuf = cong_dispspec_element_get_icon (ds_element);

	title_text = cong_dispspec_element_get_section_header_text (ds_element,
								    cong_editor_node_get_node (editor_node));


#if 0
	outer_area = cong_editor_area_border_new (cong_editor_node_get_widget (editor_node),
						  5);
#endif

	inner_area = cong_editor_area_structural_tag_new (cong_editor_node_get_widget (editor_node),
							  ds_element,
							  pixbuf,
							  title_text);

	if (pixbuf) {
		g_object_unref (G_OBJECT(pixbuf));
	}

	g_free (title_text);


	cong_editor_area_connect_node_signals (inner_area,
					       editor_node);

#if 0
	cong_editor_area_container_add_child (parent_area,
					      outer_area);
	cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER(outer_area),
					      inner_area);
#endif

	return inner_area;
}
#endif

/* Definitions of the CongEditorArea event handlers: */
