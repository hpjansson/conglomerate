/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <stdlib.h>
#include <string.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "global.h"
#include "cong-file-selection.h"

/**
 * cong_get_file_name:
 * @title:
 * @filename:
 * @parent_window:
 * @action:
 * @list_of_filters:
 * 
 * TODO: Write me
 * Returns:
 */

gchar*
cong_get_file_name (const gchar *title, 
		    const gchar *filename,
		    GtkWindow *parent_window,
		    CongFileChooserAction cong_action,
		    GList *list_of_filters)
{
	return cong_get_file_name_with_filter (title, 
					       filename,
					       parent_window,
					       cong_action,
					       list_of_filters,
					       NULL);
}

gchar*
cong_get_file_name_with_filter (const gchar *title, 
				const gchar *filename,
				GtkWindow *parent_window,
				CongFileChooserAction cong_action,
				GList *list_of_filters,
				GtkFileFilter **output_filter)
{
	gchar *result = NULL;
	GtkWidget *dialog;
	GtkFileChooserAction gtk_action;
	GList *iter;

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
					      (gtk_action==GTK_FILE_CHOOSER_ACTION_SAVE)?GTK_STOCK_SAVE:GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					      NULL);

	if (filename) {
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), filename);
	}

	for (iter = list_of_filters; iter; iter=iter->next) {
		gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog),
					     GTK_FILE_FILTER (iter->data));
	}

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		result = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		if (output_filter) {
			GtkFileFilter *filter = gtk_file_chooser_get_filter (GTK_FILE_CHOOSER (dialog));
			g_assert (GTK_IS_FILE_FILTER (filter));
			*output_filter = filter;
		}
	}	

	gtk_widget_destroy (dialog);
	
	return result;
}

GList*
cong_file_selection_make_xml_filter_list (void)
{
	GList *list = NULL;
	GtkFileFilter *test_filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (test_filter, _("XML Files"));
	gtk_file_filter_add_mime_type (test_filter, "text/xml");
	gtk_file_filter_add_mime_type (test_filter, "x-directory/normal");

	/* Workarounds for various files that don't appear for me: */
	gtk_file_filter_add_pattern (test_filter, "*.glade");
	gtk_file_filter_add_pattern (test_filter, "*.xsl");

	list = g_list_append(list, test_filter);

 	return list;	
}

