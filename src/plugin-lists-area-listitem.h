/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-lists-area-listitem.h
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

#ifndef __PLUGIN_LISTS_AREA_LISTITEM_H__
#define __PLUGIN_LISTS_AREA_LISTITEM_H__

#include "cong-editor-area-bin.h"

G_BEGIN_DECLS

typedef struct CongEditorAreaListitem CongEditorAreaListitem;
typedef struct CongEditorAreaListitemClass CongEditorAreaListitemClass;
typedef struct CongEditorAreaListitemDetails CongEditorAreaListitemDetails;

#define CONG_EDITOR_AREA_LISTITEM_TYPE	   (cong_editor_area_listitem_get_type ())
#define CONG_EDITOR_AREA_LISTITEM(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_LISTITEM_TYPE, CongEditorAreaListitem)
#define CONG_EDITOR_AREA_LISTITEM_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_LISTITEM_TYPE, CongEditorAreaListitemClass)
#define IS_CONG_EDITOR_AREA_LISTITEM(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_LISTITEM_TYPE)

struct CongEditorAreaListitem
{
	CongEditorAreaBin bin;

	CongEditorAreaListitemDetails *private;
};

struct CongEditorAreaListitemClass
{
	CongEditorAreaContainerClass klass;
};

GType
cong_editor_area_listitem_get_type (void);

CongEditorArea*
cong_editor_area_listitem_construct (CongEditorAreaListitem *area_listitem,
				     CongEditorWidget3 *editor_widget,
				     const gchar *label);

CongEditorArea*
cong_editor_area_listitem_new (CongEditorWidget3 *editor_widget,
			       const gchar *label);

void
cong_editor_area_listitem_set_label (CongEditorAreaListitem *area_listitem,
				     const gchar *label);

G_END_DECLS

#endif
