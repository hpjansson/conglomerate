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
#include "cong-vfs.h"

/*
  A routine that tries to load all the bytes requested from the handle into the buffer and bails out on any failure
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
	   if the filename is not absolute, build it */
	if (!g_path_is_absolute (filename)){
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


/* Convert a URI into a POSIX, path, assuming that this is valid: */
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

void
cong_vfs_split_uri (const GnomeVFSURI* uri, 
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
