/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-attribute-editor-nmtoken.h
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
 * Authors: Douglas Burke <dburke@cfa.harvard.edu>
 * Based on code by David Malcolm <david@davemalcolm.demon.co.uk>
 */

#ifndef __CONG_ATTRIBUTE_EDITOR_NMTOKEN_H__
#define __CONG_ATTRIBUTE_EDITOR_NMTOKEN_H__

#include "cong-attribute-editor.h"

G_BEGIN_DECLS

typedef struct _CongAttributeEditorNMTOKEN CongAttributeEditorNMTOKEN;
typedef struct _CongAttributeEditorNMTOKENClass CongAttributeEditorNMTOKENClass;
typedef struct _CongAttributeEditorNMTOKENDetails CongAttributeEditorNMTOKENDetails;

#define CONG_ATTRIBUTE_EDITOR_NMTOKEN_TYPE	      (cong_attribute_editor_nmtoken_get_type ())
#define CONG_ATTRIBUTE_EDITOR_NMTOKEN(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_ATTRIBUTE_EDITOR_NMTOKEN_TYPE, CongAttributeEditorNMTOKEN)
#define CONG_ATTRIBUTE_EDITOR_NMTOKEN_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_ATTRIBUTE_EDITOR_NMTOKEN_TYPE, CongAttributeEditorNMTOKENClass)
#define IS_CONG_ATTRIBUTE_EDITOR_NMTOKEN(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_ATTRIBUTE_EDITOR_NMTOKEN_TYPE)

struct _CongAttributeEditorNMTOKEN
{
	CongAttributeEditor attribute_editor;
	CongAttributeEditorNMTOKENDetails *private;
};

struct _CongAttributeEditorNMTOKENClass
{
	CongAttributeEditorClass attribute_editor_klass;
};

GType
cong_attribute_editor_nmtoken_get_type (void);

CongAttributeEditor*
cong_attribute_editor_nmtoken_construct (CongAttributeEditorNMTOKEN *attribute_editor_nmtoken,
					 CongDocument *doc,
					 CongNodePtr node,
					 xmlNs *ns_ptr,
					 const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_nmtoken_new (CongDocument *doc,
				   CongNodePtr node,
				   xmlNs *ns_ptr,
				   const gchar *attribute_name);


G_END_DECLS

#endif
