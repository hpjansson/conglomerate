/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <glade/glade-xml.h>

#include <stdlib.h>
#include "global.h"
#include "cong-dispspec.h"
#include "cong-dispspec-element.h"
#include "cong-document.h"
#include "cong-view.h"
#include "cong-app.h"
#include "cong-selection.h"
#include "cong-range.h"
#include "cong-editor-widget.h"
#include "cong-service-tool.h"
#include "cong-command.h"
#include "cong-plugin-manager.h"
#include "cong-util.h"
#include "cong-ui-hooks.h"
#include "cong-primary-window.h"

#include "cong-error-dialog.h"

#define UI_PATH_CONTEXT_MENU ("ui/ContextMenu")
GtkActionGroup *s_action_group = NULL;

/* Callback marshallers for the various UI action hooks: */
static void
action_marshaller_Document (GtkAction *action, 
			    gpointer user_data)
{
	CongDocument *doc = CONG_DOCUMENT (user_data);
	CongUICallback_Document wrapped_callback;

	g_assert (IS_CONG_DOCUMENT (doc));

	wrapped_callback = g_object_get_data (G_OBJECT (action),
					      "wrapped_callback");
	g_assert (wrapped_callback);

	wrapped_callback (doc);
}

static void
action_marshaller_Document_SelectedNode_ParentWindow (GtkAction *action, 
						      gpointer user_data)
{
	CongDocument *doc;
	GtkWindow *parent_window;
	CongUICallback_Document_SelectedNode_ParentWindow wrapped_callback;

	doc = g_object_get_data (G_OBJECT (action),
				 "document");
	g_assert (IS_CONG_DOCUMENT (doc));

	parent_window = g_object_get_data (G_OBJECT (action),
					   "parent_window");

	wrapped_callback = g_object_get_data (G_OBJECT (action),
					      "wrapped_callback");
	g_assert (wrapped_callback);

	wrapped_callback (doc, 
			  parent_window);
}

static void
action_marshaller_Document_ElementDescription_SelectedNode (GtkAction *action, 
							    gpointer user_data)
{
	CongDocument *doc;
	CongElementDescription *element_desc;
	CongUICallback_Document_ElementDescription_SelectedNode wrapped_callback;

	doc = g_object_get_data (G_OBJECT (action),
				 "document");
	g_assert (IS_CONG_DOCUMENT (doc));

	element_desc = g_object_get_data (G_OBJECT (action),
					  "element_desc");

	wrapped_callback = g_object_get_data (G_OBJECT (action),
					      "wrapped_callback");
	g_assert (wrapped_callback);

	wrapped_callback (doc, 
			  element_desc);
}

/* Implementations: */
/**
 * cong_action_attach_callback_Document:
 * @action:
 * @callback:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
GtkAction* 
cong_action_attach_callback_Document (GtkAction *action,
				      CongUICallback_Document callback,
				      CongDocument *doc)
{
	g_return_val_if_fail (action, NULL);
	g_return_val_if_fail (callback, NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);

	g_object_set_data (G_OBJECT (action),
			   "wrapped_callback",
			   callback);

	g_signal_connect (G_OBJECT (action),
			  "activate",
			  G_CALLBACK (action_marshaller_Document), 
			  doc);
	return action;
}

/**
 * cong_action_attach_callback_Document_SelectedNode_ParentWindow:
 * @action:
 * @callback:
 * @doc:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
GtkAction*
cong_action_attach_callback_Document_SelectedNode_ParentWindow (GtkAction *action,
							CongUICallback_Document_SelectedNode_ParentWindow callback,
							CongDocument *doc,
							GtkWindow *parent_window)
{
	g_return_val_if_fail (action, NULL);
	g_return_val_if_fail (callback, NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);

	g_object_set_data (G_OBJECT (action),
			   "wrapped_callback",
			   callback);

	g_signal_connect (G_OBJECT (action), 
			  "activate",
			  G_CALLBACK (action_marshaller_Document_SelectedNode_ParentWindow), 
			  NULL);

	g_object_set_data (G_OBJECT (action),
			   "document",
			   doc);

	g_object_set_data (G_OBJECT (action),
			   "parent_window",
			   parent_window);

	return action;
}

/**
 * cong_action_attach_callback_Document_ElementDescription_SelectedNode:
 * @action:
 * @callback:
 * @doc:
 * @ds_element:
 *
 * TODO: Write me
 * Returns:
 */
GtkAction*
cong_action_attach_callback_Document_ElementDescription_SelectedNode (GtkAction *action, 
								      CongUICallback_Document_ElementDescription_SelectedNode callback,
								      CongDocument *doc,
								      CongElementDescription *element_desc)
{
	g_return_val_if_fail (action, NULL);
	g_return_val_if_fail (callback, NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	/* element_desc can be NULL, meaning ask the user for a desc */

	g_object_set_data (G_OBJECT (action),
			   "wrapped_callback",
			   callback);

	g_signal_connect (G_OBJECT (action), 
			  "activate",
			  G_CALLBACK (action_marshaller_Document_ElementDescription_SelectedNode), 
			  NULL);

	g_object_set_data (G_OBJECT (action),
			   "document",
			   doc);

	/* FIXME: do we need to clone this? Plus we're leaking memory... */
	if (element_desc) {
		g_object_set_data (G_OBJECT (action),
				   "element_desc",
				   cong_element_description_clone (element_desc));
	} else {
		g_object_set_data (G_OBJECT (action),
				   "element_desc",
				   NULL);
	}

	return action;
}

/* 
   Set up the callback, but ensure action's GObject data has the following set:
  
   "document" -> the CongDocument
   "parent_window" -> the  parent_window
  
   The CongNodePtr is passed to the callback function
*/
/**
 * cong_action_attach_callback_legacy:
 * @action:
 * @callback:
 * @doc:
 * @node:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
GtkAction*
cong_action_attach_callback_legacy (GtkAction *action,
				    gint (*callback)(GtkAction *action, CongNodePtr node),
				    CongDocument *doc,
				    CongNodePtr node,
				    GtkWindow *parent_window)
{
	g_return_val_if_fail (action, NULL);
	g_return_val_if_fail (callback, NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	g_return_val_if_fail (node, NULL);

	g_signal_connect (G_OBJECT (action), 
			  "activate",
			  G_CALLBACK (callback), 
			  node);

	g_object_set_data (G_OBJECT (action),
			   "document",
			   doc);

	g_object_set_data (G_OBJECT (action),
			   "parent_window",
			   parent_window);

	return action;
}

/**
 * cong_menu_add_action:
 * @parent_ui_path:
 * @action:
 *
 * TODO: Write me
 * Returns:
 */
void
cong_menu_add_action (CongPrimaryWindow *primary_window,
		      const gchar *parent_ui_path,
		      GtkAction *action,
		      GtkUIManagerItemType type)
{
	g_assert (primary_window);
	g_assert (parent_ui_path);
	g_assert (action);

	gtk_action_group_add_action (s_action_group,
				     action);
	
	/* Add to the UI: */
	/* Apparently the UI path must not have a leading slash; see http://mail.gnome.org/archives/gtk-app-devel-list/2004-July/msg00263.html */

	gtk_ui_manager_add_ui (cong_primary_window_get_ui_manager (primary_window),
			       gtk_ui_manager_new_merge_id (cong_primary_window_get_ui_manager (primary_window)),
			       parent_ui_path,
			       gtk_action_get_name (action),
			       gtk_action_get_name (action),
			       type,
			       FALSE);
}

static GtkAction*
add_action_to_popup_with_callback_Document_SelectedNode_ParentWindow (const gchar *parent_ui_path,
								      GtkAction *action, 
								      CongUICallback_Document_SelectedNode_ParentWindow tree_callback,
								      CongDocument *doc,
								      GtkWindow *parent_window,
								      gboolean is_sensitive,
								      CongPrimaryWindow *primary_window);

static void
callback_invoke_node_tool (GtkAction *action, 
			   gpointer user_data)
{
	CongServiceNodeTool *node_tool = CONG_SERVICE_NODE_TOOL (g_object_get_data (G_OBJECT (action), "node_tool"));
	CongDocument *doc = CONG_DOCUMENT (g_object_get_data (G_OBJECT (action), "doc"));
	CongNodePtr node = user_data;
	GtkWindow *parent_window = GTK_WINDOW (g_object_get_data (G_OBJECT (action), "parent_window"));

	g_assert (node_tool);
	g_assert (doc);
	g_assert (node);

	cong_service_node_tool_invoke (node_tool, 
				       doc,
				       node,
				       parent_window);
}

     

struct add_node_tool_callback_data
{
	CongDocument *doc;
	CongNodePtr node;
	GtkWindow *parent_window;
	CongPrimaryWindow *primary_window;
};

GtkAction*
cong_action_new (const gchar *name,
		 const gchar *label,
		 const gchar *tooltip,
		 GdkPixbuf *pixbuf)
{
	/* FIXME: it wants a stock_id rather than a pixbuf, see http://bugzilla.gnome.org/show_bug.cgi?id=149619 
	   So we won't get any pixbufs for now...
	*/
	return gtk_action_new (name,
			       label,
			       tooltip,
			       NULL);

}

GtkAction*
cong_action_new_from_stock (const gchar *name,
			    const gchar *label,
			    const gchar *stock_id)
{
	/* FIXME: GTK should provide a way to get the label from the stock_id */
	return gtk_action_new (name,
			       label,
			       NULL,
			       stock_id);
}

void
cong_action_set_sensitive (GtkAction *action,
			   gboolean sensitive)
{
	g_object_set (G_OBJECT (action), 
		      "sensitive", sensitive, 
		      NULL);
}

static void
add_node_tool (CongServiceNodeTool *node_tool, 
	       struct add_node_tool_callback_data *callback_data,
	       gboolean is_sensitive)
{
	gchar *action_name = g_strdup_printf ("NodeTool-%s", cong_service_get_id (CONG_SERVICE (node_tool)));
	GtkAction *action = cong_action_new (action_name,
					     cong_service_tool_get_menu_text (CONG_SERVICE_TOOL (node_tool)),
					     cong_service_tool_get_tip_text (CONG_SERVICE_TOOL (node_tool)),
					     NULL); /* FIXME:  ought to have an icon */
	cong_action_set_sensitive (action,
				   is_sensitive);

	g_signal_connect (G_OBJECT (action), 
			  "activate",
			  G_CALLBACK (callback_invoke_node_tool), 
			  callback_data->node);

	g_object_set_data (G_OBJECT (action),
			   "node_tool",
			   node_tool);

	g_object_set_data (G_OBJECT (action),
			   "doc",
			   callback_data->doc);

	g_object_set_data (G_OBJECT (action),
			   "parent_window",
			   callback_data->parent_window);

	cong_menu_add_action (callback_data->primary_window,
			      UI_PATH_CONTEXT_MENU,
			      action,
			      GTK_UI_MANAGER_MENUITEM);
}

static void
add_node_tool_callback (CongServiceNodeTool *node_tool, 
			gpointer user_data)
{
	struct add_node_tool_callback_data *callback_data = user_data;
	enum NodeToolFilterResult filter_result = cong_service_node_tool_filter_node (node_tool, 
										      callback_data->doc,
										      callback_data->node);
	switch (filter_result) {
	default: g_assert_not_reached ();
	case NODE_TOOL_HIDDEN: 
		/* do nothing */
		break;
		
	case NODE_TOOL_INSENSITIVE:
		add_node_tool (node_tool,
			       callback_data,
			       FALSE);
		break;

	case NODE_TOOL_AVAILABLE:
		add_node_tool (node_tool,
			       callback_data,
			       TRUE);
		break;
	}

}




static void
span_tag_removal_popup_init (CongDispspec *ds, 
			     CongCursor *cursor, 
			     gint (*callback)(GtkAction *action, CongNodePtr node_ptr),
			     CongDocument *doc, 
			     GList *list,
			     GtkWindow *parent_window,
			     CongPrimaryWindow *primary_window);

static gint editor_popup_callback_remove_span_tag(GtkAction *action, 
						  CongNodePtr node_ptr);

static void
structural_tag_popup_init (const gchar *label,
			   const gchar *action_prefix,
			   CongUICallback_Document_ElementDescription_SelectedNode callback,
			   CongDocument *doc,
			   GList *list_of_element_desc,
			   CongPrimaryWindow *primary_window);

/*
  EDITOR POPUP CODE:
*/
/* 
   The popup menu widget (and some actions) have a pointer to the CongDocument set as a user property named "doc":
*/
static gint editor_popup_callback_action_selected (GtkAction *action, 
						   CongElementDescription *element_desc)
{
	CongNodePtr new_element;

	CongDocument *doc;
	CongDispspec *ds;
	CongSelection *selection;
	CongCursor *cursor;
	gchar *username;
	const CongLocation *logical_start;

	g_return_val_if_fail (element_desc, TRUE);

	doc = g_object_get_data(G_OBJECT (action),
				"doc");
	g_assert(doc);

	ds = cong_document_get_default_dispspec (doc);

	selection = cong_document_get_selection(doc);
	cursor = cong_document_get_cursor(doc);
	logical_start = cong_selection_get_logical_start (selection);

	username = cong_element_description_make_user_name (element_desc,
							    ds);

	new_element = cong_element_description_make_node (element_desc, 
							  doc,
							  cong_location_parent (logical_start));
	{
		gchar *desc = g_strdup_printf (_("Apply span tag: %s"), username);
		CongCommand *cmd = cong_document_begin_command (doc, desc, NULL);

		g_free (desc);

		if (cong_command_can_add_reparent_selection (cmd, new_element)) {
			cong_command_add_reparent_selection (cmd, new_element);
		}

		cong_document_end_command (doc, cmd);
	}

	g_free (username);

	return(TRUE);
}

/**
 * popup_action_handlers_destroy:
 * @widget:
 * @data:
 *
 * TODO: Write me
 */
void popup_action_handlers_destroy(GtkAction *action, gpointer data)
{
	UNUSED_VAR(int sig);
}

#if 0
/**
 * editor_popup_show:
 * @widget:
 * @bevent:
 *
 * TODO: Write me
 */
void 
editor_popup_show(GtkAction *action, GdkEventButton *bevent)
{
	gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL, bevent->button,
		       bevent->time);
	
	return;
}
#endif

/**
 * editor_popup_init:
 * @doc:
 *
 * TODO: Write me
 */
void 
editor_popup_init(CongDocument *doc)
{
	g_return_if_fail (cong_app_singleton());

	if (cong_app_singleton()->popup) gtk_widget_destroy(cong_app_singleton()->popup);
	cong_app_singleton()->popup = gtk_menu_new();

	g_object_set_data(G_OBJECT(cong_app_singleton()->popup),
			  "doc",
			  doc);

	gtk_menu_set_title(GTK_MENU(cong_app_singleton()->popup), "Editing menu");
}

static gint editor_popup_callback_remove_span_tag(GtkAction *action, CongNodePtr node_ptr)
{ 
	CongDocument *doc = (CongDocument*)(g_object_get_data(G_OBJECT (action), "document"));
	CongCommand *cmd = cong_document_begin_command (doc, _("Remove Span Tag"), NULL);
	CongNodePtr parent = node_ptr->parent;

	cong_document_begin_edit(doc);

	cong_command_add_remove_tag (cmd,
				     node_ptr);

	cong_command_add_merge_adjacent_text_children_of_node (cmd, 
							       parent);

	cong_document_end_edit(doc);

	cong_document_end_command (doc, cmd);

	return TRUE;
}

static gint editor_popup_callback_paste(GtkAction *action, CongDocument *doc)
{
	g_assert(doc);
	
	cong_document_paste_clipboard (doc);
	return TRUE;
}


static void
add_comment_menu_actions (CongDocument *doc,
			  CongNodePtr node,
			  GtkWindow *parent_window,
			  CongPrimaryWindow *primary_window)
{
	GtkAction *action;

	switch (cong_node_type (node)) {
	case CONG_NODE_TYPE_DOCUMENT:
		/* FIXME: should look through the various node types here; I suspect not all are appropriate */
		break;

	default:
		/* Convert to comment: */
		{
			cong_util_add_menu_separator (primary_window,UI_PATH_CONTEXT_MENU);

			action = cong_action_new ("ConvertToComment",
						  _("Convert to a comment"), 
						  NULL, /* FIXME */
						  cong_util_load_icon("cong-comment"));
			cong_menu_add_action (primary_window,
					      UI_PATH_CONTEXT_MENU,
					      action,
					      GTK_UI_MANAGER_MENUITEM);
			cong_action_attach_callback_Document_SelectedNode_ParentWindow (action, 
											cong_ui_hook_tree_convert_to_comment,
											doc,
											parent_window);
		}
		break;

	case CONG_NODE_TYPE_COMMENT:
		/* Convert from comment: */
		{
			cong_util_add_menu_separator (primary_window,UI_PATH_CONTEXT_MENU);

			action = cong_action_new ("ConvertFromComment",
						  _("Uncomment"), 
						  _("Convert a comment containing XML source code into the corresponding code"),
						  cong_util_load_icon("cong-uncomment"));
			cong_menu_add_action (primary_window,
					      UI_PATH_CONTEXT_MENU, 
					      action,
					      GTK_UI_MANAGER_MENUITEM);
			cong_action_attach_callback_Document_SelectedNode_ParentWindow (action, 
											cong_ui_hook_tree_convert_from_comment,
											doc,
											parent_window);
			/* FIXME: set sensitivity */
		}
		break;
	}
}

/**
 * editor_popup_build:
 * @editor_widget:
 * @parent_window:
 *
 * TODO: Write me
 */
void 
editor_popup_build (CongEditorWidget3 *editor_widget, 
		    GtkWindow *parent_window)
{
	CongPrimaryWindow *primary_window;
	GtkAction *action;
	CongDispspec *dispspec;
	CongCursor *cursor;
	CongDocument *doc;
	CongSelection *selection;
	CongRange *range;
	GList *present_span_tags_list = NULL;
	GList *available_span_tags_desc_list = NULL;		
	
	g_return_if_fail (IS_CONG_EDITOR_WIDGET3 (editor_widget));

	primary_window = cong_editor_widget_get_primary_window (editor_widget);

	/* Destroy any existing action group: */
	if (s_action_group) {
		gtk_ui_manager_remove_action_group (cong_primary_window_get_ui_manager (primary_window), 
						    s_action_group);

		g_object_unref (G_OBJECT (s_action_group));		
	}

	/* Create new action group; it will get populated below: */
	s_action_group = gtk_action_group_new ("ContextMenu");

	doc = cong_editor_widget3_get_document (editor_widget);
	dispspec = cong_document_get_default_dispspec(doc);
	cursor = cong_document_get_cursor(doc);
	selection = cong_document_get_selection(doc);
	range = cong_selection_get_ordered_range(selection);
	primary_window = cong_editor_widget_get_primary_window (editor_widget);

	present_span_tags_list = xml_all_present_span_elements (dispspec, 
								cursor->location.node);

	if (cursor->location.node) {

		/* Build list of dynamic tag insertion tools */
		/*  build the list of valid inline tags here */
		available_span_tags_desc_list = cong_document_get_valid_new_child_elements (doc,
											    cursor->location.node->parent,
											    CONG_ELEMENT_TYPE_SPAN);
		available_span_tags_desc_list = cong_element_description_list_sort (available_span_tags_desc_list,
										    dispspec);
	}

	if (cong_app_singleton()->popup) gtk_widget_destroy(cong_app_singleton()->popup);
	editor_popup_init(doc);
	
	if (present_span_tags_list) {
		CongNodePtr node = (CongNodePtr)(present_span_tags_list->data);

		action = cong_action_new_from_stock ("NodeProperties",
						     _("_Properties"),
						     GTK_STOCK_PROPERTIES);
		cong_action_attach_callback_Document_SelectedNode_ParentWindow (action, 
										cong_ui_hook_tree_properties,
										doc,
										parent_window);

		cong_menu_add_action (primary_window,
				      UI_PATH_CONTEXT_MENU,
				      action,
				      GTK_UI_MANAGER_MENUITEM);

		/* Add any plugin tools for this node: */
		{
			struct add_node_tool_callback_data callback_data;
			
			callback_data.doc = doc;
			callback_data.node = node;
			callback_data.parent_window = parent_window;
			callback_data.primary_window = primary_window;

			
			cong_plugin_manager_for_each_node_tool (cong_app_get_plugin_manager (cong_app_singleton()), 
								add_node_tool_callback,
								&callback_data);
		}
		
		cong_util_add_menu_separator (primary_window,UI_PATH_CONTEXT_MENU);
	}

	
	/* Fixed editing tools */
#if 0
	action = cong_action_new_from_stock ("NodeCut",
					     _("_Cut"),
					     GTK_STOCK_CUT);
	cong_action_attach_callback_Document (action,
					      cong_document_cut_selection,
					      doc);
	cong_action_set_sensitive (action,
				   cong_range_can_be_cut (range));
	cong_menu_add_action (primary_window,
			      UI_PATH_CONTEXT_MENU,
			      action,
			      GTK_UI_MANAGER_MENUITEM);

	action = cong_action_new_from_stock ("NodeCopy",
					     _("_Copy"),
					     GTK_STOCK_COPY);
	cong_action_attach_callback_Document (action,
					      cong_document_copy_selection,
					      doc);
	cong_action_set_sensitive (action,
				   cong_range_can_be_copied (range));
	cong_menu_add_action (primary_window,
			      UI_PATH_CONTEXT_MENU,
			      action,
			      GTK_UI_MANAGER_MENUITEM);
#endif
	
	action = cong_action_new_from_stock ("NodePaste",
					     _("_Paste"),
					     GTK_STOCK_PASTE);
	g_signal_connect(G_OBJECT (action), "activate",
			 G_CALLBACK (editor_popup_callback_paste), doc);
	cong_action_set_sensitive (action,
				   cong_document_can_paste(doc));
	cong_menu_add_action (primary_window,
			      UI_PATH_CONTEXT_MENU,
			      action,
			      GTK_UI_MANAGER_MENUITEM);

	if (present_span_tags_list != NULL) {
		cong_util_add_menu_separator (primary_window,UI_PATH_CONTEXT_MENU);
		
		span_tag_removal_popup_init (dispspec,
					     cursor, 
					     editor_popup_callback_remove_span_tag, 
					     doc, 
					     present_span_tags_list,
					     parent_window,
					     primary_window);
	}
	
	cong_util_add_menu_separator (primary_window,UI_PATH_CONTEXT_MENU);

	if (available_span_tags_desc_list) {
		GList *iter;
		
		for (iter=available_span_tags_desc_list; iter; iter=iter->next) {
			CongElementDescription *element_desc = (CongElementDescription *)iter->data;
			
			action = cong_util_make_action_for_element_desc ("AddSpanTag",
									 element_desc,
									 doc);
			/* FIXME: perhaps we should composite an "add" icon to the element's icon? */
			
			g_signal_connect(G_OBJECT (action), "activate",
					 G_CALLBACK (editor_popup_callback_action_selected), 
					 element_desc);
			
			g_object_set_data(G_OBJECT (action),
					  "doc",
					  doc);
			
			cong_menu_add_action (primary_window,
					      UI_PATH_CONTEXT_MENU,
					      action,
					      GTK_UI_MANAGER_MENUITEM);
		}
	}

#if 0
	/* FIXME: */
	cong_editor_widget3_add_popup_actions (editor_widget,
					       GTK_MENU(cong_app_singleton()->popup));
#endif

	g_list_free (present_span_tags_list);
	g_list_free (available_span_tags_desc_list);

	/* Add the action group to the UI: */
	gtk_ui_manager_insert_action_group (cong_primary_window_get_ui_manager (primary_window), 
					    s_action_group,
					    0);	
	g_message (gtk_ui_manager_get_ui (cong_primary_window_get_ui_manager (primary_window)));

}

/*
  TREE POPUP CODE:
*/


/* the popup actions have the data "document" set on them: */


static GtkAction*
add_action_to_popup_with_callback_Document_SelectedNode_ParentWindow (const gchar *parent_ui_path,
								      GtkAction *action, 
								      CongUICallback_Document_SelectedNode_ParentWindow callback,
								      CongDocument *doc,
								      GtkWindow *parent_window,
								      gboolean is_sensitive,
								      CongPrimaryWindow *primary_window)
{
	g_return_val_if_fail (parent_ui_path, NULL);
	g_return_val_if_fail (action, NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	g_return_val_if_fail (callback, NULL);	

	cong_action_attach_callback_Document_SelectedNode_ParentWindow (action,
									callback,
									doc,
									parent_window);
	cong_action_set_sensitive (action,
				   is_sensitive);   

	cong_menu_add_action (primary_window,
			      parent_ui_path,
			      action,
			      GTK_UI_MANAGER_MENUITEM);			      

	return action;
}

static gchar*
make_submenu_ui_path (const gchar *menu_name)
{
	g_assert (menu_name);

	return g_strdup_printf ("%s/%s", UI_PATH_CONTEXT_MENU, menu_name);
}

static void
span_tag_removal_popup_init (CongDispspec *ds, 
			     CongCursor *cursor, 
			     gint (*callback)(GtkAction *action, CongNodePtr node_ptr),
			     CongDocument *doc, 
			     GList *list,
			     GtkWindow *parent_window,
			     CongPrimaryWindow *primary_window) 
{
	GList *current;
	gchar *submenu_ui_path;

#define REMOVE_SPAN_TAG_SUBMENU ("RemoveSpanTagSubmenu")
	submenu_ui_path = make_submenu_ui_path (REMOVE_SPAN_TAG_SUBMENU);

	for (current = g_list_last(list); current; current = g_list_previous(current)) {
		
		CongNodePtr node = (CongNodePtr)(current->data);
		CongDispspecElement *ds_element = cong_dispspec_lookup_node(ds, node);

		/* FIXME: if you have several span tags of the same kind, the actions end up with the same name */

		GtkAction *action = cong_util_make_action_for_dispspec_element ("RemoveSpanTag",
										ds_element); /* FIXME: should we composite a deletion icon onto the pixbuf? */

		cong_action_attach_callback_legacy (action,
						    callback,					       
						    doc,
						    node,
						    parent_window);
		cong_menu_add_action (primary_window,
				      submenu_ui_path,
				      action,
				      GTK_UI_MANAGER_MENUITEM);
	}

	g_free (submenu_ui_path);
}

static void
structural_tag_popup_init (const gchar *label,
			   const gchar *action_prefix,
			   CongUICallback_Document_ElementDescription_SelectedNode callback,
			   CongDocument *doc,
			   GList *list_of_element_desc,
			   CongPrimaryWindow *primary_window)
{
	gchar *submenu_ui_path;

	g_return_if_fail (callback);
	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	/* use the action_prefix to name the submenu: */
	submenu_ui_path = make_submenu_ui_path (action_prefix);

	if (list_of_element_desc) {
		GList *current;
		CongDispspec *ds;

		ds = cong_document_get_default_dispspec (doc);
	
		list_of_element_desc = cong_element_description_list_sort (list_of_element_desc, 
									   ds);
		
		for (current = g_list_first (list_of_element_desc); current; current = g_list_next (current)) {
			
			CongElementDescription *element_desc = (CongElementDescription *)(current->data);
			
			GtkAction *action = cong_util_make_action_for_element_desc (action_prefix,
										    element_desc,
										    doc);
			
			cong_action_attach_callback_Document_ElementDescription_SelectedNode (action,
											      callback,
											      doc,
											      element_desc);
			cong_menu_add_action (primary_window,
					      submenu_ui_path,
					      action,
					      GTK_UI_MANAGER_MENUITEM);
		}

		/* Finally, add a way to add arbitary XML elements: */
		{
			GtkAction *action;
			action = cong_action_new ("OtherXMLElement",
						  _("Other XML element..."),
						  NULL, /* FIXME:  ought to have a tooltip */
						  NULL); /* FIXME:  ought to have an icon */
		
			cong_action_attach_callback_Document_ElementDescription_SelectedNode (action,
											      callback,
											      doc,
											      NULL);
			cong_menu_add_action (primary_window,
					      submenu_ui_path,
					      action,
					      GTK_UI_MANAGER_MENUITEM);
		
		}
	}

	g_free (submenu_ui_path);
}

/**
 * cong_ui_popup_init:
 * @doc:
 * @node:
 * @parent_window:
 * @primary_window:
 *
 * TODO: Write me
 */
void
cong_ui_popup_init (CongDocument *doc, 
		    CongNodePtr node,
		    GtkWindow *parent_window,
		    CongPrimaryWindow *primary_window)
{
	CongDispspec *ds;

	g_assert(doc);
	ds = cong_document_get_default_dispspec(doc);

	/* Eventually we want to eliminate the node as a parameter altogether, and work with the selection (possibly the cursor as well?): */
	if (cong_node_type(node)==CONG_NODE_TYPE_ELEMENT) {
		g_assert (node==cong_document_get_selected_node (doc));
	}

	/* Destroy any existing action group: */
	if (s_action_group) {
		gtk_ui_manager_remove_action_group (cong_primary_window_get_ui_manager (primary_window), 
						    s_action_group);

		g_object_unref (G_OBJECT (s_action_group));		
	}

	/* Create new action group; it will get populated below: */
	s_action_group = gtk_action_group_new ("ContextMenu");

	/* Add "Properties" action: */
	add_action_to_popup_with_callback_Document_SelectedNode_ParentWindow (UI_PATH_CONTEXT_MENU,
									      cong_action_new_from_stock ("NodeProperties",
													  _("_Properties"),
													  GTK_STOCK_PROPERTIES),
									      cong_ui_hook_tree_properties,
									      doc,
									      parent_window,
									      TRUE,
									      primary_window);	

	/* Add clipboard operations: */
	/* FIXME:  the clipboard stuff only currently works for elements, hence we should filter on these for now: */
	if (cong_node_type(node)==CONG_NODE_TYPE_ELEMENT) {

#if 0
		cong_util_add_menu_separator (primary_window,UI_PATH_CONTEXT_MENU);

		add_action_to_popup_with_callback_Document_SelectedNode_ParentWindow (UI_PATH_CONTEXT_MENU,
										      cong_action_new_from_stock ("NodeCut",
														  _("_Cut"),
														  GTK_STOCK_CUT),
										      cong_ui_hook_tree_cut,
										      doc,
										      parent_window,
										      cong_node_can_be_cut (node),
										      primary_window);
		add_action_to_popup_with_callback_Document_SelectedNode_ParentWindow (UI_PATH_CONTEXT_MENU,
										      cong_action_new_from_stock ("NodeCopy",
														  _("_Copy"),
														  GTK_STOCK_COPY),
										      cong_ui_hook_tree_copy,
										      doc,
										      parent_window,
										      cong_node_can_be_copied (node),
										      primary_window);
#endif
		add_action_to_popup_with_callback_Document_SelectedNode_ParentWindow (UI_PATH_CONTEXT_MENU,
										      cong_action_new ("NodePasteInto",
												       _("Paste into"),
												       NULL, /* FIXME:  ought to have a tooltip */
												       NULL), /* FIXME:  ought to have an icon */
										      cong_ui_hook_tree_paste_under,
										      doc,
										      parent_window,
										      TRUE,
										      primary_window);
		add_action_to_popup_with_callback_Document_SelectedNode_ParentWindow (UI_PATH_CONTEXT_MENU,
										      cong_action_new ("NodePasteBefore",
												       _("Paste before"),
												       NULL, /* FIXME:  ought to have a tooltip */
												       NULL), /* FIXME:  ought to have an icon */
										      cong_ui_hook_tree_paste_before,
										      doc,
										      parent_window,
										      TRUE,
										      primary_window);
		add_action_to_popup_with_callback_Document_SelectedNode_ParentWindow (UI_PATH_CONTEXT_MENU,
										      cong_action_new ("NodePasteAfter",
												       _("Paste after"),
												       NULL, /* FIXME:  ought to have a tooltip */
												       NULL), /* FIXME:  ought to have an icon */
										      cong_ui_hook_tree_paste_after,
										      doc,
										      parent_window,
										      TRUE,
										      primary_window);
	}
	
	cong_util_add_menu_separator (primary_window,UI_PATH_CONTEXT_MENU);


	/* The "New Sub-element" submenu: */
	{
		GList *list_of_element_desc = cong_document_get_valid_new_child_elements (doc,
											  node, 
											  CONG_ELEMENT_TYPE_STRUCTURAL);
		structural_tag_popup_init (_("New sub-element"),
					   "NewSubelement",
					   cong_ui_hook_tree_new_sub_element,
					   doc,
					   list_of_element_desc,
					   primary_window);
		cong_element_description_list_free (list_of_element_desc);
	}
		
	/* The "New sibling" submenu: */
	{
		GList *list_of_element_desc = cong_document_get_valid_new_next_sibling_elements (doc, 
												 node, 
												 CONG_ELEMENT_TYPE_STRUCTURAL);
		structural_tag_popup_init (_("New sibling"),
					   "NewSibling",
					   cong_ui_hook_tree_new_sibling,
					   doc,
					   list_of_element_desc,
					   primary_window);
		cong_element_description_list_free (list_of_element_desc);
	}

	/* Convert to/from comment: */
	add_comment_menu_actions (doc,
				  node,
				  parent_window,
				  primary_window);

	/* Add any plugin tools for this node: */
	{
		struct add_node_tool_callback_data callback_data;

		callback_data.doc = doc;
		callback_data.node = node;
		callback_data.parent_window = parent_window;

		cong_plugin_manager_for_each_node_tool (cong_app_get_plugin_manager (cong_app_singleton()), 
							add_node_tool_callback,
							&callback_data);
	}

	/* Add the action group to the UI: */
	gtk_ui_manager_insert_action_group (cong_primary_window_get_ui_manager (primary_window), 
					    s_action_group,
					    0);
#if 0	
	g_message (gtk_ui_manager_get_ui (cong_primary_window_get_ui_manager (primary_window)));
#endif
}

void
cong_ui_show_context_menu (CongPrimaryWindow *primary_window,
			   guint button,
			   guint32 activate_time)
{
	gtk_menu_popup (GTK_MENU(gtk_ui_manager_get_widget (cong_primary_window_get_ui_manager (primary_window),
							    "ui/ContextMenu")),
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			button,
			activate_time);
}
