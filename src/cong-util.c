/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-util.c
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
 * Fragments of code based upon libxslt: numbers.c
 */

#include <sys/types.h>
#include <pwd.h>
#include "global.h"
#include "cong-util.h"
#include "cong-app.h"
#include "cong-document.h"
#include "cong-text-cache.h"
#include "cong-glade.h"
#include "cong-spell.h"

#include "cong-dispspec.h"
#include "cong-dispspec-element.h"

#include "cong-error-dialog.h"

#include "cong-ui-hooks.h"

#include "cong-file-selection.h"

#include "cong-command.h"
#include "cong-dispspec-registry.h"

#include "cong-primary-window.h"

#if ENABLE_PRINTING
#if 0
/* FIXME: use xmlroff eventually */
#include <libfo/fo-libfo.h>
#endif
#endif

/**
 * cong_util_is_docbook:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
cong_util_is_docbook (CongDocument *doc) 
{
	const gchar* dtd_public_id;

	g_return_val_if_fail(doc, FALSE);

	dtd_public_id = cong_document_get_dtd_public_identifier(doc);
	
	if (NULL==dtd_public_id) {
		return FALSE;
	}

	/* FIXME: we may want to add more public IDs here */
	if (0==strcmp(dtd_public_id, "-//OASIS//DTD DocBook XML V4.1.2//EN")) {
		return TRUE;
	} 

	/* FIXME:  this has been used by some of the GNOME docs; e.g. the GNOME accessibility guide; is it correct? */
	if (0==strcmp(dtd_public_id, "-//OASIS/DTD DocBookXML V4.1.2//EN")) {
		return TRUE;
	}


	return FALSE;
}

/**
 * cong_util_is_pure_whitespace:
 * @utf8_text:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
cong_util_is_pure_whitespace (const gchar *utf8_text)
{
	gunichar ch;

	g_return_val_if_fail(utf8_text, FALSE);

	while ( (ch = g_utf8_get_char(utf8_text)) ) {
		if (!g_unichar_isspace(ch)) {
			return FALSE;
		}

		utf8_text = g_utf8_next_char(utf8_text);
	}
	
	return TRUE;
}

/**
 * cong_util_cleanup_text:
 * @text:  input UTF8 text
 *
 * Handy function for taking UTF8 text and turning it into something you can see in a log: tabs and carriage returns etc are turned into visible characters.
 *
 * Returns: a freshly-allocated string, with all tabs and carriage returns turned into their printf equivalents
 */
gchar* 
cong_util_cleanup_text (const gchar *src_text) 
{
#if 0
	gchar *buffer;
	gchar *dst;
	gunichar unichar;
#endif

	g_return_val_if_fail(src_text, NULL);

	g_assert(g_utf8_validate(src_text, -1, NULL));

#if 1
	return g_strescape (src_text, "");
#else

	buffer = g_malloc((strlen(src_text)*6)+1); /* allow 6 bytes per character, plus a terminating byte; this SHOULD be big enough */
	dst = buffer;

	while (	(unichar = g_utf8_get_char(src_text)) ) {
		switch (unichar) {
		default:
			dst += g_unichar_to_utf8(unichar,dst);
			break;
		case '\n':
			*(dst++) = '\\';
			*(dst++) = 'n';
			break;
		case '\t':
			*(dst++) = '\\';
			*(dst++) = 't';
			break;
		}

		src_text = g_utf8_next_char(src_text);
	}

	*(dst++) = '\0';

	return buffer;
#endif
}

/**
 * cong_util_text_header
 * @text:  input UTF8 text
 * @truncation_length:
 *
 * Handy function for taking UTF8 text and turning it into something you can use in short user-visible message: tabs and carriage returns etc are turned into spaces,
 * and it is truncated with an ellipsis if above a certain length.
 *
 * Returns: a freshly-allocated string, cleaned up as described above
 */
gchar* 
cong_util_text_header (const gchar *text,
		       guint truncation_length)
{
	CongTextCache* text_cache;
	const gchar* stripped_text;
	gchar *result;

	text_cache = cong_text_cache_new (TRUE,
					  text,
					  NULL);

	stripped_text = cong_text_cache_get_output_text (text_cache);

	if (g_utf8_strlen (stripped_text, -1)>truncation_length) {
		gchar *truncated = g_strndup (stripped_text, truncation_length);

		result = g_strdup_printf ("%s...", truncated);

		g_free (truncated);
	} else {
		result = g_strdup (stripped_text);
	}

	cong_text_cache_free (text_cache);

	return result;
}

/**
 * cong_util_load_icon:
 * @icon_basename:
 *
 * TODO: Write me
 * Returns:
 */
GdkPixbuf*
cong_util_load_icon (const gchar *icon_basename)
{
	gchar *filename;
	gchar *full_path;
	GdkPixbuf *pixbuf;

	g_return_val_if_fail(icon_basename, NULL);

	filename = g_strdup_printf("conglomerate/pixmaps/%s-16.png", icon_basename);
	full_path = cong_app_locate_file (cong_app_singleton(),
					  filename);
	pixbuf = gdk_pixbuf_new_from_file(full_path, NULL);
	
	g_free(full_path);
	g_free(filename);

	return pixbuf;
}

/**
 * cong_util_append:
 * @string:
 * @to_add:
 *
 * TODO: Write me
 */
void 
cong_util_append (gchar **string, 
		  const gchar *to_add)
{
	gchar *new_string;

	g_return_if_fail(string);
	g_return_if_fail(*string);
	g_return_if_fail(to_add);

	new_string = g_strdup_printf("%s%s", *string, to_add);
	g_free(*string);

	*string = new_string;
}

/**
 * cong_util_prepend:
 * @string:
 * @to_add:
 *
 * TODO: Write me
 */
void 
cong_util_prepend (gchar **string, 
		  const gchar *to_add)
{
	gchar *new_string;

	g_return_if_fail(string);
	g_return_if_fail(*string);
	g_return_if_fail(to_add);

	new_string = g_strdup_printf("%s%s", to_add, *string);
	g_free(*string);

	*string = new_string;
}

#if (ENABLE_PRINTING && ENABLED_LIBFO)
/**
 * cong_util_print_xslfo:
 * @toplevel_window:
 * @gpc:
 * @xml_doc:
 *
 * TODO: Write me
 */
void 
cong_util_print_xslfo (GtkWindow *toplevel_window, 
		       GnomePrintContext *gpc,
		       xmlDocPtr xml_doc)
{
	g_return_if_fail(gpc);
	g_return_if_fail(xml_doc);

#if 0
	{
		/* FIXME: ultimately we probably want to use xmlroff to do this stage */
		FoDoc *fo_doc;
		FoFo *fo_tree;
		FoArea *area_tree;
		GError *error = NULL;

		fo_doc = fo_doc_gp_new ();

		/* FIXME:  need some way to pass our gpc or job to the FoDoc constructor; would a config do it? */

		fo_xml_doc_to_fo_and_area_trees (xml_doc,
						 fo_doc,
						 &fo_tree,
						 &area_tree,
						 0, /* gint debug_level */,
						 &error);

		/* FIXME: error handling! */
	}
#else

	CONG_DO_UNIMPLEMENTED_DIALOG_WITH_BUGZILLA_ID(toplevel_window, _("Printing XSL Formatting Objects"), 108468);

#if 0
	{
		FoPrintContext *fpc;
		FoParserResult *parser_result;
		FoSolverResult *solver_result;
		
		fpc = fo_print_context_new_from_gnome_print(gpc);
		
		parser_result = fo_parser_result_new_from_xmldoc(xml_doc);
		
		if (parser_result) {
			
#if 1
			/* View solver result: */
			solver_result = fo_solver_result_new_from_parser_result(parser_result);
			
			if (solver_result) {
				fo_solver_result_render(solver_result, fpc);
				
				fo_solver_result_delete(solver_result);
			}
#else
			/* View parser result: */
			fo_parser_result_test_render(parser_result, fpc);
#endif
			
			fo_parser_result_delete(parser_result);
			
		}
		
		fo_print_context_delete(fpc);
	}
#else
	/* Some test code: */
	{
		GnomeFont *font;
		font = gnome_font_find_closest ("Helvetica", 12);
		
		gnome_print_beginpage (gpc, "1");
		
		gnome_print_setfont (gpc, font);
		gnome_print_moveto (gpc, 100, 400);
		gnome_print_show (gpc, _("This will eventually be the result from cong_util_print_xslfo"));
		
		gnome_print_moveto (gpc, 100, 200);
		gnome_print_lineto (gpc, 200, 200);
		gnome_print_stroke (gpc);
		
		gnome_print_showpage (gpc);
	}
#endif
#endif


}
#endif

/**
 * cong_util_make_dtd:
 * @xml_doc:
 * @root_element:
 * @ExternalID:
 * @SystemID:
 *
 * Make DTD declaration, and assigns it to the given document.  Doesn't add it to doc tree
 *
 * Returns: the #xmlDtdPtr that was assigned
 */ 
xmlDtdPtr
cong_util_make_dtd (xmlDocPtr xml_doc,
		    const gchar *root_element,
		    const gchar *ExternalID, 
		    const gchar *SystemID)
{
#if 1
	xmlDtdPtr  dtd_ptr;

	/* g_message ("Trying to load DTD with PUBLIC \"%s\" SYSTEM \"%s\"", ExternalID, SystemID); */

	dtd_ptr = xmlParseDTD ((const xmlChar*)ExternalID, 
			       (const xmlChar*)SystemID);
	
	if (dtd_ptr) {
		/* g_message ("Succeeded"); */

		/* Then set the document and the root_element: */
		cong_node_recursive_set_doc ((CongNodePtr)dtd_ptr, 
					     xml_doc);

		if (dtd_ptr->name) {
			xmlFree ((char*)dtd_ptr->name);
		}
		dtd_ptr->name = xmlStrdup((const xmlChar*)root_element);

		/* Set up ptr within the xml_doc: */
		xml_doc->extSubset = dtd_ptr;

		/* Nullify the tree descendants; we have to do this otherwise the entire DTD is in the tree and appears in the editor view etc etc (and is very slow for large DTDs): */
		dtd_ptr->children = NULL;
		dtd_ptr->last = NULL;

		return dtd_ptr;
	} else {
		/* g_message ("Failed"); */

		return NULL;
	}
#else
	/* But this does not actually load the DTDs... */
	return xmlNewDtd(xml_doc,
			 root_element,
			 ExternalID,
			 SystemID);
#endif
}	

/**
   Make DTD declaration, assigns it to the given document, and add it to the tree.
*/
/**
 * cong_util_add_external_dtd:
 * @xml_doc:
 * @root_element:
 * @ExternalID:
 * @SystemID:
 *
 * Make DTD declaration, assigns it to the given document, and add it to the tree.
 * Call cong_document_set_external_dtd() instead if you want notifications to work.
 *
 * Returns: the #xmlDtdPtr that was assigned
 */
xmlDtdPtr 
cong_util_add_external_dtd (xmlDocPtr xml_doc, 
			    const gchar *root_element,
			    const gchar *ExternalID, 
			    const gchar *SystemID)
{
	xmlDtdPtr xml_dtd;

	g_return_val_if_fail(xml_doc, NULL);
	g_return_val_if_fail(root_element, NULL);

	xml_dtd = cong_util_make_dtd (xml_doc,
				      root_element,
				      ExternalID, 
				      SystemID);

	if (xml_dtd) {
		if (xml_doc->children) {
			/* FIXME: what if there already is a DTD? */
			xmlAddPrevSibling((xmlNodePtr)xml_doc->children,
					  (xmlNodePtr)xml_dtd);
		} else {
			xmlAddChild((xmlNodePtr)xml_doc,
				    (xmlNodePtr)xml_dtd);
		}

		/* Ensure the DTD ptr is still set up within the xml_doc; the tree manipulation seems to make it lose the extSubset pointer: */
		xml_doc->extSubset = xml_dtd;
	}

	return xml_dtd;
}

/* Dodgy blend func: */
static void blend_col(GdkColor *dst, const GdkColor *src0, const GdkColor *src1, float proportion)
{
	float one_minus = 1.0f - proportion;

	dst->red = ((float)src1->red*proportion) + ((float)src0->red*one_minus);
	dst->green = ((float)src1->green*proportion) + ((float)src0->green*one_minus);
	dst->blue = ((float)src1->blue*proportion) + ((float)src0->blue*one_minus);
}

static GdkColor white = {0, 0xffff, 0xffff, 0xffff};

/* Dodgy hack to do lines that blend to white: */
/**
 * cong_util_draw_blended_line:
 * @w:
 * @col:
 * @x0:
 * @y0:
 * @x1:
 *
 * TODO: Write me
 */
void 
cong_util_draw_blended_line (GtkWidget *w,
			     const GdkColor *col,
			     int x0, int y0,
			     int x1)
{
	/* inefficient: claim a gc to do this! (ideally we'd just invoke the hardware... grrr... arrg...) */
	GdkGC *gc = gdk_gc_new(w->window);
	int x;
	float proportion;
	GdkColor blended_col;

	g_assert(x0!=x1);

	for (x=x0; x<x1; x++) {
		proportion = (float)(x-x0)/(float)(x1-x0);
		proportion = (proportion>0.5f)?((proportion-0.5f)*2.0f):0.0f;

		blend_col(&blended_col, col, &white, proportion);

		gdk_colormap_alloc_color(cong_gui_get_a_window()->style->colormap, &blended_col, FALSE, TRUE);

		gdk_gc_set_foreground(gc,&blended_col);

		gdk_draw_point(w->window, gc, x, y0);
	}

	gdk_gc_unref(gc);
}

/**
 * cong_util_get_int_from_rgb_hex:
 * @string:
 * 
 * Parse a string containing an HTML-style hexadecimal representation of an RGB triplet
 * into a 32 bit RGB triplet.
 *
 * FIXME: it's fundamentally broken to be using ints for this; should use a struct
 *
 * Returns: RGB triplet packed into a 32-bit value
 */
unsigned int
cong_util_get_int_from_rgb_hex (const gchar *string)
{
	unsigned int col;
	
	col = 0;
	while (*string) {
		gchar ch = *(string++);

		col <<= 4;

		if (g_ascii_isalpha (ch)) {
			col |= (g_ascii_tolower (ch) - 'a' + 10);
		} else if (g_ascii_isdigit (ch)) {
			col |= (ch - '0');
		}
	}
	
	return(col);
}

/**
 * cong_util_ns_equality
 * @xml_ns1: A namespace. Can be NULL.
 * @xml_ns2: Another namespace. Can be NULL.
 *
 * Compares the namespace URIs of both namespaces. 
 * The prefixes are not checked. If both are NULL
 * they are also equal.
 * Returns:
 */
gboolean
cong_util_ns_equality (const xmlNs *xml_ns1,
		       const xmlNs *xml_ns2)
{
	if (xml_ns1 == NULL && xml_ns2 == NULL)
		return TRUE;

	if (xml_ns1 == NULL || xml_ns2 == NULL)
		return FALSE;

	return cong_util_ns_uri_equality ((const gchar*)xml_ns1->href, 
					  (const gchar*)xml_ns2->href);
}

/**
 * cong_util_ns_uri_equality:
 * @uri0:
 * @uri1:
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
			   const gchar* uri1)
{
	return 0==cong_util_ns_uri_sort_order (uri0, 
					       uri1);

}

/**
 * cong_util_ns_uri_sort_order:
 * @uri0:
 * @uri1:
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
			     const gchar* uri1)
{
	/* Names are the same; continue searching by namespace; order the NULL namespace before all others: */
	if (NULL == uri0) {
		if (NULL == uri1) {
			return 0;
		} else {
			return -1; /* a is less than b */
		}
	} else {
		/* "a" has non-NULL namespace: */
		if (NULL == uri1) {
			return 1; /* a is greater than b */
		} else {
			/* Both have non-NULL namespaces; order based on them: */
			/* FIXME: do we need a less strict comparison? */
			return strcmp(uri0, uri1);
		}
	}

}

/**
 * cong_util_attribute_value_equality:
 * @value0:
 * @value1:
 *
 * Compare two attribute value strings for equality; either or both might be NULL
 *
 * Returns: TRUE if they are equal (i.e. the same string, or both are NULL)
 */
gboolean
cong_util_attribute_value_equality (const gchar *value0,
				    const gchar *value1)
{
	if (value0) {
		if (value1) {
			return 0==strcmp (value0,value1);
		} else {
			return FALSE;
		}
	} else {
		if (value1) {
			return FALSE;
		} else {
			return TRUE;
		}
	}
}

/**
 * cong_element_description_new:
 * @ns_uri:
 * @local_name:
 *
 * TODO: Write me
 * Returns:
 */
CongElementDescription*
cong_element_description_new (const gchar *ns_uri,
			      const gchar *local_name)
{
	CongElementDescription *element_desc;

	g_return_val_if_fail (local_name, NULL);

	element_desc = g_new0 (CongElementDescription, 1);

	if (ns_uri) {
		element_desc->ns_uri = g_strdup (ns_uri);
	}
	element_desc->local_name = g_strdup (local_name);

	return element_desc;
}

/**
 * cong_element_description_new_from_node:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongElementDescription*
cong_element_description_new_from_node (CongNodePtr node)
{
	g_return_val_if_fail (node, NULL);
	g_return_val_if_fail (node->type == XML_ELEMENT_NODE, NULL);

	if (node->ns) {
		return cong_element_description_new ((const gchar*)node->ns->href,
						     (const gchar*)node->name);
	} else {
		return cong_element_description_new (NULL,
						     (const gchar*)node->name);
	}
}
				
/**
 * cong_element_description_clone:
 * @element_desc:
 *
 * TODO: Write me
 * Returns:
 */
CongElementDescription*
cong_element_description_clone (const CongElementDescription *element_desc)
{
	CongElementDescription *new_element_desc;

	g_return_val_if_fail (element_desc, NULL);

	new_element_desc = g_new0 (CongElementDescription, 1);

	if (element_desc->ns_uri) {
		new_element_desc->ns_uri = g_strdup (element_desc->ns_uri);
	}
	new_element_desc->local_name = g_strdup (element_desc->local_name);

	return new_element_desc;
}

/**
 * cong_element_description_free:
 * @element_desc:
 *
 * TODO: Write me
 */
void
cong_element_description_free (CongElementDescription *element_desc)
{
	g_return_if_fail (element_desc);

	if (element_desc->ns_uri) {
		g_free (element_desc->ns_uri);
	}
	g_free (element_desc->local_name);
}

/**
 * cong_element_description_make_node:
 * @element_desc:
 * @doc:
 * @ns_search_node:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr
cong_element_description_make_node (const CongElementDescription *element_desc,
				    CongDocument *doc,
				    CongNodePtr ns_search_node)
{
	xmlNsPtr xml_ns;
	CongNodePtr new_node;
	
	g_return_val_if_fail (element_desc, NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	g_return_val_if_fail (ns_search_node, NULL);

	/* FIXME:  what if the namespace doesn't exist in the document yet? */
	xml_ns = cong_node_get_ns_for_uri (ns_search_node,
					   element_desc->ns_uri);
	new_node = cong_node_new_element (xml_ns,
					  element_desc->local_name,
					  doc);
	return new_node;	
}

/**
 * cong_element_description_matches_node:
 * @element_desc:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_element_description_matches_node (const CongElementDescription *element_desc,
				       CongNodePtr node)
{
	g_return_val_if_fail (element_desc, FALSE);
	g_return_val_if_fail (node, FALSE);

	
	if (node->type!=XML_ELEMENT_NODE) {
		return FALSE;
	}

	if (0!=strcmp((const char*)node->name, element_desc->local_name)) {
		return FALSE;
	}

	if (node->ns) {
		return cong_util_ns_uri_equality ((const gchar*)node->ns->href, element_desc->ns_uri);
	} else {
		return (NULL == element_desc->ns_uri);
	}
}


/**
 * cong_element_description_get_dispspec_element_for_doc:
 * @element_desc:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspecElement*
cong_element_description_get_dispspec_element_for_doc (const CongElementDescription *element_desc,
						       CongDocument *doc)
{
	/* CongDispspec *ds; */

	g_return_val_if_fail (element_desc, NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);

	return cong_dispspec_registry_get_dispspec_element_for_description (cong_app_get_dispspec_registry (cong_app_singleton ()),
									    element_desc,
									    cong_document_get_default_dispspec (doc));
}

/**
 * cong_element_description_get_dispspec_element_for_dispspec:
 * @element_desc:
 * @ds:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspecElement*
cong_element_description_get_dispspec_element_for_dispspec (const CongElementDescription *element_desc,
							    CongDispspec *ds)
{
	g_return_val_if_fail (element_desc, NULL);
	g_return_val_if_fail (ds, NULL);

	return cong_dispspec_lookup_element (ds, 
					     element_desc->ns_uri, 
					     element_desc->local_name);
}

#if 0
gchar*
cong_element_description_get_qualified_name (const CongElementDescription *element_desc)
{
	g_return_val_if_fail (element_desc, NULL);

	return ;
}
#endif

gchar*
cong_element_description_make_user_name (const CongElementDescription *element_desc,
					 CongDispspec *ds)
{
	g_return_val_if_fail (element_desc, NULL);
	
	if (ds) {
		CongDispspecElement* ds_element = cong_element_description_get_dispspec_element_for_dispspec (element_desc,
													      ds);

		if (ds_element) {
			return g_strdup (cong_dispspec_element_username (ds_element));
		}
	}
	
	if (element_desc->ns_uri) {
		return g_strdup_printf("<%s xmlns=\"%s\"/>", element_desc->local_name, element_desc->ns_uri);
	} else {
		return g_strdup_printf("<%s />", element_desc->local_name);
	}
}

const gchar*
cong_element_description_get_sort_string (const CongElementDescription *element_desc,
					  CongDispspec *ds)
{
	g_return_val_if_fail (element_desc, NULL);
	
	if (ds) {
		CongDispspecElement* ds_element = cong_element_description_get_dispspec_element_for_dispspec (element_desc,
													      ds);

		if (ds_element) {
			if (cong_dispspec_element_username (ds_element)) {
				return cong_dispspec_element_username (ds_element);
			} else {
				return cong_dispspec_element_get_local_name (ds_element);
			}
		}
	}
	
	return element_desc->local_name;
}

static gint 
element_desc_compare_func (gconstpointer a, 
			   gconstpointer b,
			   gpointer user_data)
{
	CongDispspec *ds = (CongDispspec *)user_data;
	const CongElementDescription *element_desc_a;
	const CongElementDescription *element_desc_b;
	const gchar *sort_a;
	const gchar *sort_b;
	gchar *folded_a;
	gchar *folded_b;
	gint result;

	element_desc_a = (const CongElementDescription *)a;
	element_desc_b = (const CongElementDescription *)b;

	sort_a = cong_element_description_get_sort_string (element_desc_a,
							   ds);
	sort_b = cong_element_description_get_sort_string (element_desc_b,
							   ds);

	g_assert(sort_a);
	g_assert(sort_b);

	/* g_message("comparing \"%s\" and \"%s\"", sort_a, sort_b); */

	folded_a = g_utf8_casefold(sort_a,-1);
	folded_b = g_utf8_casefold(sort_b,-1);
	result = g_utf8_collate(folded_a, folded_b);

	g_free(folded_a);
	g_free(folded_b);

	return result;
}


GList*
cong_element_description_list_sort (GList *list_of_element_desc,
				    CongDispspec *dispspec)
{
	/* Sort the list into alphabetical order of user-visible names: */
	return g_list_sort_with_data (list_of_element_desc, 
				      element_desc_compare_func,
				      dispspec);
}



/**
 * cong_element_description_list_free:
 * @list_of_element_desc:
 *
 * TODO: Write me
 */
void
cong_element_description_list_free (GList *list_of_element_desc)
{
	GList *iter;

	for (iter=list_of_element_desc; iter; iter=iter->next) {
		cong_element_description_free ((CongElementDescription*)iter->data);
	}
	g_list_free (list_of_element_desc);
}

enum {
	FIELD_ELEMENT_DESC_PTR,
	FIELD_DS_PIXBUF,
	FIELD_DS_USER_VISIBLE_NAME,
	FIELD_ELEMENT_LOCAL_NAME,

	NUM_FIELDS
};

/**
 * sort_func_ds_user_visible_name:
 * @model:
 * @a:
 * @b:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
gint
sort_func_ds_user_visible_name (GtkTreeModel *model,
				GtkTreeIter *a,
				GtkTreeIter *b,
				gpointer user_data)
{
	gint result;

	gchar *val_a = NULL;
	gchar *val_b = NULL;

	gtk_tree_model_get (model,
			    a,
			    FIELD_DS_USER_VISIBLE_NAME, &val_a, 
			    -1);
	gtk_tree_model_get (model,
			    b,
			    FIELD_DS_USER_VISIBLE_NAME, &val_b, 
			    -1);

	/* Sort all empty strings after non-empty strings: */
	if (0==strcmp (val_a, "")) {
		if (0==strcmp (val_b, "")) {
			result = 0;
		} else {
			result = 1;
		}
	} else {
		if (0==strcmp (val_b, "")) {
			result = -1;
		} else {
			result = strcmp (val_a, val_b);
		}
	}

	g_free (val_a);
	g_free (val_b);

	return result;
}

/**
 * sort_func_element_local_name:
 * @model:
 * @a:
 * @b:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
gint
sort_func_element_local_name (GtkTreeModel *model,
			      GtkTreeIter *a,
			      GtkTreeIter *b,
			      gpointer user_data)
{
	gint result;

	gchar *val_a = NULL;
	gchar *val_b = NULL;

	gtk_tree_model_get (model,
			    a,
			    FIELD_ELEMENT_LOCAL_NAME, &val_a, 
			    -1);
	gtk_tree_model_get (model,
			    b,
			    FIELD_ELEMENT_LOCAL_NAME, &val_b, 
			    -1);

	result = strcmp (val_a, val_b);

	g_free (val_a);
	g_free (val_b);

	return result;
}

static void
selection_changed_cb (GtkTreeSelection *selection,
		      gpointer user_data)
{
	GladeXML *xml = GLADE_XML (user_data);
	
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "okbutton"),
				  gtk_tree_selection_get_selected (selection, NULL, NULL));
}

/**
 * cong_util_modal_element_selection_dialog:
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
					  GList *elements)
{
	GladeXML *xml;
	GtkWidget *dialog, *label, *tree_view;
	GtkListStore *list_store;
	GtkTreeSelection *selection;
	CongElementDescription *result;

	g_return_val_if_fail (title, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	g_return_val_if_fail (elements, NULL);

	xml = cong_util_load_glade_file ("conglomerate/glade/string_selection_dialog.glade",
					 NULL,
					 doc,
					 NULL);
	dialog = glade_xml_get_widget (xml, "string_selection_dialog");
	label = glade_xml_get_widget (xml, "label");
	tree_view = glade_xml_get_widget (xml, "treeview1");
	
        gtk_window_set_title (GTK_WINDOW (dialog), 
			      title);
	gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	gtk_label_set_text (GTK_LABEL (label), 
			    description);

	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "okbutton"),
				  FALSE);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (selection), "changed",
			  G_CALLBACK (selection_changed_cb),
			  xml);

	/* Set up list view columns */
	{
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *column;

		/* First column: */
		{
			column = gtk_tree_view_column_new ();
			renderer = gtk_cell_renderer_pixbuf_new ();
			gtk_tree_view_column_pack_start (column, renderer, FALSE);
			gtk_tree_view_column_set_attributes (column,
							     renderer,
							     "pixbuf", FIELD_DS_PIXBUF,
							     NULL);
			
			renderer = gtk_cell_renderer_text_new ();
			gtk_tree_view_column_pack_start (column, renderer, FALSE);
			gtk_tree_view_column_set_attributes (column,
							     renderer,
							     "text", FIELD_DS_USER_VISIBLE_NAME,
							     NULL);			

			gtk_tree_view_column_set_sort_column_id (column,
								 FIELD_DS_USER_VISIBLE_NAME);
			
			gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
						     column);
		}

		/* Second column: */
		{
			column = gtk_tree_view_column_new ();
			renderer = gtk_cell_renderer_text_new ();
			gtk_tree_view_column_pack_start (column, renderer, FALSE);
			gtk_tree_view_column_set_attributes (column,
							     renderer,
							     "text", FIELD_ELEMENT_LOCAL_NAME,
							     NULL);

			gtk_tree_view_column_set_sort_column_id (column,
								 FIELD_ELEMENT_LOCAL_NAME);
			
			gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
						     column);
		}
	}
		
	/* Set up and populate list model: */
	{
		GList *iter;

		list_store = gtk_list_store_new (NUM_FIELDS,
						 G_TYPE_POINTER,
						 GDK_TYPE_PIXBUF,
						 G_TYPE_STRING,
						 G_TYPE_STRING);

		gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (list_store),
						 FIELD_DS_USER_VISIBLE_NAME,
						 sort_func_ds_user_visible_name,
						 NULL,
						 NULL);
		gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (list_store),
						 FIELD_ELEMENT_LOCAL_NAME,
						 sort_func_element_local_name,
						 NULL,
						 NULL);

		for (iter = elements; iter; iter=iter->next) {
			CongElementDescription *element_desc = (CongElementDescription *)iter->data;
			GtkTreeIter iter;
			gchar *tagged_name;
			CongDispspecElement *ds_element = cong_element_description_get_dispspec_element_for_doc (element_desc,
														 doc);
			gtk_list_store_append (list_store, &iter);

			tagged_name = g_strdup_printf ("<%s>", element_desc->local_name);

			gtk_list_store_set (list_store, &iter,
					    FIELD_ELEMENT_DESC_PTR, element_desc,
					    FIELD_ELEMENT_LOCAL_NAME, tagged_name,
					    -1);
			g_free (tagged_name);

			if (ds_element) {
				GdkPixbuf* icon = cong_dispspec_element_get_icon (ds_element);

				gtk_list_store_set (list_store, &iter,
						    FIELD_DS_USER_VISIBLE_NAME, cong_dispspec_element_username(ds_element),
						    -1);

				if (icon) {
					gtk_list_store_set (list_store, &iter,
							    FIELD_DS_PIXBUF, icon,
							    -1);
					g_object_unref (G_OBJECT (icon));
				}
			} else {
				gtk_list_store_set (list_store, &iter,
						    FIELD_DS_USER_VISIBLE_NAME, "",
						    -1);
			}
		}

		gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view),
					 GTK_TREE_MODEL (list_store));
		g_object_unref (G_OBJECT (list_store));
	}

	result = NULL;
	
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
	{
		/* Get selection: */
		CongElementDescription *element_desc;
		GtkTreeIter iter;

		if (gtk_tree_selection_get_selected (selection, 
						     NULL, 
						     &iter)) {
			gtk_tree_model_get (GTK_TREE_MODEL (list_store), 
					    &iter, 
					    FIELD_ELEMENT_DESC_PTR, &element_desc, 
					    -1);

			/* need to clone: */
			result = cong_element_description_clone (element_desc);
		}
	}
				
	gtk_widget_destroy(GTK_WIDGET(dialog));
        g_object_unref(G_OBJECT(xml));

	return result;
}

static gboolean 
make_element_description_list_callback (CongDocument *doc, CongNodePtr node, gpointer user_data, guint recursion_level)
{
	GList **list = (GList**)user_data;

	if (node->type == XML_ELEMENT_NODE) {
		/* Check it's not already present: */
		GList *iter = *list;

		while (iter) {
			const CongElementDescription *element_desc = (const CongElementDescription*)iter->data;

			if (cong_element_description_matches_node (element_desc,
								   node)) {
				/* Got a match; bail out: */
				return FALSE;
			}
			iter=iter->next;
		}

		/* Nothing matched: add to the list: */
		*list = g_list_prepend (*list,
					cong_element_description_new_from_node (node));
	}

	return FALSE;
}

/**
 * cong_util_make_element_description_list:
 * @doc:
 *
 * Make a list of all CongElementDescription already in the document
 * Returns: a list of CongElementDescription
 */
GList*
cong_util_make_element_description_list (CongDocument *doc)
{
	/* FIXME: this will be slow (bugzilla 145026)*/
	GList *list = NULL;

	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);

	cong_document_for_each_node (doc, 
				     make_element_description_list_callback,
				     &list);

	return list;	
}

/**
 * cong_util_make_menu_item:
 * @label:
 * @tip:
 * @pixbuf:
 *
 * TODO: Write me
 * Returns:
 */
GtkMenuItem* 
cong_util_make_menu_item (const gchar *label,
			  const gchar *tip,
			  GdkPixbuf *pixbuf)
{
	GtkWidget *item;

	g_return_val_if_fail(label, NULL);

	item = gtk_image_menu_item_new_with_label (label);

	if (pixbuf) {
		GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
		gtk_widget_show(image);
	}

	if (tip) {
		gtk_widget_set_tooltip_text (GTK_WIDGET(item), tip);
	}

	return GTK_MENU_ITEM(item);
}

/**
 * cong_util_make_stock_menu_item:
 * @stock_id:
 *
 * TODO: Write me
 * Returns:
 */
GtkMenuItem* 
cong_util_make_stock_menu_item (const gchar *stock_id)
{
	g_return_val_if_fail (stock_id, NULL);

	return GTK_MENU_ITEM (gtk_image_menu_item_new_from_stock(stock_id,
								 NULL));
}

static gchar*
make_action_name (const gchar *action_prefix,
		  const gchar *ns_uri,
		  const gchar *local_name)
{
	g_assert (action_prefix);
	g_assert (local_name);

	if (ns_uri) {
		return g_strdup_printf ("%s-<%s xmlns=\"%s\">", action_prefix, local_name, ns_uri);
	} else {
		return g_strdup_printf ("%s-<%s>", action_prefix, local_name);
	}
}

/**
 * cong_util_make_action_for_dispspec_element:
 * @action_prefix:
 * @element:
 *
 * TODO: Write me
 * Returns:
 */
GtkAction* 
cong_util_make_action_for_dispspec_element (const gchar* action_prefix,
					    CongDispspecElement *element)
{
	gchar *action_name;
	GtkAction *action;
	GdkPixbuf *pixbuf;
	const gchar *tip;

	g_return_val_if_fail (action_prefix, NULL);
	g_return_val_if_fail (element, NULL);

	action_name = make_action_name (action_prefix,
					cong_dispspec_element_get_ns_uri (element),
					cong_dispspec_element_get_local_name (element));

	pixbuf = cong_dispspec_element_get_icon (element);
	tip = cong_dispspec_element_get_description (element);

	if (NULL==tip) {
		tip = _("(no description available)");
	}
	
	action = cong_action_new (action_name,
				  cong_dispspec_element_username (element),
				  tip,
				  pixbuf);
	g_free (action_name);
	if (pixbuf) {
		g_object_unref (G_OBJECT (pixbuf));
	}

	return action;
}

/**
 * cong_util_make_action_for_element_desc:
 * @action_prefix:
 * @element_desc:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
GtkAction* 
cong_util_make_action_for_element_desc (const gchar* action_prefix,
					const CongElementDescription *element_desc,
					CongDocument *doc)
{
	CongDispspecElement* ds_element;

	g_return_val_if_fail (action_prefix, NULL);
	g_assert (element_desc);
	g_assert (IS_CONG_DOCUMENT (doc));

	ds_element = cong_element_description_get_dispspec_element_for_doc (element_desc,
									    doc);
	if (ds_element) {
		return cong_util_make_action_for_dispspec_element (action_prefix,
								   ds_element);
	} else {
		gchar *action_name = make_action_name (action_prefix,
						       element_desc->ns_uri,
						       element_desc->local_name);
		gchar *username = cong_element_description_make_user_name (element_desc,
									   NULL);

		GtkAction *action = cong_action_new (action_name,
						     username,
						     NULL,
						     NULL);
		g_free (username);
		g_free (action_name);

		return action;
	}
}

/**
 * cong_util_add_menu_separator:
 * @primary_window:
 * @parent_ui_path:
 *
 * Adds a separator to a menu.
 */
void
cong_util_add_menu_separator (CongPrimaryWindow *primary_window,
			      const gchar *parent_ui_path)
{
	GtkUIManager *ui_manager = cong_primary_window_get_ui_manager (primary_window);
	gtk_ui_manager_add_ui (ui_manager,
			       gtk_ui_manager_new_merge_id (ui_manager),
			       parent_ui_path,
			       "dummy-name", /* FIXME: what should go here? */
			       NULL,
			       GTK_UI_MANAGER_SEPARATOR,
			       FALSE);
}

/**
 * cong_util_get_qualified_attribute_name:
 * @namespace: Namespace of attribute (can be NULL).
 * @local_attribute_name: Local name of attribute.
 *
 * Returns: 
 */
char *
cong_util_get_qualified_attribute_name(const xmlNs *namespace,
				       const gchar *local_attribute_name)
{
	g_return_val_if_fail(local_attribute_name != NULL, NULL);

	if(namespace == NULL ||
	   namespace->prefix == NULL ||
	   namespace->prefix[0] == '\0')
		return g_strdup(local_attribute_name);

	return g_strdup_printf("%s:%s",
			       namespace->prefix,
			       local_attribute_name);
}



/**
 * cong_util_show_in_window:
 * @content:
 * @title:
 * 
 * Embed the chosen GtkWidget in an appropriate frame, with the application icon etc.
 * Anything using this probably needs some HIG love.
 *
 */
void
cong_util_show_in_window (GtkWidget *content,
			  const gchar *title)
{
	GtkWidget *window;

	g_return_if_fail (content);
	g_return_if_fail (title);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), title);
	gtk_container_add(GTK_CONTAINER(window), content);
	gtk_window_set_icon_name(GTK_WINDOW(window), "conglomerate");
	
	/* Set up the window nicely: */
	gtk_window_set_default_size(GTK_WINDOW(window),
				    500,
				    400);

	gtk_widget_show_all(GTK_WIDGET(window));
}

GtkFileFilter*
cong_util_make_file_filter (const gchar *name,
			    const gchar* mime_type)
{
	GtkFileFilter *filter;

	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (mime_type, NULL);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, name);
	gtk_file_filter_add_mime_type (filter, mime_type);

	return filter;
}

/**
 * cong_util_run_add_dtd_dialog:
 * @doc:
 * @parent_window:
 *
 * Open a dialog for choosing a DTD to associate with the document
 * 
 */
void
cong_util_run_add_dtd_dialog (CongDocument *doc,
			      GtkWindow *parent_window)
{
	GFile *dtd_file;
	GList *list_of_filters;

	g_return_if_fail (doc);

	list_of_filters = g_list_append (NULL, cong_util_make_file_filter (_("DTD files"), 
									   "text/x-dtd"));
	
	dtd_file = cong_get_file_name (_("Select a DTD"),
	                               NULL,
	                               parent_window,
	                               CONG_FILE_CHOOSER_ACTION_OPEN,
	                               list_of_filters);

	if (dtd_file) {
		CongCommand *cmd = cong_document_begin_command (doc,
								_("Associate with DTD"),
								NULL);
		char *dtd_uri = g_file_get_uri(dtd_file);
		cong_command_add_set_external_dtd (cmd,
						   (const gchar*)cong_document_get_root_element (doc)->name,
						   NULL,
						   dtd_uri);
		cong_document_end_command (doc,
					   cmd);
		
		g_free(dtd_uri);
		g_object_unref (dtd_file);
	}
}


gint 
cong_util_get_byte_offset (const gchar *string, 
			   gint char_offset)
{
	const gchar *result_pos;

	g_return_val_if_fail (string, 0);
	g_return_val_if_fail (char_offset>=0, 0);

	result_pos = g_utf8_offset_to_pointer(string, char_offset);
	g_assert(result_pos);

	return result_pos - string;
}

GList*
cong_util_get_words (PangoLanguage *language,
		     const gchar *string)
{
	GList *result = NULL;
	PangoLogAttr *pango_log_attrs = NULL;
	int attrs_len = 0;
	int start_of_word_char_index;

	g_return_val_if_fail (string, NULL);

	attrs_len = g_utf8_strlen (string,-1)+1;
	pango_log_attrs = g_new (PangoLogAttr, attrs_len);
	
	pango_get_log_attrs (string,
			     strlen(string), /* length in bytes */
			     -1,
			     language,
			     pango_log_attrs,
			     attrs_len);

	start_of_word_char_index = 0;

	while (start_of_word_char_index < attrs_len) {
		/* Find start of word: */
		if (!pango_log_attrs[start_of_word_char_index].is_word_start) {
			start_of_word_char_index++;
		} else {
			int length = 1;

			while (start_of_word_char_index+length < attrs_len) {
				/* Find end of word: */
				if (!pango_log_attrs[start_of_word_char_index + length].is_word_end) {
				       length++;
				} else {
					/* Got a word: */
					CongWord *new_word = g_new0 (CongWord, 1);
					new_word->start_byte_offset = cong_util_get_byte_offset (string, 
												 start_of_word_char_index);
					new_word->length_in_bytes = cong_util_get_byte_offset (string, 
											       start_of_word_char_index+length) - new_word->start_byte_offset;
					result = g_list_append (result,
								new_word);
					start_of_word_char_index+=length;
					length = 0;
					break;
				}
			}

			/* Finish outer loop if no word found: */
			if (length>0) {
				break;
			}
			
		}
	}

	return result;
}

/* Return TRUE if there's an error */
gboolean
cong_util_spellcheck_word (PangoLanguage *language,
			   const gchar *string,
			   const CongWord *word)
{
	g_return_val_if_fail (string, FALSE);
	g_return_val_if_fail (word, FALSE);

#if 0
	g_message ("Spellchecking: language=\"%s\" string=\"%s\" offset:%i ,length:%i", pango_language_to_string(language), string, word->start_byte_offset, word->length_in_bytes);
#endif

#if ENABLE_ENCHANT
	return cong_is_word_misspelt(string, word);
#else
	return FALSE;
#endif
}

gboolean
cong_util_string_to_bool (const gchar *string, 
			  gboolean default_value)
{
	g_return_val_if_fail (string, default_value);

	if (0==strcmp(string, "yes")) {
		return TRUE;
	}
	if (0==strcmp(string, "no")) {
		return FALSE;
	}
	return default_value;
}

const gchar*
cong_util_bool_to_string (gboolean value)
{
	return value?"yes":"no";
}
		

guint 
cong_str_or_null_hash (gconstpointer key)
{
	if (key) {
		return g_str_hash (key);
	} else {
		return 0;
	}
}

/**
 * cong_str_or_null_equal:
 * @a:
 * @b:
 *
 * TODO: Write me
 */
gboolean
cong_str_or_null_equal (gconstpointer a,
			gconstpointer b)
{
	if (a) {
		if (b) {
			return g_str_equal (a, b);
		}
	}

	return (a==b);
}

/**
 * cong_util_dup_and_free_xml_string
 * @xml_string:
 *
 * Returns: makes a copy of the string using the GLib allocator, and frees the input using the libxml2 deallocator
 */
gchar*
cong_util_dup_and_free_xml_string (xmlChar *xml_string)
{
	gchar *result = g_strdup ((const gchar*)xml_string);
	xmlFree (xml_string);
	return result;
}

xmlDocPtr
cong_util_new_xml_doc (void)
{
	return xmlNewDoc((const xmlChar*)"1.0");
}

xmlNodePtr
cong_util_new_xml_element (xmlDocPtr xml_doc, const gchar* local_name)
{
	g_return_val_if_fail (xml_doc, NULL);
	g_return_val_if_fail (local_name, NULL);

	return xmlNewDocNode(xml_doc,
			     NULL, /* xmlNsPtr ns, */
			     (const xmlChar*)local_name,
			     NULL);
}

void
cong_util_set_attribute_bool (xmlNodePtr xml_node, const gchar* name, gboolean value)
{
	g_return_if_fail (xml_node);
	g_return_if_fail (name);

	xmlSetProp (xml_node, (const xmlChar*)name, (const xmlChar*)cong_util_bool_to_string (value));

}

void
cong_util_set_attribute_int (xmlNodePtr xml_node, const gchar* name, int value)
{
	gchar *textual_value;

	g_return_if_fail (xml_node);
	g_return_if_fail (name);

	textual_value = g_strdup_printf("%i", value);
	xmlSetProp (xml_node, (const xmlChar*)name, (const xmlChar*)textual_value);
	g_free(textual_value);
}

/* From gnome-vfs-utils.c */

/**
 * cong_util_expand_initial_tilde:
 * @path: a local file path which may start with a '~'.
 *
 * If @path starts with a ~, representing the user's home
 * directory, expand it to the actual path location.
 *
 * Return value: a newly allocated string with the initial
 * tilde (if there was one) converted to an actual path.
 */
char *
cong_util_expand_initial_tilde(const char *path)
{
	char *slash_after_user_name, *user_name;
	struct passwd *passwd_file_entry;

	g_return_val_if_fail(path != NULL, NULL);

	if(path[0] != '~')
		return g_strdup(path);

	if(path[1] == '/' || path[1] == '\0')
		return g_strconcat(g_get_home_dir(), &path[1], NULL);

	slash_after_user_name = strchr(&path[1], '/');
	if(slash_after_user_name == NULL)
		user_name = g_strdup(&path[1]);
	else
		user_name = g_strndup(&path[1], slash_after_user_name - &path[1]);

	passwd_file_entry = getpwnam(user_name);
	g_free(user_name);

	if(passwd_file_entry == NULL || passwd_file_entry->pw_dir == NULL)
		return g_strdup(path);

	return g_strconcat(passwd_file_entry->pw_dir, slash_after_user_name, NULL);
}

/**
 * cong_util_format_file_size_for_display:
 * @size: a #gsize.
 *
 * Formats the file @size passed so that it is easy for
 * the user to read. Gives the size in bytes, kilobytes, megabytes, or
 * gigabytes, choosing whatever is appropriate.
 *
 * Returns: a newly allocated string with the size ready to be shown.
 */
#define KILOBYTE_FACTOR 1024.0
#define MEGABYTE_FACTOR 1024.0 * KILOBYTE_FACTOR
#define GIGABYTE_FACTOR 1024.0 * MEGABYTE_FACTOR
char *
cong_util_format_file_size_for_display (gsize size)
{
	if (size < (gsize)KILOBYTE_FACTOR) {
		return g_strdup_printf (dngettext(GETTEXT_PACKAGE, "%u byte", "%u bytes", (unsigned) size), (unsigned) size);
	} else {
		double displayed_size;

		if (size < (gsize) MEGABYTE_FACTOR) {
			displayed_size = (double) size / KILOBYTE_FACTOR;
			return g_strdup_printf (_("%.1f KB"),
						       displayed_size);
		} else if (size < (gsize) GIGABYTE_FACTOR) {
			displayed_size = (double) size / MEGABYTE_FACTOR;
			return g_strdup_printf (_("%.1f MB"),
						       displayed_size);
		} else  {
			displayed_size = (double) size / GIGABYTE_FACTOR;
			return g_strdup_printf (_("%.1f GB"),
						       displayed_size);
		}
	}
}
