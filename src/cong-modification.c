/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-modification.c
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
#include "cong-modification.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "cong-app.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-modification.h"

#define PRIVATE(x) ((x)->private)

struct CongModificationDetails
{
	CongDocument *doc;
};

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongModification, 
			cong_modification,
			GObject,
			G_TYPE_OBJECT );

static void
cong_modification_class_init (CongModificationClass *klass)
{
}

static void
cong_modification_instance_init (CongModification *node)
{
	node->private = g_new0(CongModificationDetails,1);
}

CongModification*
cong_modification_construct (CongModification *modification,
			     CongDocument *doc)
{
	PRIVATE(modification)->doc = doc;

	return modification;
}

/**
 * cong_modification_get_document:
 * @modification:
 * 
 * TODO: Write me
 * Returns:
 */
CongDocument*
cong_modification_get_document (CongModification *modification)
{
	g_return_val_if_fail (IS_CONG_MODIFICATION(modification), NULL);
	
	return PRIVATE(modification)->doc;
}

/**
 * cong_modification_undo:
 * @modification:
 * 
 * TODO: Write me
 */
void
cong_modification_undo (CongModification *modification)
{
	g_return_if_fail (IS_CONG_MODIFICATION(modification));

	g_message ("cong_modification_undo()");
}

/**
 * cong_modification_redo:
 * @modification:
 * 
 * TODO: Write me
 */
void
cong_modification_redo (CongModification *modification)
{
	g_return_if_fail (IS_CONG_MODIFICATION(modification));

	g_message ("cong_modification_redo()");
}

