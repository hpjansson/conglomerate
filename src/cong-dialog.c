/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-dialog.c
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

struct CongDialogContent
{
	GtkWidget *vbox;
	GtkSizeGroup *size_group;
};

struct CongDialogCategory
{
	CongDialogContent *dialog_content;
	GtkWidget *outer_vbox;
	GtkWidget *label;
	GtkWidget *hbox;
	GtkWidget *inner_vbox;
};

/**
 * cong_dialog_content_new:
 * @within_notebook:
 *
 * TODO: Write me
 * Returns:
 */
CongDialogContent *
cong_dialog_content_new(gboolean within_notebook)
{
	CongDialogContent *content;

	content = g_new0(CongDialogContent, 1);

	content->vbox = gtk_vbox_new(FALSE, 18);
	if (within_notebook) {
		gtk_container_set_border_width(GTK_CONTAINER(content->vbox), 12); /* makes it much better if inside a GtkNotebook page */
	}
	
	content->size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	gtk_widget_show (content->vbox);

	return content;
}

/**
 * cong_dialog_content_get_widget:
 * @dialog_content:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget *
cong_dialog_content_get_widget(CongDialogContent *dialog_content)
{
	g_return_val_if_fail(dialog_content, NULL);

	return dialog_content->vbox;	
}

/**
 * cong_dialog_content_add_category:
 * @dialog_content:
 * @title:
 *
 * TODO: Write me
 * Returns:
 */
CongDialogCategory *
cong_dialog_content_add_category(CongDialogContent *dialog_content, const gchar *title)
{
	CongDialogCategory *category;
	gchar *markedup_title;
	GtkWidget *spacer;

	g_return_val_if_fail(dialog_content, NULL);
	g_return_val_if_fail(title, NULL);

	category = g_new0(CongDialogCategory,1);

	category->dialog_content = dialog_content;
	category->outer_vbox = gtk_vbox_new(FALSE, 6);

	gtk_container_add(GTK_CONTAINER(dialog_content->vbox), category->outer_vbox);

	markedup_title = g_strdup_printf("<span weight=\"bold\">%s</span>", title);

	category->label = gtk_label_new(markedup_title);
	gtk_label_set_use_markup(GTK_LABEL(category->label), TRUE);
	gtk_misc_set_alignment(GTK_MISC(category->label),0.0f, 0.5f);

	category->hbox = gtk_hbox_new(FALSE, 0);
	
	gtk_box_pack_start (GTK_BOX(category->outer_vbox), 
			    category->label, 
			    FALSE, 
			    TRUE, 
			    0);
	gtk_box_pack_start (GTK_BOX(category->outer_vbox), 
			    category->hbox, 
			    TRUE, 
			    TRUE, 
			    0);

	category->inner_vbox = gtk_vbox_new(FALSE, 6);

	spacer = gtk_label_new("    ");
	gtk_widget_show(spacer);

	gtk_container_add(GTK_CONTAINER(category->hbox), spacer);
	gtk_container_add(GTK_CONTAINER(category->hbox), category->inner_vbox);	

	g_free(markedup_title);

	gtk_widget_show (category->outer_vbox);
	gtk_widget_show (category->label);
	gtk_widget_show (category->hbox);
	gtk_widget_show (category->inner_vbox);

	return category;
}

/**
 * cong_dialog_category_add_field:
 * @category:
 * @title:
 * @widget:
 * @expand:
 *
 * TODO: Write me
 */
void 
cong_dialog_category_add_field (CongDialogCategory *category, 
				const gchar *title, 
				GtkWidget *widget,
				gboolean expand)
{
	GtkWidget *hbox;
	GtkWidget *label;

	g_return_if_fail(category);
	g_return_if_fail(title);
	g_return_if_fail(widget);

	g_assert(category->dialog_content);

	hbox = gtk_hbox_new(FALSE, 6);
	label = gtk_label_new(title);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f);
	gtk_size_group_add_widget(category->dialog_content->size_group, label);
	gtk_container_add(GTK_CONTAINER(hbox), label);
	gtk_container_add(GTK_CONTAINER(hbox), widget);
	gtk_box_pack_start (GTK_BOX(category->inner_vbox), 
			    hbox,
			    expand,
			    TRUE,
			    0);

	gtk_widget_show (label);
	gtk_widget_show (hbox);
}

/**
 * cong_dialog_category_add_selflabelled_field:
 * @category:
 * @widget:
 * @expand:
 *
 * TODO: Write me
 */
void 
cong_dialog_category_add_selflabelled_field (CongDialogCategory *category, 
					     GtkWidget *widget,
					     gboolean expand)
{
	g_return_if_fail(category);
	g_return_if_fail(widget);

	gtk_box_pack_start (GTK_BOX(category->inner_vbox), 
			    widget,
			    expand,
			    TRUE,
			    0);
}

/**
 * make_dialog_message:
 * @primary_text:
 * @secondary_text:
 * @tertiary_text:
 *
 * TODO: Write me
 * Returns:
 */
static gchar*
make_dialog_message(const gchar* primary_text, 
		    const gchar* secondary_text, 
		    const gchar* tertiary_text)
{
	g_return_val_if_fail(primary_text, NULL);

	if (secondary_text) {
		if (tertiary_text) {
			return g_strdup_printf("<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s\n\n%s", primary_text, secondary_text, tertiary_text);
		} else {
			return g_strdup_printf("<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s", primary_text, secondary_text);
		}
	} else {
		if (tertiary_text) {
			return g_strdup_printf("<span weight=\"bold\" size=\"larger\">%s</span>\n\n\n\n%s", primary_text, tertiary_text);
		} else {
			return g_strdup_printf("<span weight=\"bold\" size=\"larger\">%s</span>", primary_text);
		}
	}
}

/**
 * cong_alert_content_new:
 * @stock_icon:
 * @primary_text:
 * @secondary_text:
 * @tertiary_text:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget* 
cong_alert_content_new(const gchar* stock_icon,
		       const gchar* primary_text, 
		       const gchar* secondary_text, 
		       const gchar* tertiary_text)
{
	GtkWidget *hbox1;
	GtkWidget *image1;
	GtkWidget *label1;
	gchar* msg;

	g_return_val_if_fail(stock_icon, NULL);
	g_return_val_if_fail(primary_text, NULL);

	msg = make_dialog_message(primary_text, secondary_text, tertiary_text);
	
	hbox1 = gtk_hbox_new (FALSE, 12);
	gtk_widget_show (hbox1);
	gtk_container_set_border_width (GTK_CONTAINER (hbox1), 6);
	
	image1 = gtk_image_new_from_stock (stock_icon, GTK_ICON_SIZE_DIALOG);
	gtk_widget_show (image1);
	gtk_box_pack_start (GTK_BOX (hbox1), image1, FALSE, TRUE, 0);
	gtk_misc_set_alignment (GTK_MISC (image1), 0.5, 0);
	
	label1 = gtk_label_new (msg);
	gtk_widget_show (label1);
	gtk_box_pack_start (GTK_BOX (hbox1), label1, TRUE, TRUE, 0);
	gtk_label_set_use_markup (GTK_LABEL(label1), TRUE);
	gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label1), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label1), 0.5, 0);
	
	g_free(msg);

	return hbox1;
}

/**
 * cong_dialog_save_confirmation_alert_new:
 * @parent:
 * @document_name:
 * @seconds_since_last_save_or_load:
 *
 * TODO: Write me
 * Returns:
 */
GtkDialog *
cong_dialog_save_confirmation_alert_new(GtkWindow *parent, 
					const gchar *document_name,
					glong seconds_since_last_save_or_load)
{
	GtkWidget *dialog, *content;
	gchar *primary_text, *secondary_text;
	glong minutes;

	g_return_val_if_fail(document_name, NULL);

	dialog = gtk_dialog_new_with_buttons(NULL, /* empty title string */
					     parent,
					     GTK_DIALOG_MODAL,

					     _("_Close without Saving"),
					     CONG_SAVE_CONFIRMATION_RESULT_CLOSE_WITHOUT_SAVING,

					     GTK_STOCK_CANCEL,
					     CONG_SAVE_CONFIRMATION_RESULT_CANCEL,

					     GTK_STOCK_SAVE,
					     CONG_SAVE_CONFIRMATION_RESULT_SAVE_AND_CLOSE,

					     NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
					CONG_SAVE_CONFIRMATION_RESULT_SAVE_AND_CLOSE);
					
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);

	primary_text = g_strdup_printf(_("Save changes to document \"%s\" before closing?"), document_name);

	minutes = seconds_since_last_save_or_load/60;

	if (minutes>120) {
		secondary_text = g_strdup_printf(_("If you close without saving, changes from the past %li hours will be discarded."), (minutes/60));
	} else if (minutes>1) {
		secondary_text = g_strdup_printf(_("If you close without saving, changes from the past %li minutes will be discarded."), minutes);
	} else {
		secondary_text = g_strdup_printf(_("If you close without saving, changes from the past minute will be discarded."));
	}

	content = cong_alert_content_new(GTK_STOCK_DIALOG_WARNING,
					 primary_text, 
					 secondary_text, 
					 NULL);


	g_free(primary_text);
	g_free(secondary_text);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), content);
	gtk_widget_show_all(dialog);

	return GTK_DIALOG(dialog);
}

/**
 * cong_dialog_revert_confirmation_alert_new:
 * @parent:
 * @document_name:
 * @seconds_since_last_save_or_load:
 *
 * TODO: Write me
 * Returns:
 */
GtkDialog *
cong_dialog_revert_confirmation_alert_new(GtkWindow *parent, 
					  const gchar *document_name,
					  glong seconds_since_last_save_or_load)
{
	GtkWidget *dialog, *content;
	gchar *primary_text, *secondary_text;
	glong minutes;

	g_return_val_if_fail(document_name, NULL);

	dialog = gtk_dialog_new_with_buttons(NULL, /* empty title string */
					     parent,
					     GTK_DIALOG_MODAL,

					     GTK_STOCK_CANCEL,
					     CONG_REVERT_CONFIRMATION_RESULT_CANCEL,

					     GTK_STOCK_REVERT_TO_SAVED,
					     CONG_REVERT_CONFIRMATION_RESULT_REVERT,

					     NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
					CONG_REVERT_CONFIRMATION_RESULT_REVERT);
					
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);

	primary_text = g_strdup_printf(_("Revert unsaved changes to document \"%s\"?"), document_name);

	minutes = seconds_since_last_save_or_load/60;

	if (minutes>120) {
		secondary_text = g_strdup_printf(_("Changes from the past %li hours will be discarded."), (minutes/60));
	} else if (minutes>1) {
		secondary_text = g_strdup_printf(_("Changes from the past %li minutes will be discarded."), minutes);
	} else {
		secondary_text = g_strdup_printf(_("Changes from the past minute will be discarded."));
	}

	content = cong_alert_content_new(GTK_STOCK_DIALOG_WARNING,
					 primary_text, 
					 secondary_text, 
					 NULL);


	g_free(primary_text);
	g_free(secondary_text);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), content);
	gtk_widget_show_all(dialog);

	return GTK_DIALOG(dialog);
}

/**
 * cong_dialog_information_alert_new:
 * @parent:
 * @message:
 *
 * TODO: Write me
 * Returns:
 */
GtkDialog *
cong_dialog_information_alert_new(GtkWindow *parent, 
				  const gchar *message)
{
	GtkWidget *dialog, *content;

	g_return_val_if_fail(message, NULL);

	dialog = gtk_dialog_new_with_buttons(NULL, /* empty title string */
					     parent,
					     GTK_DIALOG_MODAL,

					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,

					     NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
					GTK_RESPONSE_OK);
					
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);

	content = cong_alert_content_new(GTK_STOCK_DIALOG_INFO,
					 message, 
					 NULL, 
					 NULL);


	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), content);
	gtk_widget_show_all(dialog);

	return GTK_DIALOG(dialog);
}
