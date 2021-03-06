/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-util.h
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

#ifndef __CONG_UTIL_H__
#define __CONG_UTIL_H__

G_BEGIN_DECLS

/* Handy utility functions: */

gboolean 
cong_util_is_docbook (CongDocument *doc);

gboolean 
cong_util_is_pure_whitespace (const gchar *utf8_text);

gchar* 
cong_util_cleanup_text (const gchar *text);

gchar* 
cong_util_text_header (const gchar *text,
		       guint truncation_length);

gchar*
cong_utils_get_norman_walsh_stylesheet_path(void);

gchar*
cong_utils_get_norman_walsh_stylesheet (const gchar *stylesheet_relative_path);

/**
   Icon loading; take an icon basename e.g. "cong-docbook-set", convert to a filename and load it.
 */
 
GdkPixbuf*
cong_util_load_icon (const gchar *icon_basename);

void 
cong_util_append (gchar **string, 
		  const gchar *to_add);

void 
cong_util_prepend (gchar **string, 
		   const gchar *to_add);

#if (ENABLE_PRINTING && ENABLE_LIBFO)
void 
cong_util_print_xslfo (GtkWindow *toplevel_window, 
		       GnomePrintContext *gpc, 
		       xmlDocPtr xml_doc);
#endif

/* macro adapted from libxml's error.c; surely this exists in GLib somewhere? */
#define CONG_GET_VAR_STR(msg, str) {				\
    int       size;						\
    int       chars;						\
    char      *larger;						\
    va_list   ap;						\
								\
    str = (gchar *)g_malloc(150);				\
								\
    size = 150;							\
								\
    while (1) {							\
	va_start(ap, msg);					\
  	chars = vsnprintf(str, size, msg, ap);			\
	va_end(ap);						\
	if ((chars > -1) && (chars < size))			\
	    break;						\
	if (chars > -1)						\
	    size += chars + 1;					\
	else							\
	    size += 100;					\
	if ((larger = (char *) g_realloc(str, size)) == NULL) { \
	    g_free(str);					\
	    return;						\
	}							\
	str = larger;						\
    }								\
}

xmlDtdPtr
cong_util_make_dtd (xmlDocPtr xml_doc,
		    const gchar *root_element,
		    const gchar *ExternalID, 
		    const gchar *SystemID);

xmlDtdPtr 
cong_util_add_external_dtd (xmlDocPtr xml_doc, 
			    const gchar *root_element,
			    const gchar *ExternalID, 
			    const gchar *SystemID);

void
cong_util_run_add_dtd_dialog (CongDocument *doc,
			      GtkWindow *parent_window);

/* Dodgy hack to do lines that blend to white: */

void 
cong_util_draw_blended_line (GtkWidget *w,
			     const GdkColor *col,
			     int x0, int y0,
			     int x1);

unsigned int
cong_util_get_int_from_rgb_hex (const gchar *string);


gboolean 
cong_util_ns_equality (const xmlNs *xml_ns1,
		       const xmlNs *xml_ns2);

gboolean
cong_util_ns_uri_equality (const gchar* uri0, 
			   const gchar* uri1);

gint
cong_util_ns_uri_sort_order (const gchar* uri0, 
			     const gchar* uri1);

GtkWidget*
cong_util_make_source_view (const gchar *source_mime_type,
			    GtkTextView **output_text_view);

GtkWidget*
cong_source_view_new_full (CongDocument *doc,
			   const gchar *source_mime_type,
			   void (*regeneration_cb) (CongDocument *doc,
						    GtkTextBuffer *text_buffer));
gboolean
cong_util_attribute_value_equality (const gchar *value0,
				    const gchar *value1);

CongElementDescription*
cong_element_description_new (const gchar *ns_uri,
			      const gchar *local_name);

CongElementDescription*
cong_element_description_new_from_node (CongNodePtr node);

CongElementDescription*
cong_element_description_clone (const CongElementDescription *element_desc);

void
cong_element_description_free (CongElementDescription *element_desc);

CongNodePtr
cong_element_description_make_node (const CongElementDescription *element_desc,
				    CongDocument *doc,
				    CongNodePtr ns_search_node);

gboolean
cong_element_description_matches_node (const CongElementDescription *element_desc,
				       CongNodePtr node);

CongDispspecElement*
cong_element_description_get_dispspec_element_for_doc (const CongElementDescription *element_desc,
						       CongDocument *doc);

CongDispspecElement*
cong_element_description_get_dispspec_element_for_dispspec (const CongElementDescription *element_desc,
							    CongDispspec *ds);

#if 0
gchar*
cong_element_description_get_qualified_name (const CongElementDescription *element_desc,
					     CongDispspec *ds);
#endif

gchar*
cong_element_description_make_user_name (const CongElementDescription *element_desc,
					 CongDispspec *ds);
const gchar*
cong_element_description_get_sort_string (const CongElementDescription *element_desc,
					  CongDispspec *ds);

GList*
cong_element_description_list_sort (GList *list_of_element_desc,
				    CongDispspec *dispspec);

const gchar*
cong_element_description_get_user_name (const CongElementDescription *element_desc);

void
cong_element_description_list_free (GList *list_of_element_desc);

CongElementDescription*
cong_util_modal_element_selection_dialog (const gchar *title, 
					  const gchar *description,
					  CongDocument *doc,
					  GList *elements);

GList*
cong_util_make_element_description_list (CongDocument *doc);

/* Simple UI manipulation: */
GtkMenuItem* 
cong_util_make_menu_item (const gchar *label,
			  const gchar *tip,
			  GdkPixbuf *pixbuf);
GtkMenuItem* 
cong_util_make_stock_menu_item (const gchar *stock_id);

GtkAction* 
cong_util_make_action_for_dispspec_element (const gchar* action_prefix,
					    CongDispspecElement *element);

GtkAction* 
cong_util_make_action_for_element_desc (const gchar* action_prefix,
					const CongElementDescription *element_desc,
					CongDocument *doc);

void
cong_util_add_menu_separator (CongPrimaryWindow *primary_window,
			      const gchar *parent_ui_path);


char *
cong_util_get_qualified_attribute_name(const xmlNs *namespace,
				       const gchar *local_attribute_name);

void
cong_util_show_in_window (GtkWidget *content,
			  const gchar *title);


GtkFileFilter*
cong_util_make_file_filter (const gchar *name,
			    const gchar* mime_type);


gint 
cong_util_get_byte_offset (const gchar *string, 
			   gint char_offset);

typedef struct _CongWord CongWord;
struct _CongWord
{
	int start_byte_offset;
	int length_in_bytes;
};

GList*
cong_util_get_words (PangoLanguage *language,
		     const gchar *string);

gboolean
cong_util_spellcheck_word (PangoLanguage *language,
			   const gchar *string,
			   const CongWord *word);

gboolean
cong_util_string_to_bool (const gchar *string,
			  gboolean default_value);

const gchar*
cong_util_bool_to_string (gboolean value);


GtkAction*
cong_action_new (const gchar *name,
		 const gchar *label,
		 const gchar *tooltip,
		 GdkPixbuf *pixbuf);

guint 
cong_str_or_null_hash (gconstpointer key);

gboolean
cong_str_or_null_equal (gconstpointer a,
			gconstpointer b);

gchar*
cong_util_dup_and_free_xml_string (xmlChar *xml_string);

xmlDocPtr
cong_util_new_xml_doc (void);

xmlNodePtr
cong_util_new_xml_element (xmlDocPtr xml_doc, const gchar* local_name);

void
cong_util_set_attribute_bool (xmlNodePtr xml_node, const gchar* name, gboolean value);

void
cong_util_set_attribute_int (xmlNodePtr xml_node, const gchar* name, int value);

char *
cong_util_expand_initial_tilde(const char *path);

char *
cong_util_format_file_size_for_display (gsize size);

G_END_DECLS

#endif
