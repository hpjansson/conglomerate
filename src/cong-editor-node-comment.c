/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node-comment.c
 *
 * Copyright (C) 2004 David Malcolm
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
#include "cong-editor-node-comment.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"
#include "cong-text-cache.h"
#include "cong-app.h"

#include "cong-editor-area-text-comment.h"
#include "cong-traversal-node.h"

CONG_EDITOR_NODE_DEFINE_SUBCLASS (Comment,
				  comment,
				  CONG_EDITOR_NODE_COMMENT,
				  CongTextCache* text_cache;
				  CongEditorAreaTextComment *area_text_comment;				  
				  gulong handler_id_node_set_text;
				  gulong handler_id_selection_change;
				  )

/* Declarations of the CongDocument event handlers: */
#if 0
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
#endif

/* Exported function definitions: */
CongEditorNodeComment*
cong_editor_node_comment_construct (CongEditorNodeComment *editor_node_comment,
				    CongEditorWidget3* editor_widget,
				    CongTraversalNode *traversal_node)
{
	cong_editor_node_construct (CONG_EDITOR_NODE (editor_node_comment),
				    editor_widget,
				    traversal_node);	

	PRIVATE(editor_node_comment)->text_cache = cong_text_cache_new (FALSE, /* Don't strip whitespace */
									cong_traversal_node_get_node (traversal_node)->content,
									NULL);

	return editor_node_comment;
}

CONG_EDITOR_NODE_IMPLEMENT_DISPOSE_BEGIN(Comment, comment, CONG_EDITOR_NODE_COMMENT)
     if (PRIVATE(editor_node_comment)->text_cache) {
	     cong_text_cache_free (PRIVATE(editor_node_comment)->text_cache);
	     PRIVATE(editor_node_comment)->text_cache = NULL;
     }
CONG_EDITOR_NODE_IMPLEMENT_DISPOSE_END()

#if 1
static void 
create_areas (CongEditorNode *editor_node,
	      const CongAreaCreationInfo *creation_info)
{
	CongEditorNodeComment *node_comment = CONG_EDITOR_NODE_COMMENT(editor_node);

	CongEditorArea *block_area = cong_editor_area_text_comment_new (cong_editor_node_get_widget (editor_node),
									cong_app_get_font (cong_app_singleton(),
											   CONG_FONT_ROLE_BODY_TEXT), 
									NULL,
									cong_text_cache_get_output_text (PRIVATE(node_comment)->text_cache),
									FALSE);
	cong_editor_node_create_block_area (editor_node,
					    creation_info,
					    block_area,
					    FALSE);
	/* FIXME: should we attach signals, or store the area anywhere? */
}

CONG_EDITOR_NODE_DEFINE_BLOCK_AREA_REGENERATION_HOOK

#else
static CongEditorArea*
generate_block_area (CongEditorNode *editor_node)
{
	CongEditorNodeComment *node_comment = CONG_EDITOR_NODE_COMMENT(editor_node);
#if 0
	gchar *text;
#endif

	g_return_val_if_fail (editor_node, NULL);

	PRIVATE(node_comment)->area_text_comment = 
		CONG_EDITOR_AREA_TEXT_COMMENT( cong_editor_area_text_comment_new (cong_editor_node_get_widget (editor_node),
										  cong_app_get_font (cong_app_singleton(),
												     CONG_FONT_ROLE_BODY_TEXT), 
										  NULL,
										  cong_text_cache_get_output_text (PRIVATE(node_comment)->text_cache),
										  FALSE)
					       );

#if 0
	g_signal_connect (PRIVATE(node_comment)->area_text_comment,
			  "button_press_event",
			  G_CALLBACK(on_signal_block_button_press),
			  editor_node);

	g_signal_connect (PRIVATE(node_comment)->area_text_comment,
			  "motion_notify_event",
			  G_CALLBACK(on_signal_block_motion_notify),
			  editor_node);
#endif

#if 0
	g_signal_connect (PRIVATE(node_comment)->area_text,
			  "key_press_event",
			  G_CALLBACK(on_signal_block_key_press),
			  editor_node);
#endif

	return CONG_EDITOR_AREA(PRIVATE(node_comment)->area_text_comment);
}
#endif

/* Definitions of the CongDocument event handlers: */
#if 0
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
	gchar *markup;
	
	g_return_if_fail (IS_CONG_EDITOR_NODE_TEXT(editor_node_text));

	markup = generate_markup (editor_node_text);
	
	pango_layout_set_markup (PRIVATE(editor_node_text)->pango_layout,
				 markup,
				 -1);
	
	g_free (markup);

	cong_editor_node_line_regeneration_required (CONG_EDITOR_NODE(editor_node_text));
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


	g_message("on_signal_button_press");

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
						
						cong_location_copy(&selection->loc0, &start_of_word);
						cong_location_copy(&selection->loc1, &end_of_word);
						cong_location_copy(&cursor->location, &end_of_word);
					}						
				
					cong_document_on_selection_change(doc);
					cong_document_on_cursor_change(doc);
				}
			}
			return TRUE;

		case GDK_BUTTON_PRESS:
			/* Handle single-click by moving the cursor and selection to the location of the click: */
			{
				gtk_widget_grab_focus(GTK_WIDGET(editor_widget));
				gtk_widget_grab_default(GTK_WIDGET(editor_widget));

				if (get_location_at_xy(editor_node_text, editor_area_text_fragment, event->x, event->y, &cursor->location)) {
					
					cong_selection_start_from_curs(selection, cursor);
					cong_selection_end_from_curs(selection, cursor);
					cong_document_on_selection_change(doc);
					cong_document_on_cursor_change(doc);
						
				}
			}

			return TRUE;
		}
	} else if (event->button == 3) {
		editor_popup_build(editor_widget, parent_window);
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
				&cursor->location)) {
		cong_selection_end_from_curs(selection, cursor);
		cong_document_on_selection_change(doc);
		cong_document_on_cursor_change(doc);			
		return TRUE;
	}

	return FALSE;
}
#endif

/* Internal utilities: */
#if 0
const gchar*
get_text_cache_input (CongEditorNodeText *editor_node_text)
{
	return cong_editor_node_get_node (CONG_EDITOR_NODE(editor_node_text))->content;
}

gchar*
generate_markup (CongEditorNodeText *editor_node_text)
{
	gchar *result_markup;
	CongNodePtr this_node;
	const gchar *stripped_text;
	CongSelection *selection;
	gint selection_start_original;
	gint selection_end_original;
	gint selection_start_stripped;
	gint selection_end_stripped;
	gboolean got_selection_start;
	gboolean got_selection_end;
	
	g_return_val_if_fail (editor_node_text, NULL);

	this_node = cong_editor_node_get_node (CONG_EDITOR_NODE(editor_node_text));
	selection = cong_document_get_selection (cong_editor_node_get_document (CONG_EDITOR_NODE(editor_node_text)));
	stripped_text = cong_text_cache_get_text (PRIVATE(editor_node_text)->text_cache),

	/* See if we have any selections starting or ending in this node: */
	got_selection_start = cong_selection_get_start_byte_offset (selection, 
								    this_node, 
								    &selection_start_original);
	got_selection_end = cong_selection_get_end_byte_offset (selection, 
								this_node, 
								&selection_end_original);

	if (got_selection_start) {
		got_selection_start = cong_text_cache_convert_original_byte_offset_to_stripped (PRIVATE(editor_node_text)->text_cache,
												selection_start_original,
												&selection_start_stripped);

	}
	if (got_selection_end) {
		got_selection_end = cong_text_cache_convert_original_byte_offset_to_stripped (PRIVATE(editor_node_text)->text_cache,
											      selection_end_original,
											      &selection_end_stripped);
	}

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

		if (cong_selection_is_valid (selection)) {
			fg_col_selection = "white";
			bg_col_selection = "black";
		} else {
			fg_col_selection = "white";
			bg_col_selection = "red";
		}
		
		if (got_selection_start) {

			before_selection = g_strndup (stripped_text, selection_start_stripped);

			if (got_selection_end) {
				
				g_assert (selection_start_stripped<=selection_end_stripped);

				/* we've got a self-contained selection within this node, or one that entirely encloses it: */
				within_selection = g_strndup (stripped_text + selection_start_stripped, selection_end_stripped - selection_start_stripped);
				after_selection =  g_strdup (stripped_text + selection_end_stripped);
			} else {
				/* we've got a selection that starts in this node but carries on past the end: */
				within_selection = g_strdup (stripped_text + selection_start_stripped);
				after_selection =  g_strdup ("");
				
			}
		} else {
			before_selection = g_strdup ("");

			if (got_selection_end) {
				/* we've got a selection that starts before this node and carries on past the end: */
				within_selection = g_strndup (stripped_text,  selection_end_stripped);
				after_selection = g_strdup (stripped_text + selection_end_stripped);
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
#endif
