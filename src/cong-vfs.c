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
 * @vfs_handle:
 * @buffer:
 * @bytes:
 *
 * TODO: Write me
 * Returns:
 */
GnomeVFSResult
cong_vfs_read_bytes (GnomeVFSHandle* vfs_handle, 
		     char* buffer, 
		     GnomeVFSFileSize bytes)
{
	GnomeVFSFileSize bytes_read;
	GnomeVFSResult vfs_result = gnome_vfs_read(vfs_handle,buffer,bytes,&bytes_read);

	g_assert(bytes==bytes_read); /* for now */

	return vfs_result;
}

/* 
   A routine that tries to syncronously load a file into a buffer in memory (surely this exists already somewhere?)
*/
/**
 * cong_vfs_new_buffer_from_file:
 * @filename:
 * @buffer:
 * @size:
 *
 * TODO: Write me
 * Returns:
 */
GnomeVFSResult
cong_vfs_new_buffer_from_file (const char* filename, 
			       char** buffer, 
			       GnomeVFSFileSize* size)
{
	GnomeVFSResult vfs_result;
	GnomeVFSURI* uri;

	g_return_val_if_fail(filename,GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(buffer,GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(size,GNOME_VFS_ERROR_BAD_PARAMETERS);

	/* it seems that GnomeVFS works with absolute paths, so
	   if the filename is not absolute, build it.
	   GnomeVFS URIs are absolute. */
	   
	if (!g_path_is_absolute (filename) && !(g_str_has_prefix (filename, "file:"))) {

		gchar *absolute_path = g_strconcat (g_get_current_dir(), GNOME_VFS_URI_PATH_STR, filename, NULL);
    		uri = gnome_vfs_uri_new (absolute_path);
		g_free (absolute_path);

	} else {

		uri = gnome_vfs_uri_new (filename);
	}

	vfs_result = cong_vfs_new_buffer_from_uri(uri, buffer, size);

	gnome_vfs_uri_unref(uri);

	return vfs_result;
}

/**
 * cong_vfs_new_buffer_from_uri:
 * @vfs_uri:
 * @buffer:
 * @size:
 *
 * A routine that tries to syncronously load a file into a buffer in memory (surely this exists already somewhere?)
 *
 * Returns:
 */
GnomeVFSResult
cong_vfs_new_buffer_from_uri (GnomeVFSURI* uri, 
			      char** buffer, 
			      GnomeVFSFileSize* size)
{
	GnomeVFSResult vfs_result;
	GnomeVFSHandle *vfs_handle;

	g_return_val_if_fail(uri,GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(buffer,GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(size,GNOME_VFS_ERROR_BAD_PARAMETERS);

	vfs_result = gnome_vfs_open_uri(&vfs_handle,
					uri,
					GNOME_VFS_OPEN_READ);

	if (GNOME_VFS_OK!=vfs_result) {
		return vfs_result;
	} else {
		GnomeVFSFileInfo *info;
		*buffer=NULL;
		
		info = gnome_vfs_file_info_new ();

		/* Get the size of the file: */
		vfs_result = gnome_vfs_get_file_info_from_handle(vfs_handle,
								 info,
								 GNOME_VFS_FILE_INFO_DEFAULT);
		if (GNOME_VFS_OK!=vfs_result) {
			gnome_vfs_close(vfs_handle);
			gnome_vfs_file_info_unref (info);
			
			return vfs_result;
		}

		if (!(info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE)) {
			gnome_vfs_close(vfs_handle);
			gnome_vfs_file_info_unref (info);
			
			return GNOME_VFS_ERROR_IO; /* FIXME: is this appropriate? */
		}

		
		/* Allocate the buffer: */
		*buffer = g_malloc(info->size);
		
		/* Read the file into the buffer: */
		vfs_result = cong_vfs_read_bytes(vfs_handle, *buffer, info->size);
		
		if (GNOME_VFS_OK!=vfs_result) {
			
			g_free(*buffer);
			gnome_vfs_close(vfs_handle);
			gnome_vfs_file_info_unref (info);

			*buffer=NULL;
			
			return vfs_result;
		}
		
		gnome_vfs_close(vfs_handle);
		
		*size = info->size;
		
		gnome_vfs_file_info_unref (info);

		return GNOME_VFS_OK;
	}
}

/**
 * cong_vfs_load_xml_from_uri:
 * @string_uri:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
xmlDocPtr
cong_vfs_load_xml_from_uri (const gchar *string_uri,
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
	/* Load using GnomeVFS: */
	{
		GnomeVFSURI* vfs_uri = gnome_vfs_uri_new (string_uri);
		char* buffer;
		GnomeVFSFileSize size;
		GnomeVFSResult vfs_result = cong_vfs_new_buffer_from_file (string_uri, 
									   &buffer, 
									   &size);		
		if (vfs_result!=GNOME_VFS_OK) {
			GtkDialog* dialog = cong_error_dialog_new_from_file_open_failure_with_vfs_result (parent_window,
													  string_uri,
													  vfs_result);
			cong_error_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(GTK_WIDGET(dialog));

			gnome_vfs_uri_unref (vfs_uri);
			
			return NULL;
		}
		
		g_assert(buffer);
		
		/* Parse the file from the buffer: */
		xml_doc = cong_ui_parse_buffer (buffer, 
						size, 
						string_uri, 
						parent_window);
		g_free(buffer);

		gnome_vfs_uri_unref (vfs_uri);

		return xml_doc;
	}
#endif
}

/**
 * cong_vfs_save_xml_to_uri:
 * @doc_ptr:
 * @vfs_uri:
 * @output_file_size:
 *
 * TODO: Write me
 * Returns:
 */
GnomeVFSResult
cong_vfs_save_xml_to_uri (xmlDocPtr doc_ptr, 
			  GnomeVFSURI *file_uri,	
			  GnomeVFSFileSize *output_file_size)
{
	xmlChar* mem;
	int size;
	GnomeVFSResult vfs_result;
	GnomeVFSHandle *vfs_handle;
	GnomeVFSFileSize written_size;

	g_return_val_if_fail(doc_ptr, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(file_uri, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(output_file_size, GNOME_VFS_ERROR_BAD_PARAMETERS);

	/* Dump to a memory buffer. then write out buffer to GnomeVFS: */
	xmlDocDumpMemory(doc_ptr,
			 &mem,
			 &size);
	g_assert(mem);
	g_assert(size>0);

	*output_file_size = size;

	vfs_result = gnome_vfs_create_uri(&vfs_handle,
					  file_uri,
					  GNOME_VFS_OPEN_WRITE,
					  FALSE,
					  0644
					);

	if (vfs_result != GNOME_VFS_OK) {
		return vfs_result;
	}

	vfs_result = gnome_vfs_write(vfs_handle,
				     mem,
				     *output_file_size,
				     &written_size);

	if (vfs_result != GNOME_VFS_OK) {
		gnome_vfs_close(vfs_handle);
		return vfs_result;
	}

	g_assert(*output_file_size == written_size);

	vfs_result = gnome_vfs_close(vfs_handle);

	return vfs_result;
}

/**
 * cong_vfs_get_local_path_from_uri:
 * @vfs_uri: a #GnomeVFSURI
 *
 * Returns: a #gchar containing @uri as a POSIX path, assuming it is
 * valid
 */
gchar*
cong_vfs_get_local_path_from_uri (GnomeVFSURI *uri)
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

/**
 * cong_vfs_split_vfs_uri:
 * @vfs_uri:
 * @filename_alone:
 * @path:
 *
 * TODO: Write me
 */
void
cong_vfs_split_vfs_uri (const GnomeVFSURI* vfs_uri, 
			gchar** filename_alone, 
			gchar** path)
{
	GnomeVFSURI* parent_uri;

	g_return_if_fail(vfs_uri);
	g_return_if_fail(filename_alone);
	g_return_if_fail(path);

	parent_uri = gnome_vfs_uri_get_parent(vfs_uri);

	*filename_alone=gnome_vfs_uri_extract_short_name(vfs_uri);

#if 1
	/* This version seems better when dealing with e.g. http and ftp methods etc: */
	if (parent_uri) {

		*path=gnome_vfs_uri_to_string(parent_uri,
					      GNOME_VFS_URI_HIDE_USER_NAME|GNOME_VFS_URI_HIDE_PASSWORD);
		gnome_vfs_uri_unref(parent_uri);
	} else {
		*path=g_strdup("");
	}
#else
	/* This version seems better when dealing with the "file" method; perhaps we should have a conditional here? */ 
	*path=gnome_vfs_uri_extract_dirname(vfs_uri);

	gnome_vfs_uri_unref(parent_uri);

#endif
}

/**
 * cong_vfs_split_string_uri:
 * @string_uri:
 * @filename_alone:
 * @path:
 *
 * TODO: Write me
 */
void
cong_vfs_split_string_uri (const gchar* string_uri,
			   gchar** filename_alone, 
			   gchar** path)
{
	GnomeVFSURI* vfs_uri;

	g_return_if_fail(string_uri);

	vfs_uri = gnome_vfs_uri_new (string_uri);

	cong_vfs_split_vfs_uri (vfs_uri, 
				filename_alone, 
				path);
	
	gnome_vfs_uri_unref (vfs_uri);
}

/**
 * cong_vfs_extract_short_name:
 * @string_uri:
 *
 * TODO: Write me
 * Returns:
 */
gchar*
cong_vfs_extract_short_name (const gchar *string_uri)
{
	GnomeVFSURI* vfs_uri;
	gchar *result;

	g_return_val_if_fail(string_uri, NULL);

	vfs_uri = gnome_vfs_uri_new (string_uri);

	result = gnome_vfs_uri_extract_short_name(vfs_uri);
	
	gnome_vfs_uri_unref (vfs_uri);

	return result;
}
