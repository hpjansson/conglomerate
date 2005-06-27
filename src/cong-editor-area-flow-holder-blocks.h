/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-flow-holder-blocks.h
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

#ifndef __CONG_EDITOR_AREA_FLOW_HOLDER_BLOCKS_H__
#define __CONG_EDITOR_AREA_FLOW_HOLDER_BLOCKS_H__

#include "cong-editor-area-flow-holder.h"

G_BEGIN_DECLS

typedef struct _CongEditorAreaFlowHolderBlocks CongEditorAreaFlowHolderBlocks;
typedef struct _CongEditorAreaFlowHolderBlocksClass CongEditorAreaFlowHolderBlocksClass;
typedef struct _CongEditorAreaFlowHolderBlocksDetails CongEditorAreaFlowHolderBlocksDetails;

#define CONG_EDITOR_AREA_FLOW_HOLDER_BLOCKS_TYPE	   (cong_editor_area_flow_holder_blocks_get_type ())
#define CONG_EDITOR_AREA_FLOW_HOLDER_BLOCKS(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_FLOW_HOLDER_BLOCKS_TYPE, CongEditorAreaFlowHolderBlocks)
#define CONG_EDITOR_AREA_FLOW_HOLDER_BLOCKS_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_FLOW_HOLDER_BLOCKS_TYPE, CongEditorAreaFlowHolderBlocksClass)
#define IS_CONG_EDITOR_AREA_FLOW_HOLDER_BLOCKS(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_FLOW_HOLDER_BLOCKS_TYPE)

struct _CongEditorAreaFlowHolderBlocks
{
	CongEditorAreaFlowHolder flow_holder;

	CongEditorAreaFlowHolderBlocksDetails *private;
};

struct _CongEditorAreaFlowHolderBlocksClass
{
	CongEditorAreaFlowHolderClass klass;
};

GType
cong_editor_area_flow_holder_blocks_get_type (void);

CongEditorArea*
cong_editor_area_flow_holder_blocks_construct (CongEditorAreaFlowHolderBlocks *area_flow_holder_blocks,
					       CongEditorWidget3 *editor_widget);

CongEditorArea*
cong_editor_area_flow_holder_blocks_new (CongEditorWidget3 *editor_widget);

CongEditorAreaFlowHolder*
cong_editor_area_flow_holder_get_child_flow_holder_for_node (CongEditorAreaFlowHolderBlocks *area_flow_holder_blocks,
							     CongNodePtr doc_node);

#if 0
CongEditorAreaFlowHolder*
cong_editor_area_flow_holder_get_previous_child_flow_holder (CongEditorAreaFlowHolderBlocks *area_flow_holder_blocks,
							     CongEditorAreaFlowHolder *child_flow_holder);
#endif

G_END_DECLS

#endif
