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

CongDialogContent *cong_dialog_content_new(gboolean within_notebook)
{
	CongDialogContent *content;

	content = g_new0(CongDialogContent, 1);

	content->vbox = gtk_vbox_new(FALSE, 18);
	if (within_notebook) {
		gtk_container_set_border_width(GTK_CONTAINER(content->vbox), 12); /* makes it much better if inside a GtkNotebook page */
	}
	
	content->size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	return content;
}

GtkWidget *cong_dialog_content_get_widget(CongDialogContent *dialog_content)
{
	g_return_val_if_fail(dialog_content, NULL);

	return dialog_content->vbox;	
}

CongDialogCategory *cong_dialog_content_add_category(CongDialogContent *dialog_content, const gchar *title)
{
	CongDialogCategory *category;
	gchar *markedup_title;

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
	
	gtk_container_add(GTK_CONTAINER(category->outer_vbox), category->label);
	gtk_container_add(GTK_CONTAINER(category->outer_vbox), category->hbox);

	category->inner_vbox = gtk_vbox_new(FALSE, 6);

	gtk_container_add(GTK_CONTAINER(category->hbox), gtk_label_new("    "));
	gtk_container_add(GTK_CONTAINER(category->hbox), category->inner_vbox);	

	g_free(markedup_title);

	return category;
}

void cong_dialog_category_add_field(CongDialogCategory *category, const gchar *title, GtkWidget *widget)
{
	GtkWidget *hbox;
	GtkWidget *label;

	g_return_if_fail(category);
	g_return_if_fail(title);
	g_return_if_fail(widget);

	g_assert(category->dialog_content);

	hbox = gtk_hbox_new(FALSE, 6);
	label = gtk_label_new(title);
	gtk_size_group_add_widget(category->dialog_content->size_group, label);
	gtk_container_add(GTK_CONTAINER(hbox), label);
	gtk_container_add(GTK_CONTAINER(hbox), widget);
	gtk_container_add(GTK_CONTAINER(category->inner_vbox), hbox);
}

void cong_dialog_category_add_selflabelled_field(CongDialogCategory *category, GtkWidget *widget)
{
	g_return_if_fail(category);
	g_return_if_fail(widget);

	gtk_container_add(GTK_CONTAINER(category->inner_vbox), widget);
}

