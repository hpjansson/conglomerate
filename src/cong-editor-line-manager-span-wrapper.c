/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-line-manager-span-wrapper.c
 *
 * Copyright (C) 2004 David Malcolm
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
#include "cong-editor-line-manager-span-wrapper.h"
#include "cong-editor-line-iter-span-wrapper.h"
#include "cong-editor-node-element.h"
#include "cong-editor-area-span-tag.h"
#include "cong-dispspec-element.h"
#include "cong-editor-area-composer.h"

#include "cong-eel.h"

static CongEditorLineIter*
make_iter (CongEditorLineManager *line_manager);

static void 
begin_line (CongEditorLineManager *line_manager,
	    CongEditorCreationRecord *creation_record,
	    CongEditorLineIter *line_iter);

static void
add_to_line (CongEditorLineManager *line_manager,
	     CongEditorCreationRecord *creation_record,
	     CongEditorLineIter *line_iter,
	     CongEditorArea *area);

static void 
end_line (CongEditorLineManager *line_manager,
	  CongEditorCreationRecord *creation_record,
	  CongEditorLineIter *line_iter);

#if 1
static void 
undo_change (CongEditorLineManager *line_manager,
	     enum CongEditorCreationEvent event,
	     CongEditorLineIter *iter_before,
	     CongEditorLineIter *iter_after);
#else
static void 
delete_areas (CongEditorLineManager *line_manager,
	      CongEditorLineIter *start_iter,
	      CongEditorLineIter *end_iter);
#endif

static gint
get_line_width (CongEditorLineManager *line_manager,
		CongEditorLineIter *line_iter);

static gint
get_current_indent (CongEditorLineManager *line_manager,
		    CongEditorLineIter *line_iter);

/* Data stored about each line: */
typedef struct PerLineData PerLineData;
struct PerLineData
{
	CongEditorAreaSpanTag *span_area;
	CongEditorArea *inner_line;
};

static void
hash_value_destroy_func (gpointer data);

static PerLineData*
get_data_for_line (CongEditorLineManagerSpanWrapper *span_wrapper,
		   CongEditorAreaLine *line);


struct CongEditorLineManagerSpanWrapperPrivate
{
	CongEditorNode *editor_node;

	CongEditorLineManager *outer_line_manager;
	CongEditorCreationRecord *outer_creation_record;
	CongEditorLineIter *outer_iter;

	GHashTable *hash_of_line_to_data;
};

CONG_DEFINE_CLASS_BEGIN (CongEditorLineManagerSpanWrapper, cong_editor_line_manager_span_wrapper, CONG_EDITOR_LINE_MANAGER_SPAN_WRAPPER, CongEditorLineManager, CONG_EDITOR_LINE_MANAGER_TYPE)
{
	CongEditorLineManagerClass *lm_klass = CONG_EDITOR_LINE_MANAGER_CLASS (klass);

	lm_klass->make_iter = make_iter;

	lm_klass->begin_line = begin_line;
	lm_klass->add_to_line = add_to_line;
	lm_klass->end_line = end_line;

#if 1
	lm_klass->undo_change = undo_change;	
#else
	lm_klass->delete_areas = delete_areas;
#endif

	lm_klass->get_line_width = get_line_width;
	lm_klass->get_current_indent = get_current_indent;
}
CONG_DEFINE_CLASS_END ()

CongEditorLineManager*
cong_editor_line_manager_span_wrapper_construct (CongEditorLineManagerSpanWrapper *line_manager,
						 CongEditorWidget3 *widget,
						 CongEditorNode *editor_node,
						 CongEditorLineManager *outer_line_manager,
						 CongEditorCreationRecord *outer_creation_record,
						 CongEditorLineIter *outer_iter)
{
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_MANAGER_SPAN_WRAPPER (line_manager), NULL);
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_MANAGER (outer_line_manager), NULL);
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_ITER (outer_iter), NULL);

	cong_editor_line_manager_construct (CONG_EDITOR_LINE_MANAGER (line_manager),
					    widget);

	PRIVATE (line_manager)->editor_node = editor_node;
	PRIVATE (line_manager)->outer_line_manager = outer_line_manager;
	PRIVATE (line_manager)->outer_creation_record = outer_creation_record;
	
	PRIVATE (line_manager)->outer_iter = outer_iter;
	g_object_ref (G_OBJECT (PRIVATE (line_manager)->outer_iter));

	PRIVATE (line_manager)->hash_of_line_to_data = g_hash_table_new_full (g_direct_hash,
									      g_direct_equal,
									      NULL,
									      hash_value_destroy_func);

	return CONG_EDITOR_LINE_MANAGER (line_manager);
}

static void 
cong_editor_line_manager_span_wrapper_dispose (GObject *object)
{
	CongEditorLineManagerSpanWrapper *line_manager = CONG_EDITOR_LINE_MANAGER_SPAN_WRAPPER (object);

	if (PRIVATE (line_manager)->outer_iter) {
		g_object_unref (G_OBJECT (PRIVATE (line_manager)->outer_iter));
		PRIVATE (line_manager)->outer_iter = NULL;
	}

	if (PRIVATE (line_manager)->hash_of_line_to_data) {
		g_hash_table_destroy (PRIVATE (line_manager)->hash_of_line_to_data);
		PRIVATE (line_manager)->hash_of_line_to_data = NULL;
	}
}


CongEditorLineManager*
cong_editor_line_manager_span_wrapper_new (CongEditorWidget3 *widget,
					   CongEditorNode *editor_node,
					   CongEditorLineManager *outer_line_manager,
					   CongEditorCreationRecord *outer_creation_record,
					   CongEditorLineIter *outer_iter)
{
	return CONG_EDITOR_LINE_MANAGER (cong_editor_line_manager_span_wrapper_construct (g_object_new (CONG_EDITOR_LINE_MANAGER_SPAN_WRAPPER_TYPE, NULL),
											  widget,
											  editor_node,
											  outer_line_manager,
											  outer_creation_record,
											  outer_iter));	
}

static CongEditorAreaSpanTag*
make_span_area (CongEditorLineManagerSpanWrapper *span_wrapper,
		gboolean is_at_start,
		gboolean is_at_end)
{
	CongEditorArea *area;
	CongDispspecElement *ds_element;
	GdkPixbuf *pixbuf;
	gchar *title_text;
	
	g_return_val_if_fail (span_wrapper, NULL);
	
	ds_element = cong_editor_node_element_get_dispspec_element (CONG_EDITOR_NODE_ELEMENT(PRIVATE (span_wrapper)->editor_node));
	
	pixbuf = cong_dispspec_element_get_icon (ds_element);
	
	title_text = cong_dispspec_element_get_section_header_text (ds_element,
								    cong_editor_node_get_node (PRIVATE (span_wrapper)->editor_node));
	
	area = cong_editor_area_span_tag_new (cong_editor_node_get_widget (PRIVATE (span_wrapper)->editor_node),
					      ds_element,
					      pixbuf,
					      title_text,
					      is_at_start,
					      is_at_end);

	if (pixbuf) {
		g_object_unref (G_OBJECT(pixbuf));
	}

	g_free (title_text);

	return CONG_EDITOR_AREA_SPAN_TAG (area);
}

static CongEditorArea*
make_inner_line (CongEditorLineManagerSpanWrapper *span_wrapper)
{
	CongEditorArea *inner_line;

#if 1
	inner_line = cong_editor_area_composer_new (cong_editor_node_get_widget (PRIVATE (span_wrapper)->editor_node),
						    GTK_ORIENTATION_HORIZONTAL,
						    0);
#else
	inner_line = cong_editor_area_line_new (cong_editor_node_get_widget (PRIVATE (span_wrapper)->editor_node),
						cong_editor_line_manager_get_current_width_available (CONG_EDITOR_LINE_MANAGER (span_wrapper));
#endif

	return inner_line;
}

static CongEditorLineIter*
make_iter (CongEditorLineManager *line_manager)
{
	CongEditorLineManagerSpanWrapper *span_wrapper = CONG_EDITOR_LINE_MANAGER_SPAN_WRAPPER (line_manager);

	g_assert (PRIVATE (span_wrapper)->outer_iter);
	return CONG_EDITOR_LINE_ITER (cong_editor_line_iter_span_wrapper_new (span_wrapper,
									      PRIVATE (span_wrapper)->outer_iter));
}

static void
begin_line (CongEditorLineManager *line_manager,
	    CongEditorCreationRecord *creation_record,
	    CongEditorLineIter *line_iter)
{
	CongEditorLineManagerSpanWrapper *span_wrapper = CONG_EDITOR_LINE_MANAGER_SPAN_WRAPPER (line_manager);
	CongEditorLineIterSpanWrapper *span_wrapper_iter = CONG_EDITOR_LINE_ITER_SPAN_WRAPPER (line_iter);

	span_wrapper_iter->prev_area_on_current_line = NULL;

	/* Delegate: */
	cong_editor_line_manager_begin_line (PRIVATE (span_wrapper)->outer_line_manager,
					     PRIVATE (span_wrapper)->outer_creation_record,
					     span_wrapper_iter->outer_iter);

	/* Bugzilla 145474:
	   how do we handle this creation_record stuff? Perhaps we should it, and do the recording inside the line manager?
	   But how do we distinguish between stuff done by a node, and stuff done by the children of a node?  Wouldn't this mean
	   cleaning out parts of the change list when a child is removed?
	   Perhaps I should try to get the Simple subclass working (line removal) as I think about this?
	 */
}

static void
add_to_line (CongEditorLineManager *line_manager,
	     CongEditorCreationRecord *creation_record,
	     CongEditorLineIter *line_iter,
	     CongEditorArea *area)
{
	CongEditorLineManagerSpanWrapper *span_wrapper = CONG_EDITOR_LINE_MANAGER_SPAN_WRAPPER (line_manager);
	CongEditorLineIterSpanWrapper *span_wrapper_iter = CONG_EDITOR_LINE_ITER_SPAN_WRAPPER (line_iter);
	CongEditorAreaLine *line;
	PerLineData *line_data;

	line = cong_editor_line_iter_get_line (line_iter);

	if (NULL==line) {
		/* Need to start a new line:*/
		cong_editor_line_manager_begin_line (line_manager,
						     creation_record,
						     line_iter);
		line = cong_editor_line_iter_get_line (line_iter);
	}
	g_assert (line);

	line_data = get_data_for_line (span_wrapper,
				       line);
	
	/* Potentially we're adding the first area to this line; create the span wrapper area for the line: */
	if (NULL==line_data) {
		line_data = g_new0 (PerLineData, 1);
		g_hash_table_insert (PRIVATE (span_wrapper)->hash_of_line_to_data,
				     line,
				     line_data);

		line_data->span_area = make_span_area (span_wrapper,
						       TRUE, /* is_at_start, */
						       TRUE /* is_at_end */);
		line_data->inner_line = make_inner_line (span_wrapper);

		/* FIXME:  what about inserting at the start of a line which already has stuff on it? */
		span_wrapper_iter->prev_area_on_current_line = NULL;

		cong_editor_line_manager_add_to_line (PRIVATE (span_wrapper)->outer_line_manager,
						      PRIVATE (span_wrapper)->outer_creation_record,
						      span_wrapper_iter->outer_iter,
						      CONG_EDITOR_AREA (line_data->span_area));

		cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER (line_data->span_area),
						      line_data->inner_line,
						      TRUE);
	}
	g_assert (line_data->span_area);
	g_assert (line_data->inner_line);

	if (span_wrapper_iter->prev_area_on_current_line) {
		cong_editor_area_container_add_child_after (CONG_EDITOR_AREA_CONTAINER (line_data->inner_line),
							    area,
							    span_wrapper_iter->prev_area_on_current_line);
	} else {
		/* Add to the front of the line: */
		cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER (line_data->inner_line),
						      area,
						      FALSE);
	}

	span_wrapper_iter->prev_area_on_current_line = area;
}

static void 
end_line (CongEditorLineManager *line_manager,
	  CongEditorCreationRecord *creation_record,
	  CongEditorLineIter *line_iter)
{
	CongEditorLineManagerSpanWrapper *span_wrapper = CONG_EDITOR_LINE_MANAGER_SPAN_WRAPPER (line_manager);
	CongEditorLineIterSpanWrapper *span_wrapper_iter = CONG_EDITOR_LINE_ITER_SPAN_WRAPPER (line_iter);

	span_wrapper_iter->prev_area_on_current_line = NULL;
	
	/* Delegate: */
	return cong_editor_line_manager_end_line (PRIVATE (span_wrapper)->outer_line_manager,
						  PRIVATE (span_wrapper)->outer_creation_record,
						  span_wrapper_iter->outer_iter);
}

#if 1
static void 
undo_change (CongEditorLineManager *line_manager,
	     enum CongEditorCreationEvent event,
	     CongEditorLineIter *iter_before,
	     CongEditorLineIter *iter_after)
{
	CongEditorLineManagerSpanWrapper *span_wrapper = CONG_EDITOR_LINE_MANAGER_SPAN_WRAPPER (line_manager);
	/*CongEditorLineIterSpanWrapper *iter_before_span_wrapper = CONG_EDITOR_LINE_ITER_SPAN_WRAPPER (iter_before);*/
	CongEditorLineIterSpanWrapper *iter_after_span_wrapper = CONG_EDITOR_LINE_ITER_SPAN_WRAPPER (iter_after);

	switch (event) {
	default: g_assert_not_reached ();
	case CONG_EDITOR_CREATION_EVENT_BEGIN_LINE:
		g_message ("FIXME: unimplemented CongEditorLineManagerSpanWrapper::undo_change (BEGIN_LINE)");
		break;

	case CONG_EDITOR_CREATION_EVENT_END_LINE:
		g_message ("FIXME: unimplemented CongEditorLineManagerSpanWrapper::undo_change (END_LINE)");
		break;

	case CONG_EDITOR_CREATION_EVENT_ADD_AREA:
		{
			CongEditorAreaLine *line;
			PerLineData *line_data;
			
			line = cong_editor_line_iter_get_line (iter_after);
			g_assert (line);

			line_data = get_data_for_line (span_wrapper,
						       line);
			g_assert (line_data);

			/* This should be the area that was added: */
			g_assert (iter_after_span_wrapper->prev_area_on_current_line);

			/* This should be the line that the area was added to: */
			g_assert (line_data->inner_line);

			cong_editor_area_container_remove_child (CONG_EDITOR_AREA_CONTAINER (line_data->inner_line),
								 iter_after_span_wrapper->prev_area_on_current_line);

#if 0
			G_BREAKPOINT ();
#error 
			/* Why is this not called when removing a span tag?
			   Seems to be called when you "cut" the element in the Raw XML sidebar.
			   What about undoing the removal of a simple span tag?
			*/
#endif
		}
		break;

	}

}
#else
static void 
delete_areas (CongEditorLineManager *line_manager,
	      CongEditorLineIter *start_iter,
	      CongEditorLineIter *end_iter)
{
	CongEditorLineManagerSpanWrapper *span_wrapper = CONG_EDITOR_LINE_MANAGER_SPAN_WRAPPER (line_manager);
	CongEditorLineIterSpanWrapper *start_iter_span_wrapper = CONG_EDITOR_LINE_ITER_SPAN_WRAPPER (start_iter);
	CongEditorLineIterSpanWrapper *end_iter_span_wrapper = CONG_EDITOR_LINE_ITER_SPAN_WRAPPER (end_iter);

	/* Delegate: */
	/* FIXME: does this actually work?  what are the semantics of an iter? */
	cong_editor_line_manager_delete_areas (PRIVATE (span_wrapper)->outer_line_manager,
					       start_iter_span_wrapper->outer_iter,
					       end_iter_span_wrapper->outer_iter);

}
#endif

static gint
get_line_width (CongEditorLineManager *line_manager,
		CongEditorLineIter *line_iter)
{
	CongEditorLineManagerSpanWrapper *span_wrapper = CONG_EDITOR_LINE_MANAGER_SPAN_WRAPPER (line_manager);
	CongEditorLineIterSpanWrapper *span_wrapper_iter = CONG_EDITOR_LINE_ITER_SPAN_WRAPPER (line_iter);

	/* Delegate: */
	return cong_editor_line_manager_get_line_width (PRIVATE (span_wrapper)->outer_line_manager,
							span_wrapper_iter->outer_iter);
}

static gint
get_current_indent (CongEditorLineManager *line_manager,
		    CongEditorLineIter *line_iter)
{
	CongEditorLineManagerSpanWrapper *span_wrapper = CONG_EDITOR_LINE_MANAGER_SPAN_WRAPPER (line_manager);
	CongEditorLineIterSpanWrapper *span_wrapper_iter = CONG_EDITOR_LINE_ITER_SPAN_WRAPPER (line_iter);

	/* Delegate: */
	return cong_editor_line_manager_get_current_indent (PRIVATE (span_wrapper)->outer_line_manager,
							    span_wrapper_iter->outer_iter);
}

static void
hash_value_destroy_func (gpointer data)
{
	PerLineData *per_node_data = (PerLineData*)data;

	g_free (per_node_data);
}

static PerLineData*
get_data_for_line (CongEditorLineManagerSpanWrapper *span_wrapper,
		   CongEditorAreaLine *line)
{
	return (PerLineData*)g_hash_table_lookup (PRIVATE (span_wrapper)->hash_of_line_to_data,
						  line);
}
