/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node-dtd.c
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
#include "cong-editor-node-dtd.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "cong-editor-area-bin.h"

/* Exported function definitions: */
CONG_EDITOR_NODE_DEFINE_SUBCLASS(Dtd, 
				 dtd,
				 CONG_EDITOR_NODE_DTD,
				 int dummy;
				 )

/**
 * cong_editor_node_dtd_construct:
 * @editor_node_dtd:
 * @widget:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNodeDtd*
cong_editor_node_dtd_construct (CongEditorNodeDtd *editor_node_dtd,
				CongEditorWidget3* editor_widget,
				CongTraversalNode *traversal_node)
{
	cong_editor_node_construct (CONG_EDITOR_NODE (editor_node_dtd),
				    editor_widget,
				    traversal_node);	
	return editor_node_dtd;
}

CONG_EDITOR_NODE_IMPLEMENT_EMPTY_DISPOSE(dtd)

#if 1
static void 
create_areas (CongEditorNode *editor_node,
	      const CongAreaCreationInfo *creation_info)
{
	cong_editor_node_empty_create_area (editor_node,
					    creation_info,
					    TRUE);
}

CONG_EDITOR_NODE_DEFINE_EMPTY_AREA_REGENERATION_HOOK

#else
static CongEditorArea*
generate_block_area (CongEditorNode *editor_node)
{
	CongEditorArea *new_area;

	g_return_val_if_fail (editor_node, NULL);

	new_area = cong_editor_area_bin_new (cong_editor_node_get_widget (editor_node));

	return new_area;
}
#endif
