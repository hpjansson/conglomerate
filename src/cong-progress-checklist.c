/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-progress-checklist.c
 *
 * Copyright (C) 2003 David Malcolm
 *
 * FIXME:  This file is currently licensed under the GPL, but is intended to eventually become part of a library licensed under the LGPL
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#include <gtk/gtk.h>
#include "cong-progress-checklist.h"

enum CongProgressChecklistStageState {
	STAGE_COMPLETED,
	STAGE_IN_PROGRESS,
	STAGE_PENDING
};

typedef struct CongProgressChecklistStage
{
	GtkLabel *marker_label;
	GtkLabel *descriptive_label;
	gchar *stage_name;
} CongProgressChecklistStage;

typedef struct CongProgressChecklistDetails
{
	guint num_stages;
	guint stages_completed;
	GList *list_of_stages; /* List of CongProgressChecklistStage owned by this object */

} CongProgressChecklistDetails;

typedef struct CongProgressChecklistDialogDetails
{
	CongProgressChecklist *progress_checklist;
} CongProgressChecklistDialogDetails;

static gunichar2 tick[2]={0x2713,0};
static gunichar2 arrow[2]={0x25b8,0};
 

static void setup_text_for_stage(CongProgressChecklistStage *stage, enum CongProgressChecklistStageState state)
{
	g_return_if_fail(stage);

	switch (state) {
	default: g_assert_not_reached();
	case STAGE_COMPLETED:
		/* Ought to precompute these: */
		{
			gchar*  tick_utf8 = g_utf16_to_utf8(tick,
							    -1,
							    NULL,
							    NULL,
							    NULL);
			gtk_label_set_markup (stage->marker_label, tick_utf8); /* Use unicode 0x2713 for tick */
			g_free(tick_utf8);
		}
		break;

	case STAGE_IN_PROGRESS:
		{
			gchar*  arrow_utf8 = g_utf16_to_utf8(arrow,
							     -1,
							     NULL,
							     NULL,
							     NULL);
#if 1
			gtk_label_set_markup (stage->marker_label, arrow_utf8);
#else
			gtk_label_set_markup (stage->marker_label, "<b>arrow</b>"); /* Use unicode 0x25b8 for arrow */
#endif
			g_free(arrow_utf8);
		}
		break;

	case STAGE_PENDING:
		gtk_label_set_markup (stage->marker_label, "");
		break;

	}

	if (state==STAGE_IN_PROGRESS) {
		gchar *text = g_strdup_printf("<b>%s</b>", stage->stage_name);
		gtk_label_set_markup (stage->descriptive_label, text);
		g_free(text);
	} else {
		gtk_label_set_markup (stage->descriptive_label, stage->stage_name);
	}
}

static enum CongProgressChecklistStageState get_state_for_stage(guint stage_index, guint num_stages_completed)
{
	if (stage_index<num_stages_completed) {
		return STAGE_COMPLETED;
	} else if (stage_index==num_stages_completed) {
		return STAGE_IN_PROGRESS;
	} else {
		return STAGE_PENDING;
	}
}

static void refresh_stages(CongProgressChecklistDetails *progress_checklist_details)
{
	guint stage_index=0;
	GList *iter;

	g_return_if_fail(progress_checklist_details);

	for (iter = progress_checklist_details->list_of_stages; iter; iter=iter->next) {
		setup_text_for_stage(iter->data, get_state_for_stage(stage_index, progress_checklist_details->stages_completed));

		stage_index++;
	}
}

/**
 * cong_progress_checklist_new:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget *
cong_progress_checklist_new(void)
{
	GtkTable *widget = GTK_TABLE(gtk_table_new(0,2,FALSE));
	CongProgressChecklistDetails *details = g_new0(CongProgressChecklistDetails, 1);
	gtk_table_set_row_spacings(widget,
				   6);
	gtk_table_set_col_spacings(widget,
				   6);

	g_object_set_data(G_OBJECT(widget),
			  "details",
			  details);

	return GTK_WIDGET(widget);
}

/**
 * cong_progress_checklist_add_stage:
 * @progress_checklist:
 * @stage_name:
 *
 * TODO: Write me
 */
void 
cong_progress_checklist_add_stage(CongProgressChecklist *progress_checklist,
				  const gchar *stage_name)
{
	CongProgressChecklistDetails *details;
	CongProgressChecklistStage *new_stage;

	g_return_if_fail(progress_checklist);
	g_return_if_fail(stage_name);

	details = g_object_get_data(G_OBJECT(progress_checklist),
				    "details");

	new_stage = g_new0(CongProgressChecklistStage,1);

	new_stage->stage_name = g_strdup(stage_name);
	new_stage->marker_label = GTK_LABEL(gtk_label_new(NULL));
	gtk_label_set_use_markup (new_stage->marker_label, TRUE);

	new_stage->descriptive_label = GTK_LABEL(gtk_label_new(NULL));
	gtk_label_set_use_markup (new_stage->descriptive_label, TRUE);

	/* add to table: */
	gtk_table_attach_defaults(GTK_TABLE(progress_checklist),
				  GTK_WIDGET(new_stage->marker_label),
				  0,
				  1,
				  details->num_stages,
				  details->num_stages+1);
	gtk_table_attach_defaults(GTK_TABLE(progress_checklist),
				  GTK_WIDGET(new_stage->descriptive_label),
				  1,
				  2,
				  details->num_stages,
				  details->num_stages+1);

	details->num_stages++;
	details->list_of_stages = g_list_append(details->list_of_stages, new_stage);

	refresh_stages(details);

	gtk_widget_show(GTK_WIDGET(new_stage->marker_label));
	gtk_widget_show(GTK_WIDGET(new_stage->descriptive_label));

}

/**
 * cong_progress_checklist_complete_stage:
 * @progress_checklist:
 *
 * TODO: Write me
 */
void 
cong_progress_checklist_complete_stage(CongProgressChecklist *progress_checklist)
{
	CongProgressChecklistDetails *details;

	g_return_if_fail(progress_checklist);

	details = g_object_get_data(G_OBJECT(progress_checklist),
				    "details");
	g_assert(details);

	details->stages_completed++;
	refresh_stages(details);
}

/**
 * cong_progress_checklist_dialog_new:
 * @title:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget *
cong_progress_checklist_dialog_new(const gchar *title,
				   GtkWindow *parent_window)
{
	GtkWidget *dialog;
	CongProgressChecklistDialogDetails *details = g_new0(CongProgressChecklistDialogDetails, 1);

	dialog = gtk_dialog_new_with_buttons(title,
					     parent_window,
					     GTK_DIALOG_MODAL,
					     NULL);
					
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);

	g_object_set_data(G_OBJECT(dialog),
			  "details",
			  details);

	details->progress_checklist = CONG_PROGRESS_CHECKLIST(cong_progress_checklist_new());

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), GTK_WIDGET(details->progress_checklist));
	gtk_widget_show_all(dialog);

	return GTK_WIDGET(dialog);
}

/**
 * cong_progress_checklist_dialog_get_progress_checklist:
 * @progress_checklist_dialog:
 *
 * TODO: Write me
 * Returns:
 */
CongProgressChecklist *
cong_progress_checklist_dialog_get_progress_checklist(CongProgressChecklistDialog *progress_checklist_dialog)
{
	CongProgressChecklistDialogDetails *details;

	g_return_val_if_fail(progress_checklist_dialog, NULL);
	
	details = g_object_get_data(G_OBJECT(progress_checklist_dialog),
				    "details");
	g_assert(details);

	return details->progress_checklist;
}

/**
 * cong_progress_checklist_dialog_set_progress_bar:
 * @progress_checklist:
 * @progress_bar:
 *
 * This function is not currently implemented
 */
void 
cong_progress_checklist_dialog_set_progress_bar(CongProgressChecklist *progress_checklist, 
						GtkWidget *progress_bar)
{
	/* FIXME: unwritten */
}
