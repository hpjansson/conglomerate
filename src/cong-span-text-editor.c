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

typedef struct CongTextSpan CongTextSpan;

/* Struct representing a run of characters within the plaintext cache from a specific text node: */
struct CongTextSpan
{
	int first_byte_offset;
	int byte_count;
	CongNodePtr text_node;
};

typedef struct CongTextRange CongTextRange;

struct CongTextRange
{
	int depth;
	int first_byte_offset;
	int final_byte_offset;
	int byte_count;
	CongNodePtr node;
	CongDispspecElement* element;
};

struct CongSpanTextEditor
{
	CongElementEditor element_editor;

	int tag_height;

	/* Cached info: */
	gchar *plain_text; /* all the text of the range of nodes, merged into one string, without any markup */
	GList *list_of_cong_text_span;
	GHashTable *hash_of_node_to_text_range;
	
	/* A PangoLayout to deal with word wrap issues */
	PangoLayout *pango_layout;
};

#if 0
typedef struct CongSpanStack CongSpanStack;
typedef struct CongSpanStackEntry CongSpanStackEntry;

struct CongSpanStack
{
	GList *bottom_of_stack;
	GList *top_of_stack;
};

struct CongSpanStackEntry
{
	CongNodePtr 
};
#endif


static void regenerate_plaintext(CongSpanTextEditor *span_text_editor);

static void span_text_editor_on_recursive_delete(CongElementEditor *element_editor);
static void span_text_editor_on_recursive_self_test(CongElementEditor *element_editor);
static gboolean span_text_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event);
static void span_text_editor_get_size_requisition(CongElementEditor *element_editor, int width_hint);
static void span_text_editor_allocate_child_space(CongElementEditor *element_editor);
static void span_text_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect);
static void span_text_editor_on_button_press(CongElementEditor *element_editor, GdkEventButton *event);

static CongElementEditorClass span_text_editor_class =
{
	"span_text_editor",
	span_text_editor_on_recursive_delete,
	span_text_editor_on_recursive_self_test,
	span_text_editor_on_document_event,
	span_text_editor_get_size_requisition,
	span_text_editor_allocate_child_space,
	span_text_editor_recursive_render,
	span_text_editor_on_button_press
};

static void add_text(CongSpanTextEditor *span_text_editor, CongNodePtr node)
{
	gchar *new_value;
	CongTextSpan *text_span;

	g_return_if_fail(span_text_editor);
	g_return_if_fail(node);
	g_return_if_fail(cong_node_type(node)==CONG_NODE_TYPE_TEXT);
	g_return_if_fail(node->content);

	/* Add stuff to the list of spans */
	text_span = g_new0(CongTextSpan,1);
	text_span->first_byte_offset = strlen(span_text_editor->plain_text);
	text_span->byte_count = strlen(node->content);
	text_span->text_node = node;
	g_list_append(span_text_editor->list_of_cong_text_span, text_span);

	/* Add to the big plaintext string: */
	new_value = g_strdup_printf("%s%s", span_text_editor->plain_text, node->content);
	g_free(span_text_editor->plain_text);
	
	span_text_editor->plain_text = new_value;

}

/**
 * Recursively traverse the node tree, calling add_text on text nodes and registering CongTextRanges for the spans
 */
static void regenerate_plaintext_recursive(CongSpanTextEditor *span_text_editor, CongNodePtr node, int depth)
{
	g_return_if_fail(span_text_editor);

	if (cong_node_type(node)==CONG_NODE_TYPE_TEXT) {
		add_text(span_text_editor,node);
	} else {
		CongNodePtr child;

		CongTextRange *text_range = NULL;
		CongDispspecElement* element = cong_dispspec_lookup_node(cong_editor_widget_get_dispspec(CONG_ELEMENT_EDITOR(span_text_editor)->widget), 
									 node);

		if (element) {
			if (cong_dispspec_element_type(element)==CONG_ELEMENT_TYPE_SPAN) {
				/* Create CongTextRange: */
				text_range = g_new0(CongTextRange,1);
				
				text_range->depth = depth;
				text_range->node = node;
				text_range->element = element;

				/* Begin calculating range: */ 
				text_range->first_byte_offset = strlen(span_text_editor->plain_text);
			}
		}

		/* Recurse: */
		for (child=node->children; child; child = child->next) {
			regenerate_plaintext_recursive(span_text_editor, child, (text_range!=NULL)?depth+1:depth);
		}

		if (text_range) {

			/* Finish calculating range: */ 
			text_range->final_byte_offset = strlen(span_text_editor->plain_text)-1;
			text_range->byte_count = text_range->final_byte_offset + 1 - text_range->first_byte_offset;

			/* Add to hash table of span ranges: */
			g_assert(span_text_editor->hash_of_node_to_text_range);
			g_hash_table_insert(span_text_editor->hash_of_node_to_text_range,
					    node,
					    text_range);
		}

	}
}

static void value_destroy_func(gpointer data)
{
	g_free(data);
}

static void regenerate_plaintext(CongSpanTextEditor *span_text_editor)
{
	CongNodePtr iter;
	GList *list_iter;
	g_return_if_fail(span_text_editor);

	/* Clean up any existing representation: */
	if (span_text_editor->plain_text) {
		g_free(span_text_editor->plain_text);
	}

	for (list_iter = span_text_editor->list_of_cong_text_span; list_iter; list_iter = list_iter->next) {
		g_free(list_iter->data);
	}
	g_list_free(span_text_editor->list_of_cong_text_span);

	if (span_text_editor->hash_of_node_to_text_range) {
		g_hash_table_destroy(span_text_editor->hash_of_node_to_text_range);
	}

	/* Rebuild representation: */
	span_text_editor->plain_text = g_strdup("");

	span_text_editor->hash_of_node_to_text_range = g_hash_table_new_full(g_direct_hash,
									     g_direct_equal,
									     NULL,
									     value_destroy_func);

	iter = span_text_editor->element_editor.first_node; 

	while (1) {
		gchar *cleaned_text;
		gchar *new_value;

		g_assert(iter);

		regenerate_plaintext_recursive(span_text_editor, iter, 0);

		if (iter == span_text_editor->element_editor.final_node) {
			break;
		} else {
			iter = iter->next;
		}
	}

	g_assert(span_text_editor->plain_text);
	
	pango_layout_set_text( span_text_editor->pango_layout, 
			       span_text_editor->plain_text, 
			       -1);
}

static void span_text_editor_on_recursive_delete(CongElementEditor *element_editor)
{
	/* FIXME: unimplemented */

}

static void span_text_editor_on_recursive_self_test(CongElementEditor *element_editor)
{
	g_return_if_fail(element_editor);

	/* Test this node: */
	g_assert(element_editor->first_node);
	g_assert(element_editor->final_node);
	g_assert(element_editor->first_node->parent == element_editor->final_node->parent);

	/* FIXME: unimplemented */
}

static gboolean span_text_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event)
{
	CongSpanTextEditor *span_text = CONG_SPAN_TEXT_EDITOR(element_editor);
	/* FIXME: unimplemented */

	switch (event->type) {
	default: break;
	case CONG_DOCUMENT_EVENT_SET_TEXT:
		if (cong_element_editor_responsible_for_node(element_editor, event->data.set_text.node)) {
			regenerate_plaintext(span_text);			

			/* FIXME: but what about descendants? */

			/* hackish redraw: */
			gtk_widget_queue_draw(GTK_WIDGET(element_editor->widget));		

			return TRUE;
		}
		break;
	}
	return FALSE;	
}

static void span_text_editor_get_size_requisition(CongElementEditor *element_editor, int width_hint)
{
	CongEditorWidget *editor_widget = element_editor->widget;
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	CongSpanTextEditor *span_text = CONG_SPAN_TEXT_EDITOR(element_editor);
	CongNodePtr node = element_editor->first_node;

	pango_layout_set_width(span_text->pango_layout,
			       width_hint*PANGO_SCALE);

	pango_layout_get_pixel_size(span_text->pango_layout,
				    &element_editor->requisition.width,
				    &element_editor->requisition.height);
}

static void span_text_editor_allocate_child_space(CongElementEditor *element_editor)
{
	CongSpanTextEditor *span_text = CONG_SPAN_TEXT_EDITOR(element_editor);

	pango_layout_set_width(span_text->pango_layout,
			       element_editor->window_area.width*PANGO_SCALE);
}

struct RenderTextRangeData
{
	CongSpanTextEditor *span_text;
	PangoLayoutLine *line;
	int offset_x;
	int offset_y;
	int max_x;
	GdkDrawable *drawable;
};

static void render_text_range(CongNodePtr key, 
			      CongTextRange *text_range,
			      struct RenderTextRangeData *data)
{
	/* is this span present on the given line? */
	if (text_range->final_byte_offset < data->line->start_index) {
		/* Reject; the range ends before this line */
		return;
	}
	
	if (text_range->first_byte_offset > data->line->start_index+data->line->length) {
		/* Reject; the range starts after this line */
		return;
	}

	/* OK; at least part of this range is present on this line: */
	{
		/* Calculate start/end points on this this line: */
		int start_x,end_x;
		int width, text_width;
		int y = data->offset_y - (text_range->depth * data->span_text->tag_height);

		GdkGC *gc = cong_dispspec_element_gc(text_range->element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE);
		const gchar *span_name = cong_dispspec_element_username(text_range->element);
		CongFont *span_font = cong_span_text_editor_get_font(data->span_text, CONG_FONT_ROLE_SPAN_TAG);
		g_assert(span_font);
		
		if (text_range->first_byte_offset >= data->line->start_index) {
			/* range starts somewhere on this line: */
			pango_layout_line_index_to_x(data->line,
						     text_range->first_byte_offset,
						     FALSE,
						     &start_x);	
			start_x/=PANGO_SCALE;
			start_x+=data->offset_x;

			gdk_draw_line(data->drawable, gc, 
				      start_x, y, 
				      start_x, y - 2);
		} else {
			/* range started somewhere before this line: */
			start_x = data->offset_x;
		}

		if (text_range->final_byte_offset <= data->line->start_index+data->line->length) {
			/* range finishes somewhere on this line: */
			pango_layout_line_index_to_x(data->line,
						     text_range->final_byte_offset,
						     TRUE,
						     &end_x);	
			end_x/=PANGO_SCALE;
			end_x+=data->offset_x;

			gdk_draw_line(data->drawable, gc, 
				      end_x, y, 
				      end_x, y - 2);
		} else {
			/* range finishes somewhere after this line: */
			end_x = data->max_x;
		}
			
		width = end_x-start_x;

		text_width = gdk_string_width(span_font->gdk_font, 
					      span_name);
		if (text_width < width - 6)
		{
			int text_y = y + (span_font->asc + span_font->desc) / 2;
			
			/* Draw text and lines: */
			gdk_draw_string(data->drawable, span_font->gdk_font, gc, start_x + 1 + (width - text_width) / 2,
					text_y, span_name);
			gdk_draw_line(data->drawable, gc, 
				      start_x, y, 
				      start_x - 1 + (width - text_width) / 2, y);
			gdk_draw_line(data->drawable, gc, 
				      end_x + 1 - (width - text_width) / 2, y, 
				      end_x, y);
		}
		else
		{
			/* Draw line: */			
			gdk_draw_line(data->drawable, gc, 
				      start_x, y, 
				      end_x, y);		
		}

	}
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

#if 1
		GSList *line_iter;
		GList *text_span_iter = span_text->list_of_cong_text_span;	

		int y = element_editor->window_area.y;

		for (line_iter=pango_layout_get_lines(span_text->pango_layout); line_iter; line_iter = line_iter->next) {
			PangoLayoutLine *line = line_iter->data;
			g_assert(line);

			gdk_draw_layout_line(GDK_DRAWABLE(w->window),
					     w->style->black_gc,
					     element_editor->window_area.x,
					     y,
					     line);

			/* Render spans: */
			{
				struct RenderTextRangeData render_text_range_data;

				render_text_range_data.span_text = span_text;
				render_text_range_data.line = line;
				render_text_range_data.offset_x = element_editor->window_area.x;
				render_text_range_data.offset_y = y+5;
				render_text_range_data.max_x = element_editor->window_area.x + element_editor->window_area.width;
				render_text_range_data.drawable = GDK_DRAWABLE(w->window);				

				g_assert(span_text->hash_of_node_to_text_range);
				g_hash_table_foreach(span_text->hash_of_node_to_text_range,
						     render_text_range,
						     &render_text_range_data);
			}

			y+=25; /* for now; will want to have the line cache as before */
		}
#else
		gdk_draw_layout(GDK_DRAWABLE(w->window),
				w->style->black_gc,
				element_editor->window_area.x,
				element_editor->window_area.y,
				span_text->pango_layout);

		/*  printf("plaintext:\"%s\"\n", span_text->plain_text); */
#endif
		
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

	CongFont *body_font;
	CongFont *span_font;

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

	span_text->pango_layout = pango_layout_new(gdk_pango_context_get() /*the_globals.pango_context*/);
	g_assert(span_text->pango_layout);

	body_font = cong_span_text_editor_get_font(span_text, CONG_FONT_ROLE_BODY_TEXT);
	g_assert(body_font);

	pango_layout_set_font_description(span_text->pango_layout,
					  body_font->font_desc);

	span_font = cong_span_text_editor_get_font(span_text, CONG_FONT_ROLE_SPAN_TAG);
	g_assert(span_font);

	span_text->tag_height = (span_font->asc + span_font->desc) / 2;
	if (span_text->tag_height < 3) span_text->tag_height = 3;
	span_text->tag_height += (span_font->asc + span_font->desc) / 2;

	pango_layout_set_justify(span_text->pango_layout,
				 TRUE);
#if 0
	/* ignore carriage returns etc in the text; this adds nasty visual carriage returns to the text: */
	pango_layout_set_single_paragraph_mode(span_text->pango_layout,
					       TRUE);
#endif

	regenerate_plaintext(span_text);

	return CONG_ELEMENT_EDITOR(span_text);
}


CongFont*
cong_span_text_editor_get_font(CongSpanTextEditor *span_text, enum CongFontRole role)
{
	g_return_val_if_fail(span_text, NULL);
	g_return_val_if_fail(role<CONG_FONT_ROLE_NUM, NULL);

	/* fonts are currently a property of the app: */
	return the_globals.fonts[role];
}
