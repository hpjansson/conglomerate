/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-modification-set-dtd-ptr.c
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
#include "cong-modification-set-dtd-ptr.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongModificationSetDtdPtrDetails
{
	/* Undo info: */
	xmlDtdPtr old_dtd_ptr;

	/* Redo info: */
	xmlDtdPtr new_dtd_ptr;
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
GNOME_CLASS_BOILERPLATE(CongModificationSetDtdPtr, 
			cong_modification_set_dtd_ptr,
			CongModification,
			CONG_MODIFICATION_TYPE );

static void
cong_modification_set_dtd_ptr_class_init (CongModificationSetDtdPtrClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	CONG_MODIFICATION_CLASS (klass)->undo = undo;
	CONG_MODIFICATION_CLASS (klass)->redo = redo;
}

static void
cong_modification_set_dtd_ptr_instance_init (CongModificationSetDtdPtr *node)
{
	node->private = g_new0(CongModificationSetDtdPtrDetails,1);
}

/**
 * cong_modification_set_dtd_ptr_construct:
 * @modification_set_dtd_ptr:
 * @doc:
 * @dtd_ptr:
 *
 * TODO: Write me
 * Returns:
 */
CongModificationSetDtdPtr*
cong_modification_set_dtd_ptr_construct (CongModificationSetDtdPtr *modification_set_dtd_ptr,
					 CongDocument *doc,
					 xmlDtdPtr dtd_ptr)
{
	cong_modification_construct (CONG_MODIFICATION(modification_set_dtd_ptr),
				     doc);

	cong_document_set_with_ref (doc,
				    (CongNodePtr*)&PRIVATE(modification_set_dtd_ptr)->old_dtd_ptr,
				    (CongNodePtr)cong_document_get_xml (doc)->extSubset);
	cong_document_set_with_ref (doc,
				    (CongNodePtr*)&PRIVATE(modification_set_dtd_ptr)->new_dtd_ptr,
				    (CongNodePtr)dtd_ptr);

	return modification_set_dtd_ptr;
}

/**
 * cong_modification_set_dtd_ptr_new:
 * @doc:
 * @dtd_ptr:
 *
 * TODO: Write me
 * Returns:
 */
CongModification*
cong_modification_set_dtd_ptr_new (CongDocument *doc,
					 xmlDtdPtr dtd_ptr)
{
	return CONG_MODIFICATION(cong_modification_set_dtd_ptr_construct (CONG_MODIFICATION_SET_DTD_PTR(g_object_new (CONG_MODIFICATION_SET_DTD_PTR_TYPE, NULL)),
									  doc,
									  dtd_ptr));
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
	CongModificationSetDtdPtr *modification_set_dtd_ptr = CONG_MODIFICATION_SET_DTD_PTR (object);

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongModificationSetDtdPtr::finalize");
#endif

	g_free (modification_set_dtd_ptr->private);
	modification_set_dtd_ptr->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);

}

static void
dispose (GObject *object)
{
	CongModificationSetDtdPtr *modification_set_dtd_ptr = CONG_MODIFICATION_SET_DTD_PTR (object);
	CongDocument *doc = cong_modification_get_document (CONG_MODIFICATION (object));

#if DEBUG_MODIFICATION_LIFETIMES
	g_message ("CongModificationSetDtdPtr::dispose");
#endif

	g_assert (modification_set_dtd_ptr->private);
	
	/* Cleanup: */
	cong_document_set_with_ref (doc,
				    (CongNodePtr*)&PRIVATE(modification_set_dtd_ptr)->old_dtd_ptr,
				    NULL);
	cong_document_set_with_ref (doc,
				    (CongNodePtr*)&PRIVATE(modification_set_dtd_ptr)->new_dtd_ptr,
				    NULL);

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
undo (CongModification *modification)
{
	CongModificationSetDtdPtr *modification_set_dtd_ptr = CONG_MODIFICATION_SET_DTD_PTR (modification);
	CongDocument *doc = cong_modification_get_document (modification);

	cong_document_begin_edit (doc);
	
	cong_document_private_set_dtd_ptr (doc,
					   PRIVATE(modification_set_dtd_ptr)->old_dtd_ptr);
	
	cong_document_end_edit (doc);
}

static void
redo (CongModification *modification)
{
	CongModificationSetDtdPtr *modification_set_dtd_ptr = CONG_MODIFICATION_SET_DTD_PTR (modification);
	CongDocument *doc = cong_modification_get_document (modification);

	cong_document_begin_edit (doc);
	
	cong_document_private_set_dtd_ptr (doc,
					   PRIVATE(modification_set_dtd_ptr)->new_dtd_ptr);
	
	cong_document_end_edit (doc);
}

