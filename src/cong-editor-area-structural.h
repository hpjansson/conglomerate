/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-structural.h
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

#ifndef __CONG_EDITOR_AREA_STRUCTURAL_H__
#define __CONG_EDITOR_AREA_STRUCTURAL_H__

#include "cong-editor-area-bin.h"

G_BEGIN_DECLS

typedef struct CongEditorAreaStructural CongEditorAreaStructural;
typedef struct CongEditorAreaStructuralClass CongEditorAreaStructuralClass;
typedef struct CongEditorAreaStructuralDetails CongEditorAreaStructuralDetails;

#define CONG_EDITOR_AREA_STRUCTURAL_TYPE	   (cong_editor_area_structural_get_type ())
#define CONG_EDITOR_AREA_STRUCTURAL(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_STRUCTURAL_TYPE, CongEditorAreaStructural)
#define CONG_EDITOR_AREA_STRUCTURAL_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_STRUCTURAL_TYPE, CongEditorAreaStructuralClass)
#define IS_CONG_EDITOR_AREA_STRUCTURAL(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_STRUCTURAL_TYPE)

struct CongEditorAreaStructural
{
	CongEditorAreaBin bin;

	CongEditorAreaStructuralDetails *private;
};

struct CongEditorAreaStructuralClass
{
	CongEditorAreaContainerClass klass;
};

GType
cong_editor_area_structural_get_type (void);

CongEditorArea*
cong_editor_area_structural_construct (CongEditorAreaStructural *area_structural,
				       CongEditorWidget3 *editor_widget,
				       GdkPixbuf *pixbuf,					   
				       const gchar *text,
				       const GdkColor *col_bold,
				       const GdkColor *col_dim,
				       const GdkColor *col_background,
				       const GdkColor *col_text);

CongEditorArea*
cong_editor_area_structural_new (CongEditorWidget3 *editor_widget,
				 GdkPixbuf *pixbuf,
				 const gchar *text,
				 const GdkColor *col_bold,
				 const GdkColor *col_dim,
				 const GdkColor *col_background,
				 const GdkColor *col_text);

G_END_DECLS

#endif
