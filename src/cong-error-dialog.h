/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-error-dialog.h
 *
 * Copyright (C) 2002 David Malcolm
 *
 * FIXME:  This file is currently licensed under the GPL, but is intended to eventually become part of a library licensed under the LGPL
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#ifndef __CONG_ERROR_DIALOG_H__
#define __CONG_ERROR_DIALOG_H__

G_BEGIN_DECLS

/* 
   Error handling facilities: 

   Although these have a "cong" prefix, I hope to eventually factor these out into a useful library for other apps to use (the prefix will then get changed).
   
   The intention is to make it really easy to get good error dialogs.  Hence the parent_window parameter is required, in order to set things up nicely with the window manager.
*/

/**
 * Routine to manufacture a GNOME HIG-compliant error dialog.
 */
GtkDialog* 
cong_error_dialog_new(GtkWindow *parent_window,
		      const gchar* what_failed, 
		      const gchar* why_failed, 
		      const gchar* suggestions);

/**
 * Routine to manufacture a GNOME HIG-compliant error dialog with a "convenience button" that does something relevant.
 */
GtkDialog* 
cong_error_dialog_new_with_convenience(GtkWindow *parent_window,
				       const gchar* what_failed, 
				       const gchar* why_failed, 
				       const gchar* suggestions,
				       const gchar* convenience_label,
				       const gchar* cancel_label,
				       gboolean is_convenience_default,
				       void (*convenience_action)(gpointer data),
				       gpointer convenience_data);

/**
 * Routine to run an error dialog.  Use in preference to gtk_dialog_run as it handles convenience buttons.
 */
void
cong_error_dialog_run(GtkDialog* dialog);

/**
 * Routine to run an error dialog and destroy it afterwards.  Use in preference to gtk_dialog_run as it handles convenience buttons.
 */
void
cong_error_dialog_do(GtkDialog* dialog);

/**
 * Routine to get at the application name in a form suitable for use in error reports.  Returns a freshly-allocated string.
 */
gchar* 
cong_error_get_appname(void);

/**
 * Routine to manufacture an error dialog for unimplemented functionality
 */
GtkDialog*
cong_error_dialog_new_from_unimplemented_feature(GtkWindow *parent_window,
						 const gchar* what_failed, 
						 const char* filename, 
						 int linenum);

/**
 * Routine to manufacture an error dialog for unimplemented functionality for when you have a Bugzilla bug ID
 */
GtkDialog*
cong_error_dialog_new_from_unimplemented_feature_with_bugzilla_id(GtkWindow *parent_window,
						     const gchar* what_failed, 
						     const char* filename, 
						     int linenum,
						     const gchar* bugzilla_url,
						     int bugzilla_id);

#define CONG_BUGZILLA_URL ("http://bugzilla.gnome.org")

#define CONG_DO_UNIMPLEMENTED_DIALOG(parent_window, what_failed) (cong_error_dialog_do(cong_error_dialog_new_from_unimplemented_feature((parent_window), (what_failed), __FILE__, __LINE__)))
#define CONG_DO_UNIMPLEMENTED_DIALOG_WITH_BUGZILLA_ID(parent_window, what_failed, bugzilla_id) (cong_error_dialog_do(cong_error_dialog_new_from_unimplemented_feature_with_bugzilla_id((parent_window), (what_failed), __FILE__, __LINE__, CONG_BUGZILLA_URL, (bugzilla_id))))

/**
 * Routine to manufacture an error dialog for when some arbitrary operation fails but you have a GError available to you
 */
GtkDialog*
cong_error_dialog_new_from_gerror(GtkWindow *toplevel_window,
				  const gchar *what_failed,
				  const char *details,
				  GError *error);

/**
 * Routine to manufacture an error dialog for when some shell operation fails, when you have access to the stderr output in the form of a string.
 */
GtkDialog*
cong_error_dialog_new_from_shell_command_failure_with_command_line(GtkWindow *parent_window,
								   const gchar *what_failed,
								   gint exit_status,
								   const gchar *standard_error,
								   const gchar *command_line);

/**
 * cong_error_dialog_new_from_shell_command_failure_with_argv
 * @parent_window:
 * @what_failed:
 * @exit_status:
 * @standard_error:
 * @argv
 * 
 * Routine to manufacture an error dialog for when some shell operation fails, when you have access to the stderr output in the form of a string.
 * argv is a NULL terminated array of strings.
 * 
 * Returns: 
 */
GtkDialog*
cong_error_dialog_new_from_shell_command_failure_with_argv(GtkWindow *parent_window,
							   const gchar *what_failed,
							   gint exit_status,
							   const gchar *standard_error,
							   const gchar **argv);

/**
 * Routine to manufacture a "what failed" string for when File->Open fails.
 * @string_uri:  the stringified URI from which you tried to open the file.
 */
gchar*
cong_error_what_failed_on_file_open_failure (const gchar *string_uri, 
					     gboolean transient);

/**
 * Routine to manufacture an error dialog for when File->Open fails.
 * @string_uri:  the stringified URI from which you tried to open the file.
 */
GtkDialog*
cong_error_dialog_new_from_file_open_failure (GtkWindow *parent_window,
					      const gchar* string_uri, 
					      gboolean transient, 
					      const gchar* why_failed, 
					      const gchar* suggestions);

/**
 * Routine to manufacture an error dialog for when File->Open fails.
 * @string_uri:  the stringified URI from which you tried to open the file.
 */
GtkDialog*
cong_error_dialog_new_from_file_open_failure_with_convenience (GtkWindow *parent_window,
							       const gchar* string_uri, 
							       gboolean transient, 
							       const gchar* why_failed, 
							       const gchar* suggestions,
							       const gchar* convenience_label,
							       void (*convenience_action)(gpointer data),
							       gpointer convenience_data);


/**
 * Routine to manufacture an error dialog for when File->Open fails.
 * @string_uri:  the URI from which you tried to open the file.
 * @vfs_result: the error code that occurred.
 */
GtkDialog*
cong_error_dialog_new_from_file_open_failure_with_vfs_result(GtkWindow *parent_window,
							     const gchar *string_uri,
							     GnomeVFSResult vfs_result);


/**
 * Routine to manufacture an error dialog for when File->Save (or File->Save as...) fails.
 * @string_uri:  the URI to which you tried to save the file.
 * @vfs_result: the error code that occurred.
 * @file_size: pointer to the size of the file if known, or NULL if not (useful if the error was due to lack of space)
 */
GtkDialog*
cong_error_dialog_new_from_file_save_failure(GtkWindow *parent_window,
					     const gchar *string_uri, 
					     GnomeVFSResult vfs_result, 
					     const GnomeVFSFileSize* file_size);

/**
 * cong_error_dialog_new_from_file_operation_failure
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
						  const gchar *technical_details);
/** 
 * A bunch of self-tests.
 */
void
cong_error_tests(GtkWindow *parent_window);

G_END_DECLS

#endif
