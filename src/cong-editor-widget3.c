/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-widget3.c
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
#include "cong-editor-widget3-impl.h"
#include "cong-document.h"
#include "cong-dispspec.h"

#define DEBUG_EDITOR_WIDGET_VIEW 1

#if DEBUG_EDITOR_WIDGET_VIEW
#define CONG_EDITOR_VIEW_SELF_TEST(details) (cong_element_editor_recursive_self_test(details->root_editor))
#define CONG_EDITOR_WIDGET3_DEBUG_MSG1(x)    g_message((x))
#define CONG_EDITOR_WIDGET3_DEBUG_MSG2(x, a) g_message((x), (a))
#define CONG_EDITOR_WIDGET3_DEBUG_MSG3(x, a, b) g_message((x), (a), (b))
#else
#define CONG_EDITOR_VIEW_SELF_TEST(details) ((void)0)
#define CONG_EDITOR_WIDGET3_DEBUG_MSG1(x)    ((void)0)
#define CONG_EDITOR_WIDGET3_DEBUG_MSG2(x, a) ((void)0)
#define CONG_EDITOR_WIDGET3_DEBUG_MSG3(x, a, b) ((void)0)
#endif

/* Declarations of the widget event handlers: */
static gboolean expose_event_handler(GtkWidget *w, GdkEventExpose *event, gpointer user_data);
static gboolean configure_event_handler(GtkWidget *w, GdkEventConfigure *event, gpointer user_data);
static gboolean button_press_event_handler(GtkWidget *w, GdkEventButton *event, gpointer user_data);
static gboolean motion_notify_event_handler(GtkWidget *w, GdkEventMotion *event, gpointer user_data);
static gboolean key_press_event_handler(GtkWidget *w, GdkEventKey *event, gpointer user_data);
static void size_request_handler(GtkWidget *widget,
 				 GtkRequisition *requisition,
 				 gpointer user_data);

/* Declarations of the MVC handler functions: */
static void on_document_begin_edit(CongView *view);
static void on_document_end_edit(CongView *view);
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent);
static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling);
static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling);
static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent);
static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content);
static void on_document_node_set_attribute(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *name, const xmlChar *value);
static void on_document_node_remove_attribute(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *name);
static void on_selection_change(CongView *view);
static void on_cursor_change(CongView *view);



/* Implementations of public functions: */
GtkWidget* cong_editor_widget3_new(CongDocument *doc)
{
	CongEditorWidget3 *widget;
	CongEditorWidget3Details *details;
	CongEditorWidget3View *view;

	g_return_val_if_fail(doc, NULL);

	widget = GTK_DRAWING_AREA(gtk_drawing_area_new());

	details = g_new0(CongEditorWidget3Details,1);
	view = g_new0(CongEditorWidget3View,1);

	g_object_set_data(G_OBJECT(widget),
			  "details",
			  details);
	details->widget = widget;
	details->view = view;
	view->widget = widget;
	
	view->view.doc = doc;
	view->view.klass = g_new0(CongViewClass,1);
	view->view.klass->on_document_begin_edit = on_document_begin_edit;
	view->view.klass->on_document_end_edit = on_document_end_edit;
	view->view.klass->on_document_node_make_orphan = on_document_node_make_orphan;
	view->view.klass->on_document_node_add_after = on_document_node_add_after;
	view->view.klass->on_document_node_add_before = on_document_node_add_before;
	view->view.klass->on_document_node_set_parent = on_document_node_set_parent;
	view->view.klass->on_document_node_set_text = on_document_node_set_text;
	view->view.klass->on_document_node_set_attribute = on_document_node_set_attribute;
	view->view.klass->on_document_node_remove_attribute = on_document_node_remove_attribute;
	view->view.klass->on_selection_change = on_selection_change;
	view->view.klass->on_cursor_change = on_cursor_change;

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

#if 0
	populate_widget3(widget);
#endif


	return GTK_WIDGET(widget);
}

CongDocument *cong_editor_widget_get_document(CongEditorWidget3 *editor_widget)
{
	CongEditorWidget3Details *details;

	g_return_val_if_fail(editor_widget, NULL);

	details = GET_DETAILS(editor_widget);

	return details->view->view.doc;
}

CongDispspec *cong_editor_widget_get_dispspec(CongEditorWidget3 *editor_widget)
{
	CongEditorWidget3Details *details;

	g_return_val_if_fail(editor_widget, NULL);

	details = GET_DETAILS(editor_widget);

	return cong_document_get_dispspec(details->view->view.doc);
}

void cong_editor_widget_force_layout_update(CongEditorWidget3 *editor_widget)
{
	CongEditorWidget3Details *details;

	g_return_if_fail(editor_widget);

	details = GET_DETAILS(editor_widget);

	/* Recursively update all the size requisitions: */
	CONG_EDITOR_WIDGET3_DEBUG_MSG1("cong_editor_widget_force_layout_update");
 
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

/* Internal function implementations: */
/* Definitions of the widget event handlers: */
/* Event handlers for widget: */
static gboolean expose_event_handler(GtkWidget *w, GdkEventExpose *event, gpointer user_data)
{
	CongDocument *doc;
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(w);
	CongEditorWidget3Details* details = GET_DETAILS(editor_widget);

	CONG_EDITOR_WIDGET3_DEBUG_MSG1("expose_event_handler");

	doc = cong_editor_widget_get_document(editor_widget);

	/* Fill the rectangle with the background colour: */
	gdk_draw_rectangle(GDK_DRAWABLE(w->window),
			   w->style->white_gc,
			   TRUE, /* gint filled, */
			   event->area.x,
			   event->area.y,
			   event->area.width,
			   event->area.height);	

	/* Render the areas: */

	/* For now we render all of them */

	return TRUE;
}

static gboolean configure_event_handler(GtkWidget *w, GdkEventConfigure *event, gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(w);
	CongEditorWidget3Details* details = GET_DETAILS(editor_widget);

	CONG_EDITOR_WIDGET3_DEBUG_MSG3("configure_event_handler; w/h = %i,%i", event->width, event->height);
 

	return TRUE;
}

static gboolean button_press_event_handler(GtkWidget *w, GdkEventButton *event, gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(w);
	CongEditorWidget3Details* details = GET_DETAILS(editor_widget);

	CONG_EDITOR_WIDGET3_DEBUG_MSG1("button_press_event_handler");


	return TRUE;
}

static gboolean motion_notify_event_handler(GtkWidget *w, GdkEventMotion *event, gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(w);
	CongEditorWidget3Details* details = GET_DETAILS(editor_widget);

	CONG_EDITOR_WIDGET3_DEBUG_MSG1("motion_notify_event_handler");


	return TRUE;
}

static gboolean key_press_event_handler(GtkWidget *w, GdkEventKey *event, gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(w);
	CongEditorWidget3Details* details = GET_DETAILS(editor_widget);
	CongDocument *doc = cong_editor_widget_get_document(editor_widget);
	CongCursor *cursor = cong_document_get_cursor(doc);
	CongElementEditor *element_editor;

	CONG_EDITOR_WIDGET3_DEBUG_MSG1("key_press_event_handler");

	g_return_val_if_fail(cursor->location.node, FALSE);


	return TRUE;
}

static void size_request_handler(GtkWidget *widget,
 				 GtkRequisition *requisition,
 				 gpointer user_data)
{
 	CongDocument *doc;
 	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(widget);
 	CongEditorWidget3Details* details = GET_DETAILS(editor_widget);
 
 	CONG_EDITOR_WIDGET3_DEBUG_MSG1("size_request_handler");
 
 	g_assert(widget);
 	g_assert(requisition);
 
}

/* Definitions of the MVC handler functions: */
static void on_document_begin_edit(CongView *view)
{
}

static void on_document_end_edit(CongView *view)
{
}

static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent)
{
}

static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling)
{
}

static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling)
{
}

static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent)
{
}

static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content)
{
}

static void on_document_node_set_attribute(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *name, const xmlChar *value)
{
	/* UNWRITTEN */
}

static void on_document_node_remove_attribute(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *name)
{
	/* UNWRITTEN */
}

static void on_selection_change(CongView *view)
{
	CongEditorWidget3View *editor_widget_view;

	g_return_if_fail(view);

	#if DEBUG_EDITOR_WIDGET_VIEW
	CONG_EDITOR_WIDGET3_DEBUG_MSG1("CongEditorWidget3View - on_selection_change\n");
	#endif

	editor_widget_view = CONG_EDITOR_WIDGET3_VIEW(view);

	/* Force a redraw: */
	gtk_widget_queue_draw(GTK_WIDGET(editor_widget_view->widget));	
}

static void on_cursor_change(CongView *view)
{
	CongEditorWidget3View *editor_widget_view;

	g_return_if_fail(view);

	#if DEBUG_EDITOR_WIDGET_VIEW
	CONG_EDITOR_WIDGET3_DEBUG_MSG1("CongEditorWidget3View - on_cursor_change\n");
	#endif

	editor_widget_view = CONG_EDITOR_WIDGET3_VIEW(view);

	/* Force a redraw: */
	gtk_widget_queue_draw(GTK_WIDGET(editor_widget_view->widget));
}
