/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-lines.c
 *
 * Copyright (C) 2004 David Malcolm
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
#include "cong-editor-area-lines.h"

/* FIXME: this should probabky be done by interrogating the font, or something like that */
#define LINE_SPACING (5)

struct CongEditorAreaLinesPrivate
{
};

CONG_DEFINE_CLASS_BEGIN (CongEditorAreaLines, cong_editor_area_lines, CONG_EDITOR_AREA_LINES, CongEditorAreaComposer, CONG_EDITOR_AREA_COMPOSER_TYPE)
CONG_DEFINE_CLASS_END ()
CONG_DEFINE_EMPTY_DISPOSE(cong_editor_area_lines)


/* Implementation of CongEditorAreaLines: */
CongEditorAreaLines*
cong_editor_area_lines_construct (CongEditorAreaLines* editor_area_lines,
				  CongEditorWidget3 *editor_widget)
{
	g_return_val_if_fail (IS_CONG_EDITOR_AREA_LINES (editor_area_lines), NULL);

	cong_editor_area_composer_construct (CONG_EDITOR_AREA_COMPOSER (editor_area_lines),
					     editor_widget,
					     GTK_ORIENTATION_VERTICAL,
					     LINE_SPACING);
	return editor_area_lines;
}

CongEditorArea*
cong_editor_area_lines_new (CongEditorWidget3 *widget)
{
	return CONG_EDITOR_AREA (cong_editor_area_lines_construct (g_object_new (CONG_EDITOR_AREA_LINES_TYPE, NULL),
								   widget));
}
