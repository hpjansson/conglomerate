/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

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

static gchar*
make_dialog_message(const gchar* what_failed, 
		    const gchar* why_failed, 
		    const gchar* suggestions)
{
	return g_strdup_printf("<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s\n\n%s", what_failed, why_failed, suggestions);
}


GtkDialog* 
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

	msg = make_dialog_message(what_failed, why_failed, suggestions);
	
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
	GtkWidget *dialog2;
	GtkWidget *dialog_vbox2;
	GtkWidget *hbox2;
	GtkWidget *image2;
	GtkWidget *label2;
	GtkWidget *dialog_action_area2;
	GtkWidget *button1;
	GtkWidget *button2;
	gchar* msg;

	g_return_val_if_fail(what_failed, NULL);
	g_return_val_if_fail(why_failed, NULL);
	g_return_val_if_fail(suggestions, NULL);
	g_return_val_if_fail(convenience_label, NULL);
	g_return_val_if_fail(convenience_action, NULL);

	msg = make_dialog_message(what_failed, why_failed, suggestions);
	
	dialog2 = gtk_dialog_new ();
	gtk_container_set_border_width (GTK_CONTAINER (dialog2), 6);
	GTK_WINDOW (dialog2)->type = GTK_WINDOW_POPUP;
	gtk_window_set_resizable (GTK_WINDOW (dialog2), FALSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog2), FALSE);
	
	dialog_vbox2 = GTK_DIALOG (dialog2)->vbox;
	gtk_widget_show (dialog_vbox2);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox2), 2);
	
	hbox2 = gtk_hbox_new (FALSE, 12);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (dialog_vbox2), hbox2, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox2), 6);

	image2 = gtk_image_new_from_stock ("gtk-dialog-error", GTK_ICON_SIZE_DIALOG);
	gtk_widget_show (image2);
	gtk_box_pack_start (GTK_BOX (hbox2), image2, TRUE, TRUE, 0);
	gtk_misc_set_alignment (GTK_MISC (image2), 0.5, 0);
	
	label2 = gtk_label_new (msg);
	gtk_widget_show (label2);
	gtk_box_pack_start (GTK_BOX (hbox2), label2, FALSE, FALSE, 0);
	gtk_label_set_use_markup (GTK_LABEL(label2), TRUE);
	gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label2), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label2), 0.5, 0);
	
	dialog_action_area2 = GTK_DIALOG (dialog2)->action_area;
	gtk_widget_show (dialog_action_area2);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area2), 5);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area2), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog_action_area2), 10);

	button1 = gtk_button_new_with_mnemonic (convenience_label);
	gtk_widget_show (button1);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog2), button1, CONVENIENCE_BUTTON_ID);
	GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);
	
	button2 = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (button2);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog2), button2, GTK_RESPONSE_OK);
	GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);
  
	gtk_widget_grab_default (button2);

	g_free(msg);

	/* Record the convenience info as user-data for the dialog: */
	g_object_set_data(G_OBJECT(dialog2),"convenience-action",(gpointer)convenience_action);
	g_object_set_data(G_OBJECT(dialog2),"convenience-data",convenience_data);

	return GTK_DIALOG(dialog2);
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

