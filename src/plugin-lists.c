/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-lists.c
 *
 * Plugin for list support
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
#include "cong-plugin.h"

#include "cong-editor-widget-impl.h"
#include "cong-eel.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-font.h"

typedef struct CongListitemElementEditor CongListitemElementEditor;
struct CongListitemElementEditor
{
	CongElementEditor element_editor;
	gchar *message;
};
#define CONG_LISTITEM_ELEMENT_EDITOR(x) ((CongListitemElementEditor*)(x))

static void listitem_element_editor_on_recursive_delete(CongElementEditor *element_editor);
static void listitem_element_editor_on_recursive_self_test(CongElementEditor *element_editor);
static gboolean listitem_element_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event);
static void listitem_element_editor_get_size_requisition(CongElementEditor *element_editor, int width_hint);
static void listitem_element_editor_allocate_child_space(CongElementEditor *element_editor);
static void listitem_element_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect);
static void listitem_element_on_button_press(CongElementEditor *element_editor, GdkEventButton *event);
static void listitem_element_on_motion_notify(CongElementEditor *element_editor, GdkEventMotion *event);
static void listitem_element_on_key_press(CongElementEditor *element_editor, GdkEventKey *event);

static CongElementEditorClass listitem_element_editor_class =
{
	"listitem_element_editor",
	listitem_element_editor_on_recursive_delete,
	listitem_element_editor_on_recursive_self_test,
	listitem_element_editor_on_document_event,
	listitem_element_editor_get_size_requisition,
	listitem_element_editor_allocate_child_space,
	listitem_element_editor_recursive_render,
	listitem_element_on_button_press,
	listitem_element_on_motion_notify,
	listitem_element_on_key_press
};

static void listitem_element_editor_on_recursive_delete(CongElementEditor *element_editor)
{
	/* FIXME: unimplemented */
}

static void listitem_element_editor_on_recursive_self_test(CongElementEditor *element_editor)
{
	/* FIXME: unimplemented */
}

static gboolean listitem_element_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongListitemElementEditor *listitem_element = CONG_LISTITEM_ELEMENT_EDITOR(element_editor);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	GList *iter;

	g_return_val_if_fail(event, FALSE);

	return FALSE;
}

static void listitem_element_editor_get_size_requisition(CongElementEditor *element_editor, int width_hint)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongListitemElementEditor *listitem_element = CONG_LISTITEM_ELEMENT_EDITOR(element_editor);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	GtkRequisition *requisition = &element_editor->requisition;
	GList *iter;

	requisition->width = 100; /* for now */
	requisition->height = 25;
}

static void listitem_element_editor_allocate_child_space(CongElementEditor *element_editor)
{
	/* empty */
}


static void listitem_element_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongListitemElementEditor *listitem_element = CONG_LISTITEM_ELEMENT_EDITOR(element_editor);
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
	g_return_if_fail(listitem_element);

	window_area = &CONG_ELEMENT_EDITOR(listitem_element)->window_area;

	/* Early accept/reject against the areas: */
	if (!gdk_rectangle_intersect((GdkRectangle*)window_rect,
				     (GdkRectangle*)window_area,
				     &intersected_area)) {

		/* No intersection; return immediately - and hence do not recurse into the children of this editor */
		return;
	}

	doc = cong_editor_widget_get_document(editor_widget);
	ds = cong_document_get_dispspec(doc);
	x = CONG_ELEMENT_EDITOR(listitem_element)->first_node;
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
	cong_font_draw_string_slow(window,
				   title_font,
				   gc, 
				   listitem_element->message,
				   window_area->x, 
				   2 + window_area->y,
				   CONG_FONT_Y_POS_TOP
				   );
}

static void listitem_element_on_button_press(CongElementEditor *element_editor, GdkEventButton *event)
{
	/* empty */
}

static void listitem_element_on_motion_notify(CongElementEditor *element_editor, GdkEventMotion *event)
{
	/* empty */
}

static void listitem_element_on_key_press(CongElementEditor *element_editor, GdkEventKey *event)
{
	/* FIXME: unimplemented */
}

static CongElementEditor* manufacture_element(CongPluginEditorElement *plugin_editor_element, 
					      CongEditorWidget *editor_widget, 
					      CongNodePtr node, 
					      gpointer user_data)
{
#if 1
	CongListitemElementEditor *listitem_element = g_new0(CongListitemElementEditor,1);
	listitem_element->element_editor.klass = &listitem_element_editor_class;
	listitem_element->element_editor.widget = editor_widget;
	listitem_element->element_editor.first_node = node;
	listitem_element->element_editor.final_node = node;
	listitem_element->message = g_strdup("this is slowly becoming the list element editor thingy");

	cong_editor_widget_register_element_editor(editor_widget, CONG_ELEMENT_EDITOR(listitem_element));

	return CONG_ELEMENT_EDITOR(listitem_element);
#else
	return cong_dummy_element_editor_new(editor_widget, node, "This will be the list element");
#endif
}

 /* would be exposed as "plugin_register"? */
gboolean plugin_lists_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);
	
	cong_plugin_register_editor_element(plugin, 
					    _("List Member Element"), 
					    _("An element for visualising member of a list"),
					    "listitem",
					    manufacture_element,
					    NULL);

	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean plugin_lists_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}



