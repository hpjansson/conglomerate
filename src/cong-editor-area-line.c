/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-line.c
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
#include "cong-editor-area-line.h"
#include <libgnome/gnome-macros.h>

#include "cong-editor-area-composer.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaLineDetails
{
	CongEditorAreaComposer *outer_compose;

	gint width_limit;
};

/* Method implementation prototypes: */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint);

static void
allocate_child_space (CongEditorArea *area);

static CongEditorArea*
for_all (CongEditorArea *editor_area, 
	 CongEditorAreaCallbackFunc func, 
	 gpointer user_data);

static void
add_child (CongEditorAreaContainer *area_container,
	   CongEditorArea *child);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaLine, 
			cong_editor_area_line,
			CongEditorAreaContainer,
			CONG_EDITOR_AREA_CONTAINER_TYPE );

static void
cong_editor_area_line_class_init (CongEditorAreaLineClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);
	CongEditorAreaContainerClass *container_klass = CONG_EDITOR_AREA_CONTAINER_CLASS(klass);

	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
	area_klass->for_all = for_all;

	container_klass->add_child = add_child;
}

static void
cong_editor_area_line_instance_init (CongEditorAreaLine *line)
{
	line->private = g_new0(CongEditorAreaLineDetails,1);
}

/* Exported function definitions: */
CongEditorArea*
cong_editor_area_line_construct (CongEditorAreaLine *line,
				 CongEditorWidget3 *editor_widget,
				 gint width_limit)
{
	cong_editor_area_flow_holder_construct (CONG_EDITOR_AREA(line),
						editor_widget);

	PRIVATE(line)->outer_compose = CONG_EDITOR_AREA_COMPOSER( cong_editor_area_composer_new (editor_widget,
												 GTK_ORIENTATION_HORIZONTAL,
												 0));
	PRIVATE(line)->width_limit = width_limit;

	cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (line),
								   CONG_EDITOR_AREA (PRIVATE(line)->outer_compose));

	cong_editor_area_protected_set_parent (CONG_EDITOR_AREA (PRIVATE(line)->outer_compose),
					       CONG_EDITOR_AREA (line));

	return CONG_EDITOR_AREA (line);
}

CongEditorArea*
cong_editor_area_line_new (CongEditorWidget3 *editor_widget,
			   gint width_limit)

{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_line_new");
#endif

	return cong_editor_area_line_construct
		(g_object_new (CONG_EDITOR_AREA_LINE_TYPE, NULL),
		 editor_widget,
		 width_limit);
}

gint
cong_editor_area_line_get_width_limit (CongEditorAreaLine *area_line)
{
	g_return_val_if_fail (IS_CONG_EDITOR_AREA_LINE (area_line), 0);

	return PRIVATE(area_line)->width_limit;
}

gint
cong_editor_area_line_get_width_used (CongEditorAreaLine *area_line)
{
	g_return_val_if_fail (IS_CONG_EDITOR_AREA_LINE (area_line), 0);

	/* Get the delegated requisition: */
	return  cong_editor_area_get_requisition_width (CONG_EDITOR_AREA(PRIVATE(area_line)->outer_compose),
							PRIVATE(area_line)->width_limit);
}

gint
cong_editor_area_line_get_width_free (CongEditorAreaLine *area_line)
{
	gint width_limit;
	gint width_used;

	g_return_val_if_fail (IS_CONG_EDITOR_AREA_LINE (area_line), 0);

	width_limit = cong_editor_area_line_get_width_limit (area_line);
	width_used = cong_editor_area_line_get_width_used (area_line);

	g_message ("Line has width_limit = %i, of which %i is used", width_limit, width_used);

	return width_limit - width_used;
}

/* Method implementation definitions: */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	CongEditorAreaLine *line = CONG_EDITOR_AREA_LINE(area);

#if 1
	if (GTK_ORIENTATION_HORIZONTAL==orientation) {
		return PRIVATE(line)->width_limit;
	} else {
		return  cong_editor_area_get_requisition (CONG_EDITOR_AREA(PRIVATE(line)->outer_compose),
							  orientation,
							  width_hint);
	}
#else
	if (PRIVATE(line)->outer_compose) {
		return  cong_editor_area_get_requisition (CONG_EDITOR_AREA(PRIVATE(line)->outer_compose),
							  orientation,
							  width_hint);
	} else {
		return 0;
	}
#endif
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaLine *line = CONG_EDITOR_AREA_LINE(area);

	if (PRIVATE(line)->outer_compose) {
		const GdkRectangle *rect = cong_editor_area_get_window_coords(area);

		cong_editor_area_set_allocation (CONG_EDITOR_AREA(PRIVATE(line)->outer_compose),
						 rect->x,
						 rect->y,
						 rect->width,
						 rect->height);
	}
}

static CongEditorArea*
for_all (CongEditorArea *editor_area, 
	 CongEditorAreaCallbackFunc func, 
	 gpointer user_data)
{
	CongEditorAreaLine *line = CONG_EDITOR_AREA_LINE(editor_area);

	if (PRIVATE(line)->outer_compose) {
		if ((*func)(CONG_EDITOR_AREA(PRIVATE(line)->outer_compose), user_data)) {
			return CONG_EDITOR_AREA(PRIVATE(line)->outer_compose);
		}
	}

	return NULL;
}

static void
add_child (CongEditorAreaContainer *area_container,
	   CongEditorArea *child)
{
	CongEditorAreaLine *line = CONG_EDITOR_AREA_LINE(area_container);

	/* Delegate: */
	cong_editor_area_composer_pack(CONG_EDITOR_AREA_COMPOSER(PRIVATE(line)->outer_compose),
				       child,
				       FALSE,
				       FALSE,
				       0);
}
