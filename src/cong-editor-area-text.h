/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-text.h
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

#ifndef __CONG_EDITOR_AREA_TEXT_H__
#define __CONG_EDITOR_AREA_TEXT_H__

#include "cong-editor-area.h"

G_BEGIN_DECLS

typedef struct CongEditorAreaText CongEditorAreaText;
typedef struct CongEditorAreaTextClass CongEditorAreaTextClass;
typedef struct CongEditorAreaTextDetails CongEditorAreaTextDetails;

#define CONG_EDITOR_AREA_TEXT_TYPE	   (cong_editor_area_text_get_type ())
#define CONG_EDITOR_AREA_TEXT(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_TEXT_TYPE, CongEditorAreaText)
#define CONG_EDITOR_AREA_TEXT_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_TEXT_TYPE, CongEditorAreaTextClass)
#define IS_CONG_EDITOR_AREA_TEXT(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_TEXT_TYPE)

struct CongEditorAreaText
{
	CongEditorArea area;

	CongEditorAreaTextDetails *private;
};

struct CongEditorAreaTextClass
{
	CongEditorAreaClass klass;
};

GType
cong_editor_area_text_get_type (void);

CongEditorArea*
cong_editor_area_text_construct (CongEditorAreaText *area_text,
				 CongEditorWidget3 *editor_widget,
				 CongFont *font,
				 const GdkColor *fg_col,
				 const gchar *text,
				 gboolean use_markup);

CongEditorArea*
cong_editor_area_text_new (CongEditorWidget3 *editor_widget,
			   CongFont *font,
			   const GdkColor *fg_col,
			   const gchar *text,
			   gboolean use_markup);

void
cong_editor_area_text_set_text (CongEditorAreaText *area_text,
				const gchar *text);

void
cong_editor_area_text_set_markup (CongEditorAreaText *area_text,
				  const gchar *markup);

gint 
cong_editor_area_text_get_single_line_requisition (CongEditorAreaText *area_text,
						   GtkOrientation orientation);

gboolean
cong_editor_area_text_xy_to_index (CongEditorAreaText *area_text,
				   int x,
				   int y,
				   int *index_,
				   int *trailing);

G_END_DECLS

#endif
