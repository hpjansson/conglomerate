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
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "cong-editor-area-entity-decl.h"
#include "cong-editor-area-structural-tag.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorNodeEntityDeclDetails
{
	int dummy;
};

static CongEditorArea*
generate_block_area (CongEditorNode *editor_node);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeEntityDecl, 
			cong_editor_node_entity_decl,
			CongEditorNode,
			CONG_EDITOR_NODE_TYPE );

static void
cong_editor_node_entity_decl_class_init (CongEditorNodeEntityDeclClass *klass)
{

	CongEditorNodeClass *node_klass = CONG_EDITOR_NODE_CLASS(klass);

	node_klass->generate_block_area = generate_block_area;
}

static void
cong_editor_node_entity_decl_instance_init (CongEditorNodeEntityDecl *node_entity_decl)
{
	node_entity_decl->private = g_new0(CongEditorNodeEntityDeclDetails,1);
}

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
