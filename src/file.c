/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <stdlib.h>
#include <string.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "global.h"

static GtkWidget *filew;
static const char *file_name_selected;


static void destroy( GtkWidget *widget, gpointer   data )
{
	gtk_main_quit();
}


static void file_cancel_sel( GtkWidget *w, GtkFileSelection *fs )
{
	file_name_selected = 0;
	gtk_widget_destroy(filew);
}

static void file_ok_sel( GtkWidget *w, GtkFileSelection *fs )
{
	file_name_selected = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
	if (file_name_selected) file_name_selected = strdup(file_name_selected);
	gtk_widget_destroy(filew);
}




const gchar *cong_get_file_name(const gchar *title, 
				GtkWindow *parent_window)
{
	g_return_val_if_fail(title, NULL);
	g_return_val_if_fail(parent_window, NULL);

	/* Create a new file selection widget */
	filew = gtk_file_selection_new (title);
	          
	gtk_signal_connect (GTK_OBJECT (filew), "destroy", (GtkSignalFunc) destroy, &filew);

	/* Connect the ok_button to file_ok_sel function */
	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button), "clicked", (GtkSignalFunc) file_ok_sel, filew );

	/* Connect the cancel_button to destroy the widget */
	gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(filew)->cancel_button), "clicked", (GtkSignalFunc) file_cancel_sel, GTK_OBJECT (filew));

	gtk_window_set_transient_for(GTK_WINDOW(filew), parent_window);

	gtk_widget_show(filew);

	gtk_main();
	return(file_name_selected);
}
