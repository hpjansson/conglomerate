/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-widget3-impl.h
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


/*
 * Implementation details for the CongEditorWidget
 */
#ifndef __CONG_EDITOR_WIDGET3_IMPL_H__
#define __CONG_EDITOR_WIDGET3_IMPL_H__

#include <gtk/gtkenums.h>
#include "cong-editor-area.h"
#include "cong-editor-node.h"

G_BEGIN_DECLS

/* currently implemented as a GtkDrawingArea with user_data "details" pointing to a CongEditorWidget3Details */

/*
  Classes: 
  CongEditorArea:
  ---------------
  Represents a visible area of the widget.  Forms a hierarchy, although the implementation is delegated to subclasses.
  Base class handles hit testing, can do show/hide etc.
  A parent's rectangular area fully encloses that of all of its children, so we can quickly cull branches of the tree when rendering and doing hit testing.

  CongEditorNode:
  ---------------
  Represents a node in the document.  1-1 mapping.  Not directly visible.  Subclasses are responsible for managing a collection of CongEditorAreas.
  Stores everything relating to the node for this widget.
  Note that all nodes are represented, including the text nodes, etc etc...
*/


#if 0
void
cong_editor_element_generate_areas (CongEditorNode *editor_node,
				    CongEditorArea *editor_area_parent);
#endif

#if 0
void cong_element_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect);
#endif

typedef struct CongEditorWidget3Details CongEditorWidget3Details;

/* Macro for getting details of a widget; this will eventually be a simple field lookup */
#define GET_DETAILS(editor_widget) ((CongEditorWidget3Details*)(g_object_get_data(G_OBJECT(editor_widget), "details")))


/* Methods on CongEditorNode: */
typedef void 
(*CongEditorAreaCallback) (CongEditorArea *area,
			   gpointer user_data);

void
cong_editor_node_for_each_area (CongEditorNode *editor_node,
				CongEditorAreaCallback callback,	   
				gpointer user_data);

/* Manufacture a debug editor node: */
CongEditorNode*
cong_editor_node_dummy_new (CongEditorWidget3 *widget, 
			    CongNodePtr node, 
			    const gchar* message);

/* Manufacture an appropriate editor node: */
CongEditorNode*
cong_editor_node_new (CongEditorWidget3 *widget, 
		      CongNodePtr node);

/* Constructors for different types of concrete CongEditorNode subclasses: */
CongEditorNode*
cong_editor_node_unknown_tag_new (CongEditorWidget3 *widget, 
				  CongNodePtr node);

CongEditorNode*
cong_editor_node_structural_tag_new (CongEditorWidget3 *widget, 
				     CongNodePtr node);

CongEditorNode*
cong_editor_node_span_tag_new (CongEditorWidget3 *widget, 
			       CongNodePtr node);

CongEditorNode*
cong_editor_node_text_new (CongEditorWidget3 *widget, 
			   CongNodePtr node);

CongEditorNode*
cong_editor_node_comment_new (CongEditorWidget3 *widget, 
			      CongNodePtr node);

G_END_DECLS

#endif
