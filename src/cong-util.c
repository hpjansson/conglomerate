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

	while (	unichar = g_utf8_get_char(src_text) ) {
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



