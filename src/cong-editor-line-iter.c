/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-line-iter.c
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
#include "cong-editor-line-iter.h"

#include "cong-eel.h"

struct CongEditorLineIterPrivate
{
	CongEditorLineManager *line_manager;
};

CONG_DEFINE_CLASS_BEGIN (CongEditorLineIter, cong_editor_line_iter, CONG_EDITOR_LINE_ITER, GObject, G_TYPE_OBJECT)
CONG_DEFINE_CLASS_END ()
CONG_DEFINE_EMPTY_DISPOSE(cong_editor_line_iter)


/* Implementation of CongEditorLineIter: */
CongEditorLineIter*
cong_editor_line_iter_construct (CongEditorLineIter *line_iter,
				 CongEditorLineManager *line_manager)
{
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_ITER (line_iter), NULL);
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager), NULL);

	PRIVATE (line_iter)->line_manager = line_manager;
	
	return line_iter;
}

CongEditorLineManager*
cong_editor_line_iter_get_line_manager (CongEditorLineIter *line_iter)
{
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_ITER (line_iter), NULL);

	return PRIVATE (line_iter)->line_manager;
}

CongEditorLineIter*
cong_editor_line_iter_clone (CongEditorLineIter *line_iter)
{
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_ITER (line_iter), NULL);

	return CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_LINE_ITER_CLASS,
						       line_iter,
						       clone,
						       (line_iter));
}

CongEditorAreaLine*
cong_editor_line_iter_get_line (CongEditorLineIter *line_iter)
{
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_ITER (line_iter), NULL);

	return CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_LINE_ITER_CLASS,
						       line_iter,
						       get_line,
						       (line_iter));
}
