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

/* Generic attribute editing widget factory method: */
GtkWidget*
cong_attribute_editor_new (CongDocument *doc,
			   CongNodePtr node,
			   xmlAttributePtr attr);

/* Specific attribute editing widget factory methods: */
GtkWidget*
cong_attribute_editor_cdata_new (CongDocument *doc,
				 CongNodePtr node,
				 const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_id_new (CongDocument *doc,
			      CongNodePtr node,
			      const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_idref_new (CongDocument *doc,
				 CongNodePtr node,
				 const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_idrefs_new (CongDocument *doc,
				  CongNodePtr node,
				  const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_entity_new (CongDocument *doc,
				  CongNodePtr node,
				  const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_entities_new (CongDocument *doc,
				    CongNodePtr node,
				    const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_nmtoken_new (CongDocument *doc,
				   CongNodePtr node,
				   const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_nmtokens_new (CongDocument *doc,
				    CongNodePtr node,
				    const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_enumeration_new (CongDocument *doc,
				       CongNodePtr node,
				       const gchar *attribute_name);
GtkWidget*
cong_attribute_editor_notation_new (CongDocument *doc,
				    CongNodePtr node,
				    const gchar *attribute_name);

/* Glade hooks for various attribute types: */
GtkWidget*
create_cdata_editor (GladeXML *xml,
		     gchar *func_name,
		     gchar *name,
		     gchar *string1,
		     gchar *string2,
		     gint int1,
		     gint int2,
		     gpointer user_data);
/* FIXME: add the rest of the types */

/* Glade doesn't seem to support user_data unless you override the custom handler.  Hence we use globals for now */
extern CongDocument *global_glade_doc_ptr;
extern CongNodePtr global_glade_node_ptr;

G_END_DECLS


#endif
