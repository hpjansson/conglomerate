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

#define PRIVATE(x) ((x)->private)

struct CongEditorNodeTextDetails
{
	int dummy;

	CongTextCache* text_cache;

	CongEditorAreaText *area_text;
	
	gulong handler_id_node_set_text;

#if 0
	CongEditorAreaComposer *flow_test;
	PangoLayout *pango_layout;
	GList *list_of_text_fragments;
#endif
};

static void
finalize (GObject *object);

static void
dispose (GObject *object);

static CongEditorArea*
generate_area (CongEditorNode *editor_node);


/* FIXME:  We probably shouldn't have every text node in the doc listening to every text node change... probably should allow for a dispatch mechanism within the widget */
/* Declarations of the CongDocument event handlers: */
static void 
on_signal_set_text_notify_after (CongDocument *doc, 
				 CongNodePtr node, 
				 const xmlChar *new_content, 
				 gpointer user_data);

/* Declarations of the CongEditorArea event handlers: */
static gboolean
on_signal_button_press (CongEditorArea *editor_area, 
			GdkEventButton *event,
			gpointer user_data);
static gboolean
on_signal_motion_notify (CongEditorArea *editor_area, 
			 GdkEventMotion *event,
			 gpointer user_data);

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

	node_klass->generate_area = generate_area;
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
				 CongNodePtr node)
{
	cong_editor_node_construct (CONG_EDITOR_NODE (editor_node_text),
				    editor_widget,
				    node);

	PRIVATE(editor_node_text)->handler_id_node_set_text = g_signal_connect_after (G_OBJECT(cong_editor_widget3_get_document(editor_widget)), 
										      "node_set_text",
										      G_CALLBACK(on_signal_set_text_notify_after),
										      editor_node_text);

	return editor_node_text;
}

static void
dispose (GObject *object)
{
	CongEditorNodeText *editor_node_text = CONG_EDITOR_NODE_TEXT(object);

	g_signal_handler_disconnect (G_OBJECT(cong_editor_node_get_document(CONG_EDITOR_NODE(object))),
				     PRIVATE(editor_node_text)->handler_id_node_set_text);	

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}


CongEditorNode*
cong_editor_node_text_new (CongEditorWidget3 *widget,
			   CongNodePtr node)
{
#if DEBUG_EDITOR_NODE_LIFETIMES
	g_message("cong_editor_node_text_new(%s)", node->content);
#endif

	return CONG_EDITOR_NODE( cong_editor_node_text_construct (g_object_new (CONG_EDITOR_NODE_TEXT_TYPE, NULL),
								  widget,
								  node)
				 );
}

static CongEditorArea*
generate_area (CongEditorNode *editor_node)
{
	CongEditorNodeText *node_text = CONG_EDITOR_NODE_TEXT(editor_node);

	g_return_val_if_fail (editor_node, NULL);

	PRIVATE(node_text)->text_cache = cong_text_cache_new (TRUE,
							      cong_editor_node_get_node (editor_node)->content);

#if 0
	{	

#error
		/* FIXME: this is really broken WRT lifetimes */
		PRIVATE(node_text)->flow_test = ;
		PRIVATE(node_text)->pango_layout = pango_layout_new(gdk_pango_context_get());
		/* can't get children until we set set width which we can't do until we've got a requisition! */
		PRIVATE(node_text)->list_of_text_fragments = ;

		cong_editor_area_container_add_child (parent_area,
						      CONG_EDITOR_AREA( PRIVATE(node_text)->flow_test ));

		return PRIVATE(node_text)->flow_test;
	}
#else

	PRIVATE(node_text)->area_text = 
		CONG_EDITOR_AREA_TEXT( cong_editor_area_text_new (cong_editor_node_get_widget (editor_node),
								  cong_app_singleton()->fonts[CONG_FONT_ROLE_BODY_TEXT], 
								  NULL,
								  cong_text_cache_get_text (PRIVATE(node_text)->text_cache))
				       );

	g_signal_connect (PRIVATE(node_text)->area_text,
			  "button_press_event",
			  G_CALLBACK(on_signal_button_press),
			  editor_node);

	g_signal_connect (PRIVATE(node_text)->area_text,
			  "motion_notify_event",
			  G_CALLBACK(on_signal_motion_notify),
			  editor_node);

#if 0
	g_signal_connect (PRIVATE(node_text)->area_text,
			  "key_press_event",
			  G_CALLBACK(on_signal_key_press),
			  editor_node);
#endif
	
	return CONG_EDITOR_AREA(PRIVATE(node_text)->area_text);
#endif
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

		cong_text_cache_set_text (PRIVATE(editor_node_text)->text_cache,
					  cong_editor_node_get_node (CONG_EDITOR_NODE(editor_node_text))->content);

		g_assert(PRIVATE(editor_node_text)->area_text);
		cong_editor_area_text_set_text (PRIVATE(editor_node_text)->area_text,
						cong_text_cache_get_text(PRIVATE(editor_node_text)->text_cache));
	}
}


static gboolean 
get_location_at_xy(CongEditorNodeText *editor_node_text, 
		   int x, 
		   int y, 
		   CongLocation *result)
{
	int index_, trailing;

	g_return_val_if_fail(result, FALSE);


	if (cong_editor_area_xy_to_index (PRIVATE(editor_node_text)->area_text,
					  x,
					  y,
					  &index_,
					  &trailing)) {
		int original_byte_offset;

		g_message("(%i,%i) -> index %i", x,y, index_);

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

/* Definitions of the CongEditorArea event handlers: */
static gboolean
on_signal_button_press (CongEditorArea *editor_area, 
			GdkEventButton *event,
			gpointer user_data)
{
	CongDocument *doc;
	CongSelection *selection;
	CongCursor *cursor;
	CongEditorWidget3 *editor_widget;

	CongEditorNodeText *editor_node_text = (CongEditorNodeText*)user_data;

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

				if (get_location_at_xy(editor_node_text, event->x, event->y, &click_location)) {
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

				if (get_location_at_xy(editor_node_text, event->x, event->y, &click_location)) {
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

				if (get_location_at_xy(editor_node_text, event->x, event->y, &cursor->location)) {
					
					cong_selection_start_from_curs(selection, cursor);
					cong_selection_end_from_curs(selection, cursor);
					cong_document_on_selection_change(doc);
					cong_document_on_cursor_change(doc);
						
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

	CongEditorNodeText *editor_node_text = (CongEditorNodeText*)user_data;

	if (!(event->state & GDK_BUTTON1_MASK)) {
		return FALSE;
	}

	doc = cong_editor_area_get_document(editor_area);
	cursor = cong_document_get_cursor(doc);
	selection = cong_document_get_selection(doc);
	
	if (get_location_at_xy (editor_node_text, 
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
