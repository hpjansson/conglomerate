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
#include "cong-dialog.h"
#include "cong-error-dialog.h"

#define CONVENIENCE_BUTTON_ID (0)

/**
 * cong_error_dialog_run:
 * @dialog:
 *
 * TODO: Write me
 */
void
cong_error_dialog_run(GtkDialog* dialog)
{
	gint result = gtk_dialog_run(dialog);
	
	switch (result) {
	default: g_assert_not_reached();
	case GTK_RESPONSE_DELETE_EVENT:
		/* (the dialog was deleted, probably by the user clicking on a window manager button) */	
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

/**
 * cong_error_dialog_do:
 * @dialog:
 *
 * TODO: Write me
 */
void
cong_error_dialog_do(GtkDialog* dialog)
{
	cong_error_dialog_run(dialog);
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

/**
 * cong_error_dialog_new:
 * @parent_window:
 * @what_failed:
 * @why_failed:
 * @suggestions:
 *
 * TODO: Write me
 */
GtkDialog* 
cong_error_dialog_new(GtkWindow *parent_window,
		      const gchar* what_failed, 
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
/* 	GTK_WINDOW (dialog1)->type = GTK_WINDOW_POPUP; */
	gtk_window_set_resizable (GTK_WINDOW (dialog1), FALSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog1), FALSE);

	gtk_window_set_transient_for(GTK_WINDOW(dialog1), parent_window);

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

/**
 * cong_error_dialog_new_with_convenience:
 *
 * @parent_window: Parent window for dialog.
 * @what_failed: The description of what failed
 * @why_failed: The reasons of failture
 * @suggestions: The suggestions for user.
 * @cancel_label: Label or gtk stock for cancel button.
 * @convenience_label: Label or gtk stock for convenience button.
 * @is_convenience_default: Should the convenience action be the default response.
 * @convenience_action: Callback to call on convenience response.
 * @convenience_data: Data to pass to the callback.
 * 
 * Description:
 * Use this function to create a dialog, that will report user about error
 * and suggest a way, how user can avoid those error. It is more preferable 
 * to report about errors with this function, rather than with 
 * cong_error_dialog_new.
 *
 * Returns: Dialog widget that should be passed to cong_error_dialog_run.
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
				       gpointer convenience_data)
{
	GtkWidget *dialog;
	GtkWidget *dialog_vbox;
	GtkWidget *dialog_content;
	GtkWidget *dialog_action_area;

	g_return_val_if_fail(what_failed, NULL);
	g_return_val_if_fail(why_failed, NULL);
	g_return_val_if_fail(suggestions, NULL);
	g_return_val_if_fail(convenience_label, NULL);
	g_return_val_if_fail(convenience_action, NULL);

	dialog = gtk_dialog_new ();
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent_window);

	dialog_vbox = GTK_DIALOG (dialog)->vbox;
	gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 12);
	
	dialog_content = cong_alert_content_new(GTK_STOCK_DIALOG_ERROR,
						what_failed, 
						why_failed, 
						suggestions);

	gtk_box_pack_start (GTK_BOX (dialog_vbox), dialog_content, TRUE, TRUE, 0);


       	dialog_action_area = GTK_DIALOG (dialog)->action_area;
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area), 6);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog_action_area), 12);
        
	if (is_convenience_default)
	  {
		gtk_dialog_add_button (GTK_DIALOG (dialog), cancel_label, GTK_RESPONSE_OK);
		gtk_dialog_add_button (GTK_DIALOG (dialog), convenience_label, CONVENIENCE_BUTTON_ID);
         	gtk_dialog_set_default_response (GTK_DIALOG(dialog), CONVENIENCE_BUTTON_ID);
	  }
	else
	  {
		gtk_dialog_add_button (GTK_DIALOG (dialog), convenience_label, CONVENIENCE_BUTTON_ID);
		gtk_dialog_add_button (GTK_DIALOG (dialog), cancel_label, GTK_RESPONSE_OK);
         	gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	  }
	  
	 

	/* Record the convenience info as user-data for the dialog: */
	g_object_set_data(G_OBJECT(dialog),"convenience-action",(gpointer)convenience_action);
	g_object_set_data(G_OBJECT(dialog),"convenience-data",convenience_data);

        gtk_widget_show_all (GTK_WIDGET(dialog));
	return GTK_DIALOG(dialog);
}

/**
 * cong_error_dialog_new_from_unimplemented_feature:
 * @parent_window:
 * @what_failed:
 * @filename:
 * @linenum:
 *
 * TODO: Write me
 */
GtkDialog*
cong_error_dialog_new_from_unimplemented_feature(GtkWindow *parent_window,
						 const gchar* what_failed, 
						 const char* filename, 
						 int linenum)
{
	GtkDialog* dialog;
	
	gchar* suggestion =  g_strdup_printf(_("If you are a programmer, the problem is in file %s at line %i\n"), filename, linenum);

	dialog =  cong_error_dialog_new(parent_window,
					what_failed,
					_("That feature has not yet been implemented.  Sorry."),
					suggestion);

	g_free(suggestion);

	return dialog;
}

/**
 * cong_error_dialog_new_from_unimplemented_feature_with_bugzilla_id:
 * @parent_window:
 * @what_failed:
 * @filename:
 * @linenum:
 * @bugzilla_url:
 * @bugzilla_id:
 *
 * TODO: Write me
 */
GtkDialog*
cong_error_dialog_new_from_unimplemented_feature_with_bugzilla_id(GtkWindow *parent_window,
								  const gchar* what_failed, 
								  const char* filename, 
								  int linenum,
								  const gchar* bugzilla_url,
								  int bugzilla_id)
{
	GtkDialog* dialog;
	
	gchar* why_failed = g_strdup_printf(_("That feature has not yet been implemented.  Sorry.\n\nThis is a known problem; it is bug #%i within the Bug Tracking System at %s"),
					    bugzilla_id,
					    bugzilla_url); 

	/* FIXME: would be nice to have convenience buttons/hyperlinks for the bug */

	gchar* suggestion =  g_strdup_printf(_("If you are a programmer, the problem is in file %s at line %i\n"), filename, linenum);

	dialog =  cong_error_dialog_new(parent_window,
					what_failed,
					why_failed,
					suggestion);

	g_free(suggestion);
	g_free(why_failed);

	return dialog;
}

static void on_gerror_details(gpointer data)
{
	GtkWidget *details_dialog = data;
	gtk_dialog_run(GTK_DIALOG(details_dialog));
	gtk_widget_destroy(details_dialog);
}


/**
 * Routine to manufacture an error dialog for when some arbitrary operation fails but you have a GError available to you
 */
/**
 * cong_error_dialog_new_from_gerror:
 * @toplevel_window:
 * @what_failed:
 * @details:
 * @error:
 *
 * TODO: Write me
 */
GtkDialog*
cong_error_dialog_new_from_gerror(GtkWindow *toplevel_window,
				  const gchar *what_failed,
				  const char *details,
				  GError *error)
{
	GtkDialog *dialog;
	GtkDialog *details_dialog;
	gchar *secondary_text;
	gchar *technical_details;

	g_return_val_if_fail(what_failed,NULL);
	g_return_val_if_fail(details,NULL);
	g_return_val_if_fail(error,NULL);

	/* Manufacture a dialog that is displayed if the user requests further details: */
	secondary_text = g_strdup_printf(_("An error was reported whilst attempting the operation \"%s\""), 
					 details
					 );
	technical_details = g_strdup_printf("%s\n<i>%s</i>",
					    _("A GError occurred; the error report was:"),
					    error->message);
	details_dialog = cong_error_dialog_new(toplevel_window,
					       what_failed, 
					       secondary_text, 
					       technical_details);

	dialog = cong_error_dialog_new_with_convenience(toplevel_window,

							what_failed,							
							_("An unexpected error occurred."),
							_("For more information, click on the \"Details\" button."),							
							_("Details"),
							GTK_STOCK_OK,
							FALSE,
							on_gerror_details,							
							details_dialog);
	/* FIXME: this will leak the details dialog */

	return dialog;
}

/**
 * cong_error_dialog_new_from_shell_command_failure_with_command_line:
 * @parent_window:
 * @what_failed:
 * @exit_status:
 * @standard_error:
 * @command_line:
 *
 * TODO: Write me
 */
GtkDialog*
cong_error_dialog_new_from_shell_command_failure_with_command_line(GtkWindow *parent_window,
								   const gchar *what_failed,
								   gint exit_status,
								   const gchar *standard_error,
								   const gchar *command_line)
{
	GtkDialog *dialog;
	GtkWidget *content;
	gchar *why_failed;
	gchar *tertiary_text;
	GtkWidget *text_view;
	GtkTextBuffer *text_buffer;
	GtkWidget *scrolled_window;

	g_return_val_if_fail(what_failed, NULL);
	g_return_val_if_fail(standard_error, NULL);
	g_return_val_if_fail(command_line, NULL);

	dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(NULL,
							parent_window,
							0,
							GTK_STOCK_OK,
							GTK_RESPONSE_OK,
							NULL));

	/* Add upper content: */
	why_failed = g_strdup_printf("%s\n<tt>%s</tt>",
                                     _("An error occurred whilst running this system command:"),
                                     command_line);

	tertiary_text  = g_strdup_printf(_("The error number reported was %d.  Below is the error text that was output by the command."), exit_status);
	content = cong_alert_content_new(GTK_STOCK_DIALOG_ERROR,
					 what_failed, 
					 why_failed, 
					 tertiary_text);
	g_free(why_failed);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   content);

	/*  Add scrollable text view of the stderr text: */
	text_view = gtk_text_view_new ();
	text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

	gtk_text_buffer_set_text (text_buffer, standard_error, -1);

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
		
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   scrolled_window);
	
			
	gtk_container_add (GTK_CONTAINER (scrolled_window),
			   text_view);

	/* Set up the dialog nicely: */
	gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
#if 0
	gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 400);
#endif

	gtk_widget_show_all(GTK_WIDGET(dialog));

	return dialog;
}

/**
 * cong_error_dialog_new_from_shell_command_failure_with_argv:
 * @parent_window:
 * @what_failed:
 * @exit_status:
 * @standard_error:
 * @argv:
 *
 * TODO: Write me
 */
GtkDialog*
cong_error_dialog_new_from_shell_command_failure_with_argv(GtkWindow *parent_window,
							   const gchar *what_failed,
							   gint exit_status,
							   const gchar *standard_error,
							   const gchar **argv)
{
	GtkDialog *dialog;
	gchar *command_line; 

	g_return_val_if_fail(what_failed, NULL);
	g_return_val_if_fail(standard_error, NULL);
	g_return_val_if_fail(argv, NULL);

	command_line  = g_strjoinv(" ", (gchar**)argv);

	dialog = cong_error_dialog_new_from_shell_command_failure_with_command_line(parent_window,
										    what_failed,
										    exit_status,
										    standard_error,
										    command_line);
	g_free(command_line);

	return dialog;
}
