/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-lines.h
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

#ifndef __CONG_EDITOR_AREA_LINES_H__
#define __CONG_EDITOR_AREA_LINES_H__

#include "cong-object.h"
#include "cong-editor-area-composer.h"

G_BEGIN_DECLS

typedef struct CongEditorAreaLines CongEditorAreaLines;

#define CONG_EDITOR_AREA_LINES_TYPE	  (cong_editor_area_lines_get_type ())
#define CONG_EDITOR_AREA_LINES(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_LINES_TYPE, CongEditorAreaLines)
#define CONG_EDITOR_AREA_LINES_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_LINES_TYPE, CongEditorAreaLinesClass)
#define IS_CONG_EDITOR_AREA_LINES(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_LINES_TYPE)

CONG_DECLARE_CLASS (CongEditorAreaLines, cong_editor_area_lines, CongEditorAreaComposer)

CongEditorArea*
cong_editor_area_lines_new (CongEditorWidget3 *widget);

G_END_DECLS

#endif



