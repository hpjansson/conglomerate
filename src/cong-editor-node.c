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
#include "cong-dispspec-element.h"
#include "cong-editor-node-element-unknown.h"
#include "cong-editor-node-element-structural.h"
#include "cong-editor-node-element-span.h"
#include "cong-editor-node-comment.h"
#include "cong-editor-node-document.h"
#include "cong-editor-node-dtd.h"
#include "cong-editor-node-entity-decl.h"
#include "cong-editor-node-entity-ref.h"
#include "cong-editor-node-text.h"
#include "cong-editor-node-unimplemented.h"
#include "cong-plugin-manager.h"
#include "cong-traversal-node.h"
#include "cong-editor-area.h"
#include "cong-editor-line-manager.h"
#include "cong-editor-line-manager-simple.h"
#include "cong-editor-area-composer.h"
#include "cong-editor-area-border.h"
#include "cong-editor-area-lines.h"
#include "cong-editor-line-iter.h"
#include "cong-service-editor-node-factory.h"

#define PRIVATE(x) ((x)->priv)

enum {
	LINE_REGENERATION_REQUIRED,

	IS_SELECTED_CHANGED,

	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

struct CongEditorNodePrivate
{
	CongEditorWidget3 *widget;

	CongTraversalNode *traversal_node;

#if 1
	CongEditorLineManager *line_manager_for_children;
#else
	CongEditorChildPolicy *child_policy;
	CongEditorChildPolicy *parents_child_policy;
#endif

	gboolean is_selected;
};

static CongFlowType
get_flow_type(CongEditorNode *editor_node);

#if 0
CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_node, generate_block_area);
CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_node, generate_line_areas_recursive);
#endif


/* Exported function definitions: */
CONG_DEFINE_CLASS_BEGIN(CongEditorNode, 
			cong_editor_node,
			CONG_EDITOR_NODE,
			GObject,
			G_TYPE_OBJECT )
#if 0
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_node,
					      generate_block_area);
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_node,
					      generate_line_areas_recursive);
#endif

	signals[LINE_REGENERATION_REQUIRED] = g_signal_new ("line_regeneration_required",
							    CONG_EDITOR_NODE_TYPE,
							    G_SIGNAL_RUN_FIRST,
							    0,
							    NULL, NULL,
							    g_cclosure_marshal_VOID__VOID,
							    G_TYPE_NONE, 
							    0);

	signals[IS_SELECTED_CHANGED] = g_signal_new ("is_selected_changed",
						     CONG_EDITOR_NODE_TYPE,
						     G_SIGNAL_RUN_FIRST,
						     0,
						     NULL, NULL,
						     g_cclosure_marshal_VOID__VOID,
						     G_TYPE_NONE, 
						     0);

	klass->get_flow_type = get_flow_type;
CONG_DEFINE_CLASS_END()

/**
 * cong_editor_node_construct:
 * @editor_node:
 * @widget:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_node_construct (CongEditorNode *editor_node,
			    CongEditorWidget3* editor_widget,
			    CongTraversalNode *traversal_node)
{
	PRIVATE(editor_node)->widget = editor_widget;
	PRIVATE(editor_node)->traversal_node = traversal_node;

	return editor_node;
}

static void
cong_editor_node_dispose (GObject *object)
{
	CongEditorNode *editor_node = CONG_EDITOR_NODE (object);

	if (PRIVATE (editor_node)->line_manager_for_children) {
		g_object_unref (G_OBJECT (PRIVATE (editor_node)->line_manager_for_children));
		PRIVATE (editor_node)->line_manager_for_children = NULL;
	}	
}

/**
 * cong_editor_node_manufacture:
 * @widget:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_node_manufacture (CongEditorWidget3* widget,
			      CongTraversalNode *traversal_node)
{
	CongDocument *doc;
	CongNodePtr xml_node;
	CongNodeType type;

	g_return_val_if_fail (IS_CONG_EDITOR_WIDGET3 (widget), NULL);
	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE (traversal_node), NULL);

	doc = cong_editor_widget3_get_document (widget);
	xml_node = cong_traversal_node_get_node (traversal_node);
	type = cong_node_type (xml_node);

	switch (type) {
	default: g_assert_not_reached();
	case CONG_NODE_TYPE_ELEMENT:
		{
			CongDispspecElement *ds_element = cong_document_get_dispspec_element_for_node (doc, 
												       xml_node);
			
			if (ds_element) {
				switch (cong_dispspec_element_type(ds_element)) {
				default: g_assert_not_reached();
				case CONG_ELEMENT_TYPE_STRUCTURAL:
					return  cong_editor_node_element_structural_new (widget,
											 traversal_node);
					
				case CONG_ELEMENT_TYPE_SPAN:
					return  cong_editor_node_element_span_new (widget,
										   traversal_node);
					
				case CONG_ELEMENT_TYPE_INSERT:
				case CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE:
					return  cong_editor_node_element_unknown_new (widget,
										      traversal_node);
					
				case CONG_ELEMENT_TYPE_PLUGIN:
					{
						const gchar *service_id = cong_dispspec_element_get_editor_service_id (ds_element);
						CongServiceEditorNodeFactory* factory = cong_plugin_manager_locate_editor_node_factory_by_id (cong_app_get_plugin_manager (cong_app_singleton()),
																	      service_id);
						
						if (factory) {
							return  CONG_EDITOR_NODE( cong_plugin_editor_node_factory_invoke (factory,
															  widget,
															  traversal_node)
										  );
						} else {
							g_message ("plugin not found \"%s\"", service_id);
							return cong_editor_node_element_unknown_new (widget,
												     traversal_node);
						}							
					}
					
				case CONG_ELEMENT_TYPE_UNKNOWN:
					return cong_editor_node_element_unknown_new (widget,
										     traversal_node);
				} 
			} else {
				return cong_editor_node_element_unknown_new (widget,
									     traversal_node);
			}
		}
		
	case CONG_NODE_TYPE_ATTRIBUTE:
		{
			return  cong_editor_node_unimplemented_new (widget, 
								    traversal_node,
								    cong_node_type_description (type));
		}
		
	case CONG_NODE_TYPE_TEXT:
		{
			return cong_editor_node_text_new (widget, 
							  traversal_node);
		}
		
	case CONG_NODE_TYPE_CDATA_SECTION:
		{
			return cong_editor_node_unimplemented_new (widget, 
								   traversal_node,
								   cong_node_type_description (type));
		}

	case CONG_NODE_TYPE_ENTITY_REF:
		{
			return cong_editor_node_entity_ref_new (widget, 
								traversal_node);
		}

	case CONG_NODE_TYPE_ENTITY_NODE:
	case CONG_NODE_TYPE_PI:
		{
			return cong_editor_node_unimplemented_new (widget, 
								   traversal_node,
								   cong_node_type_description (type));
		}

	case CONG_NODE_TYPE_COMMENT:			
		{
			return cong_editor_node_comment_new (widget, 
							     traversal_node);
		}
		
	case CONG_NODE_TYPE_DOCUMENT:
		{
			return cong_editor_node_document_new (widget, 
							      traversal_node);
		}

	case CONG_NODE_TYPE_DOCUMENT_TYPE:
	case CONG_NODE_TYPE_DOCUMENT_FRAG:
	case CONG_NODE_TYPE_NOTATION:
	case CONG_NODE_TYPE_HTML_DOCUMENT:
		{
			return cong_editor_node_unimplemented_new (widget, 
								   traversal_node,
								   cong_node_type_description (type));
		}

	case CONG_NODE_TYPE_DTD:
		{
			return cong_editor_node_dtd_new (widget, 
							 traversal_node);
		}

	case CONG_NODE_TYPE_ELEMENT_DECL:
	case CONG_NODE_TYPE_ATRRIBUTE_DECL:
		{
			return cong_editor_node_unimplemented_new (widget, 
								   traversal_node,
								   cong_node_type_description (type));
		}

	case CONG_NODE_TYPE_ENTITY_DECL:
		{
			return cong_editor_node_entity_decl_new (widget, 
								 traversal_node);
		}
		
	case CONG_NODE_TYPE_NAMESPACE_DECL:
	case CONG_NODE_TYPE_XINCLUDE_START:
	case CONG_NODE_TYPE_XINCLUDE_END:
		{
			return cong_editor_node_unimplemented_new (widget, 
								   traversal_node,
								   cong_node_type_description (type));
		}
	}
}

/**
 * cong_editor_node_get_widget:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorWidget3*
cong_editor_node_get_widget (CongEditorNode *editor_node)
{
	g_return_val_if_fail (IS_CONG_EDITOR_NODE (editor_node), NULL);

	return PRIVATE(editor_node)->widget;
}

/**
 * cong_editor_node_get_document:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
CongDocument*
cong_editor_node_get_document (CongEditorNode *editor_node)
{
	g_return_val_if_fail (IS_CONG_EDITOR_NODE (editor_node), NULL);
	
	return cong_editor_widget3_get_document( cong_editor_node_get_widget(editor_node));
}

/**
 * cong_editor_node_get_node:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr
cong_editor_node_get_node (CongEditorNode *editor_node)
{
	g_return_val_if_fail (IS_CONG_EDITOR_NODE (editor_node), NULL);

	return cong_traversal_node_get_node (PRIVATE(editor_node)->traversal_node);
}

/**
 * cong_editor_node_get_traversal_node:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
CongTraversalNode*
cong_editor_node_get_traversal_node (CongEditorNode *editor_node)
{
	g_return_val_if_fail (IS_CONG_EDITOR_NODE (editor_node), NULL);

	return PRIVATE(editor_node)->traversal_node;	
}

/**
 * cong_editor_node_get_traversal_parent:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_node_get_traversal_parent (CongEditorNode *editor_node)
{
	CongTraversalNode *traversal_node_parent;

	g_return_val_if_fail (IS_CONG_EDITOR_NODE (editor_node), NULL);

	traversal_node_parent = cong_traversal_node_get_parent (PRIVATE(editor_node)->traversal_node);

	return cong_editor_widget3_get_editor_node_for_traversal_node (PRIVATE(editor_node)->widget,
								       traversal_node_parent);
}

/**
 * cong_editor_node_is_selected:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_editor_node_is_selected (CongEditorNode *editor_node)
{
	g_return_val_if_fail (IS_CONG_EDITOR_NODE(editor_node), FALSE);

	return PRIVATE(editor_node)->is_selected;	
}

/**
 * cong_editor_node_private_set_selected:
 * @editor_node:
 * @is_selected:
 *
 * TODO: Write me
 */
void
cong_editor_node_private_set_selected (CongEditorNode *editor_node,
				       gboolean is_selected)
{
	g_return_if_fail (IS_CONG_EDITOR_NODE(editor_node));

	if (PRIVATE(editor_node)->is_selected!=is_selected) {
		PRIVATE(editor_node)->is_selected =is_selected;

		g_signal_emit (G_OBJECT(editor_node),
			       signals[IS_SELECTED_CHANGED], 0);
	}	
}

gboolean
cong_editor_node_needs_area_regeneration (CongEditorNode *editor_node,
					  const CongAreaCreationGeometry *old_creation_geometry,
					  const CongAreaCreationGeometry *new_creation_geometry)
{
	gboolean result;

	g_return_val_if_fail (IS_CONG_EDITOR_NODE (editor_node), FALSE);
	g_return_val_if_fail (old_creation_geometry, FALSE);
	g_return_val_if_fail (new_creation_geometry, FALSE);

#if 1
	g_assert (CONG_EDITOR_NODE_CLASS (G_OBJECT_GET_CLASS (editor_node))->needs_area_regeneration != NULL);
	
	result = CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_NODE_CLASS,
							 editor_node,
							 needs_area_regeneration, 
							 (editor_node, old_creation_geometry, new_creation_geometry));
#else
	/* For now, always regenerate everything (expensive): */
	/* FIXME: we can't hope to interpret these iter objects;
	   probably should be being passed some simple structure containing line ptr, width, indent etc */
	result = TRUE;
#endif

#if 1
	{
		gchar *desc = cong_node_debug_description (cong_editor_node_get_node (editor_node));

		g_message ("cong_editor_node_needs_area_regeneration returning %s old:(line %p, width: %i indent: %i) new: (line %p, width: %i indent: %i), node: %s",
			   (result?"TRUE":"FALSE"),
			   old_creation_geometry->area_line, old_creation_geometry->line_width, old_creation_geometry->line_indent, 
			   new_creation_geometry->area_line, new_creation_geometry->line_width, new_creation_geometry->line_indent, 
			   desc);

		g_free (desc);
		
	}
#endif


	return result;
}

#if 0
/**
 * cong_editor_node_generate_block_area:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_node_generate_block_area (CongEditorNode *editor_node)
{
	g_return_val_if_fail (IS_CONG_EDITOR_NODE (editor_node), NULL);

	g_assert (CONG_EDITOR_NODE_CLASS (G_OBJECT_GET_CLASS (editor_node))->generate_block_area != NULL);
	
	return CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_NODE_CLASS,
						       editor_node,
						       generate_block_area, 
						       (editor_node));
}

/**
 * cong_editor_node_generate_line_areas_recursive:
 * @editor_node:
 * @line_width:
 * @initial_indent:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorLineFragments*
cong_editor_node_generate_line_areas_recursive (CongEditorNode *editor_node,
						gint line_width,
						gint initial_indent)
{
	g_return_val_if_fail (IS_CONG_EDITOR_NODE (editor_node), NULL);

	/* Dubious hack: */
	if (CONG_EDITOR_NODE_CLASS (G_OBJECT_GET_CLASS (editor_node))->generate_line_areas_recursive == NULL) {
		return NULL;
	}

	g_assert (CONG_EDITOR_NODE_CLASS (G_OBJECT_GET_CLASS (editor_node))->generate_line_areas_recursive != NULL);
	
	return CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_NODE_CLASS,
						       editor_node,
						       generate_line_areas_recursive, 
						       (editor_node, line_width, initial_indent));
}
#endif

/**
 * cong_editor_node_line_regeneration_required:
 * @editor_node:
 *
 * TODO: Write me
 */
void
cong_editor_node_line_regeneration_required (CongEditorNode *editor_node)
{
	g_return_if_fail (editor_node);

	g_signal_emit (G_OBJECT(editor_node),
		       signals[LINE_REGENERATION_REQUIRED], 0);
}

/**
 * cong_editor_node_get_flow_type:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
CongFlowType
cong_editor_node_get_flow_type (CongEditorNode *editor_node)
{
	CongFlowType flow_type;
		
	g_return_val_if_fail (editor_node, CONG_FLOW_TYPE_BLOCK);

	
	flow_type = CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_NODE_CLASS,
							    editor_node,
							    get_flow_type, 
							    (editor_node));

#if 0
	switch (flow_type) {
	default: g_assert_not_reached();
	case CONG_FLOW_TYPE_BLOCK:
		g_assert (CONG_EDITOR_NODE_CLASS (G_OBJECT_GET_CLASS (editor_node))->generate_block_area != NULL);
		break;
	case CONG_FLOW_TYPE_INLINE:
		g_assert (CONG_EDITOR_NODE_CLASS (G_OBJECT_GET_CLASS (editor_node))->generate_line_areas_recursive != NULL);
		break;
	}
#endif
	
	return flow_type;
}

#if 1
/**
 * cong_editor_node_is_referenced_entity_decl:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_editor_node_is_referenced_entity_decl (CongEditorNode *editor_node)
{
	CongTraversalNode *traversal_node;

	g_return_val_if_fail (IS_CONG_EDITOR_NODE(editor_node), FALSE);

	traversal_node = cong_editor_node_get_traversal_node (editor_node);

	return cong_traversal_node_is_referenced_entity_decl (traversal_node);
}

/**
 * cong_editor_node_get_prev:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_node_get_prev (CongEditorNode *editor_node)
{
	CongTraversalNode *traversal_node;
	CongTraversalNode *traversal_node_prev;

	g_return_val_if_fail (IS_CONG_EDITOR_NODE(editor_node), NULL);

	traversal_node = cong_editor_node_get_traversal_node (editor_node);
	traversal_node_prev = cong_traversal_node_get_prev (traversal_node);

	if (traversal_node_prev) {

		/* Scan backwards for the case in which the previous node doesn't appear in the widget (fix for bug #129907): */
		while (!cong_node_should_be_visible_in_editor (cong_traversal_node_get_node (traversal_node_prev))) {

			traversal_node_prev = cong_traversal_node_get_prev (traversal_node_prev);
			
			if (traversal_node_prev==NULL) {
				return NULL;
			}
		}

		g_assert (traversal_node_prev);
		g_assert (cong_node_should_be_visible_in_editor (cong_traversal_node_get_node (traversal_node_prev)));

		return cong_editor_widget3_get_editor_node_for_traversal_node (cong_editor_node_get_widget (editor_node),
									       traversal_node_prev);
	} else {
		return NULL;
	}
}

/**
 * cong_editor_node_get_next:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_node_get_next (CongEditorNode *editor_node)
{
	CongTraversalNode *traversal_node;
	CongTraversalNode *traversal_node_next;

	g_return_val_if_fail (IS_CONG_EDITOR_NODE(editor_node), NULL);

	traversal_node = cong_editor_node_get_traversal_node (editor_node);
	traversal_node_next = cong_traversal_node_get_next (traversal_node);

	if (traversal_node_next) {
		/* Scan forwards for the case in which the next node doesn't appear in the widget (fix for bug #129907): */
		while (!cong_node_should_be_visible_in_editor (cong_traversal_node_get_node (traversal_node_next))) {

			traversal_node_next = cong_traversal_node_get_next (traversal_node_next);
			
			if (traversal_node_next==NULL) {
				return NULL;
			}
		}

		g_assert (traversal_node_next);
		g_assert (cong_node_should_be_visible_in_editor (cong_traversal_node_get_node (traversal_node_next)));

		return cong_editor_widget3_get_editor_node_for_traversal_node (cong_editor_node_get_widget (editor_node),
									       traversal_node_next);
	} else {
		return NULL;
	}
}
#else
/**
 * cong_editor_node_is_referenced_entity_decl:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_editor_node_is_referenced_entity_decl (CongEditorNode *editor_node)
{
	g_return_val_if_fail (IS_CONG_EDITOR_NODE(editor_node), FALSE);

	if (cong_node_type(cong_editor_node_get_node(editor_node))==CONG_NODE_TYPE_ENTITY_DECL) {
		if (cong_node_type(cong_editor_node_get_node(cong_editor_node_get_traversal_parent(editor_node)))==CONG_NODE_TYPE_ENTITY_REF) {
			g_message ("got a referenced entity decl");
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * cong_editor_node_get_prev:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_node_get_prev (CongEditorNode *editor_node)
{
	CongNodePtr other_doc_node;

	g_return_val_if_fail (editor_node, NULL);

	/* If we're traversing below an entity ref node, visiting an entity decl node, then don't return the siblings (which are all the other entity decls in this document */
	if (cong_editor_node_is_referenced_entity_decl(editor_node)) {
		return NULL;
	}

	other_doc_node = cong_editor_node_get_node(editor_node)->prev;


	if (other_doc_node) {
		return cong_editor_widget3_get_editor_node (cong_editor_node_get_widget (editor_node),
							    other_doc_node,
							    PRIVATE(editor_node)->traversal_parent);
	} else {
		return NULL;
	}
}

/**
 * cong_editor_node_get_next:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_node_get_next (CongEditorNode *editor_node)
{
	CongNodePtr other_doc_node;

	g_return_val_if_fail (editor_node, NULL);

	/* If we're traversing below an entity ref node, visiting an entity decl node, then don't return the siblings (which are all the other entity decls in this document */
	if (cong_editor_node_is_referenced_entity_decl(editor_node)) {
		return NULL;
	}

	other_doc_node = cong_editor_node_get_node(editor_node)->next;

	if (other_doc_node) {
		return cong_editor_widget3_get_editor_node (cong_editor_node_get_widget (editor_node),
							    other_doc_node,
							    PRIVATE(editor_node)->traversal_parent);
	} else {
		return NULL;
	}
}
#endif


#if 1
CongEditorLineManager*
cong_editor_node_get_line_manager_for_children (CongEditorNode *editor_node)
{
	g_return_val_if_fail (IS_CONG_EDITOR_NODE(editor_node), NULL);

	return PRIVATE(editor_node)->line_manager_for_children;
}

void
cong_editor_node_set_line_manager_for_children (CongEditorNode *editor_node,
						CongEditorLineManager *line_manager)
{
	g_return_if_fail (IS_CONG_EDITOR_NODE (editor_node));
	g_return_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager));

	if (PRIVATE(editor_node)->line_manager_for_children) {
		g_object_unref (G_OBJECT (PRIVATE(editor_node)->line_manager_for_children));
	}

	PRIVATE(editor_node)->line_manager_for_children = line_manager;
	g_object_ref (G_OBJECT (line_manager));
}
#else
/**
 * cong_editor_node_get_child_policy:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorChildPolicy*
cong_editor_node_get_child_policy (CongEditorNode *editor_node)
{
	g_return_val_if_fail (IS_CONG_EDITOR_NODE(editor_node), NULL);

	return PRIVATE(editor_node)->child_policy;
}

/**
 * cong_editor_node_set_child_policy:
 * @editor_node:
 * @child_policy:
 *
 * TODO: Write me
 */
void
cong_editor_node_set_child_policy (CongEditorNode *editor_node,
				   CongEditorChildPolicy *child_policy)
{
	g_return_if_fail (IS_CONG_EDITOR_NODE(editor_node));

	PRIVATE(editor_node)->child_policy = child_policy;
}

/**
 * cong_editor_node_get_parents_child_policy:
 * @editor_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorChildPolicy*
cong_editor_node_get_parents_child_policy (CongEditorNode *editor_node)
{
	g_return_val_if_fail (IS_CONG_EDITOR_NODE(editor_node), NULL);

	return PRIVATE(editor_node)->parents_child_policy;
}

/**
 * cong_editor_node_set_parents_child_policy:
 * @editor_node:
 * @child_policy:
 *
 * TODO: Write me
 */
void
cong_editor_node_set_parents_child_policy (CongEditorNode *editor_node,
					   CongEditorChildPolicy *child_policy)
{
	g_return_if_fail (IS_CONG_EDITOR_NODE(editor_node));

	PRIVATE(editor_node)->parents_child_policy = child_policy;
}
#endif

static void
set_up_line_manager (CongEditorNode *editor_node,
		     CongEditorArea *block_area)
{
	CongEditorLineManager *line_manager;
	CongEditorArea *area_lines;
	CongEditorArea *area_border;
	CongEditorWidget3 *widget;

	g_assert (IS_CONG_EDITOR_AREA_CONTAINER (block_area));

	widget = cong_editor_node_get_widget (editor_node);

	/* Set up area for children: */
	area_lines = cong_editor_area_lines_new (widget);

	/* Add a border between the block_area and the area_lines in the hierarchy */
	area_border = cong_editor_area_border_new (widget, 
						   5,
						   0,
						   5,
						   5);

	cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER (block_area),
					       area_border,
					       TRUE);

	cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER (area_border),
					       area_lines,
					       TRUE);

	/* Set up line manager: */
	line_manager = cong_editor_line_manager_simple_new (widget,
							    CONG_EDITOR_AREA_LINES (area_lines));
	cong_editor_node_set_line_manager_for_children (editor_node,
							line_manager);
	g_object_unref (G_OBJECT (line_manager));

	/* FIXME: If this area is removed, we need to destroy the line manager: */
}

void
cong_editor_node_create_block_area (CongEditorNode *editor_node,
				    const CongAreaCreationInfo *creation_info,
				    CongEditorArea *block_area,
				    gboolean allow_children)
{
	g_return_if_fail (IS_CONG_EDITOR_NODE (editor_node));
	g_return_if_fail (creation_info);
	g_return_if_fail (IS_CONG_EDITOR_AREA (block_area));

	g_assert (IS_CONG_EDITOR_LINE_MANAGER (creation_info->line_manager));

	cong_editor_line_manager_begin_line (creation_info->line_manager,
					     creation_info->creation_record,
					     creation_info->line_iter);
	cong_editor_line_manager_add_to_line (creation_info->line_manager,
					      creation_info->creation_record,
					      creation_info->line_iter,
					      block_area);
	cong_editor_line_manager_end_line (creation_info->line_manager,
					   creation_info->creation_record,
					   creation_info->line_iter);

	if (allow_children) {
		g_assert (IS_CONG_EDITOR_AREA_CONTAINER (block_area));

		set_up_line_manager (editor_node,
				     block_area);
	}
}

void
cong_editor_node_empty_create_area (CongEditorNode *editor_node,
				    const CongAreaCreationInfo *creation_info,
				    gboolean allow_children)
{
	CongEditorArea *dummy_area;

	g_return_if_fail (IS_CONG_EDITOR_NODE (editor_node));
	g_return_if_fail (creation_info);

	dummy_area = cong_editor_area_lines_new (cong_editor_node_get_widget (editor_node));

	cong_editor_line_manager_add_to_line (creation_info->line_manager,
					      creation_info->creation_record,
					      creation_info->line_iter,
					      dummy_area);

	if (allow_children) {
		set_up_line_manager (editor_node,
				     dummy_area);
	}
}

static CongFlowType
get_flow_type(CongEditorNode *editor_node)
{
	return CONG_FLOW_TYPE_BLOCK;
}
