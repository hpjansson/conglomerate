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
#include "cong-eel.h"

#include "cong-editor-area-span-tag.h"
#include "cong-dispspec-element.h"
#include "cong-traversal-node.h"
#include "cong-editor-line-manager-span-wrapper.h"
#include "cong-error-dialog.h"

#undef PRIVATE
#define PRIVATE(x) ((x)->private)

struct _CongEditorNodeElementSpanDetails
{
	int dummy;
};


CONG_EDITOR_NODE_DECLARE_HOOKS(cong_editor_node_element_span)

static CongFlowType
get_flow_type(CongEditorNode *editor_node);

/* Extra stuff: */
#if 0
static CongEditorArea*
generate_area (CongEditorNode *editor_node,
	       gboolean is_at_start,
	       gboolean is_at_end);
#endif

/* Exported function definitions: */
G_DEFINE_TYPE(CongEditorNodeElementSpan,
              cong_editor_node_element_span,
              CONG_EDITOR_NODE_ELEMENT_TYPE );

static void
cong_editor_node_element_span_class_init (CongEditorNodeElementSpanClass *klass)
{
	CongEditorNodeClass *node_klass = CONG_EDITOR_NODE_CLASS(klass);

	CONG_EDITOR_NODE_CONNECT_HOOKS(cong_editor_node_element_span)
	node_klass->get_flow_type = get_flow_type;
}

static void
cong_editor_node_element_span_init (CongEditorNodeElementSpan *node_element_span)
{
	node_element_span->private = g_new0(CongEditorNodeElementSpanDetails,1);
}

/**
 * cong_editor_node_element_span_construct:
 * @editor_node_element_span:
 * @widget:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNodeElementSpan*
cong_editor_node_element_span_construct (CongEditorNodeElementSpan *editor_node_element_span,
					 CongEditorWidget3* editor_widget,
					 CongTraversalNode *traversal_node)
{
	cong_editor_node_element_construct (CONG_EDITOR_NODE_ELEMENT (editor_node_element_span),
					    editor_widget,
					    traversal_node);

	return editor_node_element_span;
}

/**
 * cong_editor_node_element_span_new:
 * @widget:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_node_element_span_new (CongEditorWidget3* widget,
				   CongTraversalNode *traversal_node)
{
#if DEBUG_EDITOR_NODE_LIFETIMES
	g_message("cong_editor_node_element_span_new(%s)", node->name);
#endif

	return CONG_EDITOR_NODE( cong_editor_node_element_span_construct
				 (g_object_new (CONG_EDITOR_NODE_ELEMENT_SPAN_TYPE, NULL),
				  widget,
				  traversal_node));
}

static void 
cong_editor_node_element_span_create_areas (CongEditorNode *editor_node,
					    const CongAreaCreationInfo *creation_info)
{
	CongEditorLineManager *line_manager;

	/* Create no areas; the children will add their areas via our LineManager, and this will create the wrapper areas */

	if (!cong_editor_node_get_line_manager_for_children (editor_node)) {
		/* Need to set up the line manager though: */
		line_manager = cong_editor_line_manager_span_wrapper_new
			(cong_editor_node_get_widget (editor_node),
			 editor_node,
			 creation_info->line_manager,
			 creation_info->creation_record,
			 creation_info->line_iter);

		cong_editor_node_set_line_manager_for_children (editor_node,
								line_manager);
		g_object_unref (G_OBJECT (line_manager));
	}
}

static gboolean
cong_editor_node_element_span_needs_area_regeneration (CongEditorNode *editor_node,
						       const CongAreaCreationGeometry *old_creation_geometry,
						       const CongAreaCreationGeometry *new_creation_geometry)
{
	/* What should this do?  For now: */
	return TRUE;
}


static CongFlowType
get_flow_type(CongEditorNode *editor_node)
{
	return CONG_FLOW_TYPE_INLINE;
}

/* Extra stuff: */
#if 0
static CongEditorArea*
generate_area (CongEditorNode *editor_node,
	       gboolean is_at_start,
	       gboolean is_at_end)
{
	CongEditorArea *area;
	CongDispspecElement *ds_element;
	GdkPixbuf *pixbuf;
	gchar *title_text;
	
	g_return_val_if_fail (editor_node, NULL);
	
	ds_element = cong_editor_node_element_get_dispspec_element (CONG_EDITOR_NODE_ELEMENT(editor_node));
	
	pixbuf = cong_dispspec_element_get_icon (ds_element);
	
	title_text = cong_dispspec_element_get_section_header_text (ds_element,
								    cong_editor_node_get_node (editor_node));
	
	area = cong_editor_area_span_tag_new (cong_editor_node_get_widget (editor_node),
					      ds_element,
					      pixbuf,
					      title_text,
					      is_at_start,
					      is_at_end);

	if (pixbuf) {
		g_object_unref (G_OBJECT(pixbuf));
	}

	g_free (title_text);

	return area;
}
#endif
