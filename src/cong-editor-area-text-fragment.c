/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-text-fragment.c
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
#include "cong-editor-area-text-fragment.h"
#include <libgnome/gnome-macros.h>
#include "cong-font.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaTextFragmentDetails
{
	gint width;
	gint height;
	gint text_offset;
};

/* Method implementation prototypes: */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint);

#if 0
static gboolean 
get_location_at_xy (CongEditorArea *editor_area, 
		    gint x,
		    gint y,
		    CongLocation *output);
#endif

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaTextFragment, 
			cong_editor_area_text_fragment,
			CongEditorAreaText,
			CONG_EDITOR_AREA_TEXT_TYPE );

static void
cong_editor_area_text_fragment_class_init (CongEditorAreaTextFragmentClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);

	area_klass->calc_requisition = calc_requisition;
#if 0
	area_klass->get_location_at_xy = get_location_at_xy;
#endif
}

static void
cong_editor_area_text_fragment_instance_init (CongEditorAreaTextFragment *area_text_fragment)
{
	area_text_fragment->private = g_new0(CongEditorAreaTextFragmentDetails,1);
}

/* Exported function definitions: */
CongEditorArea*
cong_editor_area_text_fragment_construct (CongEditorAreaTextFragment *area_text_fragment,
					  CongEditorWidget3 *editor_widget,
					  CongFont *font,
					  const GdkColor *fg_col,
					  const gchar *text,
					  gboolean use_markup,
					  gint width,
					  gint height,
					  gint text_offset)
{
	cong_editor_area_text_construct (CONG_EDITOR_AREA_TEXT(area_text_fragment),
					 editor_widget,
					 font,
					 fg_col,
					 text,
					 use_markup);

	PRIVATE(area_text_fragment)->width = width;
	PRIVATE(area_text_fragment)->height = height;
	PRIVATE(area_text_fragment)->text_offset = text_offset;

	return CONG_EDITOR_AREA (area_text_fragment);
}

CongEditorArea*
cong_editor_area_text_fragment_new (CongEditorWidget3 *editor_widget,
				    CongFont *font,
				    const GdkColor *fg_col,
				    const gchar *text,
				    gboolean use_markup,
				    gint width,
				    gint height,
				    gint text_offset)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_text_fragment_new(%s)", text);
#endif

	return cong_editor_area_text_fragment_construct
		(g_object_new (CONG_EDITOR_AREA_TEXT_FRAGMENT_TYPE, NULL),
		 editor_widget,
		 font,
		 fg_col,
		 text,
		 use_markup,
		 width,
		 height,
		 text_offset);
}

gint
cong_editor_area_text_fragment_get_text_offset (CongEditorAreaTextFragment *area_text_fragment)
{
	return PRIVATE(area_text_fragment)->text_offset;
}

/* Method implementation definitions: */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	CongEditorAreaTextFragment *area_text_fragment = CONG_EDITOR_AREA_TEXT_FRAGMENT(area);

	if (orientation == GTK_ORIENTATION_HORIZONTAL) {
		return PRIVATE(area_text_fragment)->width;
	} else {
		return PRIVATE(area_text_fragment)->height;
	}
}

#if 0
static gboolean 
get_location_at_xy (CongEditorArea *editor_area, 
		    gint x,
		    gint y,
		    CongLocation *output)
{
	g_assert_not_reached();
}
#endif
