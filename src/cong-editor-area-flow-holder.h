/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-flow-holder.h
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

#ifndef __CONG_EDITOR_AREA_FLOW_HOLDER_H__
#define __CONG_EDITOR_AREA_FLOW_HOLDER_H__

#include "cong-editor-area.h"
#include "cong-editor-node.h"

G_BEGIN_DECLS

typedef struct CongEditorAreaFlowHolder CongEditorAreaFlowHolder;
typedef struct CongEditorAreaFlowHolderClass CongEditorAreaFlowHolderClass;
typedef struct CongEditorAreaFlowHolderDetails CongEditorAreaFlowHolderDetails;

#define CONG_EDITOR_AREA_FLOW_HOLDER_TYPE	   (cong_editor_area_flow_holder_get_type ())
#define CONG_EDITOR_AREA_FLOW_HOLDER(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_FLOW_HOLDER_TYPE, CongEditorAreaFlowHolder)
#define CONG_EDITOR_AREA_FLOW_HOLDER_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_FLOW_HOLDER_TYPE, CongEditorAreaFlowHolderClass)
#define IS_CONG_EDITOR_AREA_FLOW_HOLDER(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_FLOW_HOLDER_TYPE)

struct CongEditorAreaFlowHolder
{
	CongEditorArea area;

	CongEditorAreaFlowHolderDetails *private;
};

struct CongEditorAreaFlowHolderClass
{
	CongEditorAreaClass klass;

	CongEditorArea* (*insert_areas_for_node) (CongEditorAreaFlowHolder *area_flow_holder,
						  CongEditorNode *node);

	void (*remove_areas_for_node) (CongEditorAreaFlowHolder *area_flow_holder,
				       CongEditorNode *node);
};

GType
cong_editor_area_flow_holder_get_type (void);

CongEditorArea*
cong_editor_area_flow_holder_construct (CongEditorAreaFlowHolder *area_flow_holder,
					CongEditorWidget3 *editor_widget);

CongEditorArea*
cong_editor_area_flow_holder_insert_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
						    CongEditorNode *node);
void
cong_editor_area_flow_holder_remove_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
						    CongEditorNode *node);


CongEditorAreaFlowHolder*
cong_editor_area_flow_holder_manufacture (CongEditorWidget3 *editor_widget,
					  enum CongFlowType flow_type);


G_END_DECLS

#endif
