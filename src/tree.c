/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include "global.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-dialog.h"
#include "cong-view.h"
#include "cong-app.h"

/* the popup items have the data "popup_data_item" set on them: */

gint tree_new_sibling(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;

	CongNodePtr text_node, new_node;
#if 1
	CongDispspecElement *element = g_object_get_data(G_OBJECT(widget),
							 "element");
#else
	char *label;

	label = g_object_get_data(G_OBJECT(widget),
				  "label");
#endif

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);
	
	
	/* GREP FOR MVC */
	cong_document_begin_edit(doc);

	/* Text node before new element */
	text_node = cong_node_new_text(" ", doc);
	cong_document_node_add_after(doc, text_node, tag);

	/* New element */
	new_node = cong_node_new_element_from_dispspec(element, doc);
	cong_document_node_add_after(doc, new_node, text_node);

	/*  add any necessary sub elements it needs */
	xml_add_required_children(doc, new_node);

	/* Text node after new element */
	text_node = cong_node_new_text(" ", doc);
	cong_document_node_add_after(doc, text_node, new_node);

	cong_document_end_edit(doc);


	return(TRUE);
}


gint tree_new_sub_element(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;

	CongNodePtr text_node, new_node;
#if 1
	CongDispspecElement *element = g_object_get_data(G_OBJECT(widget),
							 "element");
#else
	char *label;

	label = g_object_get_data(G_OBJECT(widget),
				  "label");
#endif

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	/* GREP FOR MVC */
	cong_document_begin_edit(doc);

	/* Text node before new element */
	text_node = cong_node_new_text(" ", doc);
	cong_document_node_set_parent(doc, text_node, tag);

	/* New element */
	new_node = cong_node_new_element_from_dispspec(element, doc);
	cong_document_node_set_parent(doc, new_node, tag);

	/*  add any necessary sub elements it needs */
	xml_add_required_children(doc, new_node);

	/* Text node after new element */
	text_node = cong_node_new_text(" ", doc);
	cong_document_node_set_parent(doc, text_node, tag);

	cong_document_end_edit(doc);

	return(TRUE);
}

gint tree_properties(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;
	GtkWidget *properties_dialog;
	GtkWindow *parent_window;

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	parent_window = g_object_get_data(G_OBJECT(widget),
					  "parent_window");

	properties_dialog = cong_node_properties_dialog_new(doc, tag, parent_window);

#if 1
	gtk_widget_show_all(properties_dialog);
#else
	/* FIXME:  Make this modeless */
	gtk_dialog_run(GTK_DIALOG(properties_dialog));
	gtk_widget_destroy(properties_dialog);
#endif

	return TRUE;
}


static gboolean
tree_cut_update_location_callback (CongDocument *doc,
				   CongLocation *location, 
				   gpointer user_data)
{
	CongNodePtr node = user_data;

	if (location->node) {
		if (cong_node_is_descendant_of (location->node,
						node) ) {
			cong_location_nullify(location);
			return TRUE;
		}
	}

	return FALSE;
}


gint tree_cut(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;
	CongDispspec *ds;

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	ds = cong_document_get_dispspec(doc);

	/* GREP FOR MVC */

	if (cong_app_singleton()->clipboard) cong_document_node_recursive_delete(NULL, cong_app_singleton()->clipboard);

	cong_app_singleton()->clipboard = cong_node_recursive_dup(tag);
	CONG_NODE_SELF_TEST(cong_app_singleton()->clipboard);

	cong_document_begin_edit(doc);

	cong_document_for_each_location (doc, 
					 tree_cut_update_location_callback,
					 tag);

	cong_document_node_recursive_delete(doc, tag);
	cong_document_end_edit(doc);

	return(TRUE);
}


gint tree_copy(GtkWidget *widget, CongNodePtr tag)
{
	/* GREP FOR MVC */

	if (cong_app_singleton()->clipboard) cong_document_node_recursive_delete(NULL, cong_app_singleton()->clipboard);
	cong_app_singleton()->clipboard = cong_node_recursive_dup(tag);
	CONG_NODE_SELF_TEST(cong_app_singleton()->clipboard);

	return(TRUE);
}


gint tree_paste_under(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;
	CongDispspec *ds;

	CongNodePtr new_copy;

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	ds = cong_document_get_dispspec(doc);

	if (!cong_app_singleton()->clipboard) return(TRUE);
	if (!cong_dispspec_element_structural(ds, cong_node_xmlns(tag), xml_frag_name_nice(tag))) return(TRUE);
	if (!cong_dispspec_element_structural(ds, cong_node_xmlns(cong_app_singleton()->clipboard), xml_frag_name_nice(cong_app_singleton()->clipboard))) return(TRUE);

	CONG_NODE_SELF_TEST(cong_app_singleton()->clipboard);

	new_copy = cong_node_recursive_dup(cong_app_singleton()->clipboard);

	CONG_NODE_SELF_TEST(new_copy);

	cong_document_begin_edit(doc);
	cong_document_node_set_parent(doc, new_copy,tag);
	cong_document_end_edit(doc);

	return(TRUE);
}


gint tree_paste_before(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;
	CongDispspec *ds;

	CongNodePtr new_copy;

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	ds = cong_document_get_dispspec(doc);
	if (!cong_app_singleton()->clipboard) return(TRUE);
	if (!cong_dispspec_element_structural(ds, cong_node_xmlns(tag), xml_frag_name_nice(tag))) return(TRUE);
	if (!cong_dispspec_element_structural(ds, cong_node_xmlns(cong_app_singleton()->clipboard), xml_frag_name_nice(cong_app_singleton()->clipboard))) return(TRUE);

	CONG_NODE_SELF_TEST(cong_app_singleton()->clipboard);

	new_copy = cong_node_recursive_dup(cong_app_singleton()->clipboard);

	CONG_NODE_SELF_TEST(new_copy);

	cong_document_begin_edit(doc);
	cong_document_node_add_before(doc, new_copy,tag);
	cong_document_end_edit(doc);
	
	return(TRUE);
}


gint tree_paste_after(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;
	CongDispspec *ds;

	CongNodePtr new_copy;

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	ds = cong_document_get_dispspec(doc);

	if (!cong_app_singleton()->clipboard) return(TRUE);
	if (!cong_dispspec_element_structural(ds, cong_node_xmlns(tag), xml_frag_name_nice(tag))) return(TRUE);
	if (!cong_dispspec_element_structural(ds, cong_node_xmlns(cong_app_singleton()->clipboard), xml_frag_name_nice(cong_app_singleton()->clipboard))) return(TRUE);

	CONG_NODE_SELF_TEST(cong_app_singleton()->clipboard);

	new_copy = cong_node_recursive_dup(cong_app_singleton()->clipboard);

	CONG_NODE_SELF_TEST(new_copy);

	cong_document_begin_edit(doc);
	cong_document_node_add_after(doc, new_copy,tag);
	cong_document_end_edit(doc);
	
	return(TRUE);
}

