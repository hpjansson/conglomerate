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
#include "cong-document.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaTextFragmentDetails
{
	CongEditorNodeText *editor_node_text;
	PangoLayout *pango_layout;
	guint line_index;
	int baseline;
};

/* Method implementation prototypes: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect);

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint);

static void
allocate_child_space (CongEditorArea *area);

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
			CongEditorArea,
			CONG_EDITOR_AREA_TYPE );

static void
cong_editor_area_text_fragment_class_init (CongEditorAreaTextFragmentClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);

	area_klass->render_self = render_self;
	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
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
					  CongEditorNodeText *editor_node_text,
					  PangoLayout *pango_layout,
					  guint line_index,
					  int baseline)
{
	cong_editor_area_construct (CONG_EDITOR_AREA(area_text_fragment),
				    editor_widget);

	PRIVATE(area_text_fragment)->editor_node_text = editor_node_text;
	PRIVATE(area_text_fragment)->pango_layout = pango_layout;
	PRIVATE(area_text_fragment)->line_index = line_index;
	PRIVATE(area_text_fragment)->baseline = baseline;

	return CONG_EDITOR_AREA (area_text_fragment);
}

CongEditorArea*
cong_editor_area_text_fragment_new (CongEditorWidget3 *editor_widget,
				    CongEditorNodeText *editor_node_text,
				    PangoLayout *pango_layout,
				    guint line_index,
				    int baseline)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_text_fragment_new(%i, %i)", line_index, baseline);
#endif

	return cong_editor_area_text_fragment_construct
		(g_object_new (CONG_EDITOR_AREA_TEXT_FRAGMENT_TYPE, NULL),
		 editor_widget,
		 editor_node_text,
		 pango_layout,
		 line_index,
		 baseline);
}

PangoLayoutLine*
cong_editor_area_text_get_pango_layout_line (CongEditorAreaTextFragment *area_text_fragment)
{
	PangoLayoutLine* line;

	g_return_val_if_fail (IS_CONG_EDITOR_AREA_TEXT_FRAGMENT(area_text_fragment), NULL);

	line = pango_layout_get_line (PRIVATE(area_text_fragment)->pango_layout,
						       PRIVATE(area_text_fragment)->line_index);

	return line;
}

gboolean
cong_editor_area_text_fragment_x_to_index (CongEditorAreaTextFragment *area_text_fragment,
					   int x,
					   int *index_,
					   int *trailing)
{
	const GdkRectangle *rect;
	PangoLayoutLine* line;

	g_return_val_if_fail (IS_CONG_EDITOR_AREA_TEXT_FRAGMENT(area_text_fragment), FALSE);
	g_return_val_if_fail (index_, FALSE);
	g_return_val_if_fail (trailing, FALSE);


	rect = cong_editor_area_get_window_coords (CONG_EDITOR_AREA(area_text_fragment));
	line = cong_editor_area_text_get_pango_layout_line (area_text_fragment);

	return pango_layout_line_x_to_index (line,
					     (x - rect->x) * PANGO_SCALE,
					     index_,
					     trailing);
}

#if 0
gint
cong_editor_area_text_fragment_get_text_offset (CongEditorAreaTextFragment *area_text_fragment)
{
	return PRIVATE(area_text_fragment)->text_offset;
}
#endif

/* Method implementation definitions: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect)
{
	CongEditorAreaTextFragment *area_text_fragment = CONG_EDITOR_AREA_TEXT_FRAGMENT(area);
	PangoLayoutLine* line = cong_editor_area_text_get_pango_layout_line (area_text_fragment);
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);
	CongCursor *cursor = cong_document_get_cursor( cong_editor_widget3_get_document( cong_editor_area_get_widget(area)));
	g_assert(cursor);

	/* y-coord is that of the baseline, not the top or bottom; we must adjust it: */
	gdk_draw_layout_line (window, 
			      cong_editor_widget3_get_test_gc (cong_editor_area_get_widget (area)),			      
			      rect->x,
			      rect->y + PRIVATE(area_text_fragment)->baseline,
			      line);

	/* Render the cursor if present in this node: */
	if (cursor->on) {

		if (cursor->location.node==cong_editor_node_get_node (CONG_EDITOR_NODE(PRIVATE(area_text_fragment)->editor_node_text))) {
			int cursor_stripped_offset;
			
			if (cong_editor_node_text_convert_original_byte_offset_to_stripped (PRIVATE(area_text_fragment)->editor_node_text,
											    cursor->location.byte_offset,
											    &cursor_stripped_offset)) {
				if (cursor_stripped_offset >= line->start_index) {
					if (cursor_stripped_offset < line->start_index+line->length) {
						int cursor_x;
						
						pango_layout_line_index_to_x(line,
									     cursor_stripped_offset,
									     FALSE,
									     &cursor_x);	
						cursor_x /= PANGO_SCALE;
						cursor_x += rect->x;
						
						/* Render it: */
						gdk_draw_line (GDK_DRAWABLE(window), 
							       cursor->gc, 
							       cursor_x, rect->y,
							       cursor_x, rect->y + rect->height);
						
					}
				}
			}
		}
	}

}

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	CongEditorAreaTextFragment *area_text_fragment = CONG_EDITOR_AREA_TEXT_FRAGMENT(area);
	PangoLayoutLine *line;
	PangoRectangle ink_rect;
	PangoRectangle logical_rect;
	
	line = cong_editor_area_text_get_pango_layout_line (area_text_fragment);

	if (line==NULL) {
		g_warning("Looked up a non-existant PangoLayoutLine");
		return 0; /* workaround for now; possible problems with entity refs/entity decls? */
	}

	pango_layout_line_get_pixel_extents (line,
					     &ink_rect,
					     &logical_rect);

	if (orientation == GTK_ORIENTATION_HORIZONTAL) {
		return logical_rect.width;
	} else {
		return logical_rect.height;
	}
}

static void
allocate_child_space (CongEditorArea *area)
{
	/* empty */
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