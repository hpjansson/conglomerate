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
#include "cong-document.h"

#include "cong-app.h"
#include "cong-eel.h"

#include <gtk/gtkdrawingarea.h>

#include "cong-editor-area-flow-holder.h"
#include "cong-editor-area-flow-holder-blocks.h"
#include "cong-editor-child-policy.h"
#include "cong-editor-child-policy-flow-holder.h"

/* Test code: */
#include "cong-editor-area-border.h"
#include "cong-editor-area-composer.h"

#include "cong-selection.h"
#include "cong-editor-node-text.h"
#include "cong-command.h"

#define SHOW_CURSOR_SPEW 0

/* 
   The notes below are no longer true; see notes about entities in header file.

   OLD IMPLEMENTATION NOTES

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

typedef struct EditorMapping EditorMapping;

/* A per-xml-node struct that maps from traversal_parent ptrs to editor_nodes: */
struct EditorMapping
{
	CongNodePtr xml_node;
	GHashTable *hash_of_traversal_parent_to_editor_node;
};

EditorMapping*
cong_editor_mapping_new (CongNodePtr xml_node);

void
cong_editor_mapping_free (EditorMapping* mapping);

void
cong_editor_mapping_add_editor_node (EditorMapping* mapping,
				     CongEditorNode *traversal_parent,
				     CongEditorNode *editor_node);
void
cong_editor_mapping_remove_editor_node (EditorMapping* mapping,
					CongEditorNode *traversal_parent);

static void  
value_destroy_func (gpointer data)
{
	EditorMapping *mapping = (EditorMapping*)data;

	cong_editor_mapping_free (mapping);
}


#define PRIVATE(foo) ((foo)->private)

struct CongEditorWidget3Details
{
	CongDocument *doc;

	GHashTable *hash_of_node_to_editor_mapping;

#if 0
	GHashTable *hash_of_editor_node_to_child_policy;
	GHashTable *hash_of_editor_node_to_parents_child_policy;
#endif

	CongEditorArea *root_area;
	CongEditorAreaFlowHolder *root_flow_holder;
	CongEditorChildPolicy *root_child_policy;

	GdkGC *test_gc;
};


#define DEBUG_EDITOR_WIDGET_VIEW  0
#define LOG_GTK_WIDGET_SIGNALS    0
#define LOG_CONG_DOCUMENT_SIGNALS 0
#define LOG_EDITOR_NODES 0
#define LOG_EDITOR_AREAS 0

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
recursive_add_all_nodes(CongEditorWidget3 *widget,
			CongNodePtr node);

static void 
recursive_remove_all_nodes(CongEditorWidget3 *widget,
			   CongNodePtr node);

static void 
add_node_callback (CongEditorWidget3 *widget, 
		   CongEditorNode *editor_node, 
		   gpointer user_data);

static void 
remove_node_callback (CongEditorWidget3 *widget, 
		      CongEditorNode *editor_node, 
		      gpointer user_data);

static void
add_node_mapping (CongEditorWidget3 *widget,
		  CongNodePtr xml_node,
		  CongEditorNode *editor_node,
		  CongEditorNode *traversal_parent);

static void
remove_node_mapping (CongEditorWidget3 *widget,
		     CongNodePtr xml_node,
		     CongEditorNode *editor_node,
		     CongEditorNode *traversal_parent);

static void 
recursive_add_nodes(CongEditorWidget3 *widget,
		    CongNodePtr node,
		    CongEditorNode *traversal_parent);		    

static void 
recursive_remove_nodes(CongEditorWidget3 *widget,
		       CongNodePtr node,
		       CongEditorNode *traversal_parent);


static void 
create_areas(CongEditorWidget3 *widget,
	     CongEditorNode *editor_node);
static void 
destroy_areas(CongEditorWidget3 *widget,
	      CongEditorNode *editor_node);

#if 1
CongEditorChildPolicy*
cong_editor_widget3_get_child_policy_for_editor_node (CongEditorWidget3 *widget,
						      CongEditorNode *editor_node);
CongEditorChildPolicy*
cong_editor_widget3_get_parents_child_policy_for_editor_node (CongEditorWidget3 *widget,
							      CongEditorNode *editor_node);
#else
CongEditorArea*
cong_editor_widget3_get_primary_area_for_editor_node (CongEditorWidget3 *widget,
						      CongEditorNode *editor_node);
CongEditorAreaFlowHolder*
cong_editor_widget3_get_parent_flow_holder_for_editor_node (CongEditorWidget3 *widget,
							    CongEditorNode *editor_node);
CongEditorAreaFlowHolder*
cong_editor_widget3_get_child_flow_holder_for_editor_node (CongEditorWidget3 *widget,
							   CongEditorNode *editor_node);
#endif

/* Declarations of the GObject handlers: */
static void
cong_editor_widget3_finalize (GObject *object);

static void
cong_editor_widget3_dispose (GObject *object);

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
			    GtkOrientation orientation,
			    gpointer user_data);

/* Implementations of public functions: */
GNOME_CLASS_BOILERPLATE(CongEditorWidget3, 
			cong_editor_widget3,
			GtkDrawingArea,
			GTK_TYPE_DRAWING_AREA );

static void
cong_editor_widget3_class_init (CongEditorWidget3Class *klass)
{
	G_OBJECT_CLASS (klass)->finalize = cong_editor_widget3_finalize;
	G_OBJECT_CLASS (klass)->dispose = cong_editor_widget3_dispose;
}

static void
cong_editor_widget3_instance_init (CongEditorWidget3 *widget)
{
	widget->private = g_new0(CongEditorWidget3Details,1);
}

/* Disconnects all signals from instance that have the supplied user_data as the closure data; returns the number disconnected: */
guint
cong_eel_disconnect_all_with_data (gpointer instance,
				   gpointer user_data)
{
	guint count = 0;
	while (1) {
		gulong  handler_id;

		handler_id = g_signal_handler_find (instance,
						    G_SIGNAL_MATCH_DATA,
						    0, /* guint signal_id, */
						    0, /* GQuark detail, */
						    NULL, /* GClosure *closure, */
						    NULL, /* gpointer func, */
						    user_data);

		if (handler_id) {
			g_signal_handler_disconnect (instance,
						     handler_id);
			count++;
		} else {
			/* No more handlers: */
			return count;
		}
	}
}

CongEditorWidget3*
cong_editor_widget3_construct (CongEditorWidget3 *editor_widget,
			       CongDocument *doc)
{
	PRIVATE(editor_widget)->doc = doc;

	g_object_ref(G_OBJECT(doc));

	PRIVATE(editor_widget)->hash_of_node_to_editor_mapping = g_hash_table_new_full (NULL,
											NULL,
											NULL,
											value_destroy_func);

#if 0
	PRIVATE(editor_widget)->hash_of_editor_node_to_child_policy = g_hash_table_new (NULL,
											NULL);
	PRIVATE(editor_widget)->hash_of_editor_node_to_parents_child_policy = g_hash_table_new (NULL,
												NULL);
#endif

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
		PRIVATE(editor_widget)->root_area = cong_editor_area_bin_new (editor_widget);
	
		g_signal_connect (G_OBJECT(PRIVATE(editor_widget)->root_area),
				  "flush_requisition_cache",
				  G_CALLBACK(on_root_requisition_change),
				  editor_widget);
		
		PRIVATE(editor_widget)->root_flow_holder = CONG_EDITOR_AREA_FLOW_HOLDER(cong_editor_area_flow_holder_blocks_new(editor_widget));

		cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER(PRIVATE(editor_widget)->root_area),
						      CONG_EDITOR_AREA(PRIVATE(editor_widget)->root_flow_holder));

		PRIVATE(editor_widget)->root_child_policy = cong_editor_child_policy_flow_holder_new (NULL,
												      PRIVATE(editor_widget)->root_flow_holder);
	}

	/* Traverse the doc, adding EditorNodes and EditorAreas: */
	{
		populate_widget3(editor_widget);
	}

	/* Connect to CongDocument events: */
	{
		/* (These signal handlets get disconnected during the dispose handler) */

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

struct for_each_in_hash_struct
{
	CongEditorWidget3 *editor_widget;
	CongNodePtr xml_node;
	CongEditorNodeCallback editor_node_callback;
	gpointer user_data;
};

static void
for_each_in_hash_func (gpointer key,
		       gpointer value,
		       gpointer user_data)
{
	struct for_each_in_hash_struct *tmp_struct = (struct for_each_in_hash_struct*)user_data; 

	g_assert (tmp_struct);
	g_assert (tmp_struct->editor_widget);
	g_assert (tmp_struct->xml_node);
	g_assert (tmp_struct->editor_node_callback);

	tmp_struct->editor_node_callback (tmp_struct->editor_widget,
					  CONG_EDITOR_NODE(value),
					  tmp_struct->user_data);
	
}

void
cong_editor_widget3_for_each_editor_node (CongEditorWidget3 *editor_widget,
					  CongNodePtr xml_node,
					  CongEditorNodeCallback editor_node_callback,
					  gpointer user_data)
{
	EditorMapping *editor_mapping;

	g_return_if_fail (editor_widget);
	g_return_if_fail (xml_node);
	g_return_if_fail (editor_node_callback);

	editor_mapping = g_hash_table_lookup (PRIVATE(editor_widget)->hash_of_node_to_editor_mapping,
					      xml_node);

	if (editor_mapping) {
		struct for_each_in_hash_struct tmp_struct; 
	
		tmp_struct.editor_widget = editor_widget;
		tmp_struct.xml_node = xml_node;
		tmp_struct.editor_node_callback = editor_node_callback;
		tmp_struct.user_data = user_data;

		g_hash_table_foreach (editor_mapping->hash_of_traversal_parent_to_editor_node,
				      for_each_in_hash_func,
				      &tmp_struct);
				      
	}
}

CongEditorNode*
cong_editor_widget3_get_editor_node (CongEditorWidget3 *editor_widget,
				     CongNodePtr xml_node,
				     CongEditorNode *traversal_parent)
{
	EditorMapping *editor_mapping;

	g_return_val_if_fail (editor_widget, NULL);
	g_return_val_if_fail (xml_node, NULL);

	editor_mapping = g_hash_table_lookup (PRIVATE(editor_widget)->hash_of_node_to_editor_mapping,
					      xml_node);

	if (editor_mapping) {
		return CONG_EDITOR_NODE(g_hash_table_lookup (editor_mapping->hash_of_traversal_parent_to_editor_node, 
							     traversal_parent));
	} else {
		return NULL;
	}
}

static void
copy_to_result_hash_func (gpointer key,
			  gpointer value,
			  gpointer user_data)
{
	CongEditorNode **result = user_data;
	g_assert (result);

	*result = CONG_EDITOR_NODE (value);
}

CongEditorNode*
cong_editor_widget3_get_an_editor_node (CongEditorWidget3 *editor_widget,
					CongNodePtr xml_node)
{
	EditorMapping *editor_mapping;

	g_return_val_if_fail (editor_widget, NULL);
	g_return_val_if_fail (xml_node, NULL);

	editor_mapping = g_hash_table_lookup (PRIVATE(editor_widget)->hash_of_node_to_editor_mapping,
					      xml_node);

	if (editor_mapping) {
		CongEditorNode *result = NULL;

		g_hash_table_foreach (editor_mapping->hash_of_traversal_parent_to_editor_node,
				      copy_to_result_hash_func,
				      &result);

		return result;
	} else {
		return NULL;
	}
}

EditorMapping*
cong_editor_mapping_new (CongNodePtr xml_node)
{
	EditorMapping *mapping;

	g_assert (xml_node);

	mapping = g_new0(EditorMapping, 1);

	mapping->xml_node = xml_node;
	mapping->hash_of_traversal_parent_to_editor_node = g_hash_table_new (NULL, 
									     NULL);	

	return mapping;
}

void
cong_editor_mapping_free (EditorMapping* mapping)
{
	g_assert(mapping);

	g_hash_table_destroy ( mapping->hash_of_traversal_parent_to_editor_node);

	g_free (mapping);
}

void
cong_editor_mapping_add_editor_node (EditorMapping* mapping,
				     CongEditorNode *traversal_parent,
				     CongEditorNode *editor_node)
{
	g_assert(mapping);
	g_assert(editor_node);
	
	g_assert(mapping->hash_of_traversal_parent_to_editor_node);

	g_hash_table_insert (mapping->hash_of_traversal_parent_to_editor_node,
			     traversal_parent,
			     editor_node);
	
}

void
cong_editor_mapping_remove_editor_node (EditorMapping* mapping,
					CongEditorNode *traversal_parent)
{
	g_assert(mapping);
	g_assert(mapping->hash_of_traversal_parent_to_editor_node);

	g_hash_table_remove (mapping->hash_of_traversal_parent_to_editor_node,
			     traversal_parent);
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

/* Definitions of the GObject handlers: */
static void
cong_editor_widget3_finalize (GObject *object)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3 (object);

	g_message ("cong_editor_widget3_finalize");
	
	g_free (editor_widget->private);
	editor_widget->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
cong_editor_widget3_dispose (GObject *object)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3 (object);

	g_message ("cong_editor_widget3_dispose");

	g_assert (editor_widget->private);

	if (PRIVATE (editor_widget)->doc) {
		/* Disconnect all the CongDocument signal handlers: */
		cong_eel_disconnect_all_with_data (PRIVATE (editor_widget)->doc,
						   object);

		g_object_unref (G_OBJECT (PRIVATE (editor_widget)->doc));
		PRIVATE (editor_widget)->doc = NULL;
	}

	/* FIXME: need to clean up this lot! */
#if 0
	GHashTable *hash_of_node_to_editor_mapping;

	CongEditorArea *root_area;
	CongEditorAreaFlowHolder *root_flow_holder;
	CongEditorChildPolicy *root_child_policy;

	GdkGC *test_gc;
#endif

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
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

static gboolean
cong_editor_widget3_calc_up (CongEditorWidget3 *editor_widget,
			     CongLocation *input_loc,
			     CongLocation *output_loc)
{
	CongEditorNodeText *editor_node_text;
	int output_byte_offset;

	g_return_val_if_fail (IS_CONG_EDITOR_WIDGET3 (editor_widget), FALSE);
	g_return_val_if_fail (input_loc, FALSE);
	g_return_val_if_fail (output_loc, FALSE);

	editor_node_text = CONG_EDITOR_NODE_TEXT ( cong_editor_widget3_get_an_editor_node (editor_widget,
											   input_loc->node));
	g_assert (editor_node_text);

	if (cong_editor_node_text_calc_up (editor_node_text,
					   input_loc->byte_offset,
					   &output_byte_offset)) {
		cong_location_set_node_and_byte_offset (output_loc,
							input_loc->node,
							output_byte_offset);
		return TRUE;
	}

	return FALSE;
}

static gboolean
cong_editor_widget3_calc_down (CongEditorWidget3 *editor_widget,
			       CongLocation *input_loc,
			       CongLocation *output_loc)
{
	CongEditorNodeText *editor_node_text;
	int output_byte_offset;

	g_return_val_if_fail (IS_CONG_EDITOR_WIDGET3 (editor_widget), FALSE);
	g_return_val_if_fail (input_loc, FALSE);
	g_return_val_if_fail (output_loc, FALSE);

	editor_node_text = CONG_EDITOR_NODE_TEXT ( cong_editor_widget3_get_an_editor_node (editor_widget,
											   input_loc->node));
	g_assert (editor_node_text);

	if (cong_editor_node_text_calc_down (editor_node_text,
					     input_loc->byte_offset,
					     &output_byte_offset)) {
		cong_location_set_node_and_byte_offset (output_loc,
							input_loc->node,
							output_byte_offset);
		return TRUE;
	}

	return FALSE;
}

/* 
   Method to calculate where the cursor should go as a result of the key press.
   Affected by the CTRL key (which means "whole words" rather than "individual characters" for left/right).

   Return value:  TRUE iff the output_loc has been written to with a meaningful location different from the cursor location.
*/
static gboolean
cong_editor_widget3_get_destination_location_for_keypress (CongEditorWidget3 *editor_widget,
							   guint state,
							   guint keyval,
							   CongLocation *output_loc)
{
	CongDocument *doc;
	CongCursor *cursor;
	CongDispspec *dispspec;

	g_return_val_if_fail(IS_CONG_EDITOR_WIDGET3 (editor_widget), FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	doc = cong_editor_widget3_get_document (editor_widget);
	cursor = cong_document_get_cursor (doc);
	dispspec = cong_document_get_dispspec (doc);

	switch (keyval) {
	default: 
		return FALSE;

	case GDK_Up:
		return cong_editor_widget3_calc_up (editor_widget, 
						    &cursor->location,
						    output_loc);

	case GDK_Down:
		return cong_editor_widget3_calc_down (editor_widget, 
						      &cursor->location,
						      output_loc);
	
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

static gboolean 
key_press_event_handler (GtkWidget *w, 
			 GdkEventKey *event, 
			 gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3 (w);
	CongDocument *doc = cong_editor_widget3_get_document (editor_widget);
	CongCursor *cursor = cong_document_get_cursor (doc);
	CongSelection *selection = cong_document_get_selection  (doc);

	LOG_GTK_WIDGET_SIGNAL1("key_press_event_handler");

	g_return_val_if_fail(cursor->location.node, FALSE);

	doc = cong_editor_widget3_get_document(editor_widget);
	cursor = cong_document_get_cursor(doc);
	selection = cong_document_get_selection(doc);
	g_assert(selection);

	cong_document_begin_edit (doc);

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
			CongLocation old_location;
			CongLocation target_location;

			cong_location_copy(&old_location, &cursor->location);

			/* Calculate whereabouts in the document the user wants to go: */

			if (cong_editor_widget3_get_destination_location_for_keypress (editor_widget, 
										       event->state,
										       event->keyval,
										       &target_location)) {
				/* Are we moving the cursor, or dragging out a selection? */

#if SUPPORT_UNDO
				CongCommand *cmd = cong_command_new (doc, _("Cursor Movement"));
				CongLocation new_selection_start;
				CongLocation new_selection_end;
				
				cong_document_begin_edit (doc);	

				/* Move the cursor to the new location: */
				cong_command_add_cursor_change (cmd,
								&target_location);

				cong_location_copy (&new_selection_start, cong_selection_get_logical_start (selection)); 
				cong_location_copy (&new_selection_end, cong_selection_get_logical_end (selection)); 

				if (event->state & GDK_SHIFT_MASK) {
					
					if (NULL==(cong_selection_get_logical_start(selection)->node)) {
						cong_location_copy (&new_selection_start, &old_location);
					}

					/* Then we should also drag out the selection to the new location: */
					cong_location_copy (&new_selection_end, &target_location);

				} else {
					/* Then we should clear any selection that exists: */
					cong_location_nullify (&new_selection_start);
					cong_location_nullify (&new_selection_end);
				}

				cong_command_add_selection_change (cmd,
								   &new_selection_start,
								   &new_selection_end);

				cong_document_add_command (doc,
							   cmd);
				
				g_object_unref (G_OBJECT (cmd));
				cong_document_end_edit (doc);	

#else
				/* Move the cursor to the new location: */
				cong_location_copy(&cursor->location, &target_location);

				cong_document_begin_edit (doc);

				if (event->state & GDK_SHIFT_MASK) {
					if (NULL==(cong_selection_get_logical_start(selection)->node)) {
						cong_selection_set_logical_start (selection,
										  &old_location);
					}

					/* Then we should also drag out the selection to the new location: */
					cong_selection_end_from_curs (selection, 
								      cursor);
					cong_document_on_selection_change (doc);					
				} else {
					/* Then we should clear any selection that exists: */
					cong_selection_start_from_curs (selection, 
									cursor);
					cong_document_on_selection_change (doc);
				}
				/* FIXME: should we notify the document? */ 
				cong_document_on_cursor_change (doc);

				cong_document_end_edit (doc);	
#endif
			}
		}
		break;
	
	case GDK_BackSpace:
		if (cong_selection_get_logical_end(selection)->node) {
#if SUPPORT_UNDO
			CongCommand *cmd = cong_command_new (doc, _("Delete Selection"));
			cong_command_add_delete_selection (cmd);
			cong_document_add_command (doc,
						   cmd);
			
			g_object_unref (G_OBJECT (cmd));
#else
			cong_document_delete_selection(doc);
#endif
		} else {
			cong_cursor_del_prev_char(cursor, doc);
		}
		break;
	
	case GDK_Delete:
		if (cong_selection_get_logical_end(selection)->node) {
#if SUPPORT_UNDO
			CongCommand *cmd = cong_command_new (doc, _("Delete Selection"));
			cong_command_add_delete_selection (cmd);
			cong_document_add_command (doc,
						   cmd);
			
			g_object_unref (G_OBJECT (cmd));
#else
			cong_document_delete_selection(doc);
#endif
		} else {
			cong_cursor_del_next_char(cursor, doc);
		}
		break;

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
#if SUPPORT_UNDO
			CongCommand *cmd = cong_command_new (doc, _("Typing"));

                        if (cong_selection_get_logical_end(selection)->node) {
				cong_command_add_delete_selection (cmd);
                        }
			cong_command_add_insert_text_at_cursor (cmd, event->string);
			cong_command_add_nullify_selection (cmd);

			cong_document_add_command (doc,
						   cmd);
			
			g_object_unref (G_OBJECT (cmd));
#else
                        if (cong_selection_get_logical_end(selection)->node) {
                                cong_document_delete_selection(doc);
                        }

			cong_cursor_data_insert(cursor, event->string);

			cong_selection_nullify (selection);
			cong_document_on_selection_change (doc);
#endif
		}
		break;
	}

	cong_document_on_cursor_change(doc);	

	cong_document_end_edit (doc);

	return TRUE;
}

static void size_request_handler(GtkWidget *widget,
 				 GtkRequisition *requisition,
 				 gpointer user_data)
{
 	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(widget);
	GtkRequisition root_req;
 
 	LOG_GTK_WIDGET_SIGNAL1("size_request_handler");

 	g_assert(widget);
 	g_assert(requisition);

	root_req.width = cong_editor_area_get_requisition (PRIVATE(editor_widget)->root_area,
							   GTK_ORIENTATION_HORIZONTAL,
							   widget->allocation.width); 
	root_req.height = cong_editor_area_get_requisition (PRIVATE(editor_widget)->root_area,
							    GTK_ORIENTATION_VERTICAL,
							    widget->allocation.width); 

	/* Only request up to the width you've already been allocated; don't grow in width unless your container gives you more room. */
	if (root_req.width > widget->allocation.width) {
		requisition->width = widget->allocation.width;
	} else {
		requisition->width = root_req.width;
	}
 	requisition->height = root_req.height;
}

/* Definitions of the CongDocument event handlers: */
/* Signal handling callbacks: */
/* Callbacks attached before the default handler: */

static void on_signal_begin_edit_notify_before (CongDocument *doc,
					 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_begin_edit_notify_before");

	/* empty so far */
}

static void on_signal_end_edit_notify_before (CongDocument *doc,
				       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_end_edit_notify_before");

	/* empty so far */
}

static void on_signal_make_orphan_notify_before (CongDocument *doc, 
						 CongNodePtr node, 
						 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_make_orphan_notify_before");

#if 1
	if (cong_editor_widget3_node_should_have_editor_node(node)) {
		recursive_remove_all_nodes(editor_widget, node);
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
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_add_after_notify_before");

	/* empty so far */
}

static void on_signal_add_before_notify_before (CongDocument *doc, 
					 CongNodePtr node, 
					 CongNodePtr younger_sibling, 
					 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_add_before_notify_before");

	/* empty so far */
}

static void on_signal_set_parent_notify_before (CongDocument *doc, 
					 CongNodePtr node, 
					 CongNodePtr adoptive_parent, 
					 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_set_parent_notify_before");

	if (cong_editor_widget3_node_should_have_editor_node(node)) {
		recursive_remove_all_nodes(editor_widget, node);
	} else {
		g_assert(!cong_editor_widget3_has_editor_node_for_node(editor_widget, node));
	}
}

static void on_signal_set_text_notify_before (CongDocument *doc, 
				       CongNodePtr node, 
				       const xmlChar *new_content, 
				       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_set_text_notify_before");

	/* empty so far */
}

static void on_signal_set_attribute_notify_before (CongDocument *doc, 
					    CongNodePtr node, 
					    const xmlChar *name, 
					    const xmlChar *value, 
					    gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_set_attribute_notify_before");

	/* empty so far */
}

static void on_signal_remove_attribute_notify_before (CongDocument *doc, 
					       CongNodePtr node, 
					       const xmlChar *name, 
					       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_remove_attribute_notify_before");

	/* empty so far */
}

static void on_signal_selection_change_notify_before (CongDocument *doc, 
					       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_selection_change_notify_before");

	/* empty so far */
}

static void on_signal_cursor_change_notify_before (CongDocument *doc, 
					    gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

#if SHOW_CURSOR_SPEW
	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_cursor_change_notify_before");
#endif

	/* empty so far */
}


/* Callbacks attached after the default handler: */
static void on_signal_begin_edit_notify_after (CongDocument *doc,
					 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_begin_edit_notify_after");

	/* empty so far */
}

static void on_signal_end_edit_notify_after (CongDocument *doc,
				       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_end_edit_notify_after");

	/* empty so far */
}

static void on_signal_make_orphan_notify_after (CongDocument *doc, 
					 CongNodePtr node, 
					 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 
	
	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_make_orphan_notify_after");

	/* empty so far */
}

static void on_signal_add_after_notify_after (CongDocument *doc, 
					CongNodePtr node, 
					CongNodePtr older_sibling, 
					gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_add_after_notify_after");

#if 1
	recursive_add_all_nodes(editor_widget, node);
#endif
}

static void on_signal_add_before_notify_after (CongDocument *doc, 
					 CongNodePtr node, 
					 CongNodePtr younger_sibling, 
					 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_add_before_notify_after");

#if 1
	recursive_add_all_nodes(editor_widget, node);
#endif
}

static void on_signal_set_parent_notify_after (CongDocument *doc, 
					 CongNodePtr node, 
					 CongNodePtr adoptive_parent, 
					 gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_set_parent_notify_after");

#if 1
	recursive_add_all_nodes(editor_widget, node);
#endif
}

static void on_signal_set_text_notify_after (CongDocument *doc, 
				       CongNodePtr node, 
				       const xmlChar *new_content, 
				       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_set_text_notify_after");

	/* empty so far */
}

static void on_signal_set_attribute_notify_after (CongDocument *doc, 
					   CongNodePtr node, 
					   const xmlChar *name, 
					   const xmlChar *value, 
					   gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_set_attribute_notify_after");

	/* empty so far */
}

static void on_signal_remove_attribute_notify_after (CongDocument *doc, 
					       CongNodePtr node, 
					       const xmlChar *name, 
					       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_remove_attribute_notify_after");

	/* empty so far */
}

static void on_signal_selection_change_notify_after (CongDocument *doc, 
					       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

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
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

#if SHOW_CURSOR_SPEW
	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_cursor_change_notify_after");
#endif

	/* Force a redraw: */
	gtk_widget_queue_draw(GTK_WIDGET(editor_widget));	
}

/* Definitions of CongEditorArea event handlers: */
static void
on_root_requisition_change (CongEditorArea *child_area,
			    GtkOrientation orientation,
			    gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data);

	LOG_EDITOR_AREA1 ("on_root_requisition_change");

	gtk_widget_queue_resize (GTK_WIDGET(editor_widget));
}

static void 
populate_widget3(CongEditorWidget3 *widget)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(widget);
	CongDocument *doc;

	doc = cong_editor_widget3_get_document(widget);

	recursive_add_all_nodes (widget,
				 (CongNodePtr)cong_document_get_xml (doc));		
}

gboolean
cong_editor_widget3_node_should_have_editor_node (CongNodePtr node)
{
	g_return_val_if_fail (node, FALSE);

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
	return 	g_hash_table_lookup(PRIVATE(widget)->hash_of_node_to_editor_mapping,node)!=NULL;
}

static void 
recursive_add_all_nodes(CongEditorWidget3 *widget,
			CongNodePtr node)
{
	if (node->parent) {
		/* Add editor nodes for this node into all of the editor nodes of the parent: */
		cong_editor_widget3_for_each_editor_node (widget,
							  node->parent,
							  add_node_callback,
							  node);
	} else {
		recursive_add_nodes(widget,
				    node,
				    NULL);
	}
}

static void 
recursive_remove_all_nodes(CongEditorWidget3 *widget,
			   CongNodePtr node)
{
	if (node->parent) {
		/* Remove the editor node for this node from all parent editor nodes: */
		cong_editor_widget3_for_each_editor_node (widget,
							  node->parent,
							  remove_node_callback,
							  node);
	} else {
		recursive_remove_nodes (widget,
					node,
					NULL);
	}
}

static void 
add_node_callback (CongEditorWidget3 *widget, 
		   CongEditorNode *editor_node, 
		   gpointer user_data)
{
	recursive_add_nodes(widget,
			    (CongNodePtr)user_data,
			    editor_node /* CongEditorNode *traversal_parent*/);
}

static void 
remove_node_callback (CongEditorWidget3 *widget, 
		      CongEditorNode *editor_node, 
		      gpointer user_data)
{
	recursive_remove_nodes(widget,
			       (CongNodePtr)user_data,
			       editor_node /* CongEditorNode *traversal_parent*/);
}

static void
add_node_mapping (CongEditorWidget3 *widget,
		  CongNodePtr xml_node,
		  CongEditorNode *editor_node,
		  CongEditorNode *traversal_parent)
{
	EditorMapping *editor_mapping;

	g_assert (widget);
	g_assert (xml_node);
	g_assert (editor_node);

	/* Claim our reference on the editor node: */
	g_object_ref (G_OBJECT(editor_node));


	editor_mapping = g_hash_table_lookup (PRIVATE(widget)->hash_of_node_to_editor_mapping,
					      xml_node);

	if (NULL==editor_mapping) {
		editor_mapping = cong_editor_mapping_new (xml_node);

		g_hash_table_insert (PRIVATE(widget)->hash_of_node_to_editor_mapping,
				     xml_node,
				     editor_mapping);
	}

	g_assert (editor_mapping);

	cong_editor_mapping_add_editor_node (editor_mapping,
					     traversal_parent,
					     editor_node);
}

static void
remove_node_mapping (CongEditorWidget3 *widget,
		     CongNodePtr xml_node,
		     CongEditorNode *editor_node,
		     CongEditorNode *traversal_parent)
{
	EditorMapping *editor_mapping;

	g_assert (widget);
	g_assert (xml_node);
	g_assert (editor_node);

	editor_mapping = g_hash_table_lookup (PRIVATE(widget)->hash_of_node_to_editor_mapping,
					      xml_node);

	g_assert(editor_mapping);

	cong_editor_mapping_remove_editor_node (editor_mapping,
						traversal_parent);

	/* FIXME: is there a cheaper way to do this? */
	if (0==g_hash_table_size(PRIVATE(widget)->hash_of_node_to_editor_mapping)) {
		g_hash_table_remove (PRIVATE(widget)->hash_of_node_to_editor_mapping,
				     xml_node);
	}

	CONG_EEL_LOG_REF_COUNT("redundant editor node", G_OBJECT(editor_node));
	
	/* Release our reference on the editor_node: */
	g_object_unref (editor_node);
}

static void 
recursive_add_nodes(CongEditorWidget3 *widget,
		    CongNodePtr node,
		    CongEditorNode *traversal_parent)
{
	CongEditorNode* editor_node;
	CongNodePtr iter;


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
		editor_node = cong_editor_node_manufacture (widget,
							    node,
							    traversal_parent);
		g_assert(editor_node);

		add_node_mapping (widget,
				  node,
				  editor_node,
				  traversal_parent);

		/* Our initial refernce is now help by the node mapping: */
		g_object_unref (G_OBJECT(editor_node));

		/* DHM 4th August 2003:
		   Currently the area packing routines get confused; the areas for an entity ref's content get added below the parent of the entity decl (I think; haven't looked too closely into the details.
		   In any case, I need to have a good look at the semantics of entity refs and entity decls in libxml2 before getting this to work...

		   Perhaps the editor_node should store an "effective parent" when it gets created; hence the editor nodes have a kind of resolved entities representation, but we can still get back to the
		   entities (and perhaps trigger updates...).  However it does mean that there can be more than one editor_node per doc_node :-(
		*/
		create_areas (widget,
			      editor_node);
	}
	
	/* Recurse: */
	for (iter = node->children; iter; iter=iter->next) {
		recursive_add_nodes (widget, 
				     iter,
				     editor_node);
	}
}

static void 
recursive_remove_nodes (CongEditorWidget3 *widget,
			CongNodePtr node,
			CongEditorNode *traversal_parent)
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

	editor_node = cong_editor_widget3_get_editor_node (widget,
							   node,
							   traversal_parent);
	g_assert(editor_node);

	/* Recurse: */
	for (iter = node->children; iter; iter=iter->next) {
		recursive_remove_nodes (widget, 
					iter,
					editor_node);		
	}

	destroy_areas (widget,
		       editor_node);
	
	/* Remove this editor_node: */
	remove_node_mapping (widget,
			     node,
			     editor_node,
			     traversal_parent);
}

static void 
create_areas(CongEditorWidget3 *widget,
	     CongEditorNode *editor_node)
{
	CongEditorChildPolicy *parents_child_policy = NULL;
	CongEditorChildPolicy *this_child_policy = NULL;
	enum CongFlowType flow_type;
	CongNodePtr node = cong_editor_node_get_node (editor_node);

#if LOG_EDITOR_AREAS
	{
		gchar *node_desc = cong_node_debug_description(node);

		LOG_EDITOR_AREA2("create_areas for %s", node_desc);

		g_free(node_desc);
	}
#endif

	flow_type = cong_editor_node_get_flow_type (editor_node);

#if LOG_EDITOR_AREAS
	g_message("flow type = %s", cong_flow_type_get_debug_string(flow_type));
#endif

	/* Determine the parent area where the new area should be inserted: */
	{
		if (node->parent) {
			CongEditorNode *parent_editor_node;
			
			parent_editor_node = cong_editor_node_get_traversal_parent (editor_node);
			
			parents_child_policy = cong_editor_widget3_get_child_policy_for_editor_node (widget,
												     parent_editor_node);
			
		} else {
			/* Root of the document; insert below the widget's root_area: */
			g_assert(cong_node_type(node) == CONG_NODE_TYPE_DOCUMENT);

			parents_child_policy = PRIVATE(widget)->root_child_policy;
		}

		cong_editor_node_set_parents_child_policy (editor_node,
							   parents_child_policy);

#if 0
		g_hash_table_insert (PRIVATE(widget)->hash_of_editor_node_to_parents_child_policy,
				     editor_node,
				     parents_child_policy);
#endif		

		g_assert(parents_child_policy);
	}

#if 1
	this_child_policy = cong_editor_child_policy_insert_areas_for_node (parents_child_policy,
									    editor_node);

	cong_editor_node_set_child_policy (editor_node,
					   this_child_policy);

#if 0
	g_hash_table_insert (PRIVATE(widget)->hash_of_editor_node_to_child_policy,
			     editor_node,
			     this_child_policy);
#endif

#else
	this_area = cong_editor_area_flow_holder_insert_areas_for_node (parent_flow_holder,
									editor_node);	

	g_hash_table_insert (PRIVATE(widget)->hash_of_editor_node_to_primary_area,
			     editor_node,
			     this_area);

	/* If this node can ever have children, we need to add a container for them:
	   FIXME:  slightly hackish test */
	if (IS_CONG_EDITOR_AREA_CONTAINER(this_area) ) {
		CongEditorAreaFlowHolder *flow_holder;

		flow_holder = cong_editor_area_flow_holder_manufacture (widget,
									flow_type);

		cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER (this_area),
						      CONG_EDITOR_AREA(flow_holder));

		g_hash_table_insert (PRIVATE(widget)->hash_of_editor_node_to_child_flow_holder,
				     editor_node,
				     flow_holder);
	}
#endif
}

static void 
destroy_areas(CongEditorWidget3 *widget,
	      CongEditorNode *editor_node)
{
	CongEditorChildPolicy *parents_child_policy = NULL;
	CongNodePtr node = cong_editor_node_get_node (editor_node);

#if LOG_EDITOR_AREAS
	{
		gchar *node_desc = cong_node_debug_description(node);

		LOG_EDITOR_AREA2("destroy_areas for %s", node_desc);
		
		g_free(node_desc);
	}
#endif

	if (node->parent) {
		parents_child_policy = cong_editor_widget3_get_parents_child_policy_for_editor_node (widget,
												     editor_node);
	} else {
		g_assert(cong_node_type(node) == CONG_NODE_TYPE_DOCUMENT);
		parents_child_policy = PRIVATE(widget)->root_child_policy;
	}

	g_assert(parents_child_policy);

	cong_editor_child_policy_remove_areas_for_node (parents_child_policy,
							editor_node);

	cong_editor_node_set_parents_child_policy (editor_node,
						   NULL);
	cong_editor_node_set_child_policy (editor_node,
					   NULL);

#if 0
	g_hash_table_remove (PRIVATE(widget)->hash_of_editor_node_to_parents_child_policy,
			     editor_node);
	g_hash_table_remove (PRIVATE(widget)->hash_of_editor_node_to_child_policy,
			     editor_node);
#endif
}

#if 1
CongEditorChildPolicy*
cong_editor_widget3_get_child_policy_for_editor_node (CongEditorWidget3 *widget,
						      CongEditorNode *editor_node)
{
	g_return_val_if_fail (widget, NULL);
	g_return_val_if_fail (IS_CONG_EDITOR_NODE(editor_node), NULL);

#if 1
	return cong_editor_node_get_child_policy (editor_node);						  
#else
	return CONG_EDITOR_CHILD_POLICY(g_hash_table_lookup (PRIVATE(widget)->hash_of_editor_node_to_child_policy,
							     editor_node));
#endif
	
}
CongEditorChildPolicy*
cong_editor_widget3_get_parents_child_policy_for_editor_node (CongEditorWidget3 *widget,
							      CongEditorNode *editor_node)
{
	g_return_val_if_fail (widget, NULL);
	g_return_val_if_fail (IS_CONG_EDITOR_NODE(editor_node), NULL);

#if 1
	return cong_editor_node_get_parents_child_policy (editor_node);
#else
	return CONG_EDITOR_CHILD_POLICY(g_hash_table_lookup (PRIVATE(widget)->hash_of_editor_node_to_parents_child_policy,
							     editor_node));
#endif
	
}
#else
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
#endif
