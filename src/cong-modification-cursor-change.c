/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-modification-cursor-change.c
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
#include "cong-modification-cursor-change.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"
#include "cong-command.h"

#define PRIVATE(x) ((x)->private)

struct CongModificationCursorChangeDetails
{
	/* Undo info: */
	CongLocation old_location;

	/* Redo info: */
	CongLocation new_location;
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
GNOME_CLASS_BOILERPLATE(CongModificationCursorChange, 
			cong_modification_cursor_change,
			CongModification,
			CONG_MODIFICATION_TYPE );

static void
cong_modification_cursor_change_class_init (CongModificationCursorChangeClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	CONG_MODIFICATION_CLASS (klass)->undo = undo;
	CONG_MODIFICATION_CLASS (klass)->redo = redo;
}

static void
cong_modification_cursor_change_instance_init (CongModificationCursorChange *node)
{
	node->private = g_new0(CongModificationCursorChangeDetails,1);
}

/**
 * cong_modification_cursor_change_construct:
 * @modification_cursor_change:
 * @doc:
 * @new_location:
 *
 * TODO: Write me
 * Returns:
 */
CongModificationCursorChange*
cong_modification_cursor_change_construct (CongModificationCursorChange *modification_cursor_change,
					      CongDocument *doc,
					      const CongLocation *new_location)
{
	CongCursor *cursor = cong_document_get_cursor (doc);

	g_assert (cong_location_is_valid (new_location));
	/* FIXME: does the old value of the cursor have to be valid as well? */

	cong_modification_construct (CONG_MODIFICATION(modification_cursor_change),
				     doc);

	cong_location_copy_with_ref (doc, &PRIVATE(modification_cursor_change)->old_location, cong_cursor_get_location (cursor));
	cong_location_copy_with_ref (doc, &PRIVATE(modification_cursor_change)->new_location, new_location);

	return modification_cursor_change;
}

/**
 * cong_modification_cursor_change_new:
 * @doc:
 * @new_location:
 *
 * TODO: Write me
 * Returns:
 */
CongModification*
cong_modification_cursor_change_new (CongDocument *doc,
				     const CongLocation *new_location)
{
	return CONG_MODIFICATION(cong_modification_cursor_change_construct (CONG_MODIFICATION_CURSOR_CHANGE(g_object_new (CONG_MODIFICATION_CURSOR_CHANGE_TYPE, NULL)),
									    doc,
									    new_location));
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
	CongModificationCursorChange *modification_cursor_change = CONG_MODIFICATION_CURSOR_CHANGE (object);

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongModificationCursorChange::finalize");
#endif

	g_free (modification_cursor_change->private);
	modification_cursor_change->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);

}

static void
dispose (GObject *object)
{
	CongModificationCursorChange *modification_cursor_change = CONG_MODIFICATION_CURSOR_CHANGE (object);
	CongDocument *doc = cong_modification_get_document (CONG_MODIFICATION (object));

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongModificationCursorChange::dispose");
#endif

	g_assert (modification_cursor_change->private);
	
	/* Cleanup: */
	cong_location_nullify_with_ref (doc, &PRIVATE(modification_cursor_change)->old_location);
	cong_location_nullify_with_ref (doc, &PRIVATE(modification_cursor_change)->new_location);

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
undo (CongModification *modification)
{
	CongModificationCursorChange *modification_cursor_change = CONG_MODIFICATION_CURSOR_CHANGE (modification);
	CongDocument *doc = cong_modification_get_document (modification);
	CongCursor *cursor = cong_document_get_cursor (doc);

	cong_document_begin_edit (doc);
	
	cong_location_copy (&cursor->location,
			    &PRIVATE(modification_cursor_change)->old_location);

	cong_document_private_on_cursor_change (doc);
	
	cong_document_end_edit (doc);
}

static void
redo (CongModification *modification)
{
	CongModificationCursorChange *modification_cursor_change = CONG_MODIFICATION_CURSOR_CHANGE (modification);
	CongDocument *doc = cong_modification_get_document (modification);
	CongCursor *cursor = cong_document_get_cursor (doc);

	cong_document_begin_edit (doc);
	
	cong_location_copy (&cursor->location,
			    &PRIVATE(modification_cursor_change)->new_location);

	cong_document_private_on_cursor_change (doc);
	
	cong_document_end_edit (doc);
}

