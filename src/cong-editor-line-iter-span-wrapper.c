/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-line-iter-span-wrapper.c
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
#include "cong-editor-line-iter-span-wrapper.h"

static CongEditorLineIter*
clone (CongEditorLineIter *line_iter);

static CongEditorAreaLine*
get_line (CongEditorLineIter *line_iter);

CONG_DEFINE_CLASS_PUBLIC_DATA (CongEditorLineIterSpanWrapper, cong_editor_line_iter_span_wrapper, CONG_EDITOR_LINE_ITER_SPAN_WRAPPER, CongEditorLineIter, CONG_EDITOR_LINE_ITER_TYPE, 
			       CONG_EDITOR_LINE_ITER_CLASS (klass)->clone = clone;
			       CONG_EDITOR_LINE_ITER_CLASS (klass)->get_line = get_line;)

/* Implementation of CongLineEditorIterSpanWrapper: */
CongEditorLineIterSpanWrapper*
cong_editor_line_iter_span_wrapper_construct (CongEditorLineIterSpanWrapper *line_iter,
					      CongEditorLineManagerSpanWrapper *span_wrapper,
					      CongEditorLineIter *outer_iter)
{
	cong_editor_line_iter_construct (CONG_EDITOR_LINE_ITER (line_iter),
					 CONG_EDITOR_LINE_MANAGER (span_wrapper));

#if 1
	line_iter->outer_iter = outer_iter;
	g_object_ref (G_OBJECT (outer_iter));
#else
	line_iter->outer_iter = cong_editor_line_iter_clone (outer_iter);
#endif

	g_assert (IS_CONG_EDITOR_LINE_MANAGER (cong_editor_line_iter_get_line_manager (CONG_EDITOR_LINE_ITER (line_iter))));
	
	return line_iter;
}

CongEditorLineIterSpanWrapper*
cong_editor_line_iter_span_wrapper_new (CongEditorLineManagerSpanWrapper *span_wrapper,
					CongEditorLineIter *outer_iter)
{
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_MANAGER_SPAN_WRAPPER (span_wrapper), NULL);

	return cong_editor_line_iter_span_wrapper_construct (g_object_new (CONG_EDITOR_LINE_ITER_SPAN_WRAPPER_TYPE, NULL),
							     span_wrapper,
							     outer_iter);
}

static void 
cong_editor_line_iter_span_wrapper_dispose (GObject *object)
{
	CongEditorLineIterSpanWrapper *span_wrapper_iter = CONG_EDITOR_LINE_ITER_SPAN_WRAPPER (object);

	if (span_wrapper_iter->outer_iter) {
		g_object_unref (G_OBJECT (span_wrapper_iter->outer_iter));
		span_wrapper_iter->outer_iter = NULL;
	}
	
}

static CongEditorLineIter*
clone (CongEditorLineIter *line_iter)
{
	CongEditorLineIterSpanWrapper* span_wrapper_iter = CONG_EDITOR_LINE_ITER_SPAN_WRAPPER (line_iter);
	CongEditorLineIterSpanWrapper* new_iter;

#if 1
	g_assert (IS_CONG_EDITOR_LINE_MANAGER (cong_editor_line_iter_get_line_manager (line_iter)));

	new_iter = g_object_new (CONG_EDITOR_LINE_ITER_SPAN_WRAPPER_TYPE, NULL);
	cong_editor_line_iter_construct (CONG_EDITOR_LINE_ITER (new_iter),
					 CONG_EDITOR_LINE_MANAGER (cong_editor_line_iter_get_line_manager (line_iter)));
	new_iter->outer_iter = span_wrapper_iter->outer_iter; /* I believe we shouldn't clone this; we use it directly */
	g_object_ref (G_OBJECT (new_iter->outer_iter));
#else
	new_iter = cong_editor_line_iter_span_wrapper_new (CONG_EDITOR_LINE_MANAGER_SPAN_WRAPPER (cong_editor_line_iter_get_line_manager (line_iter)));
#endif
	new_iter->prev_area_on_current_line = span_wrapper_iter->prev_area_on_current_line;

	g_assert (IS_CONG_EDITOR_LINE_MANAGER (cong_editor_line_iter_get_line_manager (CONG_EDITOR_LINE_ITER (new_iter))));

	return CONG_EDITOR_LINE_ITER (new_iter);
}

static CongEditorAreaLine*
get_line (CongEditorLineIter *line_iter)
{
	CongEditorLineIterSpanWrapper* span_wrapper_iter = CONG_EDITOR_LINE_ITER_SPAN_WRAPPER (line_iter);

	g_assert (span_wrapper_iter->outer_iter);

	return cong_editor_line_iter_get_line (span_wrapper_iter->outer_iter);
}
