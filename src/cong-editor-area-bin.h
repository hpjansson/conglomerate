/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-bin.h
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

#ifndef __CONG_EDITOR_AREA_BIN_H__
#define __CONG_EDITOR_AREA_BIN_H__

#include "cong-editor-area-container.h"

G_BEGIN_DECLS

typedef struct _CongEditorAreaBin CongEditorAreaBin;
typedef struct _CongEditorAreaBinClass CongEditorAreaBinClass;
typedef struct _CongEditorAreaBinDetails CongEditorAreaBinDetails;

#define CONG_EDITOR_AREA_BIN_TYPE	   (cong_editor_area_bin_get_type ())
#define CONG_EDITOR_AREA_BIN(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_BIN_TYPE, CongEditorAreaBin)
#define CONG_EDITOR_AREA_BIN_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_BIN_TYPE, CongEditorAreaBinClass)
#define IS_CONG_EDITOR_AREA_BIN(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_BIN_TYPE)

struct _CongEditorAreaBin
{
	CongEditorAreaContainer area;

	CongEditorAreaBinDetails *private;
};

struct _CongEditorAreaBinClass
{
	CongEditorAreaContainerClass klass;
};

GType
cong_editor_area_bin_get_type (void);

CongEditorArea*
cong_editor_area_bin_construct (CongEditorAreaBin *area_bin,
				 CongEditorWidget3 *editor_widget);

CongEditorArea*
cong_editor_area_bin_new (CongEditorWidget3 *editor_widget);

CongEditorArea*
cong_editor_area_bin_get_child  (CongEditorAreaBin *area_bin);

G_END_DECLS

#endif
