/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node.c
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
#include "cong-editor-node.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "cong-app.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-editor-node-element-unknown.h"
#include "cong-editor-node-element-structural.h"
#include "cong-editor-node-element-span.h"
#include "cong-editor-node-comment.h"
#include "cong-editor-node-document.h"
#include "cong-editor-node-dtd.h"
#include "cong-editor-node-entity-decl.h"
#include "cong-editor-node-text.h"
#include "cong-editor-node-unimplemented.h"
#include "cong-plugin.h"

#define PRIVATE(x) ((x)->private)

enum {
	LINE_REGENERATION_REQUIRED,

	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

struct CongEditorNodeDetails
{
	CongEditorWidget3 *widget;

	/* An editor node applies to a specific document node; there is a 1-1 mapping: */
	CongNodePtr node;

#if 0
	CongEditorArea *primary_area;
	CongEditorArea *inner_area;
#endif
};

static enum CongFlowType
get_flow_type(CongEditorNode *editor_node);

CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_node, generate_block_area);
CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_node, generate_line_areas_recursive);


/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNode, 
			cong_editor_node,
			GObject,
			G_TYPE_OBJECT );

static void
cong_editor_node_class_init (CongEditorNodeClass *klass)
{
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_node,
					      generate_block_area);
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_node,
					      generate_line_areas_recursive);

	signals[LINE_REGENERATION_REQUIRED] = g_signal_new ("line_regeneration_required",
							    CONG_EDITOR_NODE_TYPE,
							    G_SIGNAL_RUN_FIRST,
							    0,
							    NULL, NULL,
							    g_cclosure_marshal_VOID__VOID,
							    G_TYPE_NONE, 
							    0);

	klass->get_flow_type = get_flow_type;
}

static void
cong_editor_node_instance_init (CongEditorNode *node)
{
	node->private = g_new0(CongEditorNodeDetails,1);
}

CongEditorNode*
cong_editor_node_construct (CongEditorNode *editor_node,
			    CongEditorWidget3* editor_widget,
			    CongNodePtr node)
{
	PRIVATE(editor_node)->widget = editor_widget;
	PRIVATE(editor_node)->node = node;
}

CongEditorNode*
cong_editor_node_manufacture (CongEditorWidget3* widget,
			      CongNodePtr node)
{
	CongDocument *doc;
	enum CongNodeType type;

	g_return_val_if_fail (widget, NULL);
	g_return_val_if_fail (node, NULL);

	doc = cong_editor_widget3_get_document (widget);
	type = cong_node_type (node);

	switch (type) {
	default: g_assert_not_reached();
	case CONG_NODE_TYPE_ELEMENT:
		{
			CongDispspecElement *ds_element = cong_document_get_dispspec_element_for_node (doc, 
												       node);
			
			if (ds_element) {
				switch (cong_dispspec_element_type(ds_element)) {
				default: g_assert_not_reached();
				case CONG_ELEMENT_TYPE_STRUCTURAL:
					return  cong_editor_node_element_structural_new (widget,
											 node);
					
				case CONG_ELEMENT_TYPE_SPAN:
					return  cong_editor_node_element_span_new (widget,
										   node);
					
				case CONG_ELEMENT_TYPE_INSERT:
				case CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE:
				case CONG_ELEMENT_TYPE_PARAGRAPH:
					return  cong_editor_node_element_unknown_new (widget,
										      node);
					
				case CONG_ELEMENT_TYPE_PLUGIN:
					{
						const gchar *plugin_id = cong_dispspec_element_get_editor_plugin_id (ds_element);
						CongPluginEditorNodeFactory* factory = cong_plugin_manager_locate_editor_node_factory_by_id (cong_app_singleton()->plugin_manager,
																	     plugin_id);
						
						if (factory) {
							return  CONG_EDITOR_NODE( cong_plugin_editor_node_factory_invoke (factory,
															  widget, 
															  node));
						} else {
							g_message("plugin not found \"%s\"", plugin_id);
							return cong_editor_node_element_unknown_new (widget,
												     node);
						}							
					}
					
				case CONG_ELEMENT_TYPE_UNKNOWN:
					return cong_editor_node_element_unknown_new (widget,
										     node);
				} 
			} else {
				return cong_editor_node_element_unknown_new (widget,
									     node);
			}
		}
		
	case CONG_NODE_TYPE_ATTRIBUTE:
		{
			return  cong_editor_node_unimplemented_new (widget, 
								    node,
								    cong_node_type_description (type));
		}
		
	case CONG_NODE_TYPE_TEXT:
		{
			return cong_editor_node_text_new (widget, 
							  node);
		}
		
	case CONG_NODE_TYPE_CDATA_SECTION:
	case CONG_NODE_TYPE_ENTITY_REF:
	case CONG_NODE_TYPE_ENTITY_NODE:
	case CONG_NODE_TYPE_PI:
		{
			return cong_editor_node_unimplemented_new (widget, 
								   node,
								   cong_node_type_description (type));
		}

	case CONG_NODE_TYPE_COMMENT:			
		{
			return cong_editor_node_comment_new (widget, 
							     node);
		}
		
	case CONG_NODE_TYPE_DOCUMENT:
		{
			return cong_editor_node_document_new (widget, 
							      node);
		}

	case CONG_NODE_TYPE_DOCUMENT_TYPE:
	case CONG_NODE_TYPE_DOCUMENT_FRAG:
	case CONG_NODE_TYPE_NOTATION:
	case CONG_NODE_TYPE_HTML_DOCUMENT:
		{
			return cong_editor_node_unimplemented_new (widget, 
								   node,
								   cong_node_type_description (type));
		}

	case CONG_NODE_TYPE_DTD:
		{
			return cong_editor_node_dtd_new (widget, 
							 node);
		}

	case CONG_NODE_TYPE_ELEMENT_DECL:
	case CONG_NODE_TYPE_ATRRIBUTE_DECL:
		{
			return cong_editor_node_unimplemented_new (widget, 
								   node,
								   cong_node_type_description (type));
		}

	case CONG_NODE_TYPE_ENTITY_DECL:
		{
			return cong_editor_node_entity_decl_new (widget, 
								 node);
		}
		
	case CONG_NODE_TYPE_NAMESPACE_DECL:
	case CONG_NODE_TYPE_XINCLUDE_START:
	case CONG_NODE_TYPE_XINCLUDE_END:
		{
			return cong_editor_node_unimplemented_new (widget, 
								   node,
								   cong_node_type_description (type));
		}
	}
}



CongEditorWidget3*
cong_editor_node_get_widget (CongEditorNode *editor_node)
{
	g_return_val_if_fail (editor_node, NULL);

	return PRIVATE(editor_node)->widget;
}

CongDocument*
cong_editor_node_get_document (CongEditorNode *editor_node)
{
	g_return_val_if_fail (editor_node, NULL);
	
	return cong_editor_widget3_get_document( cong_editor_node_get_widget(editor_node));
}

CongNodePtr
cong_editor_node_get_node (CongEditorNode *editor_node)
{
	g_return_val_if_fail (editor_node, NULL);

	return PRIVATE(editor_node)->node;
}

#if 0
CongEditorArea*
cong_editor_node_get_area (CongEditorNode *editor_node)
{
#error
}
#endif

CongEditorArea*
cong_editor_node_generate_block_area (CongEditorNode *editor_node)
{
	g_return_val_if_fail (editor_node, NULL);
	
	return CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_NODE_CLASS,
						       editor_node,
						       generate_block_area, 
						       (editor_node));
}

CongEditorLineFragments*
cong_editor_node_generate_line_areas_recursive (CongEditorNode *editor_node,
						gint line_width,
						gint initial_indent)
{
	g_return_val_if_fail (editor_node, NULL);
	
	return CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_NODE_CLASS,
						       editor_node,
						       generate_line_areas_recursive, 
						       (editor_node, line_width, initial_indent));
}

void
cong_editor_node_line_regeneration_required (CongEditorNode *editor_node)
{
	g_return_if_fail (editor_node);

	g_signal_emit (G_OBJECT(editor_node),
		       signals[LINE_REGENERATION_REQUIRED], 0);
}

enum CongFlowType
cong_editor_node_get_flow_type (CongEditorNode *editor_node)
{
	g_return_if_fail (editor_node);
	
	return CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_NODE_CLASS,
						       editor_node,
						       get_flow_type, 
						       (editor_node));
}

CongEditorNode*
cong_editor_node_get_prev (CongEditorNode *editor_node)
{
	CongNodePtr other_doc_node;

	g_return_val_if_fail (editor_node, NULL);

	other_doc_node = cong_editor_node_get_node(editor_node)->prev;

	if (other_doc_node) {
		return cong_editor_widget3_get_editor_node (cong_editor_node_get_widget (editor_node),
							    other_doc_node);
	} else {
		return NULL;
	}
}

CongEditorNode*
cong_editor_node_get_next (CongEditorNode *editor_node)
{
	CongNodePtr other_doc_node;

	g_return_val_if_fail (editor_node, NULL);

	other_doc_node = cong_editor_node_get_node(editor_node)->next;

	if (other_doc_node) {
		return cong_editor_widget3_get_editor_node (cong_editor_node_get_widget (editor_node),
							    other_doc_node);
	} else {
		return NULL;
	}
}

static enum CongFlowType
get_flow_type(CongEditorNode *editor_node)
{
	return CONG_FLOW_TYPE_BLOCK;
}
