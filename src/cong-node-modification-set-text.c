/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-node-modification-set-text.c
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
#include "cong-node-modification-set-text.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongNodeModificationSetTextDetails
{
	/* Undo info: */
	gchar *old_content;

	/* Redo info: */
	gchar *new_content;
};

/* Internal function declarations: */
static void
finalize (GObject *object);

static void
dispose (GObject *object);

static void
undo (CongModification *modification);

static void
redo (CongModification *modification);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongNodeModificationSetText, 
			cong_node_modification_set_text,
			CongNodeModification,
			CONG_NODE_MODIFICATION_TYPE );

static void
cong_node_modification_set_text_class_init (CongNodeModificationSetTextClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	CONG_MODIFICATION_CLASS (klass)->undo = undo;
	CONG_MODIFICATION_CLASS (klass)->redo = redo;
}

static void
cong_node_modification_set_text_instance_init (CongNodeModificationSetText *node)
{
	node->private = g_new0(CongNodeModificationSetTextDetails,1);
}

/**
 * cong_node_modification_set_text_construct:
 * @node_modification_set_text:
 * @doc:
 * @node:
 * @new_content:
 *
 * TODO: Write me
 * Returns:
 */
CongNodeModificationSetText*
cong_node_modification_set_text_construct (CongNodeModificationSetText *node_modification_set_text,
					   CongDocument *doc,
					   CongNodePtr node,
					   const gchar *new_content)
{
	cong_node_modification_construct (CONG_NODE_MODIFICATION(node_modification_set_text),
					  doc,
					  node);

	PRIVATE (node_modification_set_text)->old_content = g_strdup (node->content);
	PRIVATE (node_modification_set_text)->new_content = g_strdup (new_content);

	return node_modification_set_text;
}

/**
 * cong_node_modification_set_text_new:
 * @doc:
 * @node:
 * @new_content:
 *
 * TODO: Write me
 * Returns:
 */
CongModification*
cong_node_modification_set_text_new (CongDocument *doc,
				     CongNodePtr node,
				     const gchar *new_content)
{
	return CONG_MODIFICATION(cong_node_modification_set_text_construct (CONG_NODE_MODIFICATION_SET_TEXT(g_object_new (CONG_NODE_MODIFICATION_SET_TEXT_TYPE, NULL)),
									    doc,
									    node,
									    new_content));
}

static void
finalize (GObject *object)
{
	CongNodeModificationSetText *node_modification_set_text = CONG_NODE_MODIFICATION_SET_TEXT (object);

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongNodeModificationSetText::finalize");
#endif

	g_free (node_modification_set_text->private);
	node_modification_set_text->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);

}

static void
dispose (GObject *object)
{
	CongNodeModificationSetText *node_modification_set_text = CONG_NODE_MODIFICATION_SET_TEXT (object);

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongNodeModificationSetText::dispose");
#endif

	g_assert (node_modification_set_text->private);
	
	/* Cleanup: */
	if (PRIVATE (node_modification_set_text)->old_content) {
		g_free (PRIVATE (node_modification_set_text)->old_content);
		PRIVATE (node_modification_set_text)->old_content = NULL;
	}
	if (PRIVATE (node_modification_set_text)->new_content) {
		g_free (PRIVATE (node_modification_set_text)->new_content);
		PRIVATE (node_modification_set_text)->new_content = NULL;
	}

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
undo (CongModification *modification)
{
	CongNodeModificationSetText *node_modification_set_text = CONG_NODE_MODIFICATION_SET_TEXT (modification);
	CongNodePtr node = cong_node_modification_get_node (CONG_NODE_MODIFICATION(modification));
	CongDocument *doc = cong_modification_get_document (modification);

	cong_document_begin_edit (doc);

	cong_document_private_node_set_text (doc,
					     node,
					     PRIVATE (node_modification_set_text)->old_content);
	cong_document_end_edit (doc);
}

static void
redo (CongModification *modification)
{
	CongNodeModificationSetText *node_modification_set_text = CONG_NODE_MODIFICATION_SET_TEXT (modification);
	CongNodePtr node = cong_node_modification_get_node (CONG_NODE_MODIFICATION(modification));
	CongDocument *doc = cong_modification_get_document (modification);

	cong_document_begin_edit (doc);

	cong_document_private_node_set_text (doc,
					     node,
					     PRIVATE (node_modification_set_text)->new_content);
	cong_document_end_edit (doc);
}
