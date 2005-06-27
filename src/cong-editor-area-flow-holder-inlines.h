/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-flow-holder-inlines.h
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

#ifndef __CONG_EDITOR_AREA_FLOW_HOLDER_INLINES_H__
#define __CONG_EDITOR_AREA_FLOW_HOLDER_INLINES_H__

#include "cong-editor-area-flow-holder.h"

G_BEGIN_DECLS

typedef struct _CongEditorAreaFlowHolderInlinesDetails CongEditorAreaFlowHolderInlinesDetails;

#define CONG_EDITOR_AREA_FLOW_HOLDER_INLINES_TYPE	   (cong_editor_area_flow_holder_inlines_get_type ())
#define CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_FLOW_HOLDER_INLINES_TYPE, CongEditorAreaFlowHolderInlines)
#define CONG_EDITOR_AREA_FLOW_HOLDER_INLINES_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_FLOW_HOLDER_INLINES_TYPE, CongEditorAreaFlowHolderInlinesClass)
#define IS_CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_FLOW_HOLDER_INLINES_TYPE)

struct _CongEditorAreaFlowHolderInlines
{
	CongEditorAreaFlowHolder flow_holder;

	CongEditorAreaFlowHolderInlinesDetails *private;
};

struct _CongEditorAreaFlowHolderInlinesClass
{
	CongEditorAreaFlowHolderClass klass;

};

GType
cong_editor_area_flow_holder_inlines_get_type (void);

CongEditorArea*
cong_editor_area_flow_holder_inlines_construct (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines,
						CongEditorWidget3 *editor_widget);

CongEditorArea*
cong_editor_area_flow_holder_inlines_new (CongEditorWidget3 *editor_widget);

CongEditorAreaLine*
cong_editor_area_flow_holder_inlines_get_current_line (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines);

gint
cong_editor_area_flow_holder_inlines_get_line_width (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines);

gint
cong_editor_area_flow_holder_inlines_get_current_indent (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines);

void
cong_editor_area_flow_holder_inlines_destroy_lines (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines);

CongEditorAreaLine*
cong_editor_area_flow_holder_inlines_insert_line (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines);

void
cong_editor_area_flow_holder_inlines_reflow_required (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines);

CongEditorNode*
cong_editor_area_flow_holder_inlines_get_first_node (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines);

CongEditorNode*
cong_editor_area_flow_holder_inlines_get_final_node (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines);

#if 0
gboolean
cong_editor_area_flow_holder_inlines_contains_editor_node_directly (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines,
								    CongEditorNode *editor_node);
#endif

G_END_DECLS

#endif
