/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-glade.h
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

#ifndef _CONG_GLADE_H_
#define _CONG_GLADE_H_

#include <glade/glade.h>

extern CongDocument *global_glade_doc_ptr;
extern CongNodePtr global_glade_node_ptr;

GladeXML*
cong_util_load_glade_file (const gchar *filename,
			   const gchar *root,
			   CongDocument *doc,
			   CongNodePtr node);

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

GtkWidget*
create_enumeration_editor (GladeXML *xml,
		     gchar *func_name,
		     gchar *name,
		     gchar *string1,
		     gchar *string2,
		     gint int1,
		     gint int2,
		     gpointer user_data);

GtkWidget*
create_lang_editor   (GladeXML *xml,
		     gchar *func_name,
		     gchar *name,
		     gchar *string1,
		     gchar *string2,
		     gint int1,
		     gint int2,
		     gpointer user_data);

/* Hook to bind a radio button to a possible value of an attribute: */
void
cong_bind_radio_button (GtkRadioButton *radio_button,
			CongDocument *doc,
			CongNodePtr node,
			xmlNs *ns_ptr,
			const gchar *attribute_name,
			const gchar *attribute_value);

/* Hook to bind a checkbox to two possible values of an attribute: */
void
cong_bind_check_button (GtkCheckButton *check_button,
			CongDocument *doc,
			CongNodePtr node,
			xmlNs *ns_ptr,
			const gchar *attribute_name,
			const gchar *attribute_value_unchecked,
			const gchar *attribute_value_checked);

#endif /* _CONG_GLADE_H_ */

