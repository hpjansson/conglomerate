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
#include "cong-document-traversal.h"

#include "cong-app.h"
#include "cong-eel.h"

#include <gtk/gtkdrawingarea.h>

/* Test code: */
#include "cong-editor-area-border.h"
#include "cong-editor-area-composer.h"

#include "cong-selection.h"
#include "cong-editor-node-text.h"
#include "cong-command.h"
#include "cong-traversal-node.h"
#include "cong-editor-line-manager.h"
#include "cong-editor-line-manager-simple.h"
#include "cong-editor-area-lines.h"
#include "cong-dispspec-element.h"
#include "cong-ui-hooks.h"

#define SHOW_CURSOR_SPEW 0
#define DEBUG_IM_CONTEXT 1

/* 
   The CongEditorWidget3 maintains a hash table from CongTraversalNodes to CongEditorNodes.

   The widget holds references to the CongEditorNodes it is storing in this hash table; these should (I think) be the only references
   on the EditorNodes.
*/


#undef PRIVATE
#define PRIVATE(foo) ((foo)->private)

struct CongEditorWidget3Details
{
	CongDocument *doc;

	CongPrimaryWindow *primary_window;

	GHashTable *hash_of_traversal_node_to_editor_node;

	CongEditorArea *root_area;

#if 1
	CongEditorLineManager *root_line_manager;
#else
	CongEditorAreaFlowHolder *root_flow_holder;
	CongEditorChildPolicy *root_child_policy;
#endif

	CongEditorArea *prehighlight_area;

	CongNodePtr selected_xml_node;

	GdkGC *test_gc;

	GtkIMContext *im_context;
	gboolean need_im_reset;
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
static void
log_editor_node_event (const gchar *msg,
		       CongEditorNode *editor_node);
#define LOG_EDITOR_NODE_EVENT(msg, node) log_editor_node_event (msg, node)
#else
#define LOG_EDITOR_NODE1(x) ((void)0)
#define LOG_EDITOR_NODE2(x, a) ((void)0)
#define LOG_EDITOR_NODE_EVENT(msg, node) ((void)0)
#endif

#if LOG_EDITOR_AREAS
#define LOG_EDITOR_AREA1(x) g_message((x))
#define LOG_EDITOR_AREA2(x, a) g_message((x), (a))
#else
#define LOG_EDITOR_AREA1(x) ((void)0)
#define LOG_EDITOR_AREA2(x, a) ((void)0)
#endif

/* Declarations of misc stuff: */
#if 0
static void 
render_area (CongEditorArea *area,
	     gpointer user_data);
#endif

static void 
populate_widget3(CongEditorWidget3 *widget);

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

/* IM Context Callbacks
 */
static void     
commit_cb               (GtkIMContext *context,
			 const gchar  *str,
			 CongEditorWidget3     *editor_widget);
static void     
preedit_changed_cb      (GtkIMContext *context,
			 CongEditorWidget3     *editor_widget);
static gboolean 
retrieve_surrounding_cb (GtkIMContext *context,
			 CongEditorWidget3     *editor_widget);
static gboolean 
delete_surrounding_cb   (GtkIMContext *context,
			 gint          offset,
			 gint          n_chars,
			 CongEditorWidget3     *editor_widget);

/* Declarations of the GtkWidget event handlers: */
static void   realize  (GtkWidget        *widget);
static void   unrealize (GtkWidget        *widget);
static gboolean expose_event_handler(GtkWidget *w, GdkEventExpose *event, gpointer user_data);
static gboolean configure_event_handler(GtkWidget *w, GdkEventConfigure *event, gpointer user_data);
static gboolean button_press_event_handler(GtkWidget *w, GdkEventButton *event, gpointer user_data);
static gboolean motion_notify_event_handler(GtkWidget *w, GdkEventMotion *event, gpointer user_data);
static gboolean key_press_event_handler(GtkWidget *w, GdkEventKey *event, gpointer user_data);
static gboolean key_release_event_handler(GtkWidget *w, GdkEventKey *event, gpointer user_data);
static void size_request_handler(GtkWidget *widget,
 				 GtkRequisition *requisition,
 				 gpointer user_data);
static gint focus_in_event_handler(GtkWidget *w, GdkEventFocus *event);
static gint focus_out_event_handler(GtkWidget *w, GdkEventFocus *event);

/* Declarations of the CongDocument event handlers: */
static void 
on_signal_selection_change_notify_after (CongDocument *doc, 
					 gpointer user_data);
static void 
on_signal_cursor_change_notify_after (CongDocument *doc, 
				      gpointer user_data);

/* Declarations of CongDocumentTraversal signal handlers: */
static void
on_traversal_node_added (CongDocumentTraversal *traversal, 
			 CongTraversalNode *traversal_node,
			 gpointer user_data);

static void
on_traversal_node_removed (CongDocumentTraversal *traversal, 
			   CongTraversalNode *traversal_node,
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
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	G_OBJECT_CLASS (klass)->finalize = cong_editor_widget3_finalize;
	G_OBJECT_CLASS (klass)->dispose = cong_editor_widget3_dispose;

	widget_class->realize = realize;
	widget_class->unrealize = unrealize;

	widget_class->focus_in_event = focus_in_event_handler;
	widget_class->focus_out_event = focus_out_event_handler;

#define _EXPANDER_SIZE 10
    
	gtk_widget_class_install_style_property (widget_class,
						 g_param_spec_int ("expander_size",
								   _("Expander Size"),
								   _("Size of the expander arrow"),
								   0,
								   G_MAXINT,
								   _EXPANDER_SIZE,
								   G_PARAM_READABLE));
}

static void
cong_editor_widget3_instance_init (CongEditorWidget3 *widget)
{
	widget->private = g_new0(CongEditorWidget3Details,1);
}

/**
 * cong_eel_disconnect_all_with_data:
 * @instance: The instance owning the signal handler to be found
 * @user_data: The closure data of the handler's closure
 *
 * Returns: The number of signals disconnected
 */
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

/**
 * cong_editor_widget3_construct:
 * @editor_widget:
 * @doc:
 * @primary_window:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorWidget3*
cong_editor_widget3_construct (CongEditorWidget3 *editor_widget,
			       CongDocument *doc,
			       CongPrimaryWindow *primary_window)
{
	PRIVATE(editor_widget)->doc = doc;

	g_object_ref(G_OBJECT(doc));

	PRIVATE(editor_widget)->primary_window = primary_window;

#if 1
	/* FIXME: need to clean up properly: */
	PRIVATE(editor_widget)->hash_of_traversal_node_to_editor_node = g_hash_table_new_full (NULL,
											       NULL,
											       NULL,
											       NULL);
#else
	PRIVATE(editor_widget)->hash_of_node_to_editor_mapping = g_hash_table_new_full (NULL,
											NULL,
											NULL,
											value_destroy_func);
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
	gtk_signal_connect_after(GTK_OBJECT(editor_widget), 
				 "key_release_event",
				 (GtkSignalFunc) key_release_event_handler, 
				 NULL);
	gtk_signal_connect(GTK_OBJECT(editor_widget),
 			   "size-request",
 			   (GtkSignalFunc) size_request_handler,
 			   NULL);

	gtk_widget_set_events(GTK_WIDGET(editor_widget), GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_KEY_PRESS_MASK);

	gtk_widget_set(GTK_WIDGET(editor_widget), "can_focus", (gboolean) TRUE, 0);
	gtk_widget_set(GTK_WIDGET(editor_widget), "can_default", (gboolean) TRUE, 0);

	/* Set up GTK Input Method support: */
	{
		PRIVATE(editor_widget)->im_context = gtk_im_multicontext_new ();
		
		g_signal_connect (PRIVATE(editor_widget)->im_context, "commit",
				  G_CALLBACK (commit_cb), editor_widget);
		g_signal_connect (PRIVATE(editor_widget)->im_context, "preedit_changed",
				  G_CALLBACK (preedit_changed_cb), editor_widget);
		g_signal_connect (PRIVATE(editor_widget)->im_context, "retrieve_surrounding",
				  G_CALLBACK (retrieve_surrounding_cb), editor_widget);
		g_signal_connect (PRIVATE(editor_widget)->im_context, "delete_surrounding",
				  G_CALLBACK (delete_surrounding_cb), editor_widget);
	}

	/* Set up root area: */
	{
		PRIVATE(editor_widget)->root_area = cong_editor_area_lines_new (editor_widget);
	
		g_signal_connect (G_OBJECT(PRIVATE(editor_widget)->root_area),
				  "flush_requisition_cache",
				  G_CALLBACK(on_root_requisition_change),
				  editor_widget);

		/* Set up root line manager: */
		PRIVATE (editor_widget)->root_line_manager = cong_editor_line_manager_simple_new (editor_widget,
												  NULL,
												  CONG_EDITOR_AREA_LINES (PRIVATE(editor_widget)->root_area));
	}

	/* Traverse the doc, adding EditorNodes and EditorAreas: */
	{
		populate_widget3(editor_widget);
	}

	/* Connect to CongDocument events: */
	{
		/* (These signal handlets get disconnected during the dispose handler) */
		g_signal_connect_after (G_OBJECT(doc), "selection_change", G_CALLBACK(on_signal_selection_change_notify_after), editor_widget);
		g_signal_connect_after (G_OBJECT(doc), "cursor_change", G_CALLBACK(on_signal_cursor_change_notify_after), editor_widget);
	}

	/* Connect to CongDocumentTraversal events: */
	{
		/* (These signal handlets get disconnected during the dispose handler) */
		/* FIXME:  Do they?  Check this! */
		CongDocumentTraversal *traversal = cong_document_get_traversal (doc);

		g_signal_connect_after (G_OBJECT (traversal), "traversal_node_added", G_CALLBACK(on_traversal_node_added), editor_widget);
		g_signal_connect (G_OBJECT (traversal), "traversal_node_removed", G_CALLBACK(on_traversal_node_removed), editor_widget);
		
		
	}

	return editor_widget;
}

/**
 * cong_editor_widget3_new:
 * @doc:
 * @primary_window:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget* 
cong_editor_widget3_new(CongDocument *doc,
			CongPrimaryWindow *primary_window)
{
	g_return_val_if_fail(doc, NULL);

	return GTK_WIDGET( cong_editor_widget3_construct (g_object_new (CONG_EDITOR_WIDGET3_TYPE, NULL),
							  doc,
							  primary_window)
			   );
}

/**
 * cong_editor_widget3_get_document:
 * @editor_widget:
 *
 * TODO: Write me
 * Returns:
 */
CongDocument *
cong_editor_widget3_get_document(CongEditorWidget3 *editor_widget)
{
	g_return_val_if_fail(editor_widget, NULL);

	return PRIVATE(editor_widget)->doc;
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

/**
 * cong_editor_widget3_get_editor_node_for_traversal_node:
 * @editor_widget:
 * @traversal_node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_widget3_get_editor_node_for_traversal_node (CongEditorWidget3 *editor_widget,
							CongTraversalNode *traversal_node)
{
	g_return_val_if_fail (IS_CONG_EDITOR_WIDGET3 (editor_widget), NULL);
	g_return_val_if_fail (IS_CONG_TRAVERSAL_NODE (traversal_node), NULL);

	return g_hash_table_lookup (PRIVATE (editor_widget)->hash_of_traversal_node_to_editor_node,
				    traversal_node);
}

CongEditorNode*
cong_editor_widget_get_root_editor_node (CongEditorWidget3 *editor_widget)
{
	g_return_val_if_fail (IS_CONG_EDITOR_WIDGET3 (editor_widget), NULL);

	return cong_editor_widget3_get_editor_node_for_traversal_node (editor_widget,
								       cong_document_get_root_traversal_node (PRIVATE(editor_widget)->doc));
}


struct for_each_editor_node_struct
{
	CongEditorWidget3 *editor_widget;
	CongEditorNodeCallback editor_node_callback;
	gpointer user_data;
};

static void
for_each_editor_node_cb (CongDocumentTraversal *doc_traversal, 
			 CongTraversalNode *traversal_node,
			 gpointer user_data)
{
	struct for_each_editor_node_struct *tmp_struct = (struct for_each_editor_node_struct*)user_data; 
	CongEditorNode *editor_node;

	g_assert (tmp_struct);
	g_assert (tmp_struct->editor_widget);
	g_assert (tmp_struct->editor_node_callback);

	editor_node = cong_editor_widget3_get_editor_node_for_traversal_node (tmp_struct->editor_widget,
									      traversal_node);

	tmp_struct->editor_node_callback (tmp_struct->editor_widget,
					  editor_node,
					  tmp_struct->user_data);
	
}

/**
 * cong_editor_widget3_for_each_editor_node:
 * @widget:
 * @xml_node:
 * @editor_node_callback:
 * @user_data:
 *
 * TODO: Write me
 */
void
cong_editor_widget3_for_each_editor_node (CongEditorWidget3 *editor_widget,
					  CongNodePtr xml_node,
					  CongEditorNodeCallback editor_node_callback,
					  gpointer user_data)
{
	struct for_each_editor_node_struct tmp_struct; 
	
	g_return_if_fail (editor_widget);
	g_return_if_fail (xml_node);
	g_return_if_fail (editor_node_callback);

	tmp_struct.editor_widget = editor_widget;
	tmp_struct.editor_node_callback = editor_node_callback;
	tmp_struct.user_data = user_data;

	cong_document_traversal_for_each_traversal_node (cong_document_get_traversal (cong_editor_widget3_get_document (editor_widget)),
							 xml_node,
							 for_each_editor_node_cb,
							 &tmp_struct);
}

/**
 * cong_editor_widget3_get_an_editor_node:
 * @editor_widget:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_widget3_get_an_editor_node (CongEditorWidget3 *editor_widget,
					CongNodePtr xml_node)
{
	CongTraversalNode *traversal_node;

	g_return_val_if_fail (editor_widget, NULL);
	g_return_val_if_fail (xml_node, NULL);

	traversal_node = cong_document_traversal_get_a_traversal_node (cong_document_get_traversal (PRIVATE (editor_widget)->doc),
								       xml_node);

	if (traversal_node) {
		return cong_editor_widget3_get_editor_node_for_traversal_node (editor_widget,
									       traversal_node);
	} else {
		return NULL;
	}
}

/**
 * cong_editor_widget3_get_prehighlight_editor_area:
 * @editor_widget:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_widget3_get_prehighlight_editor_area (CongEditorWidget3 *editor_widget)
{
	g_return_val_if_fail (editor_widget, NULL);
	
	return PRIVATE (editor_widget)->prehighlight_area;
}

/**
 * cong_editor_widget3_set_prehighlight_editor_area:
 * @editor_widget:
 * @editor_area:
 *
 * TODO: Write me
 */
void
cong_editor_widget3_set_prehighlight_editor_area (CongEditorWidget3 *editor_widget,
						  CongEditorArea* editor_area)
{
	GdkCursor *new_cursor = NULL;

	g_return_if_fail (editor_widget);

#if 0
	g_message ("set_prehighlight_area: %s", G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(editor_area)));
#endif

	if (PRIVATE (editor_widget)->prehighlight_area) {
		if (cong_editor_area_get_state (PRIVATE (editor_widget)->prehighlight_area) == GTK_STATE_PRELIGHT) {
			cong_editor_area_set_state (PRIVATE (editor_widget)->prehighlight_area,
						    GTK_STATE_NORMAL);
		}
	}
	
	PRIVATE (editor_widget)->prehighlight_area = editor_area;

	if (editor_area) {
		if (cong_editor_area_get_state (editor_area) == GTK_STATE_NORMAL) {
			cong_editor_area_set_state (editor_area,
						    GTK_STATE_PRELIGHT);
		}
	}

	if (editor_area) {
		new_cursor = cong_editor_area_get_cursor (editor_area);		
	}

	/* I believe that NULL is the default cursor: */
	gdk_window_set_cursor (GTK_WIDGET(editor_widget)->window,
			       new_cursor);

	if (new_cursor) {
		gdk_cursor_unref (new_cursor);
	}
}

/**
 * cong_editor_widget3_get_test_gc:
 * @editor_widget:
 *
 * TODO: Write me
 * Returns:
 */
GdkGC*
cong_editor_widget3_get_test_gc (CongEditorWidget3 *editor_widget)
{
	g_return_val_if_fail (editor_widget, NULL);

	return PRIVATE(editor_widget)->test_gc;
}

/**
 * cong_editor_widget3_get_area_at:
 * @editor_widget:
 * @x:
 * @y:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_widget3_get_area_at (CongEditorWidget3 *editor_widget,
				 gint x,
				 gint y)
{
	return cong_editor_area_get_deepest_child_at (PRIVATE(editor_widget)->root_area, 
						      x,
						      y);
}

/**
 * cong_flow_type_get_debug_string:
 * @flow_type:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_flow_type_get_debug_string(CongFlowType flow_type)
{
	switch (flow_type) {
	default: g_assert_not_reached();
	case CONG_FLOW_TYPE_BLOCK: return "FLOW_TYPE_BLOCK";
	case CONG_FLOW_TYPE_INLINE: return "FLOW_TYPE_INLINE";
	}
}

/**
 * cong_editor_widget3_add_popup_items:
 * @editor_widget:
 * @menu:
 *
 * TODO: Write me
 */
void
cong_editor_widget3_add_popup_items (CongEditorWidget3 *editor_widget,
				     GtkMenu *menu)
{
	GtkWidget *submenu_item;
	GtkWidget *submenu_menu;

	submenu_item = gtk_menu_item_new_with_label ( _("Input Method"));
	submenu_menu = gtk_menu_new ();

	gtk_menu_item_set_submenu (GTK_MENU_ITEM(submenu_item), submenu_menu);

	gtk_im_multicontext_append_menuitems (GTK_IM_MULTICONTEXT (PRIVATE (editor_widget)->im_context),
					      GTK_MENU_SHELL (submenu_menu));

	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), submenu_item);

	gtk_widget_show (submenu_item);
	gtk_widget_show (submenu_menu);
}

/**
 * cong_editor_widget3_get_preedit_data:
 * @editor_widget:
 * @output_string:
 * @output_pango_attr_list:
 * @output_cursor_pos:
 *
 * TODO: Write me
 */
void
cong_editor_widget3_get_preedit_data (CongEditorWidget3 *editor_widget,
				      gchar **output_string, 
				      PangoAttrList **output_pango_attr_list,
				      gint *output_cursor_pos)
{
	g_return_if_fail (IS_CONG_EDITOR_WIDGET3 (editor_widget));
	g_return_if_fail (output_string);
	g_return_if_fail (output_pango_attr_list);
	g_return_if_fail (output_cursor_pos);

	gtk_im_context_get_preedit_string (PRIVATE(editor_widget)->im_context,
					   output_string,
					   output_pango_attr_list,
					   output_cursor_pos);
}


/* Internal function implementations: */
#if LOG_EDITOR_NODES
static void
log_editor_node_event (const gchar *msg,
		       CongEditorNode *editor_node)
{
	gchar *debug_desc;

	g_assert (msg);
	g_assert (editor_node);

	debug_desc = cong_node_debug_description (cong_editor_node_get_node (editor_node));

	g_message ("%s:%s: %s", msg, G_OBJECT_TYPE_NAME (G_OBJECT (editor_node)), debug_desc);

	g_free (debug_desc);
}
#endif

/* Definitions of misc stuff: */
#if 0
static void 
render_area (CongEditorArea *area,
	     gpointer user_data)
{
#if 0
	cong_editor_area_recursive_render (area,
					   (GdkRectangle*)user_data);
#endif
}
#endif

/* Definitions of the GObject handlers: */
static void
cong_editor_widget3_finalize (GObject *object)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3 (object);

	g_message ("cong_editor_widget3_finalize");

	g_object_unref (G_OBJECT (PRIVATE(editor_widget)->im_context));
	
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

static void
add_typing_command (CongDocument *doc,
		    const gchar *str)
{
	CongCommand *cmd;
	CongSelection *selection = cong_document_get_selection  (doc);

	cmd = cong_document_begin_command (doc, 
					   _("Typing"),
					   "cong-typing");
	
	/* Do we have a selection? */
	if (cong_selection_get_logical_end(selection)->node) {
		
		/* Either we have an individual selected node, or a range of text.  In the latter case, we should delete it: */
		if (cong_node_is_valid_cursor_location (cong_selection_get_logical_end(selection)->node)) {
			cong_command_add_delete_selection (cmd);
		}
	}

	cong_command_add_insert_text_at_cursor (cmd, str);
	cong_command_add_nullify_selection (cmd);
	
	cong_document_end_command (doc,
				   cmd);
}

/* IM Context Callbacks
 */
static void     
commit_cb (GtkIMContext *context,
	   const gchar  *str,
	   CongEditorWidget3     *editor_widget)
{
	CongDocument *doc = cong_editor_widget3_get_document (editor_widget);

#if DEBUG_IM_CONTEXT
	g_message ("commit_cb: \"%s\"", str);
#endif

	add_typing_command (doc, str);


}

static void
editor_node_preedit_changed_cb (CongEditorWidget3 *widget, 
				CongEditorNode *editor_node, 
				gpointer user_data)
{
	cong_editor_node_line_regeneration_required (editor_node);
}

static void     
preedit_changed_cb (GtkIMContext *context,
		    CongEditorWidget3     *editor_widget)
{
	CongDocument *doc = cong_editor_widget3_get_document (editor_widget);
	CongCursor *cursor = cong_document_get_cursor (doc);

	gchar *preedit_string;
	PangoAttrList *preedit_pango_attr_list;
	gint preedit_cursor_pos;

	gtk_im_context_get_preedit_string (PRIVATE(editor_widget)->im_context,
					   &preedit_string,
					   &preedit_pango_attr_list,
					   &preedit_cursor_pos);

#if DEBUG_IM_CONTEXT
	g_message ("preedit_changed_cb: preedit string is: \"%s\",cursor pos=%i", preedit_string, preedit_cursor_pos);
#endif

	/* Need to regenerate the Pango data for old and new text/comment nodes; what happens when we switch between nodes?  etc...: */
	g_assert (cursor->location.node);
	cong_editor_widget3_for_each_editor_node (editor_widget,
						  cursor->location.node,
						  editor_node_preedit_changed_cb,
						  NULL);

	g_free (preedit_string);
	pango_attr_list_unref (preedit_pango_attr_list);
}

static gboolean 
retrieve_surrounding_cb (GtkIMContext *context,
			 CongEditorWidget3     *editor_widget)
{
#if DEBUG_IM_CONTEXT
	g_message ("retrieve_surrounding_cb");
#endif

	return FALSE;
}

static gboolean 
delete_surrounding_cb (GtkIMContext *context,
		       gint          offset,
		       gint          n_chars,
		       CongEditorWidget3     *editor_widget)
{
#if DEBUG_IM_CONTEXT
	g_message ("delete_surrounding_cb");
#endif

	return FALSE;
}

/* Definitions of the GtkWidget event handlers: */
/* Event handlers for widget: */
static void   realize  (GtkWidget        *widget)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(widget);

	gtk_im_context_set_client_window (PRIVATE(editor_widget)->im_context, widget->window);

	/* Call the parent method: */		
	GNOME_CALL_PARENT (GTK_WIDGET_CLASS, realize, (widget));
}

static void   unrealize (GtkWidget        *widget)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(widget);

	gtk_im_context_set_client_window (PRIVATE(editor_widget)->im_context, NULL);  

	/* Call the parent method: */		
	GNOME_CALL_PARENT (GTK_WIDGET_CLASS, unrealize, (widget));
}

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

	/* None of the areas handled the motion: */
	cong_editor_widget3_set_prehighlight_editor_area (editor_widget,
							  NULL);

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
	dispspec = cong_document_get_default_dispspec (doc);

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
			return cong_location_calc_prev_char(&cursor->location, doc, output_loc);
		}
	
	case GDK_Right:
		if (state & GDK_CONTROL_MASK) {
			return cong_location_calc_next_word(&cursor->location, doc, output_loc);
		} else {
			return cong_location_calc_next_char(&cursor->location, doc, output_loc);
		}
	case GDK_Home:
		if (state & GDK_CONTROL_MASK) {
			return cong_location_calc_document_start(&cursor->location, doc, output_loc);
		} else {
			return cong_location_calc_line_start(&cursor->location, doc, output_loc);
		}
	case GDK_End:
		if (state & GDK_CONTROL_MASK) {
			return cong_location_calc_document_end(&cursor->location, doc, output_loc);
		} else {
			return cong_location_calc_line_end(&cursor->location, doc, output_loc);
		}
	case GDK_Page_Up:
		return cong_location_calc_prev_page(&cursor->location, doc, output_loc);
	case GDK_Page_Down:
		return cong_location_calc_next_page(&cursor->location, doc, output_loc);
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

	if (gtk_im_context_filter_keypress (PRIVATE (editor_widget)->im_context, event)) {
		PRIVATE (editor_widget)->need_im_reset = TRUE;
		return TRUE;
	}

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

				CongCommand *cmd = cong_document_begin_command (doc, 
										_("Cursor Movement"),
										"cong-cursor-movement");
				CongLocation new_selection_start;
				CongLocation new_selection_end;
				
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

				cong_document_end_command (doc,
							   cmd);
			}
		}
		break;
	
	case GDK_BackSpace:
		if (cong_selection_get_logical_end(selection)->node) {
			CongCommand *cmd = cong_document_begin_command (doc, 
									_("Delete Selection"), 
									"cong-delete");
			cong_command_add_delete_selection (cmd);
			cong_document_end_command (doc,
						   cmd);
		} else {
			cong_cursor_del_prev_char(cursor, doc);
		}
		break;
	
	case GDK_Delete:
		if (cong_selection_get_logical_end(selection)->node) {
			CongCommand *cmd = cong_document_begin_command (doc, 
									_("Delete Selection"),
									"cong-delete");
			cong_command_add_delete_selection (cmd);
			cong_document_end_command (doc,
						   cmd);			
		} else {
			cong_cursor_del_next_char(cursor, doc);
		}
		break;

	case GDK_ISO_Enter:
	case GDK_Return:
		{
			if  (cong_location_exists(&cursor->location)) {
				if (cong_location_node_type(&cursor->location) == CONG_NODE_TYPE_TEXT) {
					CongDispspecElement *ds_element = cong_document_get_dispspec_element_for_node(doc, cursor->location.node->parent);

					if (ds_element) {
						switch (cong_dispspec_element_get_whitespace (ds_element)) {
						default: g_assert_not_reached ();
						case CONG_WHITESPACE_NORMALIZE:
							cong_cursor_paragraph_insert (cursor);
							break;
							
						case CONG_WHITESPACE_PRESERVE:
							add_typing_command (doc,
									    "\n");
							break;
						}
					} else {
						cong_cursor_paragraph_insert (cursor);
					}
				}			
			}
		}
		break;

	case GDK_Tab:
		/* Ignore the tab key for now... FIXME: what should we do? */
		break;
	
	default:
#if 0
		/* Is the user typing text? */
		if (event->length && event->string && strlen(event->string)) {
			CongCommand *cmd = cong_document_begin_command (doc, 
									_("Typing"),
									"cong-typing");

			/* Do we have a selection? */
                        if (cong_selection_get_logical_end(selection)->node) {

				/* Either we have an individual selected node, or a range of text.  In the latter case, we should delete it: */
				if (cong_node_is_valid_cursor_location (cong_selection_get_logical_end(selection)->node)) {
					cong_command_add_delete_selection (cmd);
				}
                        }
			cong_command_add_insert_text_at_cursor (cmd, event->string);
			cong_command_add_nullify_selection (cmd);

			cong_document_end_command (doc,
						   cmd);
		}
#endif
		break;
	}

	return TRUE;
}

static gboolean 
key_release_event_handler (GtkWidget *w, 
			 GdkEventKey *event, 
			 gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3 (w);

	LOG_GTK_WIDGET_SIGNAL1("key_release_event_handler");

	if (gtk_im_context_filter_keypress (PRIVATE (editor_widget)->im_context, event)) {
		PRIVATE (editor_widget)->need_im_reset = TRUE;
		return TRUE;
	}

	return FALSE;
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

static gint
focus_in_event_handler(GtkWidget *w, GdkEventFocus *event)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3 (w);
	gtk_im_context_focus_in(PRIVATE(editor_widget)->im_context);
	return FALSE;
}

static gint
focus_out_event_handler(GtkWidget *w, GdkEventFocus *event)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3 (w);
	gtk_im_context_focus_out(PRIVATE(editor_widget)->im_context);
	return FALSE;
}

/* Definitions of the CongDocument event handlers: */
/* Signal handling callbacks: */
static void 
set_editor_node_selection (CongEditorWidget3 *widget, 
			   CongEditorNode *editor_node, 
			   gpointer user_data)
{
	cong_editor_node_private_set_selected (editor_node,
					       (gboolean)user_data);
}

static void on_signal_selection_change_notify_after (CongDocument *doc, 
					       gpointer user_data) 
{ 
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 
	CongNodePtr selected_node;

	LOG_CONG_DOCUMENT_SIGNAL1("(CongEditorWidget3) on_signal_selection_change_notify_after");

	selected_node = cong_document_get_selected_node (doc);

	if (selected_node != PRIVATE(editor_widget)->selected_xml_node) {
		if (PRIVATE(editor_widget)->selected_xml_node) {
			cong_editor_widget3_for_each_editor_node (editor_widget,
								  PRIVATE(editor_widget)->selected_xml_node,
								  set_editor_node_selection,
								  GUINT_TO_POINTER(PRIVATE(editor_widget)->selected_xml_node == selected_node));
		}
		
		PRIVATE(editor_widget)->selected_xml_node = selected_node;
		
		if (PRIVATE(editor_widget)->selected_xml_node) {
			cong_editor_widget3_for_each_editor_node (editor_widget,
								  PRIVATE(editor_widget)->selected_xml_node,
								  set_editor_node_selection,
								  GUINT_TO_POINTER(TRUE));
		}
	}
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

static void
potentially_add_editor_node (CongEditorWidget3 *editor_widget,
			     CongTraversalNode *traversal_node)
{
	gboolean should_add_node;
	CongNodePtr xml_node;	

	g_return_if_fail (IS_CONG_EDITOR_WIDGET3 (editor_widget));
	g_return_if_fail (IS_CONG_TRAVERSAL_NODE (traversal_node));

	xml_node = cong_traversal_node_get_node (traversal_node);
	g_assert (xml_node);

	should_add_node = cong_node_should_be_visible_in_editor (xml_node);

#if 0
	{
		gchar *desc = cong_node_debug_description (xml_node);
		g_message ("Should add node %s? %s", desc, should_add_node ? "TRUE" : "FALSE");
		g_free (desc);
	}
#endif

	if (should_add_node) {
		/* Add this node: */
		CongEditorNode *editor_node = cong_editor_node_manufacture (editor_widget,
									    traversal_node);
		g_assert(editor_node);

		g_hash_table_insert (PRIVATE (editor_widget)->hash_of_traversal_node_to_editor_node,
				     traversal_node,
				     editor_node);

		LOG_EDITOR_NODE_EVENT("Adding editor node", editor_node);

		create_areas (editor_widget,
			      editor_node);
	}	
}

static void
potentially_remove_editor_node (CongEditorWidget3 *editor_widget,
				CongTraversalNode *traversal_node)
{
	CongNodePtr xml_node;
	CongEditorNode *editor_node;

	g_return_if_fail (IS_CONG_EDITOR_WIDGET3 (editor_widget));
	g_return_if_fail (IS_CONG_TRAVERSAL_NODE (traversal_node));

	xml_node = cong_traversal_node_get_node (traversal_node);

	editor_node = cong_editor_widget3_get_editor_node_for_traversal_node (editor_widget,
									      traversal_node);
	if (editor_node) {

		g_assert(editor_node);
		
		if (xml_node == PRIVATE (editor_widget)->selected_xml_node) {
			PRIVATE (editor_widget)->selected_xml_node = NULL;
		}

		LOG_EDITOR_NODE_EVENT("Removing editor node", editor_node);
		
		destroy_areas (editor_widget,
			       editor_node);
		
		/* Remove this editor_node: */
		g_hash_table_remove (PRIVATE (editor_widget)->hash_of_traversal_node_to_editor_node,
				     traversal_node);

		g_object_unref (G_OBJECT (editor_node));
	}
}
		 

/* Declarations of CongDocumentTraversal signal handlers: */
static void
on_traversal_node_added (CongDocumentTraversal *traversal, 
			 CongTraversalNode *traversal_node,
			 gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data);

	potentially_add_editor_node (editor_widget, traversal_node);
}

static void
on_traversal_node_removed (CongDocumentTraversal *traversal, 
			   CongTraversalNode *traversal_node,
			   gpointer user_data)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(user_data); 

	potentially_remove_editor_node (editor_widget, traversal_node);	
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
recursive_add_editor_nodes (CongEditorWidget3 *editor_widget,
			    CongTraversalNode *traversal_node)
{
	CongTraversalNode *iter;

	g_return_if_fail (IS_CONG_EDITOR_WIDGET3 (editor_widget));
	g_return_if_fail (IS_CONG_TRAVERSAL_NODE (traversal_node));

	/* Add this node: */
	potentially_add_editor_node (editor_widget, traversal_node);

	/* Recurse over children: */
	for (iter = cong_traversal_node_get_first_child (traversal_node); iter; iter = cong_traversal_node_get_next (iter)) {
		recursive_add_editor_nodes (editor_widget,
					    iter);		
	}
}

static void 
populate_widget3(CongEditorWidget3 *widget)
{
	CongEditorWidget3 *editor_widget = CONG_EDITOR_WIDGET3(widget);
	CongDocument *doc;

	doc = cong_editor_widget3_get_document(widget);
	
	recursive_add_editor_nodes (editor_widget,
				    cong_document_get_root_traversal_node (doc));
       	/* FIXME: Also need to do the opposite when widget is destroyed */
}

/**
 * cong_editor_widget3_node_should_have_editor_node:
 * @node:
 *
 * TODO: Write me
 */
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

static CongEditorLineManager*
get_line_manager_for_node (CongEditorWidget3 *widget,
			   CongEditorNode *editor_node)
{
	CongNodePtr node = cong_editor_node_get_node (editor_node);

	/* Determine the parent area where the new area should be inserted: */
	if (node->parent) {
		CongEditorNode *parent_editor_node;
		
		parent_editor_node = cong_editor_node_get_traversal_parent (editor_node);
		g_assert (parent_editor_node);
		
		return cong_editor_node_get_line_manager_for_children (parent_editor_node);
		
	} else {
		/* Root of the document; insert below the widget's root_area: */
		g_assert(cong_node_type(node) == CONG_NODE_TYPE_DOCUMENT);
		
		return PRIVATE (widget)->root_line_manager;
	}
}

static void 
create_areas(CongEditorWidget3 *widget,
	     CongEditorNode *editor_node)
{
	CongEditorLineManager *parent_line_manager = NULL;
#if 0
	enum CongFlowType flow_type;
#endif

#if LOG_EDITOR_AREAS
	{
		CongNodePtr node = cong_editor_node_get_node (editor_node);
		gchar *node_desc = cong_node_debug_description(node);

		LOG_EDITOR_AREA2("create_areas for %s", node_desc);

		g_free(node_desc);
	}
#endif

#if 0
	flow_type = cong_editor_node_get_flow_type (editor_node);

#if LOG_EDITOR_AREAS
	g_message("flow type = %s", cong_flow_type_get_debug_string(flow_type));
#endif
#endif

	parent_line_manager = get_line_manager_for_node (widget,
							 editor_node);
	g_assert (parent_line_manager);
	g_assert (IS_CONG_EDITOR_LINE_MANAGER (parent_line_manager));

	cong_editor_line_manager_add_node (parent_line_manager,
					   editor_node);	
}

static void 
destroy_areas(CongEditorWidget3 *widget,
	      CongEditorNode *editor_node)
{
	CongEditorLineManager *parent_line_manager = NULL;

#if LOG_EDITOR_AREAS
	{
		CongNodePtr node = cong_editor_node_get_node (editor_node);
		gchar *node_desc = cong_node_debug_description(node);

		LOG_EDITOR_AREA2("destroy_areas for %s", node_desc);
		
		g_free(node_desc);
	}
#endif

	parent_line_manager = get_line_manager_for_node (widget,
							 editor_node);
	g_assert (parent_line_manager);
	g_assert (IS_CONG_EDITOR_LINE_MANAGER (parent_line_manager));

	cong_editor_line_manager_remove_node (parent_line_manager,
					      editor_node);
}


CongPrimaryWindow*
cong_editor_widget_get_primary_window(CongEditorWidget3 *editor_widget)
{
	return PRIVATE(editor_widget)->primary_window;
}
