/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-creation-record.c
 *
 * Copyright (C) 2004 David Malcolm
 *
 * Conglomerate is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Conglomerate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#include "global.h"
#include "cong-editor-creation-record.h"

#include "cong-eel.h"

typedef struct CongEditorFlowEvent CongEditorFlowEvent;
struct CongEditorFlowEvent
{
	enum CongEditorCreationEvent event;
	CongEditorLineIter *iter_before;
	CongEditorLineIter *iter_after;
};

CongEditorFlowEvent*
cong_editor_flow_event_new (enum CongEditorCreationEvent event,
			    CongEditorLineIter *iter_before,
			    CongEditorLineIter *iter_after)
{
	CongEditorFlowEvent *flow_event = g_new0 (CongEditorFlowEvent, 1);
	
	flow_event->event = event;

	flow_event->iter_before = iter_before;
	g_object_ref (G_OBJECT (flow_event->iter_before));

	flow_event->iter_after = iter_after;
	g_object_ref (G_OBJECT (flow_event->iter_after));

	return flow_event;
}

void
cong_editor_flow_event_free (CongEditorFlowEvent *flow_event)
{
	g_return_if_fail (flow_event);

	g_object_unref (G_OBJECT (flow_event->iter_before));
	g_object_unref (G_OBJECT (flow_event->iter_after));
	g_free (flow_event);
}

struct CongEditorCreationRecordPrivate
{
	CongEditorLineManager *line_manager;

	GList *list_of_flow_events;
};

CONG_DEFINE_CLASS_BEGIN (CongEditorCreationRecord, cong_editor_creation_record, CONG_EDITOR_CREATION_RECORD, GObject, G_TYPE_OBJECT)
CONG_DEFINE_CLASS_END ()

/* Implementation of CongEditorCreationRecord: */
CongEditorCreationRecord*
cong_editor_creation_record_construct (CongEditorCreationRecord *creation_record,
				       CongEditorLineManager *line_manager)
{
	g_return_val_if_fail (IS_CONG_EDITOR_CREATION_RECORD (creation_record), NULL);

	PRIVATE (creation_record)->line_manager = line_manager;

	return creation_record;
}

static void 
cong_editor_creation_record_dispose (GObject *object)
{
	CongEditorCreationRecord *creation_record = CONG_EDITOR_CREATION_RECORD (object);
	GList *iter;

	for (iter = PRIVATE (creation_record)->list_of_flow_events; iter; iter=iter->next) {
		cong_editor_flow_event_free ((CongEditorFlowEvent*)iter->data);
	}
	
	g_list_free (PRIVATE (creation_record)->list_of_flow_events);
	PRIVATE (creation_record)->list_of_flow_events = NULL;
}

CongEditorCreationRecord*
cong_editor_creation_record_new (CongEditorLineManager *line_manager)
{
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager), NULL);

	return cong_editor_creation_record_construct (g_object_new (CONG_EDITOR_CREATION_RECORD_TYPE, NULL),
						      line_manager);
}

void
cong_editor_creation_record_add_change (CongEditorCreationRecord *creation_record,
					enum CongEditorCreationEvent event,
					CongEditorLineIter *iter_before,
					CongEditorLineIter *iter_after)
{
	PRIVATE (creation_record)->list_of_flow_events = g_list_append (PRIVATE (creation_record)->list_of_flow_events,
									cong_editor_flow_event_new (event,
											       iter_before,
											       iter_after));
}

void
cong_editor_creation_record_undo_changes (CongEditorCreationRecord *creation_record)
{
	GList *iter;

	g_return_if_fail (IS_CONG_EDITOR_CREATION_RECORD (creation_record));

	for (iter=PRIVATE (creation_record)->list_of_flow_events; iter; iter=iter->next) {
		CongEditorFlowEvent *flow_event = (CongEditorFlowEvent*)(iter->data);

		cong_editor_line_manager_undo_change (PRIVATE (creation_record)->line_manager,
						      flow_event->event,
						      flow_event->iter_before,
						      flow_event->iter_after);
	}
}

CongEditorLineManager*
cong_editor_creation_record_get_line_manager (CongEditorCreationRecord *creation_record)
{
	g_return_val_if_fail (IS_CONG_EDITOR_CREATION_RECORD (creation_record), NULL);

	return PRIVATE (creation_record)->line_manager;
}

