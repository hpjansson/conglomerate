/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-vfs.c
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
#include "cong-vfs.h"

#include "cong-error-dialog.h"
#include "cong-plugin.h"
#include "cong-parser-error.h"

/*
  A routine that tries to load all the bytes requested from the handle into the buffer and bails out on any failure
 */
/**
 * cong_vfs_read_bytes:
 * @input_stream:
 * @buffer:
 * @bytes:
 *
 * TODO: Write me
 * Returns: %TRUE in any case.
 */
gboolean
cong_vfs_read_bytes (GInputStream* stream,
		     char* buffer, 
		     gsize bytes)
{
	gsize bytes_read = g_input_stream_read(stream, buffer, bytes, NULL, NULL);

	g_assert(bytes==bytes_read); /* for now */

	return TRUE;
}

/* 
   A routine that tries to syncronously load a file into a buffer in memory (surely this exists already somewhere?)
   -- It does indeed! g_file_get_contents().
*/
/**
 * cong_vfs_new_buffer_from_file:
 * @file:
 * @buffer:
 * @size:
 * @error: Return location for an error, or %NULL.
 *
 * TODO: Write me
 * Returns: %TRUE on success, %FALSE if @error was set.
 */
gboolean
cong_vfs_new_buffer_from_file (GFile *file,
			       char** buffer, 
			       gsize* size,
			       GError **error)
{
	g_return_val_if_fail(file, FALSE);
	g_return_val_if_fail(buffer, FALSE);
	g_return_val_if_fail(size, FALSE);

	return g_file_load_contents(file, NULL, buffer, size, NULL, error);
}

/**
 * cong_vfs_load_xml_from_file:
 * @file:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
xmlDocPtr
cong_vfs_load_xml_from_file (GFile *file,
			     GtkWindow *parent_window)
{
	xmlDocPtr xml_doc = NULL;

#if 0
	/* Load using libxml directly: */
	{
		xmlDocPtr ret;
		xmlParserCtxtPtr ctxt;
		
		CongParserResult parser_result;
		
		g_return_val_if_fail(string_uri, NULL);
		
#error		
		parser_result.buffer=buffer;
		parser_result.size=size;
		parser_result.issues=NULL;
		parser_result.parent_window=parent_window;
		
		parser_result.string_uri = g_strdup (string_uri);
		
		ctxt = xmlCreateFileParserCtxt	(string_uri);
		if (ctxt == NULL) return(NULL);
		
		g_assert(ctxt->sax);
		ctxt->sax->error=on_sax_error;
		ctxt->sax->warning=on_sax_warning;
		ctxt->loadsubset = TRUE; /* try to get DTDs to be loaded */
		
		xmlCatalogSetDebug(TRUE);
		
		g_assert(global_parser_result==NULL);
		
		global_parser_result = &parser_result;
		
		xmlParseDocument(ctxt);
		
		global_parser_result = NULL;
		
		if (ctxt->wellFormed) {
			ret = ctxt->myDoc;
		} else {
			GtkDialog* dialog = cong_error_dialog_new_file_open_failed_from_parser_error (string_uri,
												      &parser_result);
			
			cong_error_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(GTK_WIDGET(dialog));
			
			/* FIXME: It will often be possible to continue; we should give the user the option of carrying on with what we've got */
			
			ret = NULL;
			xmlFreeDoc(ctxt->myDoc);
			ctxt->myDoc = NULL;
			
		}
		xmlFreeParserCtxt(ctxt);
		
		return(ret);
	}
#else
	/* Load using GIO: */
	{
		char* buffer;
		gsize size;
		GError *error = NULL;
		gboolean result = cong_vfs_new_buffer_from_file (file,
		                                                 &buffer,
		                                                 &size,
		                                                 &error);

		if (!result) {
			GtkDialog* dialog = cong_error_dialog_new_from_file_open_failure_with_gerror (parent_window,
			                                                                              file,
			                                                                              error);
			cong_error_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(GTK_WIDGET(dialog));
			
			return NULL;
		}
		
		g_assert(buffer);
		
		/* Parse the file from the buffer: */
		xml_doc = cong_ui_parse_buffer (buffer, 
						size, 
						file,
						parent_window);
		g_free(buffer);

		return xml_doc;
	}
#endif
}

/**
 * cong_vfs_save_xml_to_file:
 * @doc_ptr:
 * @file: a file reference to save to.
 * @output_file_size:
 * @error: return location for an error, or %NULL.
 *
 * TODO: Write me
 * Returns: %TRUE on success, %FALSE if @error was set.
 */
gboolean
cong_vfs_save_xml_to_file (xmlDocPtr doc_ptr,
			   GFile *file,
			   gsize *output_file_size,
                           GError **error)
{
	xmlChar* mem;
	int size;
	gboolean result;
	GFileOutputStream *stream;
	gsize written_size;

	g_return_val_if_fail(doc_ptr, FALSE);
	g_return_val_if_fail(file, FALSE);
	g_return_val_if_fail(output_file_size, FALSE);

	/* Dump to a memory buffer. then write out buffer to GnomeVFS: */
	xmlDocDumpMemory(doc_ptr,
			 &mem,
			 &size);
	g_assert(mem);
	g_assert(size>0);

	*output_file_size = size;

	stream = g_file_replace(file,
	                        NULL,
	                        FALSE,
	                        G_FILE_CREATE_NONE,
	                        NULL,
	                        error);
	if(!stream) {
		xmlFree(mem);
		return FALSE;
	}

	result = g_output_stream_write_all(G_OUTPUT_STREAM(stream),
	                                   mem,
	                                   *output_file_size,
	                                   &written_size,
	                                   NULL,
	                                   error);
	xmlFree(mem);

	if (!result) {
		g_output_stream_close(G_OUTPUT_STREAM(stream), NULL, NULL);
		return FALSE;
	}

	g_assert(*output_file_size == written_size);

	result = g_output_stream_close(G_OUTPUT_STREAM(stream), NULL, error);

	return result;
}

/**
 * cong_vfs_get_local_path_from_uri:
 * @vfs_uri: a #GnomeVFSURI
 *
 * Returns: a #gchar containing @uri as a POSIX path, assuming it is
 * valid
 */
gchar*
cong_vfs_get_local_path_from_file (GFile *file)
{
	gchar *path_string;

	g_return_val_if_fail(file, NULL);

	path_string = g_file_get_path(file);

	g_message("got \"%s\"", path_string);
	return path_string;
}

/**
 * cong_vfs_split_file_path:
 * @file:
 * @filename_alone: (allow-none): Return location for the basename of the file,
 * or %NULL if you aren't interested in it.
 * @path: (allow-none): Return location for the path to the file, or %NULL if
 * you aren't interested in it.
 *
 * TODO: Write me
 */
void
cong_vfs_split_file_path (GFile *file,
			  gchar** filename_alone,
			  gchar** path)
{
	GFile *parent;

	g_return_if_fail(file);
	g_return_if_fail(filename_alone);
	g_return_if_fail(path);

	if (filename_alone)
		*filename_alone = g_file_get_basename(file);

	if(path) {
		parent = g_file_get_parent(file);

		if (parent) {

			*path = g_file_get_path(parent);
			g_object_unref(parent);
		} else {
			*path=g_strdup("");
		}
	}
}

/**
 * cong_vfs_extract_short_name:
 * @file: file reference to query.
 *
 * Gets the short name (base name) of @file. Free the string when done. Note,
 * this string is in the filename encoding, which might not be UTF-8!
 * Returns: (transfer full): file's short name, or %NULL on error.
 */
gchar*
cong_vfs_extract_short_name (GFile *file)
{
	g_return_val_if_fail(file, NULL);
	return g_file_get_basename(file);
}

/**
 * cong_vfs_extract_display_name:
 * @file: file reference to query.
 *
 * Gets a short filename suitable for display. Free the string when done. May
 * fall back to the short non-display name if necessary.
 * Returns: (transfer full): file's display name, or %NULL on error.
 */
char *
cong_vfs_extract_display_name (GFile *file)
{
	GFileInfo *info;
	char *retval;

	g_return_val_if_fail(file, NULL);

	info = g_file_query_info(file,
	                  G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
	                  G_FILE_QUERY_INFO_NONE,
	                  NULL,
	                  NULL);
	if(info) {
		retval = g_strdup(g_file_info_get_display_name(info));
		g_object_unref(info);
	} else {
		g_warning("Couldn't query file info");
		retval = cong_vfs_extract_short_name(file);
	}

	return retval;
}
