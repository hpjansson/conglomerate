/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node-text.c
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
#include "cong-editor-node-text.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "cong-app.h"
#include "cong-editor-area-text.h"
#include "cong-text-cache.h"
#include "cong-document.h"
#include "cong-editor-line-fragments.h"
#include "cong-font.h"
#include "cong-editor-area-text-fragment.h"
#include "cong-selection.h"
#include "cong-dispspec.h"

#define PRIVATE(x) ((x)->private)

typedef struct CongEditorNodeTextSelectionState CongEditorNodeTextSelectionState;

struct CongEditorNodeTextSelectionState
{
	gboolean is_selection_valid; /* is the nesting correct? */
	gboolean got_selection_start;
	gboolean got_selection_end;
	gint selection_start_original;
	gint selection_end_original;
	gint selection_start_stripped;
	gint selection_end_stripped;
};

struct CongEditorNodeTextDetails
{
	CongTextCache* text_cache;
	
	gboolean selection_state_valid;
	CongEditorNodeTextSelectionState cached_selection_state;
	
	gulong handler_id_node_set_text;
	gulong handler_id_selection_change;

	PangoLayout *pango_layout;
	GList *list_of_text_fragments;
};

static void
finalize (GObject *object);

static void
dispose (GObject *object);

static CongEditorArea*
generate_block_area (CongEditorNode *editor_node);

static CongEditorLineFragments*
generate_line_areas_recursive (CongEditorNode *editor_node,
			       gint line_width,
			       gint initial_indent);

static enum CongFlowType
get_flow_type (CongEditorNode *editor_node);

/* FIXME:  We probably shouldn't have every text node in the doc listening to every text node change... probably should allow for a dispatch mechanism within the widget */
/* Declarations of the CongDocument event handlers: */
static void 
on_signal_set_text_notify_after (CongDocument *doc, 
				 CongNodePtr node, 
				 const xmlChar *new_content, 
				 gpointer user_data);

static void 
on_signal_selection_change_notify_after (CongDocument *doc, 
					 gpointer user_data);

/* Declarations of the CongEditorArea event handlers for the block area: */
static gboolean
on_signal_button_press (CongEditorArea *editor_area, 
			GdkEventButton *event,
			gpointer user_data);
static gboolean
on_signal_motion_notify (CongEditorArea *editor_area, 
			 GdkEventMotion *event,
			 gpointer user_data);

/* Internal utilities: */
static const gchar*
get_text_cache_input (CongEditorNodeText *editor_node_text);

/* Returns TRUE if anything has changed */
static gboolean
regenerate_selection_state (CongEditorNodeText *editor_node_text);

static gchar*
generate_markup (CongEditorNodeText *editor_node_text);


/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeText, 
			cong_editor_node_text,
			CongEditorNode,
			CONG_EDITOR_NODE_TYPE );

static void
cong_editor_node_text_class_init (CongEditorNodeTextClass *klass)
{
	CongEditorNodeClass *node_klass = CONG_EDITOR_NODE_CLASS(klass);

	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	node_klass->generate_block_area = generate_block_area;
	node_klass->generate_line_areas_recursive = generate_line_areas_recursive;
	node_klass->get_flow_type = get_flow_type;
}

static void
cong_editor_node_text_instance_init (CongEditorNodeText *node_text)
{
	node_text->private = g_new0(CongEditorNodeTextDetails,1);
}

static void
finalize (GObject *object)
{
	CongEditorNodeText *editor_node_text = CONG_EDITOR_NODE_TEXT(object);

	g_free (editor_node_text->private);
	editor_node_text->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

CongEditorNodeText*
cong_editor_node_text_construct (CongEditorNodeText *editor_node_text,
				 CongEditorWidget3* editor_widget,
				 CongNodePtr node,
				 CongEditorNode *traversal_parent)
{
	gchar *markup;	
	enum CongWhitespaceHandling whitespace = cong_node_get_whitespace_handling (cong_editor_widget3_get_document (editor_widget),
										    node);

	cong_editor_node_construct (CONG_EDITOR_NODE (editor_node_text),
				    editor_widget,
				    node,
				    traversal_parent);


	PRIVATE(editor_node_text)->text_cache = cong_text_cache_new ((whitespace==CONG_WHITESPACE_NORMALIZE),
								     get_text_cache_input (editor_node_text));

	PRIVATE(editor_node_text)->handler_id_node_set_text = g_signal_connect_after (G_OBJECT(cong_editor_widget3_get_document(editor_widget)), 
										      "node_set_text",
										      G_CALLBACK(on_signal_set_text_notify_after),
										      editor_node_text);

	PRIVATE(editor_node_text)->handler_id_selection_change = g_signal_connect_after (G_OBJECT(cong_editor_widget3_get_document(editor_widget)), 
											 "selection_change",
											 G_CALLBACK(on_signal_selection_change_notify_after),
											 editor_node_text);

	/* Set up our Pango Layout: */
	PRIVATE(editor_node_text)->pango_layout = pango_layout_new(gtk_widget_get_pango_context (GTK_WIDGET(editor_widget)));
	
	regenerate_selection_state (editor_node_text);

	markup = generate_markup (editor_node_text);

	pango_layout_set_markup (PRIVATE(editor_node_text)->pango_layout,
				 markup,
				 -1);
	g_free (markup);

	pango_layout_set_font_description (PRIVATE(editor_node_text)->pango_layout,
					   cong_font_get_pango_description(cong_app_singleton()->fonts[CONG_FONT_ROLE_BODY_TEXT]));
	
	return editor_node_text;
}

static void
dispose (GObject *object)
{
	CongEditorNodeText *editor_node_text = CONG_EDITOR_NODE_TEXT(object);

	g_signal_handler_disconnect (G_OBJECT(cong_editor_node_get_document(CONG_EDITOR_NODE(object))),
				     PRIVATE(editor_node_text)->handler_id_node_set_text);	
	g_signal_handler_disconnect (G_OBJECT(cong_editor_node_get_document(CONG_EDITOR_NODE(object))),
				     PRIVATE(editor_node_text)->handler_id_selection_change);	

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}


CongEditorNode*
cong_editor_node_text_new (CongEditorWidget3 *widget,
			   CongNodePtr node,
			   CongEditorNode *traversal_parent)
{
#if DEBUG_EDITOR_NODE_LIFETIMES
	g_message("cong_editor_node_text_new(%s)", node->content);
#endif

	return CONG_EDITOR_NODE( cong_editor_node_text_construct (g_object_new (CONG_EDITOR_NODE_TEXT_TYPE, NULL),
								  widget,
								  node,
								  traversal_parent)
				 );
}

gboolean
cong_editor_node_text_convert_original_byte_offset_to_stripped (CongEditorNodeText *editor_node_text,
								int original_byte_offset,
								int *stripped_byte_offset)
{
	return cong_text_cache_convert_original_byte_offset_to_stripped (PRIVATE(editor_node_text)->text_cache,
									 original_byte_offset,
									 stripped_byte_offset);
}

gboolean
cong_eel_pango_layout_calc_up (PangoLayout *layout,
			       int input_index,
			       int *output_index)
{
	GSList *line_iter;
	PangoLayoutLine *prev_line=NULL;

	g_return_val_if_fail (layout, FALSE);
	g_return_val_if_fail (output_index, FALSE);

	for (line_iter=pango_layout_get_lines(layout); line_iter; line_iter = line_iter->next) {
		PangoLayoutLine *line = line_iter->data;
		g_assert(line);
		
		if ( (input_index >= line->start_index) && (input_index < line->start_index+line->length)) {
			/* Then the search place is on this line; check to see that we're not on the first line: */
			if (prev_line) {
				int x;
				int trailing;
				
				pango_layout_line_index_to_x(line,
							     input_index,
							     FALSE,
							     &x);	
				
				pango_layout_line_x_to_index(prev_line,
							     x,
							     output_index,
							     &trailing);
				
				return TRUE;
			}
		}
		
		prev_line = line;
	}

	return FALSE;
}

gboolean
cong_eel_pango_layout_calc_down (PangoLayout *layout,
				 int input_index,
				 int *output_index)
{
	GSList *line_iter;

	g_return_val_if_fail (layout, FALSE);
	g_return_val_if_fail (output_index, FALSE);

	for (line_iter=pango_layout_get_lines(layout); line_iter; line_iter = line_iter->next) {
		PangoLayoutLine *line = line_iter->data;
		g_assert(line);
		
		if ( (input_index >= line->start_index) && (input_index < line->start_index+line->length)) {
			/* Then the search place is on this line; check to see that we're not on the last line: */
			if (line_iter->next) {
				int x;
				int trailing;
				
				pango_layout_line_index_to_x(line,
							     input_index,
							     FALSE,
							     &x);	
				
				pango_layout_line_x_to_index((PangoLayoutLine*)line_iter->next->data,
							     x,
							     output_index,
							     &trailing);
				
				return TRUE;
			}
		}
	}

	return FALSE;
}

gboolean
cong_editor_node_text_calc_up (CongEditorNodeText *editor_node_text,
			       int input_byte_offset,
			       int* output_byte_offset)
{

	int input_stripped_byte_offset;
	int result_stripped_byte_offset;

	g_return_val_if_fail (IS_CONG_EDITOR_NODE_TEXT (editor_node_text), FALSE);
	g_return_val_if_fail (output_byte_offset, FALSE);

	if (!cong_text_cache_convert_original_byte_offset_to_stripped (PRIVATE(editor_node_text)->text_cache,
								      input_byte_offset,
								      &input_stripped_byte_offset)) {
		return FALSE;		
	}
	       
	if (!cong_eel_pango_layout_calc_up (PRIVATE(editor_node_text)->pango_layout,
					    input_stripped_byte_offset,
					    &result_stripped_byte_offset)) {
		return FALSE;		
	}

	return cong_text_cache_convert_stripped_byte_offset_to_original (PRIVATE(editor_node_text)->text_cache,
									 result_stripped_byte_offset,
									 output_byte_offset);

}

gboolean
cong_editor_node_text_calc_down (CongEditorNodeText *editor_node_text,
				 int input_byte_offset,
				 int* output_byte_offset)
{
	int input_stripped_byte_offset;
	int result_stripped_byte_offset;

	g_return_val_if_fail (IS_CONG_EDITOR_NODE_TEXT (editor_node_text), FALSE);
	g_return_val_if_fail (output_byte_offset, FALSE);

	if (!cong_text_cache_convert_original_byte_offset_to_stripped (PRIVATE(editor_node_text)->text_cache,
								      input_byte_offset,
								      &input_stripped_byte_offset)) {
		return FALSE;		
	}

	if (!cong_eel_pango_layout_calc_down (PRIVATE(editor_node_text)->pango_layout,
					      input_stripped_byte_offset,
					      &result_stripped_byte_offset)) {
		return FALSE;		
	}

	return cong_text_cache_convert_stripped_byte_offset_to_original (PRIVATE(editor_node_text)->text_cache,
									 result_stripped_byte_offset,
									 output_byte_offset);
}


static CongEditorArea*
generate_block_area (CongEditorNode *editor_node)
{

#if 1
	g_assert_not_reached();
	return NULL;
#else
	CongEditorNodeText *node_text = CONG_EDITOR_NODE_TEXT(editor_node);

	g_return_val_if_fail (editor_node, NULL);

	PRIVATE(node_text)->area_text = 
		CONG_EDITOR_AREA_TEXT( cong_editor_area_text_new (cong_editor_node_get_widget (editor_node),
								  cong_app_singleton()->fonts[CONG_FONT_ROLE_BODY_TEXT], 
								  NULL,
								  cong_text_cache_get_text (PRIVATE(node_text)->text_cache),
								  FALSE)
				       );

	g_signal_connect (PRIVATE(node_text)->area_text,
			  "button_press_event",
			  G_CALLBACK(on_signal_block_button_press),
			  editor_node);

	g_signal_connect (PRIVATE(node_text)->area_text,
			  "motion_notify_event",
			  G_CALLBACK(on_signal_block_motion_notify),
			  editor_node);

#if 0
	g_signal_connect (PRIVATE(node_text)->area_text,
			  "key_press_event",
			  G_CALLBACK(on_signal_block_key_press),
			  editor_node);
#endif

	return CONG_EDITOR_AREA(PRIVATE(node_text)->area_text);
#endif
	
}

#if 1
gboolean
cong_selection_get_start_byte_offset (CongSelection *selection, 
				      CongNodePtr node,
				      gint *output);
gboolean
cong_selection_get_end_byte_offset (CongSelection *selection, 
				    CongNodePtr node,
				    gint *output);
gboolean
cong_selection_get_start_byte_offset (CongSelection *selection, 
				      CongNodePtr node,
				      gint *output)
{
	CongLocation* loc = cong_selection_get_ordered_start (selection);

	if (NULL==loc->node) {
		return FALSE;
	}

	if (loc->node == node) {
		*output = loc->byte_offset;
		return TRUE;

	} else {
		return FALSE; /* for now */ 
	}

}
gboolean
cong_selection_get_end_byte_offset (CongSelection *selection, 
				    CongNodePtr node,
				    gint *output)
{
	CongLocation* loc = cong_selection_get_ordered_end (selection);

	if (NULL==loc->node) {
		return FALSE;
	}

	if (loc->node == node) {
		*output = loc->byte_offset;
		return TRUE;

	} else {
		return FALSE; /* for now */ 
	}

}
#endif

static CongEditorLineFragments*
generate_line_areas_recursive (CongEditorNode *editor_node,
			       gint line_width,
			       gint initial_indent)
{
	CongEditorLineFragments *result;
	CongEditorNodeText *node_text = CONG_EDITOR_NODE_TEXT(editor_node);

#if 0
	g_message("CongEditorNodeText::generate_line_areas_recursive, cached text =\"%s\"", 
		  cong_text_cache_get_text (PRIVATE(node_text)->text_cache));
#endif

	result = cong_editor_line_fragments_new();

#if 1
	/* Set the "geometry" of the PangoLayout: */
	pango_layout_set_width (PRIVATE(node_text)->pango_layout,
				line_width*PANGO_SCALE);

	pango_layout_set_indent (PRIVATE(node_text)->pango_layout,
				initial_indent*PANGO_SCALE);

	/* Add areas for the PangoLayoutLines: */
	{
		GSList* iter;
		int index;
		PangoLayoutIter* layout_iter = pango_layout_get_iter (PRIVATE(node_text)->pango_layout);


		/* CAUTION: this is internal data of the PangoLayout */
		for (index=0; index<pango_layout_get_line_count(PRIVATE(node_text)->pango_layout); index++, pango_layout_iter_next_line(layout_iter)) {
			PangoLayoutLine *line = pango_layout_iter_get_line (layout_iter);
			gchar *line_text;
			CongEditorArea *text_fragment;
			PangoRectangle ink_rect;
			PangoRectangle logical_rect;
			
			g_assert(line);

			pango_layout_line_get_pixel_extents (line,
							     &ink_rect,
							     &logical_rect);

#if 0
			g_message("baseline = %i, logical rect.y = %i",
				  (pango_layout_iter_get_baseline(layout_iter)/PANGO_SCALE),
				  logical_rect.y);
#endif

			text_fragment = cong_editor_area_text_fragment_new (cong_editor_node_get_widget (editor_node),
									    node_text,
									    PRIVATE(node_text)->pango_layout,
									    index,
									    - logical_rect.y);
			/* FIXME: the calculation of how to offset each baseline is a hack, and might break */

			g_signal_connect (text_fragment,
					  "button_press_event",
					  G_CALLBACK(on_signal_button_press),
					  editor_node);
			
			g_signal_connect (text_fragment,
					  "motion_notify_event",
					  G_CALLBACK(on_signal_motion_notify),
					  editor_node);
			
#if 0
			g_signal_connect (text_fragment,
					  "key_press_event",
					  G_CALLBACK(on_signal_key_press),
					  editor_node);
#endif

			cong_editor_line_fragments_add_area (result,
							     text_fragment);


		}

		pango_layout_iter_free (layout_iter);
			     
	}
#else
	{
		g_message("TEST: generating 1 line");

		cong_editor_line_fragments_add_area (result,
						     cong_editor_area_text_new (cong_editor_node_get_widget (editor_node),
										cong_app_singleton()->fonts[CONG_FONT_ROLE_TITLE_TEXT],
										NULL,
										g_strndup (cong_text_cache_get_text (PRIVATE(node_text)->text_cache),
											   20),
										FALSE)
						     );
	}
#endif

	return result;
}

/* Definitions of the CongDocument event handlers: */
static void 
on_signal_set_text_notify_after (CongDocument *doc, 
				 CongNodePtr node, 
				 const xmlChar *new_content, 
				 gpointer user_data)
{
	CongEditorNodeText *editor_node_text = (CongEditorNodeText*)user_data;
	
	g_return_if_fail (IS_CONG_EDITOR_NODE_TEXT(editor_node_text));

	/* FIXME: need smarter dispatch mechanism: */
	if (node == cong_editor_node_get_node( CONG_EDITOR_NODE(editor_node_text))) {

		gchar *markup;
	
		cong_text_cache_set_text (PRIVATE(editor_node_text)->text_cache,
					  get_text_cache_input (editor_node_text));

		
		PRIVATE(editor_node_text)->selection_state_valid = FALSE;
		regenerate_selection_state (editor_node_text);

		markup = generate_markup (editor_node_text);

		pango_layout_set_markup (PRIVATE(editor_node_text)->pango_layout,
					 markup,
					 -1);

		g_free (markup);

		cong_editor_node_line_regeneration_required (CONG_EDITOR_NODE(editor_node_text));

#if 0
		if (PRIVATE(editor_node_text)->area_text) {
			cong_editor_area_text_set_text (PRIVATE(editor_node_text)->area_text,
							cong_text_cache_get_text(PRIVATE(editor_node_text)->text_cache));
		}
#endif
	}
}

static void 
on_signal_selection_change_notify_after (CongDocument *doc, 
					 gpointer user_data)
{
	CongEditorNodeText *editor_node_text = (CongEditorNodeText*)user_data;
	
	g_return_if_fail (IS_CONG_EDITOR_NODE_TEXT(editor_node_text));

	/* Optimise regeneration of markup so it only happens in the cases where the "local selection" has changed: */
	if (regenerate_selection_state (editor_node_text)) {
		gchar *markup = generate_markup (editor_node_text);
	
		pango_layout_set_markup (PRIVATE(editor_node_text)->pango_layout,
					 markup,
					 -1);
		
		g_free (markup);

		/* The change to the markup should only affect the colours of the text hence we don't need to regenerate the lines; we only need to
		   ensure all the fragments get redrawn. 

		   We don't have any way of getting at the specific areas, so redraw the entire widget for now:
		*/		
		gtk_widget_queue_draw (GTK_WIDGET(cong_editor_node_get_widget (CONG_EDITOR_NODE (editor_node_text))));
	}
}

static gboolean 
get_location_at_xy(CongEditorNodeText *editor_node_text, 
		   CongEditorAreaTextFragment *editor_area_text_fragment,
		   int x, 
		   int y, 
		   CongLocation *result)
{
	int index_, trailing;

	g_return_val_if_fail(result, FALSE);
	g_return_val_if_fail(editor_area_text_fragment, FALSE);

	if (cong_editor_area_text_fragment_x_to_index (CONG_EDITOR_AREA_TEXT_FRAGMENT(editor_area_text_fragment),
						       x,
						       &index_,
						       &trailing)) {
		int original_byte_offset;

#if 1
		g_message("(%i,%i) -> index %i", x,y, index_);
#endif

#if 0
		index_ += cong_editor_area_text_fragment_get_text_offset (editor_area_text_fragment);
#endif

		if ( cong_text_cache_convert_stripped_byte_offset_to_original (PRIVATE(editor_node_text)->text_cache,
									       index_,
									       &original_byte_offset) ) {
			
			cong_location_set_node_and_byte_offset(result,
							       cong_editor_node_get_node (CONG_EDITOR_NODE(editor_node_text)),
							       original_byte_offset);
			return TRUE;
		}
	}


	return FALSE;
}

/* Declarations of the CongEditorArea event handlers for the block areas: */
static gboolean
on_signal_button_press (CongEditorArea *editor_area, 
			      GdkEventButton *event,
			      gpointer user_data)
{
	CongDocument *doc;
	CongSelection *selection;
	CongCursor *cursor;
	CongEditorWidget3 *editor_widget;

	CongEditorNodeText *editor_node_text = CONG_EDITOR_NODE_TEXT(user_data);
	CongEditorAreaTextFragment *editor_area_text_fragment = CONG_EDITOR_AREA_TEXT_FRAGMENT(editor_area);

	GtkWindow *parent_window;

#if 0
	g_message("on_signal_button_press");
#endif

	editor_widget = cong_editor_node_get_widget (CONG_EDITOR_NODE(editor_node_text));
	parent_window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(editor_widget)));

	doc = cong_editor_node_get_document (CONG_EDITOR_NODE(editor_node_text));
	cursor = cong_document_get_cursor(doc);
	selection = cong_document_get_selection(doc);
	
	if (event->button == 1) {
		switch (event->type) {
		default: return FALSE; /* do nothing */
#if 0
		case GDK_3BUTTON_PRESS:
			/* Handle triple-click by locating the tag containing the click location, 
			   making that the selection, with the cursor at the end of it? */
			{
				CongLocation click_location;
				CongLocation start_of_tag;
				CongLocation end_of_tag;

				if (get_location_at_xy(editor_node_text, editor_area_text_fragment, event->x, event->y, &click_location)) {
					cong_location_calc_tag_start(&click_location, &tag_start);
					cong_location_calc_tag_end(&click_location, &tag_end);
						
					cong_location_copy(&selection->loc0, &start_of_tag);
					cong_location_copy(&selection->loc1, &end_of_tag);
					cong_location_copy(&cursor->location, &end_of_tag);
				
					cong_document_on_selection_change(doc);
					cong_document_on_cursor_change(doc);
				}
			}
			return TRUE;
#endif

		case GDK_2BUTTON_PRESS:
			/* Handle double-click by locating the word containing the click location, 
			   making that the selection, with the cursor at the end of it. */
			{
				CongLocation click_location;
				CongLocation start_of_word;
				CongLocation end_of_word;

				if (get_location_at_xy(editor_node_text, editor_area_text_fragment, event->x, event->y, &click_location)) {
					if (cong_location_calc_word_extent(&click_location, doc, &start_of_word, &end_of_word)) {
						CongCommand *cmd = cong_document_begin_command (doc,
												_("Select word"),
												NULL);

						cong_command_add_selection_change (cmd,										   
										   &start_of_word,
										   &end_of_word);						
						cong_command_add_cursor_change (cmd, 
										&end_of_word);

						cong_document_end_command (doc,
									   cmd);
					}			       
				}
			}
			return TRUE;

		case GDK_BUTTON_PRESS:
			/* Handle single-click by moving the cursor and selection to the location of the click: */
			{
				CongLocation new_location;

				gtk_widget_grab_focus(GTK_WIDGET(editor_widget));
				gtk_widget_grab_default(GTK_WIDGET(editor_widget));

				if (get_location_at_xy(editor_node_text, editor_area_text_fragment, event->x, event->y, &new_location)) {
					CongCommand *cmd = cong_document_begin_command (doc, _("Move cursor"), NULL);

					cong_command_add_selection_change (cmd,
									   &new_location,
									   &new_location);
					cong_command_add_cursor_change(cmd,
								       &new_location);

					cong_document_end_command (doc, cmd);
				}
			}

			return TRUE;
		}
	} else if (event->button == 3) {
		editor_popup_build(doc, parent_window);
		editor_popup_show(cong_app_singleton()->popup, event);

		return TRUE;
	}

	return FALSE;
}


static gboolean
on_signal_motion_notify (CongEditorArea *editor_area, 
			 GdkEventMotion *event,
			 gpointer user_data)
{
	CongDocument *doc;
	CongCursor *cursor;
	CongSelection *selection;
	CongLocation new_location;

	CongEditorNodeText *editor_node_text = (CongEditorNodeText*)user_data;
	CongEditorAreaTextFragment *editor_area_text_fragment = CONG_EDITOR_AREA_TEXT_FRAGMENT(editor_area);

	if (!(event->state & GDK_BUTTON1_MASK)) {
		return FALSE;
	}

	doc = cong_editor_area_get_document(editor_area);
	cursor = cong_document_get_cursor(doc);
	selection = cong_document_get_selection(doc);
	
	if (get_location_at_xy (editor_node_text, 
				editor_area_text_fragment,
				event->x, 
				event->y, 
				&new_location)) {
		CongCommand *cmd = cong_document_begin_command (doc, 
								_("Drag out selection"), 
								"cong-drag-selection");		
		cong_command_add_cursor_change (cmd,
						&new_location);
		cong_command_add_selection_change (cmd, 
						   cong_selection_get_logical_start (selection),
						   &new_location);		
		cong_document_end_command (doc, cmd);

		return TRUE;
	}

	return FALSE;
}

static enum CongFlowType
get_flow_type(CongEditorNode *editor_node)
{
	return CONG_FLOW_TYPE_INLINE;
}

/* Internal utilities: */
static const gchar*
get_text_cache_input (CongEditorNodeText *editor_node_text)
{
	return cong_editor_node_get_node (CONG_EDITOR_NODE(editor_node_text))->content;
}

#if 0
gboolean
cong_selection_is_valid (CongSelection *selection)
{
	g_return_val_if_fail (selection, FALSE);

	if (selection->loc0.node) {
		if (selection->loc1.node) {
			return (selection->loc0.node->parent == selection->loc1.node->parent);
		}
	}
	
	return FALSE;
}
#endif

static void
cong_text_selection_state_get (CongEditorNodeTextSelectionState *selection_state,
			       CongSelection *selection,
			       CongNodePtr this_node,
			       CongTextCache *text_cache);

static gboolean
cong_text_selection_state_equals  (const CongEditorNodeTextSelectionState *range_a,
				   const CongEditorNodeTextSelectionState *range_b);

static gchar*
cong_text_selection_state_generate_markup (CongEditorNodeTextSelectionState *selection_state,
					   CongSelection *selection,
					   const gchar *stripped_text);

static void
cong_text_selection_state_get (CongEditorNodeTextSelectionState *selection_state,
			       CongSelection *selection,
			       CongNodePtr this_node,
			       CongTextCache *text_cache)
{
	g_return_if_fail (selection_state);
	g_return_if_fail (selection);
	g_return_if_fail (this_node);
	g_return_if_fail (text_cache);

	selection_state->is_selection_valid = cong_selection_is_valid (selection);

	/* See if we have any selections starting or ending in this node: */
	selection_state->got_selection_start = cong_selection_get_start_byte_offset (selection, 
										     this_node, 
										     &selection_state->selection_start_original);
	selection_state->got_selection_end = cong_selection_get_end_byte_offset (selection, 
										 this_node, 
										 &selection_state->selection_end_original);

	if (selection_state->got_selection_start) {
		selection_state->got_selection_start = cong_text_cache_convert_original_byte_offset_to_stripped (text_cache,
														 selection_state->selection_start_original,
														 &selection_state->selection_start_stripped);		
	}
	if (selection_state->got_selection_end) {
		selection_state->got_selection_end = cong_text_cache_convert_original_byte_offset_to_stripped (text_cache,
													       selection_state->selection_end_original,
													       &selection_state->selection_end_stripped);
	}
}

static gboolean
cong_text_selection_state_equals  (const CongEditorNodeTextSelectionState *range_a,
				   const CongEditorNodeTextSelectionState *range_b)
{
	if (range_a->got_selection_end) {
		if (range_b->got_selection_end) {
			if (range_a->selection_end_original!=range_b->selection_end_original) {
				return FALSE;
			}
			if (range_a->selection_end_stripped!=range_b->selection_end_stripped) {
				return FALSE;
			}
		} else {
			return FALSE;
		}
	} else {
		if (range_b->got_selection_end) {
			return FALSE;
		}
	}

	if (range_a->got_selection_start) {
		if (range_b->got_selection_start) {
			if (range_a->selection_start_original!=range_b->selection_start_original) {
				return FALSE;
			}
			if (range_a->selection_start_stripped!=range_b->selection_start_stripped) {
				return FALSE;
			}
		} else {
			return FALSE;
		}
	} else {
		if (range_b->got_selection_start) {
			return FALSE;
		}
	}

	if (range_a->is_selection_valid!=range_b->is_selection_valid) {
		return FALSE;
	}
	
	return TRUE;
}

static gchar*
cong_text_selection_state_generate_markup (CongEditorNodeTextSelectionState *selection_state,
					   CongSelection *selection,
					   const gchar *stripped_text)
{
	gchar *result_markup;

	g_return_val_if_fail (selection_state, NULL);
	g_return_val_if_fail (stripped_text, NULL);

	{
		/* FIXME: get these from theme! */
		/* FIXME: also ought to do something about invalid selections... */
		const gchar *fg_col_selection;
		const gchar *bg_col_selection;
		const gchar *fg_col_normal = "black";
		const gchar *bg_col_normal = "white";
		
		gchar *before_selection = NULL;
		gchar *within_selection = NULL;
		gchar *after_selection = NULL;

		if (selection_state->is_selection_valid) {
			fg_col_selection = "white";
			bg_col_selection = "black";
		} else {
			fg_col_selection = "white";
			bg_col_selection = "red";
		}
		
		if (selection_state->got_selection_start) {

			before_selection = g_strndup (stripped_text, selection_state->selection_start_stripped);

			if (selection_state->got_selection_end) {
				
				g_assert (selection_state->selection_start_stripped<=selection_state->selection_end_stripped);

				/* we've got a self-contained selection within this node, or one that entirely encloses it: */
				within_selection = g_strndup (stripped_text + selection_state->selection_start_stripped, selection_state->selection_end_stripped - selection_state->selection_start_stripped);
				after_selection =  g_strdup (stripped_text + selection_state->selection_end_stripped);
			} else {
				/* we've got a selection that starts in this node but carries on past the end: */
				g_assert (selection_state->selection_start_stripped<=strlen(stripped_text));

				within_selection = g_strdup (stripped_text + selection_state->selection_start_stripped);
				after_selection =  g_strdup ("");
				
			}
		} else {
			before_selection = g_strdup ("");

			if (selection_state->got_selection_end) {
				/* we've got a selection that starts before this node and carries on past the end: */
				within_selection = g_strndup (stripped_text,  selection_state->selection_end_stripped);
				after_selection = g_strdup (stripped_text + selection_state->selection_end_stripped);
			} else {
				/* no selections present: */
				within_selection =  g_strdup ("");
				after_selection =  g_strdup (stripped_text);
			}
		}
		
		g_assert(before_selection);
		g_assert(within_selection);
		g_assert(after_selection);
		
		/* We now escape these three strings and use them to generate the resulting markup: */
		{
			gchar *before_selection_escaped = g_markup_escape_text (before_selection,
										strlen(before_selection));
			gchar *within_selection_escaped = g_markup_escape_text (within_selection,
										strlen(within_selection));
			gchar *after_selection_escaped = g_markup_escape_text (after_selection,
										strlen(after_selection));
			
			result_markup = g_strdup_printf ("<span background=\"%s\" foreground=\"%s\">%s</span>"
							 "<span background=\"%s\" foreground=\"%s\">%s</span>"
							 "<span background=\"%s\" foreground=\"%s\">%s</span>", 
							 bg_col_normal, fg_col_normal, before_selection_escaped,
							 bg_col_selection, fg_col_selection, within_selection_escaped,
							 bg_col_normal, fg_col_normal, after_selection_escaped);
			
			g_free (before_selection_escaped);
			g_free (within_selection_escaped);
			g_free (after_selection_escaped);
		}
		
		g_free (before_selection);
		g_free (within_selection);
		g_free (after_selection);
	}

	return result_markup;
	
}

/* Returns TRUE if anything has changed */
static gboolean
regenerate_selection_state (CongEditorNodeText *editor_node_text)
{
	CongNodePtr this_node;
	CongSelection *selection;

	g_return_val_if_fail (editor_node_text, FALSE);

	this_node = cong_editor_node_get_node (CONG_EDITOR_NODE(editor_node_text));
	selection = cong_document_get_selection (cong_editor_node_get_document (CONG_EDITOR_NODE(editor_node_text)));

	if (PRIVATE(editor_node_text)->selection_state_valid) {
		CongEditorNodeTextSelectionState tmp_selection_state;

		cong_text_selection_state_get (&tmp_selection_state,
					       selection,
					       this_node,
					       PRIVATE(editor_node_text)->text_cache);

		if (cong_text_selection_state_equals (&tmp_selection_state,
						      &PRIVATE(editor_node_text)->cached_selection_state)) {
			/* Nothing has changed. */
			return FALSE;

		} else {
			PRIVATE(editor_node_text)->cached_selection_state = tmp_selection_state;
			return TRUE;
		}

	} else {
		cong_text_selection_state_get (&PRIVATE(editor_node_text)->cached_selection_state,
					       selection,
					       this_node,
					       PRIVATE(editor_node_text)->text_cache);

		PRIVATE(editor_node_text)->selection_state_valid = TRUE;

		return TRUE;
	}
	
}

static gchar*
generate_markup (CongEditorNodeText *editor_node_text)
{
	CongNodePtr this_node;
	const gchar *stripped_text;
	CongSelection *selection;

	g_return_val_if_fail (editor_node_text, NULL);

	selection = cong_document_get_selection (cong_editor_node_get_document (CONG_EDITOR_NODE(editor_node_text)));
	stripped_text = cong_text_cache_get_text (PRIVATE(editor_node_text)->text_cache);
	
	g_assert (PRIVATE(editor_node_text)->selection_state_valid);

	return cong_text_selection_state_generate_markup (&PRIVATE(editor_node_text)->cached_selection_state,
							  selection,
							  stripped_text);
}
