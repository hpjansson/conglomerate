/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-flow-holder.c
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
#include "cong-editor-area-flow-holder.h"
#include "cong-editor-area-flow-holder-blocks.h"
#include "cong-editor-area-flow-holder-inlines.h"
#include <libgnome/gnome-macros.h>

#include "cong-editor-area-composer.h"

#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaFlowHolderDetails
{
	int dummy;
};


/* Method implementation prototypes: */

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaFlowHolder, 
			cong_editor_area_flow_holder,
			CongEditorArea,
			CONG_EDITOR_AREA_TYPE );

CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_area_flow_holder, insert_areas_for_node);
CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_area_flow_holder, remove_areas_for_node);

static void
cong_editor_area_flow_holder_class_init (CongEditorAreaFlowHolderClass *klass)
{
#if 0
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);
#endif
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_area_flow_holder,
					      insert_areas_for_node);
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_area_flow_holder,
					      remove_areas_for_node);
}

static void
cong_editor_area_flow_holder_instance_init (CongEditorAreaFlowHolder *area_flow_holder)
{
	area_flow_holder->private = g_new0(CongEditorAreaFlowHolderDetails,1);
}

/* Exported function definitions: */
/**
 * cong_editor_area_flow_holder_construct:
 * @area_flow_holder:
 * @editor_widget:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_flow_holder_construct (CongEditorAreaFlowHolder *area_flow_holder,
					CongEditorWidget3 *editor_widget)
{
	cong_editor_area_construct (CONG_EDITOR_AREA(area_flow_holder),
				    editor_widget);

	return CONG_EDITOR_AREA (area_flow_holder);
}

/**
 * cong_editor_area_flow_holder_insert_areas_for_node:
 * @area_flow_holder:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorChildPolicy*
cong_editor_area_flow_holder_insert_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
						    CongEditorNode *editor_node)
{
	return CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_AREA_FLOW_HOLDER_CLASS,
						       area_flow_holder,
						       insert_areas_for_node, 
						       (area_flow_holder, editor_node));

}

/**
 * cong_editor_area_flow_holder_remove_areas_for_node:
 * @area_flow_holder:
 * @node:
 *
 * TODO: Write me
 */
void
cong_editor_area_flow_holder_remove_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
						    CongEditorNode *editor_node)
{
	CONG_EEL_CALL_METHOD (CONG_EDITOR_AREA_FLOW_HOLDER_CLASS,
			      area_flow_holder,
			      remove_areas_for_node, 
			      (area_flow_holder, editor_node));
}

/**
 * cong_editor_area_flow_holder_manufacture:
 * @editor_widget:
 * @flow_type:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorAreaFlowHolder*
cong_editor_area_flow_holder_manufacture (CongEditorWidget3 *editor_widget,
					  CongFlowType flow_type)
{
	switch (flow_type) {
	default: g_assert_not_reached();
	case CONG_FLOW_TYPE_BLOCK: 
		return CONG_EDITOR_AREA_FLOW_HOLDER(cong_editor_area_flow_holder_blocks_new(editor_widget));
					   
	case CONG_FLOW_TYPE_INLINE:
		return CONG_EDITOR_AREA_FLOW_HOLDER(cong_editor_area_flow_holder_inlines_new(editor_widget));
	}
}
