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

#include "global.h"
#include "cong-util.h"
#include "cong-app.h"
#include "cong-document.h"

#include <libxml/globals.h>
#include <libxml/catalog.h>

#if ENABLE_PRINTING
#if 0
/* FIXME: use xmlroff eventually */
#include <libfo/fo-libfo.h>
#else
#include "fo.h"
#endif
#endif

gboolean 
cong_util_is_docbook (CongDocument *doc) 
{
	const CongXMLChar* dtd_public_id;

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

gchar* 
cong_util_cleanup_text (const xmlChar *src_text) 
{
	gchar *buffer;
	gchar *src;
	gchar *dst;
	gunichar unichar;

	g_return_val_if_fail(src_text, NULL);

	g_assert(g_utf8_validate(src_text, -1, NULL));

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
}

gchar*
cong_utils_get_norman_walsh_stylesheet_path(void)
{
       /* This should be changed if another catalog is in use, i guess */
       xmlChar *resolved_path = NULL;

       resolved_path = xmlCatalogResolveURI ("http://docbook.sourceforge.net/release/xsl/current/");

       g_message ("Norman Walsh XSL path: %s", resolved_path);

       return resolved_path;
}

gchar*
cong_utils_get_norman_walsh_stylesheet(const gchar *stylesheet_relative_path)
{
	xmlChar *path;
	gchar *result;

	g_return_val_if_fail(stylesheet_relative_path, NULL);

	path = cong_utils_get_norman_walsh_stylesheet_path();

	result = g_strdup_printf("%s%s", path, stylesheet_relative_path);

	xmlFree (path);

	return result;
}

/* Convert a URI into a POSIX, path, assuming that this is valid: */
gchar*
cong_util_get_local_path_from_uri (GnomeVFSURI *uri)
{
	gchar *uri_string;

	g_return_val_if_fail(uri, NULL);

	uri_string = gnome_vfs_uri_to_string(uri, 
					     (GNOME_VFS_URI_HIDE_USER_NAME
					      |GNOME_VFS_URI_HIDE_PASSWORD
					      |GNOME_VFS_URI_HIDE_HOST_NAME
					      |GNOME_VFS_URI_HIDE_HOST_PORT
					      |GNOME_VFS_URI_HIDE_TOPLEVEL_METHOD
					      |GNOME_VFS_URI_HIDE_FRAGMENT_IDENTIFIER)
					     );

	g_message("got \"%s\"",uri_string);
	return uri_string;
}

GdkPixbuf*
cong_util_load_icon (const gchar *icon_basename)
{
	gchar *filename;
	gchar *full_path;
	GdkPixbuf *pixbuf;

	g_return_val_if_fail(icon_basename, NULL);

	filename = g_strdup_printf("%s-16.png", icon_basename);
	full_path = gnome_program_locate_file(cong_app_singleton()->gnome_program,
					      GNOME_FILE_DOMAIN_APP_PIXMAP,
					      filename,
					      FALSE,
					      NULL);
	pixbuf = gdk_pixbuf_new_from_file(full_path, NULL);
	
	g_free(full_path);
	g_free(filename);

	return pixbuf;
}

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

#if ENABLE_PRINTING
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

#if !SUPPORT_UNDO
void
cong_util_set_cursor_to_first_text_descendant (CongDocument *doc,
					       CongNodePtr node)
{
	CongNodePtr cursor_node;

	g_return_if_fail (doc);
	g_return_if_fail (node);

	cursor_node = cong_node_get_first_text_node_descendant (node);
	
	if (cursor_node) {
		CongCursor *cursor = cong_document_get_cursor (doc);
		cong_document_begin_edit (doc);

		cong_location_set_to_start_of_node (&cursor->location,
						    cursor_node);

		cong_document_on_cursor_change (doc);

		cong_document_end_edit (doc);		
	}
}
#endif /* #if !SUPPORT_UNDO */

/**
   Make DTD declaration, and assigns it to the given document.  Doesn't add it to doc tree.
 */
xmlDtdPtr
cong_util_make_dtd (xmlDocPtr xml_doc,
		    const xmlChar *root_element,
		    const xmlChar *ExternalID, 
		    const xmlChar *SystemID)
{
#if 1
	xmlDtdPtr  dtd_ptr;

	g_message ("Trying to load DTD with PUBLIC \"%s\" SYSTEM \"%s\"", ExternalID, SystemID);

	dtd_ptr = xmlParseDTD (ExternalID, 
			       SystemID);
	
	if (dtd_ptr) {
		g_message ("Succeeded");

		/* Then set the document and the root_element: */
		cong_node_recursive_set_doc ((CongNodePtr)dtd_ptr, 
					     xml_doc);

		if (dtd_ptr->name) {
			xmlFree (dtd_ptr->name);
		}
		dtd_ptr->name = xmlStrdup(root_element);

		/* Set up ptr within the xml_doc: */
		xml_doc->extSubset = dtd_ptr;

		/* Nullify the tree descendants; we have to do this otherwise the entire DTD is in the tree and appears in the editor view etc etc (and is very slow for large DTDs): */
		dtd_ptr->children = NULL;
		dtd_ptr->last = NULL;

		return dtd_ptr;
	} else {
		g_message ("Failed");

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
xmlDtdPtr 
cong_util_add_external_dtd (xmlDocPtr xml_doc, 
			    const xmlChar *root_element,
			    const xmlChar *ExternalID, 
			    const xmlChar *SystemID)
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


void
cong_util_split_uri (const GnomeVFSURI* uri, 
		     gchar** filename_alone, 
		     gchar** path)
{
	GnomeVFSURI* parent_uri;

	g_return_if_fail(uri);
	g_return_if_fail(filename_alone);
	g_return_if_fail(path);

	parent_uri = gnome_vfs_uri_get_parent(uri);

	*filename_alone=gnome_vfs_uri_extract_short_name(uri);

#if 1
	/* This version seems better when dealing with e.g. http and ftp methods etc: */
	if (parent_uri) {

		*path=gnome_vfs_uri_to_string(parent_uri,
					      GNOME_VFS_URI_HIDE_USER_NAME|GNOME_VFS_URI_HIDE_PASSWORD);
	} else {
		*path=g_strdup("");
	}
#else
	/* This version seems better when dealing with the "file" method; perhaps we should have a conditional here? */ 
	*path=gnome_vfs_uri_extract_dirname(uri);
#endif

	gnome_vfs_uri_unref(parent_uri);

}

/*
  Function to remove a node x from the tree; all its children become children of x's parents in the natural place in the tree.
 */
#if !SUPPORT_UNDO
void 
cong_util_remove_tag (CongDocument *doc, 
		      CongNodePtr x)
{
	CongNodePtr n0;
	CongNodePtr n0_next;

	g_return_if_fail(x);

	/* GREP FOR MVC */

#if 1
	for (n0 = x->children; n0; n0 = n0_next) {
		n0_next = n0->next;
		
		cong_document_node_add_before(doc, n0, x);
	}

	cong_document_node_make_orphan(doc, x);

	cong_document_node_recursive_delete (doc, x);
#else
	n0 = cong_node_first_child(x);

	if (n0) {
		n0->prev = x->prev;
	}

	if (NULL==x->prev) {
		x->parent->children = n0;
	} else {
		x->prev->next = n0;
	}

	for (; n0->next; n0 = n0->next) {
		n0->parent = x->parent;
	}
	n0->parent = x->parent;

	n0->next = x->next;
	if (x->next) {
		x->next->prev = n0;
	} else {
		x->parent->last = n0;
	}
	
	x->next = NULL;
	x->prev = NULL;
	x->parent = NULL;
	x->children = NULL;
	x->last = NULL;

	xmlFreeNode(x);
#endif
}
#endif /* #if !SUPPORT_UNDO */

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
