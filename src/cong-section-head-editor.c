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
#include "cong-eel.h"
#include "cong-dispspec.h"
#include "cong-document.h"

static void recursively_create_children(CongSectionHeadEditor *section_head);

#define V_SPACING (4)
#define H_SPACING (4)
#define H_INDENT (4)
#define FRAGMENT_WIDTH (45)
#define TITLE_HEIGHT (20)

struct CongSectionHeadEditor
{
	CongElementEditor element_editor;

	CongDispspecElement *element;

	gboolean expanded;

	GList *list_of_child; /* of type element editor */

	GdkRectangle title_bar_window_rect;
	PangoLayout *title_bar_pango_layout;
};

static void section_head_editor_on_recursive_delete(CongElementEditor *element_editor);
static void section_head_editor_on_recursive_self_test(CongElementEditor *element_editor);
static gboolean section_head_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event);
static void section_head_editor_get_size_requisition(CongElementEditor *element_editor, int width_hint);
static void section_head_editor_allocate_child_space(CongElementEditor *element_editor);
static void section_head_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect);
static void section_head_on_button_press(CongElementEditor *element_editor, GdkEventButton *event);
static void section_head_on_motion_notify(CongElementEditor *element_editor, GdkEventMotion *event);
static void section_head_on_key_press(CongElementEditor *element_editor, GdkEventKey *event);

static CongElementEditorClass section_head_editor_class =
{
	"section_head_editor",
	section_head_editor_on_recursive_delete,
	section_head_editor_on_recursive_self_test,
	section_head_editor_on_document_event,
	section_head_editor_get_size_requisition,
	section_head_editor_allocate_child_space,
	section_head_editor_recursive_render,
	section_head_on_button_press,
	section_head_on_motion_notify,
	section_head_on_key_press
};

static void section_head_editor_on_recursive_delete(CongElementEditor *element_editor)
{
	GList *iter;
	CongSectionHeadEditor *section_head = CONG_SECTION_HEAD_EDITOR(element_editor);

	for (iter = section_head->list_of_child; iter; iter=iter->next) {
		CongElementEditor *child_editor = iter->data;
		g_assert(child_editor);

		cong_element_editor_recursive_delete(child_editor);
	}
}

static void section_head_editor_on_recursive_self_test(CongElementEditor *element_editor)
{
	GList *iter;
	CongElementEditor *last_child_editor = NULL;
	CongSectionHeadEditor *section_head = CONG_SECTION_HEAD_EDITOR(element_editor);

	g_return_if_fail(element_editor);

	/* Test this node: */
	g_assert(element_editor->first_node);
	g_assert(element_editor->first_node == element_editor->final_node);

	/* Check integrity of children: */
	for (iter = section_head->list_of_child; iter; iter=iter->next) {
		CongElementEditor *child_editor = iter->data;
		g_assert(child_editor);

		g_assert(child_editor->first_node);
		g_assert(child_editor->final_node);
		g_assert(child_editor->first_node->parent == element_editor->first_node);
		g_assert(child_editor->final_node->parent == element_editor->first_node);

		/* After the first child, ensure that the node ranges for each child match up to cover all the children of the node: */
		if (last_child_editor) {
			g_assert(last_child_editor->final_node);
			g_assert(last_child_editor->final_node->next == child_editor->first_node);
			g_assert(last_child_editor->final_node == child_editor->first_node->prev);
		} else {
			g_assert(child_editor->first_node->prev == NULL);
		}

		if (iter->next==NULL) {
			g_assert(child_editor->final_node->next==NULL);
		}

		last_child_editor = child_editor;
	}

	/* Recurse: */
	for (iter = section_head->list_of_child; iter; iter=iter->next) {
		CongElementEditor *child_editor = iter->data;
		g_assert(child_editor);

		cong_element_editor_recursive_self_test(child_editor);
	}
}

static void delete_children(CongSectionHeadEditor *section_head)
{
/*  	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget); */
	GList *iter;

	/* Delete all child editors: */
	for (iter = section_head->list_of_child; iter; ) {
		GList *iter_next = iter->next;
		CongElementEditor *child_editor = iter->data;
		g_assert(child_editor);

		cong_element_editor_recursive_delete(child_editor);
		section_head->list_of_child = g_list_delete_link(section_head->list_of_child, iter);

		iter=iter_next;
	}
		
}

static void generate_children(CongSectionHeadEditor *section_head)
{
	CongEditorWidget *editor_widget = section_head->element_editor.widget;

	/* Regenerate the child editors: */
	recursively_create_children(section_head);

	cong_editor_widget_force_layout_update(editor_widget);
}

static void regenerate_children(CongSectionHeadEditor *section_head, gboolean before_event) 
{
	if (before_event) {
		delete_children(section_head);
	} else {
		generate_children(section_head);
	}
}


static gboolean section_head_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongSectionHeadEditor *section_head = CONG_SECTION_HEAD_EDITOR(element_editor);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	GList *iter;

	g_return_val_if_fail(event, FALSE);

#if 1
	switch (event->type) {
	default: g_assert_not_reached();
	case CONG_DOCUMENT_EVENT_MAKE_ORPHAN:
		/* If this editor is responsible for the node then delete and regenerate our children.  Otherwise,
		   pass the message on to all children? */
		if (element_editor->first_node == event->data.make_orphan.former_parent) {
			regenerate_children(section_head, event->before_event);

			return TRUE;
		} else {
			/* Recurse: */
			for (iter = section_head->list_of_child; iter; iter=iter->next) {
				CongElementEditor *child_editor = iter->data;
				g_assert(child_editor);

				if (cong_element_editor_on_document_event(child_editor, event)) {
					return TRUE;
				}
			}
		}
		break;
		
	case CONG_DOCUMENT_EVENT_ADD_AFTER:
		/* If the sibling is in our list, add it to our children in the correct place; otherwise delegate to children: */
		if (element_editor->first_node == event->data.add_after.older_sibling->parent) {
			regenerate_children(section_head, event->before_event);

			return TRUE;
		} else {
			/* Recurse: */
			for (iter = section_head->list_of_child; iter; iter=iter->next) {
				CongElementEditor *child_editor = iter->data;
				g_assert(child_editor);

				if (cong_element_editor_on_document_event(child_editor, event)) {
					return TRUE;
				}
			}
		}
		break;

	case CONG_DOCUMENT_EVENT_ADD_BEFORE:
		/* If the sibling is in our list, add it to our children in the correct place; otherwise delegate to children: */
		if (element_editor->first_node == event->data.add_before.younger_sibling->parent) {
			regenerate_children(section_head, event->before_event);
			
			return TRUE;
		} else {
			/* Recurse: */
			for (iter = section_head->list_of_child; iter; iter=iter->next) {
				CongElementEditor *child_editor = iter->data;
				g_assert(child_editor);

				if (cong_element_editor_on_document_event(child_editor, event)) {
					return TRUE;
				}
			}
		}
		break;
		
	case CONG_DOCUMENT_EVENT_SET_PARENT:
		/* If this is the parent, add it to the children (at the end); otherwise delegate to children: */
		if (element_editor->first_node == event->data.set_parent.adoptive_parent) {
			regenerate_children(section_head, event->before_event);

			return TRUE;
		} else {
			for (iter = section_head->list_of_child; iter; iter=iter->next) {
				CongElementEditor *child_editor = iter->data;
				g_assert(child_editor);
				
				if (cong_element_editor_on_document_event(child_editor, event)) {
					return TRUE;
				}
			}
		}
		break;

	case CONG_DOCUMENT_EVENT_SET_TEXT:
		/* This should never be sent to this type of editor: */
		g_assert_not_reached();
		break;
	}
#else
	switch (event->type) {
	default: g_assert_not_reached();
	case CONG_DOCUMENT_EVENT_MAKE_ORPHAN:
		/* Search for the node amongst the (direct) children; if found, then delete that child.  Otherwise,
		   pass the message on to all children? */
		for (iter = section_head->list_of_child; iter; iter=iter->next) {
			CongElementEditor *child_editor = iter->data;
			g_assert(child_editor);

			#error this will never succeed on the post-processing phase...
			if (cong_element_editor_responsible_for_node(child_editor,event->data.make_orphan.node)) {
				regenerate_children(section_head, event->before_event);

				return TRUE;
			} else {
				/* Recurse: */
				if (cong_element_editor_on_document_event(child_editor, event)) {
					return TRUE;
				}
			}
		}
		break;
		
	case CONG_DOCUMENT_EVENT_ADD_AFTER:
		/* If the sibling is in our list, add it to our children in the correct place; otherwise delegate to children: */
		for (iter = section_head->list_of_child; iter; iter=iter->next) {
			CongElementEditor *child_editor = iter->data;
			g_assert(child_editor);

			if (cong_element_editor_responsible_for_node(child_editor,event->data.add_after.older_sibling)) {
				regenerate_children(section_head, event->before_event);

				return TRUE;
			} else {
				/* Recurse: */
				if (cong_element_editor_on_document_event(child_editor, event)) {
					return TRUE;
				}
			}
		}
		break;

	case CONG_DOCUMENT_EVENT_ADD_BEFORE:
		/* If the sibling is in our list, add it to our children in the correct place; otherwise delegate to children: */
		for (iter = section_head->list_of_child; iter; iter=iter->next) {
			CongElementEditor *child_editor = iter->data;
			g_assert(child_editor);

			if (cong_element_editor_responsible_for_node(child_editor,event->data.add_before.younger_sibling)) {
				regenerate_children(section_head, event->before_event);

				return TRUE;
			} else {
				/* Recurse: */
				if (cong_element_editor_on_document_event(child_editor, event)) {
					return TRUE;
				}
			}
		}
		break;
		
	case CONG_DOCUMENT_EVENT_SET_PARENT:
		/* If this is the parent, add it to the children (at the end); otherwise delegate to children: */
		if (element_editor->first_node == event->data.set_parent.adoptive_parent) {
			regenerate_children(section_head, event->before_event);

			return TRUE;
		} else {
			for (iter = section_head->list_of_child; iter; iter=iter->next) {
				CongElementEditor *child_editor = iter->data;
				g_assert(child_editor);
				
				if (cong_element_editor_on_document_event(child_editor, event)) {
					return TRUE;
				}
			}
		}
		break;

	case CONG_DOCUMENT_EVENT_SET_TEXT:
		/* This should never be sent to this type of editor: */
		g_assert_not_reached();
		break;
	}
#endif

	return FALSE;
}

static void section_head_editor_get_size_requisition(CongElementEditor *element_editor, int width_hint)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongSectionHeadEditor *section_head = CONG_SECTION_HEAD_EDITOR(element_editor);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	GtkRequisition *requisition = &element_editor->requisition;
	GList *iter;

	requisition->width = 100; /* for now */
	requisition->height = TITLE_HEIGHT + 1 + V_SPACING;

	if (section_head->expanded) {

		int child_width_hint =  width_hint - (1+H_SPACING);

		if (child_width_hint<0) {
			child_width_hint = 0;
		}

		for (iter = section_head->list_of_child; iter; iter=iter->next) {
			CongElementEditor *child_editor = iter->data;
			g_assert(child_editor);
			
			cong_element_editor_get_size_requisition(child_editor,  child_width_hint);
			requisition->height += child_editor->requisition.height;
		} 
	}
}

static void section_head_editor_allocate_child_space(CongElementEditor *element_editor)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongSectionHeadEditor *section_head = CONG_SECTION_HEAD_EDITOR(element_editor);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	GList *iter;
	GdkRectangle free_rectangle;

	section_head->title_bar_window_rect.x = element_editor->window_area.x;
	section_head->title_bar_window_rect.y = element_editor->window_area.y;
	section_head->title_bar_window_rect.width = element_editor->window_area.width;
	section_head->title_bar_window_rect.height = TITLE_HEIGHT;

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
#if 0
	CongFont *title_font;
#endif
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
	x = CONG_ELEMENT_EDITOR(section_head)->first_node;

#if 0
 	title_font = cong_dispspec_element_get_font(element, CONG_FONT_ROLE_TITLE_TEXT);
	g_assert(title_font);
#endif

	gc = cong_dispspec_element_gc(section_head->element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE);
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
	gc = cong_dispspec_element_gc(section_head->element, CONG_DISPSPEC_GC_USAGE_BACKGROUND);
	g_assert(gc);
	
	gdk_draw_rectangle(window, gc, 
			   TRUE, 
			   1 + H_SPACING + window_area->x, 1 + window_area->y, 
			   window_area->width - 1 - H_SPACING, TITLE_HEIGHT - 2 - V_SPACING);

	/* Render the text: */
	title_text = cong_dispspec_element_get_section_header_text(section_head->element, x);
	gc = cong_dispspec_element_gc(section_head->element, CONG_DISPSPEC_GC_USAGE_TEXT);
#if 1
	pango_layout_set_text(section_head->title_bar_pango_layout,
			      title_text,
			      -1);
	pango_layout_set_width(section_head->title_bar_pango_layout,
			       window_area->width * PANGO_SCALE);

	gdk_draw_layout(window, 
			gc,
			H_SPACING + H_INDENT + window_area->x, 
			window_area->y,
			section_head->title_bar_pango_layout);
#else
	gdk_draw_string(window,
			title_font->gdk_font,
			gc, 
			H_SPACING + H_INDENT + window_area->x, 2 + title_font->asc + window_area->y,
			title_text);
#endif
	g_free(title_text);

	/* FIXME:  this will fail to update when the text is edited */


	/* Bottom */  
	if (section_head->expanded) {
		gc = cong_dispspec_element_gc(section_head->element, CONG_DISPSPEC_GC_USAGE_DIM_LINE);
		g_assert(gc);

		/* Bottom of title bar: */
		gdk_draw_line(window, gc, 
			      1 + H_SPACING + window_area->x, TITLE_HEIGHT-1-V_SPACING + window_area->y,
			      window_area->width + window_area->x, TITLE_HEIGHT-1-V_SPACING + window_area->y);

		/* Short horizontal line along very bottom of area: */
		draw_blended_line(GTK_WIDGET(editor_widget),
				  cong_dispspec_element_col(section_head->element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE),
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
		gc = cong_dispspec_element_gc(section_head->element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE);
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

	/* Test to see if the header bar was clicked: */
	if ( cong_eel_rectangle_contains(&section_head->title_bar_window_rect, 
					 event->x,
					 event->y) ){
		section_head->expanded = !section_head->expanded;

		cong_editor_widget_force_layout_update(editor_widget);

	} else {
		/* See if a child is "under" this event; delegate to the child: */

		GList *iter;

		for (iter = section_head->list_of_child; iter; iter=iter->next) {
			CongElementEditor *child_editor = iter->data;
			g_assert(child_editor);

			if ( cong_eel_rectangle_contains(&child_editor->window_area, 
							 event->x,
							 event->y) ){
				cong_element_editor_on_button_press(child_editor, event);
			}
		}
	}

}

static void section_head_on_motion_notify(CongElementEditor *element_editor, GdkEventMotion *event)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongSectionHeadEditor *section_head = CONG_SECTION_HEAD_EDITOR(element_editor);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);

	/* Test to see if the header bar was clicked: */
	if ( cong_eel_rectangle_contains(&section_head->title_bar_window_rect, 
					 event->x,
					 event->y) ){
		/* empty */
	} else {
		/* See if a child is "under" this event; delegate to the child: */

		GList *iter;

		for (iter = section_head->list_of_child; iter; iter=iter->next) {
			CongElementEditor *child_editor = iter->data;
			g_assert(child_editor);

			if ( cong_eel_rectangle_contains(&child_editor->window_area, 
							 event->x,
							 event->y) ){
				cong_element_editor_on_motion_notify(child_editor, event);
			}
		}
	}
}

static void section_head_on_key_press(CongElementEditor *element_editor, GdkEventKey *event)
{
	/* FIXME: unimplemented */
}

#if 0
static CongEditorWidget *create_child(CongSectionHeadEditor *section_head, CongNodePtr child_node)
{
	CongEditorWidget *editor_widget;
	CongDocument *doc;
	CongDispspec *ds;
	enum CongNodeType node_type = cong_node_type(child_node);
	const char *name = xml_frag_name_nice(child_node);

	editor_widget = section_head->element_editor.widget;
	doc = cong_editor_widget_get_document(editor_widget);
	ds = cong_document_get_dispspec(doc);

	if (node_type == CONG_NODE_TYPE_ELEMENT) {
		CongDispspecElement* element = cong_dispspec_lookup_element(ds, name);
		
		if (element) {
			if (cong_dispspec_element_is_structural(element)) {
				return CONG_EDITOR_WIDGET( cong_section_head_editor_new(editor_widget, child_node) );
				/*  collapsed_child = cong_dispspec_element_collapseto(element); */
				
			} else if (cong_dispspec_element_is_span(element) ||
				   CONG_ELEMENT_TYPE_INSERT == cong_dispspec_element_type(element)) {
				
				return CONG_EDITOR_WIDGET( cong_span_text_editor_new(editor_widget, child_node, child_node) );
				
			} else if (CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE==cong_dispspec_element_type(element)) {
				/* unwritten */
			}
		}	
	} else if (node_type == CONG_NODE_TYPE_TEXT) {
		return CONG_EDITOR_WIDGET( cong_span_text_editor_new(editor_widget, child_node, child_node) );
	}

	
	return NULL;

}
#endif

/* 
   Given an intitial node that can be used for a span text editor, find a younger sibling
   that is the final node of a range of siblings suitable for control by a single span text editor.
 */
static CongNodePtr get_final_node_of_span_text(CongNodePtr x, CongDispspec *ds)
{
	g_return_val_if_fail(x, NULL);
	g_return_val_if_fail(ds, NULL);

	for (; x->next ; x = cong_node_next(x))
	{
		enum CongNodeType node_type = cong_node_type(x);
		const char *name = xml_frag_name_nice(x);

		if (node_type == CONG_NODE_TYPE_ELEMENT && cong_dispspec_element_structural(ds, name))
		{
			return(cong_node_prev(x));
		}

		if (CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE==cong_dispspec_type(ds, name)) {
			return(cong_node_prev(x));
		}
	}

	g_assert(x);

	return(x);
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

      	this_node = CONG_ELEMENT_EDITOR(section_head)->first_node->children;

	while (this_node) {
		enum CongNodeType node_type;
		const char *name;
		CongElementEditor *child_editor = NULL;
		CongNodePtr next_node = this_node->next;

		g_assert(this_node);

		node_type = cong_node_type(this_node);
		name = xml_frag_name_nice(this_node);

		if (node_type == CONG_NODE_TYPE_ELEMENT) {
			CongDispspecElement* element = cong_dispspec_lookup_element(ds, name);
			
			if (element) {
				if (cong_dispspec_element_is_structural(element)) {
					child_editor = cong_section_head_editor_new(editor_widget, this_node);
					next_node = this_node->next;

				/*  collapsed_child = cong_dispspec_element_collapseto(element); */
					
				} else if (cong_dispspec_element_is_span(element) ||
					   CONG_ELEMENT_TYPE_INSERT == cong_dispspec_element_type(element)) {

					CongNodePtr final_node_of_span = get_final_node_of_span_text(this_node, ds);
					
					child_editor = cong_span_text_editor_new(editor_widget, this_node, final_node_of_span);

					next_node = final_node_of_span->next;

				} else if (CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE==cong_dispspec_element_type(element)) {
				/* unwritten */
				}
			}	
		} else if (node_type == CONG_NODE_TYPE_TEXT) {
			CongNodePtr final_node_of_span = get_final_node_of_span_text(this_node, ds);

			child_editor = cong_span_text_editor_new(editor_widget, this_node, final_node_of_span);

			next_node = final_node_of_span->next;
		}

		/* If no child editor has been created, create a dummy one: */
		if (child_editor==NULL) {
			child_editor = cong_dummy_element_editor_new(editor_widget, this_node);
		}

		/* add any child editor that's been created at the correct position: */
		g_assert(child_editor);
		section_head->list_of_child = g_list_append(section_head->list_of_child, child_editor);

		this_node = next_node;
	}

}

/* Public API: */
CongElementEditor *cong_section_head_editor_new(CongEditorWidget *widget, CongNodePtr node)
{
	CongSectionHeadEditor *section_head;
	CongDocument *doc;
	CongDispspec *ds;
	CongDispspecElement *element;
	CongFont *title_font;

	doc = cong_editor_widget_get_document(widget);
	ds = cong_document_get_dispspec(doc);
	element = cong_dispspec_lookup_element(ds, cong_node_name(node));
	g_assert(element);

	section_head = g_new0(CongSectionHeadEditor,1);
	section_head->element_editor.klass = &section_head_editor_class;
	section_head->element_editor.widget = widget;
	section_head->element_editor.first_node = node;
	section_head->element_editor.final_node = node;
	section_head->expanded = TRUE;
	section_head->element = element;

	cong_editor_widget_register_element_editor(widget, CONG_ELEMENT_EDITOR(section_head));

	section_head->title_bar_pango_layout = pango_layout_new(gdk_pango_context_get());
	g_assert(section_head->title_bar_pango_layout);

	title_font = cong_dispspec_element_get_font(element, CONG_FONT_ROLE_TITLE_TEXT);
	g_assert(title_font);

	pango_layout_set_font_description(section_head->title_bar_pango_layout,
					  title_font->font_desc);


	/* recursive creation? */
	recursively_create_children(section_head);

	return CONG_ELEMENT_EDITOR(section_head);
}

