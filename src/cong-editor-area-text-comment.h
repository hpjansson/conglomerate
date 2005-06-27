/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-text_comment.h
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

#ifndef __CONG_EDITOR_AREA_TEXT_COMMENT_H__
#define __CONG_EDITOR_AREA_TEXT_COMMENT_H__

#include "cong-editor-area-text.h"

G_BEGIN_DECLS

typedef struct _CongEditorAreaTextComment CongEditorAreaTextComment;
typedef struct _CongEditorAreaTextCommentClass CongEditorAreaTextCommentClass;
typedef struct _CongEditorAreaTextCommentDetails CongEditorAreaTextCommentDetails;

#define CONG_EDITOR_AREA_TEXT_COMMENT_TYPE	   (cong_editor_area_text_comment_get_type ())
#define CONG_EDITOR_AREA_TEXT_COMMENT(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_TEXT_COMMENT_TYPE, CongEditorAreaTextComment)
#define CONG_EDITOR_AREA_TEXT_COMMENT_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_TEXT_COMMENT_TYPE, CongEditorAreaTextCommentClass)
#define IS_CONG_EDITOR_AREA_TEXT_COMMENT(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_TEXT_COMMENT_TYPE)

struct _CongEditorAreaTextComment
{
	CongEditorAreaText area;

	CongEditorAreaTextCommentDetails *private;
};

struct _CongEditorAreaTextCommentClass
{
	CongEditorAreaTextClass klass;
};

GType
cong_editor_area_text_comment_get_type (void);

CongEditorArea*
cong_editor_area_text_comment_construct (CongEditorAreaTextComment *area_text_comment,
				 CongEditorWidget3 *editor_widget,
				 CongFont *font,
				 const GdkColor *fg_col,
				 const gchar *text_comment,
				 gboolean use_markup);

CongEditorArea*
cong_editor_area_text_comment_new (CongEditorWidget3 *editor_widget,
			   CongFont *font,
			   const GdkColor *fg_col,
			   const gchar *text_comment,
			   gboolean use_markup);

#if 0
void
cong_editor_area_text_comment_set_text_comment (CongEditorAreaTextComment *area_text_comment,
				const gchar *text_comment);

void
cong_editor_area_text_comment_set_markup (CongEditorAreaTextComment *area_text_comment,
				  const gchar *markup);

gint 
cong_editor_area_text_comment_get_single_line_requisition (CongEditorAreaTextComment *area_text_comment,
							   GtkOrientation orientation);

gboolean
cong_editor_area_text_comment_xy_to_index (CongEditorAreaTextComment *area_text_comment,
				   int x,
				   int y,
				   int *index_,
				   int *trailing);
#endif

G_END_DECLS

#endif
