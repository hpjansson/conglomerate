/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gtk/gtk.h>

#include "global.h"

/* FIXME: i18n! */

void
cong_error_split_filename(const gchar* filename, gchar** filename_alone, gchar** path)
{
	g_return_if_fail(filename);
	g_return_if_fail(filename_alone);
	g_return_if_fail(path);

	g_message("stub implementation of cong_error_split_filename called\n");

	*filename_alone=g_strdup("fubar.xml");
	*path=g_strdup("/some_location/some_subdir");
}

GtkWidget*
cong_error_dialog_new_file_open_failed(const gchar* filename)
{
	/* should the filename be a URI? */

	GtkWidget* dialog;
       	gchar* what_failed;
	gchar* filename_alone;
	gchar* path;

	g_return_val_if_fail(filename, NULL);

	cong_error_split_filename(filename, &filename_alone, &path);

	g_assert(filename_alone);
	g_assert(path);

	what_failed = g_strdup_printf("%s cannot find \"%s\" at %s.","Conglomerate", filename_alone, path);

	g_free(filename_alone);
	g_free(path);

	dialog = cong_error_dialog_new(what_failed,
				       
				       "Either a file with that name is not present at the location specified, or the location does not exist",
				       /* FIXME:  isolate all these cases and handle separately.  Also deal separately with permission errors etc */

				       "(i) Try checking that you spelt the file's name correctly.  Remember that capitalisation is significant (\"MyFile\" is not the same as \"MYFILE\" or \"myfile\").\n"
				       "(ii) Try using the GNOME Search Tool to find your file.");


	g_free(what_failed);

	return dialog;
}

gchar* 
cong_error_get_appname(void)
{
	return g_strdup("Conglomerate");
}

GtkWidget*
cong_error_dialog_new_file_open_failed2(const GnomeVFSURI* file_uri, GnomeVFSResult vfs_result)
{
	GtkWidget* dialog = NULL;
	gchar* app_name;
	gchar* filename_alone;
	gchar* path;
	gchar* what_failed_permanent;
	gchar* what_failed_transient;

	g_return_val_if_fail(file_uri, NULL);
	g_return_val_if_fail(GNOME_VFS_OK!=vfs_result, NULL);

	app_name = cong_error_get_appname();

	cong_error_split_uri(file_uri, &filename_alone, &path);

	g_assert(filename_alone);
	g_assert(path);

	/* A "what failed" message when the failure is likely to be permanent; this URI won't be openable */
	what_failed_permanent = g_strdup_printf("%s cannot read \"%s\" from %s.",app_name, filename_alone, path);

	/* A "what failed" message when the failure is likely to be transient; this URI might be openable on subsequent attempts, or with some troubleshooting. */
	what_failed_transient = g_strdup_printf("%s could not read \"%s\" from %s.",app_name, filename_alone, path);

	switch (vfs_result) {
	default:
	case GNOME_VFS_ERROR_INTERNAL:
	case GNOME_VFS_ERROR_BAD_PARAMETERS:
	case GNOME_VFS_ERROR_GENERIC:
	case GNOME_VFS_ERROR_TOO_BIG:
	case GNOME_VFS_ERROR_NO_SPACE:
	case GNOME_VFS_ERROR_READ_ONLY:
	case GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM:
	case GNOME_VFS_ERROR_TOO_MANY_LINKS:
	case GNOME_VFS_ERROR_NOT_OPEN:
	case GNOME_VFS_ERROR_INVALID_OPEN_MODE:
	case GNOME_VFS_ERROR_READ_ONLY_FILE_SYSTEM:
	case GNOME_VFS_ERROR_FILE_EXISTS:
	case GNOME_VFS_ERROR_LOOP:
	case GNOME_VFS_ERROR_CANCELLED:
	case GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY:
	case GNOME_VFS_ERROR_NAME_TOO_LONG:

	case GNOME_VFS_ERROR_NOT_A_DIRECTORY: /* FIXME: when does this occur? */
	case GNOME_VFS_ERROR_IN_PROGRESS: /* FIXME: when does this occur? */
	case GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE: /* FIXME: when does this occur? */
	case GNOME_VFS_ERROR_SERVICE_OBSOLETE: /* FIXME: when does this occur? */
	case GNOME_VFS_ERROR_PROTOCOL_ERROR: /* FIXME: when does this occur? */
		{
			/* Unknown (or inapplicable) error */
			dialog = cong_error_dialog_new(what_failed_transient,
						       "An unexpected internal error occurred.",
						       "Try again.  If it fails again, file a bug report with the maintainer of this application.");
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
			/* Either "path not found" or "file not found": */
			/* FIXME Distinguish between the case of path not found and file not found: */
			dialog = cong_error_dialog_new(what_failed_transient,
						       "Either a file with that name is not present at the location specified, or the location does not exist",
						       /* FIXME:  isolate all these cases and handle separately.  Also deal separately with permission errors etc */
						       
						       "(i) Try checking that you spelt the file's name correctly.  Remember that capitalisation is significant (\"MyFile\" is not the same as \"MYFILE\" or \"myfile\").\n"
						       "(ii) Try using the GNOME Search Tool to find your file.");
		}
		break;
		
	case GNOME_VFS_ERROR_NOT_SUPPORTED:
	case GNOME_VFS_ERROR_NOT_PERMITTED:
		{
			/* FIXME: need some thought about the messages for this */
			gchar* why_failed = g_strdup_printf("The location \"%s\" does not support the reading of files.",path);
			dialog = cong_error_dialog_new(what_failed_permanent,
						       why_failed,
						       "Try loading a file from a different location.  If you think that you ought to be able to read this file, contact your system administrator.");
			g_free(why_failed);
		}
		break;
		
	case GNOME_VFS_ERROR_IO:
	case GNOME_VFS_ERROR_EOF:
		{
			dialog = cong_error_dialog_new(what_failed_transient,
						       "There were problems reading the content of the file.",
						       "Try again.  If it fails again, contact your system administrator.");
		}
		break;
		
	case GNOME_VFS_ERROR_CORRUPTED_DATA:
	case GNOME_VFS_ERROR_BAD_FILE:
		{
	    dialog = cong_error_dialog_new(what_failed_transient,
					   "The contents of the file seem to be corrupt.",
					   "Try again.  If it fails again, try looking for a backup copy of the file.");
		}
		break;
	case GNOME_VFS_ERROR_WRONG_FORMAT:
		{
	    dialog = cong_error_dialog_new(what_failed_transient,
					   "There were problems reading the contents of the file.",
					   "Try again.  If it fails again, contact your system administrator.");
		}
		break;
	case GNOME_VFS_ERROR_INVALID_URI:
		{
			/* FIXME: is case significant for VFS method names? */
			dialog = cong_error_dialog_new(what_failed_permanent,
						       "The system does not recognise that as a valid location.",
						       "(i) Try checking that you spelt the location correctly.  Remember that capitalisation is significant (\"http\" is not the same as \"Http\" or \"HTTP\").\n"
						       "(ii) Try using the GNOME Search Tool to find your file.");
		}
		break;
	case GNOME_VFS_ERROR_ACCESS_DENIED:
	  {
		  dialog = cong_error_dialog_new(what_failed_permanent,
						 "You do not have permission to read that file.",
						 "Try asking your system administrator to give you permission.");
	  }
	  break;
	case GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES:
		{
			dialog = cong_error_dialog_new(what_failed_transient,
						       "The system is trying to operate on too many files at once.",
						       "Try again.  If it fails again, try closing unwanted applications, or contact your system administrator.");
		}
		break;
		
	case GNOME_VFS_ERROR_INTERRUPTED:
		{
			/* FIXME: need a better "why-failed" message */
			dialog = cong_error_dialog_new(what_failed_transient,
						       "There were problems reading the contents of the file.",
						       "Try again.  If it fails again, contact your system administrator.");
		}
		break;
		
	case GNOME_VFS_ERROR_IS_DIRECTORY:
		{
			/* FIXME:  capitalisation issues */
			gchar* why_failed = g_strdup_printf("\"%s\" is a directory, rather than a file.",filename_alone);
			dialog = cong_error_dialog_new(what_failed_permanent,
						       why_failed,
						       "Try using the GNOME Search Tool to find your file.");
			g_free(why_failed);
	  }
		break;
	case GNOME_VFS_ERROR_NO_MEMORY:
		{
			dialog = cong_error_dialog_new(what_failed_transient,
					   "The system ran out of memory.",
						       "Try again.  If it fails again, try closing unwanted applications, or contact your system administrator.");
		}
		break;
	case GNOME_VFS_ERROR_HOST_NOT_FOUND:
		{
	    /* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new(what_failed_permanent,
						       "The server could not be contacted.",
						       "Try again.  If it fails again, the server may be down.");
		}
	  break;
	case GNOME_VFS_ERROR_INVALID_HOST_NAME:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new(what_failed_permanent,
						       "The server could not be contacted.",
						       "(i) Try checking that you spelt the location correctly.\n"
						       "(ii) Try again. If it fails again, the server may be down.");
		}
		break;
	case GNOME_VFS_ERROR_HOST_HAS_NO_ADDRESS:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new(what_failed_permanent,
						       "The server could not be contacted.",
						       "(i) Try checking that you spelt the location correctly.\n"
						       "(ii) Try again. If it fails again, the server may be down.");
		}
		break;
	case GNOME_VFS_ERROR_LOGIN_FAILED:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new(what_failed_transient,
						       "The system could not login to the location.",
						       "Try again. If it fails again, contact your system administrator.");
		}
		break;
		
	case GNOME_VFS_ERROR_DIRECTORY_BUSY:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new(what_failed_transient,
						       "The location was too busy.",
						       "Try again. If it fails again, contact your system administrator.");
		}
		break;

	}

	g_free(what_failed_transient);
	g_free(what_failed_permanent);
	g_free(filename_alone);
	g_free(path);
	g_free(app_name);

	g_assert(dialog);

	return dialog;
}

