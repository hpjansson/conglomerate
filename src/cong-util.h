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

/**
 * cong_util_cleanup_text:
 *
 * @text:  input UTF8 text
 *
 * Handy function for taking UTF8 text and turning it into something you can see in a log: tabs and carriage returns etc are turned into visible characters.
 *
 * Returns: a freshly-allocated string, with all tabs and carriage returns turned into their printf equivalents
 */
gchar* 
cong_util_cleanup_text (const xmlChar *text);

/**
 * cong_util_text_header
 *
 * @text:  input UTF8 text
 *
 * Handy function for taking UTF8 text and turning it into something you can use in short user-visible message: tabs and carriage returns etc are turned into spaces,
 * and it is truncated with an ellipsis if above a certain length.
 *
 * Returns: a freshly-allocated string, cleaned up as described above
 */
gchar* 
cong_util_text_header (const xmlChar *text,
		       guint truncation_length);

/**
 * cong_utils_get_norman_walsh_stylesheet_path:
 *
 *  Try to locate Norman Walsh's stylesheets for DocBook using the local catalog to find them.
 *
 * Returns:  a string containing the path (which the caller must delete), or NULL
 */
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

/**
 * cong_util_make_dtd:
 *
 * Make DTD declaration, and assigns it to the given document.  Doesn't add it to doc tree
 *
 * Returns: the #xmlDtdPtr that was assigned
 */
xmlDtdPtr
cong_util_make_dtd (xmlDocPtr xml_doc,
		    const xmlChar *root_element,
		    const xmlChar *ExternalID, 
		    const xmlChar *SystemID);

/**
 * cong_util_add_external_dtd:
 *
 * Make DTD declaration, assigns it to the given document, and add it to the tree.
 * Call cong_document_set_external_dtd() instead if you want notifications to work.
 *
 * Returns: the #xmlDtdPtr that was assigned
 */
xmlDtdPtr 
cong_util_add_external_dtd (xmlDocPtr xml_doc, 
			    const xmlChar *root_element,
			    const xmlChar *ExternalID, 
			    const xmlChar *SystemID);

/**
 * cong_util_run_add_dtd_dialog:
 *
 * Open a dialog for choosing a DTD to associate with the document
 * 
 */
void
cong_util_run_add_dtd_dialog (CongDocument *doc,
			      GtkWindow *parent_window);

/* Dodgy hack to do lines that blend to white: */
void 
cong_util_draw_blended_line (GtkWidget *w,
			     const GdkColor *col,
			     int x0, int y0,
			     int x1);

/**
 * cong_util_get_int_from_rgb_hex:
 * 
 * Parse a string containing an HTML-style hexadecimal representation of an RGB triplet
 * into a 32 bit RGB triplet.
 *
 * FIXME: it's fundamentally broken to be using ints for this; should use a struct
 *
 * Returns: RGB triplet packed into a 32-bit value
 */
unsigned int
cong_util_get_int_from_rgb_hex (const gchar *string);


/**
 * cong_util_ns_equality
 *
 * @xml_ns1: A namespace. Can be NULL.
 * @xml_ns2: Another namespace. Can be NULL.
 *
 * Compares the namespace URIs of both namespaces. 
 * The prefixes are not checked. If both are NULL
 * they are also equal.
 */
gboolean 
cong_util_ns_equality (const xmlNs *xml_ns1,
		       const xmlNs *xml_ns2);

/**
 * cong_util_ns_uri_equality:
 * 
 * Compare two namespace URIs, either or both of which can be NULL.
 *
 * Currently the comparison is an exact string comparison, which might be too strict,
 * see p118 of "Effective XML" for a discussion of ambiguities in the spec.
 *
 * Returns: TRUE if they are the same URI (or both NULL), FALSE otherwise
 *
 */
gboolean
cong_util_ns_uri_equality (const gchar* uri0, 
			   const gchar* uri1);

/**
 * cong_util_ns_uri_sort_order:
 * 
 * Compare two namespace URIs, either or both of which can be NULL.
 *
 * Currently the comparison is an exact string comparison, which might be too strict,
 * see p118 of "Effective XML" for a discussion of ambiguities in the spec.
 *
 * Returns: 0 if they are the same URI (or both NULL), otherwise positive or negative to give an ordering
 *
 */
gint
cong_util_ns_uri_sort_order (const gchar* uri0, 
			     const gchar* uri1);

/* 
 * cong_util_make_source_view:
 *
 * Make a scrollable source view for a programming language, with syntax highlighting if available.
 * The view is uneditable.
 * The widget is already "shown".
 *
 */
GtkWidget*
cong_util_make_source_view (const gchar *source_mime_type,
			    GtkTextView **output_text_view);

GtkWidget*
cong_source_view_new_full (CongDocument *doc,
			   const gchar *source_mime_type,
			   void (*regeneration_cb) (CongDocument *doc,
						    GtkTextBuffer *text_buffer));
/**
 * cong_util_attribute_value_equality:
 *
 * Compare two attribute value strings for equality; either or both might be NULL
 *
 * Returns TRUE if they are equal (i.e. the same string, or both are NULL)
 *
 */
gboolean
cong_util_attribute_value_equality (const gchar *value0,
				    const gchar *value1);

typedef struct CongElementDescription CongElementDescription;

struct CongElementDescription 
{
	gchar *ns_uri;
	gchar *local_name;
};

CongElementDescription*
cong_element_description_new (const gchar *ns_uri,
			      const gchar *local_name);

CongElementDescription*
cong_element_description_clone (const CongElementDescription *element_desc);

void
cong_element_description_free (CongElementDescription *element_desc);

CongNodePtr
cong_element_description_make_node (const CongElementDescription *element_desc,
				    CongDocument *doc,
				    CongNodePtr ns_search_node);

CongDispspecElement*
cong_element_description_get_dispspec_element_for_doc (const CongElementDescription *element_desc,
						       CongDocument *doc);

CongDispspecElement*
cong_element_description_get_dispspec_element_for_dispspec (const CongElementDescription *element_desc,
							    CongDispspec *ds);

#if 0
gchar*
cong_element_description_get_qualified_name (const CongElementDescription *element_desc);
#endif

void
cong_element_description_list_free (GList *list_of_element_desc);

/**
 * cong_util_modal_element_selection_dialog:
 *
 * @title: Title for the dialog
 * @description: Descriptive text for the dialog
 * @doc: The document, so that dispspecs can be searched for descriptions
 * @elements: A GList of #CongElementDescription
 *
 * Runs a modal element selection dialog.
 *
 * Returns: the selected element (which the caller must free), or NULL if the dialog was cancelled
 */
CongElementDescription*
cong_util_modal_element_selection_dialog (const gchar *title, 
					  const gchar *description,
					  CongDocument *doc,
					  GList *elements);

/* Simple UI manipulation: */
GtkMenuItem* 
cong_util_make_menu_item (const gchar *label,
			  const gchar *tip,
			  GdkPixbuf *pixbuf);
GtkMenuItem* 
cong_util_make_stock_menu_item (const gchar *stock_id);

GtkMenuItem* 
cong_util_make_menu_item_for_dispspec_element (CongDispspecElement *element);

GtkMenuItem* 
cong_util_make_menu_item_for_element_desc (const CongElementDescription *element_desc,
					   CongDocument *doc);

GtkWidget*
cong_util_add_menu_separator (GtkMenu *menu);


/**
 * cong_util_attribute_name_qualified
 *
 * @namespace: Namespace of attribute (can be NULL).
 * @local_attribute_name: Local name of attribute.
 *
 * Returns: 
 */
char *
cong_util_get_qualified_attribute_name(const xmlNs *namespace,
				       const xmlChar *local_attribute_name);

/**
 * cong_util_show_in_window:
 * 
 * Embed the chosen GtkWidget in an appropriate frame, with the application icon etc.
 * Anything using this probably needs some HIG love.
 *
 */
void
cong_util_show_in_window (GtkWidget *content,
			  const gchar *title);


GtkFileFilter*
cong_util_make_file_filter (const gchar *name,
			    const gchar* mime_type);


G_END_DECLS

#endif
