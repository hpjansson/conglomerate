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
	guint left_pixels;
	guint right_pixels;
	guint top_pixels;
	guint bottom_pixels;
};

/* Method implementation prototypes: */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
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

	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
}

static void
cong_editor_area_border_instance_init (CongEditorAreaBorder *area_border)
{
	area_border->private = g_new0(CongEditorAreaBorderDetails,1);
}

/* Exported function definitions: */
/**
 * cong_editor_area_border_construct:
 * @area_border:
 * @editor_widget:
 * @left_pixels:
 * @right_pixels:
 * @top_pixels:
 * @bottom_pixels:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_border_construct (CongEditorAreaBorder *area_border,
				   CongEditorWidget3 *editor_widget,
				   guint left_pixels,
				   guint right_pixels,
				   guint top_pixels,
				   guint bottom_pixels)
{
	cong_editor_area_bin_construct (CONG_EDITOR_AREA_BIN(area_border),
					editor_widget);
	
	PRIVATE(area_border)->left_pixels = left_pixels;
	PRIVATE(area_border)->right_pixels = right_pixels;
	PRIVATE(area_border)->top_pixels = top_pixels;
	PRIVATE(area_border)->bottom_pixels = bottom_pixels;
	
	return CONG_EDITOR_AREA (area_border);
}

/**
 * cong_editor_area_border_new:
 * @editor_widget:
 * @left_pixels:
 * @right_pixels:
 * @top_pixels:
 * @bottom_pixels:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_border_new (CongEditorWidget3 *editor_widget,
			     guint left_pixels,
			     guint right_pixels,
			     guint top_pixels,
			     guint bottom_pixels)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_border_new");
#endif
	return cong_editor_area_border_construct
		(g_object_new (CONG_EDITOR_AREA_BORDER_TYPE, NULL),
		 editor_widget,
		 left_pixels,
		 right_pixels,
		 top_pixels,
		 bottom_pixels);
}

/* Method implementation definitions: */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	CongEditorAreaBorder *border = CONG_EDITOR_AREA_BORDER(area);
	CongEditorAreaBin *bin = CONG_EDITOR_AREA_BIN(area);
	CongEditorArea *child;
	gint border_pixels;
	gint child_req;

	child = cong_editor_area_bin_get_child(bin);

	if (child) {
		child_req = cong_editor_area_get_requisition (child,
							      orientation,
							      width_hint);
	} else {
		child_req = 0;
	}

	if (orientation == GTK_ORIENTATION_HORIZONTAL) {
		border_pixels = PRIVATE(border)->left_pixels + PRIVATE(border)->right_pixels;
	} else {
		border_pixels = PRIVATE(border)->top_pixels + PRIVATE(border)->bottom_pixels;
	}

	return child_req + border_pixels;
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
						 rect->x + PRIVATE(border)->left_pixels,
						 rect->y + PRIVATE(border)->top_pixels,
						 rect->width - (PRIVATE(border)->left_pixels + PRIVATE(border)->right_pixels),
						 rect->height - (PRIVATE(border)->top_pixels + PRIVATE(border)->bottom_pixels));
	}
}
