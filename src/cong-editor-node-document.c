/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node-document.c
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
#include "cong-editor-node-document.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "cong-editor-area-bin.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorNodeDocumentDetails
{
	int dummy;
};

static CongEditorArea*
generate_block_area (CongEditorNode *editor_node);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeDocument, 
			cong_editor_node_document,
			CongEditorNode,
			CONG_EDITOR_NODE_TYPE );

static void
cong_editor_node_document_class_init (CongEditorNodeDocumentClass *klass)
{

	CongEditorNodeClass *node_klass = CONG_EDITOR_NODE_CLASS(klass);

	node_klass->generate_block_area = generate_block_area;
}

static void
cong_editor_node_document_instance_init (CongEditorNodeDocument *node_document)
{
	node_document->private = g_new0(CongEditorNodeDocumentDetails,1);
}

CongEditorNodeDocument*
cong_editor_node_document_construct (CongEditorNodeDocument *editor_node_document,
				     CongEditorWidget3* editor_widget,
				     CongTraversalNode *traversal_node)
{
	cong_editor_node_construct (CONG_EDITOR_NODE (editor_node_document),
				    editor_widget,
				    traversal_node);	
	return editor_node_document;
}

CongEditorNode*
cong_editor_node_document_new (CongEditorWidget3 *widget,
			       CongTraversalNode *traversal_node)
{
#if DEBUG_EDITOR_NODE_LIFETIMES
	g_message("cong_editor_node_document_new()");
#endif
	return CONG_EDITOR_NODE( cong_editor_node_document_construct (g_object_new (CONG_EDITOR_NODE_DOCUMENT_TYPE, NULL),
								      widget,
								      traversal_node)
				 );
}

static CongEditorArea*
generate_block_area (CongEditorNode *editor_node)
{
	CongEditorArea *new_area;

	g_return_val_if_fail (editor_node, NULL);

	new_area = cong_editor_area_bin_new (cong_editor_node_get_widget (editor_node));

	return new_area;
}
