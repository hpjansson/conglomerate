/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-glade.c
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


#include "global.h"
#include "cong-app.h"
#include "cong-glade.h"

#include "cong-attribute-wrapper.h"
#include "cong-attribute-wrapper-check-button.h"
#include "cong-attribute-wrapper-radio-button.h"

#include "cong-attribute-editor.h"

#include "cong-attribute-editor-cdata.h"
#include "cong-attribute-editor-lang.h"
#include "cong-attribute-editor-enumeration.h"

CongDocument *global_glade_doc_ptr = NULL;
CongNodePtr global_glade_node_ptr = NULL;

/**
 * cong_util_load_glade_file:
 * @filename:  Datadir-relative path of the file, e.g. "conglomerate/glade/my-file.glade"
 * @root:  The root widget to be created, or NULL for all of them
 * @doc:  The #CongDocument to be available to custom widgets, or NULL
 * @node:  The #CongNodePtr to be available to custom widgets, or NULL
 *
 * Convenience function for loading interfaces using libglade.
 *
 * Converts the filename from a datadir-relative path to an installed path internally,
 * and sets up the CongDocument and node pointers (if any) so that custom widgets can
 * wire themselves up properly.
 *
 * Returns: a freshly-loaded #GladeXML interface; client must unref it.
 */
GladeXML*
cong_util_load_glade_file (const gchar *filename,
			   const gchar *root,
			   CongDocument *doc,
			   CongNodePtr node) 
{
	gchar* glade_filename;
	GladeXML *xml;
		
	g_return_val_if_fail (filename, NULL);
	if (doc || node) {
		g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	}

	glade_filename = gnome_program_locate_file (cong_app_get_gnome_program (cong_app_singleton()),
						    GNOME_FILE_DOMAIN_APP_DATADIR,
						    filename,
						    FALSE,
						    NULL);
	global_glade_doc_ptr = doc;
	global_glade_node_ptr = node;
	
	xml = glade_xml_new (glade_filename, 
			     root, 
			     NULL);
	glade_xml_signal_autoconnect(xml);
	
	global_glade_doc_ptr = NULL;
	global_glade_node_ptr = NULL;
	
	g_free(glade_filename);

	return xml;
}

/**
 * cong_bind_radio_button:
 * @radio_button:
 * @doc:
 * @node:
 * @ns_ptr:
 * @attribute_name:
 * @attribute_value:
 *
 * TODO: Write me
 */
void
cong_bind_radio_button (GtkRadioButton *radio_button,
			CongDocument *doc,
			CongNodePtr node,
			xmlNs *ns_ptr,
			const gchar *attribute_name,
			const gchar *attribute_value)
{
	CongAttributeWrapperRadioButton* wrapper = cong_attribute_wrapper_radio_button_new ( doc,
											     node,
											     ns_ptr,
											     attribute_name,
											     NULL,
											     radio_button,
											     attribute_value);
	cong_attribute_wrapper_bind_to_widget (CONG_ATTRIBUTE_WRAPPER (wrapper),
				GTK_WIDGET (radio_button));
}

/**
 * cong_bind_check_button:
 * @check_button:
 * @doc:
 * @node:
 * @ns_ptr:
 * @attribute_name:
 * @attribute_value_unchecked:
 * @attribute_value_checked:
 *
 * TODO: Write me
 */
void
cong_bind_check_button (GtkCheckButton *check_button,
			CongDocument *doc,
			CongNodePtr node,
			xmlNs *ns_ptr,
			const gchar *attribute_name,
			const gchar *attribute_value_unchecked,
			const gchar *attribute_value_checked)
{
	CongAttributeWrapperCheckButton* wrapper = cong_attribute_wrapper_check_button_new ( doc,
											     node,
											     ns_ptr,
											     attribute_name,
											     NULL,
											     check_button,
											     attribute_value_unchecked,
											     attribute_value_checked);
	cong_attribute_wrapper_bind_to_widget (CONG_ATTRIBUTE_WRAPPER (wrapper),
					       GTK_WIDGET (check_button));
}


/**
 * create_cdata_editor:
 * @xml:
 * @func_name:
 * @name:
 * @string1:
 * @string2:
 * @int1:
 * @int2:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget*
create_cdata_editor (GladeXML *xml,
		     gchar *func_name,
		     gchar *name,
		     gchar *string1,
		     gchar *string2,
		     gint int1,
		     gint int2,
		     gpointer user_data)
{
	GtkWidget *custom_widget;

#if 1
	/* for some reason, the string1 stuff is coming through in func_name on my machine: */

	/* FIXME: Should we store the namespace URI somewhere or is the prefix enough. */

	const char *local_name;
	xmlNs *ns_ptr = cong_node_get_attr_ns(global_glade_node_ptr,
					      func_name,
					      &local_name);

	custom_widget = cong_attribute_editor_cdata_new (global_glade_doc_ptr, 
							 global_glade_node_ptr, 
							 ns_ptr,
							 local_name,
							 NULL);
#else
	custom_widget = gtk_label_new(g_strdup_printf("custom widget \"%s\" \"%s\" \"%s\" \"%s\" %i %i", func_name, name, string1, string2, int1, int2)); /* for now */

	gtk_widget_show_all(custom_widget);
#endif

	gtk_widget_show (custom_widget);

	return custom_widget;
}

/**
 * create_lang_editor:
 * @xml:
 * @func_name:
 * @name:
 * @string1:
 * @string2:
 * @int1:
 * @int2:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget*
create_lang_editor (GladeXML *xml,
		     gchar *func_name,
		     gchar *name,
		     gchar *string1,
		     gchar *string2,
		     gint int1,
		     gint int2,
		     gpointer user_data)
{
	GtkWidget *custom_widget;
	const char *local_name;

	xmlNs *ns_ptr = cong_node_get_attr_ns(global_glade_node_ptr,
					      "lang",
					      &local_name);

	custom_widget = cong_attribute_editor_lang_new (global_glade_doc_ptr, 
							 global_glade_node_ptr, 
							 ns_ptr,
							 NULL);
	gtk_widget_show_all (custom_widget);

	return custom_widget;
}
