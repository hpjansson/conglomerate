/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node-text.c
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
#include "cong-editor-node-text.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "cong-app.h"
#include "cong-editor-area-text.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorNodeTextDetails
{
	int dummy;

#if 0
	CongEditorAreaComposer *flow_test;
	PangoLayout *pango_layout;
	GList *list_of_text_fragments;
#endif
};

static CongEditorArea*
generate_area (CongEditorNode *editor_node);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeText, 
			cong_editor_node_text,
			CongEditorNode,
			CONG_EDITOR_NODE_TYPE );

static void
cong_editor_node_text_class_init (CongEditorNodeTextClass *klass)
{
	CongEditorNodeClass *node_klass = CONG_EDITOR_NODE_CLASS(klass);

	node_klass->generate_area = generate_area;
}

static void
cong_editor_node_text_instance_init (CongEditorNodeText *node_text)
{
	node_text->private = g_new0(CongEditorNodeTextDetails,1);
}

CongEditorNodeText*
cong_editor_node_text_construct (CongEditorNodeText *editor_node_text,
				 CongEditorWidget3* editor_widget,
				 CongNodePtr node)
{
	cong_editor_node_construct (CONG_EDITOR_NODE (editor_node_text),
				    editor_widget,
				    node);

	return editor_node_text;
}

CongEditorNode*
cong_editor_node_text_new (CongEditorWidget3 *widget,
			   CongNodePtr node)
{
	g_message("cong_editor_node_text_new(%s)", node->content);

	return CONG_EDITOR_NODE( cong_editor_node_text_construct (g_object_new (CONG_EDITOR_NODE_TEXT_TYPE, NULL),
								  widget,
								  node)
				 );
}

static CongEditorArea*
generate_area (CongEditorNode *editor_node)
{
	CongEditorNodeText *node_text = CONG_EDITOR_NODE_TEXT(editor_node);
	CongEditorArea *new_area;
	gchar* stripped_text;

	g_return_val_if_fail (editor_node, NULL);

	/* strip out surplus whitespace; use that for the cong_editor_area_text: */
	stripped_text = cong_util_strip_whitespace_from_string (cong_editor_node_get_node (editor_node)->content);

#if 0
	{	

#error
		/* FIXME: this is really broken WRT lifetimes */
		PRIVATE(node_text)->flow_test = ;
		PRIVATE(node_text)->pango_layout = pango_layout_new(gdk_pango_context_get());
		/* can't get children until we set set width which we can't do until we've got a requisition! */
		PRIVATE(node_text)->list_of_text_fragments = ;

		cong_editor_area_container_add_child (parent_area,
						      CONG_EDITOR_AREA( PRIVATE(node_text)->flow_test ));

		return PRIVATE(node_text)->flow_test;
	}
#else

	new_area = cong_editor_area_text_new (cong_editor_node_get_widget (editor_node),
					      cong_app_singleton()->fonts[CONG_FONT_ROLE_TITLE_TEXT], 
					      stripped_text);

	g_free (stripped_text);
	
	return new_area;
#endif
}
