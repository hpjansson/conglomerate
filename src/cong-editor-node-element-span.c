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

#include "cong-editor-area-span-tag.h"
#include "cong-dispspec.h"
#include "cong-editor-line-fragments.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorNodeElementSpanDetails
{
	int dummy;
};

static CongEditorArea*
generate_block_area (CongEditorNode *editor_node);

static CongEditorLineFragments*
generate_line_areas_recursive (CongEditorNode *editor_node,
			       gint line_width,
			       gint initial_indent);

static enum CongFlowType
get_flow_type(CongEditorNode *editor_node);

/* Extra stuff: */
static CongEditorArea*
generate_area (CongEditorNode *editor_node,
	       gboolean is_at_start,
	       gboolean is_at_end);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeElementSpan, 
			cong_editor_node_element_span,
			CongEditorNodeElement,
			CONG_EDITOR_NODE_ELEMENT_TYPE );

static void
cong_editor_node_element_span_class_init (CongEditorNodeElementSpanClass *klass)
{
	CongEditorNodeClass *node_klass = CONG_EDITOR_NODE_CLASS(klass);

	node_klass->generate_block_area = generate_block_area;
	node_klass->generate_line_areas_recursive = generate_line_areas_recursive;
	node_klass->get_flow_type = get_flow_type;
}

static void
cong_editor_node_element_span_instance_init (CongEditorNodeElementSpan *node_element_span)
{
	node_element_span->private = g_new0(CongEditorNodeElementSpanDetails,1);
}

CongEditorNodeElementSpan*
cong_editor_node_element_span_construct (CongEditorNodeElementSpan *editor_node_element_span,
					 CongEditorWidget3* editor_widget,
					 CongNodePtr node,
					 CongEditorNode *traversal_parent)
{
	cong_editor_node_element_construct (CONG_EDITOR_NODE_ELEMENT (editor_node_element_span),
					    editor_widget,
					    node,
					    traversal_parent);

	return editor_node_element_span;
}

CongEditorNode*
cong_editor_node_element_span_new (CongEditorWidget3* widget,
				   CongNodePtr node,
				   CongEditorNode *traversal_parent)
{
#if DEBUG_EDITOR_NODE_LIFETIMES
	g_message("cong_editor_node_element_span_new(%s)", node->name);
#endif

	return CONG_EDITOR_NODE( cong_editor_node_element_span_construct
				 (g_object_new (CONG_EDITOR_NODE_ELEMENT_SPAN_TYPE, NULL),
				  widget,
				  node,
				  traversal_parent));
}

static CongEditorArea*
generate_block_area (CongEditorNode *editor_node)
{
	return generate_area (editor_node,
			      TRUE,
			      TRUE);
}

static CongEditorLineFragments*
generate_line_areas_recursive (CongEditorNode *editor_node,
			       gint line_width,
			       gint initial_indent)
{
	CongEditorLineFragments *result;

#if 0
	g_message("CongEditorNodeElementSpan::generate_line_areas_recursive");
#endif

	result = cong_editor_line_fragments_new();

	/* Iterate over children, getting their line fragments, embellishing them, and adding them to the result: */
	{
		CongNodePtr iter;

		for (iter = cong_editor_node_get_node(editor_node)->children; iter; iter=iter->next) {

			CongEditorNode *editor_node_iter = cong_editor_widget3_get_editor_node (cong_editor_node_get_widget (editor_node),
												iter,
												editor_node);

			if (editor_node_iter) {
				CongEditorLineFragments *child_line_fragments = cong_editor_node_generate_line_areas_recursive (editor_node_iter,
																line_width,
																initial_indent);

				GList* list_of_areas = cong_editor_line_fragments_get_area_list (child_line_fragments);

				gboolean is_first = TRUE;

				while (list_of_areas) {
					
					CongEditorArea *child_area = CONG_EDITOR_AREA(list_of_areas->data);

					CongEditorArea *enclosing_area = generate_area(editor_node,
										       is_first,
										       (NULL==list_of_areas->next));
					
					cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER (enclosing_area),
									      child_area);

					cong_editor_line_fragments_add_area (result,
									     enclosing_area);

					is_first = FALSE;
					list_of_areas = list_of_areas->next;
				}

				g_object_unref (child_line_fragments);
			}
		}
	}
	
	return result;
}

static enum CongFlowType
get_flow_type(CongEditorNode *editor_node)
{
	return CONG_FLOW_TYPE_INLINE;
}

/* Extra stuff: */
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
