/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-section-head-editor.c
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

#define V_SPACING (4)
#define H_SPACING (4)
#define H_INDENT (4)
#define FRAGMENT_WIDTH (45)
#define TITLE_HEIGHT (20)

struct CongSectionHeadEditor
{
	CongElementEditor element_editor;
	gboolean expanded;

	GList *list_of_child; /* of type element editor */
};

static void section_head_editor_get_size_requisition(CongElementEditor *element_editor);
static void section_head_editor_allocate_child_space(CongElementEditor *element_editor);
static void section_head_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect);
static void section_head_on_button_press(CongElementEditor *element_editor, GdkEventButton *event);

static CongElementEditorClass section_head_editor_class =
{
	"section_head_editor",
	section_head_editor_get_size_requisition,
	section_head_editor_allocate_child_space,
	section_head_editor_recursive_render,
	section_head_on_button_press
};

static void section_head_editor_get_size_requisition(CongElementEditor *element_editor)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongSectionHeadEditor *section_head = CONG_SECTION_HEAD_EDITOR(element_editor);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	GtkRequisition *requisition = &element_editor->requisition;
	GList *iter;

	requisition->width = 100; /* for now */
	requisition->height = TITLE_HEIGHT + 1 + V_SPACING;

	for (iter = section_head->list_of_child; iter; iter=iter->next) {
		CongElementEditor *child_editor = iter->data;
		g_assert(child_editor);

		cong_element_editor_get_size_requisition(child_editor);
		requisition->height += child_editor->requisition.height;
	}	
}

static void section_head_editor_allocate_child_space(CongElementEditor *element_editor)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongSectionHeadEditor *section_head = CONG_SECTION_HEAD_EDITOR(element_editor);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	GList *iter;
	GdkRectangle free_rectangle;

	free_rectangle.x = element_editor->window_area.x+1+H_SPACING;
	free_rectangle.y = element_editor->window_area.y+TITLE_HEIGHT;
	free_rectangle.width = element_editor->window_area.width - (1+H_SPACING);
	free_rectangle.height = element_editor->window_area.height - (TITLE_HEIGHT + 1 + V_SPACING);

	for (iter = section_head->list_of_child; iter; iter=iter->next) {
		CongElementEditor *child_editor;
		int child_height;

		child_editor = iter->data;
		g_assert(child_editor);

		child_height = child_editor->requisition.height;

		cong_element_editor_set_allocation(child_editor,
						   free_rectangle.x,
						   free_rectangle.y,
						   free_rectangle.width,
						   child_height);

		free_rectangle.y+=child_height;
		free_rectangle.height-=child_height;
	}	
}


static void section_head_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongSectionHeadEditor *section_head = CONG_SECTION_HEAD_EDITOR(element_editor);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	GList *iter;
	GdkGC *gc;
	int str_width;
	gchar *title_text;

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
	g_return_if_fail(section_head);

	window_area = &CONG_ELEMENT_EDITOR(section_head)->window_area;

	/* Early accept/reject against the areas: */
	if (!gdk_rectangle_intersect((GdkRectangle*)window_rect,
				     (GdkRectangle*)window_area,
				     &intersected_area)) {

		/* No intersection; return immediately - and hence do not recurse into the children of this editor */
		return;
	}

	doc = cong_editor_widget_get_document(editor_widget);
	ds = cong_document_get_dispspec(doc);
	x = CONG_ELEMENT_EDITOR(section_head)->node;
	element = cong_dispspec_lookup_element(ds, cong_node_name(x));
	g_assert(element);

 	title_font = cong_dispspec_element_get_font(element, CONG_FONT_ROLE_TITLE_TEXT);
	g_assert(title_font);

	gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE);
	g_assert(gc);

	/* Draw the frame rectangle "open" on the right-hand side : */
	/* Top */
	gdk_draw_line(window, gc, 
		      H_SPACING + window_area->x, 0 + window_area->y, 
		      window_area->width + window_area->x, 0 + window_area->y);

	/* Left */
	gdk_draw_line(window, gc, 
		      H_SPACING + window_area->x, 0 + window_area->y,
		      H_SPACING + window_area->x, (section_head->expanded ? window_area->height-1-V_SPACING : TITLE_HEIGHT-1-V_SPACING) + window_area->y);	

	/* Fill the inside of the rectangle: */
	gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_BACKGROUND);
	g_assert(gc);
	
	gdk_draw_rectangle(window, gc, 
			   TRUE, 
			   1 + H_SPACING + window_area->x, 1 + window_area->y, 
			   window_area->width - 1 - H_SPACING, TITLE_HEIGHT - 2 - V_SPACING);

	/* Render the text: */
	title_text = cong_dispspec_element_get_section_header_text(element, x);
	gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_TEXT);
	gdk_draw_string(window,
			title_font->gdk_font,
			gc, 
			H_SPACING + H_INDENT + window_area->x, 2 + title_font->asc + window_area->y,
			title_text);
	g_free(title_text);

	/* FIXME:  this will fail to update when the text is edited */


	/* Bottom */  
	if (section_head->expanded) {
		gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_DIM_LINE);
		g_assert(gc);

		/* Bottom of title bar: */
		gdk_draw_line(window, gc, 
			      1 + H_SPACING + window_area->x, TITLE_HEIGHT-1-V_SPACING + window_area->y,
			      window_area->width + window_area->x, TITLE_HEIGHT-1-V_SPACING + window_area->y);

		/* Short horizontal line along very bottom of area: */
		draw_blended_line(GTK_WIDGET(editor_widget),
				  cong_dispspec_element_col(element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE),
				  H_SPACING + window_area->x, window_area->height-1-V_SPACING + window_area->y,
				  H_SPACING + 45 + window_area->x);

		/* Render children: */
		for (iter = section_head->list_of_child; iter; iter=iter->next) {
			GdkRectangle intersected_child_area;
			CongElementEditor *child_editor = iter->data;
			g_assert(child_editor);

			/* Early accept/reject against the areas: */
			if (gdk_rectangle_intersect(&intersected_area,
						    &child_editor->window_area,
						    &intersected_child_area)) {
				
				cong_element_editor_recursive_render(child_editor, &intersected_area);
			}
		}

	} else {
		gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE);
		g_assert(gc);

		/* Bottom of title bar: */
		gdk_draw_line(window, gc, 
			      1 + H_SPACING + window_area->x, TITLE_HEIGHT-1-V_SPACING + window_area->y,
			      window_area->width + window_area->x, TITLE_HEIGHT-1-V_SPACING + window_area->y);
	}

}

static void section_head_on_button_press(CongElementEditor *element_editor, GdkEventButton *event)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongSectionHeadEditor *section_head = CONG_SECTION_HEAD_EDITOR(element_editor);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);

	section_head->expanded = !section_head->expanded;

#if 0
	gtk_widget_queue_draw_area(GTK_WIDGET(editor_widget),
				   gint x,
				   gint y,
				   gint width,
				   gint height);
#else
	gtk_widget_queue_draw(GTK_WIDGET(editor_widget));
#endif


}

static void recursively_create_children(CongSectionHeadEditor *section_head)
{
	CongEditorWidget *editor_widget;
	CongDocument *doc;
	CongDispspec *ds;
	CongNodePtr this_node;
	CongNodePtr child_node;

	g_return_if_fail(section_head);

	editor_widget = section_head->element_editor.widget;
	doc = cong_editor_widget_get_document(editor_widget);
	ds = cong_document_get_dispspec(doc);

      	this_node = cong_element_editor_get_node( CONG_ELEMENT_EDITOR(section_head) );
	for ( child_node = cong_node_first_child(this_node); child_node; child_node = cong_node_next(child_node))
	{
		enum CongNodeType node_type = cong_node_type(child_node);
		const char *name = xml_frag_name_nice(child_node);
		CongElementEditor *child = NULL;
		gboolean collapsed_child = FALSE;

		/* g_message("Examining frag %s\n",name); */

		if (node_type == CONG_NODE_TYPE_ELEMENT)
		{
			CongDispspecElement* element = cong_dispspec_lookup_element(ds, name);

			if (element) {
				if (cong_dispspec_element_is_structural(element)) {
					child = cong_section_head_editor_new(editor_widget, child_node);
					collapsed_child = cong_dispspec_element_collapseto(element);

				} else if (cong_dispspec_element_is_span(element) ||
					   CONG_ELEMENT_TYPE_INSERT == cong_dispspec_element_type(element)) {
					
					child = cong_span_text_editor_new(editor_widget, child_node);

				} else if (CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE==cong_dispspec_element_type(element)) {
					/* unwritten */
				}
			}
		}
		else if (node_type == CONG_NODE_TYPE_TEXT)
		{
			child = cong_span_text_editor_new(editor_widget, child_node);
		}

		if (child) {
			section_head->list_of_child = g_list_append(section_head->list_of_child, child);
		}		
	}
}

/* Public API: */
CongElementEditor *cong_section_head_editor_new(CongEditorWidget *widget, CongNodePtr node)
{
	CongSectionHeadEditor *section_head = g_new0(CongSectionHeadEditor,1);
	section_head->element_editor.klass = &section_head_editor_class;
	section_head->element_editor.widget = widget;
	section_head->element_editor.node = node;
	section_head->expanded = TRUE;

	/* recursive creation? */
	recursively_create_children(section_head);

	return CONG_ELEMENT_EDITOR(section_head);
}

