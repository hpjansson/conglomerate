/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-line-iter-simple.c
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
#include "cong-editor-line-iter-simple.h"

static CongEditorLineIter*
clone (CongEditorLineIter *line_iter);

static CongEditorAreaLine*
get_line (CongEditorLineIter *line_iter);

CONG_DEFINE_CLASS_PUBLIC_DATA (CongEditorLineIterSimple, cong_editor_line_iter_simple, CONG_EDITOR_LINE_ITER_SIMPLE, CongEditorLineIter, CONG_EDITOR_LINE_ITER_TYPE, 
			       CONG_EDITOR_LINE_ITER_CLASS (klass)->clone = clone;
			       CONG_EDITOR_LINE_ITER_CLASS (klass)->get_line = get_line;)

CONG_DEFINE_EMPTY_DISPOSE(cong_editor_line_iter_simple)


/* Implementation of CongLineEditorIterSimple: */
CongEditorLineIterSimple*
cong_editor_line_iter_simple_construct (CongEditorLineIterSimple *line_iter,
					CongEditorLineManagerSimple *simple)
{
	
	return CONG_EDITOR_LINE_ITER_SIMPLE (cong_editor_line_iter_construct (CONG_EDITOR_LINE_ITER (line_iter),
									      CONG_EDITOR_LINE_MANAGER (simple)));
}

CongEditorLineIterSimple*
cong_editor_line_iter_simple_new (CongEditorLineManagerSimple *simple)
{
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_MANAGER_SIMPLE (simple), NULL);

	return cong_editor_line_iter_simple_construct (g_object_new (CONG_EDITOR_LINE_ITER_SIMPLE_TYPE, NULL),
						       simple);
}

static CongEditorLineIter*
clone (CongEditorLineIter *line_iter)
{
	CongEditorLineIterSimple* simple_iter = CONG_EDITOR_LINE_ITER_SIMPLE (line_iter);
	CongEditorLineIterSimple* new_iter;

	new_iter = cong_editor_line_iter_simple_new (CONG_EDITOR_LINE_MANAGER_SIMPLE (cong_editor_line_iter_get_line_manager (line_iter)));

	new_iter->current_line = simple_iter->current_line;
	new_iter->current_prev_line = simple_iter->current_prev_line;
	new_iter->current_prev_area = simple_iter->current_prev_area;

	return CONG_EDITOR_LINE_ITER (new_iter);
}

static CongEditorAreaLine*
get_line (CongEditorLineIter *line_iter)
{
	CongEditorLineIterSimple* simple_iter = CONG_EDITOR_LINE_ITER_SIMPLE (line_iter);
	
	return simple_iter->current_line;
}
