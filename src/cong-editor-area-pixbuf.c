/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-pixbuf.c
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
#include "cong-editor-area-pixbuf.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"
#include "cong-eel-graphic-effects.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaPixbufDetails
{
	GdkPixbuf *pixbuf_states[5];
	gint pixbuf_size[2];
};

/* Method implementation prototypes: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect);

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint);

GdkPixbuf*
get_pixbuf (CongEditorAreaPixbuf *area_pixbuf);

GdkPixbuf*
generate_pixbuf_for_state (GdkPixbuf *normal_pixbuf,
			   GtkStateType state);


/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaPixbuf, 
			cong_editor_area_pixbuf,
			CongEditorArea,
			CONG_EDITOR_AREA_TYPE );

static void
cong_editor_area_pixbuf_class_init (CongEditorAreaPixbufClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);

	area_klass->render_self = render_self;
	area_klass->calc_requisition = calc_requisition;
}

static void
cong_editor_area_pixbuf_instance_init (CongEditorAreaPixbuf *area_pixbuf)
{
	area_pixbuf->private = g_new0(CongEditorAreaPixbufDetails,1);
}

/* Exported function definitions: */
/**
 * cong_editor_area_pixbuf_construct:
 * @area_pixbuf:
 * @editor_widget:
 * @pixbuf:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_pixbuf_construct (CongEditorAreaPixbuf *area_pixbuf,
				   CongEditorWidget3 *editor_widget,
				   GdkPixbuf *pixbuf)
{
	g_return_val_if_fail (pixbuf, NULL);

	cong_editor_area_construct (CONG_EDITOR_AREA(area_pixbuf),
				    editor_widget);

	PRIVATE(area_pixbuf)->pixbuf_states [GTK_STATE_NORMAL] = pixbuf;
	g_object_ref( G_OBJECT(pixbuf));

	PRIVATE(area_pixbuf)->pixbuf_size[GTK_ORIENTATION_HORIZONTAL] = gdk_pixbuf_get_width(pixbuf);
	PRIVATE(area_pixbuf)->pixbuf_size[GTK_ORIENTATION_VERTICAL] = gdk_pixbuf_get_height(pixbuf);

	return CONG_EDITOR_AREA (area_pixbuf);
}

/**
 * cong_editor_area_pixbuf_new:
 * @editor_widget:
 * @pixbuf:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_pixbuf_new (CongEditorWidget3 *editor_widget,
			     GdkPixbuf *pixbuf)
{
	return cong_editor_area_pixbuf_construct
		(g_object_new (CONG_EDITOR_AREA_PIXBUF_TYPE, NULL),
		 editor_widget,
		 pixbuf);
}

/* Method implementation definitions: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect)
{
	CongEditorAreaPixbuf *area_pixbuf = CONG_EDITOR_AREA_PIXBUF(area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);
	GdkPixbuf *pixbuf = get_pixbuf (area_pixbuf);


	cong_eel_draw_pixbuf (window,
			      NULL,
			      pixbuf,
			      0,
			      0,
			      rect->x,
			      rect->y,
			      gdk_pixbuf_get_width(pixbuf),
			      gdk_pixbuf_get_height(pixbuf),
			      GDK_RGB_DITHER_NONE,
			      0,
			      0);
}

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	CongEditorAreaPixbuf *area_pixbuf = CONG_EDITOR_AREA_PIXBUF(area);

	return PRIVATE(area_pixbuf)->pixbuf_size[orientation];
}

/**
 * get_pixbuf:
 * @area_pixbuf:
 *
 * TODO: Write me
 * Returns:
 */
GdkPixbuf*
get_pixbuf (CongEditorAreaPixbuf *area_pixbuf)
{
	GtkStateType state = cong_editor_area_get_state (CONG_EDITOR_AREA (area_pixbuf));

	/* Lazily generate versions of the pixbuf: */
	if (NULL==PRIVATE(area_pixbuf)->pixbuf_states[state]) {
		PRIVATE(area_pixbuf)->pixbuf_states[state] = generate_pixbuf_for_state (PRIVATE(area_pixbuf)->pixbuf_states[GTK_STATE_NORMAL],
											state);
	}

	return PRIVATE(area_pixbuf)->pixbuf_states[state];
}

/**
 * generate_pixbuf_for_state:
 * @normal_pixbuf:
 * @state:
 *
 * TODO: Write me
 * Returns:
 */
GdkPixbuf*
generate_pixbuf_for_state (GdkPixbuf *normal_pixbuf,
			   GtkStateType state)
{
	g_return_val_if_fail (normal_pixbuf, NULL);

	switch (state) {
	default: g_assert_not_reached();

	case GTK_STATE_NORMAL:
		g_assert_not_reached();
	case GTK_STATE_INSENSITIVE:
		g_assert_not_reached();

	case GTK_STATE_ACTIVE:
	case GTK_STATE_SELECTED:
		/* Simply return the input (with an extra ref): */
		g_object_ref (G_OBJECT (normal_pixbuf));
		return normal_pixbuf;
		
	case GTK_STATE_PRELIGHT:
		return eel_create_spotlight_pixbuf (normal_pixbuf);
	}
}
