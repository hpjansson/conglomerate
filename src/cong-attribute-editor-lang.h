/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-attribute-editor-lang.h
 *
 * Copyright (C) 2004 David Malcolm
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
 *
 */

#ifndef __CONG_ATTRIBUTE_EDITOR_LANG_H__
#define __CONG_ATTRIBUTE_EDITOR_LANG_H__

#include "cong-attribute-editor.h"

G_BEGIN_DECLS

typedef struct _CongAttributeEditorLang CongAttributeEditorLang;
typedef struct _CongAttributeEditorLangClass CongAttributeEditorLangClass;
typedef struct _CongAttributeEditorLangDetails CongAttributeEditorLangDetails;

#define CONG_ATTRIBUTE_EDITOR_LANG_TYPE	      (cong_attribute_editor_lang_get_type ())
#define CONG_ATTRIBUTE_EDITOR_LANG(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_ATTRIBUTE_EDITOR_LANG_TYPE, CongAttributeEditorLang)
#define CONG_ATTRIBUTE_EDITOR_LANG_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_ATTRIBUTE_EDITOR_LANG_TYPE, CongAttributeEditorLangClass)
#define IS_CONG_ATTRIBUTE_EDITOR_LANG(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_ATTRIBUTE_EDITOR_LANG_TYPE)

struct _CongAttributeEditorLang
{
	CongAttributeEditor attribute_editor;
	CongAttributeEditorLangDetails *private;
};

struct _CongAttributeEditorLangClass
{
	CongAttributeEditorClass attribute_editor_klass;
};

GType
cong_attribute_editor_lang_get_type (void);

CongAttributeEditor*
cong_attribute_editor_lang_construct (CongAttributeEditorLang *attribute_editor_lang,
				       CongDocument *doc,
				       CongNodePtr node,
				       xmlNs *ns_ptr,
				       xmlAttributePtr attr);
GtkWidget*
cong_attribute_editor_lang_new (CongDocument *doc,
			        CongNodePtr node,
			        xmlNs *ns_ptr,
				xmlAttributePtr attr);
G_END_DECLS

#endif
