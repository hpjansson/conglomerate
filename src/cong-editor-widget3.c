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

#include "cong-editor-node-element-unknown.h"
#include "cong-editor-node-element-structural.h"
#include "cong-editor-node-element-span.h"
#include "cong-editor-node-text.h"
#include "cong-editor-node-document.h"
#include "cong-editor-node-unimplemented.h"
#include "cong-plugin.h"

#include "cong-app.h"
#include "cong-eel.h"

#include <gtk/gtkdrawingarea.h>

#include "cong-editor-area-flow-holder.h"

/* Test code: */
#include "cong-editor-area-border.h"
#include "cong-editor-area-composer.h"

#define SHOW_CURSOR_SPEW 0

/* 
   IMPLEMENTATION NOTES

   The CongEditorWidget3 maintains a hash table from xml doc nodes to CongEditorNodes.

   A CongEditorNode exists for an xml node iff the xml node exists within the tree i.e. you can currently
   trace a path back from the node up to root of the xml tree,

   The widget holds references to the CongEditorNodes it is storing in its hash table; these should (I think) be the only references
   on the EditorNodes.
*/

gboolean
cong_editor_widget3_node_should_have_editor_node (CongNodePtr node);

gboolean
cong_editor_widget3_has_editor_node_for_node (CongEditorWidget3 *widget,
					      CongNodePtr node);

#define PRIVATE(foo) ((foo)->private)

struct CongEditorWidget3Details
{
	CongDocument *doc;

	GHashTable *hash_of_node_to_editor;

	/* Record the "primary area" for each editor node: */
	GHashTable *hash_of_editor_node_to_primary_area;

	/* Record the area each editor node's primary area was inserted into: */
	GHashTable *hash_of_editor_node_to_parent_flow_holder;

	/* Record the "insetion area" for each editor node: */
	GHashTable *hash_of_editor_node_to_child_flow_holder;

	CongEditorArea *root_area;
	CongEditorAreaFlowHolder *root_flow_holder;

	GdkGC *test_gc;
};


#define DEBUG_EDITOR_WIDGET_VIEW  0
#define LOG_GTK_WIDGET_SIGNALS    1
#define LOG_CONG_DOCUMENT_SIGNALS 0
#define LOG_EDITOR_NODES 0
#define LOG_EDITOR_AREAS 1

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

#if LOG_GTK_WIDGET_SIGNALS
#define LOG_GTK_WIDGET_SIGNAL1(x) g_message(x)
#define LOG_GTK_WIDGET_SIGNAL3(x, a, b) g_message((x), (a), (b))
#else
#define LOG_GTK_WIDGET_SIGNAL1(x) ((void)0)
#define LOG_GTK_WIDGET_SIGNAL3(x, a, b) ((void)0)
#endif

#if LOG_CONG_DOCUMENT_SIGNALS
#define LOG_CONG_DOCUMENT_SIGNAL1(x) g_message(x)
#else
#define LOG_CONG_DOCUMENT_SIGNAL1(x) ((void)0)
#endif

#if LOG_EDITOR_NODES
#define LOG_EDITOR_NODE1(x) g_message(x)
#define LOG_EDITOR_NODE2(x, a) g_message((x), (a))
#else
#define LOG_EDITOR_NODE1(x) ((void)0)
#define LOG_EDITOR_NODE2(x, a) ((void)0)
#endif

#if LOG_EDITOR_AREAS
#define LOG_EDITOR_AREA1(x) g_message((x))
#define LOG_EDITOR_AREA2(x, a) g_message((x), (a))
#else
#define LOG_EDITOR_AREA1(x) ((void)0)
#define LOG_EDITOR_AREA2(x, a) ((void)0)
#endif

/* Declarations of misc stuff: */
static void 
render_area (CongEditorArea *area,
	     gpointer user_data);

static void 
populate_widget3(CongEditorWidget3 *widget);

static void 
recursive_add_nodes(CongEditorWidget3 *widget,
		    CongNodePtr node);		    

static void 
recursive_remove_nodes(CongEditorWidget3 *widget,
		       CongNodePtr node);


static void 
create_areas(CongEditorWidget3 *widget,
	     CongNodePtr node);
static void 
destroy_areas(CongEditorWidget3 *widget,
	      CongNodePtr node);

CongEditorArea*
cong_editor_widget3_get_primary_area_for_editor_node (CongEditorWidget3 *widget,
						      CongEditorNode *editor_node);
CongEditorAreaFlowHolder*
cong_editor_widget3_get_parent_flow_holder_for_editor_node (CongEditorWidget3 *widget,
							    CongEditorNode *editor_node);
CongEditorAreaFlowHolder*
cong_editor_widget3_get_child_flow_holder_for_editor_node (CongEditorWidget3 *widget,
							   CongEditorNode *editor_node);

/* Declarations of the GtkWidget event handlers: */
static gboolean expose_event_handler(GtkWidget *w, GdkEventExpose *event, gpointer user_data);
static gboolean configure_event_handler(GtkWidget *w, GdkEventConfigure *event, gpointer user_data);
static gboolean button_press_event_handler(GtkWidget *w, GdkEventButton *event, gpointer user_data);
static gboolean motion_notify_event_handler(GtkWidget *w, GdkEventMotion *event, gpointer user_data);
static gboolean key_press_event_handler(GtkWidget *w, GdkEventKey *event, gpointer user_data);
static void size_request_handler(GtkWidget *widget,
 				 GtkRequisition *requisition,
 				 gpointer user_data);

/* Declarations of the CongDocument event handlers: */
/* Signal handling callbacks: */
/* Callbacks attached before the default handler: */
static void on_signal_begin_edit_notify_before (CongDocument *doc,
					 gpointer user_data);
static void on_signal_end_edit_notify_before (CongDocument *doc,
				       gpointer user_data);
static void on_signal_make_orphan_notify_before (CongDocument *doc, 
					  CongNodePtr node, 
					  gpointer user_data);
static void on_signal_add_after_notify_before (CongDocument *doc, 
					CongNodePtr node, 
					CongNodePtr older_sibling, 
					gpointer user_data);
static void on_signal_add_before_notify_before (CongDocument *doc, 
					 CongNodePtr node, 
					 CongNodePtr younger_sibling, 
					 gpointer user_data);
static void on_signal_set_parent_notify_before (CongDocument *doc, 
					 CongNodePtr node, 
					 CongNodePtr adoptive_parent, 
					 gpointer user_data);
static void on_signal_set_text_notify_before (CongDocument *doc, 
				       CongNodePtr node, 
				       const xmlChar *new_content, 
				       gpointer user_data);
static void on_signal_set_attribute_notify_before (CongDocument *doc, 
					    CongNodePtr node, 
					    const xmlChar *name, 
					    const xmlChar *value, 
					    gpointer user_data);
static void on_signal_remove_attribute_notify_before (CongDocument *doc, 
					       CongNodePtr node, 
					       const xmlChar *name, 
					       gpointer user_data);
static void on_signal_selection_change_notify_before (CongDocument *doc, 
					       gpointer user_data);
static void on_signal_cursor_change_notify_before (CongDocument *doc, 
					    gpointer user_data);

/* Callbacks attached after the default handler: */
static void on_signal_begin_edit_notify_after (CongDocument *doc,
					gpointer user_data);
static void on_signal_end_edit_notify_after (CongDocument *doc,
				      gpointer user_data);
static void on_signal_make_orphan_notify_after (CongDocument *doc, 
					 CongNodePtr node, 
					 gpointer user_data);
static void on_signal_add_after_notify_after (CongDocument *doc, 
				       CongNodePtr node, 
				       CongNodePtr older_sibling, 
				       gpointer user_data);
static void on_signal_add_before_notify_after (CongDocument *doc, 
					CongNodePtr node, 
					CongNodePtr younger_sibling, 
					gpointer user_data);
static void on_signal_set_parent_notify_after (CongDocument *doc, 
					CongNodePtr node, 
					CongNodePtr adoptive_parent, 
					gpointer user_data);
static void on_signal_set_text_notify_after (CongDocument *doc, 
				      CongNodePtr node, 
				      const xmlChar *new_content, 
				      gpointer user_data);
static void on_signal_set_attribute_notify_after (CongDocument *doc, 
					   CongNodePtr node, 
					   const xmlChar *name, 
					   const xmlChar *value, 
					   gpointer user_data);
static void on_signal_remove_attribute_notify_after (CongDocument *doc, 
					      CongNodePtr node, 
					      const xmlChar *name, 
					      gpointer user_data);
static void on_signal_selection_change_notify_after (CongDocument *doc, 
					      gpointer user_data);
static void on_signal_cursor_change_notify_after (CongDocument *doc, 
					   gpointer user_data);

/* Declarations of CongEditorArea event handlers: */
static void
on_root_requisition_change (CongEditorArea *child_area,
			    gpointer user_data);

/* Implementations of public functions: */
GNOME_CLASS_BOILERPLATE(CongEditorWidget3, 
			cong_editor_widget3,
			GtkDrawingArea,
			GTK_TYPE_DRAWING_AREA );

static void
cong_editor_widget3_class_init (CongEditorWidget3Class *klass)
{
}

static void
cong_editor_widget3_instance_init (CongEditorWidget3 *widget)
{
	widget->private = g_new0(CongEditorWidget3Details,1);
}

CongEditorWidget3*
cong_editor_widget3_construct (CongEditorWidget3 *editor_widget,
			       CongDocument *doc)
{
	PRIVATE(editor_widget)->doc = doc;

	g_object_ref(G_OBJECT(doc));

	PRIVATE(editor_widget)->hash_of_node_to_editor = g_hash_table_new (NULL,
							    NULL);
	PRIVATE(editor_widget)->hash_of_editor_node_to_primary_area = g_hash_table_new (NULL,
									 NULL);
	PRIVATE(editor_widget)->hash_of_editor_node_to_parent_flow_holder = g_hash_table_new (NULL,
											      NULL);
	PRIVATE(editor_widget)->hash_of_editor_node_to_child_flow_holder = g_hash_table_new (NULL,
											     NULL);

	PRIVATE(editor_widget)->test_gc =  gdk_gc_new(cong_gui_get_a_window()->window);
	
	/* Connect to GtkWidget events: */
	gtk_signal_connect(GTK_OBJECT(editor_widget), 
			   "expose_event",
			   (GtkSignalFunc) expose_event_handler, 
			   NULL);
	gtk_signal_connect(GTK_OBJECT(editor_widget), 
			   "configure_event",
			   (GtkSignalFunc) configure_event_handler, 
			   NULL);
	gtk_signal_connect(GTK_OBJECT(editor_widget), 
			   "button_press_event",
			   (GtkSignalFunc) button_press_event_handler, 
			   NULL);
	gtk_signal_connect(GTK_OBJECT(editor_widget), 
			   "motion_notify_event",
			   (GtkSignalFunc) motion_notify_event_handler, 
			   NULL);
	gtk_signal_connect_after(GTK_OBJECT(editor_widget), 
				 "key_press_event",
				 (GtkSignalFunc) key_press_event_handler, 
				 NULL);
	gtk_signal_connect(GTK_OBJECT(editor_widget),
 			   "size-request",
 			   (GtkSignalFunc) size_request_handler,
 			   NULL);

	gtk_widget_set_events(GTK_WIDGET(editor_widget), GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_KEY_PRESS_MASK);

	gtk_widget_set(GTK_WIDGET(editor_widget), "can_focus", (gboolean) TRUE, 0);
	gtk_widget_set(GTK_WIDGET(editor_widget), "can_default", (gboolean) TRUE, 0);

	/* Set up root area: */
	{
		PRIVATE(editor_widget)->root_area = cong_editor_area_border_new (editor_widget, 5);
	
		g_signal_connect (G_OBJECT(PRIVATE(editor_widget)->root_area),
				  "flush_requisition_cache",
				  G_CALLBACK(on_root_requisition_change),
				  editor_widget);
		
		PRIVATE(editor_widget)->root_flow_holder = CONG_EDITOR_AREA_FLOW_HOLDER(cong_editor_area_flow_holder_new(editor_widget));

		cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER(PRIVATE(editor_widget)->root_area),
						      CONG_EDITOR_AREA(PRIVATE(editor_widget)->root_flow_holder));
	}

	/* Traverse the doc, adding EditorNodes and EditorAreas: */
	{
		populate_widget3(editor_widget);
	}

	/* Connect to CongDocument events: */
	{
		/* attach signal handlers to document for notification before change happens: */
		g_signal_connect (G_OBJECT(doc), "begin_edit", G_CALLBACK(on_signal_begin_edit_notify_before), editor_widget);
		g_signal_connect (G_OBJECT(doc), "end_edit", G_CALLBACK(on_signal_end_edit_notify_before), editor_widget);
		g_signal_connect (G_OBJECT(doc), "node_make_orphan", G_CALLBACK(on_signal_make_orphan_notify_before), editor_widget);
		g_signal_connect (G_OBJECT(doc), "node_add_after", G_CALLBACK(on_signal_add_after_notify_before), editor_widget);
		g_signal_connect (G_OBJECT(doc), "node_add_before", G_CALLBACK(on_signal_add_before_notify_before), editor_widget);
		g_signal_connect (G_OBJECT(doc), "node_set_parent", G_CALLBACK(on_signal_set_parent_notify_before), editor_widget);
		g_signal_connect (G_OBJECT(doc), "node_set_text", G_CALLBACK(on_signal_set_text_notify_before), editor_widget);
		g_signal_connect (G_OBJECT(doc), "node_set_attribute", G_CALLBACK(on_signal_set_attribute_notify_before), editor_widget);
		g_signal_connect (G_OBJECT(doc), "node_remove_attribute", G_CALLBACK(on_signal_remove_attribute_notify_before), editor_widget);
		g_signal_connect (G_OBJECT(doc), "selection_change", G_CALLBACK(on_signal_selection_change_notify_before), editor_widget);
		g_signal_connect (G_OBJECT(doc), "cursor_change", G_CALLBACK(on_signal_cursor_change_notify_before), editor_widget);
		
		/* attach signal handlers to document for notification after change happens: */
		g_signal_connect_after (G_OBJECT(doc), "begin_edit", G_CALLBACK(on_signal_begin_edit_notify_after), editor_widget);
		g_signal_connect_after (G_OBJECT(doc), "end_edit", G_CALLBACK(on_signal_end_edit_notify_after), editor_widget);
		g_signal_connect_after (G_OBJECT(doc), "node_make_orphan", G_CALLBACK(on_signal_make_orphan_notify_after), editor_widget);
		g_signal_connect_after (G_OBJECT(doc), "node_add_after", G_CALLBACK(on_signal_add_after_notify_after), editor_widget);
		g_signal_connect_after (G_OBJECT(doc), "node_add_before", G_CALLBACK(on_signal_add_before_notify_after), editor_widget);
		g_signal_connect_after (G_OBJECT(doc), "node_set_parent", G_CALLBACK(on_signal_set_parent_notify_after), editor_widget);
		g_signal_connect_after (G_OBJECT(doc), "node_set_text", G_CALLBACK(on_signal_set_text_notify_after), editor_widget);
		g_signal_connect_after (G_OBJECT(doc), "node_set_attribute", G_CALLBACK(on_signal_set_attribute_notify_after), editor_widget);
		g_signal_connect_after (G_OBJECT(doc), "node_remove_attribute", G_CALLBACK(on_signal_remove_attribute_notify_after), editor_widget);
		g_signal_connect_after (G_OBJECT(doc), "selection_change", G_CALLBACK(on_signal_selection_change_notify_after), editor_widget);
		g_signal_connect_after (G_OBJECT(doc), "cursor_change", G_CALLBACK(on_signal_cursor_change_notify_after), editor_widget);
	}

	return editor_widget;
}

GtkWidget* cong_editor_widget3_new(CongDocument *doc)
{
	CongEditorWidget3 *widget;
	CongEditorWidget3Details *details;

	g_return_val_if_fail(doc, NULL);

	return GTK_WIDGET( cong_editor_widget3_construct (g_object_new (CONG_EDITOR_WIDGET3_TYPE, NULL),
							  doc)
			   );
}

CongDocument *cong_editor_widget3_get_document(CongEditorWidget3 *editor_widget)
{
	g_return_val_if_fail(editor_widget, NULL);

	return PRIVATE(editor_widget)->doc;
}

CongDispspec *cong_editor_widget_get_dispspec(CongEditorWidget3 *editor_widget)
{
	g_return_val_if_fail(editor_widget, NULL);

	return cong_document_get_dispspec(PRIVATE(editor_widget)->doc);
}

#if 0
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
	cong_element_editor_get_size_requisition(PRIVATE(editor_widget)->root_editor, GTK_WIDGET(editor_widget)->allocation.width);

	gtk_widget_set_size_request(GTK_WIDGET(editor_widget),
				    PRIVATE(editor_widget)->root_editor->requisition.width,
				    PRIVATE(editor_widget)->root_editor->requisition.height);
	
	gtk_widget_queue_draw(GTK_WIDGET(editor_widget));
#endif
}
#endif

CongEditorNode*
cong_editor_widget3_get_editor_node (CongEditorWidget3 *editor_widget,
				     CongNodePtr node)
{
	g_return_val_if_fail (editor_widget, NULL);
	g_return_val_if_fail (node, NULL);

	return g_hash_table_lookup (PRIVATE(editor_widget)->hash_of_node_to_editor, 
				    node);
}

GdkGC*
cong_editor_widget3_get_test_gc (CongEditorWidget3 *editor_widget)
{
	g_return_val_if_fail (editor_widget, NULL);

	return PRIVATE(editor_widget)->test_gc;
}


CongEditorArea*
cong_editor_widget3_get_area_at (CongEditorWidget3 *editor_widget,
				 gint x,
				 gint y)
{
	return cong_editor_area_get_deepest_child_at (PRIVATE(editor_widget)->root_area, 
						      x,
						      y);
}

const gchar*
cong_flow_type_get_debug_string(enum CongFlowType flow_type)
{
	switch (flow_type) {
	default: g_assert_not_reached();
	case CONG_FLOW_TYPE_BLOCK: return "FLOW_TYPE_BLOCK";
	case CONG_FLOW_TYPE_INLINE: return "FLOW_TYPE_INLINE";
	}
}

/* Internal function implementations: */
/* Definitions of misc stuff: */
static void 
render_area (CongEditorArea *area,
	     gpointer user_data)
{
#if 0
	cong_editor_area_recursive_render (area,
					   (GdkRectangle*)user_data);
#endif
}

/* Definitions of the GtkWidget event handlers: */
/* Event handlers for widget: */
static gboolean expose_event_handler(GtkWidget *w, GdkEventExpose *event, gpointer user_data)
{
	CongDocument *doc;
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(w);

	LOG_GTK_WIDGET_SIGNAL1("expose_event_handler");

	doc = cong_editor_widget3_get_document(editor_widget);

	/* Fill the rectangle with the background colour: */
	gdk_draw_rectangle(GDK_DRAWABLE(w->window),
			   w->style->white_gc,
			   TRUE, /* gint filled, */
			   event->area.x,
			   event->area.y,
			   event->area.width,
			   event->area.height);	

	/* Render the areas: */
	cong_editor_area_recursive_render (PRIVATE(editor_widget)->root_area,
					   &event->area);

#if 0
	cong_editor_element_for_each_area (render_area,
					   &event->area);
#endif

	/* For now we render all of them */

	return TRUE;
}

static gboolean configure_event_handler(GtkWidget *w, GdkEventConfigure *event, gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(w);

	LOG_GTK_WIDGET_SIGNAL3("configure_event_handler; w/h = %i,%i", event->width, event->height);

#if 0
	if (event->width != cong_editor_area_get_cached_width_hint (PRIVATE(editor_widget)->root_area)) {
		
	}
#endif

#if 0
  	cong_editor_area_update_requisition(PRIVATE(editor_widget)->root_area, event->width);
#endif

	/* Pass all of the allocation to root editor; this will recursively allocate space to its children: */
	cong_editor_area_set_allocation (PRIVATE(editor_widget)->root_area, 
					 event->x,
					 event->y,
					 event->width,
					 event->height);
	return TRUE;
}

static gboolean button_press_event_handler(GtkWidget *w, GdkEventButton *event, gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(w);
	CongEditorArea* area;

	LOG_GTK_WIDGET_SIGNAL1("button_press_event_handler");

	/* Try deepest area, then next deepest, etc until something handles the signal */
	area = cong_editor_widget3_get_area_at (editor_widget,
						event->x,
						event->y);

	while (area) {

#if 0		
		g_message("Trying button_press on %p %s", 
			  area,
			  G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(G_OBJECT(area))));
#endif

		if (cong_editor_area_on_button_press (area, event)) {
			/* This area handled the event: */
			return TRUE;
		}

		area = cong_editor_area_get_parent (area);
	}

	/* None of the areas handled the click: */
	return FALSE;
}

static gboolean motion_notify_event_handler(GtkWidget *w, GdkEventMotion *event, gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(w);
	CongEditorArea* area;

	LOG_GTK_WIDGET_SIGNAL1("motion_notify_event_handler");

#if 1
	/* Try deepest area, then next deepest, etc until something handles the signal */
	area = cong_editor_widget3_get_area_at (editor_widget,
						event->x,
						event->y);

	while (area) {
		
#if 0
		g_message("Trying motion_notify on %p %s", 
			  area,
			  G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(G_OBJECT(area))));
#endif

		if (cong_editor_area_on_motion_notify (area, event)) {
			/* This area handled the event: */
			return TRUE;
		}

		area = cong_editor_area_get_parent (area);
	}

	/* None of the areas handled the click: */
	return FALSE;
#else
	{
		CongEditorArea* deepest_area = cong_editor_widget3_get_area_at (editor_widget,
										event->x,
										event->y);

		g_message("deepest area: %p (%s)", deepest_area, G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(G_OBJECT(deepest_area))));
	}

	return TRUE;
#endif
}

static gboolean key_press_event_handler(GtkWidget *w, GdkEventKey *event, gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(w);
	CongDocument *doc = cong_editor_widget3_get_document(editor_widget);
	CongCursor *cursor = cong_document_get_cursor(doc);
	CongElementEditor *element_editor;

	LOG_GTK_WIDGET_SIGNAL1("key_press_event_handler");

	g_return_val_if_fail(cursor->location.node, FALSE);


	return TRUE;
}

static void size_request_handler(GtkWidget *widget,
 				 GtkRequisition *requisition,
 				 gpointer user_data)
{
 	CongDocument *doc;
 	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(widget);
	const GtkRequisition* req;
 
 	LOG_GTK_WIDGET_SIGNAL1("size_request_handler");

 	g_assert(widget);
 	g_assert(requisition);

	req = cong_editor_area_get_requisition (PRIVATE(editor_widget)->root_area,
						widget->allocation.width); 

	/* Only request up to the width you've already been allocated; don't grow in width unless your container gives you more room. */
	if (req->width > widget->allocation.width) {
		requisition->width = widget->allocation.width;
	} else {
		requisition->width = req->width;
	}
 	requisition->height = req->height;
}

/* Definitions of the CongDocument event handlers: */
/* Signal handling callbacks: */
/* Callbacks attached before the default handler: */

static void on_signal_begin_edit_notify_before (CongDocument *doc,
					 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_begin_edit_notify_before");

	/* empty so far */
}

static void on_signal_end_edit_notify_before (CongDocument *doc,
				       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_end_edit_notify_before");

	/* empty so far */
}

static void on_signal_make_orphan_notify_before (CongDocument *doc, 
						 CongNodePtr node, 
						 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_make_orphan_notify_before");

#if 1
	if (cong_editor_widget3_node_should_have_editor_node(node)) {
		recursive_remove_nodes(editor_widget, node);
	} else {
		g_assert(!cong_editor_widget3_has_editor_node_for_node(editor_widget, node));
	}
#endif
}

static void on_signal_add_after_notify_before (CongDocument *doc, 
					CongNodePtr node, 
					CongNodePtr older_sibling, 
					gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_add_after_notify_before");

	/* empty so far */
}

static void on_signal_add_before_notify_before (CongDocument *doc, 
					 CongNodePtr node, 
					 CongNodePtr younger_sibling, 
					 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_add_before_notify_before");

	/* empty so far */
}

static void on_signal_set_parent_notify_before (CongDocument *doc, 
					 CongNodePtr node, 
					 CongNodePtr adoptive_parent, 
					 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_set_parent_notify_before");

	if (cong_editor_widget3_node_should_have_editor_node(node)) {
		recursive_remove_nodes(editor_widget, node);
	} else {
		g_assert(!cong_editor_widget3_has_editor_node_for_node(editor_widget, node));
	}
}

static void on_signal_set_text_notify_before (CongDocument *doc, 
				       CongNodePtr node, 
				       const xmlChar *new_content, 
				       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_set_text_notify_before");

	/* empty so far */
}

static void on_signal_set_attribute_notify_before (CongDocument *doc, 
					    CongNodePtr node, 
					    const xmlChar *name, 
					    const xmlChar *value, 
					    gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_set_attribute_notify_before");

	/* empty so far */
}

static void on_signal_remove_attribute_notify_before (CongDocument *doc, 
					       CongNodePtr node, 
					       const xmlChar *name, 
					       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_remove_attribute_notify_before");

	/* empty so far */
}

static void on_signal_selection_change_notify_before (CongDocument *doc, 
					       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_selection_change_notify_before");

	/* empty so far */
}

static void on_signal_cursor_change_notify_before (CongDocument *doc, 
					    gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

#if SHOW_CURSOR_SPEW
	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_cursor_change_notify_before");
#endif

	/* empty so far */
}


/* Callbacks attached after the default handler: */
static void on_signal_begin_edit_notify_after (CongDocument *doc,
					 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_begin_edit_notify_after");

	/* empty so far */
}

static void on_signal_end_edit_notify_after (CongDocument *doc,
				       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_end_edit_notify_after");

	/* empty so far */
}

static void on_signal_make_orphan_notify_after (CongDocument *doc, 
					 CongNodePtr node, 
					 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 
	
	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_make_orphan_notify_after");

	/* empty so far */
}

static void on_signal_add_after_notify_after (CongDocument *doc, 
					CongNodePtr node, 
					CongNodePtr older_sibling, 
					gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_add_after_notify_after");

#if 1
	recursive_add_nodes(editor_widget, node);
#endif
}

static void on_signal_add_before_notify_after (CongDocument *doc, 
					 CongNodePtr node, 
					 CongNodePtr younger_sibling, 
					 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_add_before_notify_after");

#if 1
	recursive_add_nodes(editor_widget, node);
#endif
}

static void on_signal_set_parent_notify_after (CongDocument *doc, 
					 CongNodePtr node, 
					 CongNodePtr adoptive_parent, 
					 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_set_parent_notify_after");

#if 1
	recursive_add_nodes(editor_widget, node);
#endif
}

static void on_signal_set_text_notify_after (CongDocument *doc, 
				       CongNodePtr node, 
				       const xmlChar *new_content, 
				       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_set_text_notify_after");

	/* empty so far */
}

static void on_signal_set_attribute_notify_after (CongDocument *doc, 
					   CongNodePtr node, 
					   const xmlChar *name, 
					   const xmlChar *value, 
					   gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_set_attribute_notify_after");

	/* empty so far */
}

static void on_signal_remove_attribute_notify_after (CongDocument *doc, 
					       CongNodePtr node, 
					       const xmlChar *name, 
					       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_remove_attribute_notify_after");

	/* empty so far */
}

static void on_signal_selection_change_notify_after (CongDocument *doc, 
					       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_selection_change_notify_after");

	/* empty so far */

#if 0
	/* Force a redraw: */
	gtk_widget_queue_draw(GTK_WIDGET(editor_widget));	
#endif
}

static void on_signal_cursor_change_notify_after (CongDocument *doc, 
					    gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = (CongEditorWidget3*)user_data; 

#if SHOW_CURSOR_SPEW
	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_cursor_change_notify_after");
#endif

	/* empty so far */

#if 0
	/* Force a redraw: */
	gtk_widget_queue_draw(GTK_WIDGET(editor_widget));	
#endif
}

/* Definitions of CongEditorArea event handlers: */
static void
on_root_requisition_change (CongEditorArea *child_area,
			    gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data);

	g_message("on_root_requisition_change");
	gtk_widget_queue_resize (GTK_WIDGET(editor_widget));
}

static void 
populate_widget3(CongEditorWidget3 *widget)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(widget);
	CongDocument *doc;

	doc = cong_editor_widget3_get_document(widget);

	recursive_add_nodes (widget,
			     (CongNodePtr)cong_document_get_xml (doc));		
}

gboolean
cong_editor_widget3_node_should_have_editor_node (CongNodePtr node)
{
	g_return_if_fail (node);

	/* CongNodePtrs should have CongEditorNodes iff they are part of a tree going up to the document node */

	if (node->parent) {
		return cong_editor_widget3_node_should_have_editor_node(node->parent);
	} else {
		return cong_node_type(node)==CONG_NODE_TYPE_DOCUMENT;
	}
}

gboolean
cong_editor_widget3_has_editor_node_for_node (CongEditorWidget3 *widget,
					      CongNodePtr node)
{
	return 	g_hash_table_lookup(PRIVATE(widget)->hash_of_node_to_editor,node)!=NULL;

}

static void 
recursive_add_nodes(CongEditorWidget3 *widget,
		    CongNodePtr node)
{
	CongDocument *doc;
	CongNodePtr iter;

	doc = cong_editor_widget3_get_document(widget);

#if LOG_EDITOR_NODES
	{
		gchar *node_desc = cong_node_debug_description(node);

		LOG_EDITOR_NODE2("recursive_add_nodes: %s", node_desc);

		g_free(node_desc);
	}
#endif

	g_assert(cong_editor_widget3_node_should_have_editor_node(node));

	/* Add this node: */
	{
		enum CongNodeType type = cong_node_type(node);
		CongEditorNode *editor_node = NULL;

		switch (type) {
		default: g_assert_not_reached();
		case CONG_NODE_TYPE_ELEMENT:
			{
				CongDispspecElement *ds_element = cong_document_get_dispspec_element_for_node (doc, 
													       node);

				if (ds_element) {
					switch (cong_dispspec_element_type(ds_element)) {
					default: g_assert_not_reached();
					case CONG_ELEMENT_TYPE_STRUCTURAL:
						editor_node = cong_editor_node_element_structural_new (widget,
												       node);
						break;

					case CONG_ELEMENT_TYPE_SPAN:
						editor_node = cong_editor_node_element_span_new (widget,
												 node);
						break;

					case CONG_ELEMENT_TYPE_INSERT:
					case CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE:
					case CONG_ELEMENT_TYPE_PARAGRAPH:
						editor_node = cong_editor_node_element_unknown_new (widget,
												    node);
						break;

					case CONG_ELEMENT_TYPE_PLUGIN:
						{
							const gchar *plugin_id = cong_dispspec_element_get_editor_plugin_id (ds_element);
							CongPluginEditorNodeFactory* factory = cong_plugin_manager_locate_editor_node_factory_by_id (cong_app_singleton()->plugin_manager,
																		     plugin_id);
							
							if (factory) {
								editor_node = CONG_EDITOR_NODE( cong_plugin_editor_node_factory_invoke (factory,
																	widget, 
																	node));
							} else {
								g_message("plugin not found \"%s\"", plugin_id);
								editor_node = cong_editor_node_element_unknown_new (widget,
														    node);
							}							
						}
						break;

					case CONG_ELEMENT_TYPE_UNKNOWN:
						editor_node = cong_editor_node_element_unknown_new (widget,
												    node);
						break;
					} 
				} else {
					editor_node = cong_editor_node_element_unknown_new (widget,
											    node);
				}
			}
			break;

		case CONG_NODE_TYPE_ATTRIBUTE:
			{
				editor_node = cong_editor_node_unimplemented_new (widget, 
										  node,
										  cong_node_type_description (type));
			}
			break;

		case CONG_NODE_TYPE_TEXT:
			{
				editor_node = cong_editor_node_text_new (widget, 
									 node);
			}
			break;

		case CONG_NODE_TYPE_CDATA_SECTION:
		case CONG_NODE_TYPE_ENTITY_REF:
		case CONG_NODE_TYPE_ENTITY_NODE:
		case CONG_NODE_TYPE_PI:
		case CONG_NODE_TYPE_COMMENT:			
			{
				editor_node = cong_editor_node_unimplemented_new (widget, 
										  node,
										  cong_node_type_description (type));
			}
			break;

		case CONG_NODE_TYPE_DOCUMENT:
			{
				editor_node = cong_editor_node_document_new (widget, 
									     node);
			}
			break;
		case CONG_NODE_TYPE_DOCUMENT_TYPE:
		case CONG_NODE_TYPE_DOCUMENT_FRAG:
		case CONG_NODE_TYPE_NOTATION:
		case CONG_NODE_TYPE_HTML_DOCUMENT:
		case CONG_NODE_TYPE_DTD:
		case CONG_NODE_TYPE_ELEMENT_DECL:
		case CONG_NODE_TYPE_ATRRIBUTE_DECL:
		case CONG_NODE_TYPE_ENTITY_DECL:
		case CONG_NODE_TYPE_NAMESPACE_DECL:
		case CONG_NODE_TYPE_XINCLUDE_START:
		case CONG_NODE_TYPE_XINCLUDE_END:
			{
				editor_node = cong_editor_node_unimplemented_new (widget, 
										  node,
										  cong_node_type_description (type));
			}
			break;
		}

		g_assert(editor_node);

		g_hash_table_insert (PRIVATE(widget)->hash_of_node_to_editor, 
				     node,
				     editor_node);

		create_areas (widget,
			      node);
	}
	
	/* Recurse: */
	for (iter = node->children; iter; iter=iter->next) {
		recursive_add_nodes (widget, 
				     iter);		
	}
}

static void 
recursive_remove_nodes (CongEditorWidget3 *widget,
			CongNodePtr node)
{
	CongEditorNode *editor_node;
	CongNodePtr iter;

#if LOG_EDITOR_NODES
	{
		gchar *node_desc = cong_node_debug_description(node);

		LOG_EDITOR_NODE2("recursive_remove_nodes: %s", node_desc);

		g_free(node_desc);
	}
#endif

	g_assert(cong_editor_widget3_node_should_have_editor_node(node));

	g_assert(g_hash_table_lookup(PRIVATE(widget)->hash_of_node_to_editor,node)!=NULL);
	
	/* Recurse: */
	for (iter = node->children; iter; iter=iter->next) {
		recursive_remove_nodes (widget, 
					iter);		
	}
	
	destroy_areas (widget,
		       node);
	
	editor_node = g_hash_table_lookup (PRIVATE(widget)->hash_of_node_to_editor,
					   node);
	
	/* Remove this editor_node: */
	g_hash_table_remove (PRIVATE(widget)->hash_of_node_to_editor, 
			     node);

	CONG_EEL_LOG_REF_COUNT("redundant editor node", G_OBJECT(editor_node));
	
	/* Unref the editor_node: */
	g_object_unref (editor_node);
}

static void 
create_areas(CongEditorWidget3 *widget,
	     CongNodePtr node)
{
	CongEditorNode *editor_node = NULL;
	CongEditorArea *this_area = NULL;
	CongEditorAreaFlowHolder* parent_flow_holder = NULL;
	CongEditorNode *older_sibling_node = NULL;
	CongEditorArea *older_sibling_primary_area = NULL;
	enum CongFlowType flow_type;

#if LOG_EDITOR_AREAS
	{
		gchar *node_desc = cong_node_debug_description(node);

		LOG_EDITOR_AREA2("create_areas for %s", node_desc);

		g_free(node_desc);
	}
#endif

	editor_node = cong_editor_widget3_get_editor_node (widget,
							   node);

	flow_type = cong_editor_node_get_flow_type (editor_node);

#if LOG_EDITOR_AREAS
	g_message("flow type = %s", cong_flow_type_get_debug_string(flow_type));
#endif

	/* Determine the parent area where the new area should be inserted: */
	{
		if (node->parent) {
			CongEditorNode *parent_editor_node;
			
			parent_editor_node = cong_editor_widget3_get_editor_node (widget,
										  node->parent);
			
			parent_flow_holder = cong_editor_widget3_get_child_flow_holder_for_editor_node (widget,
													parent_editor_node);
			
		} else {
			/* Root of the document; insert below the widget's root_area: */
			g_assert(cong_node_type(node) == CONG_NODE_TYPE_DOCUMENT);

			parent_flow_holder = PRIVATE(widget)->root_flow_holder;			
		}

		g_hash_table_insert (PRIVATE(widget)->hash_of_editor_node_to_parent_flow_holder,
				     editor_node,
				     parent_flow_holder);
		
		g_assert(parent_flow_holder);
	}

	this_area = cong_editor_area_flow_holder_insert_areas_for_node (parent_flow_holder,
									editor_node);	

	g_hash_table_insert (PRIVATE(widget)->hash_of_editor_node_to_primary_area,
			     editor_node,
			     this_area);

	/* If this node can ever have children, we need to add a container for them:
	   FIXME:  slightly hackish test */
	if (IS_CONG_EDITOR_AREA_CONTAINER(this_area) ) {
		CongEditorAreaFlowHolder *flow_holder;

		flow_holder = CONG_EDITOR_AREA_FLOW_HOLDER(cong_editor_area_flow_holder_new (widget));

		cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER (this_area),
						      CONG_EDITOR_AREA(flow_holder));

		g_hash_table_insert (PRIVATE(widget)->hash_of_editor_node_to_child_flow_holder,
				     editor_node,
				     flow_holder);
	}
}

static void 
destroy_areas(CongEditorWidget3 *widget,
	      CongNodePtr node)
{
	CongEditorNode *editor_node;
	CongEditorArea *this_area;
	CongEditorAreaFlowHolder *parent_flow_holder;

#if LOG_EDITOR_AREAS
	{
		gchar *node_desc = cong_node_debug_description(node);

		LOG_EDITOR_AREA2("destroy_areas for %s", node_desc);
		
		g_free(node_desc);
	}
#endif

	editor_node = cong_editor_widget3_get_editor_node (widget,
							   node);

	if (node->parent) {
		parent_flow_holder = cong_editor_widget3_get_parent_flow_holder_for_editor_node (widget,
												 editor_node);
	} else {
		g_assert(cong_node_type(node) == CONG_NODE_TYPE_DOCUMENT);
		parent_flow_holder = PRIVATE(widget)->root_flow_holder;
	}

	cong_editor_area_flow_holder_remove_areas_for_node (parent_flow_holder,
							    editor_node);

	g_hash_table_remove (PRIVATE(widget)->hash_of_editor_node_to_primary_area,
			     editor_node);
	g_hash_table_remove (PRIVATE(widget)->hash_of_editor_node_to_parent_flow_holder,
			     editor_node);
	g_hash_table_remove (PRIVATE(widget)->hash_of_editor_node_to_child_flow_holder,
			     editor_node);


#if 0
	cong_editor_area_container_remove_child (CONG_EDITOR_AREA_CONTAINER (this_area),
						 vcomposer);
#endif
}

CongEditorArea*
cong_editor_widget3_get_primary_area_for_editor_node (CongEditorWidget3 *widget,
						      CongEditorNode *editor_node)
{
	g_return_val_if_fail (widget, NULL);
	g_return_val_if_fail (IS_CONG_EDITOR_NODE(editor_node), NULL);

	return CONG_EDITOR_AREA(g_hash_table_lookup (PRIVATE(widget)->hash_of_editor_node_to_primary_area,
						     editor_node));
}

CongEditorAreaFlowHolder*
cong_editor_widget3_get_parent_flow_holder_for_editor_node (CongEditorWidget3 *widget,
							    CongEditorNode *editor_node)
{
	g_return_val_if_fail (widget, NULL);
	g_return_val_if_fail (IS_CONG_EDITOR_NODE(editor_node), NULL);

	return CONG_EDITOR_AREA_FLOW_HOLDER(g_hash_table_lookup (PRIVATE(widget)->hash_of_editor_node_to_parent_flow_holder,
								 editor_node));
}

CongEditorAreaFlowHolder*
cong_editor_widget3_get_child_flow_holder_for_editor_node (CongEditorWidget3 *widget,
							   CongEditorNode *editor_node)
{
	g_return_val_if_fail (widget, NULL);
	g_return_val_if_fail (IS_CONG_EDITOR_NODE(editor_node), NULL);

	return CONG_EDITOR_AREA_FLOW_HOLDER(g_hash_table_lookup (PRIVATE(widget)->hash_of_editor_node_to_child_flow_holder,
								editor_node));
}
