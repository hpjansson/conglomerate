/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-container.c
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
#include "cong-editor-area-container.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaContainerDetails
{
	int dummy;
};

/* Method implementation prototypes: */
CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_area_container, add_child);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaContainer, 
			cong_editor_area_container,
			CongEditorArea,
			CONG_EDITOR_AREA_TYPE );

static void
cong_editor_area_container_class_init (CongEditorAreaContainerClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);

	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_area_container,
					      add_child);
}

static void
cong_editor_area_container_instance_init (CongEditorAreaContainer *area_container)
{
	area_container->private = g_new0(CongEditorAreaContainerDetails,1);
}

/* Exported function definitions: */
CongEditorArea*
cong_editor_area_container_construct (CongEditorAreaContainer *area_container,
				      CongEditorWidget3 *editor_widget)
{
	cong_editor_area_construct (CONG_EDITOR_AREA(area_container),
				    editor_widget);

	return CONG_EDITOR_AREA (area_container);
}

void
cong_editor_area_container_add_child ( CongEditorAreaContainer *area_container,
				       CongEditorArea *child)
{
	g_return_if_fail (area_container);
	g_return_if_fail (child);

#if 0
	g_return_if_fail (NULL!= cong_editor_area_get_parent (child));
#endif

	CONG_EEL_CALL_METHOD (CONG_EDITOR_AREA_CONTAINER_CLASS,
			      area_container,
			      add_child, 
			      (area_container, child));
}

/* Method implementation definitions: */
