/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-text-fragment.h
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

#ifndef __CONG_EDITOR_AREA_TEXT_FRAGMENT_H__
#define __CONG_EDITOR_AREA_TEXT_FRAGMENT_H__

#include "cong-editor-area-text.h"

G_BEGIN_DECLS

typedef struct CongEditorAreaTextFragment CongEditorAreaTextFragment;
typedef struct CongEditorAreaTextFragmentClass CongEditorAreaTextFragmentClass;
typedef struct CongEditorAreaTextFragmentDetails CongEditorAreaTextFragmentDetails;

#define CONG_EDITOR_AREA_TEXT_FRAGMENT_TYPE	   (cong_editor_area_text_fragment_get_type ())
#define CONG_EDITOR_AREA_TEXT_FRAGMENT(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_TEXT_FRAGMENT_TYPE, CongEditorAreaTextFragment)
#define CONG_EDITOR_AREA_TEXT_FRAGMENT_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_TEXT_FRAGMENT_TYPE, CongEditorAreaTextFragmentClass)
#define IS_CONG_EDITOR_AREA_TEXT_FRAGMENT(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_TEXT_FRAGMENT_TYPE)

struct CongEditorAreaTextFragment
{
	CongEditorArea area;

	CongEditorAreaTextFragmentDetails *private;
};

struct CongEditorAreaTextFragmentClass
{
	CongEditorAreaClass klass;
};

GType
cong_editor_area_text_fragment_get_type (void);

CongEditorArea*
cong_editor_area_text_fragment_construct (CongEditorAreaTextFragment *area_text_fragment,
					  CongEditorWidget3 *editor_widget,
					  PangoLayout *pango_layout,
					  guint line_index,
					  int baseline);

CongEditorArea*
cong_editor_area_text_fragment_new (CongEditorWidget3 *editor_widget,
				    PangoLayout *pango_layout,
				    guint line_index,
				    int baseline);

PangoLayoutLine*
cong_editor_area_text_get_pango_layout_line (CongEditorAreaTextFragment *area_text_fragment);

gboolean
cong_editor_area_text_fragment_x_to_index (CongEditorAreaTextFragment *area_text_fragment,
					   int x,
					   int *index_,
					   int *trailing);

G_END_DECLS

#endif
