/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-error-file-open.c
 *
 * Copyright (C) 2002 David Malcolm
 *
 * FIXME:  This file is currently licensed under the GPL, but is intended to eventually become part of a library licensed under the LGPL
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#include <gtk/gtk.h>

#include "global.h"
#include <libgnome/libgnome.h>
#include "cong-error-dialog.h"
#include "cong-util.h"

#include "cong-vfs.h"

/**
 * cong_error_split_filename:
 * @filename:
 * @filename_alone:
 * @path:
 *
 * TODO: Write me
 */
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

/**
 * cong_error_get_appname:
 *
 * Returns: a string containing "Conglomerate"
 */
gchar* 
cong_error_get_appname(void)
{
	return g_strdup("Conglomerate");
}

static void on_search(gpointer data)
{
	char* argv[1];
	int process_id;

	g_message("on_search\n");

	/* Launch the GNOME Search Tool: */
	argv[0] = "gnome-search-tool";
	process_id = gnome_execute_async(NULL,1,argv);

	if (-1==process_id) {
		cong_error_dialog_do( cong_error_dialog_new(NULL, /* FIXME: ought to set up the parent window properly */
							    _("Conglomerate could not run the Search Tool.\n"),
							    "FIXME",
							    "FIXME") );
	}
}

/**
 * cong_error_what_failed_on_file_open_failure:
 * @string_uri:
 * @transient:
 *
 * TODO: Write me
 * Returns:
 */
gchar*
cong_error_what_failed_on_file_open_failure (const gchar *string_uri, 
					     gboolean transient)
{
	gchar* app_name;
	gchar* filename_alone;
	gchar* path;
	gchar* what_failed;

	g_return_val_if_fail (string_uri, NULL);

	app_name = cong_error_get_appname();

	cong_vfs_split_string_uri (string_uri, 
				   &filename_alone, 
				   &path);

	g_assert(filename_alone);
	g_assert(path);

	if (transient) {
		/* A "what failed" message when the failure is likely to be permanent; this URI won't be openable */
		what_failed = g_strdup_printf(_("%s cannot read \"%s\" from %s."),app_name, filename_alone, path);
	} else	{
		/* A "what failed" message when the failure is likely to be transient; this URI might be openable on subsequent attempts, or with some troubleshooting. */
		what_failed = g_strdup_printf(_("%s could not read \"%s\" from %s."),app_name, filename_alone, path);
	}

	g_free(filename_alone);
	g_free(path);
	g_free(app_name);	

	return what_failed;
}

/**
 * cong_error_dialog_new_from_file_open_failure:
 * @parent_window:
 * @string_uri:
 * @transient:
 * @why_failed:
 * @suggestions:
 *
 * TODO: Write me
 * Returns:
 */
GtkDialog*
cong_error_dialog_new_from_file_open_failure(GtkWindow *parent_window,
					     const gchar* string_uri, 
					     gboolean transient, 
					     const gchar* why_failed, 
					     const gchar* suggestions)
{
	GtkDialog* dialog = NULL;

	gchar* what_failed;

	g_return_val_if_fail(string_uri, NULL);
	g_return_val_if_fail(why_failed, NULL);
	g_return_val_if_fail(suggestions, NULL);

	what_failed = cong_error_what_failed_on_file_open_failure (string_uri, 
								   transient);
	
	dialog = cong_error_dialog_new (parent_window,
					what_failed,
					why_failed,
					suggestions);

	g_free(what_failed);
	
	return dialog;
}

/**
 * cong_error_dialog_new_from_file_open_failure_with_convenience:
 * @parent_window:
 * @string_uri:
 * @transient:
 * @why_failed:
 * @suggestions:
 * @convenience_label:
 * @convenience_action:
 * @convenience_data:
 *
 * TODO: Write me
 * Returns:
 */
GtkDialog*
cong_error_dialog_new_from_file_open_failure_with_convenience(GtkWindow *parent_window,
							      const gchar *string_uri, 
							      gboolean transient, 
							      const gchar* why_failed, 
							      const gchar* suggestions,
							      const gchar* convenience_label,
							      void (*convenience_action)(gpointer data),
							      gpointer convenience_data)
{
	GtkDialog* dialog = NULL;

	gchar* what_failed;

	g_return_val_if_fail(string_uri, NULL);
	g_return_val_if_fail(why_failed, NULL);
	g_return_val_if_fail(suggestions, NULL);

	what_failed = cong_error_what_failed_on_file_open_failure(string_uri, transient);
	
	dialog = cong_error_dialog_new_with_convenience(parent_window,
							what_failed,
							why_failed,
							suggestions,
							convenience_label,
							GTK_STOCK_CANCEL,
							FALSE,
							convenience_action,
							convenience_data);

	g_free(what_failed);
	
	return dialog;
}

/**
 * cong_error_dialog_new_from_file_open_failure_with_vfs_result:
 * @parent_window:
 * @string_uri:
 * @vfs_result:
 *
 * TODO: Write me
 * Returns:
 */
GtkDialog*
cong_error_dialog_new_from_file_open_failure_with_vfs_result(GtkWindow *parent_window,
							     const gchar *string_uri, 
							     GnomeVFSResult vfs_result)
{
	GtkDialog* dialog = NULL;
	gchar* filename_alone;
	gchar* path;
	GnomeVFSURI* vfs_uri;
	GnomeVFSURI* parent_uri;

	g_return_val_if_fail (string_uri, NULL);
	g_return_val_if_fail (GNOME_VFS_OK!=vfs_result, NULL);

	cong_vfs_split_string_uri (string_uri, &filename_alone, &path);

	g_assert(filename_alone);
	g_assert(path);

	/* Get at the parent URI in case it's needed: */
	vfs_uri = gnome_vfs_uri_new (string_uri);
	parent_uri = gnome_vfs_uri_get_parent (vfs_uri);

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
			dialog = cong_error_dialog_new_from_file_open_failure(parent_window, 
									      string_uri, TRUE, 
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
			/* Either "file not found" or "path not found": */
			/* Does the parent_uri exist? */
			GnomeVFSDirectoryHandle *handle;
			GnomeVFSResult vfs_result = gnome_vfs_directory_open_from_uri(&handle,
										      parent_uri,
										      GNOME_VFS_FILE_INFO_DEFAULT);

			if (vfs_result==GNOME_VFS_OK) {
				gnome_vfs_directory_close(handle);
				
				/* OK; the path exists, but the file doesn't: */
				dialog = cong_error_dialog_new_from_file_open_failure_with_convenience(parent_window, 
												 string_uri, TRUE, 
												 _("There is no file with that name at that location."),
												 _("(i) Try checking that you spelt the file's name correctly.  Remember that capitalisation is significant (\"MyFile\" is not the same as \"MYFILE\" or \"myfile\").\n(ii) Try using the Search Tool to find your file."),
												 _("Search"),
												 on_search,
												 NULL);
			} else {
				/* The path doesn't exist: */
				dialog = cong_error_dialog_new_from_file_open_failure_with_convenience(parent_window, 
												 string_uri, TRUE, 
												 _("The location does not exist."),
												 _("(i) Try checking that you spelt the location correctly.  Remember that capitalisation is significant (\"MyDirectory\" is not the same as \"mydirectory\" or \"MYDIRECTORY\").\n(ii) Try using the Search Tool to find your file."),
												 _("Search"),
												 on_search,
												 NULL);
		}
			
		}
		break;
		
	case GNOME_VFS_ERROR_NOT_SUPPORTED:
	case GNOME_VFS_ERROR_NOT_PERMITTED:
		{
			/* FIXME: need some thought about the messages for this */
			gchar* why_failed = g_strdup_printf(_("The location \"%s\" does not support the reading of files."),path);
			dialog = cong_error_dialog_new_from_file_open_failure(parent_window, 
									string_uri, FALSE, 
									why_failed,
									_("Try loading a file from a different location.  If you think that you ought to be able to read this file, contact your system administrator."));
			g_free(why_failed);
		}
		break;
		
	case GNOME_VFS_ERROR_IO:
	case GNOME_VFS_ERROR_EOF:
		{
			dialog = cong_error_dialog_new_from_file_open_failure(parent_window, 
									string_uri, TRUE, 
									_("There were problems reading the content of the file."),
									_("Try again.  If it fails again, contact your system administrator."));
		}
		break;
		
	case GNOME_VFS_ERROR_CORRUPTED_DATA:
	case GNOME_VFS_ERROR_BAD_FILE:
		{
			dialog = cong_error_dialog_new_from_file_open_failure(parent_window, 
									string_uri, TRUE, 
									_("The contents of the file seem to be corrupt."),
									_("Try again.  If it fails again, try looking for a backup copy of the file."));
		}
		break;
	case GNOME_VFS_ERROR_WRONG_FORMAT:
		{
			dialog = cong_error_dialog_new_from_file_open_failure(parent_window, 
									string_uri, TRUE, 
									_("There were problems reading the contents of the file."),
									_("Try again.  If it fails again, contact your system administrator."));
		}
		break;
	case GNOME_VFS_ERROR_INVALID_URI:
		{
			/* FIXME: is case significant for VFS method names? */
			dialog = cong_error_dialog_new_from_file_open_failure_with_convenience(parent_window, 
											 string_uri, FALSE, 
											 _("The system does not recognise that as a valid location."),
											 _("(i) Try checking that you spelt the location correctly.  Remember that capitalisation is significant (\"http\" is not the same as \"Http\" or \"HTTP\").\n(ii) Try using the Search Tool to find your file."),
											 _("Search"),
											 on_search,
											 NULL);

		}
		break;
	case GNOME_VFS_ERROR_ACCESS_DENIED:
		{
			dialog = cong_error_dialog_new_from_file_open_failure(parent_window, 
									string_uri, FALSE, 
									_("You do not have permission to read that file."),
									_("Try asking your system administrator to give you permission."));
		}
		break;
	case GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES:
		{
			dialog = cong_error_dialog_new_from_file_open_failure(parent_window, 
									string_uri, TRUE, 
									_("The system is trying to operate on too many files at once."),
									_("Try again.  If it fails again, try closing unwanted applications, or contact your system administrator."));
		}
		break;
		
	case GNOME_VFS_ERROR_INTERRUPTED:
		{
			/* FIXME: need a better "why-failed" message */
			dialog = cong_error_dialog_new_from_file_open_failure(parent_window, 
									string_uri, TRUE, 
									_("There were problems reading the contents of the file."),
									_("Try again.  If it fails again, contact your system administrator."));
		}
		break;
		
	case GNOME_VFS_ERROR_IS_DIRECTORY:
		{
			/* FIXME:  capitalisation issues */
			gchar* why_failed = g_strdup_printf(_("\"%s\" is a directory, rather than a file."),filename_alone);
			dialog = cong_error_dialog_new_from_file_open_failure_with_convenience(parent_window, 
											 string_uri, FALSE, 
											 why_failed,
											 _("Try using the Search Tool to find your file."),
											 _("Search"),
											 on_search,
											 NULL);

			g_free(why_failed);
	  }
		break;
	case GNOME_VFS_ERROR_NO_MEMORY:
		{
			dialog = cong_error_dialog_new_from_file_open_failure(parent_window, 
									string_uri, TRUE, 
									_("The system ran out of memory."),
									_("Try again.  If it fails again, try closing unwanted applications, or contact your system administrator."));
		}
		break;
	case GNOME_VFS_ERROR_HOST_NOT_FOUND:
		{
	    /* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new_from_file_open_failure(parent_window, 
									string_uri, FALSE, 
									_("The server could not be contacted."),
									_("Try again.  If it fails again, the server may be down."));
		}
	  break;
	case GNOME_VFS_ERROR_INVALID_HOST_NAME:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new_from_file_open_failure(parent_window, 
									string_uri, FALSE, 
									_("The server could not be contacted."),
									_("(i) Try checking that you spelt the location correctly.\n(ii) Try again. If it fails again, the server may be down."));
		}
		break;
	case GNOME_VFS_ERROR_HOST_HAS_NO_ADDRESS:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new_from_file_open_failure(parent_window, 
									string_uri, FALSE, 
									_("The server could not be contacted."),
									_("(i) Try checking that you spelt the location correctly.\n(ii) Try again. If it fails again, the server may be down."));
		}
		break;
	case GNOME_VFS_ERROR_LOGIN_FAILED:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new_from_file_open_failure(parent_window, 
									string_uri, TRUE, 
									_("The system could not login to the location."),
									_("Try again. If it fails again, contact your system administrator."));
		}
		break;
		
	case GNOME_VFS_ERROR_DIRECTORY_BUSY:
		{
			/* FIXME: need to think more about these messages */
			dialog = cong_error_dialog_new_from_file_open_failure(parent_window, 
									      string_uri, TRUE, 
									_("The location was too busy."),
									_("Try again. If it fails again, contact your system administrator."));
		}
		break;

	}

	gnome_vfs_uri_unref (parent_uri);
	gnome_vfs_uri_unref (vfs_uri);

	g_assert(dialog);

	return dialog;
}

static void on_details(gpointer data)
{
	GtkWidget *details_dialog = data;
	gtk_dialog_run(GTK_DIALOG(details_dialog));
	gtk_widget_destroy(details_dialog);
}

/**
 * cong_error_dialog_new_from_file_operation_failure:
 * @parent_window:
 * @what_failed:
 * @string_uri: the URI from which you tried to access file.
 * @vfs_result:
 * @technical_details:
 * 
 * Routine to manufacture an error dialog for when some file operation fails that doesn't fall into one of the categories above.
 * Displays what operation has failed, with a convenience button to get more techical information.
 * 
 * Returns:
 */
GtkDialog*
cong_error_dialog_new_from_file_operation_failure(GtkWindow *parent_window,
						  const gchar *what_failed,
						  const gchar *string_uri, 
						  GnomeVFSResult vfs_result, 
						  const gchar *technical_details)
{
	GtkDialog *dialog;
	GtkDialog *details_dialog;
	gchar *secondary_text;

	g_return_val_if_fail(what_failed,NULL);
	g_return_val_if_fail(technical_details,NULL);

	/* Manufacture a dialog that is displayed if the user requests further details: */
	secondary_text = g_strdup_printf(_("The error \"%s\" was reported whilst accessing \"%s\""), 
					 gnome_vfs_result_to_string(vfs_result),
					 string_uri);
	details_dialog = cong_error_dialog_new(parent_window,
						_("The program unexpectedly received an error report from the GNOME Virtual File System."), 
						secondary_text, 
						technical_details);

	dialog = cong_error_dialog_new_with_convenience(parent_window,
							what_failed,
							
							_("An unexpected error occurred."),
							_("For more information, click on the \"Details\" button."),							
							_("Details"),
							GTK_STOCK_OK,
							FALSE,
							on_details,							
							details_dialog);
	/* FIXME: this will leak the details dialog */

	return dialog;
}
