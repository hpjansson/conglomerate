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

/* Test code: */
#include "cong-app.h"
#include "cong-editor-area-border.h"
#include "cong-editor-area-text.h"
#include "cong-editor-area-composer.h"
#include "cong-editor-area-spacer.h"
#include "cong-editor-area-bin.h"
#include "cong-editor-area-unknown-tag.h"

#define PRIVATE(foo) ((foo)->private)

struct CongEditorWidget3Details
{
	CongDocument *doc;
#if 0
	CongEditorWidget3 *widget;
#endif

	GHashTable *hash_of_node_to_editor;

	CongEditorArea *test_area;

	GdkGC *test_gc;
};


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

/* Declarations of misc stuff: */
static void 
render_area (CongEditorArea *area,
	     gpointer user_data);

static void 
populate_widget3(CongEditorWidget3 *widget);

static void 
create_areas(CongEditorWidget3 *widget);

static void 
recursive_add_nodes(CongEditorWidget3 *widget,
		    CongNodePtr node);		    

static void 
recursive_remove_nodes(CongEditorWidget3 *widget,
		       CongNodePtr node);


static void 
recursive_create_areas(CongEditorWidget3 *widget,
		       CongNodePtr node,
		       CongEditorArea *parent_area);

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
static void 
on_signal_set_text_notify_after (CongDocument *doc, 
				 CongNodePtr node, 
				 const xmlChar *new_content, 
				 gpointer user_data);

static void 
on_signal_set_attribute_notify_after (CongDocument *doc, 
				      CongNodePtr node, 
				      const xmlChar *name, 
				      const xmlChar *value, 
				      gpointer user_data); 

static void 
on_signal_remove_attribute_notify_after (CongDocument *doc, 
					 CongNodePtr node, 
					 const xmlChar *name, 
					 gpointer user_data); 

/* Declarations of CongEditorArea event handlers: */
static void
on_root_requisition_change (CongEditorArea *child_area,
			    gpointer user_data);

#if 0
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
#endif

/* Implementations of public functions: */
GtkWidget* cong_editor_widget3_new(CongDocument *doc)
{
	CongEditorWidget3 *widget;
	CongEditorWidget3Details *details;

	g_return_val_if_fail(doc, NULL);

	widget = GTK_DRAWING_AREA(gtk_drawing_area_new());

	details = g_new0(CongEditorWidget3Details,1);

	g_object_set_data(G_OBJECT(widget),
			  "details",
			  details);

	details->doc = doc;
	g_object_ref(G_OBJECT(doc));

	details->hash_of_node_to_editor = g_hash_table_new (NULL,
							    NULL);

	details->test_gc =  gdk_gc_new(cong_gui_get_a_window()->window);
	
	/* Connect to GtkWidget events: */
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


#if 1
	populate_widget3(widget);
#endif


#if 1
#if 1
#if 1
	create_areas(widget);
#else
	{
		int i,j;
		CongEditorArea *outer_area;
		CongEditorArea *vcompose_outer;

		details->test_area = cong_editor_area_unknown_tag_new (widget,
								       "foo");

		vcompose_outer = cong_editor_area_composer_new (widget,
								GTK_ORIENTATION_VERTICAL,
								5);

		cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER(details->test_area),
						       vcompose_outer);

		for (i=0;i<10;i++) {
			CongEditorArea *middle_tag = cong_editor_area_unknown_tag_new (widget,
										      "bar");
			
			CongEditorArea *vcompose_inner = cong_editor_area_composer_new (widget,
											GTK_ORIENTATION_VERTICAL,
											5);

			cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER(vcompose_outer),
							       middle_tag);
			cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER(middle_tag),
							       vcompose_inner);
			
			for (j=0;j<5;j++) {
				cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER(vcompose_inner),
								       cong_editor_area_unknown_tag_new (widget,
													 "fubar")
								       );
				
			}			
		}
	}
#endif
#else
	{
		int i;

		details->test_area = cong_editor_area_composer_new (widget,
								    GTK_ORIENTATION_VERTICAL,
								    10);

		for (i=0;i<10;i++) {
			cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER(details->test_area),
							       cong_editor_area_text_new (widget,
											  cong_app_singleton()->fonts[CONG_FONT_ROLE_TITLE_TEXT], 
											  g_strdup_printf("this is a test string %i",i))
							       );
		}
	}
#endif
#else
	details->test_area = cong_editor_area_text_new (widget,
							cong_app_singleton()->fonts[CONG_FONT_ROLE_TITLE_TEXT], 
							"this is a test");
#endif

	/* Connect to CongDocument events: */
	{
	}

	return GTK_WIDGET(widget);
}

CongDocument *cong_editor_widget3_get_document(CongEditorWidget3 *editor_widget)
{
	CongEditorWidget3Details *details;

	g_return_val_if_fail(editor_widget, NULL);

	details = GET_DETAILS(editor_widget);

	return details->doc;
}

CongDispspec *cong_editor_widget_get_dispspec(CongEditorWidget3 *editor_widget)
{
	CongEditorWidget3Details *details;

	g_return_val_if_fail(editor_widget, NULL);

	details = GET_DETAILS(editor_widget);

	return cong_document_get_dispspec(details->doc);
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
	cong_element_editor_get_size_requisition(details->root_editor, GTK_WIDGET(editor_widget)->allocation.width);

	gtk_widget_set_size_request(GTK_WIDGET(editor_widget),
				    details->root_editor->requisition.width,
				    details->root_editor->requisition.height);
	
	gtk_widget_queue_draw(GTK_WIDGET(editor_widget));
#endif
}
#endif

CongEditorNode*
cong_editor_widget3_get_editor_node (CongEditorWidget3 *editor_widget,
				     CongNodePtr node)
{
	CongEditorWidget3Details *details;

	g_return_val_if_fail (editor_widget, NULL);
	g_return_val_if_fail (node, NULL);

	details = GET_DETAILS(editor_widget);

	return g_hash_table_lookup (details->hash_of_node_to_editor, 
				    node);
}

GdkGC*
cong_editor_widget3_get_test_gc (CongEditorWidget3 *editor_widget)
{
	CongEditorWidget3Details *details;

	g_return_val_if_fail (editor_widget, NULL);

	details = GET_DETAILS(editor_widget);
	
	return details->test_gc;
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
	CongEditorWidget3Details* details = GET_DETAILS(editor_widget);

	CONG_EDITOR_WIDGET3_DEBUG_MSG1("expose_event_handler");

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
	cong_editor_area_recursive_render (details->test_area,
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
	CongEditorWidget3Details* details = GET_DETAILS(editor_widget);

	CONG_EDITOR_WIDGET3_DEBUG_MSG3("configure_event_handler; w/h = %i,%i", event->width, event->height);

#if 0
	if (event->width != cong_editor_area_get_cached_width_hint (details->test_area)) {
		
	}
#endif

#if 0
  	cong_editor_area_update_requisition(details->test_area, event->width);
#endif

	/* Pass all of the allocation to root editor; this will recursively allocate space to its children: */
	cong_editor_area_set_allocation (details->test_area, 
					 event->x,
					 event->y,
					 event->width,
					 event->height);
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
	CongDocument *doc = cong_editor_widget3_get_document(editor_widget);
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
	const GtkRequisition* req;
 
 	CONG_EDITOR_WIDGET3_DEBUG_MSG1("size_request_handler");

 	g_assert(widget);
 	g_assert(requisition);

	req = cong_editor_area_get_requisition (details->test_area,
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
#if 0
static void 
on_signal_set_text_notify_after (CongDocument *doc, 
				  CongNodePtr node, 
				  const xmlChar *new_content, 
				  gpointer user_data)
{
	g_assert_not_reached();
}

static void 
on_signal_set_attribute_notify_after (CongDocument *doc, 
				       CongNodePtr node, 
				       const xmlChar *name, 
				       const xmlChar *value, 
				       gpointer user_data)
{
	g_assert_not_reached();
}

static void 
on_signal_remove_attribute_notify_after (CongDocument *doc, 
					  CongNodePtr node, 
					  const xmlChar *name, 
					  gpointer user_data)
{
	g_assert_not_reached();
}
#endif

/* Definitions of CongEditorArea event handlers: */
static void
on_root_requisition_change (CongEditorArea *child_area,
			    gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data);

	g_message("on_root_requisition_change");
	gtk_widget_queue_resize (GTK_WIDGET(editor_widget));
}


/* Definitions of the MVC handler functions: */
#if 0
static void on_document_begin_edit(CongView *view)
{
	/* UNWRITTEN */
}

static void on_document_end_edit(CongView *view)
{
	/* UNWRITTEN */
}

static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent)
{
	if (before_event) {
		recursive_remove_nodes(CONG_EDITOR_WIDGET3_VIEW(view)->widget, node);
	}

}

static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling)
{
	if (before_event) {
		return;
	}

	recursive_add_nodes(CONG_EDITOR_WIDGET3_VIEW(view)->widget, node);
}

static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling)
{
	if (before_event) {
		return;
	}

	recursive_add_nodes(CONG_EDITOR_WIDGET3_VIEW(view)->widget, node);
}

static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent)
{
	if (before_event) {
		recursive_remove_nodes(CONG_EDITOR_WIDGET3_VIEW(view)->widget, node);
	} else {
		recursive_add_nodes(CONG_EDITOR_WIDGET3_VIEW(view)->widget, node);
	}
}

static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content)
{
	/* UNWRITTEN */
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

#if 0
	/* Force a redraw: */
	gtk_widget_queue_draw(GTK_WIDGET(editor_widget_view->widget));	
#endif
}

static void on_cursor_change(CongView *view)
{
	CongEditorWidget3View *editor_widget_view;

	g_return_if_fail(view);

	#if DEBUG_EDITOR_WIDGET_VIEW
	CONG_EDITOR_WIDGET3_DEBUG_MSG1("CongEditorWidget3View - on_cursor_change\n");
	#endif

	editor_widget_view = CONG_EDITOR_WIDGET3_VIEW(view);

#if 0
	/* Force a redraw: */
	gtk_widget_queue_draw(GTK_WIDGET(editor_widget_view->widget));
#endif
}
#endif

static void 
populate_widget3(CongEditorWidget3 *widget)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(widget);
	CongEditorWidget3Details *details = GET_DETAILS(editor_widget);
	CongDocument *doc;

	doc = cong_editor_widget3_get_document(widget);

	recursive_add_nodes (widget,
			     (CongNodePtr)cong_document_get_xml (doc));		
}

static void 
create_areas(CongEditorWidget3 *widget)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(widget);
	CongEditorWidget3Details *details = GET_DETAILS(editor_widget);
	CongDocument *doc;

	doc = cong_editor_widget3_get_document(widget);

	details->test_area = cong_editor_area_border_new (widget, 5);

	recursive_create_areas (widget,
				(CongNodePtr)cong_document_get_xml (doc),
				details->test_area);

	
	g_signal_connect (G_OBJECT(details->test_area),
			  "flush_requisition_cache",
			  G_CALLBACK(on_root_requisition_change),
			  widget);
}




static void 
recursive_add_nodes(CongEditorWidget3 *widget,
		    CongNodePtr node)
{
	CongEditorWidget3Details *details = GET_DETAILS(widget);
	CongDocument *doc;
	CongNodePtr iter;

	doc = cong_editor_widget3_get_document(widget);

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
					case CONG_ELEMENT_TYPE_PLUGIN:
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

		g_hash_table_insert (details->hash_of_node_to_editor, 
				     node,
				     editor_node);
	}
	
	/* Recurse: */
	for (iter = node->children; iter; iter=iter->next) {
		recursive_add_nodes (widget, 
				     iter);		
	}
}


static void 
recursive_remove_nodes(CongEditorWidget3 *widget,
		       CongNodePtr node)
{
	CongEditorWidget3Details *details = GET_DETAILS(widget);
	CongEditorNode *editor_node;
	CongNodePtr iter;

	/* Recurse: */
	for (iter = node->children; iter; iter=iter->next) {
		recursive_remove_nodes (widget, 
					iter);		
	}

	editor_node = g_hash_table_lookup (details->hash_of_node_to_editor,
					  node);

	/* Remove this editor_node: */
	g_hash_table_remove (details->hash_of_node_to_editor, 
			     node);

	/* Unref the editor_node: */
	g_object_unref (editor_node);
}

static void 
recursive_create_areas(CongEditorWidget3 *widget,
		       CongNodePtr node,
		       CongEditorArea *parent_area)
{
	CongEditorArea *vcomposer;
	CongNodePtr iter;
	CongEditorNode *editor_node = cong_editor_widget3_get_editor_node (widget,
									   node);

	CongEditorArea *this_area = cong_editor_node_generate_area (editor_node);

	if (IS_CONG_EDITOR_AREA_COMPOSER(parent_area)) {

		cong_editor_area_composer_pack (CONG_EDITOR_AREA_COMPOSER(parent_area),
						this_area,
						FALSE,
						FALSE,
						0);
	} else {
		cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER(parent_area),
						      this_area);
	}

	if (node->children) {
		vcomposer = cong_editor_area_composer_new (widget,
							   GTK_ORIENTATION_VERTICAL,
							   0);

		cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER (this_area),
						      vcomposer);
	}

	/* Recurse: */
	for (iter = node->children; iter; iter=iter->next) {
		recursive_create_areas (widget,
					iter,
					vcomposer);
	}
	
}

