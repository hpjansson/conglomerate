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
#include "cong-plugin.h"
#include "cong-app.h"

#define CONG_ADVANCED_NODE_PROPERTIES_VIEW(x) ((CongAdvancedNodePropertiesView*)(x))
typedef struct CongAdvancedNodePropertiesView CongAdvancedNodePropertiesView;

#define DEBUG_PROPERTIES_VIEW 0
#define DEBUG_DTD_PROPERTIES_HACK 0

struct RawAttr;

static void raw_attr_list_refresh(CongAdvancedNodePropertiesView *view,
				  struct RawAttr* raw_attr);

/* Prototypes of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent);
static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling);
static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling);
static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content);
static void on_document_node_set_attribute(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *name, const xmlChar *value);
static void on_document_node_remove_attribute(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *name);
static void on_selection_change(CongView *view);
static void on_cursor_change(CongView *view);

/* The "XPath" view: */
struct XPathView
{
	CongDialogCategory *category;

	GtkLabel *label;
};

/* The "modelled attributes" view: */
struct ModelledAttr
{
	CongDialogCategory *category;
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
	struct ModelledAttr modelled_attr;
	struct RawAttr raw_attr;
};

/* Definitions of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent)
{
	CongAdvancedNodePropertiesView *properties_view;
	GtkTreeIter tree_iter;

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
	GtkTreeIter tree_iter_sibling;
	GtkTreeIter tree_iter_parent;

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
	GtkTreeIter tree_iter_sibling;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);

	#if DEBUG_PROPERTIES_VIEW
	g_message("CongAdvancedNodePropertiesView - on_document_node_add_before\n");
	#endif

	properties_view = CONG_ADVANCED_NODE_PROPERTIES_VIEW(view);

	/* UNWRITTEN */
}

static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent)
{
	CongAdvancedNodePropertiesView *properties_view;
	GtkTreeIter tree_iter_node;
	GtkTreeIter tree_iter_parent;

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
	GtkTreeIter tree_iter;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_PROPERTIES_VIEW
	g_message("CongAdvancedNodePropertiesView - on_document_node_set_text\n");
	#endif

	properties_view = CONG_ADVANCED_NODE_PROPERTIES_VIEW(view);

	/* UNWRITTEN */
}

static void on_document_node_set_attribute(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *name, const xmlChar *value)
{
	CongAdvancedNodePropertiesView *properties_view;
	GtkTreeIter tree_iter;

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

static void on_document_node_remove_attribute(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *name)
{
	CongAdvancedNodePropertiesView *properties_view;
	GtkTreeIter tree_iter;

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


void init_view_xpath_view(CongAdvancedNodePropertiesView *view,
			  struct XPathView* xpath_view)
{
	gchar *xpath;

	g_assert(view);
	g_assert(xpath_view);

	xpath_view->category = cong_dialog_content_add_category(view->dialog_content, 
								_("Location"));
	xpath = cong_node_get_path(view->node);

	xpath_view->label = GTK_LABEL(gtk_label_new(xpath));

	cong_dialog_category_add_field(xpath_view->category, _("XPath"), GTK_WIDGET(xpath_view->label));
	g_free(xpath);
}

void init_view_modelled_attr(CongAdvancedNodePropertiesView *view,
			     struct ModelledAttr *modelled_attr)
{
	xmlElementPtr xml_element;

	g_assert(view);
	g_assert(modelled_attr);
	g_assert(cong_node_type(view->node)==CONG_NODE_TYPE_ELEMENT);

	xml_element = xml_get_dtd_element(cong_view_get_document(CONG_VIEW(view)), view->node);

	if (xml_element) {
		xmlAttributePtr attr;
		modelled_attr->category = cong_dialog_content_add_category(view->dialog_content, 
									   _("Properties from DTD/schema"));

		for (attr=xml_element->attributes; attr; attr=attr->nexth) {
			switch (attr->atype) {
			default: g_assert_not_reached();
			case XML_ATTRIBUTE_CDATA:
				cong_dialog_category_add_field(modelled_attr->category, attr->name, gtk_label_new("CDATA"));
				break;
				
			case XML_ATTRIBUTE_ID:
				cong_dialog_category_add_field(modelled_attr->category, attr->name, gtk_label_new("ID"));
				break;
				
			case XML_ATTRIBUTE_IDREF:
				cong_dialog_category_add_field(modelled_attr->category, attr->name, gtk_label_new("IDREF"));
				break;
				
			case XML_ATTRIBUTE_IDREFS:
				cong_dialog_category_add_field(modelled_attr->category, attr->name, gtk_label_new("IDREFS"));
				break;

			case XML_ATTRIBUTE_ENTITY:
				cong_dialog_category_add_field(modelled_attr->category, attr->name, gtk_label_new("ENTITY"));
				break;
				
			case XML_ATTRIBUTE_ENTITIES:
				cong_dialog_category_add_field(modelled_attr->category, attr->name, gtk_label_new("ENTITIES"));
				break;
				
			case XML_ATTRIBUTE_NMTOKEN:
				cong_dialog_category_add_field(modelled_attr->category, attr->name, gtk_label_new("NMTOKEN"));
				break;
				
			case XML_ATTRIBUTE_NMTOKENS:
				cong_dialog_category_add_field(modelled_attr->category, attr->name, gtk_label_new("NMTOKENS"));
				break;
				
			case XML_ATTRIBUTE_ENUMERATION:
				{
					GtkWidget *option_menu = gtk_option_menu_new();
					GtkWidget *menu = gtk_menu_new();
					xmlEnumerationPtr enum_ptr;

					for (enum_ptr=attr->tree; enum_ptr; enum_ptr=enum_ptr->next) {
						gtk_menu_shell_append(GTK_MENU_SHELL(menu), 
								      gtk_menu_item_new_with_label(enum_ptr->name));
					}

					gtk_option_menu_set_menu(GTK_OPTION_MENU(option_menu), 
								 menu);

					cong_dialog_category_add_field(modelled_attr->category, 
								       attr->name, 
								       option_menu);
				}
				break;

			case XML_ATTRIBUTE_NOTATION:
				cong_dialog_category_add_field(modelled_attr->category, attr->name, gtk_label_new("NOTATION"));
				break;
			}
		}
	}

}

static void raw_attr_list_refresh(CongAdvancedNodePropertiesView *view,
				  struct RawAttr* raw_attr)
{
	xmlAttrPtr attr_iter;

	gtk_list_store_clear(raw_attr->list_store);

	for (attr_iter = view->node->properties; attr_iter; attr_iter=attr_iter->next) {
		GtkTreeIter iter;
		gtk_list_store_append(raw_attr->list_store,
				      &iter);
		
		gtk_list_store_set(raw_attr->list_store,
				   &iter,
				   RAW_ATTR_MODEL_COLUMN_NAME, attr_iter->name,
				   RAW_ATTR_MODEL_COLUMN_VALUE, attr_iter->children->content,
				   -1);
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
			   gchar *arg1,
			   gchar *arg2,
			   gpointer user_data)
{
	/* FIXME: arg1 appears to be path; arg2 is value; is this really the case? */

	CongAdvancedNodePropertiesView *view = user_data;
	gchar* attr_name = get_attr_name_for_tree_path(view, arg1);
	gchar *attr_value = cong_node_get_attribute(view->node, attr_name);

	g_message("on_name_edited");

	/* Ignore if the node already has an attribute of that name: */
	if (!xmlHasProp(view->node, arg2)) {
		/* Remove old attribute: */
		cong_document_node_remove_attribute(cong_view_get_document(CONG_VIEW(view)), view->node, attr_name);
	
		/* Add new attribute: */
		cong_document_node_set_attribute(cong_view_get_document(CONG_VIEW(view)), view->node, arg2, attr_value);
	}
	
	g_free(attr_name);
	g_free(attr_value);
}

static void on_value_edited(GtkCellRendererText *cellrenderertext,
			    gchar *arg1,
			    gchar *arg2,
			    gpointer user_data)
{
	/* FIXME: arg1 appears to be path; arg2 is value; is this really the case? */

	CongAdvancedNodePropertiesView *view = user_data;
	gchar* attr_name = get_attr_name_for_tree_path(view, arg1);

	g_message("on_value_edited %s = %s", attr_name, arg2);

	cong_document_node_set_attribute(cong_view_get_document(CONG_VIEW(view)), view->node, attr_name, arg2);

	g_free(attr_name);
}

static void on_add_attribute(GtkButton *button,
			     gpointer user_data)
{
	int num = 0;
	gchar *attr_name;

	CongAdvancedNodePropertiesView *view = user_data;

	/* Generate a unique name: */
	while (1) {
		attr_name = g_strdup_printf("attribute%i", num);

		if (xmlHasProp(view->node, attr_name)) {
			g_free(attr_name);
			num++;
		} else {
			break;
		}
	}

	g_assert(!xmlHasProp(view->node, attr_name));

	cong_document_node_set_attribute(cong_view_get_document(CONG_VIEW(view)), view->node, attr_name, "");
}

static void on_delete_attribute(GtkButton *button,
			     gpointer user_data)
{
	CongAdvancedNodePropertiesView *view = user_data;
	GtkTreeSelection* selection = gtk_tree_view_get_selection(view->raw_attr.tree_view);
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected (selection,
                                             NULL,
                                             &iter)) {
		gchar* attr_name = get_attr_name_for_tree_iter(view, &iter);

		cong_document_node_remove_attribute(cong_view_get_document(CONG_VIEW(view)), view->node, attr_name);

		g_free(attr_name);
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

void init_view_raw_attr(CongAdvancedNodePropertiesView *view,
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
	}

	gtk_container_add(GTK_CONTAINER(vbox), GTK_WIDGET(raw_attr->tree_view));
	gtk_container_add(GTK_CONTAINER(vbox), hbox);

	cong_dialog_category_add_selflabelled_field(raw_attr->category, vbox);
}

GtkWidget *cong_node_properties_dialog_advanced_new(CongDocument *doc, 
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
#if DEBUG_DTD_PROPERTIES_HACK
		init_view_modelled_attr(view, &view->modelled_attr);
#endif /* #if DEBUG_DTD_PROPERTIES_HACK */
		
		init_view_raw_attr(view, &view->raw_attr);
	}
		
	widget = cong_dialog_content_get_widget(view->dialog_content);

	gtk_widget_show_all (widget);

	return widget;
}

void cong_ui_append_advanced_node_properties_page(GtkNotebook *notebook,
						  CongDocument *doc, 
						  CongNodePtr node)
{
	g_return_if_fail(notebook);
	g_return_if_fail(doc);
	g_return_if_fail(node);

	gtk_notebook_append_page(notebook,
				 cong_node_properties_dialog_advanced_new(doc, 
									  node,
									  TRUE),
				 gtk_label_new(_("Advanced"))
				 );
}

GtkWidget *cong_node_properties_dialog_new(CongDocument *doc, 
					   CongNodePtr node, 
					   GtkWindow *parent_window)
{
	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);

	/* Should we use a plugin for this node?: */
	if (cong_node_type(node)==CONG_NODE_TYPE_ELEMENT) {
		CongDispspec *ds;
		CongDispspecElement* element;
		const gchar* plugin_id;

		ds = cong_document_get_dispspec(doc);
		g_assert(ds);

		element = cong_dispspec_lookup_node(ds, node);
		if (element) {
			plugin_id = cong_dispspec_element_get_property_dialog_plugin_id(element);

			/* Is there a plugin for this type of node? */
			if (plugin_id) {
				CongCustomPropertyDialog *dialog_factory = cong_plugin_manager_locate_custom_property_dialog_by_id(cong_app_singleton()->plugin_manager, plugin_id);

				if (dialog_factory) {
					GtkWidget *dialog = cong_custom_property_dialog_make(dialog_factory, doc, node);

					return dialog;
				}
			}
		}
	}


	/* Otherwise: */
	{
		GtkWidget *dialog, *vbox;
		GtkWidget *advanced_properties;

		dialog = gtk_dialog_new_with_buttons(_("Properties"),
						     parent_window,
						     GTK_DIALOG_MODAL,
						     GTK_STOCK_OK, GTK_RESPONSE_OK,
						     NULL);		

		gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);

		vbox = GTK_DIALOG(dialog)->vbox;
		advanced_properties = cong_node_properties_dialog_advanced_new(doc, 
									       node,
									       FALSE);
		gtk_box_pack_start(GTK_BOX(vbox), advanced_properties, FALSE, FALSE, 0);

		g_signal_connect_swapped (G_OBJECT (dialog), 
					  "response", 
					  G_CALLBACK (gtk_widget_destroy),
					  GTK_OBJECT (dialog));

		return GTK_WIDGET(dialog);
	}
}

