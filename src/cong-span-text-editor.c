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
#include "cong-dispspec.h"
#include "cong-document.h"
#include "cong-error-dialog.h"
#include "cong-font.h"
#include "cong-app.h"

#if 0
#define CONG_SPAN_TEXT_DEBUG_MSG1(x)    g_message((x))
#define CONG_SPAN_TEXT_DEBUG_MSG2(x, a) g_message((x), (a))
#else
#define CONG_SPAN_TEXT_DEBUG_MSG1(x)    ((void)0)
#define CONG_SPAN_TEXT_DEBUG_MSG2(x, a) ((void)0)
#endif

#define H_SPACING (4)

enum CongLineVisitor
{
	CONG_LINE_VISITOR_CALCULATE_HEIGHT,
	CONG_LINE_VISITOR_RENDER,
	CONG_LINE_VISITOR_HIT_TEST
};

struct CongHitTest
{
	/* Inputs: */
	GdkPoint window_coord;

	/* Outputs: */
	gboolean got_hit;
	int byte_offset;
};

static int visit_lines(CongElementEditor *element_editor, enum CongLineVisitor visitor, struct CongHitTest *hit_test);

typedef struct CongTextSpan CongTextSpan;

/**
 * Struct representing a run of characters within the plaintext cache from a specific text node; 
 * useful for converting from PangoLayoutLines back to the underlying XML: 
 * There can be more than one of these for a particular text node; each is a subset of the characters
 * within the text node - this is to cope with the case where surplus whitespace characters are
 * stripped when the plaintext cache is built.
 */
struct CongTextSpan
{
	int original_first_byte_offset; /* offset into the text within the original node */
	int stripped_first_byte_offset; /* offset into the stripped plaintext cache */
	int byte_count; /* number of bytes within the stripped plaintext cache */
	CongNodePtr text_node; /* the original node */
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

static void regenerate_plaintext(CongSpanTextEditor *span_text_editor);
static CongNodePtr get_node_at_stripped_byte_offset(CongSpanTextEditor *span_text_editor, int byte_offset);
static CongTextSpan *get_text_span_at_stripped_byte_offset(CongSpanTextEditor *span_text_editor, int byte_offset);
static gboolean get_location_at_stripped_byte_offset(CongSpanTextEditor *span_text_editor, int byte_offset, CongLocation *location);
static gboolean get_stripped_byte_offset_at_location(CongSpanTextEditor *span_text_editor, const CongLocation *location, int *byte_offset);
static gboolean get_coord_for_location(CongSpanTextEditor *span_text_editor, const CongLocation *location, GdkPoint *window_location);

static void span_text_editor_on_recursive_delete(CongElementEditor *element_editor);
static void span_text_editor_on_recursive_self_test(CongElementEditor *element_editor);
static gboolean span_text_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event);
static void span_text_editor_get_size_requisition(CongElementEditor *element_editor, int width_hint);
static void span_text_editor_allocate_child_space(CongElementEditor *element_editor);
static void span_text_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect);
static void span_text_editor_on_button_press(CongElementEditor *element_editor, GdkEventButton *event);
static void span_text_editor_on_motion_notify(CongElementEditor *element_editor, GdkEventMotion *event);
static void span_text_editor_on_key_press(CongElementEditor *element_editor, GdkEventKey *event);

static CongElementEditorClass span_text_editor_class =
{
	"span_text_editor",
	span_text_editor_on_recursive_delete,
	span_text_editor_on_recursive_self_test,
	span_text_editor_on_document_event,
	span_text_editor_get_size_requisition,
	span_text_editor_allocate_child_space,
	span_text_editor_recursive_render,
	span_text_editor_on_button_press,
	span_text_editor_on_motion_notify,
	span_text_editor_on_key_press
};

CongTextSpan* cong_text_span_new(int original_first_byte_offset,
				 int stripped_first_byte_offset,
				 int byte_count,
				 CongNodePtr text_node)
{
	CongTextSpan *text_span = g_new0(CongTextSpan,1);
	text_span->original_first_byte_offset = original_first_byte_offset;
	text_span->stripped_first_byte_offset = stripped_first_byte_offset;
	text_span->byte_count = byte_count;
	text_span->text_node = text_node;

	return text_span;
}

/* Strips repeated whitespace from strings; converts all into spaces; adds text spans accordingly to the list: */
gchar* strip_whitespace_from_string(const gchar* input_string,
				    GList **list_of_cong_text_span,
				    int byte_offset_start_of_text,
				    CongNodePtr node)
{
	gunichar *unichar_string;
	glong num_chars;
	gchar *result_string;
	gchar *dst;
	int i;
	gboolean last_char_was_space=FALSE;
	CongTextSpan *text_span;
	int original_byte_offset_start_of_span = 0;
	int stripped_byte_offset_start_of_span = byte_offset_start_of_text;

	g_return_val_if_fail(input_string, NULL);
	g_return_val_if_fail(list_of_cong_text_span, NULL);

	unichar_string = g_utf8_to_ucs4_fast(input_string,
                                             -1,
                                             &num_chars);

	result_string = g_malloc((num_chars*8)+1);

	dst = result_string;

	for (i=0;i<num_chars;i++) {
		gunichar c = unichar_string[i];
		gboolean this_char_is_space = g_unichar_isspace(c);

		if (this_char_is_space) {

			if (!last_char_was_space) {
				
				/* Write a space into the buffer: */
				*(dst++) = ' ';

				/* Add stuff to the list of spans */
				text_span = cong_text_span_new(original_byte_offset_start_of_span,
							       stripped_byte_offset_start_of_span,
							       (i+1-original_byte_offset_start_of_span),
							       node);
				*list_of_cong_text_span = g_list_append(*list_of_cong_text_span, 
									text_span);

			}

		} else {

			if (last_char_was_space) {
				/* We're starting what will be a new span; record where we've got to: */
				original_byte_offset_start_of_span = i;
				stripped_byte_offset_start_of_span = byte_offset_start_of_text + (dst-result_string);
			}

			/* Write character as utf-8 into buffer: */
			dst += g_unichar_to_utf8(c, dst);
		}
		
		last_char_was_space = this_char_is_space;
	}

	g_free(unichar_string);

	if (!last_char_was_space) {
		/* Add stuff to the list of spans */
		text_span = cong_text_span_new(original_byte_offset_start_of_span,
					       stripped_byte_offset_start_of_span,
					       (i-original_byte_offset_start_of_span),
					       node);
		*list_of_cong_text_span = g_list_append(*list_of_cong_text_span, 
							text_span);
	}

	/* Terminate the string: */
	*dst = '\0';

	return result_string;	
}

static void add_text(CongSpanTextEditor *span_text_editor, CongNodePtr node, gboolean strip_whitespace)
{
	gchar *new_value;
	CongTextSpan *text_span;
	gchar *string_to_append;

	g_return_if_fail(span_text_editor);
	g_return_if_fail(node);
	g_return_if_fail(cong_node_type(node)==CONG_NODE_TYPE_TEXT);
	g_return_if_fail(node->content);

	if (strip_whitespace) {
		string_to_append = strip_whitespace_from_string(node->content,
								&span_text_editor->list_of_cong_text_span,
								strlen(span_text_editor->plain_text),
								node);
	} else {
		string_to_append = g_strdup(node->content);

		/* Add stuff to the list of spans */
		text_span = cong_text_span_new(0,
					       strlen(span_text_editor->plain_text),
					       strlen(string_to_append),
					       node);
		span_text_editor->list_of_cong_text_span = g_list_append(span_text_editor->list_of_cong_text_span, text_span);
	}

	
	/* Add to the big plaintext string: */
	new_value = g_strdup_printf("%s%s", span_text_editor->plain_text, string_to_append);

	g_free(string_to_append);	
	g_free(span_text_editor->plain_text);	

	span_text_editor->plain_text = new_value;

}

/**
 * Recursively traverse the node tree, calling add_text on text nodes and registering CongTextRanges for the spans
 */
static void regenerate_plaintext_recursive(CongSpanTextEditor *span_text_editor, CongNodePtr node, int depth, gboolean strip_whitespace)
{
	g_return_if_fail(span_text_editor);

	if (cong_node_type(node)==CONG_NODE_TYPE_TEXT) {
		add_text(span_text_editor,
			 node,
			 strip_whitespace);
	} else {
		CongNodePtr child;

		CongTextRange *text_range = NULL;
		CongDispspecElement* element = cong_dispspec_lookup_node(cong_editor_widget2_get_dispspec(CONG_ELEMENT_EDITOR(span_text_editor)->widget), 
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
			/* We're slightly picky about the recursion; was having lots of problems with entity refs etc: */
			switch (cong_node_type(child)) {
			case CONG_NODE_TYPE_TEXT:
			case CONG_NODE_TYPE_ELEMENT:
				regenerate_plaintext_recursive(span_text_editor, child, (text_range!=NULL)?depth+1:depth, strip_whitespace);
				break;

			default: /* do nothing */
				break;
			}
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
	gboolean strip_whitespace = TRUE; /* for now */

	g_return_if_fail(span_text_editor);


	/* Clean up any existing representation: */
	if (span_text_editor->plain_text) {
		g_free(span_text_editor->plain_text);
	}

	for (list_iter = span_text_editor->list_of_cong_text_span; list_iter; list_iter = list_iter->next) {
		g_free(list_iter->data);
	}
	g_list_free(span_text_editor->list_of_cong_text_span);
	span_text_editor->list_of_cong_text_span = NULL;

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
		g_assert(iter);

		regenerate_plaintext_recursive(span_text_editor, 
					       iter, 
					       0, 
					       strip_whitespace);

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

static CongNodePtr get_node_at_stripped_byte_offset(CongSpanTextEditor *span_text_editor, int byte_offset)
{
	CongTextSpan *text_span;

	g_return_val_if_fail(span_text_editor, NULL);

	text_span = get_text_span_at_stripped_byte_offset(span_text_editor, byte_offset);

	if (text_span) {
		return text_span->text_node;
	} else {
		return NULL;
	}
}

static CongTextSpan *get_text_span_at_stripped_byte_offset(CongSpanTextEditor *span_text_editor, int byte_offset)
{
	GList *iter;

	g_return_val_if_fail(span_text_editor, NULL);
	
	/* Scan through the text spans, looking for the byte offset: */
	for (iter = span_text_editor->list_of_cong_text_span; iter; iter = iter->next) {
		CongTextSpan *text_span = iter->data;
		g_assert(text_span);
		
		g_assert(byte_offset >= text_span->stripped_first_byte_offset);

		if (byte_offset < (text_span->stripped_first_byte_offset + text_span->byte_count) ) {
			return text_span;
		}
	}

	return NULL;
}

static gboolean get_location_at_stripped_byte_offset(CongSpanTextEditor *span_text_editor, int byte_offset, CongLocation *location)
{
	CongTextSpan *text_span;

	g_return_val_if_fail(span_text_editor, FALSE);
	g_return_val_if_fail(location, FALSE);

	text_span = get_text_span_at_stripped_byte_offset(span_text_editor, byte_offset);
	
	if (text_span) {
		location->node = text_span->text_node;
		location->byte_offset = text_span->original_first_byte_offset + (byte_offset - text_span->stripped_first_byte_offset);
		return TRUE;
	}

	return FALSE;
	
}

static gboolean get_stripped_byte_offset_at_location(CongSpanTextEditor *span_text_editor, const CongLocation *location, int *byte_offset)
{
	GList *iter;

	g_return_val_if_fail(span_text_editor, FALSE);
	g_return_val_if_fail(location, FALSE);
	g_return_val_if_fail(byte_offset, FALSE);

	/* Scan through the text spans, looking for the node: */
	for (iter = span_text_editor->list_of_cong_text_span; iter; iter = iter->next) {
		CongTextSpan *text_span = iter->data;
		g_assert(text_span);
		
		if (text_span->text_node==location->node) {
			if (location->byte_offset < text_span->original_first_byte_offset) {
				/* Then the location is between the last span and this span; it's probably a whitespace
				   character that isn't directly represented within the plaintext cache.
				   The trailing character of the last span should be whitespace, so it's a good candidate for the return
				   value:
				*/
				g_assert(iter->prev);
				g_assert(text_span->stripped_first_byte_offset>0);

				*byte_offset = text_span->stripped_first_byte_offset;
				return TRUE;
			}

			if (location->byte_offset < text_span->original_first_byte_offset + text_span->byte_count) {
				/* Found the text span: */
				*byte_offset = text_span->stripped_first_byte_offset + (location->byte_offset - text_span->original_first_byte_offset);
				return TRUE;
			}
		}
	}

	/* Node not found: */
	return FALSE;
}


/* Event handlers: */
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

	switch (event->type) {
	default: break;
	case CONG_DOCUMENT_EVENT_MAKE_ORPHAN:
	case CONG_DOCUMENT_EVENT_ADD_AFTER:
	case CONG_DOCUMENT_EVENT_ADD_BEFORE:
	case CONG_DOCUMENT_EVENT_SET_PARENT:
	case CONG_DOCUMENT_EVENT_SET_TEXT:
		/* These messages go direct from the widget to the relevant element editor: */
		if (!event->before_event) {
			regenerate_plaintext(span_text);			

			/* Ensure layout is regenerated: */
			cong_editor_widget2_force_layout_update(element_editor->widget);

			/* And force a redraw for good measure: */
			gtk_widget_queue_draw(GTK_WIDGET(element_editor->widget));		
			
			return TRUE;
		}
		break;
	}
	return FALSE;	
}

static void span_text_editor_get_size_requisition(CongElementEditor *element_editor, int width_hint)
{
	CongEditorWidget2 *editor_widget = element_editor->widget;
	CongSpanTextEditor *span_text = CONG_SPAN_TEXT_EDITOR(element_editor);

	pango_layout_set_width(span_text->pango_layout,
			       (width_hint-H_SPACING)*PANGO_SCALE);

#if 1
	element_editor->requisition.width = width_hint;
	element_editor->requisition.height = visit_lines(element_editor, CONG_LINE_VISITOR_CALCULATE_HEIGHT, NULL);
#else
	pango_layout_get_pixel_size(span_text->pango_layout,
				    &element_editor->requisition.width,
				    &element_editor->requisition.height);
#endif
}

static void span_text_editor_allocate_child_space(CongElementEditor *element_editor)
{
	CongSpanTextEditor *span_text = CONG_SPAN_TEXT_EDITOR(element_editor);

	pango_layout_set_width(span_text->pango_layout,
			       (element_editor->window_area.width-H_SPACING)*PANGO_SCALE);
}

#if 0
struct RenderAnySelectionData
{
	CongSpanTextEditor *span_text;
	PangoLayoutLine *line;

	const CongSelection *selection;
	GdkGC *gc;
};

static void render_any_selection(CongTextSpan *text_span,
				 struct RenderAnySelectionData *data)
{
	g_assert(text_span);
	g_assert(data);

	/* is this span present on the given line? */
	if (text_span->final_byte_offset < data->line->start_index) {
		/* Reject; the span ends before this line */
		return;
	}
	
	if (text_span->first_byte_offset > data->line->start_index+data->line->length) {
		/* Reject; the span starts after this line */
		return;
	}

	/* OK; at least part of this text_span is present on this line: */
	
#error
	struct CongTextSpan
	{
		int first_byte_offset;
		int byte_count;
		CongNodePtr text_node;
	};

} 
#endif

struct CalculateSpanHeightData
{
	CongSpanTextEditor *span_text;
	PangoLayoutLine *line;

	int max_depth;
};

static void calculate_span_height(CongNodePtr key, 
				  CongTextRange *text_range,
				  struct CalculateSpanHeightData *data)
{
	/* is this range present on the given line? */
	if (text_range->final_byte_offset < data->line->start_index) {
		/* Reject; the range ends before this line */
		return;
	}
	
	if (text_range->first_byte_offset > data->line->start_index+data->line->length) {
		/* Reject; the range starts after this line */
		return;
	}

	/* OK; at least part of this range is present on this line: */
	if (data->max_depth<text_range->depth) {
		data->max_depth = text_range->depth;
	}
}

struct RenderTextRangeData
{
	CongSpanTextEditor *span_text;
	PangoLayoutLine *line;

	int offset_x;
	int offset_y;
	int max_x;
	GdkDrawable *drawable;
	int deepest_depth_for_line;
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
				      start_x, y + 2);
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
				      end_x, y + 2);
		} else {
			/* range finishes somewhere after this line: */
			end_x = data->max_x;
		}

		width = end_x-start_x;

		text_width = cong_font_string_width_slow(span_font, 
							 span_name);
		if (text_width < width - 6) {
			int text_y = y + 2 + cong_font_get_height(span_font, span_name) / 2;
				
			/* Draw text and lines: */
			cong_font_draw_string_slow(data->drawable, 
						   span_font, 
						   gc, 
						   span_name,
						   start_x + 1 + (width - text_width) / 2,
						   y + 2, 
						   CONG_FONT_Y_POS_MIDDLE);
			gdk_draw_line(data->drawable, gc, 
				      start_x, y + 2, 
				      start_x - 1 + (width - text_width) / 2, y + 2);
			gdk_draw_line(data->drawable, gc, 
				      end_x + 1 - (width - text_width) / 2, y + 2, 
				      end_x, y + 2);
		} else {
				/* Draw line: */			
			gdk_draw_line(data->drawable, gc, 
				      start_x, y + 2, 
				      end_x, y + 2);		
		}			
	}
}

#define INTER_LINE_SPACING (8)

/**
 * Return value: total height used
 */
static int visit_lines(CongElementEditor *element_editor, enum CongLineVisitor visitor, struct CongHitTest *hit_test)
{
	CongEditorWidget2 *editor_widget = element_editor->widget;
	CongEditorWidget2Details* details = GET_DETAILS(editor_widget);
	CongSpanTextEditor *span_text = CONG_SPAN_TEXT_EDITOR(element_editor);
	GtkWidget *w = GTK_WIDGET(editor_widget);
	CongDocument *doc = cong_editor_widget2_get_document(editor_widget);
	const CongSelection *selection = cong_document_get_selection(doc);
	const CongCursor *cursor = cong_document_get_cursor(doc);
	gboolean got_selection_start_byte_offset = FALSE;
	gboolean got_selection_end_byte_offset = FALSE;
	gboolean got_cursor_byte_offset = FALSE;
	int selection_start_byte_offset;
	int selection_end_byte_offset;
	int cursor_byte_offset;

	GSList *line_iter;
	GList *text_span_iter = span_text->list_of_cong_text_span;	

	int offset_x = element_editor->window_area.x + H_SPACING;
	int y = element_editor->window_area.y;

	if (cong_location_exists(&selection->loc0) && cong_location_exists(&selection->loc1)) {
		g_assert(cong_node_type(selection->loc0.node)==CONG_NODE_TYPE_TEXT);
		g_assert(cong_node_type(selection->loc1.node)==CONG_NODE_TYPE_TEXT);

		got_selection_start_byte_offset = get_stripped_byte_offset_at_location(span_text, &selection->loc0, &selection_start_byte_offset);
		got_selection_end_byte_offset = get_stripped_byte_offset_at_location(span_text, &selection->loc1, &selection_end_byte_offset);

		if (selection_end_byte_offset<selection_start_byte_offset) {
			int tmp = selection_end_byte_offset;
			selection_end_byte_offset = selection_start_byte_offset;
			selection_start_byte_offset = tmp;
		}
	}

	if (cong_location_exists(&cursor->location)) {
		if (cursor->on) {
			got_cursor_byte_offset = get_stripped_byte_offset_at_location(span_text, &cursor->location, &cursor_byte_offset);
		}
	}

	for (line_iter=pango_layout_get_lines(span_text->pango_layout); line_iter; line_iter = line_iter->next) {
		struct CalculateSpanHeightData calculate_span_height_data;				

		PangoRectangle logical_rect;
		
		PangoLayoutLine *line = line_iter->data;
		g_assert(line);

		pango_layout_line_get_pixel_extents(line,
						    NULL,
						    &logical_rect);
		
#if 0
		CONG_SPAN_TEXT_DEBUG_MSG1("ink_rect: (%i,%i,%i,%i), logical_rect:(%i,%i,%i,%i)",
			  ink_rect.x,
			  ink_rect.y,
			  ink_rect.width,
			  ink_rect.height,
			  logical_rect.x,
			  logical_rect.y,
			  logical_rect.width,
			  logical_rect.height);
#endif

		/* Calculate height of all spans applying to this line: */
		{
			calculate_span_height_data.span_text = span_text;
			calculate_span_height_data.max_depth = 0;
			calculate_span_height_data.line = line;
			
			g_assert(span_text->hash_of_node_to_text_range);
			g_hash_table_foreach(span_text->hash_of_node_to_text_range,
					     (GHFunc)calculate_span_height,
					     &calculate_span_height_data);
		}		

		if (visitor==CONG_LINE_VISITOR_HIT_TEST) {
			int index, trailing;
			g_assert(hit_test);

			if (hit_test->window_coord.y>=y) {
				if (hit_test->window_coord.y< (y + logical_rect.height + INTER_LINE_SPACING + (calculate_span_height_data.max_depth * span_text->tag_height))) {
					if ( pango_layout_line_x_to_index(line,
									  (hit_test->window_coord.x - offset_x) * PANGO_SCALE,
									  &index,
									  &trailing)) {
						hit_test->got_hit = TRUE;
						hit_test->byte_offset = index;
						return 0;
					}
				}
			}
		}
		
		if (visitor==CONG_LINE_VISITOR_RENDER) {
			/* If selection applies, render it underneath the layout_line: */
			if (got_selection_start_byte_offset && got_selection_end_byte_offset) {
				/* Then selection exists within this span_text_editor: */
				GdkGC *selection_gc;
				int start_x, end_x;

				if (cong_location_parent(&selection->loc0) == cong_location_parent(&selection->loc1)) {
					selection_gc = selection->gc_valid;
				} else {
					selection_gc = selection->gc_invalid;
				}

				/* Check that the selection is to be rendered on this layout_line: */
				if (selection_end_byte_offset >= line->start_index) {
					if (selection_start_byte_offset <= line->start_index+line->length) {
						/* OK, render the selection on this layout_line: */
						if (selection_start_byte_offset >= line->start_index) {
							/* selection starts somewhere on this line: */
							pango_layout_line_index_to_x(line,
										     selection_start_byte_offset,
										     FALSE,
										     &start_x);	
							start_x /= PANGO_SCALE;
							start_x += offset_x;
							
						} else {
							/* selection started somewhere before this line: */
							start_x = element_editor->window_area.x + H_SPACING;
						}
						
						if (selection_end_byte_offset <= line->start_index+line->length) {
							/* selection finishes somewhere on this line: */
							pango_layout_line_index_to_x(line,
										     selection_end_byte_offset,
										     TRUE,
										     &end_x);	
							end_x /= PANGO_SCALE;
							end_x += offset_x;
						} else {
							/* selection finishes somewhere after this line: */
							end_x = element_editor->window_area.x + element_editor->window_area.width;
						}
						
						/* Render it: */
						gdk_draw_rectangle(GDK_DRAWABLE(w->window),
								   selection_gc, 
								   TRUE,
								   start_x,
								   y,
								   end_x-start_x,
								   logical_rect.height + INTER_LINE_SPACING + (calculate_span_height_data.max_depth * span_text->tag_height));
					}
				}
			}

			/* Render the cursor under the text: */
			#if 0
			CONG_SPAN_TEXT_DEBUG_MSG1("rendering cursor");
			#endif
						
			if (got_cursor_byte_offset) {
				
				#if 0
				CONG_SPAN_TEXT_DEBUG_MSG1("got cursor byte offset");
				#endif

				if (cursor_byte_offset >= line->start_index) {
					if (cursor_byte_offset < line->start_index+line->length) {
						int cursor_x;
						
						pango_layout_line_index_to_x(line,
									     cursor_byte_offset,
									     FALSE,
									     &cursor_x);	
						cursor_x /= PANGO_SCALE;
						cursor_x += offset_x;

						/* Render it: */
						gdk_draw_line(GDK_DRAWABLE(w->window), 
							      cursor->gc, 
							      cursor_x, y,
							      cursor_x, y + logical_rect.height);
						
					}
				}
			}
			
			/* Render the PangoLayoutLine: */
			gdk_draw_layout_line(GDK_DRAWABLE(w->window),
					     w->style->black_gc,
					     offset_x,
					     (y - logical_rect.y),
					     line);


		}

		/* Render spans: */
		if (visitor==CONG_LINE_VISITOR_RENDER) {
			struct RenderTextRangeData render_text_range_data;
			render_text_range_data.span_text = span_text;
			render_text_range_data.line = line;
			render_text_range_data.offset_x = offset_x;
			render_text_range_data.offset_y = y + logical_rect.height + (calculate_span_height_data.max_depth * span_text->tag_height);
			render_text_range_data.max_x = element_editor->window_area.x + element_editor->window_area.width;
			render_text_range_data.drawable = GDK_DRAWABLE(w->window);				
			
			g_assert(span_text->hash_of_node_to_text_range);
			g_hash_table_foreach(span_text->hash_of_node_to_text_range,
					     (GHFunc)render_text_range,
					     &render_text_range_data);
		}
		
		y += logical_rect.height + INTER_LINE_SPACING + (calculate_span_height_data.max_depth * span_text->tag_height);
	}

	return y - element_editor->window_area.y;
}

static void span_text_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect)
{
	CongEditorWidget2 *editor_widget = element_editor->widget;
	GtkWidget *w = GTK_WIDGET(editor_widget);

	GdkRectangle intersected_area;

	g_return_if_fail(editor_widget);
	g_return_if_fail(window_rect);

	/* Early accept/reject against the areas: */
	if (gdk_rectangle_intersect((GdkRectangle*)window_rect,
				     &element_editor->window_area,
				     &intersected_area)) {
		
#if 1
		visit_lines(element_editor, CONG_LINE_VISITOR_RENDER, NULL);
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

static gboolean do_hit_test(CongSpanTextEditor *span_text_editor, int window_x, int window_y, int *stripped_byte_offset)
{
	struct CongHitTest hit_test;

	g_return_val_if_fail(span_text_editor, FALSE);
	g_return_val_if_fail(stripped_byte_offset, FALSE);

	hit_test.window_coord.x = window_x;
	hit_test.window_coord.y = window_y;
	hit_test.got_hit = FALSE;
				
	visit_lines(CONG_ELEMENT_EDITOR(span_text_editor), CONG_LINE_VISITOR_HIT_TEST, &hit_test);

	if (hit_test.got_hit) {
		*stripped_byte_offset = hit_test.byte_offset;
		return TRUE;
	} else {
		return FALSE;
	}
	
}

static gboolean get_click_location(CongSpanTextEditor *span_text_editor, int window_x, int window_y, CongLocation *result)
{
	int stripped_byte_offset;

	g_return_val_if_fail(result, FALSE);

	if (do_hit_test(span_text_editor, window_x, window_y, &stripped_byte_offset)) {
		CongTextSpan *text_span = get_text_span_at_stripped_byte_offset(span_text_editor, stripped_byte_offset);
		
		if (text_span) {
			cong_location_set_node_and_byte_offset(result,
							       text_span->text_node,
							       text_span->original_first_byte_offset + (stripped_byte_offset - text_span->stripped_first_byte_offset));
			return TRUE;
			
		}
	}

	return FALSE;
}

static void span_text_editor_on_button_press(CongElementEditor *element_editor, GdkEventButton *event)
{
	CongDocument *doc;
	CongCursor *cursor;
	CongSelection *selection;

	CongEditorWidget2 *editor_widget = element_editor->widget;
	CongSpanTextEditor *span_text_editor = CONG_SPAN_TEXT_EDITOR(element_editor);

	GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(editor_widget)));

	/* FIXME: unimplemented */
	CONG_SPAN_TEXT_DEBUG_MSG1("span_text_editor_on_button_press");

	doc = cong_editor_widget2_get_document(editor_widget);
	cursor = cong_document_get_cursor(doc);
	selection = cong_document_get_selection(doc);
	
	if (event->button == 1) {
		switch (event->type) {
		default: return; /* do nothing */
#if 0
		case GDK_3BUTTON_PRESS:
			/* Handle triple-click by locating the tag containing the click location, 
			   making that the selection, with the cursor at the end of it? */
			{
				CongLocation click_location;
				CongLocation start_of_tag;
				CongLocation end_of_tag;

				if (get_click_location(span_text_editor, event->x, event->y, &click_location)) {
					cong_location_calc_tag_start(&click_location, &tag_start);
					cong_location_calc_tag_end(&click_location, &tag_end);
						
					cong_location_copy(&selection->loc0, &start_of_tag);
					cong_location_copy(&selection->loc1, &end_of_tag);
					cong_location_copy(&cursor->location, &end_of_tag);
				
					cong_document_on_selection_change(doc);
					cong_document_on_cursor_change(doc);
				}
			}
			return;
#endif

		case GDK_2BUTTON_PRESS:
			/* Handle double-click by locating the word containing the click location, 
			   making that the selection, with the cursor at the end of it. */
			{
				CongLocation click_location;
				CongLocation start_of_word;
				CongLocation end_of_word;

				if (get_click_location(span_text_editor, event->x, event->y, &click_location)) {
					if (cong_location_calc_word_extent(&click_location, doc, &start_of_word, &end_of_word)) {
						
						cong_location_copy(&selection->loc0, &start_of_word);
						cong_location_copy(&selection->loc1, &end_of_word);
						cong_location_copy(&cursor->location, &end_of_word);
					}						
				
					cong_document_on_selection_change(doc);
					cong_document_on_cursor_change(doc);
				}
			}
			return;

		case GDK_BUTTON_PRESS:
			/* Handle single-click by moving the cursor and selection to the location of the click: */
			{
				gtk_widget_grab_focus(GTK_WIDGET(editor_widget));
				gtk_widget_grab_default(GTK_WIDGET(editor_widget));

				if (get_click_location(span_text_editor, event->x, event->y, &cursor->location)) {
					
					cong_selection_start_from_curs(selection, cursor);
					cong_selection_end_from_curs(selection, cursor);
					cong_document_on_selection_change(doc);
					cong_document_on_cursor_change(doc);
						
				}
			}

			return;
		}
	} else if (event->button == 3) {

		editor_popup_build(doc, parent_window);
		editor_popup_show(cong_app_singleton()->popup, event);

	}
	
}

static void span_text_editor_on_motion_notify(CongElementEditor *element_editor, GdkEventMotion *event)
{
	struct CongHitTest hit_test;

	CongDocument *doc;
	CongCursor *cursor;
	CongSelection *selection;

	CongEditorWidget2 *editor_widget = element_editor->widget;
	CongSpanTextEditor *span_text_editor = CONG_SPAN_TEXT_EDITOR(element_editor);

	CONG_SPAN_TEXT_DEBUG_MSG1("span_text_editor_on_motion_notify");

	if (!(event->state & GDK_BUTTON1_MASK)) return;

	doc = cong_editor_widget2_get_document(editor_widget);
	cursor = cong_document_get_cursor(doc);
	selection = cong_document_get_selection(doc);
	

	hit_test.window_coord.x = event->x;
	hit_test.window_coord.y = event->y;
	hit_test.got_hit = FALSE;

	visit_lines(element_editor, CONG_LINE_VISITOR_HIT_TEST, &hit_test);

	if (hit_test.got_hit) {
		CongTextSpan *text_span = get_text_span_at_stripped_byte_offset(span_text_editor, hit_test.byte_offset);
		
		if (text_span) {
			cong_location_set_node_and_byte_offset(&cursor->location,
							       text_span->text_node,
							       text_span->original_first_byte_offset + (hit_test.byte_offset - text_span->stripped_first_byte_offset));
			
			cong_selection_end_from_curs(selection, cursor);
			cong_document_on_selection_change(doc);
			cong_document_on_cursor_change(doc);			
			return;
		}
	}
}

static gboolean span_text_editor_calc_up(CongSpanTextEditor *span_text_editor, 
					 CongDocument *doc, 
					 CongCursor *cursor,
					 CongLocation *output_loc)
{
	int byte_offset;
	GSList *line_iter;
	PangoLayoutLine *prev_line=NULL;

	g_assert(span_text_editor);
	g_assert(cursor);

	if (get_stripped_byte_offset_at_location(span_text_editor, &cursor->location, &byte_offset)) {
		for (line_iter=pango_layout_get_lines(span_text_editor->pango_layout); line_iter; line_iter = line_iter->next) {
			PangoLayoutLine *line = line_iter->data;
			g_assert(line);
			
			if ( (byte_offset >= line->start_index) && (byte_offset < line->start_index+line->length)) {
				/* Then the search place is on this line; check to see that we're not on the first line: */
				if (prev_line) {
					int x;
					int index;
					int trailing;
					
					pango_layout_line_index_to_x(line,
								     byte_offset,
								     FALSE,
								     &x);	
					
					pango_layout_line_x_to_index(prev_line,
								     x,
								     &index,
								     &trailing);


					if (get_location_at_stripped_byte_offset(span_text_editor, index, output_loc)) {
						return TRUE;
					}					
				}
			}
			
			prev_line = line;
		}
	}

	return FALSE;
}

static gboolean span_text_editor_calc_down(CongSpanTextEditor *span_text_editor, 
					   CongDocument *doc, 
					   CongCursor *cursor,
					   CongLocation *output_loc)
{
	int byte_offset;
	GSList *line_iter;

	g_assert(span_text_editor);
	g_assert(cursor);

	if (get_stripped_byte_offset_at_location(span_text_editor, &cursor->location, &byte_offset)) {
		for (line_iter=pango_layout_get_lines(span_text_editor->pango_layout); line_iter; line_iter = line_iter->next) {
			PangoLayoutLine *line = line_iter->data;
			g_assert(line);
			
			if ( (byte_offset >= line->start_index) && (byte_offset < line->start_index+line->length)) {
				/* Then the search place is on this line; check to see that we're not on the last line: */
				if (line_iter->next) {
					int x;
					int index;
					int trailing;
					
					pango_layout_line_index_to_x(line,
								     byte_offset,
								     FALSE,
								     &x);	
					
					pango_layout_line_x_to_index((PangoLayoutLine*)line_iter->next->data,
								     x,
								     &index,
								     &trailing);


					if (get_location_at_stripped_byte_offset(span_text_editor, index, output_loc)) {
						return TRUE;
					}					
				}
			}
		}
	}

	return FALSE;
}

/* 
   Method to calculate where the cursor should go as a result of the key press.
   Affected by the CTRL key (which means "whole words" rather than "individual characters" for left/right).

   Return value:  TRUE iff the output_loc has been written to with a meaningful location different from the cursor location.
*/
gboolean span_text_editor_get_destination_location_for_keypress(CongSpanTextEditor *span_text_editor, 
								CongDocument *doc, 
								CongCursor *cursor, 
								guint state,
								guint keyval,
								CongLocation *output_loc)
{
	CongDispspec *dispspec;

	g_return_val_if_fail(span_text_editor, FALSE);
	g_return_val_if_fail(doc, FALSE);
	g_return_val_if_fail(cursor, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	dispspec = cong_document_get_dispspec(doc);

	switch (keyval) {
	default: 
		return FALSE;

	case GDK_Up:
		return span_text_editor_calc_up(span_text_editor, doc, cursor, output_loc);

	case GDK_Down:
		return span_text_editor_calc_down(span_text_editor, doc, cursor, output_loc);
	
	case GDK_Left:
		if (state & GDK_CONTROL_MASK) {
			return cong_location_calc_prev_word(&cursor->location, doc, output_loc);
		} else {
			return cong_location_calc_prev_char(&cursor->location, dispspec, output_loc);
		}
	
	case GDK_Right:
		if (state & GDK_CONTROL_MASK) {
			return cong_location_calc_next_word(&cursor->location, doc, output_loc);
		} else {
			return cong_location_calc_next_char(&cursor->location, dispspec, output_loc);
		}
	case GDK_Home:
		if (state & GDK_CONTROL_MASK) {
			return cong_location_calc_document_start(&cursor->location, dispspec, output_loc);
		} else {
			return cong_location_calc_line_start(&cursor->location, dispspec, output_loc);
		}
	case GDK_End:
		if (state & GDK_CONTROL_MASK) {
			return cong_location_calc_document_end(&cursor->location, dispspec, output_loc);
		} else {
			return cong_location_calc_line_end(&cursor->location, dispspec, output_loc);
		}
	case GDK_Page_Up:
		return cong_location_calc_prev_page(&cursor->location, dispspec, output_loc);
	case GDK_Page_Down:
		return cong_location_calc_next_page(&cursor->location, dispspec, output_loc);
	}
}

static void span_text_editor_on_key_press(CongElementEditor *element_editor, GdkEventKey *event)
{
	CongDocument *doc;
	CongCursor *cursor;
	CongSelection *selection;

	CongEditorWidget2 *editor_widget = element_editor->widget;
	CongSpanTextEditor *span_text_editor = CONG_SPAN_TEXT_EDITOR(element_editor);

	CONG_SPAN_TEXT_DEBUG_MSG1("span_text_editor_on_key_press");

#ifndef RELEASE		
	printf("Keyval: %d, State: %d\n", event->keyval, event->state);
#endif
	
	doc = cong_editor_widget2_get_document(editor_widget);
	cursor = cong_document_get_cursor(doc);
	selection = cong_document_get_selection(doc);
	g_assert(selection);

	switch (event->keyval)
	{
	case GDK_Up:
	case GDK_Down:
	case GDK_Left:
	case GDK_Right:
	case GDK_Home:
	case GDK_End:
	case GDK_Page_Up:
	case GDK_Page_Down:
		{
			CongLocation target_location;

			/* Calculate whereabouts in the document the user wants to go: */

			if (span_text_editor_get_destination_location_for_keypress(span_text_editor, 
										   doc, 
										   cursor, 
										   event->state,
										   event->keyval,
										   &target_location)) {
				/* Are we moving the cursor, or dragging out a selection? */

				/* Move the cursor to the new location: */
				cong_location_copy(&cursor->location, &target_location);

				if (event->state & GDK_SHIFT_MASK) {
					/* Then we should also drag out the selection to the new location: */
					cong_selection_end_from_curs(selection, cursor);
				} else {
					/* Then we should clear any selection that exists: */
					cong_selection_start_from_curs(selection, cursor);
				}

				cong_document_on_cursor_change(doc);
			}
		}
		break;
	
#if 0
	case GDK_BackSpace:
		if (selection->loc0.node) {
			cong_document_delete_selection(doc);
		} else {
			cong_cursor_del_prev_char(cursor, doc);
		}
		break;
	
	case GDK_Delete:
		if (selection->loc0.node) {
			cong_document_delete_selection(doc);
		} else {
			cong_cursor_del_next_char(cursor, doc);
		}
		break;
#else
	case GDK_BackSpace:
		cong_cursor_del_prev_char(cursor, doc);
		break;
	
	case GDK_Delete:
		cong_cursor_del_next_char(cursor, doc);
		break;
#endif

	case GDK_ISO_Enter:
	case GDK_Return:
		cong_cursor_paragraph_insert(cursor);
		break;

	case GDK_Tab:
		/* Ignore the tab key for now... FIXME: what should we do? */
		break;
	
	default:
		/* Is the user typing text? */
		if (event->length && event->string && strlen(event->string)) {
			cong_cursor_data_insert(cursor, event->string);
		}
		break;
	}

	cong_document_on_cursor_change(doc);	

}

/* Public API: */
CongElementEditor *cong_span_text_editor_new(CongEditorWidget2 *widget, CongNodePtr first_node, CongNodePtr final_node)
{
	CongSpanTextEditor *span_text;

	CongFont *body_font;
	CongFont *span_font;

	g_return_val_if_fail(widget, NULL);
	g_return_val_if_fail(first_node, NULL);
	g_return_val_if_fail(final_node, NULL);
	g_return_val_if_fail( (first_node->parent == final_node->parent), NULL);

	CONG_SPAN_TEXT_DEBUG_MSG1("cong_span_text_editor_new");

	span_text = g_new0(CongSpanTextEditor,1);
	span_text->element_editor.klass = &span_text_editor_class;
	span_text->element_editor.widget = widget;
	span_text->element_editor.first_node = first_node;
	span_text->element_editor.final_node = final_node;

	cong_editor_widget2_register_element_editor(widget, CONG_ELEMENT_EDITOR(span_text));

	span_text->pango_layout = pango_layout_new(gdk_pango_context_get() /*cong_app_singleton()->pango_context*/);
	g_assert(span_text->pango_layout);

	body_font = cong_span_text_editor_get_font(span_text, CONG_FONT_ROLE_BODY_TEXT);
	g_assert(body_font);

	pango_layout_set_font_description(span_text->pango_layout,
					  cong_font_get_pango_description(body_font));

	span_font = cong_span_text_editor_get_font(span_text, CONG_FONT_ROLE_SPAN_TAG);
	g_assert(span_font);

	span_text->tag_height = cong_font_get_height(span_font, "fubar") / 2;
	if (span_text->tag_height < 3) span_text->tag_height = 3;
	span_text->tag_height += cong_font_get_height(span_font, "fubar") / 2;

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
	return cong_app_singleton()->fonts[role];
}
