/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-border.c
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
#include "cong-editor-area-border.h"
#include <libgnome/gnome-macros.h>

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaBorderDetails
{
	guint pixels;
};

/* Method implementation prototypes: */

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaBorder, 
			cong_editor_area_border,
			CongEditorArea,
			CONG_EDITOR_AREA_TYPE );

static void
cong_editor_area_border_class_init (CongEditorAreaBorderClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);

}

static void
cong_editor_area_border_instance_init (CongEditorAreaBorder *area_border)
{
	area_border->private = g_new0(CongEditorAreaBorderDetails,1);
}

/* Exported function definitions: */
CongEditorArea*
cong_editor_area_border_construct (CongEditorAreaBorder *area_border,
				   CongEditorWidget3 *editor_widget,
				   guint pixels)     
{
	cong_editor_area_bin_construct (CONG_EDITOR_AREA_BIN(area_border),
					editor_widget);
	
	PRIVATE(area_border)->pixels = pixels;
	
	return CONG_EDITOR_AREA (area_border);
}

CongEditorArea*
cong_editor_area_border_new (CongEditorWidget3 *editor_widget,
			     guint pixels)
{
	return cong_editor_area_border_construct
		(g_object_new (CONG_EDITOR_AREA_BORDER_TYPE, NULL),
		 editor_widget,
		 pixels);
}

/* Method implementation definitions: */
