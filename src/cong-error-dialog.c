/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gtk/gtk.h>

#include "global.h"

GtkWidget* 
cong_error_dialog_new(const gchar* what_failed, 
		 const gchar* why_failed, 
		 const gchar* suggestions)
{
	GtkWidget *dialog1;
	GtkWidget *dialog_vbox1;
	GtkWidget *hbox1;
	GtkWidget *image1;
	GtkWidget *label1;
	GtkWidget *dialog_action_area1;
	GtkWidget *okbutton1;
	gchar* msg;

	g_return_val_if_fail(what_failed, NULL);
	g_return_val_if_fail(why_failed, NULL);
	g_return_val_if_fail(suggestions, NULL);

	msg = g_strdup_printf("<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s\n\n%s", what_failed, why_failed, suggestions);
	
	dialog1 = gtk_dialog_new ();
	gtk_container_set_border_width (GTK_CONTAINER (dialog1), 6);
	GTK_WINDOW (dialog1)->type = GTK_WINDOW_POPUP;
	gtk_window_set_resizable (GTK_WINDOW (dialog1), FALSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog1), FALSE);
	
	dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
	gtk_widget_show (dialog_vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox1), 2);
	
	hbox1 = gtk_hbox_new (FALSE, 12);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox1, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox1), 6);
	
	image1 = gtk_image_new_from_stock ("gtk-dialog-error", GTK_ICON_SIZE_DIALOG);
	gtk_widget_show (image1);
	gtk_box_pack_start (GTK_BOX (hbox1), image1, TRUE, TRUE, 0);
	gtk_misc_set_alignment (GTK_MISC (image1), 0.5, 0);
	
	label1 = gtk_label_new (msg);
	gtk_widget_show (label1);
	gtk_box_pack_start (GTK_BOX (hbox1), label1, FALSE, FALSE, 0);
	gtk_label_set_use_markup (GTK_LABEL(label1), TRUE);
	gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label1), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label1), 0.5, 0);
	
	dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
	gtk_widget_show (dialog_action_area1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 5);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog_action_area1), 10);
	
	okbutton1 = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (okbutton1);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), okbutton1, GTK_RESPONSE_OK);
	GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

	g_free(msg);

	return dialog1;
}

