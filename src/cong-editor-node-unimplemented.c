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

#define PRIVATE(x) ((x)->private)

struct CongEditorNodeUnimplementedDetails
{
	gchar *description;
};

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeUnimplemented, 
			cong_editor_node_unimplemented,
			CongEditorNode,
			CONG_EDITOR_NODE_TYPE );

static void
cong_editor_node_unimplemented_class_init (CongEditorNodeUnimplementedClass *klass)
{
}

static void
cong_editor_node_unimplemented_instance_init (CongEditorNodeUnimplemented *node_unimplemented)
{
	node_unimplemented->private = g_new0(CongEditorNodeUnimplementedDetails,1);
}

CongEditorNodeUnimplemented*
cong_editor_node_unimplemented_construct (CongEditorNodeUnimplemented *editor_node_unimplemented,
					  CongEditorWidget3* editor_widget,
					  CongNodePtr node,
					  const gchar *description)
{
	g_message("cong_editor_node_unimplemented_construct(%s)", description);

	cong_editor_node_construct (CONG_EDITOR_NODE (editor_node_unimplemented),
				    editor_widget,
				    node);	

	PRIVATE(editor_node_unimplemented)->description = g_strdup(description);

	g_message("finished:cong_editor_node_unimplemented_construct(%s)", description);

	return editor_node_unimplemented;
}

CongEditorNode*
cong_editor_node_unimplemented_new (CongEditorWidget3 *widget,
				    CongNodePtr node,
				    const gchar *description)
{
	g_message("cong_editor_node_unimplemented_new(%s)", description);

	return CONG_EDITOR_NODE( cong_editor_node_unimplemented_construct (g_object_new (CONG_EDITOR_NODE_UNIMPLEMENTED_TYPE, NULL),
									   widget,
									   node,
									   description)
				 );
}
