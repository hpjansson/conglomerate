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

#include "cong-document.h"
#include "cong-editor-widget.h"
#include "cong-object.h"

G_BEGIN_DECLS

typedef struct CongAreaCreationInfo CongAreaCreationInfo;
typedef struct CongAreaCreationGeometry CongAreaCreationGeometry;

#define DEBUG_EDITOR_NODE_LIFETIMES 0

#define CONG_EDITOR_NODE_TYPE	      (cong_editor_node_get_type ())
#define CONG_EDITOR_NODE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_NODE_TYPE, CongEditorNode)
#define CONG_EDITOR_NODE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_NODE_TYPE, CongEditorNodeClass)
#define IS_CONG_EDITOR_NODE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_NODE_TYPE)

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
CONG_DECLARE_CLASS_BEGIN(CongEditorNode, cong_editor_node, GObject)
	/* Methods? */
#if 1
	void (*create_areas) (CongEditorNode *editor_node,
			      const CongAreaCreationInfo *creation_info);

	gboolean
	(*needs_area_regeneration) (CongEditorNode *editor_node,
				    const CongAreaCreationGeometry *old_creation_geometry,
				    const CongAreaCreationGeometry *new_creation_geometry);
#else
	/* Simplistic interface for now: */
	CongEditorArea* (*generate_block_area) (CongEditorNode *editor_node);

	CongEditorLineFragments* (*generate_line_areas_recursive) (CongEditorNode *editor_node,
								   gint line_width,
								   gint initial_indent);
#endif

	void (*line_regeneration_required) (CongEditorNode *editor_node);
	
	CongFlowType (*get_flow_type) (CongEditorNode *editor_node);
CONG_DECLARE_CLASS_END()

struct CongAreaCreationInfo
{
	CongEditorLineManager *line_manager;

	/* record of the various nodes added, lines begun and ended: */
	CongEditorCreationRecord *creation_record;

	/* Position at which to add areas (so that when a node is inserted between two existing nodes, 
	   we can add the areas between their areas.  Note, though that subsequent areas may well need regenerating
	   since the word-wrap will often start at a different place).

	   The line_iter will get modified as actions are performed on it; it represents the "current" position.
	*/
	CongEditorLineIter *line_iter;
};


struct CongAreaCreationGeometry
{
	/* Cache of data that the areas of a node were created with; if changes occur then the areas may need to be regenerated: */
	CongEditorAreaLine *area_line;
	gint line_width;
	gint line_indent;
};

CongEditorNode*
cong_editor_node_construct (CongEditorNode *editor_node,
			    CongEditorWidget3* widget,
			    CongTraversalNode *traversal_node);

/*
 * Factory method for creating editor nodes of an appropriate sub-class
 */
CongEditorNode*
cong_editor_node_manufacture (CongEditorWidget3* widget,
			      CongTraversalNode *traversal_node);

CongEditorWidget3*
cong_editor_node_get_widget (CongEditorNode *editor_node);

CongDocument*
cong_editor_node_get_document (CongEditorNode *editor_node);

CongNodePtr
cong_editor_node_get_node (CongEditorNode *editor_node);

CongTraversalNode*
cong_editor_node_get_traversal_node (CongEditorNode *editor_node);

CongEditorNode*
cong_editor_node_get_traversal_parent (CongEditorNode *editor_node);

gboolean
cong_editor_node_is_selected (CongEditorNode *editor_node);

/*
  This function should only be called by the editor widget internals:
 */
void
cong_editor_node_private_set_selected (CongEditorNode *editor_node,
				       gboolean is_selected);


#if 1
void 
cong_editor_node_create_areas (CongEditorNode *editor_node,
			       const CongAreaCreationInfo *creation_info);

/*
  Function to decide if the areas for this node should be regenerated, based upon pertinent information such as
  current line, width of line and current indent etc.
  
  Typically a block-style area won't be affected by its start position; it always starts a newline etc.
  But inline flow stuff might look at this stuff and reflow when needed...
*/
gboolean
cong_editor_node_needs_area_regeneration (CongEditorNode *editor_node,
					  const CongAreaCreationGeometry *old_creation_geometry,
					  const CongAreaCreationGeometry *new_creation_geometry);
#else
CongEditorArea*
cong_editor_node_generate_block_area (CongEditorNode *editor_node);

/* This doesn't actually add the areas anywhere; this has to be done separately (to avoid reparenting issues when the span tags embellish their children's lines: */
CongEditorLineFragments*
cong_editor_node_generate_line_areas_recursive (CongEditorNode *editor_node,
						gint line_width,
						gint initial_indent);
#endif

void
cong_editor_node_line_regeneration_required (CongEditorNode *editor_node);

#if 0
enum CongFlowType
cong_editor_node_get_flow_type (CongEditorNode *editor_node);
#endif

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

#if 1
CongEditorLineManager*
cong_editor_node_get_line_manager_for_children (CongEditorNode *editor_node);

void
cong_editor_node_set_line_manager_for_children (CongEditorNode *editor_node,
						CongEditorLineManager *line_manager);
#else
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
#endif


/**
 * cong_editor_node_create_block_area:
 *
 * @editor_node: the #CongEditorNode
 * @creation_info:
 * @block_area: the block-style area that will represent this editor node.
 * @allow_children: Can this node have children?  If so, then the block area must be a subclass of #CongEditorAreaContainer
 *
 * Utility function for implementing the create_areas function for node subclasses that expect to create a single block area.
 */
void
cong_editor_node_create_block_area (CongEditorNode *editor_node,
				    const CongAreaCreationInfo *creation_info,
				    CongEditorArea *block_area,
				    gboolean allow_children);

/* Utility placeholder function for node subclasses that don't really create an area themselves (e.g. for the document, for DTDs etc;
   might be empty, might not; not sure yet. */
void
cong_editor_node_empty_create_area (CongEditorNode *editor_node,
				    const CongAreaCreationInfo *creation_info,
				    gboolean allow_children);

/* Macros for creating subclasses more easily: */
/* Macros for generating declarations: */
#define CONG_EDITOR_NODE_DECLARE_SUBCLASS(SubclassName, subclass_name) \
  typedef struct CongEditorNode##SubclassName CongEditorNode##SubclassName; \
  typedef struct CongEditorNode##SubclassName##Class CongEditorNode##SubclassName##Class; \
  typedef struct CongEditorNode##SubclassName##Private CongEditorNode##SubclassName##Private; \
  struct CongEditorNode##SubclassName \
  { \
	  CongEditorNode node; \
	  CongEditorNode##SubclassName##Private *priv; \
  }; \
  struct CongEditorNode##SubclassName##Class \
  { \
	  CongEditorNodeClass klass; \
  }; \
  GType \
  cong_editor_node_##subclass_name##_get_type (void); \
  CongEditorNode##SubclassName * \
  cong_editor_node_##subclass_name##_construct (CongEditorNode##SubclassName *editor_node_##subclass_name, \
						CongEditorWidget3* widget, \
						CongTraversalNode *traversal_node); \
  CongEditorNode* \
  cong_editor_node_##subclass_name##_new (CongEditorWidget3* widget, \
					  CongTraversalNode *traversal_node);

/* Macros for generating implementations: */
/* Submacros: */
#if 1
#define CONG_EDITOR_NODE_DECLARE_HOOKS(subclass_name) \
        static void \
        subclass_name##_create_areas (CongEditorNode *editor_node, \
	                              const CongAreaCreationInfo *creation_info); \
        \
	static gboolean \
	subclass_name##_needs_area_regeneration (CongEditorNode *editor_node, \
				                const CongAreaCreationGeometry *old_creation_geometry, \
				                const CongAreaCreationGeometry *new_creation_geometry);

#define CONG_EDITOR_NODE_CONNECT_HOOKS(subclass_name) \
    {  \
	CongEditorNodeClass *node_klass = CONG_EDITOR_NODE_CLASS(klass); \
	node_klass->create_areas = subclass_name##_create_areas; \
	node_klass->needs_area_regeneration = subclass_name##_needs_area_regeneration; \
    }

#else

#define CONG_EDITOR_NODE_DECLARE_HOOKS \
        static CongEditorArea* \
        generate_block_area (CongEditorNode *editor_node);

#define CONG_EDITOR_NODE_CONNECT_HOOKS \
	CongEditorNodeClass *node_klass = CONG_EDITOR_NODE_CLASS(klass); \
	node_klass->generate_block_area = generate_block_area;
#endif

#define CONG_EDITOR_NODE_IMPLEMENT_NEW(subclass_name) \
     CongEditorNode* \
     cong_editor_node_##subclass_name##_new (CongEditorWidget3 *widget, \
					     CongTraversalNode *traversal_node) \
     { \
	     return CONG_EDITOR_NODE( cong_editor_node_##subclass_name##_construct (g_object_new (cong_editor_node_##subclass_name##_get_type (), NULL), \
										    widget, \
										    traversal_node) \
				      ); \
     }

/* The macro that does it all: */
#define CONG_EDITOR_NODE_DEFINE_SUBCLASS(SubclassName, subclass_name, SUBCLASS_MACRO, PrivateData) \
     struct CongEditorNode##SubclassName##Private { \
	     PrivateData \
     }; \
     CONG_EDITOR_NODE_DECLARE_HOOKS(subclass_name) \
     CONG_DEFINE_CLASS_BEGIN(CongEditorNode##SubclassName, cong_editor_node_##subclass_name, SUBCLASS_MACRO, CongEditorNode, CONG_EDITOR_NODE_TYPE ) \
        CONG_EDITOR_NODE_CONNECT_HOOKS(subclass_name) \
     CONG_DEFINE_CLASS_END() \
     CONG_EDITOR_NODE_IMPLEMENT_NEW(subclass_name)
     
#define CONG_EDITOR_NODE_IMPLEMENT_DISPOSE_BEGIN(SubclassName, subclass_name, SUBCLASS_MACRO) \
     CONG_OBJECT_IMPLEMENT_DISPOSE_BEGIN(CongEditorNode##SubclassName, cong_editor_node_##subclass_name, SUBCLASS_MACRO, editor_node_##subclass_name)

#define CONG_EDITOR_NODE_IMPLEMENT_DISPOSE_END(subclass_name) \
     CONG_OBJECT_IMPLEMENT_DISPOSE_END(cong_editor_node_##subclass_name)

#define CONG_EDITOR_NODE_IMPLEMENT_EMPTY_DISPOSE(subclass_name) \
     CONG_DEFINE_EMPTY_DISPOSE(cong_editor_node_##subclass_name)

#define CONG_EDITOR_NODE_DEFINE_BLOCK_AREA_REGENERATION_HOOK(subclass_name) \
     static gboolean \
     subclass_name##_needs_area_regeneration (CongEditorNode *editor_node, \
			                        const CongAreaCreationGeometry *old_creation_geometry, \
			                        const CongAreaCreationGeometry *new_creation_geometry) \
     { \
	     /* Changes to creation geometry never force a regeneration of a block area: */ \
	     return FALSE; \
     }

#define CONG_EDITOR_NODE_DEFINE_EMPTY_AREA_REGENERATION_HOOK(subclass_name) \
     static gboolean \
     subclass_name##_needs_area_regeneration (CongEditorNode *editor_node, \
			                        const CongAreaCreationGeometry *old_creation_geometry, \
			                        const CongAreaCreationGeometry *new_creation_geometry) \
     { \
	     /* Empty areas never need regenerating: */ \
	     return FALSE; \
     }

#if 1
#define CONG_EDITOR_NODE_DEFINE_BLOCK_AREA_CREATION_HOOK(block_area_creation_function, subclass_name) \
      static void \
      subclass_name##_create_areas (CongEditorNode *editor_node,\
		                    const CongAreaCreationInfo *creation_info)\
      {\
      	CongEditorArea *block_area;\
      	g_return_if_fail (IS_CONG_EDITOR_NODE (editor_node));\
      	block_area = block_area_creation_function (cong_editor_node_get_widget (editor_node));\
      	cong_editor_node_create_block_area (editor_node,\
      					    creation_info,\
      					    block_area,\
      					    TRUE);\
      	/* FIXME: should we attach signals, or store the area anywhere? */\
      }
#else
#define CONG_EDITOR_NODE_DEFINE_BLOCK_AREA_CREATION_HOOK(block_area_creation_function) \
      static CongEditorArea*\
      generate_block_area (CongEditorNode *editor_node)\
      {\
      	CongEditorArea *new_area;\
      	g_return_val_if_fail (editor_node, NULL);\
      	new_area = block_area_creation_function (cong_editor_node_get_widget (editor_node));\
      	cong_editor_area_connect_node_signals (new_area,\
      					       editor_node);\
      	return new_area;\
      }
#endif

/* Macros for creating CongEditorNodeElement subclasses: */
#define CONG_EDITOR_NODE_DECLARE_PLUGIN_SUBCLASS(SubclassName, subclass_name) \
     CONG_EDITOR_NODE_DECLARE_SUBCLASS(Element##SubclassName, element_##subclass_name)

#define CONG_EDITOR_NODE_ELEMENT_DEFINE_EMPTY_CONSTRUCT(SubclassName, subclass_name) \
      CongEditorNodeElement##SubclassName* \
      cong_editor_node_element_##subclass_name##_construct (CongEditorNodeElement##SubclassName *editor_node_element_##subclass_name,\
      					                    CongEditorWidget3* editor_widget,\
      					                    CongTraversalNode *traversal_node)\
      {\
      	cong_editor_node_element_construct (CONG_EDITOR_NODE_ELEMENT (editor_node_element_##subclass_name),\
      					    editor_widget,\
      					    traversal_node);\
      	return editor_node_element_##subclass_name;\
      }

#define CONG_EDITOR_NODE_ELEMENT_DEFINE_SUBCLASS(SubclassName, subclass_name, SUBCLASS_MACRO, PrivateData) \
     struct CongEditorNodeElement##SubclassName##Private { \
	     PrivateData \
     }; \
     CONG_EDITOR_NODE_DECLARE_HOOKS(subclass_name) \
     CONG_DEFINE_CLASS_BEGIN(CongEditorNodeElement##SubclassName, cong_editor_node_element_##subclass_name, SUBCLASS_MACRO, CongEditorNodeElement, CONG_EDITOR_NODE_ELEMENT_TYPE ) \
        CONG_EDITOR_NODE_CONNECT_HOOKS(subclass_name) \
     CONG_DEFINE_CLASS_END() \
     CONG_EDITOR_NODE_IMPLEMENT_NEW(element_##subclass_name)

/* A macro to make it easier to create a plugin CongEditorNodeElement given a block area creation function: */
#define CONG_EDITOR_NODE_DEFINE_PLUGIN_SUBCLASS(SubclassName, subclass_name, SUBCLASS_MACRO, block_area_creation_function) \
     CONG_EDITOR_NODE_ELEMENT_DEFINE_SUBCLASS(SubclassName, subclass_name, SUBCLASS_MACRO, int dummy;) \
     CONG_EDITOR_NODE_ELEMENT_DEFINE_EMPTY_CONSTRUCT(SubclassName, subclass_name) \
     CONG_EDITOR_NODE_IMPLEMENT_EMPTY_DISPOSE(element_##subclass_name) \
     CONG_EDITOR_NODE_DEFINE_BLOCK_AREA_CREATION_HOOK(block_area_creation_function, subclass_name) \
     CONG_EDITOR_NODE_DEFINE_BLOCK_AREA_REGENERATION_HOOK(subclass_name)

#define CONG_EDITOR_NODE_DEFINE_BLOCK_AREA_CREATION_HOOK_SPECIAL(SubclassName, subclass_name, SUBCLASS_MACRO) \
      static CongEditorArea* \
      subclass_name##_create_block_area (CongEditorNodeElement##SubclassName *editor_node_element_##subclass_name); \
      static void \
      subclass_name##_create_areas (CongEditorNode *editor_node,\
		                    const CongAreaCreationInfo *creation_info)\
      {\
      	CongEditorNodeElement##SubclassName *editor_node_element_##subclass_name = SUBCLASS_MACRO(editor_node);\
      	CongEditorArea *block_area;\
      	g_return_if_fail (editor_node);\
      	block_area = subclass_name##_create_block_area (editor_node_element_##subclass_name);\
      	cong_editor_node_create_block_area (editor_node,\
      					    creation_info,\
      					    block_area,\
      					    TRUE);\
      	/* FIXME: should we attach signals, or store the area anywhere? */\
      }

#define CONG_EDITOR_NODE_DEFINE_PLUGIN_SUBCLASS_SPECIAL(SubclassName, subclass_name, SUBCLASS_MACRO) \
     CONG_EDITOR_NODE_ELEMENT_DEFINE_SUBCLASS(SubclassName, subclass_name, SUBCLASS_MACRO, int dummy;) \
     CONG_EDITOR_NODE_ELEMENT_DEFINE_EMPTY_CONSTRUCT(SubclassName, subclass_name) \
     CONG_EDITOR_NODE_IMPLEMENT_EMPTY_DISPOSE(element_##subclass_name) \
     CONG_EDITOR_NODE_DEFINE_BLOCK_AREA_CREATION_HOOK_SPECIAL(SubclassName, subclass_name, SUBCLASS_MACRO) \
     CONG_EDITOR_NODE_DEFINE_BLOCK_AREA_REGENERATION_HOOK(subclass_name)

G_END_DECLS

#endif
