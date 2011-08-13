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
 * @file: the file reference for the file you tried to access
 * @error: the error that occurred. For convenience, this function frees the error.
 * @file_size:
 *
 * TODO: Write me
 * Returns:
 */
GtkDialog*
cong_error_dialog_new_from_file_save_failure(GtkWindow *parent_window, 
					     GFile *file,
					     GError *error,
					     gsize* file_size)
{
	GtkDialog* dialog = NULL;
	gchar* app_name;
	gchar* filename_alone;
	gchar* path;
	gchar* what_failed_permanent;
	gchar* what_failed_transient;
	GFile* parent;

	g_return_val_if_fail(file, NULL);
	g_return_val_if_fail(error, NULL);

	app_name = cong_error_get_appname();

	cong_vfs_split_file_path (file, &filename_alone, &path);

	g_assert(filename_alone);
	g_assert(path);

	/* Get at the parent URI in case it's needed: */
	parent = g_file_get_parent (file);

	/* A "what failed" message when the failure is likely to be permanent; this URI won't be saveable */
	what_failed_permanent = g_strdup_printf(_("%s cannot save \"%s\" to %s."),app_name, filename_alone, path);

	/* A "what failed" message when the failure is likely to be transient; this URI might be "saveable-to" on subsequent attempts, or with some troubleshooting. */
	what_failed_transient = g_strdup_printf(_("%s could not save \"%s\" to %s."),app_name, filename_alone, path);

	switch (error->code) {
	default:
	case G_IO_ERROR_FAILED:
	case G_IO_ERROR_INVALID_ARGUMENT:
	case G_IO_ERROR_CLOSED:
	case G_IO_ERROR_WOULD_RECURSE:
	case G_IO_ERROR_CANCELLED:
	case G_IO_ERROR_NOT_EMPTY:
	case G_IO_ERROR_TOO_MANY_LINKS:

	case G_IO_ERROR_NOT_DIRECTORY: /* FIXME: when does this occur? */
	case G_IO_ERROR_PENDING: /* FIXME: when does this occur? */
	case G_IO_ERROR_EXISTS: /* FIXME: when does this occur? */
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
	  
	case G_IO_ERROR_NOT_FOUND:
		{
			/* Since we're saving, this must be "path not found" rather than "file not found": */
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_transient,
						       _("The location does not exist."),
						       _("(i) Try checking that you spelt the location's name correctly.  Remember that capitalisation is significant (\"MyDirectory\" is not the same as \"MYDIRECTORY\" or \"mydirectory\").\n(ii) Try saving to a different location."));
		}
		break;
		
	case G_IO_ERROR_NOT_SUPPORTED:
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

	case G_IO_ERROR_NO_SPACE:
		{
			gchar* why_failed = NULL;

			if (file_size) {

				char* file_size_string = cong_util_format_file_size_for_display(*file_size);

				/* We call the "get space" function on the parent URI rather than the file URI since this function fails if
				   the URI does not exist (it decides it's not a local URI as it can't stat the file) */
				GFileInfo *info = g_file_query_info(parent,
				                                    G_FILE_ATTRIBUTE_FILESYSTEM_FREE,
				                                    G_FILE_QUERY_INFO_NONE,
				                                    NULL,
				                                    NULL);
				if(info) {
					guint64 free_space = g_file_info_get_attribute_uint64(info,
					                                                      G_FILE_ATTRIBUTE_FILESYSTEM_FREE);

					char* free_space_string = cong_util_format_file_size_for_display(free_space);
				
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
	case G_IO_ERROR_READ_ONLY:
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
	case G_IO_ERROR_INVALID_FILENAME:
		{
			/* FIXME: is case significant for VFS method names? */
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_permanent,
						       _("The system does not recognise that as a valid location."),
						       _("(i) Try checking that you spelt the location correctly.  Remember that capitalisation is significant (\"http\" is not the same as \"Http\" or \"HTTP\").\n(ii) Try saving to a different location."));
		}
		break;
	case G_IO_ERROR_PERMISSION_DENIED:
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
	case G_IO_ERROR_TOO_MANY_OPEN_FILES:
		{
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_transient,
						       _("The system is trying to operate on too many files at once."),
						       _("Try again.  If it fails again, try closing unwanted applications, or contact your system administrator."));
		}
		break;
	case G_IO_ERROR_IS_DIRECTORY:
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
	case G_IO_ERROR_HOST_NOT_FOUND:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_permanent,
						       _("The server could not be contacted."),
						       _("Try again.  If it fails again, the server may be down; try saving to another location."));
		}
		break;
#if GLIB_CHECK_VERSION(2,26,0)
	case G_IO_ERROR_CONNECTION_REFUSED:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_transient,
						       _("The system could not login to the location."),
						       _("Try again. If it fails again, try saving to another location."));
		}
		break;
#endif
	case G_IO_ERROR_BUSY:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new(parent_window, 
						       what_failed_transient,
						       _("The location was too busy."),
						       _("Try again. If it fails again, try saving to another location."));
		}
		break;

	case G_IO_ERROR_FILENAME_TOO_LONG:
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

	case G_IO_ERROR_FAILED_HANDLED:
		{
			/* FIXME: This means a helper program has already interacted
			 * with the user and we shouldn't display an error dialog. */
			dialog = GTK_DIALOG(gtk_message_dialog_new(parent_window,
			                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			                                           GTK_MESSAGE_INFO,
			                                           GTK_BUTTONS_OK,
			                                           _("Click OK to continue.")));
		}
		break;

	}

	g_free(what_failed_transient);
	g_free(what_failed_permanent);
	g_free(filename_alone);
	g_free(path);

	g_object_unref (parent);

	g_free(app_name);

	g_assert(dialog);

	g_error_free(error);

	return dialog;
}
