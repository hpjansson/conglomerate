/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-spacer.c
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
#include "cong-editor-area-spacer.h"
#include <libgnome/gnome-macros.h>

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaSpacerDetails
{
	GtkOrientation orientation;
	guint spacing;

};

/* Method implementation prototypes: */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaSpacer, 
			cong_editor_area_spacer,
			CongEditorArea,
			CONG_EDITOR_AREA_TYPE );

static void
cong_editor_area_spacer_class_init (CongEditorAreaSpacerClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);
	
	area_klass->calc_requisition = calc_requisition;
}

static void
cong_editor_area_spacer_instance_init (CongEditorAreaSpacer *area_spacer)
{
	area_spacer->private = g_new0(CongEditorAreaSpacerDetails,1);
}

/* Exported function definitions: */
CongEditorArea*
cong_editor_area_spacer_construct (CongEditorAreaSpacer *area_spacer,
				   CongEditorWidget3 *editor_widget,
				   GtkOrientation orientation,
				   guint spacing)
{
	cong_editor_area_construct (CONG_EDITOR_AREA(area_spacer),
				    editor_widget);

	PRIVATE(area_spacer)->orientation = orientation;
	PRIVATE(area_spacer)->spacing = spacing;

	return CONG_EDITOR_AREA (area_spacer);
}

/**
 * cong_editor_area_spacer_new:
 * @editor_widget:
 * @orientation:
 * @spacing:
 *
 * Returns: the new #CongEditorArea
 */
CongEditorArea*
cong_editor_area_spacer_new (CongEditorWidget3 *editor_widget,
			     GtkOrientation orientation,
			     guint spacing)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_spacer_new");
#endif

	return cong_editor_area_spacer_construct
		(g_object_new (CONG_EDITOR_AREA_SPACER_TYPE, NULL),
		 editor_widget,
		 orientation,
		 spacing);
}

/* Method implementation definitions: */
/**
 * calc_requisition:
 * @area:
 * @orientation:
 * @width_hint:
 *
 * TODO: Write me
 * Returns:
 */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	CongEditorAreaSpacer *area_spacer = CONG_EDITOR_AREA_SPACER(area);

	if (PRIVATE(area_spacer)->orientation == orientation) {
		return PRIVATE(area_spacer)->spacing;
	} else {
		return 0;
	}
}
