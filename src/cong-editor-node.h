/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node.h
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

#ifndef __CONG_EDITOR_NODE_H__
#define __CONG_EDITOR_NODE_H__

#include "cong-editor-widget.h"

G_BEGIN_DECLS

#define DEBUG_EDITOR_NODE_LIFETIMES 0

#define CONG_EDITOR_NODE_TYPE	      (cong_editor_node_get_type ())
#define CONG_EDITOR_NODE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_NODE_TYPE, CongEditorNode)
#define CONG_EDITOR_NODE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_NODE_TYPE, CongEditorNodeClass)
#define IS_CONG_EDITOR_NODE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_NODE_TYPE)

typedef struct CongEditorNodeDetails CongEditorNodeDetails;

/**
 * CongEditorNode
 * 
 * A CongEditorNode is a per-editor-widget GObject, and represents a node that is visited in a traversal of the xml tree.
 * Hence there is generally a 1-1 mapping between xml nodes and CongEditorNodes.  When an xmlnode is added or removed from the tree, 
 * even temporarily, then a corresponding CongEditorNode is added/removed.
 *
 * However.  if you have an entity ref, then the nodes below the entity decls get visited multiple times in a traversal,
 * hence there are multiple CongEditorNodes for such nodes, one for below the entity decl, and one below every entity ref.
 *
 * In order to support this every editor node know both which xml node it represents, and which "traversal parent" it has.
 * So although it is generally the case that the traversal parent is the parent of the xml node, it is NOT always the case.
 * 
 * The motivating example is for the immediate children of entity references, for which the parent of the xml node is the
 * entity declaration, not the entity reference.  In this case, the traversal parent IS the entity reference node.
 *
 * The traversal parent is stored as a pointer to the relevant CongEditorNode, rather than a CongNodePtr.
 *
 */
struct CongEditorNode
{
	GObject object;

	CongEditorNodeDetails *private;
};

struct CongEditorNodeClass
{
	GObjectClass klass;

	/* Methods? */

	/* Simplistic interface for now: */
	CongEditorArea* (*generate_block_area) (CongEditorNode *editor_node);

	CongEditorLineFragments* (*generate_line_areas_recursive) (CongEditorNode *editor_node,
								   gint line_width,
								   gint initial_indent);

	void (*line_regeneration_required) (CongEditorNode *editor_node);
	
	enum CongFlowType (*get_flow_type) (CongEditorNode *editor_node);
};

GType
cong_editor_node_get_type (void);

CongEditorNode*
cong_editor_node_construct (CongEditorNode *editor_node,
			    CongEditorWidget3* widget,
			    CongNodePtr node,
			    CongEditorNode *traversal_parent);


/*
 * Factory method for creating editor nodes of an appropriate sub-class
 */
CongEditorNode*
cong_editor_node_manufacture (CongEditorWidget3* widget,
			      CongNodePtr node,
			      CongEditorNode *traversal_parent);

CongEditorWidget3*
cong_editor_node_get_widget (CongEditorNode *editor_node);

CongDocument*
cong_editor_node_get_document (CongEditorNode *editor_node);

CongNodePtr
cong_editor_node_get_node (CongEditorNode *editor_node);

CongEditorNode*
cong_editor_node_get_traversal_parent (CongEditorNode *editor_node);

enum CongEditorState
cong_editor_node_get_state (CongEditorNode *editor_node);

void
cong_editor_node_set_state (CongEditorNode *editor_node,
			    enum CongEditorState state);

/* 
   Simplistic node->area interface (1-1 for now).
*/
#if 0
CongEditorArea*
cong_editor_node_get_primary_area (CongEditorNode *editor_node);

CongEditorArea*
cong_editor_node_get_inner_area (CongEditorNode *editor_node);
#endif

CongEditorArea*
cong_editor_node_generate_block_area (CongEditorNode *editor_node);

/* This doesn't actually add the areas anywhere; this has to be done separately (to avoid reparenting issues when the span tags embellish their children's lines: */
CongEditorLineFragments*
cong_editor_node_generate_line_areas_recursive (CongEditorNode *editor_node,
						gint line_width,
						gint initial_indent);

void
cong_editor_node_line_regeneration_required (CongEditorNode *editor_node);

enum CongFlowType
cong_editor_node_get_flow_type (CongEditorNode *editor_node);

/**
 * cong_editor_node_is_referenced_entity_decl
 * @editor_node:
 *
 *  Entity decls can be visited in the tree both below the DTD node, and below each entity ref node that references them.
 *  This function returns TRUE iff the editor_node represents the latter case.
 *  This is useful e.g. if you want to know the "effective siblings" of the node, which should be the other entity decls in the
 *  former case, and should be NULL in the latter case.
 *
 * Returns:
 */
gboolean
cong_editor_node_is_referenced_entity_decl (CongEditorNode *editor_node);

/* May not always succeed; if called during the node creation, the relevant editor_node might not have been created yet: */
CongEditorNode*
cong_editor_node_get_prev (CongEditorNode *editor_node);

CongEditorNode*
cong_editor_node_get_next (CongEditorNode *editor_node);

/* Get the child policy; should only be needed by internals of widget implementation: */
CongEditorChildPolicy*
cong_editor_node_get_child_policy (CongEditorNode *editor_node);

/* Set the child policy; should only be needed by internals of widget implementation: */
void
cong_editor_node_set_child_policy (CongEditorNode *editor_node,
				   CongEditorChildPolicy *child_policy);

/* Get the parent's child policy; should only be needed by internals of widget implementation: */
CongEditorChildPolicy*
cong_editor_node_get_parents_child_policy (CongEditorNode *editor_node);

/* Set the parent's child policy; should only be needed by internals of widget implementation: */
void
cong_editor_node_set_parents_child_policy (CongEditorNode *editor_node,
					   CongEditorChildPolicy *child_policy);


G_END_DECLS

#endif
