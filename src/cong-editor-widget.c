/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-widget.c
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
#include "cong-document.h"
#include "cong-dispspec.h"

/* Prototypes of the handler functions: */
static void on_document_coarse_update(CongView *view);
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent);
static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling);
static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling);
static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content);
static void on_selection_change(CongView *view);
static void on_cursor_change(CongView *view);

#define DEBUG_EDITOR_WIDGET_VIEW 0

#if DEBUG_EDITOR_WIDGET_VIEW
#define CONG_EDITOR_VIEW_SELF_TEST(details) (cong_element_editor_recursive_self_test(details->root_editor))
#define CONG_EDITOR_WIDGET_DEBUG_MSG1(x)    g_message((x))
#define CONG_EDITOR_WIDGET_DEBUG_MSG2(x, a) g_message((x), (a))
#define CONG_EDITOR_WIDGET_DEBUG_MSG3(x, a, b) g_message((x), (a), (b))
#else
#define CONG_EDITOR_VIEW_SELF_TEST(details) ((void)0)
#define CONG_EDITOR_WIDGET_DEBUG_MSG1(x)    ((void)0)
#define CONG_EDITOR_WIDGET_DEBUG_MSG2(x, a) ((void)0)
#define CONG_EDITOR_WIDGET_DEBUG_MSG3(x, a, b) ((void)0)
#endif

/* Definitions of the handler functions: */
static void on_document_coarse_update(CongView *view)
{
	CongEditorWidgetView *editor_widget_view;

	g_return_if_fail(view);

	#if DEBUG_EDITOR_WIDGET_VIEW
	CONG_EDITOR_WIDGET_DEBUG_MSG1("CongEditorWidgetView - on_document_coarse_update\n");
	#endif

	editor_widget_view = CONG_EDITOR_WIDGET_VIEW(view);

	/* Ignore for now */
}

static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent)
{
	CongEditorWidgetView *editor_widget_view;
	CongEditorWidgetDetails* details;
	CongDocumentEvent event;
	CongElementEditor *element_editor;

	g_return_if_fail(view);
	g_return_if_fail(node);

	#if DEBUG_EDITOR_WIDGET_VIEW
	CONG_EDITOR_WIDGET_DEBUG_MSG1("CongEditorWidgetView - on_document_node_make_orphan\n");
	#endif

	editor_widget_view = CONG_EDITOR_WIDGET_VIEW(view);
	details = GET_DETAILS(editor_widget_view->widget);

	if (before_event) {
		CONG_EDITOR_VIEW_SELF_TEST(details);
	}

	event.before_event = before_event;
	event.type = CONG_DOCUMENT_EVENT_MAKE_ORPHAN;
	event.data.make_orphan.node = node;
	event.data.make_orphan.former_parent = former_parent;

	element_editor = cong_editor_widget_get_element_editor_for_node(editor_widget_view->widget, former_parent);
	g_assert(element_editor);

	cong_element_editor_on_document_event(element_editor, &event);

	if (!before_event) {
		CONG_EDITOR_VIEW_SELF_TEST(details);
	}

}

static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling)
{
	CongEditorWidgetView *editor_widget_view;
	CongEditorWidgetDetails* details;
	CongDocumentEvent event;
	CongElementEditor *element_editor;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(older_sibling);

	#if DEBUG_EDITOR_WIDGET_VIEW
	CONG_EDITOR_WIDGET_DEBUG_MSG1("CongEditorWidgetView - on_document_node_add_after\n");
	#endif

	editor_widget_view = CONG_EDITOR_WIDGET_VIEW(view);
	details = GET_DETAILS(editor_widget_view->widget);

	if (before_event) {
		CONG_EDITOR_VIEW_SELF_TEST(details);
	}

	event.before_event = before_event;
	event.type = CONG_DOCUMENT_EVENT_ADD_AFTER;
	event.data.add_after.node = node;
	event.data.add_after.older_sibling = older_sibling;

	element_editor = cong_editor_widget_get_element_editor_for_node(editor_widget_view->widget, older_sibling->parent);
	g_assert(element_editor);

	cong_element_editor_on_document_event(element_editor, &event);

	if (!before_event) {
		CONG_EDITOR_VIEW_SELF_TEST(details);
	}
}

static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling)
{
	CongEditorWidgetView *editor_widget_view;
	CongEditorWidgetDetails* details;
	CongDocumentEvent event;
	CongElementEditor *element_editor;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);

	#if DEBUG_EDITOR_WIDGET_VIEW
	CONG_EDITOR_WIDGET_DEBUG_MSG1("CongEditorWidgetView - on_document_node_add_before\n");
	#endif

	editor_widget_view = CONG_EDITOR_WIDGET_VIEW(view);
	details = GET_DETAILS(editor_widget_view->widget);

	if (before_event) {
		CONG_EDITOR_VIEW_SELF_TEST(details);
	}

	event.before_event = before_event;
	event.type = CONG_DOCUMENT_EVENT_ADD_BEFORE;
	event.data.add_before.node = node;
	event.data.add_before.younger_sibling = younger_sibling;

	element_editor = cong_editor_widget_get_element_editor_for_node(editor_widget_view->widget, younger_sibling->parent);
	g_assert(element_editor);

	cong_element_editor_on_document_event(element_editor, &event);

	if (!before_event) {
		CONG_EDITOR_VIEW_SELF_TEST(details);
	}
}

static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent)
{
	CongEditorWidgetView *editor_widget_view;
	CongEditorWidgetDetails* details;
	CongDocumentEvent event;
	CongElementEditor *element_editor;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);

	#if DEBUG_EDITOR_WIDGET_VIEW
	CONG_EDITOR_WIDGET_DEBUG_MSG1("CongEditorWidgetView - on_document_node_set_parent\n");
	#endif

	editor_widget_view = CONG_EDITOR_WIDGET_VIEW(view);
	details = GET_DETAILS(editor_widget_view->widget);

	if (before_event) {
		CONG_EDITOR_VIEW_SELF_TEST(details);
	}

	event.before_event = before_event;
	event.type = CONG_DOCUMENT_EVENT_SET_PARENT;
	event.data.set_parent.node = node;
	event.data.set_parent.adoptive_parent = adoptive_parent;

	element_editor = cong_editor_widget_get_element_editor_for_node(editor_widget_view->widget, adoptive_parent);
	g_assert(element_editor);

	cong_element_editor_on_document_event(element_editor, &event);

	if (!before_event) {
		CONG_EDITOR_VIEW_SELF_TEST(details);
	}
}

static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content)
{
	CongEditorWidgetView *editor_widget_view;
	CongEditorWidgetDetails* details;
	CongDocumentEvent event;
	CongElementEditor *element_editor;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_EDITOR_WIDGET_VIEW
	CONG_EDITOR_WIDGET_DEBUG_MSG1("CongEditorWidgetView - on_document_node_set_text\n");
	#endif

	editor_widget_view = CONG_EDITOR_WIDGET_VIEW(view);
	details = GET_DETAILS(editor_widget_view->widget);

	if (before_event) {
		CONG_EDITOR_VIEW_SELF_TEST(details);
	}

	event.before_event = before_event;
	event.type = CONG_DOCUMENT_EVENT_SET_TEXT;
	event.data.set_text.node = node;
	event.data.set_text.new_content = new_content;

	/* This message goes direct to the node concerned, rather than to the root node: */
	element_editor = cong_editor_widget_get_element_editor_for_node(editor_widget_view->widget, node);
	g_assert(element_editor);

	cong_element_editor_on_document_event(element_editor, &event);

	if (!before_event) {
		CONG_EDITOR_VIEW_SELF_TEST(details);
	}
}

static void on_selection_change(CongView *view)
{
	CongEditorWidgetView *editor_widget_view;

	g_return_if_fail(view);

	#if DEBUG_EDITOR_WIDGET_VIEW
	CONG_EDITOR_WIDGET_DEBUG_MSG1("CongEditorWidgetView - on_selection_change\n");
	#endif

	editor_widget_view = CONG_EDITOR_WIDGET_VIEW(view);

	/* Force a redraw: */
	gtk_widget_queue_draw(GTK_WIDGET(editor_widget_view->widget));	
}

static void on_cursor_change(CongView *view)
{
	CongEditorWidgetView *editor_widget_view;

	g_return_if_fail(view);

	#if DEBUG_EDITOR_WIDGET_VIEW
	CONG_EDITOR_WIDGET_DEBUG_MSG1("CongEditorWidgetView - on_cursor_change\n");
	#endif

	editor_widget_view = CONG_EDITOR_WIDGET_VIEW(view);

	/* Force a redraw: */
	gtk_widget_queue_draw(GTK_WIDGET(editor_widget_view->widget));
}



/* Event handlers for widget: */
static gboolean expose_event_handler(GtkWidget *w, GdkEventExpose *event, gpointer user_data)
{
	CongDocument *doc;
	CongEditorWidget *editor_widget = CONG_EDITOR_WIDGET(w);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);

	CONG_EDITOR_WIDGET_DEBUG_MSG1("expose_event_handler");

	doc = cong_editor_widget_get_document(editor_widget);

	/* Fill the rectangle with the background colour: */
	gdk_draw_rectangle(GDK_DRAWABLE(w->window),
			   w->style->white_gc,
			   TRUE, /* gint filled, */
			   event->area.x,
			   event->area.y,
			   event->area.width,
			   event->area.height);	

	/* Render the ElementEditors: */
	cong_element_editor_recursive_render(details->root_editor, &event->area);

	/* For now we render all of them */

	return TRUE;
}

static gboolean configure_event_handler(GtkWidget *w, GdkEventConfigure *event, gpointer user_data)
{
	CongEditorWidget *editor_widget = CONG_EDITOR_WIDGET(w);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);

	CONG_EDITOR_WIDGET_DEBUG_MSG3("configure_event_handler; w/h = %i,%i", event->width, event->height);
 
 	cong_element_editor_get_size_requisition(details->root_editor, event->width);

	/* Pass all of the allocation to root editor; this will recursively allocate space to its children: */
	cong_element_editor_set_allocation(details->root_editor, 
					   event->x,
					   event->y,
					   event->width,
					   event->height);

	return TRUE;
}

static gboolean button_press_event_handler(GtkWidget *w, GdkEventButton *event, gpointer user_data)
{
	CongEditorWidget *editor_widget = CONG_EDITOR_WIDGET(w);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);

	CONG_EDITOR_WIDGET_DEBUG_MSG1("button_press_event_handler");

	cong_element_editor_on_button_press(details->root_editor, event);

	return TRUE;
}

static gboolean motion_notify_event_handler(GtkWidget *w, GdkEventMotion *event, gpointer user_data)
{
	CongEditorWidget *editor_widget = CONG_EDITOR_WIDGET(w);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);

	CONG_EDITOR_WIDGET_DEBUG_MSG1("motion_notify_event_handler");

	cong_element_editor_on_motion_notify(details->root_editor, event);

	return TRUE;
}

static gboolean key_press_event_handler(GtkWidget *w, GdkEventKey *event, gpointer user_data)
{
	CongEditorWidget *editor_widget = CONG_EDITOR_WIDGET(w);
	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
	CongDocument *doc = cong_editor_widget_get_document(editor_widget);
	CongCursor *cursor = cong_document_get_cursor(doc);
	CongElementEditor *element_editor;

	CONG_EDITOR_WIDGET_DEBUG_MSG1("key_press_event_handler");

	g_return_val_if_fail(cursor->location.node, FALSE);

	/* Get element for cursor location; get it to handle the message: */
	element_editor = cong_editor_widget_get_element_editor_for_node(editor_widget, cursor->location.node);
	g_assert(element_editor);

	cong_element_editor_on_key_press(element_editor, event);

	return TRUE;
}

static void size_request_handler(GtkWidget *widget,
 				 GtkRequisition *requisition,
 				 gpointer user_data)
{
 	CongDocument *doc;
 	CongEditorWidget *editor_widget = CONG_EDITOR_WIDGET(widget);
 	CongEditorWidgetDetails* details = GET_DETAILS(editor_widget);
 
 	CONG_EDITOR_WIDGET_DEBUG_MSG1("size_request_handler");
 
 	g_assert(widget);
 	g_assert(requisition);
 
 	cong_element_editor_get_size_requisition(details->root_editor, widget->allocation.width);
 
 	requisition->width = details->root_editor->requisition.width;
 	requisition->height = details->root_editor->requisition.height;
}
 

#if 0
void recursively_populate_ui(CongEditorView *editor_view,
			     CongNodePtr x, 
			     int collapsed)
{
	CongNodePtr x_orig;
	GtkWidget *sub = NULL;
	CongDocument *doc;
	CongDispspec *ds;

	g_return_if_fail(editor_view);
	
	doc = editor_view->view.doc;
	ds = cong_document_get_dispspec(doc);
      	
	x_orig = x;
	
	x = cong_node_first_child(x);
	if (!x) return;

	for ( ; x; )
	{
		enum CongNodeType node_type = cong_node_type(x);
		const char *name = xml_frag_name_nice(x);

		/* CONG_EDITOR_WIDGET_DEBUG_MSG1("Examining frag %s\n",name); */

		if (node_type == CONG_NODE_TYPE_ELEMENT)
		{
			CongDispspecElement* element = cong_dispspec_lookup_element(ds, name);

			if (element) {
				if (cong_dispspec_element_is_structural(element)) {
					if (cong_dispspec_element_collapseto(element)) {
						gtk_box_pack_start(GTK_BOX(root), xv_fragment_head(ds, x), FALSE, TRUE, 0);
						
						/* Recurse here: */
						cong_editor_recursively_populate_ui(editor_view, x, root, TRUE);
						
						gtk_box_pack_start(GTK_BOX(root), xv_fragment_tail(ds, x), FALSE, TRUE, 0);
					} else {
						/* New structural element */
						CongSectionHead *section_head;
						GtkWidget *poot;
						GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
						gtk_widget_show(hbox);
						
						gtk_box_pack_start(GTK_BOX(root), hbox, FALSE, TRUE, 0);
						gtk_box_pack_start(GTK_BOX(hbox), xv_section_vline_and_space(ds, cong_node_parent(x)), FALSE, TRUE, 0);
						xv_style_r(hbox, style_white);
						section_head = cong_section_head_new(doc, x);
						poot = section_head->vbox;
						gtk_box_pack_start(GTK_BOX(hbox), poot, TRUE, TRUE, 0);
								
						/* Recurse here: */
						cong_editor_recursively_populate_ui(editor_view, x, poot, FALSE);
								
						sub = xv_section_tail(ds, x);
						xv_style_r(sub, style_white);
						gtk_box_pack_start(GTK_BOX(poot), sub, FALSE, TRUE, 0);
					}
				} else if (cong_dispspec_element_is_span(element) ||
					   CONG_ELEMENT_TYPE_INSERT == cong_dispspec_element_type(element)) {
				        /* New editor window */
				
					sub = xv_section_data(x, doc, ds, collapsed);
					if (sub) {
						gtk_box_pack_start(GTK_BOX(root), sub, FALSE, TRUE, 0);
						xv_style_r(sub, style_white);
					}
				
					x = xv_editor_elements_skip(x, ds);
				} else if (CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE==cong_dispspec_element_type(element)) {
					CongSectionHead *section_head;
					GtkWidget *poot;
					GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
					gtk_widget_show(hbox);

					gtk_box_pack_start(GTK_BOX(root), hbox, FALSE, TRUE, 0);
					gtk_box_pack_start(GTK_BOX(hbox), xv_section_vline_and_space(ds, cong_node_parent(x)), FALSE, TRUE, 0);
					xv_style_r(hbox, style_white);
					section_head = cong_section_head_new(doc,x);
					poot = section_head->vbox;
					gtk_box_pack_start(GTK_BOX(hbox), poot, TRUE, TRUE, 0);
				        /* xv_style_r(poot, style_white); */
				
					sub = xv_section_embedded(doc, x,ds,collapsed);
					gtk_box_pack_start(GTK_BOX(poot), sub, FALSE, TRUE, 0);
					
					sub = xv_section_tail(ds, x);
				        /* xv_style_r(sub, style_white); */
					gtk_box_pack_start(GTK_BOX(poot), sub, FALSE, TRUE, 0);
				}
			}
		}
		else if (node_type == CONG_NODE_TYPE_TEXT)
		{
			/* New editor window */

			sub = xv_section_data(x, doc, ds, collapsed);
			if (sub)
			{
				gtk_box_pack_start(GTK_BOX(root), sub, FALSE, TRUE, 0);
				xv_style_r(sub, style_white);
			}
			
			x = xv_editor_elements_skip(x, ds);
		}

		if (x) {
			x = cong_node_next(x);
		}
	}

	xv_style_r(sub, style_white);
}
#endif

void populate_widget(CongEditorWidget *widget)
{
	CongEditorWidget *editor_widget = CONG_EDITOR_WIDGET(widget);
	CongEditorWidgetDetails *details = GET_DETAILS(editor_widget);
	CongDocument *doc;
	CongDispspec *displayspec;
	CongSectionHeadEditor *section_head;
	CongNodePtr x;

	g_return_if_fail(widget);

	/* FIXME: ultimately we might want a special-purpose root element editor class */
	
	doc = cong_editor_widget_get_document(widget);
	displayspec = cong_document_get_dispspec(doc);
		
	x = cong_document_get_root(doc);

	for ( ; x; x = cong_node_next(x))
	{
		enum CongNodeType type = cong_node_type(x);

		const char *name = xml_frag_name_nice(x);

		CONG_EDITOR_WIDGET_DEBUG_MSG3("examining frag \"%s\", type = %s\n", name, cong_node_type_description(type));
		
		if (type == CONG_NODE_TYPE_ELEMENT && cong_dispspec_type(displayspec, name)==CONG_ELEMENT_TYPE_STRUCTURAL)
		{
			section_head = CONG_SECTION_HEAD_EDITOR(cong_section_head_editor_new(widget, x));
			details->root_editor = CONG_ELEMENT_EDITOR(section_head);
		}

		/* FIXME:  this is a dodgy hack; what if more than one such node? */
	}
}


/* Public interface: */

GtkWidget* cong_editor_widget_new(CongDocument *doc)
{
	CongEditorWidget *widget;
	CongEditorWidgetDetails *details;
	CongEditorWidgetView *view;

	g_return_val_if_fail(doc, NULL);

	widget = GTK_DRAWING_AREA(gtk_drawing_area_new());

	details = g_new0(CongEditorWidgetDetails,1);
	view = g_new0(CongEditorWidgetView,1);

	g_object_set_data(G_OBJECT(widget),
			  "details",
			  details);
	details->widget = widget;
	details->view = view;
	view->widget = widget;
	
	view->view.doc = doc;
	view->view.klass = g_new0(CongViewClass,1);
	view->view.klass->on_document_coarse_update = on_document_coarse_update;
	view->view.klass->on_document_node_make_orphan = on_document_node_make_orphan;
	view->view.klass->on_document_node_add_after = on_document_node_add_after;
	view->view.klass->on_document_node_add_before = on_document_node_add_before;
	view->view.klass->on_document_node_set_parent = on_document_node_set_parent;
	view->view.klass->on_document_node_set_text = on_document_node_set_text;
	view->view.klass->on_selection_change = on_selection_change;
	view->view.klass->on_cursor_change = on_cursor_change;

	details->hash_of_node_to_editor = g_hash_table_new(NULL,
							   NULL);

	cong_document_register_view( doc, CONG_VIEW(view) );

	gtk_signal_connect(GTK_OBJECT(widget), 
			   "expose_event",
			   (GtkSignalFunc) expose_event_handler, 
			   NULL);
	gtk_signal_connect(GTK_OBJECT(widget), 
			   "configure_event",
			   (GtkSignalFunc) configure_event_handler, 
			   NULL);
	gtk_signal_connect(GTK_OBJECT(widget), 
			   "button_press_event",
			   (GtkSignalFunc) button_press_event_handler, 
			   NULL);
	gtk_signal_connect(GTK_OBJECT(widget), 
			   "motion_notify_event",
			   (GtkSignalFunc) motion_notify_event_handler, 
			   NULL);
	gtk_signal_connect_after(GTK_OBJECT(widget), 
				 "key_press_event",
				 (GtkSignalFunc) key_press_event_handler, 
				 NULL);
	gtk_signal_connect(GTK_OBJECT(widget),
 			   "size-request",
 			   (GtkSignalFunc) size_request_handler,
 			   NULL);

	gtk_widget_set_events(GTK_WIDGET(widget), GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_KEY_PRESS_MASK);

	gtk_widget_set(GTK_WIDGET(widget), "can_focus", (gboolean) TRUE, 0);
	gtk_widget_set(GTK_WIDGET(widget), "can_default", (gboolean) TRUE, 0);

	populate_widget(widget);

	/* Recursively update all the size requisitions: */
	cong_element_editor_get_size_requisition(details->root_editor, 100);

	gtk_widget_set_size_request(GTK_WIDGET(widget),
				    details->root_editor->requisition.width,
				    details->root_editor->requisition.height);

	CONG_EDITOR_VIEW_SELF_TEST(details);

	return GTK_WIDGET(widget);
}

CongDocument *cong_editor_widget_get_document(CongEditorWidget *editor_widget)
{
	CongEditorWidgetDetails *details;

	g_return_val_if_fail(editor_widget, NULL);

	details = GET_DETAILS(editor_widget);

	return details->view->view.doc;
}

CongDispspec *cong_editor_widget_get_dispspec(CongEditorWidget *editor_widget)
{
	CongEditorWidgetDetails *details;

	g_return_val_if_fail(editor_widget, NULL);

	details = GET_DETAILS(editor_widget);

	return cong_document_get_dispspec(details->view->view.doc);
}

void cong_editor_widget_force_layout_update(CongEditorWidget *editor_widget)
{
	CongEditorWidgetDetails *details;

	g_return_if_fail(editor_widget);

	details = GET_DETAILS(editor_widget);

	/* Recursively update all the size requisitions: */
	CONG_EDITOR_WIDGET_DEBUG_MSG1("cong_editor_widget_force_layout_update");
 
#if 1
 	gtk_widget_queue_resize(GTK_WIDGET(editor_widget));
#else
	cong_element_editor_get_size_requisition(details->root_editor, GTK_WIDGET(editor_widget)->allocation.width);

	gtk_widget_set_size_request(GTK_WIDGET(editor_widget),
				    details->root_editor->requisition.width,
				    details->root_editor->requisition.height);
	
	gtk_widget_queue_draw(GTK_WIDGET(editor_widget));
#endif
}

/* Internal utility functions: */
void cong_editor_widget_register_element_editor(CongEditorWidget *widget, CongElementEditor *element_editor)
{
	CongEditorWidgetDetails *details;
	CongNodePtr node;

	g_return_if_fail(widget);
	g_return_if_fail(element_editor);

	details = GET_DETAILS(widget);
	
	/* Register all of the nodes in this span with the hashtable: */
	node = element_editor->first_node; 

	while (1) {
		g_assert(node);
		g_hash_table_insert(details->hash_of_node_to_editor,
				    node,
				    element_editor);

		if (node == element_editor->final_node) {
			break;
		} else {
			node = node->next;
		}
	}
	
}

void cong_editor_widget_unregister_element_editor(CongEditorWidget *widget, CongElementEditor *element_editor)
{
	CongEditorWidgetDetails *details;
	CongNodePtr node;

	g_return_if_fail(widget);
	g_return_if_fail(element_editor);

	details = GET_DETAILS(widget);
	
	/* Unregister all of the nodes in this span from the hashtable: */
	node = element_editor->first_node; 

	while (1) {
		g_assert(node);
		g_hash_table_remove(details->hash_of_node_to_editor,
				    node);

		if (node == element_editor->final_node) {
			break;
		} else {
			node = node->next;
		}
	}

}

CongElementEditor* cong_editor_widget_get_element_editor_for_node(CongEditorWidget *widget, CongNodePtr node)
{
	CongEditorWidgetDetails *details;
	CongElementEditor* editor;

	g_return_val_if_fail(widget, NULL);
	g_return_val_if_fail(node, NULL);

	details = GET_DETAILS(widget);

	g_assert(details);

	editor = g_hash_table_lookup(details->hash_of_node_to_editor, node);
	
	if (editor) {
		return editor;
	} else {
		/* Recurse to parent node: */
		g_assert(node->parent);
		return cong_editor_widget_get_element_editor_for_node(widget, node->parent);
	}
}
