/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-widget.h
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

#ifndef __CONG_EDITOR_WIDGET_H__
#define __CONG_EDITOR_WIDGET_H__

#include "cong-document.h"

G_BEGIN_DECLS

/* Third attempt at an editor widget: */

#define CONG_EDITOR_WIDGET3_TYPE	 (cong_editor_widget3_get_type ())
#define CONG_EDITOR_WIDGET3(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_WIDGET3_TYPE, CongEditorWidget3)
#define CONG_EDITOR_WIDGET3_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_WIDGET3_TYPE, CongEditorWidget3Class)
#define IS_CONG_EDITOR_WIDGET3(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_WIDGET3_TYPE)

typedef struct CongEditorWidget3 CongEditorWidget3;
typedef struct CongEditorWidget3Class CongEditorWidget3Class;
typedef struct CongEditorWidget3Details CongEditorWidget3Details;

typedef struct CongEditorLineIter CongEditorLineIter;
typedef struct CongEditorLineManager CongEditorLineManager;
typedef struct CongEditorLineManagerSimple CongEditorLineManagerSimple;
typedef struct CongEditorLineManagerSpanWrapper CongEditorLineManagerSpanWrapper;
typedef struct CongEditorCreationRecord CongEditorCreationRecord;

/* CongEditorArea and some common subclasses: */
typedef struct CongEditorArea CongEditorArea;
typedef struct CongEditorAreaClass CongEditorAreaClass;

typedef struct CongEditorAreaFlowHolder CongEditorAreaFlowHolder;
typedef struct CongEditorAreaFlowHolderClass CongEditorAreaFlowHolderClass;

typedef struct CongEditorAreaFlowHolderInlines CongEditorAreaFlowHolderInlines;
typedef struct CongEditorAreaFlowHolderInlinesClass CongEditorAreaFlowHolderInlinesClass;

typedef struct CongEditorAreaLine CongEditorAreaLine;
typedef struct CongEditorAreaLineClass CongEditorAreaLineClass;

/* CongEditorNode and some common subclasses: */
typedef struct CongEditorNode CongEditorNode;

typedef struct CongEditorNodeElement CongEditorNodeElement;
typedef struct CongEditorNodeElementClass CongEditorNodeElementClass;

/* Other related classes: */
typedef struct CongEditorLineFragments CongEditorLineFragments;
typedef struct CongEditorLineFragmentsClass CongEditorLineFragmentsClass;

typedef struct CongEditorChildPolicy CongEditorChildPolicy;
typedef struct CongEditorChildPolicyClass CongEditorChildPolicyClass;

struct CongEditorWidget3
{
	GtkDrawingArea drawing_area;

	CongEditorWidget3Details *private;
};

struct CongEditorWidget3Class
{
	GtkDrawingAreaClass klass;

	/* Methods? */
};

GType
cong_editor_widget3_get_type (void);

CongEditorWidget3*
cong_editor_widget3_construct (CongEditorWidget3 *editor_widget,
			       CongDocument *doc,
			       CongPrimaryWindow *primary_window);

GtkWidget*
cong_editor_widget3_new(CongDocument *doc,
			CongPrimaryWindow *primary_window);

CongDocument*
cong_editor_widget3_get_document(CongEditorWidget3 *editor_widget);

void 
cong_editor_widget3_force_layout_update (CongEditorWidget3 *editor_widget);

typedef void (*CongEditorNodeCallback) (CongEditorWidget3 *widget, 
					CongEditorNode *editor_node, 
					gpointer user_data);
void
cong_editor_widget3_for_each_editor_node (CongEditorWidget3 *widget,
					  CongNodePtr xml_node,
					  CongEditorNodeCallback editor_node_callback,
					  gpointer user_data);

CongEditorNode*
cong_editor_widget3_get_editor_node_for_traversal_node (CongEditorWidget3 *editor_widget,
							CongTraversalNode *traversal_node);

#if 0
CongEditorNode*
cong_editor_widget3_get_editor_node (CongEditorWidget3 *editor_widget,
				     CongNodePtr node,
				     CongEditorNode *traversal_parent);

CongEditorNode*
cong_editor_widget3_get_an_editor_node (CongEditorWidget3 *editor_widget,
					CongNodePtr node);
#endif

CongEditorArea*
cong_editor_widget3_get_prehighlight_editor_area (CongEditorWidget3 *editor_widget);

void
cong_editor_widget3_set_prehighlight_editor_area (CongEditorWidget3 *editor_widget,
						  CongEditorArea* editor_area);

GdkGC*
cong_editor_widget3_get_test_gc (CongEditorWidget3 *editor_widget);


/* Get the "deepest" area in the tree at the given coordinate (if any) 
 * Returns NULL if no area present.
 */
CongEditorArea*
cong_editor_widget3_get_area_at (CongEditorWidget3 *editor_widget,
				 gint x,
				 gint y);

const gchar*
cong_flow_type_get_debug_string(CongFlowType flow_type);

void 
editor_popup_build (CongEditorWidget3 *editor_widget, 
		    GtkWindow *parent_window);

void
cong_editor_widget3_add_popup_items (CongEditorWidget3 *editor_widget,
				     GtkMenu *menu);

void
cong_editor_widget3_get_preedit_data (CongEditorWidget3 *editor_widget,
				      gchar **output_string, 
				      PangoAttrList **output_pango_attr_list,
				      gint *output_cursor_pos);

/* This function exists so that the popup menu can be rebuilt the hackish way; it can disappear after the GtkAction code is completed */
CongPrimaryWindow*
cong_editor_widget_get_primary_window(CongEditorWidget3 *editor_widget);

G_END_DECLS

#endif
