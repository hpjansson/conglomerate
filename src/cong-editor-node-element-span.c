/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node-element-span.c
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
#include "cong-editor-node-element-span.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorNodeElementSpanDetails
{
	int dummy;
};

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeElementSpan, 
			cong_editor_node_element_span,
			CongEditorNodeElement,
			CONG_EDITOR_NODE_ELEMENT_TYPE );

static void
cong_editor_node_element_span_class_init (CongEditorNodeElementSpanClass *klass)
{
}

static void
cong_editor_node_element_span_instance_init (CongEditorNodeElementSpan *node_element_span)
{
	node_element_span->private = g_new0(CongEditorNodeElementSpanDetails,1);
}

CongEditorNodeElementSpan*
cong_editor_node_element_span_construct (CongEditorNodeElementSpan *editor_node_element_span,
					 CongEditorWidget3* editor_widget,
					 CongNodePtr node)
{
	cong_editor_node_element_construct (CONG_EDITOR_NODE_ELEMENT (editor_node_element_span),
					    editor_widget,
					    node);

	return editor_node_element_span;
}

CongEditorNode*
cong_editor_node_element_span_new (CongEditorWidget3* widget,
				   CongNodePtr node)
{
	return CONG_EDITOR_NODE( cong_editor_node_element_span_construct
				 (g_object_new (CONG_EDITOR_NODE_ELEMENT_SPAN_TYPE, NULL),
				  widget,
				  node));
}
