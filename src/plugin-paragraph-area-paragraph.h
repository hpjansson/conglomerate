/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-paragraph-area-paragraph.h
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

#ifndef __PLUGIN_PARAGRAPH_AREA_PARAGRAPH_H__
#define __PLUGIN_PARAGRAPH_AREA_PARAGRAPH_H__

#include "cong-editor-area-border.h"

G_BEGIN_DECLS

typedef struct CongEditorAreaParagraph CongEditorAreaParagraph;
typedef struct CongEditorAreaParagraphClass CongEditorAreaParagraphClass;
typedef struct CongEditorAreaParagraphDetails CongEditorAreaParagraphDetails;

#define CONG_EDITOR_AREA_PARAGRAPH_TYPE	        (cong_editor_area_paragraph_get_type ())
#define CONG_EDITOR_AREA_PARAGRAPH(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_PARAGRAPH_TYPE, CongEditorAreaParagraph)
#define CONG_EDITOR_AREA_PARAGRAPH_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_PARAGRAPH_TYPE, CongEditorAreaParagraphClass)
#define IS_CONG_EDITOR_AREA_PARAGRAPH(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_PARAGRAPH_TYPE)

struct CongEditorAreaParagraph
{
	CongEditorAreaBorder border;

	CongEditorAreaParagraphDetails *private;
};

struct CongEditorAreaParagraphClass
{
	CongEditorAreaBorderClass klass;
};

GType
cong_editor_area_paragraph_get_type (void);

CongEditorArea*
cong_editor_area_paragraph_construct (CongEditorAreaParagraph *area_paragraph,
				      CongEditorWidget3 *editor_widget);

CongEditorArea*
cong_editor_area_paragraph_new (CongEditorWidget3 *editor_widget);

G_END_DECLS

#endif
