/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-dummy-element-editor.c
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
#include "cong-eel.h"
#include "cong-document.h"
#include "cong-dispspec.h"

struct CongDummyElementEditor
{
	CongElementEditor element_editor;
	gchar *message;
};

static void dummy_element_editor_on_recursive_delete(CongElementEditor *element_editor);
static void dummy_element_editor_on_recursive_self_test(CongElementEditor *element_editor);
static gboolean dummy_element_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event);
static void dummy_element_editor_get_size_requisition(CongElementEditor *element_editor, int width_hint);
static void dummy_element_editor_allocate_child_space(CongElementEditor *element_editor);
static void dummy_element_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect);
static void dummy_element_on_button_press(CongElementEditor *element_editor, GdkEventButton *event);
static void dummy_element_on_motion_notify(CongElementEditor *element_editor, GdkEventMotion *event);
static void dummy_element_on_key_press(CongElementEditor *element_editor, GdkEventKey *event);

static CongElementEditorClass dummy_element_editor_class =
{
	"dummy_element_editor",
	dummy_element_editor_on_recursive_delete,
	dummy_element_editor_on_recursive_self_test,
	dummy_element_editor_on_document_event,
	dummy_element_editor_get_size_requisition,
	dummy_element_editor_allocate_child_space,
	dummy_element_editor_recursive_render,
	dummy_element_on_button_press,
	dummy_element_on_motion_notify,
	dummy_element_on_key_press
};

static void dummy_element_editor_on_recursive_delete(CongElementEditor *element_editor)
{
	/* FIXME: unimplemented */
}

static void dummy_element_editor_on_recursive_self_test(CongElementEditor *element_editor)
{
	/* FIXME: unimplemented */
}

static gboolean dummy_element_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongDummyElementEditor *dummy_element = CONG_DUMMY_ELEMENT_EDITOR(element_editor);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	GList *iter;

	g_return_val_if_fail(event, FALSE);

	return FALSE;
}

static void dummy_element_editor_get_size_requisition(CongElementEditor *element_editor, int width_hint)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongDummyElementEditor *dummy_element = CONG_DUMMY_ELEMENT_EDITOR(element_editor);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	GtkRequisition *requisition = &element_editor->requisition;
	GList *iter;

	requisition->width = 100; /* for now */
	requisition->height = 25;
}

static void dummy_element_editor_allocate_child_space(CongElementEditor *element_editor)
{
	/* empty */
}


static void dummy_element_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongDummyElementEditor *dummy_element = CONG_DUMMY_ELEMENT_EDITOR(element_editor);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	GList *iter;
	GdkGC *gc;
	int str_width;

	CongDocument *doc;
	CongDispspec *ds;
 	CongNodePtr x;
	CongDispspecElement *element;
	CongFont *title_font;
	GdkWindow *window = GTK_WIDGET(editor_widget)->window;
	GdkRectangle *window_area;
	GdkRectangle intersected_area;

	g_return_if_fail(window_rect);
	g_return_if_fail(editor_widget);
	g_return_if_fail(dummy_element);

	window_area = &CONG_ELEMENT_EDITOR(dummy_element)->window_area;

	/* Early accept/reject against the areas: */
	if (!gdk_rectangle_intersect((GdkRectangle*)window_rect,
				     (GdkRectangle*)window_area,
				     &intersected_area)) {

		/* No intersection; return immediately - and hence do not recurse into the children of this editor */
		return;
	}

	doc = cong_editor_widget_get_document(editor_widget);
	ds = cong_document_get_dispspec(doc);
	x = CONG_ELEMENT_EDITOR(dummy_element)->first_node;
	element = cong_dispspec_get_first_element(ds);
	g_assert(element);

 	title_font = cong_dispspec_element_get_font(element, CONG_FONT_ROLE_TITLE_TEXT);
	g_assert(title_font);

	gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_TEXT);
	
	/* Render a rectangle to indicate the area covered by this element_editor: */
	gdk_draw_rectangle(window,
			   gc,
			   FALSE,
			   window_area->x,
			   window_area->y,
			   window_area->width,
			   window_area->height);

	/* Render the string: */
	gdk_draw_string(window,
			title_font->gdk_font,
			gc, 
			window_area->x, 2 + title_font->asc + window_area->y,
			dummy_element->message);
}

static void dummy_element_on_button_press(CongElementEditor *element_editor, GdkEventButton *event)
{
	/* empty */
}

static void dummy_element_on_motion_notify(CongElementEditor *element_editor, GdkEventMotion *event)
{
	/* empty */
}

static void dummy_element_on_key_press(CongElementEditor *element_editor, GdkEventKey *event)
{
	/* FIXME: unimplemented */
}

/* Public API: */
CongElementEditor *cong_dummy_element_editor_new(CongEditorWidget *widget, CongNodePtr node, const gchar *message)
{
	CongDummyElementEditor *dummy_element = g_new0(CongDummyElementEditor,1);
	dummy_element->element_editor.klass = &dummy_element_editor_class;
	dummy_element->element_editor.widget = widget;
	dummy_element->element_editor.first_node = node;
	dummy_element->element_editor.final_node = node;
	dummy_element->message = g_strdup(message);

	cong_editor_widget_register_element_editor(widget, CONG_ELEMENT_EDITOR(dummy_element));

	return CONG_ELEMENT_EDITOR(dummy_element);
}

