/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-composer.c
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
#include "cong-editor-area-composer.h"
#include <libgnome/gnome-macros.h>

#define DEBUG_CHILD_ALLOCATIONS 0

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaComposerDetails
{
	GList *list_of_child_details;
	GtkOrientation orientation;
	guint spacing;
};


typedef struct CongEditorAreaComposerChildDetails CongEditorAreaComposerChildDetails;

struct CongEditorAreaComposerChildDetails
{
	CongEditorArea *child;
	gboolean expand;
	gboolean fill;
	guint extra_padding;
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
static void
add_child_after (CongEditorAreaContainer *area_container,
		 CongEditorArea *new_child,
		 CongEditorArea *relative_to);
static void
remove_child (CongEditorAreaContainer *area_container,
	      CongEditorArea *child);
static void
remove_all_children (CongEditorAreaContainer *area_container);

static CongEditorAreaComposerChildDetails*
get_child_details (CongEditorAreaComposer *area_composer,
		   CongEditorArea *child);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaComposer, 
			cong_editor_area_composer,
			CongEditorAreaContainer,
			CONG_EDITOR_AREA_CONTAINER_TYPE );

static void
cong_editor_area_composer_class_init (CongEditorAreaComposerClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);
	CongEditorAreaContainerClass *container_klass = CONG_EDITOR_AREA_CONTAINER_CLASS(klass);

	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
	area_klass->for_all = for_all;

	container_klass->add_child = add_child;
	container_klass->add_child_after = add_child_after;
	container_klass->remove_child = remove_child;
	container_klass->remove_all_children = remove_all_children;
}

static void
cong_editor_area_composer_instance_init (CongEditorAreaComposer *area_composer)
{
	area_composer->private = g_new0(CongEditorAreaComposerDetails,1);
}

/* Exported function definitions: */
CongEditorArea*
cong_editor_area_composer_construct (CongEditorAreaComposer *area_composer,
				     CongEditorWidget3 *editor_widget,
				     GtkOrientation orientation,
				     guint spacing)
{
	cong_editor_area_container_construct (CONG_EDITOR_AREA_CONTAINER(area_composer),
					      editor_widget);

	PRIVATE(area_composer)->orientation = orientation;
	PRIVATE(area_composer)->spacing = spacing;

	return CONG_EDITOR_AREA (area_composer);
}

CongEditorArea*
cong_editor_area_composer_new (CongEditorWidget3 *editor_widget,
			       GtkOrientation orientation,
			       guint spacing)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_composer_new");
#endif

	return cong_editor_area_composer_construct
		(g_object_new (CONG_EDITOR_AREA_COMPOSER_TYPE, NULL),
		 editor_widget,
		 orientation,
		 spacing);
}

void
cong_editor_area_composer_pack_start (CongEditorAreaComposer *area_composer,
				      CongEditorArea *child,
				      gboolean expand,
				      gboolean fill,
				      guint extra_padding)
{
	CongEditorAreaComposerChildDetails *child_details;

	g_return_if_fail (IS_CONG_EDITOR_AREA_COMPOSER(area_composer));
	g_return_if_fail (IS_CONG_EDITOR_AREA(child));

	g_object_ref (G_OBJECT(child));

	child_details = g_new0(CongEditorAreaComposerChildDetails,1);

	child_details->child = child;
	child_details->expand = expand;
	child_details->fill = fill;
	child_details->extra_padding = extra_padding;

	PRIVATE(area_composer)->list_of_child_details = g_list_prepend (PRIVATE(area_composer)->list_of_child_details, 
									child_details);

	cong_editor_area_container_protected_postprocess_add_non_internal_child (CONG_EDITOR_AREA_CONTAINER(area_composer),
										 child);

	cong_editor_area_protected_set_parent (child,
					       CONG_EDITOR_AREA(area_composer));

	cong_editor_area_container_children_changed ( CONG_EDITOR_AREA_CONTAINER(area_composer));
}

void
cong_editor_area_composer_pack_end (CongEditorAreaComposer *area_composer,
				    CongEditorArea *child,
				    gboolean expand,
				    gboolean fill,
				    guint extra_padding)
{
	CongEditorAreaComposerChildDetails *child_details;

	g_return_if_fail (IS_CONG_EDITOR_AREA_COMPOSER(area_composer));
	g_return_if_fail (IS_CONG_EDITOR_AREA(child));

	g_object_ref (G_OBJECT(child));

	child_details = g_new0(CongEditorAreaComposerChildDetails,1);

	child_details->child = child;
	child_details->expand = expand;
	child_details->fill = fill;
	child_details->extra_padding = extra_padding;

	PRIVATE(area_composer)->list_of_child_details = g_list_append (PRIVATE(area_composer)->list_of_child_details, 
								       child_details);

	cong_editor_area_container_protected_postprocess_add_non_internal_child (CONG_EDITOR_AREA_CONTAINER(area_composer),
										 child);

	cong_editor_area_protected_set_parent (child,
					       CONG_EDITOR_AREA(area_composer));

	cong_editor_area_container_children_changed ( CONG_EDITOR_AREA_CONTAINER(area_composer));
}

#if 0
static gint 
find_area (gconstpointer *a,
	   gconstpointer *b)
{
	CongEditorAreaComposerChildDetails *a_details;
	CongEditorAreaComposerChildDetails *b_details;

#if 1
	a_details = (CongEditorAreaComposerChildDetails *)a;
	b_details = (CongEditorAreaComposerChildDetails *)b;
#else
	a_details = (CongEditorAreaComposerChildDetails *)a->data;
	b_details = (CongEditorAreaComposerChildDetails *)b->data;
#endif

	return (a_details->child) - (b_details->child);
}
#endif

void
cong_editor_area_composer_pack_after (CongEditorAreaComposer *area_composer,
				      CongEditorArea *new_child,
				      CongEditorArea *ooh_relative_to,
				      gboolean expand,
				      gboolean fill,
				      guint extra_padding)
{
	GList *position = NULL;
	CongEditorAreaComposerChildDetails *child_details;
	
	g_return_if_fail (IS_CONG_EDITOR_AREA_COMPOSER(area_composer));
	g_return_if_fail (IS_CONG_EDITOR_AREA(new_child));
	g_return_if_fail (IS_CONG_EDITOR_AREA(ooh_relative_to));

	for (position = PRIVATE(area_composer)->list_of_child_details; position; position = position->next) {
		CongEditorAreaComposerChildDetails *position_details = (CongEditorAreaComposerChildDetails*)(position->data);
		if (position_details->child == ooh_relative_to) {
			break;
		}
	}

	if (NULL==position) {
		g_error("cong_editor_area_composer_pack_after: relative position not found");
	}

	g_object_ref (G_OBJECT(new_child));

	child_details = g_new0(CongEditorAreaComposerChildDetails,1);

	child_details->child = new_child;
	child_details->expand = expand;
	child_details->fill = fill;
	child_details->extra_padding = extra_padding;

	/* Don't store result; we don't need to update the head of the list: */
	g_list_insert_before (PRIVATE(area_composer)->list_of_child_details,
			      position->next,
			      child_details);
	
	cong_editor_area_container_protected_postprocess_add_non_internal_child (CONG_EDITOR_AREA_CONTAINER(area_composer),
										 new_child);

	cong_editor_area_protected_set_parent (new_child,
					       CONG_EDITOR_AREA(area_composer));

	cong_editor_area_container_children_changed ( CONG_EDITOR_AREA_CONTAINER(area_composer));
}

void 
cong_editor_area_composer_set_child_packing (CongEditorAreaComposer *area_composer,
					     CongEditorArea *child,
					     gboolean expand,
					     gboolean fill,
					     guint extra_padding)
{
	CongEditorAreaComposerChildDetails* child_details;

	g_return_if_fail (IS_CONG_EDITOR_AREA_COMPOSER(area_composer));
	g_return_if_fail (IS_CONG_EDITOR_AREA(child));

	child_details = get_child_details (area_composer,
					   child);

	g_return_if_fail (child_details);

	child_details->expand = expand;
	child_details->fill = fill;
	child_details->extra_padding = extra_padding;

	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA (area_composer),
						  PRIVATE(area_composer)->orientation);
	cong_editor_area_flush_requisition_cache (child,
						  PRIVATE(area_composer)->orientation);
}

/* Method implementation definitions: */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	CongEditorAreaComposer *area_composer = CONG_EDITOR_AREA_COMPOSER(area);
	gint result;
	GList *iter;
	int child_count = 0;
	gint extra_padding = 0;

	result = 0;

	if (orientation == PRIVATE(area_composer)->orientation) {
		/* Then we're getting the req along the "important" axis; it's the sum of all the kids plus any padding etc: */
		for (iter = PRIVATE(area_composer)->list_of_child_details; iter; iter=iter->next) {
			CongEditorAreaComposerChildDetails *child_details;
			CongEditorArea *child;
			guint child_req;
			
			child_details = (CongEditorAreaComposerChildDetails*)(iter->data);
			child = CONG_EDITOR_AREA(child_details->child);
			g_assert (child);
			
			extra_padding += child_details->extra_padding;
			
			child_req = cong_editor_area_get_requisition (child,
								      orientation,
								      width_hint);
			
			result += child_req;
			
			child_count++;
		}
		
		if (child_count>1) {
			result += (PRIVATE(area_composer)->spacing * (child_count-1)) + extra_padding;
		}		
		
	} else {
		/* Then we're getting the req along the "lesser" axis; it's merely the maximal req of any kid in this dimension: */
		for (iter = PRIVATE(area_composer)->list_of_child_details; iter; iter=iter->next) {
			CongEditorAreaComposerChildDetails *child_details;
			CongEditorArea *child;
			guint child_req;
			
			child_details = (CongEditorAreaComposerChildDetails*)(iter->data);
			child = CONG_EDITOR_AREA(child_details->child);
			g_assert (child);
			
			child_req = cong_editor_area_get_requisition (child,
								      orientation,
								      width_hint);
			if (result<child_req) {
				result = child_req;
			}
			
			child_count++;
		}
	}

	return result;
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaComposer *area_composer = CONG_EDITOR_AREA_COMPOSER(area);
	GList *iter;
	gint x;
	gint y;
	const GdkRectangle *rect = cong_editor_area_get_window_coords(area);
#if 0
	guint this_width = rect->width;
	guint this_height = rect->height;
#else
	guint this_width = cong_editor_area_get_requisition (area, 
							     GTK_ORIENTATION_HORIZONTAL,
							     rect->width);
	guint this_height = cong_editor_area_get_requisition (area, 
							      GTK_ORIENTATION_VERTICAL,
							      rect->width);
#endif
	gint total_surplus_space = 0;
	gint surplus_space_per_expandable_child = 0;
	guint num_expandable_children = 0;

	x = rect->x;
	y = rect->y;

	/* First pass: calculate the num expandable children, the amount of surplus space and hence the amount per expandable child: */
	{
		for (iter = PRIVATE(area_composer)->list_of_child_details; iter; iter=iter->next) {
			CongEditorAreaComposerChildDetails *child_details;
			CongEditorArea *child;
			
			child_details = (CongEditorAreaComposerChildDetails*)(iter->data);
			child = CONG_EDITOR_AREA(child_details->child);
			
			if (child_details->expand) {
				num_expandable_children++;
			}
		}
		
		if (PRIVATE(area_composer)->orientation == GTK_ORIENTATION_HORIZONTAL) {
			total_surplus_space = rect->width - this_width;
		} else {
			total_surplus_space = rect->height - this_height;
		}
		
		if (total_surplus_space<0) {
			total_surplus_space = 0;
		}
		
		if (num_expandable_children>0) {
			surplus_space_per_expandable_child = total_surplus_space/num_expandable_children;
#if 0
			g_message("surplus space per:%i",surplus_space_per_expandable_child);
#endif
		}
	}

#if DEBUG_CHILD_ALLOCATIONS
	g_message("-->%ccomposer giving children an alloc of (%i,%i); surplus space=%i (%i per expandable child)",
		  ((PRIVATE(area_composer)->orientation == GTK_ORIENTATION_HORIZONTAL)?'h':'v'),
		  this_width,
		  this_height,
		  total_surplus_space,
		  surplus_space_per_expandable_child);
#endif

	/* Second pass: allocate all the space: */
	for (iter = PRIVATE(area_composer)->list_of_child_details; iter; iter=iter->next) {
		CongEditorAreaComposerChildDetails *child_details;
		CongEditorArea *child;
		guint child_req_width;
		guint child_req_height;
		int child_width;
		int child_height;
		int extra_offset;

		child_details = (CongEditorAreaComposerChildDetails*)(iter->data);
		child = CONG_EDITOR_AREA(child_details->child);

		child_req_width = cong_editor_area_get_requisition (child, GTK_ORIENTATION_HORIZONTAL, this_width);
		child_req_height = cong_editor_area_get_requisition (child, GTK_ORIENTATION_VERTICAL, this_width);

		if (PRIVATE(area_composer)->orientation == GTK_ORIENTATION_HORIZONTAL) {
			child_width = child_req_width;
			child_height = rect->height;
		} else {
			child_width = rect->width;
			child_height = child_req_height;
		}
		extra_offset = child_details->extra_padding;

		if (child_details->expand) {
			extra_offset += surplus_space_per_expandable_child;

			if (child_details->fill) {
				if (PRIVATE(area_composer)->orientation == GTK_ORIENTATION_HORIZONTAL) {
					child_width += surplus_space_per_expandable_child;
				} else {
					child_height += surplus_space_per_expandable_child;
				}
			}
		}
 
#if DEBUG_CHILD_ALLOCATIONS
		g_message("-->%ccomposer giving %s child with cached req (%i,%i) an alloc of (%i,%i)",
			  ((PRIVATE(area_composer)->orientation == GTK_ORIENTATION_HORIZONTAL)?'h':'v'),
			  (child_details->expand?"expandable":"non-expandable"),
			  child_req_width,
			  child_req_height,
			  child_width,
			  child_height);
#endif

		cong_editor_area_set_allocation (child,
						 x,
						 y,
						 child_width,
						 child_height);
		
		if (PRIVATE(area_composer)->orientation == GTK_ORIENTATION_HORIZONTAL) {
			x += child_req_width + PRIVATE(area_composer)->spacing + extra_offset;
		} else {
			y += child_req_height + PRIVATE(area_composer)->spacing + extra_offset;
		}
	}
}

static CongEditorArea*
for_all (CongEditorArea *editor_area, 
	 CongEditorAreaCallbackFunc func, 
	 gpointer user_data)
{
	GList *iter;
	CongEditorAreaComposer *area_composer = CONG_EDITOR_AREA_COMPOSER(editor_area);

	for (iter = PRIVATE(area_composer)->list_of_child_details; iter; iter=iter->next) {
		CongEditorAreaComposerChildDetails *child_details;
		CongEditorArea *child;

		child_details = (CongEditorAreaComposerChildDetails*)(iter->data);
		child = CONG_EDITOR_AREA(child_details->child);
		
		if ((*func)(child, user_data)) {
			return child;
		}
	}

	return NULL;
}


static void
add_child ( CongEditorAreaContainer *area_container,
	    CongEditorArea *child)
{
	CongEditorAreaComposer *area_composer = CONG_EDITOR_AREA_COMPOSER(area_container);

	cong_editor_area_composer_pack_end (area_composer,
					    child,
					    TRUE,
					    TRUE,
					    0);
	
}

static void
add_child_after ( CongEditorAreaContainer *area_container,
		  CongEditorArea *new_child,
		  CongEditorArea *relative_to)
{
	CongEditorAreaComposer *area_composer = CONG_EDITOR_AREA_COMPOSER(area_container);
	
	cong_editor_area_composer_pack_after (area_composer,
					      new_child,
					      relative_to,
					      TRUE,
					      TRUE,
					      0);
	
}

static void
remove_child ( CongEditorAreaContainer *area_container,
	       CongEditorArea *child)
{
	CongEditorAreaComposer *area_composer = CONG_EDITOR_AREA_COMPOSER(area_container);
	GList *iter;

	g_return_if_fail (IS_CONG_EDITOR_AREA(child));

	for (iter = PRIVATE(area_composer)->list_of_child_details; iter; iter=iter->next) {
		CongEditorAreaComposerChildDetails *iter_child_details;
		CongEditorArea *iter_child;

		iter_child_details = (CongEditorAreaComposerChildDetails*)(iter->data);
		iter_child = CONG_EDITOR_AREA(iter_child_details->child);

		if (child==iter_child) {
			/* Found it: */
			PRIVATE(area_composer)->list_of_child_details = g_list_delete_link (PRIVATE(area_composer)->list_of_child_details,
											    iter);
			g_object_unref (child);

			return;
		}

	}

	/* Not found: */
	g_error ("CongEditorAreaComposer::remove_child called for an area that wawn't a child");
}

static void
remove_all_children (CongEditorAreaContainer *area_container)
{
	CongEditorAreaComposer *area_composer = CONG_EDITOR_AREA_COMPOSER(area_container);
	GList *iter;
	GList *next;

	for (iter = PRIVATE(area_composer)->list_of_child_details; iter; iter = next) {
		CongEditorAreaComposerChildDetails *iter_child_details;
		CongEditorArea *iter_child;
		next = iter->next;

		iter_child_details = (CongEditorAreaComposerChildDetails*)(iter->data);
		iter_child = CONG_EDITOR_AREA(iter_child_details->child);

		PRIVATE(area_composer)->list_of_child_details = g_list_delete_link (PRIVATE(area_composer)->list_of_child_details,
										    iter);
		g_object_unref (iter_child);
	}
}

static CongEditorAreaComposerChildDetails* 
get_child_details (CongEditorAreaComposer *area_composer,
		   CongEditorArea *child)
{
	GList *iter;

	for (iter = PRIVATE(area_composer)->list_of_child_details; iter; iter = iter->next) {
		CongEditorAreaComposerChildDetails *iter_details = (CongEditorAreaComposerChildDetails*)(iter->data);
		if (iter_details->child == child) {
			return iter_details;
		}
	}

	return NULL;
}
