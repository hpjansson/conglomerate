/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-dialog.h
 *
 * Copyright (C) 2002 David Malcolm
 *
 * FIXME:  This file is currently licensed under the GPL, but is intended to eventually become part of a library licensed under the LGPL
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#ifndef __CONG_DIALOG_H__
#define __CONG_DIALOG_H__

G_BEGIN_DECLS

/*
  Dialog-handling functions.

  Handy functions for building HIG-compliant property dialogs etc.
*/
typedef struct CongDialogContent CongDialogContent;
typedef struct CongDialogCategory CongDialogCategory;

/* An object suitable for use either as the innards of a dialog, or for a page in a property dialog */
CongDialogContent *cong_dialog_content_new(gboolean within_notebook);
GtkWidget *cong_dialog_content_get_widget(CongDialogContent *dialog_content);

/* Category headings within a CongDialogContent */
CongDialogCategory *cong_dialog_content_add_category(CongDialogContent *dialog_content, const gchar *title);

/* Method to add left-side labelled controls such as text boxes, option menus etc */
void cong_dialog_category_add_field(CongDialogCategory *category, const gchar *title, GtkWidget *widget);

/* Method to add right-side labelled controls usch as check boxes and radio buttons: */
void cong_dialog_category_add_selflabelled_field(CongDialogCategory *category, GtkWidget *widget);

/* Function to manufacture the "content area" of a dialog */
GtkWidget* 
cong_alert_content_new(const gchar* stock_icon,
		       const gchar* primary_text, 
		       const gchar* secondary_text, 
		       const gchar* tertiary_text);

enum CongSaveConfirmationResult
{
	CONG_SAVE_CONFIRMATION_RESULT_SAVE_AND_CLOSE = 1,
	CONG_SAVE_CONFIRMATION_RESULT_CLOSE_WITHOUT_SAVING,
	CONG_SAVE_CONFIRMATION_RESULT_CANCEL,
};

GtkDialog *cong_dialog_save_confirmation_alert_new(GtkWindow *parent, 
						   const gchar *document_name,
						   glong seconds_since_last_save_or_load);

enum CongRevertConfirmationResult
{
	CONG_REVERT_CONFIRMATION_RESULT_CANCEL = 1,
	CONG_REVERT_CONFIRMATION_RESULT_REVERT
};

GtkDialog *cong_dialog_revert_confirmation_alert_new(GtkWindow *parent, 
						     const gchar *document_name,
						     glong seconds_since_last_save_or_load);


G_END_DECLS

#endif
