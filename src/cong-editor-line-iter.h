/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-line-iter.h
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

#ifndef __CONG_EDITOR_LINE_ITER_H__
#define __CONG_EDITOR_LINE_ITER_H__

#include "cong-object.h"
#include "cong-editor-line-manager.h"

G_BEGIN_DECLS

#define CONG_EDITOR_LINE_ITER_TYPE	  (cong_editor_line_iter_get_type ())
#define CONG_EDITOR_LINE_ITER(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_LINE_ITER_TYPE, CongEditorLineIter)
#define CONG_EDITOR_LINE_ITER_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_LINE_ITER_TYPE, CongEditorLineIterClass)
#define IS_CONG_EDITOR_LINE_ITER(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_LINE_ITER_TYPE)

CONG_DECLARE_CLASS_BEGIN (CongEditorLineIter, cong_editor_line_iter, GObject)
     
     CongEditorLineIter* (*clone) (CongEditorLineIter *line_iter);
     CongEditorAreaLine* (*get_line) (CongEditorLineIter *line_iter);

CONG_DECLARE_CLASS_END ()

CongEditorLineIter*
cong_editor_line_iter_construct (CongEditorLineIter *line_iter,
				 CongEditorLineManager *line_manager);

CongEditorLineManager*
cong_editor_line_iter_get_line_manager (CongEditorLineIter *line_iter);

CongEditorLineIter*
cong_editor_line_iter_clone (CongEditorLineIter *line_iter);

CongEditorAreaLine*
cong_editor_line_iter_get_line (CongEditorLineIter *line_iter);

G_END_DECLS

#endif



