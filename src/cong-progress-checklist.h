/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-progress-checklist.h
 *
 * Copyright (C) 2003 David Malcolm
 *
 * FIXME:  This file is currently licensed under the GPL, but is intended to eventually become part of a library licensed under the LGPL
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#ifndef __CONG_PROGRESS_CHECKLIST__
#define __CONG_PROGRESS_CHECKLIST__

G_BEGIN_DECLS


/**
   A widget for producing HIG-compliant progress checklists
 */
typedef GtkTable CongProgressChecklist;
#define CONG_PROGRESS_CHECKLIST(x) ((CongProgressChecklist*)(x))
GtkWidget *cong_progress_checklist_new(void);
void cong_progress_checklist_add_stage(CongProgressChecklist *progress_checklist,
				       const gchar *stage_name);
void cong_progress_checklist_complete_stage(CongProgressChecklist *progress_checklist);

/**
   A dialog embedding a HIG-compliant progress checklist, with or without a progress indicator, and with a "Cancel" button.
 */
typedef GtkDialog CongProgressChecklistDialog;
#define CONG_PROGRESS_CHECKLIST_DIALOG(x) ((CongProgressChecklistDialog*)(x))
GtkWidget *cong_progress_checklist_dialog_new(const gchar *title,
					      GtkWindow *parent_window);
CongProgressChecklist *cong_progress_checklist_dialog_get_progress_checklist(CongProgressChecklistDialog *progress_checklist_dialog);
void cong_progress_checklist_dialog_set_progress_bar(CongProgressChecklist *progress_checklist, 
						    GtkWidget *progress_bar);

G_END_DECLS

#endif
