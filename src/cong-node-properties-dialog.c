/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-node-properties-dialog.c
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
#include "cong-dialog.h"
#include "cong-view.h"
#include "cong-document.h"
#include "cong-error-dialog.h"
#include "cong-dispspec.h"
#include "cong-dispspec-element.h"
#include "cong-service-node-property-dialog.h"
#include "cong-plugin-manager.h"
#include "cong-app.h"
#include "cong-eel.h"
#include "cong-attribute-editor.h"
#include "cong-command.h"
#include "cong-util.h"

#define CONG_ADVANCED_NODE_PROPERTIES_VIEW(x) ((CongAdvancedNodePropertiesView*)(x))
typedef struct CongAdvancedNodePropertiesView CongAdvancedNodePropertiesView;

#define DEBUG_PROPERTIES_VIEW 0

struct RawAttr;

static void raw_attr_list_refresh(CongAdvancedNodePropertiesView *view,
				  struct RawAttr* raw_attr);

/* Prototypes of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent);
static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling);
static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling);
static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent, gboolean add_to_end);
static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content);
static void on_document_node_set_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name, const xmlChar *value);
static void on_document_node_remove_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name);
static void on_selection_change(CongView *view);
static void on_cursor_change(CongView *view);

/* The "XPath" view: */
struct XPathView
{
	CongDialogCategory *category;

	GtkLabel *label;
};


/* The "raw xml attributes" view: */
enum {
	RAW_ATTR_MODEL_COLUMN_NAME,
	RAW_ATTR_MODEL_COLUMN_VALUE,
	RAW_ATTR_MODEL_NUM_COLUMNS
};

struct RawAttr
{
	CongDialogCategory *category;
	GtkListStore *list_store;
	GtkTreeView *tree_view;
	GtkWidget *delete_button;
};

struct CongAdvancedNodePropertiesView
{
	CongView view;
	CongNodePtr node;

#if 0
	GtkWidget *notebook;
#endif

	CongDialogContent *dialog_content;

	struct XPathView xpath_view;
	struct RawAttr raw_attr;
};

/* Definitions of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent)
{
	CongAdvancedNodePropertiesView *properties_view;

	g_return_if_fail(view);
	g_return_if_fail(node);

	#if DEBUG_PROPERTIES_VIEW
	g_message("CongAdvancedNodePropertiesView - on_document_node_make_orphan\n");
	#endif

	properties_view = CONG_ADVANCED_NODE_PROPERTIES_VIEW(view);

	/* UNWRITTEN */
}

static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling)
{
	CongAdvancedNodePropertiesView *properties_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(older_sibling);

	#if DEBUG_PROPERTIES_VIEW
	g_message("CongAdvancedNodePropertiesView - on_document_node_add_after\n");
	#endif

	properties_view = CONG_ADVANCED_NODE_PROPERTIES_VIEW(view);

	/* UNWRITTEN */
}

static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling)
{
	CongAdvancedNodePropertiesView *properties_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);

	#if DEBUG_PROPERTIES_VIEW
	g_message("CongAdvancedNodePropertiesView - on_document_node_add_before\n");
	#endif

	properties_view = CONG_ADVANCED_NODE_PROPERTIES_VIEW(view);

	/* UNWRITTEN */
}

static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent, gboolean add_to_end)
{
	CongAdvancedNodePropertiesView *properties_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);

	#if DEBUG_PROPERTIES_VIEW
	g_message("CongAdvancedNodePropertiesView - on_document_node_set_parent\n");
	#endif

	properties_view = CONG_ADVANCED_NODE_PROPERTIES_VIEW(view);

	/* UNWRITTEN */
}

static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content)
{
	CongAdvancedNodePropertiesView *properties_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_PROPERTIES_VIEW
	g_message("CongAdvancedNodePropertiesView - on_document_node_set_text\n");
	#endif

	properties_view = CONG_ADVANCED_NODE_PROPERTIES_VIEW(view);

	/* UNWRITTEN */
}

static void on_document_node_set_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name, const xmlChar *value)
{
	CongAdvancedNodePropertiesView *properties_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(name);
	g_return_if_fail(value);

	#if DEBUG_PROPERTIES_VIEW
	g_message("CongAdvancedNodePropertiesView - on_document_node_set_attribute\n");
	#endif

	properties_view = CONG_ADVANCED_NODE_PROPERTIES_VIEW(view);

	raw_attr_list_refresh(properties_view,
			      &properties_view->raw_attr);
}

static void on_document_node_remove_attribute(CongView *view, gboolean before_event, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name)
{
	CongAdvancedNodePropertiesView *properties_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(name);

	#if DEBUG_PROPERTIES_VIEW
	g_message("CongAdvancedNodePropertiesView - on_document_node_remove_attribute\n");
	#endif

	properties_view = CONG_ADVANCED_NODE_PROPERTIES_VIEW(view);

	raw_attr_list_refresh(properties_view,
			      &properties_view->raw_attr);
}

static void on_selection_change(CongView *view)
{
}

static void on_cursor_change(CongView *view)
{
}

/**
 * init_view_xpath_view:
 * @view:
 * @xpath_view:
 *
 * TODO: Write me
 */
void 
init_view_xpath_view(CongAdvancedNodePropertiesView *view,
		     struct XPathView* xpath_view)
{
	gchar *xpath;

	g_assert(view);
	g_assert(xpath_view);

	xpath_view->category = cong_dialog_content_add_category(view->dialog_content, 
								_("Location"));
	xpath = cong_node_get_path(view->node);

	xpath_view->label = GTK_LABEL(gtk_label_new(xpath));

	gtk_widget_show (GTK_WIDGET (xpath_view->label));

	cong_dialog_category_add_field(xpath_view->category, _("XPath"), GTK_WIDGET(xpath_view->label), FALSE);
	g_free(xpath);
}

static void raw_attr_list_refresh(CongAdvancedNodePropertiesView *view,
				  struct RawAttr* raw_attr)
{
	xmlAttrPtr attr_iter;

	gtk_list_store_clear(raw_attr->list_store);

	for (attr_iter = view->node->properties; attr_iter; attr_iter=attr_iter->next) {
		GtkTreeIter iter;

		char *qualified_name;
		
		qualified_name = 
			cong_util_get_qualified_attribute_name(attr_iter->ns,
							       attr_iter->name);
		

		gtk_list_store_append(raw_attr->list_store,
				      &iter);
		
		gtk_list_store_set(raw_attr->list_store,
				   &iter,
				   RAW_ATTR_MODEL_COLUMN_NAME, qualified_name,
				   RAW_ATTR_MODEL_COLUMN_VALUE, attr_iter->children->content,
				   -1);

		g_free(qualified_name);
	}
}

static gchar* get_attr_name_for_tree_iter(CongAdvancedNodePropertiesView *view, GtkTreeIter *iter)
{
	GValue *value = g_new0(GValue, 1);
	gchar *result = NULL;

	gtk_tree_model_get_value(GTK_TREE_MODEL(view->raw_attr.list_store),
				 iter,
				 RAW_ATTR_MODEL_COLUMN_NAME,
				 value);
	
	result = g_value_dup_string(value);
	g_value_unset(value);

	g_free(value);

	return result;
}

static gchar* get_attr_name_for_tree_path(CongAdvancedNodePropertiesView *view, const gchar *path_string)
{
	GtkTreePath* tree_path;
	GtkTreeIter iter;
	gchar *result = NULL;

	g_assert(view);
	g_assert(path_string);

	tree_path = gtk_tree_path_new_from_string(path_string);

	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(view->raw_attr.list_store),
                                    &iter,
				    tree_path)) {
		result = get_attr_name_for_tree_iter(view, &iter);
	}

	gtk_tree_path_free(tree_path);

	return result;
}

static void on_name_edited(GtkCellRendererText *cellrenderertext,
			   gchar *path,
			   gchar *new_text,
			   gpointer user_data)
{
	/* FIXME: path appears to be path; new_text is value; 
	 * is this really the case? At least in "gtkcellrednderertext.h" */

	CongAdvancedNodePropertiesView *view = user_data;
	gchar *qualified_attr_name = get_attr_name_for_tree_path(view, path);

	const char *local_name;
	xmlNs *ns_ptr = cong_node_get_attr_ns(view->node, 
							qualified_attr_name,
							&local_name);
	const char *new_local_name;
	xmlNs *new_ns_ptr = cong_node_get_attr_ns(view->node, 
							    new_text,
							    &new_local_name);

	gchar *attr_value = cong_node_get_attribute(view->node, 
						    ns_ptr,
						    local_name);

	g_message("on_name_edited");

	/* ##FIXME: Add name check for qualified attribute name  */

	if(new_ns_ptr == NULL && new_local_name != new_text) {
		/* new unknown namespace */
		gchar *msg = g_strdup_printf ("Creation of new namespace for attribute \"%s\"", 
					      new_text);

		/* ##FIXME */
		CONG_DO_UNIMPLEMENTED_DIALOG_WITH_BUGZILLA_ID (NULL, 
							       msg,
							       135858);

		g_free (msg);
		g_free(qualified_attr_name);
		g_free(attr_value);
		
		return ;
	}
	


	/* Ignore if the node already has an attribute of that name: */
	if (!cong_node_has_attribute(view->node, new_ns_ptr, new_local_name)) {
		CongDocument *doc = cong_view_get_document(CONG_VIEW(view));

		gchar *desc = g_strdup_printf(_("Rename attribute \"%s\" as \"%s\""),
					      qualified_attr_name,
					      new_text);

		CongCommand *cmd = cong_document_begin_command (doc,
								desc,
								NULL);

		cong_command_add_node_remove_attribute (cmd,
							view->node,
							ns_ptr,
							local_name);
		cong_command_add_node_set_attribute (cmd,
						     view->node,
						     new_ns_ptr,
						     new_local_name,
						     attr_value);
		cong_document_end_command (doc,
					   cmd);
	}
	
	g_free(qualified_attr_name);
	g_free(attr_value);
}

static void on_value_edited(GtkCellRendererText *cellrenderertext,
			    gchar *path,
			    gchar *new_text,
			    gpointer user_data)
{
	/* FIXME: path appears to be path; new_text is value; 
	 * is this really the case? At least in "gtkcellrednderertext.h" */

	CongAdvancedNodePropertiesView *view = user_data;
	gchar* qualified_name = get_attr_name_for_tree_path(view, path);
	const char *local_name;
	xmlNs *ns_ptr = cong_node_get_attr_ns(view->node, 
							qualified_name,
							&local_name);

	CongDocument *doc = cong_view_get_document(CONG_VIEW(view));

	g_message("on_value_edited %s = %s", qualified_name, new_text);

	{
		gchar *desc = g_strdup_printf(_("Set attribute \"%s\" to \"%s\""),
					      qualified_name,
					      new_text);

		CongCommand *cmd = cong_document_begin_command (doc,
								desc,
								NULL);

		cong_command_add_node_set_attribute (cmd,
						     view->node,
						     ns_ptr,
						     local_name,
						     new_text);
		cong_document_end_command (doc,
					   cmd);
	}

	g_free(qualified_name);
}

static void on_add_attribute(GtkButton *button,
			     gpointer user_data)
{
	int num = 0;
	gchar *attr_name;

	CongAdvancedNodePropertiesView *view = user_data;

	CongDocument *doc = cong_view_get_document(CONG_VIEW(view));

	/* Generate a unique name: */
	while (1) {
		attr_name = g_strdup_printf("attribute%i", num);

		if (cong_node_has_attribute(view->node, NULL, attr_name)) {
			g_free(attr_name);
			num++;
		} else {
			break;
		}
	}

	/* is this _not_ always true ? */
	g_assert(!cong_node_has_attribute(view->node, NULL, attr_name));

	{
		gchar *desc = g_strdup_printf(_("Add attribute \"%s\""),
					      attr_name);

		CongCommand *cmd = cong_document_begin_command (doc,
								desc,
								NULL);

		cong_command_add_node_set_attribute (cmd,
						     view->node,
						     NULL,
						     attr_name,
						     "");
		cong_document_end_command (doc,
					   cmd);
	}

	g_free(attr_name);
}

static void on_delete_attribute(GtkButton *button,
			     gpointer user_data)
{
	CongAdvancedNodePropertiesView *view = user_data;
	GtkTreeSelection* selection = gtk_tree_view_get_selection(view->raw_attr.tree_view);
	GtkTreeIter iter;
	CongDocument *doc = cong_view_get_document(CONG_VIEW(view));

	if (gtk_tree_selection_get_selected (selection,
                                             NULL,
                                             &iter)) {
		char *qualified_name = get_attr_name_for_tree_iter(view, &iter);
		const char *local_name;
		xmlNs *ns_ptr = cong_node_get_attr_ns(view->node, 
								qualified_name,
								&local_name);

		gchar *desc = g_strdup_printf(_("Delete attribute \"%s\""),
					      qualified_name);

		CongCommand *cmd = cong_document_begin_command (doc,
								desc,
								NULL);

		cong_command_add_node_remove_attribute (cmd,
							view->node,
							ns_ptr,
							local_name);
		cong_document_end_command (doc,
					   cmd);

		g_free(qualified_name);
	}
}

static void on_tree_view_selection_change(GtkTreeSelection *treeselection,
					  gpointer user_data)
{
	CongAdvancedNodePropertiesView *view = user_data;

	if (view->raw_attr.delete_button) {
		gtk_widget_set_sensitive(view->raw_attr.delete_button, gtk_tree_selection_get_selected (treeselection, NULL, NULL));
	}
	
}

/**
 * init_view_namespace:
 * @view:
 *
 * TODO: Write me
 */
void 
init_view_namespace (CongAdvancedNodePropertiesView *view)
{
	CongDialogCategory *category;
	GtkWidget *label;
	const gchar *ns_uri;

	g_assert (view);
	g_assert (cong_node_type (view->node)==CONG_NODE_TYPE_ELEMENT);

	ns_uri = cong_node_get_ns_uri (view->node);

	if (ns_uri) {
		label = gtk_label_new (ns_uri);
	} else {
		label = gtk_label_new ("None"); /* FIXME: i18n */
	}

	category = cong_dialog_content_add_category (view->dialog_content, 
						     _("Namespace"));
	gtk_widget_show (label);
	
	cong_dialog_category_add_selflabelled_field (category, 
						     label,
						     TRUE);
}

/**
 * init_view_raw_attr:
 * @view:
 * @raw_attr:
 *
 * TODO: Write me
 */
void 
init_view_raw_attr(CongAdvancedNodePropertiesView *view,
		   struct RawAttr* raw_attr)
{
	/* FIXME: use libglade for this... */
	GtkWidget *vbox = gtk_vbox_new(FALSE, 6);
	GtkWidget *hbox = gtk_hbox_new(FALSE, 6);

	g_assert(view);
	g_assert(raw_attr);
	g_assert(cong_node_type(view->node)==CONG_NODE_TYPE_ELEMENT);

	raw_attr->category = cong_dialog_content_add_category(view->dialog_content, 
							      _("Raw Attributes"));
	
	raw_attr->list_store = GTK_LIST_STORE(gtk_list_store_new(RAW_ATTR_MODEL_NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING));

	/* Initialise store: */
	raw_attr_list_refresh(view, &view->raw_attr);

	/* Initialise treeview */
	{
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *column;

		raw_attr->tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(raw_attr->list_store)));
		
		/* Build columns & renderers: */
		renderer = gtk_cell_renderer_text_new();
		g_object_set(G_OBJECT(renderer),
			     "editable", TRUE,
			     NULL);
		g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(on_name_edited), view);

		column = gtk_tree_view_column_new_with_attributes( _("Name"),
								   renderer,
								   "text", RAW_ATTR_MODEL_COLUMN_NAME,
								   NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (raw_attr->tree_view), column);

		renderer = gtk_cell_renderer_text_new();
		g_object_set(G_OBJECT(renderer),
			     "editable", TRUE,
			     NULL);				      
		g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(on_value_edited), view);

		column = gtk_tree_view_column_new_with_attributes( _("Value"),
								   renderer,
								   "text", RAW_ATTR_MODEL_COLUMN_VALUE,
								   NULL);

		gtk_tree_view_append_column (GTK_TREE_VIEW (raw_attr->tree_view), column);

		g_signal_connect(G_OBJECT(gtk_tree_view_get_selection(raw_attr->tree_view)), "changed", G_CALLBACK(on_tree_view_selection_change), view);

	}

	/* Buttons to add and remove attributes: */
	{
		GtkWidget *add_button = gtk_button_new_from_stock(GTK_STOCK_ADD);
		raw_attr->delete_button = gtk_button_new_from_stock(GTK_STOCK_DELETE);

		gtk_container_add(GTK_CONTAINER(hbox), add_button);
		gtk_container_add(GTK_CONTAINER(hbox), raw_attr->delete_button);

		gtk_widget_set_sensitive(raw_attr->delete_button, FALSE);

		g_signal_connect(G_OBJECT(add_button), "clicked", G_CALLBACK(on_add_attribute), view);
		g_signal_connect(G_OBJECT(raw_attr->delete_button), "clicked", G_CALLBACK(on_delete_attribute), view);

		gtk_widget_show(raw_attr->delete_button);
		gtk_widget_show(add_button);
	}

	gtk_container_add(GTK_CONTAINER(vbox), GTK_WIDGET(raw_attr->tree_view));
	gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	gtk_widget_show(GTK_WIDGET(raw_attr->tree_view));
	gtk_widget_show(hbox);
	gtk_widget_show(vbox);

	cong_dialog_category_add_selflabelled_field(raw_attr->category, vbox, TRUE);
}

/**
 * cong_util_get_tag_string_for_node:
 * @node:
 *
 * TODO: Write me
 */
gchar*
cong_util_get_tag_string_for_node (CongNodePtr node)
{
#if 0
	if (node->xmlns) {
	} else {
	}
#else
	return g_strdup_printf("<%s>", node->name); /* for now */
#endif
}

/**
 * cong_util_get_tag_string_for_node_escaped:
 * @node:
 *
 * TODO: Write me
 */
gchar*
cong_util_get_tag_string_for_node_escaped (CongNodePtr node)
{
	gchar *unescaped = cong_util_get_tag_string_for_node (node);
	gchar *result = g_markup_escape_text (unescaped,
					      strlen (unescaped));
	
	g_free (unescaped);
	
	return result;
}

static void
make_dtd_attribute_editor (CongDocument *doc, 
			   CongNodePtr node,
			   xmlElementPtr xml_element,
			   CongDialogContent *dialog_content)
{
	xmlAttributePtr attr;
	
	CongDialogCategory *category;
	gchar *category_name;
	
	GtkWidget *scrolled_window;
	GtkWidget *vbox_attributes;
	GtkSizeGroup *size_group;								
	
	{
		gchar *tag_name = cong_util_get_tag_string_for_node_escaped (node);
		
		category_name = g_strdup_printf ( _("Attributes for <tt>%s</tt> elements"),
						  tag_name);
		g_free (tag_name);
	}
	
	category = cong_dialog_content_add_category (dialog_content, 
						     category_name);
	g_free (category_name);
	
	
	scrolled_window = gtk_scrolled_window_new (NULL,
						   NULL);
	
	gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW(scrolled_window),
					 GTK_POLICY_NEVER,
					 GTK_POLICY_AUTOMATIC);
	
	
	cong_dialog_category_add_selflabelled_field (category, 
						     scrolled_window,
						     TRUE);
	
	vbox_attributes = gtk_vbox_new (FALSE,
					6);
	
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scrolled_window),
					       vbox_attributes);
	
	size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	
	for (attr=xml_element->attributes; attr; attr=attr->nexth) {
		GtkWidget *hbox;
		GtkWidget *label;
		GtkWidget *attr_editor = cong_attribute_editor_new (doc,
								    node,
								    attr);
		
		gtk_widget_show (attr_editor);
		
		hbox = gtk_hbox_new(FALSE, 6);
		label = gtk_label_new(attr->name);
		gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f);
		gtk_size_group_add_widget(size_group, label);
		gtk_container_add(GTK_CONTAINER(hbox), label);
		gtk_container_add(GTK_CONTAINER(hbox), attr_editor);
		gtk_widget_show (label);
		gtk_widget_show (hbox);
		gtk_box_pack_start (GTK_BOX(vbox_attributes),
				    hbox,
				    FALSE, 
				    TRUE,
				    0);
		
	}
	
	gtk_widget_show (vbox_attributes);
	gtk_widget_show (scrolled_window);
}

enum
{
	CONTENT_COLUMN,
	N_COLUMNS
};

static void
set_node_text (GtkTreeStore *store,
	       GtkTreeIter *iter,
	       const gchar *text)
{
	gtk_tree_store_set (store, 
			    iter,
			    CONTENT_COLUMN, text,
			    -1);
}
	  
static void
set_node_text_for_element_by_name (GtkTreeStore *store,
				   GtkTreeIter *iter,
				   const gchar *name)  /* FIXME: namespace? */
{
	gchar *element_name;
	
	element_name = g_strdup_printf ("<%s>", name);
	
	set_node_text (store,
		       iter,
		       element_name);

	g_free (element_name);
}

static void
populate_recursive (GtkTreeStore *store,
		    GtkTreeIter *iter, 
		    xmlElementContentPtr content);

static void
add_content_for_type (GtkTreeStore *store,
		      GtkTreeIter *parent_iter, 
		      xmlElementContentPtr content)
{
	g_assert (content);

	switch (content->type) {
	default: g_assert_not_reached ();
	case XML_ELEMENT_CONTENT_PCDATA:
		{
			GtkTreeIter iter_new;

			gtk_tree_store_append (store, &iter_new, parent_iter);

			set_node_text (store,
				       &iter_new,
				       _("Text"));
		}
		break;
	case XML_ELEMENT_CONTENT_ELEMENT:
		{
			GtkTreeIter iter_new;

			gtk_tree_store_append (store, &iter_new, parent_iter);

			set_node_text_for_element_by_name (store,
							   &iter_new,
							   content->name);
		}
		break;
	case XML_ELEMENT_CONTENT_SEQ:
		/* Do both c1 and c2 in sequence: */
		populate_recursive (store,
				    parent_iter,
				    content->c1);
		populate_recursive (store,
				    parent_iter,
				    content->c2);
		break;
	case XML_ELEMENT_CONTENT_OR:
		{
			GtkTreeIter iter_wrap;

			/* The naive implementation is to add a choice node, and recurse.
			   But if we have something like ( tag-a | tag-b | tag-c | ... | tag-z) in the DTD, we get a tree of ( choice tag-a (choice tag-b (choice tag-c ( ... choice tag-y tag-z))))
			   which creates a grim-looking tree.
			   
			   So if we are a CONTENT_ONCE and our parent content is CONTENT_OR, then we can merge into the parent choice; it is associative and hence bracketing should make no difference...
			*/
			if (content->ocur==XML_ELEMENT_CONTENT_ONCE) {
				if (content->parent) {
					if (content->parent->type==XML_ELEMENT_CONTENT_OR) {
						/* optimised case */
						populate_recursive (store,
								    parent_iter,
								    content->c1);
						populate_recursive (store,
								    parent_iter,
								    content->c2);						
						break;
					}
				}
			}

			gtk_tree_store_append (store, &iter_wrap, parent_iter);

			set_node_text (store,
				       &iter_wrap,
				       _("Choice"));
			populate_recursive (store,
					    &iter_wrap,
					    content->c1);
			populate_recursive (store,
					    &iter_wrap,
					    content->c2);						
		}
		break;
	}
}

static void
populate_recursive (GtkTreeStore *store,
		    GtkTreeIter *parent_iter, 
		    xmlElementContentPtr content)
{
	g_assert (content);

	switch (content->ocur) {
	default: g_assert_not_reached ();
	case XML_ELEMENT_CONTENT_ONCE:
		add_content_for_type (store,
				      parent_iter,
				      content);
		break;

	case XML_ELEMENT_CONTENT_OPT:
		{
			GtkTreeIter iter_wrap;

			gtk_tree_store_append (store, &iter_wrap, parent_iter);

			set_node_text (store,
				       &iter_wrap,
				       _("Optional"));
			
			add_content_for_type (store,
					      &iter_wrap,
					      content);
		}
		break;

	case XML_ELEMENT_CONTENT_MULT:
		{
			GtkTreeIter iter_wrap;

			gtk_tree_store_append (store, &iter_wrap, parent_iter);

			set_node_text (store,
				       &iter_wrap,
				       _("Zero or more"));
			
			add_content_for_type (store,
					      &iter_wrap,
					      content);
		}
		break;

	case XML_ELEMENT_CONTENT_PLUS:
		{
			GtkTreeIter iter_wrap;

			gtk_tree_store_append (store, &iter_wrap, parent_iter);

			set_node_text (store,
				       &iter_wrap,
				       _("One or more"));
			
			add_content_for_type (store,
					      &iter_wrap,
					      content);
		}
		break;
	}

}

/**
 * make_tree_model_for_element_model:
 * @xml_element:
 *
 * TODO: Write me
 */
GtkTreeModel*
make_tree_model_for_element_model (xmlElementPtr xml_element)
{  
	GtkTreeStore *store;
	GtkTreeIter root_iter;

	g_assert (xml_element);  

	store = gtk_tree_store_new (N_COLUMNS,
				    G_TYPE_STRING);

	gtk_tree_store_append (store, &root_iter, NULL);  /* Acquire a top-level iterator */

	set_node_text_for_element_by_name (store,
					   &root_iter,
					   xml_element->name);  /* FIXME: namespace? */

	if (xml_element->content) {
		populate_recursive (store, 
				    &root_iter, 
				    xml_element->content);
	}

	return GTK_TREE_MODEL (store);
}

/**
 * make_widget_for_element_model:
 * @xml_element:
 *
 * TODO: Write me
 */
GtkWidget*
make_widget_for_element_model (xmlElementPtr xml_element)
{
	GtkWidget *tree;
	GtkTreeModel *tree_model;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	g_assert (xml_element);

	tree_model = make_tree_model_for_element_model (xml_element);
	
	tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (tree_model));
	g_object_unref (G_OBJECT (tree_model));

	renderer = gtk_cell_renderer_text_new ();

	/* To translators: this is the label of a widget that shows the valid combinations of child elements for an element in the document; "content" is a noun */
	column = gtk_tree_view_column_new_with_attributes (_("Valid Content"), renderer,
							   "text", CONTENT_COLUMN,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

	gtk_tree_view_expand_all (GTK_TREE_VIEW (tree));

	return tree;
}

static void
make_dtd_content_model (CongDocument *doc, 
			CongNodePtr node,
			xmlElementPtr xml_element,
			CongDialogContent *dialog_content)
{
	CongDialogCategory *category;
	gchar *category_name;
	gchar *tag_name;
	GtkWidget *scrolled_window;
	GtkWidget *widget;

	tag_name = cong_util_get_tag_string_for_node_escaped (node);

	/* To translators: "content" is a noun; this is XML jargon */
	category_name = g_strdup_printf ( _("Content model for <tt>%s</tt> elements"),
					  tag_name);
	g_free (tag_name);

	category = cong_dialog_content_add_category (dialog_content, 
						     category_name);
	g_free (category_name);

	scrolled_window = gtk_scrolled_window_new (NULL,
						   NULL);
	
	gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW(scrolled_window),
					 GTK_POLICY_NEVER,
					 GTK_POLICY_AUTOMATIC);
	gtk_widget_show (scrolled_window);	

	widget = make_widget_for_element_model (xml_element);
	gtk_widget_show (widget);

	gtk_container_add (GTK_CONTAINER (scrolled_window),
			   widget);

	cong_dialog_category_add_selflabelled_field (category,
						     scrolled_window,
						     TRUE);
}

/**
 * cong_node_properties_dtd_new:
 * @doc:
 * @node:
 * @within_notebook:
 *
 * TODO: Write me
 */
GtkWidget*
cong_node_properties_dtd_new (CongDocument *doc, 
			      CongNodePtr node,
			      gboolean within_notebook)
{	
	if (cong_node_type(node)==CONG_NODE_TYPE_ELEMENT) {
		xmlElementPtr xml_element;
		
		xml_element = cong_document_get_dtd_element (doc,
							     node);
		if (xml_element) {
			CongDialogContent *dialog_content;
			dialog_content = cong_dialog_content_new (within_notebook);

			if (xml_element->attributes!=NULL) {
				make_dtd_attribute_editor (doc,
							   node,
							   xml_element,
							   dialog_content);				
			}

			make_dtd_content_model (doc,
						node,
						xml_element,
						dialog_content);

			return cong_dialog_content_get_widget (dialog_content);
		}
	}

	return NULL;
}

/**
 * cong_node_properties_dialog_advanced_new:
 * @doc:
 * @node:
 * @within_notebook:
 *
 * TODO: Write me
 */
GtkWidget*
cong_node_properties_dialog_advanced_new(CongDocument *doc, 
					 CongNodePtr node,
					 gboolean within_notebook)
{
	CongAdvancedNodePropertiesView *view;
	GtkWidget *widget;

	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);

	view = g_new0(CongAdvancedNodePropertiesView,1);
	view->view.doc = doc;
	view->view.klass = g_new0(CongViewClass,1);
	view->view.klass->on_document_node_make_orphan = on_document_node_make_orphan;
	view->view.klass->on_document_node_add_after = on_document_node_add_after;
	view->view.klass->on_document_node_add_before = on_document_node_add_before;
	view->view.klass->on_document_node_set_parent = on_document_node_set_parent;
	view->view.klass->on_document_node_set_text = on_document_node_set_text;
	view->view.klass->on_document_node_set_attribute = on_document_node_set_attribute;
	view->view.klass->on_document_node_remove_attribute = on_document_node_remove_attribute;
	view->view.klass->on_selection_change = on_selection_change;
	view->view.klass->on_cursor_change = on_cursor_change;

	view->node = node;

	cong_document_register_view( doc, CONG_VIEW(view) );

	view->dialog_content = cong_dialog_content_new(within_notebook);

	init_view_xpath_view(view, &view->xpath_view);

	if (cong_node_type(node)==CONG_NODE_TYPE_ELEMENT) {
		init_view_namespace (view);

		init_view_raw_attr(view, &view->raw_attr);
	}
		
	widget = cong_dialog_content_get_widget(view->dialog_content);

	gtk_widget_show (widget);

	return widget;
}

/**
 * cong_ui_append_advanced_node_properties_page:
 * @notebook:
 * @doc:
 * @node:
 *
 * TODO: Write me
 */
void 
cong_ui_append_advanced_node_properties_page(GtkNotebook *notebook,
					     CongDocument *doc, 
					     CongNodePtr node)
{
	GtkWidget* dtd_page;

	g_return_if_fail(notebook);
	g_return_if_fail(doc);
	g_return_if_fail(node);

	dtd_page = cong_node_properties_dtd_new (doc, 
						 node,
						 TRUE);

	if (dtd_page) {
		gtk_notebook_append_page(notebook,
					 dtd_page,
					 gtk_label_new(_("DTD"))
					 );		
	}

	gtk_notebook_append_page(notebook,
				 cong_node_properties_dialog_advanced_new(doc, 
									  node,
									  TRUE),
				 gtk_label_new(_("Advanced"))
				 );
}

/**
 * cong_node_properties_dialog_new:
 * @doc:
 * @node:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget*
cong_node_properties_dialog_new (CongDocument *doc, 
				 CongNodePtr node, 
				 GtkWindow *parent_window)
{
	g_return_val_if_fail (doc, NULL);
	g_return_val_if_fail (node, NULL);

	/* Should we use a plugin for this node?: */
	if (cong_node_type(node)==CONG_NODE_TYPE_ELEMENT) {
		CongDispspecElement *element;
		const gchar *service_id;

		element = cong_document_get_dispspec_element_for_node (doc,
								       node);
		if (element) {
			service_id = cong_dispspec_element_get_property_dialog_service_id(element);

			/* Is there a plugin for this type of node? */
			if (service_id) {
				CongServiceNodePropertyDialog *dialog_factory = cong_plugin_manager_locate_custom_property_dialog_by_id (cong_app_get_plugin_manager (cong_app_singleton()), 
																	 service_id);

				if (dialog_factory) {
					GtkWidget *dialog = cong_custom_property_dialog_make (dialog_factory, 
											      doc, 
											      node);
					return dialog;
				}
			}
		}
	}


	/* Otherwise: */
	{
		GtkWidget* dtd_page;

		GtkWidget *dialog, *vbox;
		GtkWidget *advanced_properties;

		dialog = gtk_dialog_new_with_buttons(_("Properties"),
						     parent_window,
						     GTK_DIALOG_MODAL,
						     GTK_STOCK_OK, GTK_RESPONSE_OK,
						     NULL);		

		gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);

		vbox = GTK_DIALOG(dialog)->vbox;

		dtd_page = cong_node_properties_dtd_new (doc, 
							 node,
							 TRUE);

		advanced_properties = cong_node_properties_dialog_advanced_new (doc, 
										node,
										(dtd_page!=NULL));
			
		gtk_widget_show (advanced_properties);

		if (dtd_page) {
			GtkWidget *notebook = gtk_notebook_new ();

			gtk_widget_show (notebook);

			gtk_box_pack_start (GTK_BOX(vbox), 
					    notebook, 
					    TRUE, 
					    TRUE, 
					    0);

			gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
						 dtd_page,
						 gtk_label_new(_("DTD"))
						 );		

			gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
						 advanced_properties,
						 gtk_label_new(_("Advanced"))
						 );
		} else {
			gtk_box_pack_start (GTK_BOX(vbox), 
					    advanced_properties, 
					    TRUE, 
					    TRUE, 
					    0);
		}

		g_signal_connect_swapped (G_OBJECT (dialog), 
					  "response", G_CALLBACK (gtk_widget_destroy),
					  GTK_OBJECT (dialog));


		return GTK_WIDGET(dialog);
	}
}
