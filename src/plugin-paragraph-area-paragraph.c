/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-paragraph-area-paragraph.c
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
#include "plugin-paragraph-area-paragraph.h"
#include <libgnome/gnome-macros.h>

#include "cong-eel.h"
#include "cong-app.h"
#include "cong-editor-area-text.h"
#include "cong-editor-area-composer.h"
#include "cong-editor-area-bin.h"

#include "cong-ui-hooks.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaParagraphDetails
{
	GdkGC *gc;
};

/* Method implementation prototypes: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaParagraph, 
			cong_editor_area_paragraph,
			CongEditorAreaBorder,
			CONG_EDITOR_AREA_BORDER_TYPE );

static void
cong_editor_area_paragraph_class_init (CongEditorAreaParagraphClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);

	area_klass->render_self = render_self;

}

static void
cong_editor_area_paragraph_instance_init (CongEditorAreaParagraph *area_paragraph)
{
	area_paragraph->private = g_new0(CongEditorAreaParagraphDetails,1);

	PRIVATE(area_paragraph)->gc = gdk_gc_new(cong_gui_get_a_window()->window);

	gdk_gc_set_line_attributes (PRIVATE(area_paragraph)->gc,
				    1,
				    GDK_LINE_ON_OFF_DASH,
				    GDK_CAP_NOT_LAST,
				    GDK_JOIN_MITER);

#if 0
	gdk_gc_set_dashes (PRIVATE(area_paragraph)->gc,
			   0,
			   gint8 dash_list[],
			   gint n);
#endif
}


/* Exported function definitions: */
/**
 * cong_editor_area_paragraph_construct:
 * @area_paragraph:
 * @editor_widget:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_paragraph_construct (CongEditorAreaParagraph *area_paragraph,
				      CongEditorWidget3 *editor_widget)
{
	cong_editor_area_border_construct (CONG_EDITOR_AREA_BORDER(area_paragraph),
					   editor_widget,
					   5,
					   5,
					   5,
					   5);

	return CONG_EDITOR_AREA (area_paragraph);
}

/**
 * cong_editor_area_paragraph_new:
 * @editor_widget:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_paragraph_new (CongEditorWidget3 *editor_widget)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_paragraph_new");
#endif

	return cong_editor_area_paragraph_construct
		(g_object_new (CONG_EDITOR_AREA_PARAGRAPH_TYPE, NULL),
		 editor_widget);
}

/* Method implementation definitions: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect)
{
	CongEditorAreaParagraph *area_paragraph = CONG_EDITOR_AREA_PARAGRAPH(area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);

	gdk_draw_rectangle (GDK_DRAWABLE(window),
			    PRIVATE(area_paragraph)->gc,
			    FALSE,
			    rect->x+2,
			    rect->y+2,
			    rect->width-3,
			    rect->height-3);

	cong_editor_area_debug_render_state (area);
}
