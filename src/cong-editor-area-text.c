/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-text.c
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
#include "cong-editor-area-text.h"
#include <libgnome/gnome-macros.h>
#include "cong-font.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaTextDetails
{
	gchar *text;

	GdkGC *gc;
	PangoLayout *pango_layout;
	CongFont *font;
};

/* Method implementation prototypes: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect);

static void 
update_requisition (CongEditorArea *area, 
		    int width_hint);


/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaText, 
			cong_editor_area_text,
			CongEditorArea,
			CONG_EDITOR_AREA_TYPE );

static void
cong_editor_area_text_class_init (CongEditorAreaTextClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);

	area_klass->render_self = render_self;
	area_klass->update_requisition = update_requisition;
}

static void
cong_editor_area_text_instance_init (CongEditorAreaText *area_text)
{
	area_text->private = g_new0(CongEditorAreaTextDetails,1);
}

/* Exported function definitions: */
CongEditorArea*
cong_editor_area_text_construct (CongEditorAreaText *area_text,
				 CongEditorWidget3 *editor_widget,
				 CongFont *font,
				 const gchar *text)
{
	g_return_val_if_fail (text, NULL);

	cong_editor_area_construct (CONG_EDITOR_AREA(area_text),
				    editor_widget);

	PRIVATE(area_text)->text = g_strdup(text);

	PRIVATE(area_text)->gc = gdk_gc_new(cong_gui_get_a_window()->window);

	PRIVATE(area_text)->pango_layout = pango_layout_new(gdk_pango_context_get());

	pango_layout_set_text (PRIVATE(area_text)->pango_layout,
			       text,
			       -1);
	/* in theory should call pango_layout_set_text instead, though this causes errors with colormaps */

	PRIVATE(area_text)->font = font;
	g_assert(PRIVATE(area_text)->font);

	pango_layout_set_font_description(PRIVATE(area_text)->pango_layout,
					  cong_font_get_pango_description(PRIVATE(area_text)->font));


	return CONG_EDITOR_AREA (area_text);
}

CongEditorArea*
cong_editor_area_text_new (CongEditorWidget3 *editor_widget,
			   CongFont *font,
			   const gchar *text)
{
	g_message("cong_editor_area_text_new(\"%s\")", text);

	return cong_editor_area_text_construct
		(g_object_new (CONG_EDITOR_AREA_TEXT_TYPE, NULL),
		 editor_widget,
		 font,
		 text);
}

/* Method implementation definitions: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect)
{
	CongEditorAreaText *area_text = CONG_EDITOR_AREA_TEXT(area);
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);

	pango_layout_set_width (PRIVATE(area_text)->pango_layout,
				rect->width * PANGO_SCALE);

	cong_editor_area_debug_render_area (area,
					    PRIVATE(area_text)->gc);

	gdk_draw_layout (window, 
			 PRIVATE(area_text)->gc,
			 rect->x, 
			 rect->y,
			 PRIVATE(area_text)->pango_layout);
}

static void 
update_requisition (CongEditorArea *area, 
		    int width_hint)
{
#if 1
	int width, height;

	CongEditorAreaText *area_text = CONG_EDITOR_AREA_TEXT(area);

	pango_layout_set_width (PRIVATE(area_text)->pango_layout,
				width_hint*PANGO_SCALE);
	
	pango_layout_get_pixel_size (PRIVATE(area_text)->pango_layout,
				     &width,
				     &height);

	cong_editor_area_set_requisition (area,
					  width,
					  height);
#else
	/* For now: */
	cong_editor_area_set_requisition (area,
					  100,
					  50);
#endif
	
}
