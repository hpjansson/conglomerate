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
   
   The intention is to make it really easy to get good error dialogs.  Hence the parent_window parameter is required, in order to set things up niceky with the window manager.
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
cong_error_dialog_new_unimplemented(GtkWindow *parent_window,
				    const gchar* what_failed, 
				    const char* filename, 
				    int linenum);

/**
 * Routine to manufacture an error dialog for unimplemented functionality for when you have a Bugzilla bug ID
 */
GtkDialog*
cong_error_dialog_new_unimplemented_with_bugzilla_id(GtkWindow *parent_window,
						     const gchar* what_failed, 
						     const char* filename, 
						     int linenum,
						     const gchar* bugzilla_url,
						     int bugzilla_id);

#define CONG_BUGZILLA_URL ("http://bugzilla.gnome.org")

#define CONG_DO_UNIMPLEMENTED_DIALOG(parent_window, what_failed) (cong_error_dialog_do(cong_error_dialog_new_unimplemented((parent_window), (what_failed), __FILE__, __LINE__)))
#define CONG_DO_UNIMPLEMENTED_DIALOG_WITH_BUGZILLA_ID(parent_window, what_failed, bugzilla_id) (cong_error_dialog_do(cong_error_dialog_new_unimplemented_with_bugzilla_id((parent_window), (what_failed), __FILE__, __LINE__, CONG_BUGZILLA_URL, (bugzilla_id))))

/**
 * Routine to manufacture a "what failed" string for when File->Open fails.
 * @vfs_uri:  the URI from which you tried to open the file.
 */
gchar*
cong_error_what_failed_on_file_open_failure(const GnomeVFSURI* file_uri, 
					    gboolean transient);

/**
 * Routine to manufacture an error dialog for when File->Open fails.
 * @vfs_uri:  the URI from which you tried to open the file.
 */
GtkDialog*
cong_error_dialog_new_file_open_failed(GtkWindow *parent_window,
				       const GnomeVFSURI* file_uri, 
				       gboolean transient, 
				       const gchar* why_failed, 
				       const gchar* suggestions);

/**
 * Routine to manufacture an error dialog for when File->Open fails.
 * @vfs_uri:  the URI to which you tried to save the file.
 */
GtkDialog*
cong_error_dialog_new_file_open_failed_with_convenience(GtkWindow *parent_window,
							const GnomeVFSURI* file_uri, 
							gboolean transient, 
							const gchar* why_failed, 
							const gchar* suggestions,
							const gchar* convenience_label,
							void (*convenience_action)(gpointer data),
							gpointer convenience_data);


/**
 * Routine to manufacture an error dialog for when File->Open fails.
 * @vfs_uri:  the URI to which you tried to save the file.
 * @vfs_result: the error code that occurred.
 */
GtkDialog*
cong_error_dialog_new_file_open_failed_from_vfs_result(GtkWindow *parent_window,
						       const GnomeVFSURI* file_uri, 
						       GnomeVFSResult vfs_result);


/**
 * Routine to manufacture an error dialog for when File->Save (or File->Save as...) fails.
 * @vfs_uri:  the URI to which you tried to save the file.
 * @vfs_result: the error code that occurred.
 * @file_size: pointer to the size of the file if known, or NULL if not (useful if the error was due to lack of space)
 */
GtkDialog*
cong_error_dialog_new_file_save_failed(GtkWindow *parent_window,
				       const GnomeVFSURI* file_uri, 
				       GnomeVFSResult vfs_result, 
				       const GnomeVFSFileSize* file_size);

/** 
 * A bunch of self-tests.
 */
void
cong_error_tests(GtkWindow *parent_window);

G_END_DECLS

#endif
