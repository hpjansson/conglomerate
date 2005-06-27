/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-entity-decl.h
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

#ifndef __CONG_EDITOR_AREA_ENTITY_DECL_H__
#define __CONG_EDITOR_AREA_ENTITY_DECL_H__

#include "cong-editor-area-bin.h"

G_BEGIN_DECLS

typedef struct _CongEditorAreaEntityDecl CongEditorAreaEntityDecl;
typedef struct _CongEditorAreaEntityDeclClass CongEditorAreaEntityDeclClass;
typedef struct _CongEditorAreaEntityDeclDetails CongEditorAreaEntityDeclDetails;

#define CONG_EDITOR_AREA_ENTITY_DECL_TYPE	   (cong_editor_area_entity_decl_get_type ())
#define CONG_EDITOR_AREA_ENTITY_DECL(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_ENTITY_DECL_TYPE, CongEditorAreaEntityDecl)
#define CONG_EDITOR_AREA_ENTITY_DECL_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_ENTITY_DECL_TYPE, CongEditorAreaEntityDeclClass)
#define IS_CONG_EDITOR_AREA_ENTITY_DECL(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_ENTITY_DECL_TYPE)

struct _CongEditorAreaEntityDecl
{
	CongEditorAreaBin bin;

	CongEditorAreaEntityDeclDetails *private;
};

struct _CongEditorAreaEntityDeclClass
{
	CongEditorAreaContainerClass klass;
};

GType
cong_editor_area_entity_decl_get_type (void);

CongEditorArea*
cong_editor_area_entity_decl_construct (CongEditorAreaEntityDecl *area_entity_decl,
					CongEditorWidget3 *editor_widget,
					const gchar *text);

CongEditorArea*
cong_editor_area_entity_decl_new (CongEditorWidget3 *editor_widget,
				  const gchar *text);

G_END_DECLS

#endif
