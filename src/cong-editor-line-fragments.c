/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-line-fragments.c
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
#include "cong-editor-line-fragments.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorLineFragmentsDetails
{
	GList *list_of_areas;
};

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorLineFragments, 
			cong_editor_line_fragments,
			GObject,
			G_TYPE_OBJECT );

static void
cong_editor_line_fragments_class_init (CongEditorLineFragmentsClass *klass)
{
}

static void
cong_editor_line_fragments_instance_init (CongEditorLineFragments *line_fragments)
{
	line_fragments->private = g_new0(CongEditorLineFragmentsDetails,1);
}

CongEditorLineFragments*
cong_editor_line_fragments_construct (CongEditorLineFragments *line_fragments)
{
	return line_fragments;
}

/**
 * cong_editor_line_fragments_new:
 *
 * Returns: a new #CongEditorLineFragments
 */
CongEditorLineFragments*
cong_editor_line_fragments_new (void)
{
	return cong_editor_line_fragments_construct
		(g_object_new (CONG_EDITOR_LINE_FRAGMENTS_TYPE, NULL));
}

/**
 * cong_editor_line_fragments_get_area_list:
 * @line_fragments:
 *
 * TODO: Write me
 * Returns:
 */
GList*
cong_editor_line_fragments_get_area_list (CongEditorLineFragments *line_fragments)
{
	return PRIVATE(line_fragments)->list_of_areas;
}

/**
 * cong_editor_line_fragments_add_area:
 * @line_fragments:
 * @area:
 *
 * TODO: Write me
 */
void
cong_editor_line_fragments_add_area (CongEditorLineFragments *line_fragments,
				     CongEditorArea *area)
{
	PRIVATE(line_fragments)->list_of_areas = g_list_append (PRIVATE(line_fragments)->list_of_areas,
								area);
}
