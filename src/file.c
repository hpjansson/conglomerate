/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <stdlib.h>
#include <string.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "global.h"
#include "cong-file-selection.h"

#if 0
/* Implementation in terms of new GtkFileChooser interface: */
gchar *cong_get_file_name(const gchar *title, 
			  const gchar *filename,
			  GtkWindow *parent_window,
			  CongFileChooserAction action)
{
	GtkWidget *dialog;
	gchar *result = NULL;
	GtkFileChooserAction gtk_action;

	g_return_val_if_fail (title, NULL);
	g_return_val_if_fail (parent_window, NULL);

	switch (action) {
	default: g_assert_not_reached();
	case GTK_FILE_CHOOSER_ACTION_OPEN:
		gtk_action = GTK_FILE_CHOOSER_ACTION_OPEN;
		break;
	case GTK_FILE_CHOOSER_ACTION_SAVE:
		gtk_action = GTK_FILE_CHOOSER_ACTION_SAVE;
		break;		
	}

	dialog = gtk_file_chooser_dialog_new (title,
					      parent_window,
					      gtk_action,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					      NULL);
	
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		result = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	}
	
	gtk_widget_destroy (dialog);
	
	return result;
}
#else
/* Implementation in terms of legacy GtkFileSelection interface: */
gchar *cong_get_file_name(const gchar *title, 
			  const gchar *filename,
			  GtkWindow *parent_window,
			  CongFileChooserAction action)
{
	GtkFileSelection *file_selection;
	gint result_int;
	gchar *result_filename;

	g_return_val_if_fail(title, NULL);
	g_return_val_if_fail(parent_window, NULL);

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
