/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-span-text-editor.c
 *
 * Copyright (C) 2002 David Malcolm
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
#include "cong-editor-widget-impl.h"

struct CongSpanTextEditor
{
	CongElementEditor element_editor;

};

static void span_text_editor_get_size_requisition(CongElementEditor *element_editor);
static void span_text_editor_allocate_child_space(CongElementEditor *element_editor);
static void span_text_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect);
static void span_text_editor_on_button_press(CongElementEditor *element_editor, GdkEventButton *event);

static CongElementEditorClass span_text_editor_class =
{
	"span_text_editor",
	span_text_editor_get_size_requisition,
	span_text_editor_allocate_child_space,
	span_text_editor_recursive_render,
	span_text_editor_on_button_press
};

static void span_text_editor_get_size_requisition(CongElementEditor *element_editor)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	CongSpanTextEditor *span_text = CONG_SPAN_TEXT_EDITOR(element_editor);

	element_editor->requisition.width=100;
	element_editor->requisition.height=70;
}

static void span_text_editor_allocate_child_space(CongElementEditor *element_editor)
{
	/* FIXME: unimplemented */
}

static void span_text_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	CongSpanTextEditor *span_text = CONG_SPAN_TEXT_EDITOR(element_editor);
	GtkWidget *w = GTK_WIDGET(editor_widget);

	GdkRectangle intersected_area;

	g_return_if_fail(editor_widget);
	g_return_if_fail(window_rect);

	/* Early accept/reject against the areas: */
	if (gdk_rectangle_intersect((GdkRectangle*)window_rect,
				     &element_editor->window_area,
				     &intersected_area)) {

		/* FIXME: unimplemented */

		gdk_draw_rectangle(GDK_DRAWABLE(w->window),
				   w->style->black_gc,
				   FALSE, /* gint filled, */
				   intersected_area.x,
				   intersected_area.y,
				   intersected_area.width-1,
				   intersected_area.height-1);
	}

}

static void span_text_editor_on_button_press(CongElementEditor *element_editor, GdkEventButton *event)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	CongSpanTextEditor *span_text = CONG_SPAN_TEXT_EDITOR(element_editor);

	/* FIXME: unimplemented */
}

/* Public API: */
CongElementEditor *cong_span_text_editor_new(CongEditorWidget *widget, CongNodePtr node)
{
	CongSpanTextEditor *span_text;

	g_return_val_if_fail(widget, NULL);
	g_return_val_if_fail(node, NULL);

	g_message("cong_span_text_editor_new");

	span_text = g_new0(CongSpanTextEditor,1);
	span_text->element_editor.klass = &span_text_editor_class;
	span_text->element_editor.widget = widget;
	span_text->element_editor.node = node;

	/* recursive creation? */
#if 0
	recursively_create_children(span_text);
#endif

	return CONG_ELEMENT_EDITOR(span_text);
}

