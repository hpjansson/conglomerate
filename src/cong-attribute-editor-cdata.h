/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-attribute-editor-cdata.h
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

#ifndef __CONG_ATTRIBUTE_EDITOR_CDATA_H__
#define __CONG_ATTRIBUTE_EDITOR_CDATA_H__

#include "cong-attribute-editor.h"

G_BEGIN_DECLS

typedef struct _CongAttributeEditorCDATA CongAttributeEditorCDATA;
typedef struct _CongAttributeEditorCDATAClass CongAttributeEditorCDATAClass;
typedef struct _CongAttributeEditorCDATADetails CongAttributeEditorCDATADetails;

#define CONG_ATTRIBUTE_EDITOR_CDATA_TYPE	      (cong_attribute_editor_cdata_get_type ())
#define CONG_ATTRIBUTE_EDITOR_CDATA(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_ATTRIBUTE_EDITOR_CDATA_TYPE, CongAttributeEditorCDATA)
#define CONG_ATTRIBUTE_EDITOR_CDATA_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_ATTRIBUTE_EDITOR_CDATA_TYPE, CongAttributeEditorCDATAClass)
#define IS_CONG_ATTRIBUTE_EDITOR_CDATA(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_ATTRIBUTE_EDITOR_CDATA_TYPE)

struct _CongAttributeEditorCDATA
{
	CongAttributeEditor attribute_editor;
	CongAttributeEditorCDATADetails *private;
};

struct _CongAttributeEditorCDATAClass
{
	CongAttributeEditorClass attribute_editor_klass;
};

GType
cong_attribute_editor_cdata_get_type (void);

CongAttributeEditor*
cong_attribute_editor_cdata_construct (CongAttributeEditorCDATA *attribute_editor_cdata,
				       CongDocument *doc,
				       CongNodePtr node,
				       xmlNs *ns_ptr,
				       const gchar *attribute_name,
				       xmlAttributePtr attr);
GtkWidget*
cong_attribute_editor_cdata_new (CongDocument *doc,
				 CongNodePtr node,
				 xmlNs *ns_ptr,
				 const gchar *attribute_name,
				 xmlAttributePtr attr);
G_END_DECLS

#endif
