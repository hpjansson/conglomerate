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

	CongEditorAreaText *area_text;
	
	gulong handler_id_node_set_text;

#if 0
	CongEditorAreaComposer *flow_test;
	PangoLayout *pango_layout;
	GList *list_of_text_fragments;
#endif
};

static void
finalize (GObject *object);

static void
dispose (GObject *object);

static CongEditorArea*
generate_area (CongEditorNode *editor_node);


/* FIXME:  We probably shouldn't have every text node in the doc listening to every text node change... probably should allow for a dispatch mechanism within the widget */
/* Declarations of the CongDocument event handlers: */
static void 
on_signal_set_text_notify_after (CongDocument *doc, 
				 CongNodePtr node, 
				 const xmlChar *new_content, 
				 gpointer user_data);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeText, 
			cong_editor_node_text,
			CongEditorNode,
			CONG_EDITOR_NODE_TYPE );

static void
cong_editor_node_text_class_init (CongEditorNodeTextClass *klass)
{
	CongEditorNodeClass *node_klass = CONG_EDITOR_NODE_CLASS(klass);

	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	node_klass->generate_area = generate_area;
}

static void
cong_editor_node_text_instance_init (CongEditorNodeText *node_text)
{
	node_text->private = g_new0(CongEditorNodeTextDetails,1);
}

static void
finalize (GObject *object)
{
	CongEditorNodeText *editor_node_text = CONG_EDITOR_NODE_TEXT(object);

	g_free (editor_node_text->private);
	editor_node_text->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}


CongEditorNodeText*
cong_editor_node_text_construct (CongEditorNodeText *editor_node_text,
				 CongEditorWidget3* editor_widget,
				 CongNodePtr node)
{
	cong_editor_node_construct (CONG_EDITOR_NODE (editor_node_text),
				    editor_widget,
				    node);

	PRIVATE(editor_node_text)->handler_id_node_set_text = g_signal_connect_after (G_OBJECT(cong_editor_widget3_get_document(editor_widget)), 
										      "node_set_text",
										      G_CALLBACK(on_signal_set_text_notify_after),
										      editor_node_text);

	return editor_node_text;
}

static void
dispose (GObject *object)
{
	CongEditorNodeText *editor_node_text = CONG_EDITOR_NODE_TEXT(object);

	g_signal_handler_disconnect (G_OBJECT(cong_editor_node_get_document(CONG_EDITOR_NODE(object))),
				     PRIVATE(editor_node_text)->handler_id_node_set_text);	

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}


CongEditorNode*
cong_editor_node_text_new (CongEditorWidget3 *widget,
			   CongNodePtr node)
{
#if DEBUG_EDITOR_NODE_LIFETIMES
	g_message("cong_editor_node_text_new(%s)", node->content);
#endif

	return CONG_EDITOR_NODE( cong_editor_node_text_construct (g_object_new (CONG_EDITOR_NODE_TEXT_TYPE, NULL),
								  widget,
								  node)
				 );
}

static CongEditorArea*
generate_area (CongEditorNode *editor_node)
{
	CongEditorNodeText *node_text = CONG_EDITOR_NODE_TEXT(editor_node);
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

	PRIVATE(node_text)->area_text = 
		CONG_EDITOR_AREA_TEXT( cong_editor_area_text_new (cong_editor_node_get_widget (editor_node),
								  cong_app_singleton()->fonts[CONG_FONT_ROLE_BODY_TEXT], 
								  NULL, 
								  stripped_text)
				       );
	g_free (stripped_text);
	
	return CONG_EDITOR_AREA(PRIVATE(node_text)->area_text);
#endif
}

/* Definitions of the CongDocument event handlers: */
static void 
on_signal_set_text_notify_after (CongDocument *doc, 
				 CongNodePtr node, 
				 const xmlChar *new_content, 
				 gpointer user_data)
{
	CongEditorNodeText *editor_node_text = (CongEditorNodeText*)user_data;
	gchar* stripped_text;
	
	g_return_if_fail (IS_CONG_EDITOR_NODE_TEXT(editor_node_text));

	/* FIXME: need smarter dispatch mechanism: */
	if (node == cong_editor_node_get_node( CONG_EDITOR_NODE(editor_node_text))) {

		stripped_text = cong_util_strip_whitespace_from_string (cong_editor_node_get_node (CONG_EDITOR_NODE(editor_node_text))->content);

		g_assert(PRIVATE(editor_node_text)->area_text);
		cong_editor_area_text_set_text (PRIVATE(editor_node_text)->area_text,
						stripped_text);

		g_free (stripped_text);
	}
}

