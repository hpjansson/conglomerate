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
#include "cong-editor-area-container.h"

#define DEBUG_LINE 0

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
	   CongEditorArea *child,
	   gboolean add_to_end);

static void
add_child_after (CongEditorAreaContainer *area_container,
		 CongEditorArea *child,
		 CongEditorArea *relative_to);

static void
remove_child ( CongEditorAreaContainer *area_container,
	       CongEditorArea *child);

static gboolean 
set_to_not_expand_cb (CongEditorArea *editor_area, 
		      gpointer user_data);

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
	container_klass->add_child_after = add_child_after;
	container_klass->remove_child = remove_child;
}

static void
cong_editor_area_line_instance_init (CongEditorAreaLine *line)
{
	line->private = g_new0(CongEditorAreaLineDetails,1);
}

/* Exported function definitions: */
/**
 * cong_editor_area_line_construct:
 * @area_line:
 * @editor_widget:
 * @width_limit:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_line_construct (CongEditorAreaLine *line,
				 CongEditorWidget3 *editor_widget,
				 gint width_limit)
{
	cong_editor_area_container_construct (CONG_EDITOR_AREA_CONTAINER(line),
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
/**
 * cong_editor_area_line_new:
 * @editor_widget:
 * @width_limit:
 *
 * TODO: Write me
 * Returns:
 */
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

/**
 * cong_editor_area_line_get_width_limit:
 * @area_line:
 *
 * TODO: Write me
 * Returns:
 */
gint
cong_editor_area_line_get_width_limit (CongEditorAreaLine *area_line)
{
	g_return_val_if_fail (IS_CONG_EDITOR_AREA_LINE (area_line), 0);

	return PRIVATE(area_line)->width_limit;
}

/**
 * cong_editor_area_line_get_width_used:
 * @area_line:
 *
 * TODO: Write me
 * Returns:
 */
gint
cong_editor_area_line_get_width_used_up_to (CongEditorAreaLine *area_line,
					    CongEditorArea *child_area)
{
	g_return_val_if_fail (IS_CONG_EDITOR_AREA_LINE (area_line), 0);

	if (child_area) {
		GList *iter;
		gint width = 0;

		g_return_val_if_fail (IS_CONG_EDITOR_AREA (child_area), 0);

		iter = cong_editor_area_composer_get_child_area_iter_first (PRIVATE (area_line)->outer_compose);

		while (iter) {
			CongEditorArea *iter_child_area = cong_editor_area_composer_get_child_area (PRIVATE (area_line)->outer_compose,
												    iter);
			g_assert (iter_child_area);

			width += cong_editor_area_get_requisition_width (iter_child_area,
									 PRIVATE(area_line)->width_limit);
			
			if (child_area==iter_child_area) {
				return width;
			}

			iter=iter->next;
		}

		g_error ("child_area not found in CongEditorAreaLine");
		return 0;
	} else {
		return 0;
	}
}

gint
cong_editor_area_line_get_width_used (CongEditorAreaLine *area_line)
{
	g_return_val_if_fail (IS_CONG_EDITOR_AREA_LINE (area_line), 0);

	/* Get the delegated requisition: */
	return  cong_editor_area_get_requisition_width (CONG_EDITOR_AREA(PRIVATE(area_line)->outer_compose),
							PRIVATE(area_line)->width_limit);
}

/**
 * cong_editor_area_line_get_width_free:
 * @area_line:
 *
 * TODO: Write me
 * Returns:
 */
gint
cong_editor_area_line_get_width_free (CongEditorAreaLine *area_line)
{
	gint width_limit;
	gint width_used;

	g_return_val_if_fail (IS_CONG_EDITOR_AREA_LINE (area_line), 0);

	width_limit = cong_editor_area_line_get_width_limit (area_line);
	width_used = cong_editor_area_line_get_width_used (area_line);

#if DEBUG_LINE
	g_message ("Line has width_limit = %i, of which %i is used", width_limit, width_used);
#endif

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
		return cong_editor_area_get_requisition (CONG_EDITOR_AREA(PRIVATE(line)->outer_compose),
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
	   CongEditorArea *child,
	   gboolean add_to_end)
{
	CongEditorAreaLine *line = CONG_EDITOR_AREA_LINE(area_container);

	/* We delegate to the outer_compose child: */

	/* Ensure all existing children of the line don't expand: */
	cong_editor_area_for_all (CONG_EDITOR_AREA (PRIVATE(line)->outer_compose), 
				  set_to_not_expand_cb, 
				  PRIVATE(line)->outer_compose);
	
	/* Make the new child expand to fill to the end of the line: */
	if (add_to_end) {
		cong_editor_area_composer_pack_end (CONG_EDITOR_AREA_COMPOSER (PRIVATE(line)->outer_compose),
						    child,
						    TRUE,
						    TRUE,
						    0);
	} else {
		cong_editor_area_composer_pack_start (CONG_EDITOR_AREA_COMPOSER (PRIVATE(line)->outer_compose),
						      child,
						      TRUE,
						      TRUE,
						      0);
	}
}	

static void
add_child_after (CongEditorAreaContainer *area_container,
		 CongEditorArea *child,
		 CongEditorArea *relative_to)
{
	CongEditorAreaLine *line = CONG_EDITOR_AREA_LINE(area_container);

	/* Delegate, set not to expand: */
	cong_editor_area_composer_pack_after (CONG_EDITOR_AREA_COMPOSER (PRIVATE(line)->outer_compose),
					      child,
					      relative_to,
					      FALSE,
					      FALSE,
					      0);
}

static void
remove_child ( CongEditorAreaContainer *area_container,
	       CongEditorArea *child)
{
	CongEditorAreaLine *line = CONG_EDITOR_AREA_LINE(area_container);

	/* Delegate: */
	cong_editor_area_container_remove_child (CONG_EDITOR_AREA_CONTAINER (PRIVATE(line)->outer_compose),
						 child);
}

static gboolean 
set_to_not_expand_cb (CongEditorArea *editor_area, 
		      gpointer user_data)
{

	cong_editor_area_composer_set_child_packing (CONG_EDITOR_AREA_COMPOSER(user_data),
						     editor_area,
						     FALSE,
						     FALSE,
						     0);

	return FALSE;
}
