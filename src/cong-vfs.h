/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-vfs.h
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

#ifndef __CONG_VFS_H__
#define __CONG_VFS_H__

#include <libgnomevfs/gnome-vfs.h>

G_BEGIN_DECLS

/* Handy utility functions: */

/**
 * cong_vfs_new_buffer_from_file
 * @filename:
 * @buffer:
 * @size:
 * 
 * A routine that tries to syncronously load a file into a buffer in memory (surely this exists already somewhere?)
 * (I believe that CVS gnome-vfs has a routine gnome_vfs_read_entire_file that does this)
 * 
 * Returns:
 */
GnomeVFSResult
cong_vfs_new_buffer_from_file (const char* filename, 
			       char** buffer, 
			       GnomeVFSFileSize* size);

/**
 * cong_vfs_new_buffer_from_uri
 * @uri:
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
			      GnomeVFSFileSize* size);

GnomeVFSResult
cong_vfs_save_xml_to_uri (xmlDocPtr doc_ptr, 
			  GnomeVFSURI *file_uri,
			  GnomeVFSFileSize *output_file_size);

/**
   Convert a URI into a POSIX, path, assuming that this is valid: 
*/
gchar*
cong_vfs_get_local_path_from_uri (GnomeVFSURI *uri);

void
cong_vfs_split_uri (const GnomeVFSURI* uri, 
		    gchar** filename_alone, 
		    gchar** path);

G_END_DECLS

#endif
