/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-attribute-editor.h
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

#ifndef __CONG_ATTRIBUTE_EDITOR_H__
#define __CONG_ATTRIBUTE_EDITOR_H__

#include "cong-document.h"
#include <glade/glade.h>

G_BEGIN_DECLS

typedef struct _CongAttributeEditor CongAttributeEditor;
typedef struct _CongAttributeEditorClass CongAttributeEditorClass;
typedef struct _CongAttributeEditorDetails CongAttributeEditorDetails;

#define CONG_ATTRIBUTE_EDITOR_TYPE	      (cong_attribute_editor_get_type ())
#define CONG_ATTRIBUTE_EDITOR(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_ATTRIBUTE_EDITOR_TYPE, CongAttributeEditor)
#define CONG_ATTRIBUTE_EDITOR_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_ATTRIBUTE_EDITOR_TYPE, CongAttributeEditorClass)
#define IS_CONG_ATTRIBUTE_EDITOR(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_ATTRIBUTE_EDITOR_TYPE)

struct _CongAttributeEditor
{
	GtkHBox hbox;

	CongAttributeEditorDetails *private;
};

struct _CongAttributeEditorClass
{
	GtkHBoxClass klass;

	void (*set_attribute_handler) (CongAttributeEditor *attribute_editor);
	void (*remove_attribute_handler) (CongAttributeEditor *attribute_editor);
};

GType
cong_attribute_editor_get_type (void);

CongAttributeEditor*
cong_attribute_editor_construct (CongAttributeEditor *attribute_editor,
				 CongDocument *doc,
				 CongNodePtr node,
				 xmlNs *ns_ptr,
				 const gchar *attribute_name);

CongDocument*
cong_attribute_editor_get_document (CongAttributeEditor *attribute_editor);

CongNodePtr
cong_attribute_editor_get_node (CongAttributeEditor *attribute_editor);

xmlNs *
cong_attribute_editor_get_ns (CongAttributeEditor *attribute_editor);

const gchar*
cong_attribute_editor_get_attribute_name (CongAttributeEditor *attribute_editor);

gchar*
cong_attribute_editor_get_attribute_value (CongAttributeEditor *attribute_editor);

void
cong_attribute_editor_try_set_value (CongAttributeEditor *attribute_editor, const gchar *new_attr_value);

/* Generic attribute editing widget factory method: */
GtkWidget*
cong_attribute_editor_new (CongDocument *doc,
			   CongNodePtr node,
			   xmlAttributePtr attr);

/* Specific attribute editing widget factory methods (to be moved to separate headers): */
GtkWidget*
cong_attribute_editor_id_new (CongDocument *doc,
			      CongNodePtr node,
			      xmlNs *ns_ptr,
			      const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_idref_new (CongDocument *doc,
				 CongNodePtr node,
				 xmlNs *ns_ptr,
				 const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_idrefs_new (CongDocument *doc,
				  CongNodePtr node,
				  xmlNs *ns_ptr,
				  const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_entity_new (CongDocument *doc,
				  CongNodePtr node,
				  xmlNs *ns_ptr,
				  const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_entities_new (CongDocument *doc,
				    CongNodePtr node,
				    xmlNs *ns_ptr,
				    const gchar *attribute_name);
#if 0
GtkWidget*
cong_attribute_editor_nmtoken_new (CongDocument *doc,
				   CongNodePtr node,
				   xmlNs *ns_ptr,
				   const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_nmtokens_new (CongDocument *doc,
				    CongNodePtr node,
				    xmlNs *ns_ptr,
				    const gchar *attribute_name);
#endif

GtkWidget*
cong_attribute_editor_enumeration_new (CongDocument *doc,
				       CongNodePtr node,
				       xmlNs *ns_ptr,
				       const gchar *attribute_name,
				       xmlAttributePtr attr);
GtkWidget*
cong_attribute_editor_notation_new (CongDocument *doc,
				    CongNodePtr node,
				    xmlNs *ns_ptr,
				    const gchar *attribute_name);


G_END_DECLS


#endif
