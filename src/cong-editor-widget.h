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

G_BEGIN_DECLS

/* Third attempt at an editor widget: */

enum CongFlowType
{
	CONG_FLOW_TYPE_BLOCK,
	CONG_FLOW_TYPE_INLINE
};

#define CONG_EDITOR_WIDGET3_TYPE	 (cong_editor_widget3_get_type ())
#define CONG_EDITOR_WIDGET3(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_WIDGET3_TYPE, CongEditorWidget3)
#define CONG_EDITOR_WIDGET3_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_WIDGET3_TYPE, CongEditorWidget3Class)
#define IS_CONG_EDITOR_WIDGET3(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_WIDGET3_TYPE)

typedef struct CongEditorWidget3 CongEditorWidget3;
typedef struct CongEditorWidget3Class CongEditorWidget3Class;
typedef struct CongEditorWidget3Details CongEditorWidget3Details;

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
typedef struct CongEditorNodeClass CongEditorNodeClass;

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
cong_editor_widget_get_type (void);

CongEditorWidget3*
cong_editor_widget_construct (CongEditorWidget3 *editor_widget,
			      CongDocument *doc);

GtkWidget*
cong_editor_widget3_new(CongDocument *doc);

CongDocument*
cong_editor_widget3_get_document(CongEditorWidget3 *editor_widget);

CongDispspec*
cong_editor_widget3_get_dispspec(CongEditorWidget3 *editor_widget);

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
cong_editor_widget3_get_editor_node (CongEditorWidget3 *editor_widget,
				     CongNodePtr node,
				     CongEditorNode *traversal_parent);

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
cong_flow_type_get_debug_string(enum CongFlowType flow_type);

G_END_DECLS

#endif
