/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-error-dialog.c
 *
 * Copyright (C) 2002 David Malcolm
 *
 * FIXME:  This file is currently licensed under the GPL, but is intended to eventually become part of a library licensed under the LGPL
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */
#include <gtk/gtk.h>

#include "global.h"

#define CONVENIENCE_BUTTON_ID (0)

void
cong_error_dialog_run(GtkDialog* dialog)
{
	gint result = gtk_dialog_run(dialog);
	
	switch (result) {
	default: g_assert(0);
	case GTK_RESPONSE_OK:
		/* Do nothing */
		break;
	case CONVENIENCE_BUTTON_ID:
		{
			/* Extract the convenience info from user-data of the dialog: */
			void (*convenience_action)(gpointer data) = g_object_get_data(G_OBJECT(dialog),"convenience-action");
			gpointer convenience_data = g_object_get_data(G_OBJECT(dialog),"convenience-data");

			g_assert(convenience_action);

			/* Invoke the callback: */
			(*convenience_action)(convenience_data);
		}
		break;
	}
}

void
cong_error_dialog_do(GtkDialog* dialog)
{
	cong_error_dialog_run(dialog);
	gtk_widget_destroy(GTK_WIDGET(dialog));
}


GtkDialog* 
cong_error_dialog_new(const gchar* what_failed, 
		      const gchar* why_failed, 
		      const gchar* suggestions)
{
	GtkWidget *dialog1;
	GtkWidget *dialog_vbox1;
	GtkWidget *dialog_content;
	GtkWidget *dialog_action_area1;
	GtkWidget *okbutton1;

	g_return_val_if_fail(what_failed, NULL);
	g_return_val_if_fail(why_failed, NULL);
	g_return_val_if_fail(suggestions, NULL);

	dialog1 = gtk_dialog_new ();
	gtk_container_set_border_width (GTK_CONTAINER (dialog1), 6);
	GTK_WINDOW (dialog1)->type = GTK_WINDOW_POPUP;
	gtk_window_set_resizable (GTK_WINDOW (dialog1), FALSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog1), FALSE);

	dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
	gtk_widget_show (dialog_vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox1), 2);
	
	dialog_content = cong_alert_content_new(GTK_STOCK_DIALOG_ERROR,
						what_failed, 
						why_failed, 
						suggestions);

	gtk_box_pack_start (GTK_BOX (dialog_vbox1), dialog_content, TRUE, TRUE, 0);

	dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
	gtk_widget_show (dialog_action_area1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 5);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog_action_area1), 10);
	
	okbutton1 = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (okbutton1);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), okbutton1, GTK_RESPONSE_OK);
	GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

	return GTK_DIALOG(dialog1);

}

GtkDialog* 
cong_error_dialog_new_with_convenience(const gchar* what_failed, 
				       const gchar* why_failed, 
				       const gchar* suggestions,
				       const gchar* convenience_label,
				       void (*convenience_action)(gpointer data),
				       gpointer convenience_data)
{
	GtkWidget *dialog;
	GtkWidget *dialog_vbox;
	GtkWidget *dialog_content;
	GtkWidget *dialog_action_area;
	GtkWidget *button1;
	GtkWidget *button2;

	g_return_val_if_fail(what_failed, NULL);
	g_return_val_if_fail(why_failed, NULL);
	g_return_val_if_fail(suggestions, NULL);
	g_return_val_if_fail(convenience_label, NULL);
	g_return_val_if_fail(convenience_action, NULL);

	dialog = gtk_dialog_new ();
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);
	GTK_WINDOW (dialog)->type = GTK_WINDOW_POPUP;
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);

	dialog_vbox = GTK_DIALOG (dialog)->vbox;
	gtk_widget_show (dialog_vbox);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 2);
	
	dialog_content = cong_alert_content_new(GTK_STOCK_DIALOG_ERROR,
						what_failed, 
						why_failed, 
						suggestions);

	gtk_box_pack_start (GTK_BOX (dialog_vbox), dialog_content, TRUE, TRUE, 0);


       	dialog_action_area = GTK_DIALOG (dialog)->action_area;
	gtk_widget_show (dialog_action_area);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area), 5);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog_action_area), 10);

	button1 = gtk_button_new_with_mnemonic (convenience_label);
	gtk_widget_show (button1);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button1, CONVENIENCE_BUTTON_ID);
	GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);
	
	button2 = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (button2);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button2, GTK_RESPONSE_OK);
	GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);
  
	gtk_widget_grab_default (button2);

	/* Record the convenience info as user-data for the dialog: */
	g_object_set_data(G_OBJECT(dialog),"convenience-action",(gpointer)convenience_action);
	g_object_set_data(G_OBJECT(dialog),"convenience-data",convenience_data);

	return GTK_DIALOG(dialog);
}

GtkDialog*
cong_error_dialog_new_unimplemented(const gchar* what_failed, const char* filename, int linenum)
{
	GtkDialog* dialog;
	
	gchar* suggestion =  g_strdup_printf("If you are a programmer, the problem is in file %s at line %i\n", filename, linenum);

	dialog =  cong_error_dialog_new(what_failed,
					"That feature has not yet been implemented.  Sorry.",
					suggestion);

	g_free(suggestion);

	return dialog;
}

