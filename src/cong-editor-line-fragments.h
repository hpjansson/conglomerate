/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-line-fragments.h
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

#ifndef __CONG_EDITOR_LINE_FRAGMENTS_H__
#define __CONG_EDITOR_LINE_FRAGMENTS_H__

#include "cong-editor-widget.h"

G_BEGIN_DECLS

#define DEBUG_EDITOR_LINE_FRAGMENTS_LIFETIMES 0

typedef struct _CongEditorLineFragmentsDetails CongEditorLineFragmentsDetails;

#define CONG_EDITOR_LINE_FRAGMENTS_TYPE	      (cong_editor_line_fragments_get_type ())
#define CONG_EDITOR_LINE_FRAGMENTS(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_LINE_FRAGMENTS_TYPE, CongEditorLineFragments)
#define CONG_EDITOR_LINE_FRAGMENTS_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_LINE_FRAGMENTS_TYPE, CongEditorLineFragmentsClass)
#define IS_CONG_EDITOR_LINE_FRAGMENTS(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_LINE_FRAGMENTS_TYPE)

struct _CongEditorLineFragments
{
	GObject object;

	CongEditorLineFragmentsDetails *private;
};

struct _CongEditorLineFragmentsClass
{
	GObjectClass klass;
};

GType
cong_editor_line_fragments_get_type (void);

CongEditorLineFragments*
cong_editor_line_fragments_construct (CongEditorLineFragments *line_fragments,
				      CongWhitespaceHandling whitespace);

CongEditorLineFragments*
cong_editor_line_fragments_new (CongWhitespaceHandling whitespace);

/* FIXME: perhaps we should make this a for_each, with a callback? */
GList*
cong_editor_line_fragments_get_area_list (CongEditorLineFragments *line_fragments);

void
cong_editor_line_fragments_add_area (CongEditorLineFragments *line_fragments,
				     CongEditorArea *area);

CongWhitespaceHandling
cong_editor_line_fragments_get_whitespace (CongEditorLineFragments *line_fragments);

G_END_DECLS

#endif
