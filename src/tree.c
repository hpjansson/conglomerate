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
#include "cong-util.h"
#include "cong-command.h"

/* the popup items have the data "popup_data_item" set on them: */

gint tree_new_sibling(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;

	CongNodePtr new_node;
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

	{
		gchar *desc = g_strdup_printf (_("Insert sibling: %s"), cong_dispspec_element_username (element));
		CongCommand *cmd = cong_document_begin_command (doc, desc, NULL);
		g_free (desc);

		/* New element */
		new_node = cong_node_new_element_from_dispspec(element, doc);
		cong_command_add_node_add_after(cmd, new_node, tag);

		/*  add any necessary sub elements it needs */
		cong_command_add_xml_add_required_children (cmd, 
							    new_node);
		
		cong_command_add_set_cursor_to_first_text_descendant (cmd, 
								      new_node);

		cong_document_end_command (doc, cmd);		
	}

	cong_document_end_edit(doc);



	return(TRUE);
}


gint tree_new_sub_element(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;

	CongNodePtr new_node;
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

	{
		gchar *desc = g_strdup_printf (_("Insert child: %s"), cong_dispspec_element_username (element));
		CongCommand *cmd = cong_document_begin_command (doc, desc, NULL);
		g_free (desc);

		/* New element */
		new_node = cong_node_new_element_from_dispspec(element, doc);
		cong_command_add_node_set_parent (cmd, new_node, tag);

		/*  add any necessary sub elements it needs */
		cong_command_add_xml_add_required_children (cmd, 
							    new_node);
		
		cong_command_add_set_cursor_to_first_text_descendant (cmd, 
								      new_node);

		cong_document_end_command (doc, cmd);
	}

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
	gtk_widget_show (properties_dialog);
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
	gchar *source;

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	/* GREP FOR MVC */
	source = cong_node_generate_source(tag);

	/* FIXME: set clipboard state within command? */
	cong_app_set_clipboard (cong_app_singleton(),
				source);
	g_free(source);

	cong_document_begin_edit(doc);

	{
		CongCommand *cmd = cong_document_begin_command (doc, _("Cut"), NULL);

		cong_command_for_each_location (cmd, 
						tree_cut_update_location_callback,
						tag);
		
		cong_command_add_node_recursive_delete(cmd, tag);

		cong_document_end_command (doc, cmd);
	}

	cong_document_end_edit(doc);

	return(TRUE);
}


gint tree_copy(GtkWidget *widget, CongNodePtr tag)
{
	/* GREP FOR MVC */

#if 1
	gchar *source;

	source = cong_node_generate_source(tag);
	cong_app_set_clipboard (cong_app_singleton(),
				cong_node_generate_source(tag));	
	g_free(source);
#else
	if (cong_app_singleton()->clipboard) cong_document_node_recursive_delete(NULL, cong_app_singleton()->clipboard);
	cong_app_singleton()->clipboard = cong_node_recursive_dup(tag);
#endif

	return(TRUE);
}

gint tree_paste_under(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;
	CongDispspec *ds;
	const gchar *clipboard_source;

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	ds = cong_document_get_dispspec(doc);

	clipboard_source = cong_app_get_clipboard (cong_app_singleton());
						   
	if (clipboard_source) {
		cong_document_paste_source_under (doc,
						  tag,
						  clipboard_source);
	}

	return(TRUE);
}


gint tree_paste_before(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;
	CongDispspec *ds;
	const gchar *clipboard_source;

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	ds = cong_document_get_dispspec(doc);

	clipboard_source = cong_app_get_clipboard (cong_app_singleton());
						   
	if (clipboard_source) {
		cong_document_paste_source_before (doc,
						   tag,
						   clipboard_source);
	}
	
	return(TRUE);
}


gint tree_paste_after(GtkWidget *widget, CongNodePtr tag)
{
	CongDocument *doc;
	CongDispspec *ds;
	const gchar *clipboard_source;

	doc = g_object_get_data(G_OBJECT(widget),"document");
	g_assert(doc);

	ds = cong_document_get_dispspec(doc);

	clipboard_source = cong_app_get_clipboard (cong_app_singleton());
						   
	if (clipboard_source) {
		cong_document_paste_source_after (doc,
						  tag,
						  clipboard_source);
	}
	
	return(TRUE);
}

