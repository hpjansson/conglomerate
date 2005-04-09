/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include "global.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-dispspec-element.h"
#include "cong-dialog.h"
#include "cong-view.h"
#include "cong-app.h"
#include "cong-util.h"
#include "cong-command.h"
#include "cong-ui-hooks.h"
#include "cong-glade.h"

static CongNodePtr
get_element_name(CongDocument* doc)
{
	GladeXML* xml;
	GtkWidget* dialog;
	CongNodePtr new_node;

	xml = cong_util_load_glade_file (
		"conglomerate/glade/custom-element.glade",
		NULL,
		doc,
		NULL);

	dialog = glade_xml_get_widget (xml,
			"custom-element-dialog");

	if(gtk_dialog_run (GTK_DIALOG (dialog))==GTK_RESPONSE_OK) 
	{
		GtkWidget* element_name;
		GtkWidget* element_type_span;
		GtkWidget* element_type_structural;

		element_name = glade_xml_get_widget(xml, "element-name");
		element_type_span = glade_xml_get_widget(xml,
				"element-type-span");
		element_type_structural = glade_xml_get_widget(xml,
				"element-type-structural");

		g_assert(element_name);
		g_assert(element_type_structural);
		g_assert(element_type_span);

		new_node = cong_node_new_element(NULL,
			gtk_entry_get_text(GTK_ENTRY(element_name)), doc);
	}
	else
	{
		new_node = NULL;
	}

	gtk_widget_destroy (dialog);

	g_object_unref (G_OBJECT (xml));

	return new_node;
}

/* the popup items have the data "popup_data_item" set on them: */

/**
 * cong_ui_hook_tree_new_sibling:
 * @doc:
 * @element_desc:
 *
 * TODO: Write me
 */
void 
cong_ui_hook_tree_new_sibling (CongDocument *doc,
			       CongElementDescription *element_desc)
{
	CongNodePtr node;
	CongNodePtr new_node;
	CongDispspec *ds;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	node = cong_document_get_selected_node (doc);

	g_return_if_fail (node);

	ds = cong_document_get_default_dispspec(doc);

	/* GREP FOR MVC */
	cong_document_begin_edit(doc);

	{
		if(element_desc)
		{
			gchar *username;
			gchar *desc;
			CongCommand *cmd;

			username = cong_element_description_make_user_name (element_desc,
									    ds);

			desc = g_strdup_printf (_("Insert sibling: %s"), username);
			cmd = cong_document_begin_command (doc, desc, NULL);
			g_free (desc);

			/* New element */
			new_node = cong_element_description_make_node (element_desc, 
								       doc,
								       node->parent);
			cong_command_add_node_add_after (cmd, 
							 new_node, 
							 node);

			/*  add any necessary sub elements it needs */
			if (cong_command_add_required_sub_elements (cmd,new_node)) {
			
				cong_command_add_set_cursor_to_first_text_descendant (cmd, 
										      new_node);

				cong_document_end_command (doc, cmd);		
			} else {

				cong_document_abort_command (doc, cmd);

			}

			g_free (username);
		}
		else
		{
			new_node = get_element_name(doc);
			if(new_node!=NULL)
			{
				gchar *desc = g_strdup_printf (_("Insert sibling: %s"), new_node->name);
				CongCommand *cmd = cong_document_begin_command (doc, desc, NULL);
				g_free (desc);

				cong_command_add_node_add_after(cmd, new_node, node);
				cong_document_end_command (doc, cmd);
			}
		}
	}

	cong_document_end_edit(doc);

	
}

/**
 * cong_ui_hook_tree_new_sub_element:
 * @doc:
 * @element_desc:
 *
 * TODO: Write me
 */
void
cong_ui_hook_tree_new_sub_element (CongDocument *doc,
				   CongElementDescription *element_desc)
{
	CongNodePtr node;
	CongNodePtr new_node;
	CongDispspec *ds;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	node = cong_document_get_selected_node (doc);

	g_return_if_fail (node);

	ds = cong_document_get_default_dispspec(doc);

	/* GREP FOR MVC */
	cong_document_begin_edit(doc);

	{

		/* New element */
		if(element_desc)
		{
			gchar *username;
			gchar *desc;
			CongCommand *cmd;

			username = cong_element_description_make_user_name (element_desc,
									    ds);
			desc = g_strdup_printf (_("Insert child: %s"), username);
			cmd = cong_document_begin_command (doc, desc, NULL);
			g_free (desc);
			new_node = cong_element_description_make_node (
				element_desc, 
				doc,
				node);
			cong_command_add_node_set_parent (cmd, 
							  new_node, 
							  node);
			/*  add any necessary sub elements it needs */
			if (cong_command_add_required_sub_elements (cmd,new_node)) {
			
				cong_command_add_set_cursor_to_first_text_descendant (cmd, 
										      new_node);
				cong_document_end_command (doc, cmd);		

			} else {
				cong_document_abort_command (doc, cmd);
			}
			
			g_free (username);
		}
		else
		{
			new_node = get_element_name(doc);
			if(new_node!=NULL)
			{
				gchar *desc = g_strdup_printf (_("Insert child: %s"), new_node->name);
				CongCommand *cmd = cong_document_begin_command (doc, desc, NULL);
				g_free (desc);

				cong_command_add_node_set_parent (cmd, new_node, node);
				cong_document_end_command (doc, cmd);
			}
		}
	}

	cong_document_end_edit(doc);
}

/**
 * cong_ui_hook_tree_properties:
 * @doc:
 * @parent_window:
 *
 * TODO: Write me
 */
void
cong_ui_hook_tree_properties (CongDocument *doc,
			      GtkWindow *parent_window)
{
	GtkWidget *properties_dialog;
	CongNodePtr node;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	node = cong_document_get_selected_node (doc);
	g_return_if_fail (node);

	properties_dialog = cong_node_properties_dialog_new (doc, 
							     node, 
							     parent_window);
	
#if 1
	gtk_widget_show (properties_dialog);
#else
	/* FIXME:  Make this modeless */
	gtk_dialog_run(GTK_DIALOG(properties_dialog));
	gtk_widget_destroy(properties_dialog);
#endif
}


static gboolean
tree_cut_update_location_callback (CongDocument *doc,
				   CongLocation *location, 
				   gpointer user_data)
{
	CongNodePtr node = user_data;

	if (location->node) {
		if (location->node == node || cong_node_is_descendant_of (location->node,
									  node) ) {
			cong_location_nullify(location);
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * cong_ui_hook_tree_cut:
 * @doc:
 * @parent_window:
 *
 * TODO: Write me
 */
void 
cong_ui_hook_tree_cut (CongDocument *doc,
		       GtkWindow *parent_window)
{
	gchar *source;
	CongNodePtr node;
		
	g_return_if_fail (IS_CONG_DOCUMENT (doc));
	
	node = cong_document_get_selected_node (doc);
	g_return_if_fail (node);

	source = cong_node_generate_source(node);
	
	/* FIXME: set clipboard state within command? */
	cong_app_set_clipboard_from_xml_fragment (cong_app_singleton(),
						  GDK_SELECTION_CLIPBOARD,
						  source,
						  doc);
	g_free(source);
	
	cong_document_begin_edit(doc);
	
	{
		CongCommand *cmd = cong_document_begin_command (doc, _("Cut"), NULL);
		
		cong_command_for_each_location (cmd, 
						tree_cut_update_location_callback,
						node);
		
		cong_command_add_node_recursive_delete(cmd, node);
		
		cong_document_end_command (doc, cmd);
	}
	
	cong_document_end_edit(doc);
}

/**
 * cong_ui_hook_tree_copy:
 * @doc:
 * @parent_window:
 *
 * TODO: Write me
 */
void 
cong_ui_hook_tree_copy (CongDocument *doc,
			GtkWindow *parent_window)
{
	gchar *source;
	CongNodePtr node;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	node = cong_document_get_selected_node (doc);
	g_return_if_fail (node);

	/* GREP FOR MVC */

	source = cong_node_generate_source(node);
	cong_app_set_clipboard_from_xml_fragment (cong_app_singleton(),
						  GDK_SELECTION_CLIPBOARD,
						  source,
						  doc);	
	g_free(source);
}

/**
 * cong_ui_hook_tree_paste_under:
 * @doc:
 * @parent_window:
 *
 * TODO: Write me
 */
void 
cong_ui_hook_tree_paste_under (CongDocument *doc,
			       GtkWindow *parent_window)
{
	CongDispspec *ds;
	CongNodePtr node;
	const gchar *clipboard_source;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	node = cong_document_get_selected_node (doc);
	g_return_if_fail (node);

	ds = cong_document_get_default_dispspec(doc);

	clipboard_source = cong_app_get_clipboard_xml_source (cong_app_singleton(),
							      GDK_SELECTION_CLIPBOARD,
							      doc);
	if (clipboard_source) {
		cong_document_paste_source_under (doc,
						  node,
						  clipboard_source);
	}
}

/**
 * cong_ui_hook_tree_paste_before:
 * @doc:
 * @parent_window:
 *
 * TODO: Write me
 */
void
cong_ui_hook_tree_paste_before (CongDocument *doc,
				GtkWindow *parent_window)
{
	CongDispspec *ds;
	CongNodePtr node;
	const gchar *clipboard_source;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	node = cong_document_get_selected_node (doc);
	g_return_if_fail (node);

	ds = cong_document_get_default_dispspec(doc);

	clipboard_source = cong_app_get_clipboard_xml_source (cong_app_singleton(),
							      GDK_SELECTION_CLIPBOARD,
							      doc);
	if (clipboard_source) {
		cong_document_paste_source_before (doc,
						   node,
						   clipboard_source);
	}
}

/**
 * cong_ui_hook_tree_paste_after:
 * @doc:
 * @parent_window:
 *
 * TODO: Write me
 */
void
cong_ui_hook_tree_paste_after (CongDocument *doc,
			       GtkWindow *parent_window)
{
	CongDispspec *ds;
	CongNodePtr node;
	const gchar *clipboard_source;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	node = cong_document_get_selected_node (doc);
	g_return_if_fail (node);

	ds = cong_document_get_default_dispspec(doc);

	clipboard_source = cong_app_get_clipboard_xml_source (cong_app_singleton(),
							      GDK_SELECTION_CLIPBOARD,
							      doc);
	if (clipboard_source) {
		cong_document_paste_source_after (doc,
						  node,
						  clipboard_source);
	}
}

void
cong_ui_hook_tree_convert_to_comment (CongDocument *doc,
				      GtkWindow *parent_window)
{
	gchar *source;
	CongNodePtr node;
	CongNodePtr comment_node;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	node = cong_document_get_selected_node (doc);
	g_return_if_fail (node);

	source = cong_node_generate_source(node);
	g_message ("source");

	cong_document_begin_edit(doc);

	{
		CongCommand *cmd = cong_document_begin_command (doc, _("Convert to comment"), NULL);

		/* New element: */
		comment_node = cong_node_new_comment (source,
						      doc);


		cong_command_add_node_add_after (cmd, 
						 comment_node, 
						 node);

		/* Remove old node: */
		cong_command_for_each_location (cmd, 
						tree_cut_update_location_callback,
						node);
		cong_command_add_node_recursive_delete (cmd,
							node);

		cong_document_end_command (doc, cmd);		
	}

	cong_document_end_edit(doc);
	
	g_free(source);
}

void
cong_ui_hook_tree_convert_from_comment (CongDocument *doc,
					GtkWindow *parent_window)
{
	CongNodePtr comment_node;
	CongNodePtr new_nodes; 
	CongNodePtr iter, iter_next;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	comment_node = cong_document_get_selected_node (doc);
	g_return_if_fail (comment_node);
	g_return_if_fail (CONG_NODE_TYPE_COMMENT == cong_node_type (comment_node));
	g_return_if_fail (comment_node->content);


	new_nodes = cong_document_make_nodes_from_source_fragment (doc, 
								   (const gchar*)comment_node->content);
	if (NULL==new_nodes) {
		/* Couldn't parse the source */
		return;
	}

	cong_document_begin_edit (doc);

	{
		CongCommand *cmd = cong_document_begin_command (doc, _("Uncomment"), NULL);
		CongNodePtr relative_to_node = comment_node;


		/* Add the new nodes: */
		for (iter = new_nodes->children; iter; iter = iter_next) {
			iter_next = iter->next;
			
			cong_command_add_node_add_after (cmd, 
							 iter, 
							 relative_to_node);
			
			relative_to_node = iter;
		}
		
		/* Delete the placeholder parent: */
		cong_command_add_node_recursive_delete (cmd, 
							new_nodes);


		/* Remove the original comment node: */
		cong_command_for_each_location (cmd, 
						tree_cut_update_location_callback,
						comment_node);
		cong_command_add_node_recursive_delete (cmd,
							comment_node);
		
		/* Merge adjacent text nodes: */
		cong_command_add_merge_adjacent_text_children_of_node (cmd, 
								       relative_to_node->parent);

		cong_document_end_command (doc,
					   cmd);
	}
	
	cong_document_end_edit (doc);


}

void
cong_ui_hook_tree_convert_from_entity_ref_to_copy  (CongDocument *doc,
						    GtkWindow *parent_window)
{
	CongNodePtr entity_ref_node;
	CongNodePtr entity_decl_node;
	CongNodePtr iter;
	gchar *command_name;
	CongCommand *cmd;
	CongNodePtr relative_to_node;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	entity_ref_node = cong_document_get_selected_node (doc);
	g_return_if_fail (entity_ref_node);
	g_return_if_fail (CONG_NODE_TYPE_ENTITY_REF == cong_node_type (entity_ref_node));

	/* Expect a single child of the ref: the entity definition: */	
	entity_decl_node = entity_ref_node->children;
	g_return_if_fail (entity_decl_node);
	g_return_if_fail (CONG_NODE_TYPE_ENTITY_DECL == cong_node_type (entity_decl_node));

	cong_document_begin_edit (doc);

	command_name = g_strdup_printf (_("Convert reference to \"%s\" to copy"), entity_ref_node->name);
	cmd = cong_document_begin_command (doc, command_name, NULL);
	relative_to_node = entity_ref_node;

	for (iter=entity_decl_node->children; iter; iter=iter->next) {
		CongNodePtr new_node = cong_node_recursive_dup (iter);

		cong_command_add_node_add_after (cmd, 
						 new_node,
						 relative_to_node);
		
		relative_to_node = new_node;
	}

	/* Remove the original entity ref node: */
	cong_command_for_each_location (cmd, 
					tree_cut_update_location_callback,
					entity_ref_node);
	cong_command_add_node_make_orphan (cmd,
					   entity_ref_node);

	/* Merge adjacent text nodes: */
	cong_command_add_merge_adjacent_text_children_of_node (cmd, 
							       relative_to_node->parent);
	
	cong_document_end_command (doc,
				   cmd);

	cong_document_end_edit (doc);
}
