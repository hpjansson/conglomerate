/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-line-manager-simple.c
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
#include "cong-editor-line-manager-simple.h"
#include "cong-editor-line-iter-simple.h"
#include "cong-editor-node.h"
#include "cong-editor-area.h"
#include "cong-editor-area-line.h"
#include "cong-editor-area-composer.h"

#include "cong-eel.h"

#define HACKED_WIDTH (150)
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

struct CongEditorLineManagerSimplePrivate
{
	CongEditorAreaLines *area_lines;
};

CONG_DEFINE_CLASS_BEGIN (CongEditorLineManagerSimple, cong_editor_line_manager_simple, CONG_EDITOR_LINE_MANAGER_SIMPLE, CongEditorLineManager, CONG_EDITOR_LINE_MANAGER_TYPE)
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
CONG_DEFINE_EMPTY_DISPOSE(cong_editor_line_manager_simple)

CongEditorLineManager*
cong_editor_line_manager_simple_construct (CongEditorLineManagerSimple *line_manager,
					   CongEditorWidget3 *widget,
					   CongEditorAreaLines *area_lines)
{
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_MANAGER_SIMPLE (line_manager), NULL);
	g_return_val_if_fail (IS_CONG_EDITOR_AREA_LINES (area_lines), NULL);

	cong_editor_line_manager_construct (CONG_EDITOR_LINE_MANAGER (line_manager),
					    widget);

	PRIVATE (line_manager)->area_lines = area_lines;

	return CONG_EDITOR_LINE_MANAGER (line_manager);
}

CongEditorLineManager*
cong_editor_line_manager_simple_new (CongEditorWidget3 *widget,
				     CongEditorAreaLines *area_lines)
{
	g_return_val_if_fail (IS_CONG_EDITOR_AREA (area_lines), NULL);

	return CONG_EDITOR_LINE_MANAGER (cong_editor_line_manager_simple_construct (g_object_new (CONG_EDITOR_LINE_MANAGER_SIMPLE_TYPE, NULL),
										    widget,
										    area_lines));
}


static CongEditorLineIter*
make_iter (CongEditorLineManager *line_manager)
{
	CongEditorLineManagerSimple *simple = CONG_EDITOR_LINE_MANAGER_SIMPLE (line_manager);

	return CONG_EDITOR_LINE_ITER (cong_editor_line_iter_simple_new (simple));
}

static void 
begin_line (CongEditorLineManager *line_manager,
	    CongEditorCreationRecord *creation_record,
	    CongEditorLineIter *line_iter)
{
	CongEditorLineManagerSimple *simple = CONG_EDITOR_LINE_MANAGER_SIMPLE (line_manager);
	CongEditorAreaLine *new_line;
	CongEditorLineIterSimple *line_iter_simple = CONG_EDITOR_LINE_ITER_SIMPLE (line_iter);

	new_line = CONG_EDITOR_AREA_LINE (cong_editor_area_line_new (cong_editor_line_manager_get_widget (line_manager),
								     HACKED_WIDTH)); /* FIXME */
	if (line_iter_simple->current_line) {
		line_iter_simple->current_prev_line = line_iter_simple->current_line;
	}
	line_iter_simple->current_line = new_line;
	line_iter_simple->current_prev_area = NULL;

	if (line_iter_simple->current_prev_line) {
		cong_editor_area_composer_pack_after (CONG_EDITOR_AREA_COMPOSER (PRIVATE (simple)->area_lines),
						      CONG_EDITOR_AREA (new_line),
						      CONG_EDITOR_AREA (line_iter_simple->current_prev_line),
						      FALSE,
						      TRUE,
						      0);
	} else {
		cong_editor_area_composer_pack_start (CONG_EDITOR_AREA_COMPOSER (PRIVATE (simple)->area_lines),
						      CONG_EDITOR_AREA (new_line),
						      FALSE,
						      TRUE,
						      0);
	}
}

static void
add_to_line (CongEditorLineManager *line_manager,
	     CongEditorCreationRecord *creation_record,
	     CongEditorLineIter *line_iter,
	     CongEditorArea *area)
{
	/* CongEditorLineManagerSimple *simple = CONG_EDITOR_LINE_MANAGER_SIMPLE (line_manager); */
	CongEditorLineIterSimple *line_iter_simple = CONG_EDITOR_LINE_ITER_SIMPLE (line_iter);

	/* Ensure we have a line to add the area to: */
	if (NULL==line_iter_simple->current_line) {
		cong_editor_line_manager_begin_line (line_manager,
						     creation_record,
						     line_iter);
	}

	if (line_iter_simple->current_prev_area) {
		cong_editor_area_container_add_child_after (CONG_EDITOR_AREA_CONTAINER (line_iter_simple->current_line),
							    area,
							    line_iter_simple->current_prev_area);
	} else {
		/* Add to the front of the line: */
		cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER (line_iter_simple->current_line),
						      area,
						      FALSE);
	}

	line_iter_simple->current_prev_area = area;
}

static void 
end_line (CongEditorLineManager *line_manager,
	  CongEditorCreationRecord *creation_record,
	  CongEditorLineIter *line_iter)
{
	/* CongEditorLineManagerSimple *simple = CONG_EDITOR_LINE_MANAGER_SIMPLE (line_manager); */
	CongEditorLineIterSimple *line_iter_simple = CONG_EDITOR_LINE_ITER_SIMPLE (line_iter);

	if (line_iter_simple->current_line) {
		line_iter_simple->current_prev_line = line_iter_simple->current_line;
	}
	line_iter_simple->current_line = NULL;
}

#if 0
static void
delete_whole_line (CongEditorLineManager *line_manager,
		   CongEditorAreaLine *line)
{
#error
}
#endif

#if 1
static void 
undo_change (CongEditorLineManager *line_manager,
	     enum CongEditorCreationEvent event,
	     CongEditorLineIter *iter_before,
	     CongEditorLineIter *iter_after)
{
	/*CongEditorLineManagerSimple *simple = CONG_EDITOR_LINE_MANAGER_SIMPLE (line_manager); */
	/*CongEditorLineIterSimple *iter_before_simple = CONG_EDITOR_LINE_ITER_SIMPLE (iter_before);*/
	CongEditorLineIterSimple *iter_after_simple = CONG_EDITOR_LINE_ITER_SIMPLE (iter_after);

	switch (event) {
	default: g_assert_not_reached ();
	case CONG_EDITOR_CREATION_EVENT_BEGIN_LINE:
		g_message ("FIXME: unimplemented CongEditorLineManagerSimple::undo_change (BEGIN_LINE)");
		break;

	case CONG_EDITOR_CREATION_EVENT_END_LINE:
		g_message ("FIXME: unimplemented CongEditorLineManagerSimple::undo_change (END_LINE)");
		break;

	case CONG_EDITOR_CREATION_EVENT_ADD_AREA:
		{
			/* This should be the area that was added: */
			g_assert (iter_after_simple->current_prev_area);

			/* This should be the line that the area was added to: */
			g_assert (iter_after_simple->current_line);

			cong_editor_area_container_remove_child (CONG_EDITOR_AREA_CONTAINER (iter_after_simple->current_line),
								 iter_after_simple->current_prev_area);
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
	g_assert_not_reached ();
#if 0
	/*	CongEditorLineManagerSimple *simple = CONG_EDITOR_LINE_MANAGER_SIMPLE (line_manager); */
	CongEditorLineIterSimple *start_iter_simple = CONG_EDITOR_LINE_ITER_SIMPLE (start_iter);
	CongEditorLineIterSimple *end_iter_simple = CONG_EDITOR_LINE_ITER_SIMPLE (end_iter);

	if (start_iter_simple->current_line == end_iter_simple->current_line) {
		/* Simple case: both on the same line: */
		g_assert_not_reached ();
	} else {
		/* Complex case: they are on different lines: */
		CongEditorAreaLine *current_line = start_iter_simple->current_line;
		
		/* Delete final stuff on first line: */
		
		/* Delete entire lines: */
		while (current_line!=end_iter_simple->current_line) {
			CongEditorAreaLine *next_line = ;
			delete_whole_line (line_manager_simple,
					   current_line);

			
			
		}
		
		/* Delete initial stuff on last line: */
		cong_editor_area_container
		delete_from_line (
	}
#endif


}
#endif


static gint
get_line_width (CongEditorLineManager *line_manager,
		CongEditorLineIter *line_iter)
{
#if 0
	CongEditorLineManagerSimple *simple = CONG_EDITOR_LINE_MANAGER_SIMPLE (line_manager);
	CongEditorLineIterSimple *line_iter_simple = CONG_EDITOR_LINE_ITER_SIMPLE (line_iter);
#endif

	return HACKED_WIDTH; /* FIXME */
}

static gint
get_current_indent (CongEditorLineManager *line_manager,
		    CongEditorLineIter *line_iter)
{
	/* CongEditorLineManagerSimple *simple = CONG_EDITOR_LINE_MANAGER_SIMPLE (line_manager); */
	CongEditorLineIterSimple *line_iter_simple = CONG_EDITOR_LINE_ITER_SIMPLE (line_iter);

#if 1
	if (line_iter_simple->current_line) {
		return cong_editor_area_line_get_width_used_up_to (line_iter_simple->current_line,
								   line_iter_simple->current_prev_area);
	} else {
		return 0;
	}
#else
	return cong_editor_area_get_requisition (CONG_EDITOR_AREA (PRIVATE (simple)->current_line),
						 GTK_ORIENTATION_HORIZONTAL,
						 HACKED_WIDTH); /* FIXME */
#endif
}
