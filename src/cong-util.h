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
 * @text:  input xml text
 *
 * Handy function for taking xml text and turning it into something you can see in a log: tabs and carriage returns etc are turned into visible characters.
 *
 * Returns: a freshly-allocated string, with all tabs and carriage returns turned into their printf equivalents
 */
gchar* 
cong_util_cleanup_text (const xmlChar *text);


/**
   Convert a URI into a POSIX, path, assuming that this is valid: 
*/
gchar*
cong_util_get_local_path_from_uri (GnomeVFSURI *uri);

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

#if ENABLE_PRINTING
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

void
cong_util_set_cursor_to_first_text_descendant (CongDocument *doc,
					       CongNodePtr node);

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

void
cong_util_split_uri (const GnomeVFSURI* uri, 
		     gchar** filename_alone, 
		     gchar** path);

/**
 * cong_util_remove_tag:
 *
 * Function to remove a node "tag" from the tree; all its children become children of tag's parents in the natural place in the tree.
 */
void 
cong_util_remove_tag (CongDocument *doc, 
		      CongNodePtr tag);


/* Dodgy hack to do lines that blend to white: */
void 
cong_util_draw_blended_line (GtkWidget *w,
			     const GdkColor *col,
			     int x0, int y0,
			     int x1);
G_END_DECLS

#endif
