/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node-entity-ref.c
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
#include "cong-editor-node-entity-ref.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "cong-editor-area-entity-ref.h"
#include "cong-editor-area-structural.h"

#undef PRIVATE
#define PRIVATE(x) ((x)->private)

struct CongEditorNodeEntityRefDetails
{
	int dummy;
};

CONG_EDITOR_NODE_DECLARE_HOOKS

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeEntityRef, 
			cong_editor_node_entity_ref,
			CongEditorNode,
			CONG_EDITOR_NODE_TYPE );

static void
cong_editor_node_entity_ref_class_init (CongEditorNodeEntityRefClass *klass)
{
	CONG_EDITOR_NODE_CONNECT_HOOKS
}

static void
cong_editor_node_entity_ref_instance_init (CongEditorNodeEntityRef *node_entity_ref)
{
	node_entity_ref->private = g_new0(CongEditorNodeEntityRefDetails,1);
}

/**
 * cong_editor_node_entity_ref_construct:
 * @editor_node_entity_ref:
 * @widget:
 * @traversal_node:
 * 
 * TODO: Write me
 * Returns:
 */
CongEditorNodeEntityRef*
cong_editor_node_entity_ref_construct (CongEditorNodeEntityRef *editor_node_entity_ref,
				       CongEditorWidget3 *editor_widget,
				       CongTraversalNode *traversal_node)
{
	cong_editor_node_construct (CONG_EDITOR_NODE (editor_node_entity_ref),
				    editor_widget,
				    traversal_node);	
	return editor_node_entity_ref;
}

/**
 * cong_editor_node_entity_ref_new:
 * @widget:
 * @traversal_node:
 * 
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_node_entity_ref_new (CongEditorWidget3 *widget,
				 CongTraversalNode *traversal_node)
{
#if DEBUG_EDITOR_NODE_LIFETIMES
	g_message("cong_editor_node_entity_ref_new()");
#endif
	return CONG_EDITOR_NODE( cong_editor_node_entity_ref_construct (g_object_new (CONG_EDITOR_NODE_ENTITY_REF_TYPE, NULL),
									 widget,
									 traversal_node)
				 );
}

#if 1
static void 
create_areas (CongEditorNode *editor_node,
	      const CongAreaCreationInfo *creation_info)
{
	CongEditorArea *block_area;
	gchar *title;
	GdkColor col_bold;
	GdkColor col_dim;
	GdkColor col_background;
	GdkColor col_text;

	g_return_if_fail (editor_node);

#if 1
	title = g_strdup_printf ("Entity reference: \"%s\"", 
				 cong_editor_node_get_node (editor_node)->name);

	cong_eel_rgb_to_gdk_color (&col_bold,
				   0x00,
				   0x00,
				   0x00);
	cong_eel_rgb_to_gdk_color (&col_dim,
				   0x00,
				   0x00,
				   0x00);
	cong_eel_rgb_to_gdk_color (&col_background,
				   0xff,
				   0xff,
				   0xff);
	cong_eel_rgb_to_gdk_color (&col_text,
				   0x00,
				   0x00,
				   0x00);

	block_area = cong_editor_area_structural_new (cong_editor_node_get_widget (editor_node),
						      NULL,
						      title,
						      &col_bold,
						      &col_dim,
						      &col_background,
						      &col_text);

	g_free (title);
#else
	block_area = cong_editor_area_entity_ref_new (cong_editor_node_get_widget (editor_node),
						      cong_editor_node_get_node (editor_node)->name);
#endif

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
	CongEditorArea *new_area;
	gchar *title;
	GdkColor col_bold;
	GdkColor col_dim;
	GdkColor col_background;
	GdkColor col_text;

	g_return_val_if_fail (editor_node, NULL);

#if 1
	title = g_strdup_printf ("Entity reference: \"%s\"", 
				 cong_editor_node_get_node (editor_node)->name);

	cong_eel_rgb_to_gdk_color (&col_bold,
				   0x00,
				   0x00,
				   0x00);
	cong_eel_rgb_to_gdk_color (&col_dim,
				   0x00,
				   0x00,
				   0x00);
	cong_eel_rgb_to_gdk_color (&col_background,
				   0xff,
				   0xff,
				   0xff);
	cong_eel_rgb_to_gdk_color (&col_text,
				   0x00,
				   0x00,
				   0x00);

	new_area = cong_editor_area_structural_new (cong_editor_node_get_widget (editor_node),
						    NULL,
						    title,
						    &col_bold,
						    &col_dim,
						    &col_background,
						    &col_text);

	g_free (title);
#else
	new_area = cong_editor_area_entity_ref_new (cong_editor_node_get_widget (editor_node),
						    cong_editor_node_get_node (editor_node)->name);
#endif

	return new_area;
}
#endif
