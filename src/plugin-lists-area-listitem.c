/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-lists-area-listitem.c
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
#include "plugin-lists-area-listitem.h"
#include <libgnome/gnome-macros.h>

#include "cong-eel.h"
#include "cong-app.h"
#include "cong-editor-area-text.h"
#include "cong-editor-area-labelled.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaListitemDetails
{
	CongEditorAreaText *area_label;
};

/* Method implementation prototypes: */
/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaListitem, 
			cong_editor_area_listitem,
			CongEditorAreaLabelled,
			CONG_EDITOR_AREA_LABELLED_TYPE );

static void
cong_editor_area_listitem_class_init (CongEditorAreaListitemClass *klass)
{
}

static void
cong_editor_area_listitem_instance_init (CongEditorAreaListitem *area_listitem)
{
	area_listitem->private = g_new0(CongEditorAreaListitemDetails,1);
}


/* Exported function definitions: */
/**
 * cong_editor_area_listitem_construct:
 * @area_listitem:
 * @editor_widget:
 * @label:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_listitem_construct (CongEditorAreaListitem *area_listitem,
				     CongEditorWidget3 *editor_widget,
				     const gchar *label)
{
	PRIVATE(area_listitem)->area_label = CONG_EDITOR_AREA_TEXT(cong_editor_area_text_new (editor_widget,
											      cong_app_get_font (cong_app_singleton(),
														 CONG_FONT_ROLE_TITLE_TEXT), 
											      NULL,
											      label,
											      FALSE));
	cong_editor_area_labelled_construct (CONG_EDITOR_AREA_LABELLED(area_listitem),
					     editor_widget,
					     CONG_EDITOR_AREA(PRIVATE(area_listitem)->area_label));

	return CONG_EDITOR_AREA (area_listitem);
}

/**
 * cong_editor_area_listitem_new:
 * @editor_widget:
 * @label:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_listitem_new (CongEditorWidget3 *editor_widget,
			       const gchar *label)

{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_listitem_new(%s)", label);
#endif

	return cong_editor_area_listitem_construct
		(g_object_new (CONG_EDITOR_AREA_LISTITEM_TYPE, NULL),
		 editor_widget,
		 label);
}

/**
 * cong_editor_area_listitem_set_label:
 * @area_listitem:
 * @label:
 *
 * TODO: Write me
 */
void
cong_editor_area_listitem_set_label (CongEditorAreaListitem *area_listitem,
				     const gchar *label)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA_LISTITEM (area_listitem));
	g_return_if_fail (label);

	cong_editor_area_text_set_text (PRIVATE(area_listitem)->area_label,
					label);
}

