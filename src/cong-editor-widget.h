/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-widget.h
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

#ifndef __CONG_EDITOR_WIDGET_H__
#define __CONG_EDITOR_WIDGET_H__

G_BEGIN_DECLS

/* Third attempt at an editor widget: */
typedef GtkDrawingArea CongEditorWidget3;
#define CONG_EDITOR_WIDGET3(x) ((CongEditorWidget3*)(x))

GtkWidget*
cong_editor_widget3_new(CongDocument *doc);

CongDocument*
cong_editor_widget3_get_document(CongEditorWidget3 *editor_widget);

CongDispspec*
cong_editor_widget3_get_dispspec(CongEditorWidget3 *editor_widget);

void 
cong_editor_widget3_force_layout_update (CongEditorWidget3 *editor_widget);

GdkGC*
cong_editor_widget3_get_test_gc (CongEditorWidget3 *editor_widget);


/* Get the "deepest" area in the tree at the given coordinate (if any) 
 * Returns NULL if no area present.
 */
CongEditorArea*
cong_editor_widget3_get_area_at (CongEditorWidget3 *editor_widget,
				 gint x,
				 gint y);

G_END_DECLS

#endif
