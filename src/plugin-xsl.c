/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-xsl.c
 *
 * Plugin for XSL support
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

#include "cong-eel.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-font.h"

#include "cong-fake-plugin-hooks.h"

#if 0
typedef struct CongXSLTemplateElementEditor CongXSLTemplateElementEditor;
struct CongXSLTemplateElementEditor
{
	CongElementEditor element_editor;
	gchar *message;
};
#define CONG_XSLTEMPLATE_ELEMENT_EDITOR(x) ((CongXSLTemplateElementEditor*)(x))

#define ARROW_SCALE (10)
#define BOX_PADDING (5)

static void xsltemplate_element_editor_on_recursive_delete(CongElementEditor *element_editor);
static void xsltemplate_element_editor_on_recursive_self_test(CongElementEditor *element_editor);
static gboolean xsltemplate_element_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event);
static void xsltemplate_element_editor_get_size_requisition(CongElementEditor *element_editor, int width_hint);
static void xsltemplate_element_editor_allocate_child_space(CongElementEditor *element_editor);
static void xsltemplate_element_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect);
static void xsltemplate_element_on_button_press(CongElementEditor *element_editor, GdkEventButton *event);
static void xsltemplate_element_on_motion_notify(CongElementEditor *element_editor, GdkEventMotion *event);
static void xsltemplate_element_on_key_press(CongElementEditor *element_editor, GdkEventKey *event);

static CongElementEditorClass xsltemplate_element_editor_class =
{
	"xsltemplate_element_editor",
	xsltemplate_element_editor_on_recursive_delete,
	xsltemplate_element_editor_on_recursive_self_test,
	xsltemplate_element_editor_on_document_event,
	xsltemplate_element_editor_get_size_requisition,
	xsltemplate_element_editor_allocate_child_space,
	xsltemplate_element_editor_recursive_render,
	xsltemplate_element_on_button_press,
	xsltemplate_element_on_motion_notify,
	xsltemplate_element_on_key_press
};

static void xsltemplate_element_editor_on_recursive_delete(CongElementEditor *element_editor)
{
	/* FIXME: unimplemented */
}

static void xsltemplate_element_editor_on_recursive_self_test(CongElementEditor *element_editor)
{
	/* FIXME: unimplemented */
}

static gboolean xsltemplate_element_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event)
{
	CongEditorWidget2 *editor_widget = element_editor->widget;
	CongXSLTemplateElementEditor *xsltemplate_element = CONG_XSLTEMPLATE_ELEMENT_EDITOR(element_editor);
	CongEditorWidget2Details* details = GET_DETAILS(editor_widget);
	GList *iter;

	g_return_val_if_fail(event, FALSE);

	return FALSE;
}

static void xsltemplate_element_editor_get_size_requisition(CongElementEditor *element_editor, int width_hint)
{
	CongEditorWidget2 *editor_widget = element_editor->widget;
	CongXSLTemplateElementEditor *xsltemplate_element = CONG_XSLTEMPLATE_ELEMENT_EDITOR(element_editor);
	CongEditorWidget2Details* details = GET_DETAILS(editor_widget);
	GtkRequisition *requisition = &element_editor->requisition;
	GList *iter;

	requisition->width = 300; /* for now */
	requisition->height = 100; /* for now */
}

static void xsltemplate_element_editor_allocate_child_space(CongElementEditor *element_editor)
{
	/* empty */
}


static void xsltemplate_element_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect)
{
	CongEditorWidget2 *editor_widget = element_editor->widget;
	CongXSLTemplateElementEditor *xsltemplate_element = CONG_XSLTEMPLATE_ELEMENT_EDITOR(element_editor);
	CongEditorWidget2Details* details = GET_DETAILS(editor_widget);
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

	gint centre_x;
	gint centre_y;
	gint box_top;
	gint box_height;
	gint box_width;
	gint left_box_x;
	gint right_box_x;

	g_return_if_fail(window_rect);
	g_return_if_fail(editor_widget);
	g_return_if_fail(xsltemplate_element);

	window_area = &CONG_ELEMENT_EDITOR(xsltemplate_element)->window_area;

	/* Early accept/reject against the areas: */
	if (!gdk_rectangle_intersect((GdkRectangle*)window_rect,
				     (GdkRectangle*)window_area,
				     &intersected_area)) {

		/* No intersection; return immediately - and hence do not recurse into the children of this editor */
		return;
	}

	centre_x = window_area->x + (window_area->width/2);
	centre_y = window_area->y + (window_area->height/2);
	box_top = window_area->y + BOX_PADDING;
	box_height = window_area->height - (BOX_PADDING*2);
	box_width = (window_area->width - ((BOX_PADDING*4) + (ARROW_SCALE*8)))/2;
	left_box_x = centre_x - ((ARROW_SCALE*4) + BOX_PADDING + box_width);
	right_box_x = centre_x + ((ARROW_SCALE*4) + BOX_PADDING);

	doc = cong_editor_widget2_get_document(editor_widget);
	ds = cong_document_get_dispspec(doc);
	x = CONG_ELEMENT_EDITOR(xsltemplate_element)->first_node;
	element = cong_dispspec_get_first_element(ds);
	g_assert(element);

 	title_font = cong_dispspec_element_get_font(element, CONG_FONT_ROLE_TITLE_TEXT);
	g_assert(title_font);

	gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_TEXT);

#if 0
	/* Render a rectangle to indicate the area covered by this element_editor: */
	gdk_draw_rectangle(window,
			   gc,
			   FALSE,
			   window_area->x,
			   window_area->y,
			   window_area->width,
			   window_area->height);
#endif

	/* Draw arrow in hackish way (FIXME: eventually use SVG for this!): */
	{

		gdk_draw_line(window,
			      gc,
			      centre_x - (4*ARROW_SCALE),
			      centre_y - (ARROW_SCALE),
			      centre_x,
			      centre_y - (ARROW_SCALE));
		gdk_draw_line(window,
			      gc,
			      centre_x,
			      centre_y - (ARROW_SCALE),
			      centre_x,
			      centre_y - (3*ARROW_SCALE));
		gdk_draw_line(window,
			      gc,
			      centre_x,
			      centre_y - (3*ARROW_SCALE),
			      centre_x + (4*ARROW_SCALE),
			      centre_y);
		gdk_draw_line(window,
			      gc,
			      centre_x + (4*ARROW_SCALE),
			      centre_y,
			      centre_x,
			      centre_y + (3*ARROW_SCALE));
		gdk_draw_line(window,
			      gc,
			      centre_x,
			      centre_y + (3*ARROW_SCALE),
			      centre_x,
			      centre_y + (ARROW_SCALE));
		gdk_draw_line(window,
			      gc,
			      centre_x ,
			      centre_y + (ARROW_SCALE),
			      centre_x - (4*ARROW_SCALE),
			      centre_y + (ARROW_SCALE));
		gdk_draw_line(window,
			      gc,
			      centre_x - (4*ARROW_SCALE),
			      centre_y + (ARROW_SCALE),
			      centre_x - (4*ARROW_SCALE),
			      centre_y - (ARROW_SCALE));
	}

	/* Draw boxes around the left and right-hand sides of the rule: */
	{
		/* Left-hand side: */
		gdk_draw_rectangle(window,
				   gc,
				   FALSE,
				   left_box_x,
				   box_top,
				   box_width,
				   box_height);
		
		/* Right-hand side: */
		gdk_draw_rectangle(window,
				   gc,
				   FALSE,
				   right_box_x,
				   box_top,
				   box_width,
				   box_height);
	}

	/* Render text on the left-hand side: */
	/* FIXME:  should render a decent visual representation of the "match" rule */
	{
		CongNodePtr node = cong_element_editor_get_first_node(CONG_ELEMENT_EDITOR(xsltemplate_element));
		xmlChar* match = xmlGetProp(node,"match");

		if (match) {
			cong_font_draw_string_slow(window,
						   title_font,
						   gc, 
						   match,
						   left_box_x+(box_width/2), 
						   centre_y,
						   CONG_FONT_Y_POS_BASELINE);
		}
	}

}

static void xsltemplate_element_on_button_press(CongElementEditor *element_editor, GdkEventButton *event)
{
	/* empty */
}

static void xsltemplate_element_on_motion_notify(CongElementEditor *element_editor, GdkEventMotion *event)
{
	/* empty */
}

static void xsltemplate_element_on_key_press(CongElementEditor *element_editor, GdkEventKey *event)
{
	/* FIXME: unimplemented */
}

static CongElementEditor* manufacture_element(CongPluginEditorElement *plugin_editor_element, 
					      CongEditorWidget2 *editor_widget, 
					      CongNodePtr node, 
					      gpointer user_data)
{
#if 1
	CongXSLTemplateElementEditor *xsltemplate_element = g_new0(CongXSLTemplateElementEditor,1);
	xsltemplate_element->element_editor.klass = &xsltemplate_element_editor_class;
	xsltemplate_element->element_editor.widget = editor_widget;
	xsltemplate_element->element_editor.first_node = node;
	xsltemplate_element->element_editor.final_node = node;
	xsltemplate_element->message = g_strdup("this is slowly becoming the XSL template element editor thingy");

	cong_editor_widget2_register_element_editor(editor_widget, CONG_ELEMENT_EDITOR(xsltemplate_element));

	return CONG_ELEMENT_EDITOR(xsltemplate_element);
#else
	return cong_dummy_element_editor_new(editor_widget, node, "This will be the XSL template element");
#endif
}
#endif

/* would be exposed as "plugin_register"? */
/**
 * plugin_xsl_plugin_register:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_xsl_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	/* FIXME: port all of this code over to the CongEditorWidget3 framework: */
#if 0	
	cong_plugin_register_editor_element(plugin, 
					    _("XSL Template Element"), 
					    _("An element for visualising template rules in an XSL stylesheet"),
					    "xsl-template",
					    manufacture_element,
					    NULL);
#endif

	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
/**
 * plugin_xsl_plugin_configure:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_xsl_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
