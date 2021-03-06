/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node-entity-decl.c
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
#include "cong-editor-node-entity-decl.h"
#include "cong-eel.h"

#include "cong-editor-area-entity-decl.h"
#include "cong-editor-area-structural-tag.h"

#undef PRIVATE
#define PRIVATE(x) ((x)->private)

struct _CongEditorNodeEntityDeclDetails
{
	int dummy;
};

CONG_EDITOR_NODE_DECLARE_HOOKS(entity_decl)

/* Exported function definitions: */
G_DEFINE_TYPE(CongEditorNodeEntityDecl,
              cong_editor_node_entity_decl,
              CONG_EDITOR_NODE_TYPE );

static void
cong_editor_node_entity_decl_class_init (CongEditorNodeEntityDeclClass *klass)
{
	CONG_EDITOR_NODE_CONNECT_HOOKS(entity_decl)
}

static void
cong_editor_node_entity_decl_init (CongEditorNodeEntityDecl *node_entity_decl)
{
	node_entity_decl->private = g_new0(CongEditorNodeEntityDeclDetails,1);
}

/**
 * cong_editor_node_entity_decl_construct:
 * @editor_node_entity_decl:
 * @widget:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNodeEntityDecl*
cong_editor_node_entity_decl_construct (CongEditorNodeEntityDecl *editor_node_entity_decl,
					CongEditorWidget3* editor_widget,
					CongTraversalNode *traversal_node)
{
	cong_editor_node_construct (CONG_EDITOR_NODE (editor_node_entity_decl),
				    editor_widget,
				    traversal_node);	
	return editor_node_entity_decl;
}

/**
 * cong_editor_node_entity_decl_new:
 * @widget:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_node_entity_decl_new (CongEditorWidget3 *widget,
				  CongTraversalNode *traversal_node)
{
#if DEBUG_EDITOR_NODE_LIFETIMES
	g_message("cong_editor_node_entity_decl_new()");
#endif
	return CONG_EDITOR_NODE( cong_editor_node_entity_decl_construct (g_object_new (CONG_EDITOR_NODE_ENTITY_DECL_TYPE, NULL),
									 widget,
									 traversal_node)
				 );
}

#if 1
static GdkColor* 
theme_cb (CongEditorArea *area,
	  CongDispspecGCUsage usage)
{
	switch (usage) {
	default: g_assert_not_reached ();
	case CONG_DISPSPEC_GC_USAGE_BOLD_LINE:
		return cong_eel_rgb_to_gdk_color2 (0x00,
						   0x00,
						   0x00);
	case CONG_DISPSPEC_GC_USAGE_DIM_LINE:
		return cong_eel_rgb_to_gdk_color2 (0x00,
						   0x00,
						   0x00);
	case CONG_DISPSPEC_GC_USAGE_BACKGROUND:
		return cong_eel_rgb_to_gdk_color2 (0xff,
						   0xff,
						   0xff);
	case CONG_DISPSPEC_GC_USAGE_TEXT:
		return cong_eel_rgb_to_gdk_color2 (0x00,
						   0x00,
						   0x00);
	}
}

static void 
entity_decl_create_areas (CongEditorNode *editor_node,
			  const CongAreaCreationInfo *creation_info)
{
	CongEditorArea *block_area;
	gchar *title;

	g_return_if_fail (editor_node);

#if 1
	if (1) {
		title = g_strdup_printf ("Entity declaration: \"%s\"", 
					 cong_editor_node_get_node (editor_node)->name);
		
		block_area = cong_editor_area_structural_new (cong_editor_node_get_widget (editor_node),
							      NULL,
							      title,
							      theme_cb);
		
		g_free (title);
	} else {
	}
#else
	block_area = cong_editor_area_entity_decl_new (cong_editor_node_get_widget (editor_node),
						       cong_editor_node_get_node (editor_node)->name);
#endif

	cong_editor_area_connect_node_signals (block_area,
					       editor_node);

	cong_editor_node_create_block_area (editor_node,
					    creation_info,
					    block_area,
					    TRUE);
}

CONG_EDITOR_NODE_DEFINE_BLOCK_AREA_REGENERATION_HOOK(entity_decl)

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
	title = g_strdup_printf ("Entity declaration: \"%s\"", 
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
	new_area = cong_editor_area_entity_decl_new (cong_editor_node_get_widget (editor_node),
						     cong_editor_node_get_node (editor_node)->name);
#endif

	return new_area;
}
#endif
