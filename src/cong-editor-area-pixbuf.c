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

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaPixbufDetails
{
	GdkPixbuf *pixbuf;
};

/* Method implementation prototypes: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect);

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
}

static void
cong_editor_area_pixbuf_instance_init (CongEditorAreaPixbuf *area_pixbuf)
{
	area_pixbuf->private = g_new0(CongEditorAreaPixbufDetails,1);
}

/* Exported function definitions: */
CongEditorArea*
cong_editor_area_pixbuf_construct (CongEditorAreaPixbuf *area_pixbuf,
				   CongEditorWidget3 *editor_widget,
				   GdkPixbuf *pixbuf)
{
	g_return_val_if_fail (pixbuf, NULL);

	cong_editor_area_construct (CONG_EDITOR_AREA(area_pixbuf),
				    editor_widget);

	PRIVATE(area_pixbuf)->pixbuf = pixbuf;
	g_object_ref( G_OBJECT(pixbuf));

	return CONG_EDITOR_AREA (area_pixbuf);
}

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

	g_message("unimplemented (area_pixbuf->render_self)");
}
