/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-flow-holder-line.h
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

#ifndef __CONG_EDITOR_AREA_LINE_H__
#define __CONG_EDITOR_AREA_LINE_H__

#include "cong-editor-area-container.h"

G_BEGIN_DECLS

typedef struct CongEditorAreaLineDetails CongEditorAreaLineDetails;

#define CONG_EDITOR_AREA_LINE_TYPE	   (cong_editor_area_line_get_type ())
#define CONG_EDITOR_AREA_LINE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_LINE_TYPE, CongEditorAreaLine)
#define CONG_EDITOR_AREA_LINE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_LINE_TYPE, CongEditorAreaLineClass)
#define IS_CONG_EDITOR_AREA_LINE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_LINE_TYPE)

struct CongEditorAreaLine
{
	CongEditorAreaContainer area;
	
	CongEditorAreaLineDetails *private;
};

struct CongEditorAreaLineClass
{
	CongEditorAreaContainerClass klass;
};

GType
cong_editor_area_line_get_type (void);

CongEditorArea*
cong_editor_area_line_construct (CongEditorAreaLine *area_line,
				 CongEditorWidget3 *editor_widget,
				 gint width_limit);

CongEditorArea*
cong_editor_area_line_new (CongEditorWidget3 *editor_widget,
			   gint width_limit);

gint
cong_editor_area_line_get_width_limit (CongEditorAreaLine *area_line);

gint
cong_editor_area_line_get_width_used (CongEditorAreaLine *area_line);

/**
 * cong_editor_area_line_get_width_used_up_to:
 *
 * @area_line: the line of interest
 * @child_area: final child to consider width for. Can be NULL (i.e. consider none of the children)
 * 
 * Calculate the width used by a subset of the child areas, from the first child up to and 
 * including @child_area.
 *
 * Returns: width in pixels
*/
gint
cong_editor_area_line_get_width_used_up_to (CongEditorAreaLine *area_line,
					    CongEditorArea *child_area);					    

gint
cong_editor_area_line_get_width_free (CongEditorAreaLine *area_line);

G_END_DECLS

#endif
