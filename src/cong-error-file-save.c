/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-error-file-save.c
 *
 * Copyright (C) 2002 David Malcolm
 *
 * FIXME:  This file is currently licensed under the GPL, but is intended to eventually become part of a library licensed under the LGPL
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#include <gtk/gtk.h>

#include "global.h"
#include "cong-error-dialog.h"
#include "cong-util.h"
#include "cong-vfs.h"

/* FIXME: i18n! */
/**
 * cong_error_dialog_new_from_file_save_failure:
 * @parent_window:
 * @string_uri:
 * @vfs_result:
 * @file_size:
 *
 * TODO: Write me
 * Returns:
 */
GtkDialog*
cong_error_dialog_new_from_file_save_failure(GtkWindow *parent_window, 
					     const gchar* string_uri, 
					     GnomeVFSResult vfs_result, 
					     const GnomeVFSFileSize* file_size)
{
	GtkDialog* dialog = NULL;
	gchar* app_name;
	gchar* filename_alone;
	gchar* path;
	gchar* what_failed_permanent;
	gchar* what_failed_transient;
	GnomeVFSURI* vfs_uri;
	GnomeVFSURI* parent_uri;

	g_return_val_if_fail(string_uri, NULL);
	g_return_val_if_fail(GNOME_VFS_OK!=vfs_result, NULL);

	app_name = cong_error_get_appname();

	cong_vfs_split_string_uri (string_uri, &filename_alone, &path);

	g_assert(filename_alone);
	g_assert(path);

	/* Get at the parent URI in case it's needed: */
	vfs_uri = gnome_vfs_uri_new (string_uri);
	parent_uri = gnome_vfs_uri_get_parent(vfs_uri);

	/* A "what failed" message when the failure is likely to be permanent; this URI won't be saveable */
	what_failed_permanent = g_strdup_printf(_("%s cannot save \"%s\" to %s."),app_name, filename_alone, path);

	/* A "what failed" message when the failure is likely to be transient; this URI might be "saveable-to" on subsequent attempts, or with some troubleshooting. */
	what_failed_transient = g_strdup_printf(_("%s could not save \"%s\" to %s."),app_name, filename_alone, path);

	switch (vfs_result) {
	default:
	case GNOME_VFS_ERROR_GENERIC:
	case GNOME_VFS_ERROR_INTERNAL:
	case GNOME_VFS_ERROR_BAD_PARAMETERS:
	case GNOME_VFS_ERROR_WRONG_FORMAT:
	case GNOME_VFS_ERROR_NOT_OPEN:
	case GNOME_VFS_ERROR_INVALID_OPEN_MODE:
	case GNOME_VFS_ERROR_EOF:
	case GNOME_VFS_ERROR_LOOP:
	case GNOME_VFS_ERROR_CANCELLED:
	case GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY:
	case GNOME_VFS_ERROR_TOO_MANY_LINKS:
	case GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM:

	case GNOME_VFS_ERROR_NOT_A_DIRECTORY: /* FIXME: when does this occur? */
	case GNOME_VFS_ERROR_IN_PROGRESS: /* FIXME: when does this occur? */
	case GNOME_VFS_ERROR_INTERRUPTED: /* FIXME: when does this occur? */
	case GNOME_VFS_ERROR_FILE_EXISTS: /* FIXME: when does this occur? */
	case GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE: /* FIXME: when does this occur? */
	case GNOME_VFS_ERROR_SERVICE_OBSOLETE: /* FIXME: when does this occur? */
	case GNOME_VFS_ERROR_PROTOCOL_ERROR: /* FIXME: when does this occur? */
	  {
		  /* Unknown (or inapplicable) error */
		  dialog = cong_error_dialog_new(parent_window, 
						 what_failed_transient,
						 _("An unexpected internal error occurred."),
						 _("Try again.  If it fails again, file a bug report with the maintainer of this application."));
		  /* FIXME: ought to provide a convenience button that launches bug-buddy with lots of details filled in, including info
		     on the internal state at this point. */
		  
		  /* FIXME: ought to make a distinction between results that should be filed as bugs:
		     (i) with the app, 
		     (ii) with the GnomeVFS module
		     (iii) with the error-reporting system
		  */
	  }
	  break;
	  
	case GNOME_VFS_ERROR_NOT_FOUND:
		{
			/* Since we're saving, this must be "path not found" rather than "file not found": */
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_transient,
						       _("The location does not exist."),
						       _("(i) Try checking that you spelt the location's name correctly.  Remember that capitalisation is significant (\"MyDirectory\" is not the same as \"MYDIRECTORY\" or \"mydirectory\").\n(ii) Try saving to a different location."));
		}
		break;
		
	case GNOME_VFS_ERROR_NOT_SUPPORTED:
	case GNOME_VFS_ERROR_NOT_PERMITTED:
		{
			/* FIXME: need some thought about the messages for this */
			gchar* why_failed = g_strdup_printf(_("The location \"%s\" does not support the writing of files."),path);
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_permanent,
						       why_failed,
						       _("Try saving to a different location."));
			g_free(why_failed);
		}
		break;
		
	case GNOME_VFS_ERROR_IO:
	case GNOME_VFS_ERROR_CORRUPTED_DATA:
	case GNOME_VFS_ERROR_BAD_FILE:
		{
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_transient,
						       _("There were problems writing the content of the file."),
						       _("Try again.  If it fails again, try saving to a different location."));
		}
		break;
		
	case GNOME_VFS_ERROR_TOO_BIG:
		{
			gchar* why_failed = NULL;

			if (file_size) {

				char* file_size_string = gnome_vfs_format_file_size_for_display(*file_size);

				GnomeVFSFileSize volume_capacity;

				/* We call the "get space" function on the parent URI rather than the file URI since this function fails if
				   the URI does not exist (it decides it's not a local URI as it can't stat the file) */

				/* FIXME: this function only exists at the moment on my local version of GNOME VFS */
#if 0
				GnomeVFSResult volume_capacity_result = gnome_vfs_get_volume_capacity(parent_uri,
												      &volume_capacity);
#else
				/* FIXME: This is the workaround: */
				GnomeVFSResult volume_capacity_result = GNOME_VFS_ERROR_NOT_SUPPORTED;
#endif

				if (volume_capacity_result==GNOME_VFS_OK) {
					char* volume_capacity_string = gnome_vfs_format_file_size_for_display(volume_capacity);
				
					why_failed = g_strdup_printf(_("The size of the file would be %s, but the device only has a capacity of %s."),file_size_string,volume_capacity_string);
				
					g_free(volume_capacity_string);
				} else {
					
					/* Can't get at the capacity for the device or "volume": */
					why_failed = g_strdup_printf(_("The file is too big to fit on the device (file size would be %s)."), file_size_string);
				}

				g_free(file_size_string);

			} else {
				
				/* We don't know the size of the file: */
				why_failed = g_strdup_printf(_("The file is too big to fit on the device."));

			}

			g_assert(why_failed);

			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_permanent,
						       why_failed,
						       _("Try saving the file to a different location."));
			g_free(why_failed);
		}
		break;
	case GNOME_VFS_ERROR_NO_SPACE:
		{
			gchar* why_failed = NULL;

			if (file_size) {

				char* file_size_string = gnome_vfs_format_file_size_for_display(*file_size);

				/* We call the "get space" function on the parent URI rather than the file URI since this function fails if
				   the URI does not exist (it decides it's not a local URI as it can't stat the file) */
				GnomeVFSFileSize free_space;
				GnomeVFSResult free_space_result = gnome_vfs_get_volume_free_space(parent_uri,
												   &free_space);
				
				if (free_space_result==GNOME_VFS_OK) {
					char* free_space_string = gnome_vfs_format_file_size_for_display(free_space);
				
					why_failed = g_strdup_printf(_("The size of the file would be %s, but you only have %s free on that device."),file_size_string,free_space_string);
				
					g_free(free_space_string);
				} else {
					
					/* Can't get at the free space for the device or "volume": */
					why_failed = g_strdup_printf(_("The file is too big to fit in the remaining space on the device (file size would be %s)."), file_size_string);
				}

				g_free(file_size_string);

			} else {
				
				/* We don't know the size of the file: */
				why_failed = g_strdup_printf(_("The file is too big to fit in the remaining space on the device."));

			}

			g_assert(why_failed);

			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_permanent,
						       why_failed,
						       _("Try saving the file to a different location, or making more space by moving unwanted files from that device to the trash."));
			g_free(why_failed);
		}
		break;
	case GNOME_VFS_ERROR_READ_ONLY:
		{
			/* FIXME: need some thought about the messages for this */
			gchar* why_failed = g_strdup_printf(_("The location \"%s\" does not support the writing of files."),path);
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_permanent,
						       why_failed,
						       _("Try saving to a different location."));
			g_free(why_failed);
		}
		break;
	case GNOME_VFS_ERROR_INVALID_URI:
		{
			/* FIXME: is case significant for VFS method names? */
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_permanent,
						       _("The system does not recognise that as a valid location."),
						       _("(i) Try checking that you spelt the location correctly.  Remember that capitalisation is significant (\"http\" is not the same as \"Http\" or \"HTTP\").\n(ii) Try saving to a different location."));
		}
		break;
	case GNOME_VFS_ERROR_ACCESS_DENIED:
		{
			/**
			   FIXME:  This error can occur when attempting to write to the mountpoint of a filesystem that has not been mounted e.g. "file:///mnt/floppy".

			   We should spot this special-case, and distinguish between unmounted floppies that could be writable to, and unmounted CD-ROMs that will not be writable to.

			   Probably should also spot write-protected floppies etc etc...
			*/
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_permanent,
						       _("You do not have permission to write to that location."),
						       _("Try saving to a different location, or ask your system administrator to give you permission."));
		}
		break;
	case GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES:
		{
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_transient,
						       _("The system is trying to operate on too many files at once."),
						       _("Try again.  If it fails again, try closing unwanted applications, or contact your system administrator."));
		}
		break;
	case GNOME_VFS_ERROR_IS_DIRECTORY:
		{
			/* FIXME:  capitalisation issues */
			gchar* why_failed = g_strdup_printf(_("\"%s\" is a directory."),filename_alone);
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_permanent,
						       why_failed,
						       _("You must save to a file within a directory, rather than to the directory itself."));
			g_free(why_failed);
		}
		break;
	case GNOME_VFS_ERROR_NO_MEMORY:
		{
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_transient,
						       _("The system ran out of memory."),
						       _("Try again.  If it fails again, try closing unwanted applications, or contact your system administrator."));
		}
		break;
	case GNOME_VFS_ERROR_HOST_NOT_FOUND:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_permanent,
						       _("The server could not be contacted."),
						       _("Try again.  If it fails again, the server may be down; try saving to another location."));
		}
		break;
	case GNOME_VFS_ERROR_INVALID_HOST_NAME:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_permanent,
						       _("The server could not be contacted."),
						       _("(i) Try checking that you spelt the location correctly.\n(ii) Try again. If it fails again, the server may be down; try saving to another location."));
		}
		break;
	case GNOME_VFS_ERROR_HOST_HAS_NO_ADDRESS:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_permanent,
						       _("The server could not be contacted."),
						       _("(i) Try checking that you spelt the location correctly.\n(ii) Try again. If it fails again, the server may be down; try saving to another location."));
		}
		break;
	case GNOME_VFS_ERROR_LOGIN_FAILED:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_transient,
						       _("The system could not login to the location."),
						       _("Try again. If it fails again, try saving to another location."));
		}
		break;
	case GNOME_VFS_ERROR_DIRECTORY_BUSY:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_transient,
						       _("The location was too busy."),
						       _("Try again. If it fails again, try saving to another location."));
		}
		break;

	case GNOME_VFS_ERROR_READ_ONLY_FILE_SYSTEM:
		{
			/* FIXME: need some thought about the messages for this */
			gchar* why_failed = g_strdup_printf(_("The location \"%s\" only allows files to be read, not written."),path);
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_permanent,
						       why_failed,
						       _("Try saving to a different location."));
			g_free(why_failed);
		}
		break;

	case GNOME_VFS_ERROR_NAME_TOO_LONG:
		{
			/* FIXME: need to think more about these messages */
			char* why_failed = g_strdup_printf(_("The name \"%s\" is too long for the location to manage."), filename_alone);
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_permanent,
						       why_failed,
						       _("Try again with a shorter file name."));
			g_free(why_failed);
		}
		break;

	}

	g_free(what_failed_transient);
	g_free(what_failed_permanent);
	g_free(filename_alone);
	g_free(path);

	gnome_vfs_uri_unref (parent_uri);
	gnome_vfs_uri_unref (vfs_uri);

	g_free(app_name);

	g_assert(dialog);

	return dialog;
}
