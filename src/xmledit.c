/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include "global.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-app.h"
#include "cong-selection.h"
#include "cong-range.h"
#include "cong-error-dialog.h"
#include "cong-command.h"

/* --- Cut/copy/paste --- */
void cong_document_cut_selection(CongDocument *doc)
{
	CongSelection *selection;
	CongCursor *curs;
	CongRange *ordered_range; 
	gchar *source;

	g_return_if_fail(doc);

	selection = cong_document_get_selection(doc);
	ordered_range = cong_selection_get_ordered_range (selection);
	curs = cong_document_get_cursor(doc);
	
	if (!cong_location_exists(&curs->location)) return;

	if (!cong_range_can_be_cut (ordered_range)) {
		g_warning ("Selection cannot be cut - UI should be insensitive");
		return;
	}

	cong_document_begin_edit (doc);

	source = cong_range_generate_source (ordered_range);

	{
		CongCommand *cmd = cong_document_begin_command (doc, _("Cut"), NULL);

		cong_app_set_clipboard_from_xml_fragment (cong_app_singleton(),
							  GDK_SELECTION_CLIPBOARD,
							  source,
							  doc);
		g_free(source);
		
		cong_command_add_delete_range (cmd,
					       ordered_range);
		
		cong_document_end_command (doc,
					   cmd);
	}

	cong_document_end_edit (doc);
}

void cong_document_copy_selection(CongDocument *doc)
{
	CongSelection *selection;
	CongCursor *curs;

	g_return_if_fail(doc);

	selection = cong_document_get_selection(doc);
	curs = cong_document_get_cursor(doc);
	
	if (!cong_location_exists(&curs->location)) return;
	
	if (!(cong_range_exists (cong_selection_get_logical_range (selection)) &&
	      cong_range_is_valid (cong_selection_get_logical_range (selection)))) { 
		return;
	}

	if (cong_range_is_empty (cong_selection_get_logical_range (selection))) {
		return;
	}

	if (!cong_range_can_be_copied (cong_selection_get_ordered_range (selection))) {
		g_warning ("Selection cannot be copied - UI should be insensitive");
		return;
	}

	/* GREP FOR MVC */
	cong_document_begin_edit (doc);

	{
		gchar *source = cong_range_generate_source (cong_selection_get_ordered_range (selection));
		
		cong_app_set_clipboard_from_xml_fragment (cong_app_singleton(),
							  GDK_SELECTION_CLIPBOARD,
							  source,
							  doc);

		g_free (source);
	}

	cong_document_end_edit (doc);
}


void cong_document_paste_clipboard_or_selection(CongDocument *doc, GtkWidget *widget)
{
	CongSelection *selection;
	CongCursor *curs;
	const gchar *source_fragment;

	g_return_if_fail(doc);

	selection = cong_document_get_selection(doc);
	curs = cong_document_get_cursor(doc);
	
	if (!cong_location_exists(&curs->location)) return;

	source_fragment = cong_app_get_clipboard_xml_source (cong_app_singleton (),
							     GDK_SELECTION_CLIPBOARD,
							     doc);


	if (source_fragment) {
		cong_document_paste_source_at (doc, 
					       &curs->location, 
					       source_fragment);
	}
}

void 
cong_document_paste_source_at (CongDocument *doc, 
			       CongLocation *insert_loc, 
			       const gchar *source_fragment)
{
	CongNodePtr new_nodes; 
	CongNodePtr node_after = NULL; /* shut compiler up */
	CongNodePtr iter, iter_next;

	g_return_if_fail (doc);
	g_return_if_fail (insert_loc);
	g_return_if_fail (source_fragment);
	g_return_if_fail (insert_loc->node);	

	new_nodes = cong_document_make_nodes_from_source_fragment (doc, 
								   source_fragment);

	if (NULL==new_nodes) {
		/* FIXME: need some kind of error reporting? */
		return;
	}

	cong_document_begin_edit (doc);

	/* We will add the children of new_node in place, then delete the placeholder parent, then merge adjacent text nodes if necessary. */

	{
		CongCommand *cmd = cong_document_begin_command (doc, _("Paste"), NULL);

		/* Calculate insertion point, splitting text nodes if necessary: */
		if (cong_location_node_type(insert_loc) == CONG_NODE_TYPE_TEXT) {
			
			if (0==insert_loc->byte_offset) {
				node_after = insert_loc->node;
			} else if (!cong_location_get_unichar(insert_loc)) {
				node_after = cong_location_xml_frag_next(insert_loc);
			} else {
				/* Split data node */
				cong_command_add_xml_frag_data_nice_split2 (cmd,
									    insert_loc);
				
				node_after = cong_location_xml_frag_next(insert_loc);
			}
		} else {
			g_assert_not_reached();
		}
		
		if (node_after) {
			
			/* Add the new nodes: */
			for (iter = new_nodes->children; iter; iter = iter_next) {
				iter_next = iter->next;

				cong_command_add_node_add_before (cmd, 
								  iter, 
								  node_after);		
			}
			
			/* Merge adjacent text nodes: */
			cong_command_add_merge_adjacent_text_children_of_node (cmd, 
									       node_after->parent);
		} else {
			
			/* Add the new nodes at the end of the parent's list: */
			for (iter = new_nodes->children; iter; iter = iter_next) {
				iter_next = iter->next;
				
				cong_command_add_node_set_parent (cmd, 
								  iter, 
								  insert_loc->node->parent);		
			}
			
			/* Merge adjacent text nodes: */
			cong_command_add_merge_adjacent_text_children_of_node (cmd, 
									       insert_loc->node->parent);
		}
		
		/* Delete the placeholder parent: */
		cong_command_add_node_recursive_delete (cmd, 
							new_nodes);
		cong_document_end_command (doc,
					   cmd);
	}
	
	cong_document_end_edit (doc);

}

void
cong_document_paste_source_under (CongDocument *doc, 
				  CongNodePtr relative_to_node,
				  const gchar *source_fragment)
{
	CongNodePtr new_nodes; 
	CongNodePtr iter, iter_next;

	g_return_if_fail (doc);
	g_return_if_fail (relative_to_node);
	g_return_if_fail (source_fragment);

	new_nodes = cong_document_make_nodes_from_source_fragment (doc, 
								   source_fragment);
	cong_document_begin_edit (doc);

	{
		CongCommand *cmd = cong_document_begin_command (doc, _("Paste Under"), NULL);

		/* Add the new nodes: */
		for (iter = new_nodes->children; iter; iter = iter_next) {
			iter_next = iter->next;
			
			cong_command_add_node_set_parent (cmd, 
							  iter, 
							  relative_to_node);		
		}
		
		/* Delete the placeholder parent: */
		cong_command_add_node_recursive_delete (cmd, 
							new_nodes);
		
		/* Merge adjacent text nodes: */
		cong_command_add_merge_adjacent_text_children_of_node (cmd, 
								       relative_to_node);

		cong_document_end_command (doc,
					   cmd);
	}
	
	cong_document_end_edit (doc);
}

void
cong_document_paste_source_before (CongDocument *doc, 
				   CongNodePtr relative_to_node,
				   const gchar *source_fragment)
{
	CongNodePtr new_nodes; 
	CongNodePtr iter, iter_next;

	g_return_if_fail (doc);
	g_return_if_fail (relative_to_node);
	g_return_if_fail (source_fragment);

	new_nodes = cong_document_make_nodes_from_source_fragment (doc, 
								   source_fragment);
	cong_document_begin_edit (doc);

	{
		CongCommand *cmd = cong_document_begin_command (doc, _("Paste Before"), NULL);

		/* Add the new nodes: */
		for (iter = new_nodes->children; iter; iter = iter_next) {
			iter_next = iter->next;
			
			cong_command_add_node_add_before (cmd, 
							  iter, 
							  relative_to_node);
		}
		
		/* Delete the placeholder parent: */
		cong_command_add_node_recursive_delete (cmd, 
							new_nodes);
		
		/* Merge adjacent text nodes: */
		cong_command_add_merge_adjacent_text_children_of_node (cmd, 
								       relative_to_node->parent);
		cong_document_end_command (doc,
					   cmd);
	}
	
	cong_document_end_edit (doc);
}

void
cong_document_paste_source_after (CongDocument *doc, 
				  CongNodePtr relative_to_node,
				  const gchar *source_fragment)
{
	CongNodePtr new_nodes; 
	CongNodePtr iter, iter_next;

	g_return_if_fail (doc);
	g_return_if_fail (relative_to_node);
	g_return_if_fail (source_fragment);

	new_nodes = cong_document_make_nodes_from_source_fragment (doc, 
								   source_fragment);
	cong_document_begin_edit (doc);

	{
		CongCommand *cmd = cong_document_begin_command (doc, _("Paste After"), NULL);


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
		
		/* Merge adjacent text nodes: */
		cong_command_add_merge_adjacent_text_children_of_node (cmd, 
								       relative_to_node->parent);

		cong_document_end_command (doc,
					   cmd);
	}
	
	cong_document_end_edit (doc);
}

extern char *ilogo_xpm[];

void cong_document_view_source(CongDocument *doc)
{
	GtkWidget *window;
	GtkWidget *source_view;

	g_return_if_fail(doc);

	window = gnome_app_new(PACKAGE_NAME,
			       _("Source View - Conglomerate"));
	source_view = cong_source_view_new(doc);

	gnome_app_set_contents(GNOME_APP(window), source_view);

	/* Set up the window nicely: */
	{	
		GdkPixbuf *icon_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)ilogo_xpm);
		
		gtk_window_set_icon(GTK_WINDOW(window),
				    icon_pixbuf);

		gdk_pixbuf_unref(icon_pixbuf);

	}

	gtk_window_set_default_size(GTK_WINDOW(window),
				    500,
				    400);

	gtk_widget_show(GTK_WIDGET(window));

}










