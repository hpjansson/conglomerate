/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node-unimplemented.c
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
#include "cong-editor-node-unimplemented.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "cong-app.h"
#include "cong-editor-area-text.h"

#undef PRIVATE
#define PRIVATE(x) ((x)->private)

struct CongEditorNodeUnimplementedDetails
{
	gchar *description;
};

CONG_EDITOR_NODE_DECLARE_HOOKS

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeUnimplemented, 
			cong_editor_node_unimplemented,
			CongEditorNode,
			CONG_EDITOR_NODE_TYPE );

static void
cong_editor_node_unimplemented_class_init (CongEditorNodeUnimplementedClass *klass)
{
	CONG_EDITOR_NODE_CONNECT_HOOKS
}

static void
cong_editor_node_unimplemented_instance_init (CongEditorNodeUnimplemented *node_unimplemented)
{
	node_unimplemented->private = g_new0(CongEditorNodeUnimplementedDetails,1);
}

/**
 * cong_editor_node_unimplemented_construct:
 * @editor_node_unimplemented:
 * @widget:
 * @traversal_node:
 * @description:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNodeUnimplemented*
cong_editor_node_unimplemented_construct (CongEditorNodeUnimplemented *editor_node_unimplemented,
					  CongEditorWidget3* editor_widget,
					  CongTraversalNode *traversal_node,
					  const gchar *description)
{
	cong_editor_node_construct (CONG_EDITOR_NODE (editor_node_unimplemented),
				    editor_widget,
				    traversal_node);

	PRIVATE(editor_node_unimplemented)->description = g_strdup(description);

	return editor_node_unimplemented;
}

/**
 * cong_editor_node_unimplemented_new:
 * @widget:
 * @traversal_node:
 * @description:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_node_unimplemented_new (CongEditorWidget3 *widget,
				    CongTraversalNode *traversal_node,
				    const gchar *description)
{
#if DEBUG_EDITOR_NODE_LIFETIMES
	g_message("cong_editor_node_unimplemented_new(%s)", description);
#endif
	return CONG_EDITOR_NODE( cong_editor_node_unimplemented_construct (g_object_new (CONG_EDITOR_NODE_UNIMPLEMENTED_TYPE, NULL),
									   widget,
									   traversal_node,
									   description)
				 );
}


#if 1
static void 
create_areas (CongEditorNode *editor_node,
	      const CongAreaCreationInfo *creation_info)
{
	CongEditorNodeUnimplemented *editor_node_unimplemented = CONG_EDITOR_NODE_UNIMPLEMENTED(editor_node);
	CongEditorArea *block_area;
	
	block_area = cong_editor_area_text_new (cong_editor_node_get_widget (editor_node),
						cong_app_get_font (cong_app_singleton(),
								   CONG_FONT_ROLE_TITLE_TEXT),
						NULL,
						PRIVATE(editor_node_unimplemented)->description,
						FALSE);

	cong_editor_node_create_block_area (editor_node,
					    creation_info,
					    block_area,
					    FALSE);
}
gboolean
needs_area_regeneration (CongEditorNode *editor_node,
			 const CongAreaCreationGeometry *old_creation_geometry,
			 const CongAreaCreationGeometry *new_creation_geometry)
{
	/* Block areas don't need regenerating: */
	return FALSE;
}
#else
static CongEditorArea*
generate_block_area (CongEditorNode *editor_node)
{
	CongEditorNodeUnimplemented *editor_node_unimplemented = CONG_EDITOR_NODE_UNIMPLEMENTED(editor_node);
	CongEditorArea *new_area;

	new_area = cong_editor_area_text_new (cong_editor_node_get_widget (editor_node),
					      cong_app_get_font (cong_app_singleton(),
								 CONG_FONT_ROLE_TITLE_TEXT),
					      NULL,
					      PRIVATE(editor_node_unimplemented)->description,
					      FALSE);

	return new_area;
}
#endif
