/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-modification.h
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

#ifndef __CONG_MODIFICATION_H__
#define __CONG_MODIFICATION_H__

#include "cong-document.h"

G_BEGIN_DECLS

#define DEBUG_MODIFICATION_LIFETIMES 0

#define CONG_MODIFICATION_TYPE	  (cong_modification_get_type ())
#define CONG_MODIFICATION(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_MODIFICATION_TYPE, CongModification)
#define CONG_MODIFICATION_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_MODIFICATION_TYPE, CongModificationClass)
#define IS_CONG_MODIFICATION(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_MODIFICATION_TYPE)

typedef struct CongModification CongModification;
typedef struct CongModificationClass CongModificationClass;
typedef struct CongModificationDetails CongModificationDetails;


struct CongModification
{
	GObject object;

	CongModificationDetails *private;
};

struct CongModificationClass
{
	GObjectClass klass;

	/* Methods? */
	void (*undo) (CongModification *modification);
	void (*redo) (CongModification *modification);
};

GType
cong_modification_get_type (void);

CongModification*
cong_modification_construct (CongModification *modification,
			     CongDocument *doc);

CongDocument*
cong_modification_get_document (CongModification *modification);

void
cong_modification_undo (CongModification *modification);

void
cong_modification_redo (CongModification *modification);

/* Modification: */
#if 0
CongModification*
cong_modification_node_add_after_new (CongDocument *doc,
				      CongNodePtr node, 
				      CongNodePtr older_sibling);

CongModification* 
cong_modification_node_add_before_new (CongDocument *doc,
				       CongNodePtr node, 
				       CongNodePtr younger_sibling);

CongModification* 
cong_modification_node_set_parent_new (CongDocument *doc, 
				       CongNodePtr node,
				       CongNodePtr adoptive_parent); /* added to end of child list */

CongModification* 
cong_modification_node_set_text_new (CongDocument *doc, 
				     CongNodePtr node, 
				     const gchar *new_content);

CongModification* 
cong_modification_node_set_attribute_new (CongDocument *doc, 
					  CongNodePtr node, 
					  const gchar *name, 
					  const gchar *value);

CongModification* 
cong_modification_node_remove_attribute_new (CongDocument *doc, 
					     CongNodePtr node, 
					     const gchar *name);

CongModification* 
cong_modification_selection_change_new (CongDocument *doc);

CongModification* 
cong_modification_cursor_change_new (CongDocument *doc);

CongModification* 
cong_modification_set_external_dtd_new (CongDocument *doc,
					const gchar* root_element,
					const gchar* public_id,
					const gchar* system_id);
#endif

G_END_DECLS

#endif
