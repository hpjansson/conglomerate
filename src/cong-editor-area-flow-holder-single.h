/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-flow-holder-single.h
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

#ifndef __CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE_H__
#define __CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE_H__

#include "cong-editor-area-flow-holder.h"

G_BEGIN_DECLS

typedef struct CongEditorAreaFlowHolderSingle CongEditorAreaFlowHolderSingle;
typedef struct CongEditorAreaFlowHolderSingleClass CongEditorAreaFlowHolderSingleClass;
typedef struct CongEditorAreaFlowHolderSingleDetails CongEditorAreaFlowHolderSingleDetails;

#define CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE_TYPE	   (cong_editor_area_flow_holder_single_get_type ())
#define CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE_TYPE, CongEditorAreaFlowHolderSingle)
#define CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE_TYPE, CongEditorAreaFlowHolderSingleClass)
#define IS_CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_FLOW_HOLDER_SINGLE_TYPE)

struct CongEditorAreaFlowHolderSingle
{
	CongEditorAreaFlowHolder flow_holder;

	CongEditorAreaFlowHolderSingleDetails *private;
};

struct CongEditorAreaFlowHolderSingleClass
{
	CongEditorAreaFlowHolderClass klass;
};

GType
cong_editor_area_flow_holder_single_get_type (void);

CongEditorArea*
cong_editor_area_flow_holder_single_construct (CongEditorAreaFlowHolderSingle *area_flow_holder_single,
					       CongEditorWidget3 *editor_widget);

CongEditorArea*
cong_editor_area_flow_holder_single_new (CongEditorWidget3 *editor_widget);

G_END_DECLS

#endif
