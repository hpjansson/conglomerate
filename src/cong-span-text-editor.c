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

static void span_text_editor_on_recursive_delete(CongElementEditor *element_editor);
static gboolean span_text_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event);
static void span_text_editor_get_size_requisition(CongElementEditor *element_editor);
static void span_text_editor_allocate_child_space(CongElementEditor *element_editor);
static void span_text_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect);
static void span_text_editor_on_button_press(CongElementEditor *element_editor, GdkEventButton *event);

static CongElementEditorClass span_text_editor_class =
{
	"span_text_editor",
	span_text_editor_on_recursive_delete,
	span_text_editor_on_document_event,
	span_text_editor_get_size_requisition,
	span_text_editor_allocate_child_space,
	span_text_editor_recursive_render,
	span_text_editor_on_button_press
};

static void span_text_editor_on_recursive_delete(CongElementEditor *element_editor)
{
	/* FIXME: unimplemented */

}


static gboolean span_text_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event)
{
	/* FIXME: unimplemented */

	switch (event->type) {
	default: break;
	case CONG_DOCUMENT_EVENT_SET_TEXT:
		/* hackish redraw: */
		gtk_widget_queue_draw(GTK_WIDGET(element_editor->widget));
		break;
	}
	return FALSE;	
}

static void span_text_editor_get_size_requisition(CongElementEditor *element_editor)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	CongSpanTextEditor *span_text = CONG_SPAN_TEXT_EDITOR(element_editor);
	CongNodePtr node = element_editor->first_node;

	/* FIXME: this is a hack */
	element_editor->requisition.width=100;
	element_editor->requisition.height=25;

	while (node!=element_editor->final_node) {
		element_editor->requisition.height+=25;		
		node = node->next;
	}
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
		CongDocument *doc;
		CongDispspec *ds;
		CongNodePtr iter;
		int y;

		/* FIXME: unimplemented */

		gdk_draw_rectangle(GDK_DRAWABLE(w->window),
				   w->style->black_gc,
				   FALSE, /* gint filled, */
				   intersected_area.x,
				   intersected_area.y,
				   intersected_area.width-1,
				   intersected_area.height-1);

		iter=element_editor->first_node;

		doc = cong_editor_widget_get_document(editor_widget);
		ds = cong_document_get_dispspec(doc);

		y = element_editor->window_area.y;

		while (1){
			/* Hackish test code: */
			CongFont *title_font;
			GdkGC *gc;
			CongDispspecElement *element = cong_dispspec_get_first_element(ds);

			g_assert(iter);

			title_font = cong_dispspec_element_get_font(element, CONG_FONT_ROLE_TITLE_TEXT);
			g_assert(title_font);

			gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_TEXT);
			if (iter->content) {
				gdk_draw_string(w->window,
						title_font->gdk_font,
						gc, 
						element_editor->window_area.x,
						y,
						iter->content);				
				y+=25;
			}

			if (iter==element_editor->final_node) {
				break;
			} else {
				iter = iter->next;
			}
		} 
		
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
CongElementEditor *cong_span_text_editor_new(CongEditorWidget *widget, CongNodePtr first_node, CongNodePtr final_node)
{
	CongSpanTextEditor *span_text;

	g_return_val_if_fail(widget, NULL);
	g_return_val_if_fail(first_node, NULL);
	g_return_val_if_fail(final_node, NULL);
	g_return_val_if_fail( (first_node->parent == final_node->parent), NULL);

	g_message("cong_span_text_editor_new");

	span_text = g_new0(CongSpanTextEditor,1);
	span_text->element_editor.klass = &span_text_editor_class;
	span_text->element_editor.widget = widget;
	span_text->element_editor.first_node = first_node;
	span_text->element_editor.final_node = final_node;

	/* recursive creation? */
#if 0
	recursively_create_children(span_text);
#endif

	return CONG_ELEMENT_EDITOR(span_text);
}

