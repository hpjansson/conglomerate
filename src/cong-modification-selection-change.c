/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-modification-selection-change.c
 *
 * Copyright (C) 2003 David Malcolm
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
#include "cong-modification-selection-change.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"
#include "cong-selection.h"

#define PRIVATE(x) ((x)->private)

struct CongModificationSelectionChangeDetails
{
	/* Undo info: */
	CongLocation old_logical_start;
	CongLocation old_logical_end;

	/* Redo info: */
	CongLocation new_logical_start;
	CongLocation new_logical_end;
};

/* Internal function declarations: */
static void
finalize (GObject *object);

static void
dispose (GObject *object);

static void
undo (CongModification *modification);

static void
redo (CongModification *modification);


/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongModificationSelectionChange, 
			cong_modification_selection_change,
			CongModification,
			CONG_MODIFICATION_TYPE );

static void
cong_modification_selection_change_class_init (CongModificationSelectionChangeClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	CONG_MODIFICATION_CLASS (klass)->undo = undo;
	CONG_MODIFICATION_CLASS (klass)->redo = redo;
}

static void
cong_modification_selection_change_instance_init (CongModificationSelectionChange *node)
{
	node->private = g_new0(CongModificationSelectionChangeDetails,1);
}

/**
 * cong_modification_selection_change_construct:
 * @modification_selection_change:
 * @doc:
 * @new_logical_start:
 * @new_logical_end:
 *
 * TODO: Write me
 * Returns:
 */
CongModificationSelectionChange*
cong_modification_selection_change_construct (CongModificationSelectionChange *modification_selection_change,
					      CongDocument *doc,
					      const CongLocation *new_logical_start,
					      const CongLocation *new_logical_end)
{
	CongSelection *selection = cong_document_get_selection (doc);

	g_assert (cong_location_is_valid (new_logical_start));
	g_assert (cong_location_is_valid (new_logical_end));
	/* FIXME: does the old value of the selection have to be valid as well? */

	cong_modification_construct (CONG_MODIFICATION(modification_selection_change),
				     doc);

	cong_location_copy_with_ref (doc, &PRIVATE(modification_selection_change)->old_logical_start, cong_selection_get_logical_start (selection));
	cong_location_copy_with_ref (doc, &PRIVATE(modification_selection_change)->old_logical_end, cong_selection_get_logical_end (selection));
	cong_location_copy_with_ref (doc, &PRIVATE(modification_selection_change)->new_logical_start, new_logical_start);
	cong_location_copy_with_ref (doc, &PRIVATE(modification_selection_change)->new_logical_end, new_logical_end);

	return modification_selection_change;
}

/**
 * cong_modification_selection_change_new:
 * @doc:
 * @new_logical_start:
 * @new_logical_end:
 *
 * TODO: Write me
 * Returns:
 */
CongModification*
cong_modification_selection_change_new (CongDocument *doc,
					const CongLocation *new_logical_start,
					const CongLocation *new_logical_end)
{
	return CONG_MODIFICATION(cong_modification_selection_change_construct (CONG_MODIFICATION_SELECTION_CHANGE(g_object_new (CONG_MODIFICATION_SELECTION_CHANGE_TYPE, NULL)),
									       doc,
									       new_logical_start,
									       new_logical_end));
}

/**
 * finalize:
 * @object:
 *
 * TODO: Write me
 */
void
finalize (GObject *object)
{
	CongModificationSelectionChange *modification_selection_change = CONG_MODIFICATION_SELECTION_CHANGE (object);

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongModificationSelectionChange::finalize");
#endif

	g_free (modification_selection_change->private);
	modification_selection_change->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);

}

static void
dispose (GObject *object)
{
	CongModificationSelectionChange *modification_selection_change = CONG_MODIFICATION_SELECTION_CHANGE (object);
	CongDocument *doc = cong_modification_get_document (CONG_MODIFICATION (object));

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongModificationSelectionChange::dispose");
#endif

	g_assert (modification_selection_change->private);
	
	/* Cleanup: */
	cong_location_nullify_with_ref (doc, &PRIVATE(modification_selection_change)->old_logical_start);
	cong_location_nullify_with_ref (doc, &PRIVATE(modification_selection_change)->old_logical_end);
	cong_location_nullify_with_ref (doc, &PRIVATE(modification_selection_change)->new_logical_start);
	cong_location_nullify_with_ref (doc, &PRIVATE(modification_selection_change)->new_logical_end);

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
undo (CongModification *modification)
{
	CongModificationSelectionChange *modification_selection_change = CONG_MODIFICATION_SELECTION_CHANGE (modification);
	CongDocument *doc = cong_modification_get_document (modification);
	CongSelection *selection = cong_document_get_selection (doc);

	cong_document_begin_edit (doc);
	
	cong_selection_set_logical_range (selection,
					  &PRIVATE(modification_selection_change)->old_logical_start,
					  &PRIVATE(modification_selection_change)->old_logical_end);

	cong_document_private_on_selection_change (doc);
	
	cong_document_end_edit (doc);
}

static void
redo (CongModification *modification)
{
	CongModificationSelectionChange *modification_selection_change = CONG_MODIFICATION_SELECTION_CHANGE (modification);
	CongDocument *doc = cong_modification_get_document (modification);
	CongSelection *selection = cong_document_get_selection (doc);

	cong_document_begin_edit (doc);
	
	cong_selection_set_logical_range (selection,
					  &PRIVATE(modification_selection_change)->new_logical_start,
					  &PRIVATE(modification_selection_change)->new_logical_end);

	cong_document_private_on_selection_change (doc);
	
	cong_document_end_edit (doc);
}

