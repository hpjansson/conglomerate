/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include "global.h"
#include "cong-document.h"
#include "cong-error-dialog.h"
#include "cong-app.h"
#include "cong-primary-window.h"
#include "cong-edit-find-and-replace.h"
#include "cong-util.h"
#include "cong-command.h"
#include <glade/glade.h>

typedef struct CongFindDialogDetails CongFindDialogDetails;

struct CongFindDialogDetails
{
	GladeXML *xml;
	CongDocument *doc;
};

static void
on_find_dialog_find (GtkWidget *widget,
		     CongFindDialogDetails *dialog_details);

static gboolean
on_find_dialog_destroy (GtkWidget *widget,
			CongFindDialogDetails *dialog_details);


/**
 * cong_document_find:
 * @doc: the #CongDocument for which the find dialog is to be run
 *
 * Opens the Find dialog for this #CongDocument
 *
 */
void
cong_document_find (CongDocument *doc)
{
	GtkDialog *dialog;
	CongFindDialogDetails *dialog_details;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	dialog_details = g_new0 (CongFindDialogDetails, 1);

	dialog_details->xml = cong_util_load_glade_file ("conglomerate/glade/cong-find-replace.glade", 
							 "find_dialog",
							 doc,
							 NULL);
	dialog_details->doc = doc;
	g_object_ref (G_OBJECT (doc));

	dialog = GTK_DIALOG (glade_xml_get_widget(dialog_details->xml, "find_dialog"));

	/* Button handler: */
	g_signal_connect (G_OBJECT (glade_xml_get_widget(dialog_details->xml, "button_find")),
			  "clicked",
			  G_CALLBACK (on_find_dialog_find),
			  dialog_details);
	
	
	/* Cleanup handler: */
	g_signal_connect (G_OBJECT (dialog),
			  "destroy",
			  G_CALLBACK (on_find_dialog_destroy),
			  dialog_details);
}

/**
 * cong_document_find_next:
 * @doc:
 *
 * This function is not currently implemented
 */
void
cong_document_find_next (CongDocument *doc)
{
	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	CONG_DO_UNIMPLEMENTED_DIALOG (NULL, "Find Next");
}

/**
 * cong_document_find_prev:
 * @doc:
 *
 * This function is not currently implemented
 */
void
cong_document_find_prev (CongDocument *doc)
{
	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	CONG_DO_UNIMPLEMENTED_DIALOG (NULL, "Find Previous");
}

/**
 * cong_document_replace:
 * @doc:
 *
 * TODO: Write me
 */
void
cong_document_replace (CongDocument *doc)
{
	GladeXML *xml;
	GtkDialog *dialog;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	xml = cong_util_load_glade_file ("conglomerate/glade/cong-find-replace.glade", 
					 "replace_dialog",
					 doc,
					 NULL);
	dialog = GTK_DIALOG (glade_xml_get_widget(xml, "replace_dialog"));

}

/* Utility routines: */
typedef struct CongFindParameters CongFindParameters;

struct CongFindParameters
{
	const gchar *search_string;
	gboolean match_case;

	/* might add a filter like Only Text Nodes vs Only Comments vs Both */
};

/**
 * cong_util_strstr_with_case:
 * @haystack:
 * @needle:
 * @match_case:
 *
 * TODO: Write me
 */
const gchar*
cong_util_strstr_with_case (const gchar *haystack,
			    const gchar *needle,
			    gboolean match_case)
{
	/* FIXME: case sensitive for now */
	/* FIXME: is strstr safe enough? */

	return strstr (haystack, needle);
}

static gboolean 
contains_search_string (CongNodePtr node,
			gpointer user_data)
{
	const CongFindParameters *params = (const CongFindParameters*)user_data;
	g_assert (node);

	if (cong_node_type (node)==CONG_NODE_TYPE_TEXT) {
		if (cong_util_strstr_with_case (node->content,
						params->search_string,
						params->match_case)) {
			return TRUE;
		}
	}	

	return FALSE;
}

/* Find start of next occurrence of string: */
static gboolean
find_next (const CongLocation *start_loc,
           CongFindDialogData *data,
	   CongLocation *output, CongDocument *doc)
{
	const gchar *result;
	g_return_val_if_fail (start_loc, FALSE);
	g_return_val_if_fail (output, FALSE);

	/* Search in current node: */
	if (data->is_search_backwards)
	{
		gchar *content_copy;
		gchar *prev_char;
		
		prev_char = g_utf8_find_prev_char(start_loc->node->content, 
		             start_loc->node->content +start_loc->byte_offset);
                
		if (!prev_char)
		 {
		   result = NULL;
		 }
		else
		{
        		content_copy = g_strndup (start_loc->node->content, prev_char - (gchar *)start_loc->node->content);
			result = dlg_strstr (content_copy,
					     data->last_find, data);
		
			if (result)
				result = start_loc->node->content + (result - content_copy);
			g_free (content_copy);
		}
	}
	else
	{
		result = dlg_strstr (start_loc->node->content + start_loc->byte_offset,
					     data->last_find, data);
	}

	if (result) {
		cong_location_set_node_and_byte_offset (output, 
							start_loc->node, 
							result - (const gchar*)start_loc->node->content);
		return TRUE;
	} else {
		/* Not found in this node; scan through other nodes: */
		CongNodePtr next_node;

                if (data->is_search_backwards)
		{
		next_node = cong_node_calc_prev_node_satisfying (start_loc->node, 
								 contains_search_string, data);
                }								 
		else
		{
		next_node = cong_node_calc_next_node_satisfying (start_loc->node, 
								 contains_search_string, data);
                }								 

		if (next_node) {
			result = dlg_strstr (next_node->content, data->last_find, data);
			g_assert (result);

			cong_location_set_node_and_byte_offset (output, 
								next_node, 
								result - (const gchar*)next_node->content);
			return TRUE;
		} else {
		     if (data->is_wrap_around)
		     {
		         if (data->is_search_backwards)
				{
					next_node = cong_node_calc_final_node_in_subtree_satisfying (cong_document_get_root(doc), 
								 contains_search_string, data);
		                }								 
	    		else
				{
					next_node = cong_node_calc_first_node_in_subtree_satisfying (cong_document_get_root (doc), 
								 contains_search_string, data);
		                }								 

			if (next_node) {
				result = dlg_strstr (next_node->content, data->last_find, data);
				g_assert (result);

				cong_location_set_node_and_byte_offset (output, 
									next_node, 
									result - (const gchar*)next_node->content);
				return TRUE;
				}
		     }
		}
	}
  return FALSE;
}


static void
dlg_find_button_pressed (CongDialogReplace *dialog)
{
	const gchar* search_string = NULL;
	gboolean found;
	CongCursor *cursor;
	CongLocation result;
	CongFindDialogData *data;

	search_string = gtk_entry_get_text (GTK_ENTRY (dialog->search_entry));		
	
	g_return_if_fail (search_string != NULL);

	if (strlen (search_string) <= 0)
		return;

	setup_find_data_from_dialog(dialog);
	
	data = cong_document_get_find_dialog_data (dialog->doc);
	
	cursor = cong_document_get_cursor (dialog->doc);

	if (find_next (&cursor->location, data, &result, dialog->doc)) {
		CongLocation string_end;

		CongCommand *cmd = cong_document_begin_command (dialog->doc,
								_("Find"),
								NULL);

		cong_location_set_node_and_byte_offset (&string_end,
							result.node,
							result.byte_offset+strlen (data->last_find));

		cong_command_add_selection_change (cmd,
						   &result,
						   &string_end);

		cong_command_add_cursor_change (cmd,
						&string_end);
		
		cong_document_end_command (dialog->doc,
					   cmd);

         	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog->dialog), 
							CONG_RESPONSE_REPLACE, TRUE);
	} else {
	  text_not_found_dialog(data->last_find, NULL);
     	  gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog->dialog), 
							CONG_RESPONSE_REPLACE, FALSE);
	}

	
	update_menu_items_sensitivity (dialog->doc);
}



static void
replace_dlg_replace_button_pressed (CongDialogReplace *dialog)
{
	const gchar* search_string = NULL;
	const gchar* replace_string = NULL;

	CongFindDialogData *data;

	CongSelection *selection;
	CongCursor *cursor;

	CongRange *ordered_range;
	CongLocation string_start;
	CongLocation result;

	CongCommand *cmd;

	search_string = gtk_entry_get_text (GTK_ENTRY (dialog->search_entry));		
	replace_string = gtk_entry_get_text (GTK_ENTRY (dialog->search_entry));		
	
	g_return_if_fail (search_string != NULL);

	if (strlen (search_string) <= 0)
		return;

	setup_find_data_from_dialog(dialog);
	
	data = cong_document_get_find_dialog_data (dialog->doc);
	
	cursor = cong_document_get_cursor (dialog->doc);

	selection = cong_document_get_selection(dialog->doc);

	ordered_range = cong_selection_get_ordered_range (selection);

	if (!cong_location_exists(&cursor->location)) return;

	if (!cong_range_can_be_cut (ordered_range)) {
		g_warning ("Selection cannot be cut - UI should be insensitive");
		return;
	}

	cong_document_begin_edit (dialog->doc);

	
	cmd = cong_document_begin_command (dialog->doc, _("Replace"),
							NULL);

	cong_command_add_delete_range (cmd, ordered_range);

	cong_command_add_insert_text_at_cursor (cmd, data->last_replace);	
	cong_document_end_command (dialog->doc, cmd);

	cong_document_end_edit (dialog->doc);

	if (find_next (&cursor->location, data, &result, dialog->doc)) {
		CongLocation string_end;

		CongCommand *cmd = cong_document_begin_command (dialog->doc,
								_("Replace"),
								NULL);

		cong_location_set_node_and_byte_offset (&string_end,
							result.node,
							result.byte_offset+strlen (data->last_find));

		cong_command_add_selection_change (cmd,
						   &result,
						   &string_end);

		cong_command_add_cursor_change (cmd,
						&string_end);
		
		cong_document_end_command (dialog->doc,
					   cmd);
      	       gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog->dialog), 
							CONG_RESPONSE_REPLACE, TRUE);
	} else {
	  text_not_found_dialog(data->last_find, NULL);
	  gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog->dialog), 
							CONG_RESPONSE_REPLACE, FALSE);
	}

	
	update_menu_items_sensitivity (dialog->doc);
}


static void
replace_dlg_replace_all_button_pressed (CongDialogReplace *dialog)
{
	const gchar* search_string = NULL;
	const gchar* replace_string = NULL;

	CongCursor *cursor;
	CongLocation result;
	CongLocation from;
	CongFindDialogData *data;
	CongCommand *cmd;

	search_string = gtk_entry_get_text (GTK_ENTRY (dialog->search_entry));		
	replace_string = gtk_entry_get_text (GTK_ENTRY (dialog->search_entry));		
	
	g_return_if_fail (search_string != NULL);

	if (strlen (search_string) <= 0)
		return;

	setup_find_data_from_dialog(dialog);
	
	data = cong_document_get_find_dialog_data (dialog->doc);
	
	cursor = cong_document_get_cursor (dialog->doc);

	cong_document_begin_edit (dialog->doc);

        cmd =  cong_document_begin_command (dialog->doc,
                 			     _("Replace All"),
					     NULL);
	
	from = cursor->location;

	while (find_next (&from, data, &result, dialog->doc)) {
		CongLocation string_end;
		CongRange range;

		cong_location_set_node_and_byte_offset (&string_end,
							result.node,
							result.byte_offset+strlen (data->last_find));
                
		range.loc0 = result;
		range.loc1 = string_end;
		
		
		cong_command_add_delete_range (cmd, &range);

		cong_command_add_cursor_change (cmd, &result);

		cong_command_add_insert_text_at_cursor (cmd, data->last_replace);
						
		from = result;		
	} 
	cong_document_end_command (dialog->doc, cmd);
	cong_document_end_edit (dialog->doc);
	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog->dialog), 
					   CONG_RESPONSE_REPLACE, FALSE);
 	
	
	update_menu_items_sensitivity (dialog->doc);
}

void
cong_document_find_next (CongDocument *doc)
{
	CongCursor *cursor;
	CongLocation result;
	CongFindDialogData *data;

	data = cong_document_get_find_dialog_data (doc);

	data->is_search_backwards = FALSE;
	
	cursor = cong_document_get_cursor (doc);

	if (find_next (&cursor->location, data, &result, doc)) {
		CongLocation string_end;

		CongCommand *cmd = cong_document_begin_command (doc,
								_("Find"),
								NULL);

		cong_location_set_node_and_byte_offset (&string_end,
							result.node,
							result.byte_offset+strlen (data->last_find));

		cong_command_add_selection_change (cmd,
						   &result,
						   &string_end);

		cong_command_add_cursor_change (cmd,
						&string_end);
		
		cong_document_end_command (doc,
					   cmd);

	} else {
	  text_not_found_dialog(data->last_find, NULL);
	}

	update_menu_items_sensitivity (doc);
}

void
cong_document_find_prev (CongDocument *doc)
{
	CongCursor *cursor;
	CongLocation result;
	CongFindDialogData *data;

	data = cong_document_get_find_dialog_data (doc);

	data->is_search_backwards = TRUE;
	
	cursor = cong_document_get_cursor (doc);

	if (find_next (&cursor->location, data, &result, doc)) {
		CongLocation string_end;

		CongCommand *cmd = cong_document_begin_command (doc,
								_("Find"),
								NULL);

		cong_location_set_node_and_byte_offset (&string_end,
							result.node,
							result.byte_offset+strlen (data->last_find));

		cong_command_add_selection_change (cmd,
						   &result,
						   &string_end);

		cong_command_add_cursor_change (cmd,
						&string_end);
		
		cong_document_end_command (doc,
					   cmd);

	} else {
	  text_not_found_dialog(data->last_find, NULL);
        }
	update_menu_items_sensitivity (doc);
}

static void
update_menu_items_sensitivity (CongDocument *document)
{
}
