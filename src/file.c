/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <stdlib.h>
#include <string.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "global.h"
#include "cong-file-selection.h"

/*
  This file isolates the API change-over between gtk_file_selection and gtk_file_chooser.
 */
#if 0
/* Implementation in terms of gtk_file_chooser: */
gchar*
cong_get_file_name (const gchar *title, 
		    const gchar *filename,
		    GtkWindow *parent_window,
		    CongFileChooserAction cong_action)
{
	gchar *result = NULL;
	GtkWidget *dialog;
	GtkFileChooserAction gtk_action;

	g_return_val_if_fail (title, NULL);
	g_return_val_if_fail (parent_window, NULL);

	switch (cong_action) {
	default: g_assert_not_reached ();
	case CONG_FILE_CHOOSER_ACTION_OPEN:
		gtk_action = GTK_FILE_CHOOSER_ACTION_OPEN;
		break;

	case CONG_FILE_CHOOSER_ACTION_SAVE:
		gtk_action = GTK_FILE_CHOOSER_ACTION_SAVE;
		break;
	}
	
	dialog = gtk_file_chooser_dialog_new (title,
					      parent_window,
					      gtk_action,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					      NULL);

	if (filename) {
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), filename);
	}
#if 0
	GtkFileFilter *filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, "All Files");
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

	GtkFileFilter *test_filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (test_filter, "My Test Filter");
	gtk_file_filter_add_mime_type (test_filter, "text/xml");
	gtk_file_filter_add_mime_type (test_filter, "x-directory/normal");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), 
				     test_filter);
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), 
				     test_filter);
#endif
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		result = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	}
	
	gtk_widget_destroy (dialog);
	
	return result;
}
#else

/* Implementation in terms of gtk_file_selection: */
/**
 * cong_get_file_name:
 * @title:
 * @filename:
 * @parent_window:
 * @cong_action:
 * 
 * TODO: Write me
 */
gchar*
cong_get_file_name (const gchar *title, 
		    const gchar *filename,
		    GtkWindow *parent_window,
		    CongFileChooserAction cong_action)
{
	GtkFileSelection *file_selection;
	gint result_int;
	gchar *result_filename;

	g_return_val_if_fail (title, NULL);
	g_return_val_if_fail (parent_window, NULL);

	/* Create a new file selection widget */
	file_selection = GTK_FILE_SELECTION(gtk_file_selection_new (title));

	if (filename) {
		gtk_file_selection_set_filename(file_selection, 
						filename);
	}

	gtk_window_set_transient_for(GTK_WINDOW(file_selection), 
				     parent_window);

	result_int = gtk_dialog_run(GTK_DIALOG(file_selection));

	switch (result_int) {
	default: g_assert(0);
	case GTK_RESPONSE_DELETE_EVENT:
	case GTK_RESPONSE_NONE:
	case GTK_RESPONSE_CANCEL:
		result_filename = NULL;
		break;

	case GTK_RESPONSE_OK:
		result_filename = g_strdup(gtk_file_selection_get_filename(file_selection));
		break;
	}

	gtk_widget_destroy(GTK_WIDGET(file_selection));

	return result_filename;
}
#endif
