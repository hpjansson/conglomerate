#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "global.h"

GtkWidget *newdoc;

int capsula_mode = 0;

char *capsula_str = "capsula";
char *extracta_str = "extracta";
char *selecta_str = "selecta";
char *informa_str = "informa";
char *analiza_str = "analiza";


int new_document()
{
	gui_window_new_document_make();
	return(1);
}


static gint new_document_from_template(GtkWidget *w, char *type)
{
	gtk_widget_destroy(newdoc);

#ifndef RELEASE
	fputs(type, stdout);
	printf("\n");
#endif	

	if (capsula_mode && !strcmp(type, "capsula"))
	{
/*		
		add_capsula_template();
 */
	}
	else
	{
	  xmlview_destroy(TRUE);
/*		
	  get_template(type);
 */
	}

	gtk_widget_set_sensitive(cong_gui_get_button_submit(&the_gui), TRUE);

	if (!strcmp(type, "capsula"))
	{
/*		
		vect_win_open();
 */
		capsula_mode = 1;
	}
	else capsula_mode = 0;

	return(TRUE);
}


int gui_window_new_document_make()
{
	GtkWidget *w0, *w1;

  newdoc = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(newdoc), 10);
  gtk_widget_set_usize(GTK_WIDGET(newdoc), 240, 120);
  gtk_window_set_title(GTK_WINDOW(newdoc), "Select document class");
  gtk_window_set_position(GTK_WINDOW(newdoc), GTK_WIN_POS_MOUSE);
	gtk_window_set_modal(GTK_WINDOW(newdoc), 1);
	gtk_window_set_policy(GTK_WINDOW(newdoc), 0, 0, 1);

  /* --- Window -> vbox --- */

  w0 = gtk_vbox_new(TRUE, 1);
  gtk_container_add(GTK_CONTAINER(newdoc), w0);
  gtk_widget_show(w0);

	/* Window -> vbox -> buttons */

	w1 = gtk_button_new_with_label("Capsula");
	gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
	gtk_widget_show(w1);
  gtk_signal_connect(GTK_OBJECT(w1), "clicked", (GtkSignalFunc) new_document_from_template, (gpointer) capsula_str);

#if 0	
	w1 = gtk_button_new_with_label("Extracta");
	gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
	gtk_widget_show(w1);
  gtk_signal_connect(GTK_OBJECT(w1), "clicked", (GtkSignalFunc) new_document_from_template, (gpointer) extracta_str);

	w1 = gtk_button_new_with_label("Informa");
	gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
	gtk_widget_show(w1);
  gtk_signal_connect(GTK_OBJECT(w1), "clicked", (GtkSignalFunc) new_document_from_template, (gpointer) informa_str);

	w1 = gtk_button_new_with_label("Selecta");
	gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
	gtk_widget_show(w1);
  gtk_signal_connect(GTK_OBJECT(w1), "clicked", (GtkSignalFunc) new_document_from_template, (gpointer) selecta_str);

	w1 = gtk_button_new_with_label("Analiza");
	gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
	gtk_widget_show(w1);
  gtk_signal_connect(GTK_OBJECT(w1), "clicked", (GtkSignalFunc) new_document_from_template, (gpointer) analiza_str);
#endif
	
	gtk_widget_show(newdoc);
	return(1);
}
