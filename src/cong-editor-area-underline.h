/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-underline.h
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

#ifndef __CONG_EDITOR_AREA_UNDERLINE_H__
#define __CONG_EDITOR_AREA_UNDERLINE_H__

#include "cong-editor-area.h"

G_BEGIN_DECLS

typedef struct CongEditorAreaUnderline CongEditorAreaUnderline;
typedef struct CongEditorAreaUnderlineClass CongEditorAreaUnderlineClass;
typedef struct CongEditorAreaUnderlineDetails CongEditorAreaUnderlineDetails;

#define CONG_EDITOR_AREA_UNDERLINE_TYPE	   (cong_editor_area_underline_get_type ())
#define CONG_EDITOR_AREA_UNDERLINE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_UNDERLINE_TYPE, CongEditorAreaUnderline)
#define CONG_EDITOR_AREA_UNDERLINE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_UNDERLINE_TYPE, CongEditorAreaUnderlineClass)
#define IS_CONG_EDITOR_AREA_UNDERLINE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_UNDERLINE_TYPE)

typedef enum  {
	CONG_EDITOR_AREA_UNDERLINE_STYLE_SPAN_TAG_START,
	CONG_EDITOR_AREA_UNDERLINE_STYLE_SPAN_TAG_END,
	CONG_EDITOR_AREA_UNDERLINE_STYLE_SPELLING_ERROR,
	CONG_EDITOR_AREA_UNDERLINE_STYLE_GRAMMAR_ERROR
} CongEditorAreaUnderlineStyle;

struct CongEditorAreaUnderline
{
	CongEditorArea area;

	CongEditorAreaUnderlineDetails *private;
};

struct CongEditorAreaUnderlineClass
{
	CongEditorAreaClass klass;
};

GType
cong_editor_area_underline_get_type (void);

CongEditorArea*
cong_editor_area_underline_construct (CongEditorAreaUnderline *area_underline,
				      CongEditorWidget3 *editor_widget,
				      CongEditorAreaUnderlineStyle style);

CongEditorArea*
cong_editor_area_underline_new (CongEditorWidget3 *editor_widget,
				CongEditorAreaUnderlineStyle style);

G_END_DECLS

#endif
