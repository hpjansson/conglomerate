/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-border.c
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
#include "cong-editor-area-border.h"
#include <libgnome/gnome-macros.h>

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaBorderDetails
{
	guint pixels;
};

/* Method implementation prototypes: */
static void 
update_requisition (CongEditorArea *area, 
		    int width_hint);

static void
allocate_child_space (CongEditorArea *area);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaBorder, 
			cong_editor_area_border,
			CongEditorAreaBin,
			CONG_EDITOR_AREA_BIN_TYPE );

static void
cong_editor_area_border_class_init (CongEditorAreaBorderClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);

	area_klass->update_requisition = update_requisition;
	area_klass->allocate_child_space = allocate_child_space;
}

static void
cong_editor_area_border_instance_init (CongEditorAreaBorder *area_border)
{
	area_border->private = g_new0(CongEditorAreaBorderDetails,1);
}

/* Exported function definitions: */
CongEditorArea*
cong_editor_area_border_construct (CongEditorAreaBorder *area_border,
				   CongEditorWidget3 *editor_widget,
				   guint pixels)     
{
	cong_editor_area_bin_construct (CONG_EDITOR_AREA_BIN(area_border),
					editor_widget);
	
	PRIVATE(area_border)->pixels = pixels;
	
	return CONG_EDITOR_AREA (area_border);
}

CongEditorArea*
cong_editor_area_border_new (CongEditorWidget3 *editor_widget,
			     guint pixels)
{
	g_message("cong_editor_area_border_new");

	return cong_editor_area_border_construct
		(g_object_new (CONG_EDITOR_AREA_BORDER_TYPE, NULL),
		 editor_widget,
		 pixels);
}

/* Method implementation definitions: */
static void 
update_requisition (CongEditorArea *area, 
		    int width_hint)
{
	const GtkRequisition *child_req = NULL;

	CongEditorAreaBorder *border = CONG_EDITOR_AREA_BORDER(area);
	CongEditorAreaBin *bin = CONG_EDITOR_AREA_BIN(area);
	CongEditorArea *child;

	child = cong_editor_area_bin_get_child(bin);

	if (child) {

		cong_editor_area_update_requisition (child, 
						     width_hint);
		
		child_req = cong_editor_area_get_requisition (child);
		g_assert(child_req);
		
		cong_editor_area_set_requisition (area,
						  child_req->width + (2 * PRIVATE(border)->pixels),
						  child_req->height + (2 * PRIVATE(border)->pixels));
	} else {
		cong_editor_area_set_requisition (area,
						  (2 * PRIVATE(border)->pixels),
						  (2 * PRIVATE(border)->pixels));
	}
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaBorder *border = CONG_EDITOR_AREA_BORDER(area);
	CongEditorArea *child;

	child = cong_editor_area_bin_get_child (CONG_EDITOR_AREA_BIN(area));

	if (child) {
		const GdkRectangle *rect = cong_editor_area_get_window_coords(area);

		cong_editor_area_set_allocation (child,
						 rect->x + PRIVATE(border)->pixels,
						 rect->y + PRIVATE(border)->pixels,
						 rect->width - (2*PRIVATE(border)->pixels),
						 rect->height - (2*PRIVATE(border)->pixels));
	}
}
