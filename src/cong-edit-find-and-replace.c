/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * cong-edit-find-and-replace.c
 *
 * This code is mostly taken from gedit 2.6
 * gedit/dialogs/gedit-replace-dialog.c
 *
 * Copyright (C) 2001-2002 Paolo Maggi 
 * Copyright (C) 2004 Nickolay V. Shmyrev
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */


#include <string.h>

#include <glade/glade-xml.h>
#include <gtk/gtk.h>

#include "global.h"
#include "cong-document.h"
#include "cong-util.h"
#include "cong-edit-find-and-replace.h"
#include "cong-selection.h"
#include "cong-primary-window.h"
#include "cong-command.h"
#include "cong-glade.h"

#define CONG_RESPONSE_FIND		101
#define CONG_RESPONSE_REPLACE		102
#define CONG_RESPONSE_REPLACE_ALL	103

typedef struct _CongDialogReplace CongDialogReplace;

struct _CongDialogReplace {

	CongDocument *doc;
	GtkWidget *dialog;

	GtkWidget *search_entry;
	GtkWidget *replace_entry;
	GtkWidget *search_entry_list;
	GtkWidget *replace_entry_list;
	GtkWidget *match_case_checkbutton;
	GtkWidget *entire_word_checkbutton;
	GtkWidget *wrap_around_checkbutton;
	GtkWidget *search_backwards_checkbutton;
};


static void
dlg_find_button_pressed (CongDialogReplace *dialog);
static void
replace_dlg_replace_button_pressed (CongDialogReplace *dialog);
static void
replace_dlg_replace_all_button_pressed (CongDialogReplace *dialog);
static void
update_menu_items_sensitivity (CongDocument *doc);

static gboolean
dialog_replace_destroyed (GtkObject *obj,  void *user_data)
{
        CongDialogReplace ** data = (CongDialogReplace **)user_data;

        g_object_unref (G_OBJECT ((*data)->doc));
	g_free (*data);
	*data = NULL;

	return FALSE;
}


static void 
text_not_found_dialog (const gchar *text, GtkWindow *parent)
{
	GtkWidget *message_dlg;

	g_return_if_fail (text != NULL);
	
	message_dlg = gtk_message_dialog_new (
			parent,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK,
			_("The text \"%s\" was not found."), text);
		
	gtk_dialog_set_default_response (GTK_DIALOG (message_dlg), GTK_RESPONSE_OK);

	gtk_window_set_resizable (GTK_WINDOW (message_dlg), FALSE);
	
	gtk_dialog_run (GTK_DIALOG (message_dlg));
  	gtk_widget_destroy (message_dlg);
}


static void
dialog_replace_response_handler (GtkDialog *dlg, gint res_id,  CongDialogReplace *replace_dialog)
{

	switch (res_id) {
		case CONG_RESPONSE_FIND:
			dlg_find_button_pressed (replace_dialog);
			break;
			
		case CONG_RESPONSE_REPLACE:
			replace_dlg_replace_button_pressed (replace_dialog);
			break;
			
		case CONG_RESPONSE_REPLACE_ALL:
			replace_dlg_replace_all_button_pressed (replace_dialog);
			break;
		default:
			gtk_widget_destroy (replace_dialog->dialog);
	}
}

static void 
replace_search_entry_changed (GtkEditable *editable, CongDialogReplace *dialog)
{
	const gchar *search_string;
	
	search_string = gtk_entry_get_text (GTK_ENTRY (dialog->search_entry));		
	g_return_if_fail (search_string != NULL);

	if (strlen (search_string) <= 0)
	{
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog->dialog), 
			CONG_RESPONSE_FIND, FALSE);
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog->dialog), 
			CONG_RESPONSE_REPLACE, FALSE);
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog->dialog), 
			CONG_RESPONSE_REPLACE_ALL, FALSE);
	}
	else
	{
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog->dialog), 
			CONG_RESPONSE_FIND, TRUE);
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog->dialog), 
			CONG_RESPONSE_REPLACE_ALL, TRUE);
	}
}

static CongDialogReplace *
dialog_replace_get_dialog (CongDocument *doc)
{
	static CongDialogReplace *dialog = NULL;
	GladeXML *gui;
	GtkWindow *window;
	GtkWidget *content;
	GtkWidget *replace_with_label;
	
	window = GTK_WINDOW (cong_primary_window_get_toplevel
	                      (cong_document_get_primary_window (doc)));
	if (dialog != NULL)
	{
		gtk_window_set_transient_for (GTK_WINDOW (dialog->dialog),
					      GTK_WINDOW (window));
		gtk_window_present (GTK_WINDOW (dialog->dialog));
		gtk_widget_grab_focus (dialog->dialog);

		return dialog;
	}

	gui = cong_util_load_glade_file ("conglomerate/glade/cong-find-replace.glade", 
					 "replace_dialog_content",
					 doc,
					 NULL);

	dialog = g_new0 (CongDialogReplace, 1);

        dialog->doc = doc;
	g_object_ref (G_OBJECT(doc));

	dialog->dialog = gtk_dialog_new_with_buttons (_("Replace"),
						      window,						      
						      GTK_DIALOG_DESTROY_WITH_PARENT,
						      GTK_STOCK_CLOSE,
						      GTK_RESPONSE_CLOSE,
						      NULL);

	g_return_val_if_fail (dialog->dialog != NULL, NULL);

	gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog->dialog), FALSE);

	gtk_dialog_add_button (GTK_DIALOG (dialog->dialog), 
			       _("Replace _All"), CONG_RESPONSE_REPLACE_ALL);

	gtk_dialog_add_button (GTK_DIALOG (dialog->dialog), GTK_STOCK_FIND_AND_REPLACE,
				 CONG_RESPONSE_REPLACE);

	gtk_dialog_add_button (GTK_DIALOG (dialog->dialog), 
			       GTK_STOCK_FIND, CONG_RESPONSE_FIND);

	content = glade_xml_get_widget (gui, "replace_dialog_content");
	dialog->search_entry       = glade_xml_get_widget (gui, "search_for_text_entry");
	dialog->replace_entry      = glade_xml_get_widget (gui, "replace_with_text_entry");
	dialog->search_entry_list  = glade_xml_get_widget (gui, "search_for_text_entry_list");
	dialog->replace_entry_list = glade_xml_get_widget (gui, "replace_with_text_entry_list");
	replace_with_label         = glade_xml_get_widget (gui, "replace_with_label");
	dialog->match_case_checkbutton = glade_xml_get_widget (gui, "match_case_checkbutton");
	dialog->wrap_around_checkbutton =  glade_xml_get_widget (gui, "wrap_around_checkbutton");
	dialog->entire_word_checkbutton = glade_xml_get_widget (gui, "entire_word_checkbutton");
	dialog->search_backwards_checkbutton = glade_xml_get_widget (gui, "search_backwards_checkbutton");

	if (!content				||
	    !dialog->search_entry 		||
	    !dialog->replace_entry  		||
	    !dialog->search_entry_list		||
	    !dialog->replace_entry_list		||  
	    !replace_with_label 		||
	    !dialog->match_case_checkbutton 	||
	    !dialog->entire_word_checkbutton 	||
	    !dialog->wrap_around_checkbutton 	||
	    !dialog->search_backwards_checkbutton)
	{
		g_warning ("Missing widget in cong-find-replace.glade");
		return NULL;
	}

	gtk_widget_show (replace_with_label);
	gtk_widget_show (dialog->replace_entry);
	gtk_widget_show (dialog->replace_entry_list);
	
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->dialog)->vbox),
			    content, FALSE, FALSE, 0);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog->dialog),
					 CONG_RESPONSE_FIND);

	g_signal_connect (G_OBJECT (dialog->search_entry_list), "changed",
			  G_CALLBACK (replace_search_entry_changed), dialog);
	
	g_signal_connect (G_OBJECT (dialog->dialog), "destroy",
			  G_CALLBACK (dialog_replace_destroyed), &dialog);

	g_signal_connect (G_OBJECT (dialog->dialog), "response",
			  G_CALLBACK (dialog_replace_response_handler), dialog);

	g_object_unref (G_OBJECT (gui));

	return dialog;
}

static void 
find_search_entry_changed (GtkEditable *editable, CongDialogReplace *dialog)
{
	const gchar *search_string;
	
	search_string = gtk_entry_get_text (GTK_ENTRY (dialog->search_entry));		
	g_return_if_fail (search_string != NULL);

	if (strlen (search_string) <= 0)
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog->dialog), 
			CONG_RESPONSE_FIND, FALSE);
	else
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog->dialog), 
			CONG_RESPONSE_FIND, TRUE);
}

static CongDialogReplace *
dialog_find_get_dialog (CongDocument *doc)
{
	static CongDialogReplace *dialog = NULL;
	GladeXML *gui;
	GtkWindow *window;
	GtkWidget *content;
	GtkWidget *table;
	GtkWidget *replace_with_label;
	
	window = GTK_WINDOW (cong_primary_window_get_toplevel
	                      (cong_document_get_primary_window (doc)));

	if (dialog != NULL)
	{
		gtk_window_set_transient_for (GTK_WINDOW (dialog->dialog),
					      GTK_WINDOW (window));
		gtk_window_present (GTK_WINDOW (dialog->dialog));
		gtk_widget_grab_focus (dialog->dialog);

		return dialog;
	}

	gui = cong_util_load_glade_file ("conglomerate/glade/cong-find-replace.glade", 
					 "replace_dialog_content",
					 doc,
					 NULL);

	dialog = g_new0 (CongDialogReplace, 1);

        dialog->doc = doc;
	g_object_ref (G_OBJECT(doc));
	
	dialog->dialog = gtk_dialog_new_with_buttons (_("Find"),
						      window,
						      GTK_DIALOG_DESTROY_WITH_PARENT,
						      GTK_STOCK_CLOSE,
						      GTK_RESPONSE_CLOSE,
						      GTK_STOCK_FIND,
						      CONG_RESPONSE_FIND,
						      NULL);

	g_return_val_if_fail (dialog->dialog != NULL, NULL);

	gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog->dialog), FALSE);

	content = glade_xml_get_widget (gui, "replace_dialog_content");

	dialog->search_entry       = glade_xml_get_widget (gui, "search_for_text_entry");
	dialog->search_entry_list  = glade_xml_get_widget (gui, "search_for_text_entry_list");	
	dialog->replace_entry  	   = glade_xml_get_widget (gui, "replace_with_text_entry");
	dialog->replace_entry_list = glade_xml_get_widget (gui, "replace_with_text_entry_list");

	replace_with_label         = glade_xml_get_widget (gui, "replace_with_label");
	
	dialog->match_case_checkbutton = glade_xml_get_widget (gui, "match_case_checkbutton");
	dialog->wrap_around_checkbutton =  glade_xml_get_widget (gui, "wrap_around_checkbutton");
	dialog->entire_word_checkbutton = glade_xml_get_widget (gui, "entire_word_checkbutton");
	dialog->search_backwards_checkbutton = glade_xml_get_widget (gui, "search_backwards_checkbutton");

	table                      = glade_xml_get_widget (gui, "table");

	if (!content				||
	    !table				||
	    !dialog->search_entry 		||
	    !dialog->search_entry_list 		||
	    !dialog->replace_entry		||
	    !dialog->replace_entry_list 	||
	    !replace_with_label 		||
	    !dialog->match_case_checkbutton 	||
	    !dialog->entire_word_checkbutton 	||
	    !dialog->wrap_around_checkbutton	||
	    !dialog->search_backwards_checkbutton)
	{
		g_warning ("Missing widget in cong-find-replace.glade");
		return NULL;
	}

	gtk_widget_hide (replace_with_label);
	gtk_widget_hide (dialog->replace_entry_list);

	gtk_table_set_row_spacings (GTK_TABLE (table), 0);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->dialog)->vbox),
			    content, FALSE, FALSE, 0);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog->dialog),
					 CONG_RESPONSE_FIND);

	g_signal_connect (G_OBJECT (dialog->search_entry_list), "changed",
			  G_CALLBACK (find_search_entry_changed), dialog);

	g_signal_connect(G_OBJECT (dialog->dialog), "destroy",
			 G_CALLBACK (dialog_replace_destroyed), &dialog);

	g_signal_connect(G_OBJECT (dialog->dialog), "response",
			 G_CALLBACK (dialog_replace_response_handler), dialog);

	g_object_unref (G_OBJECT (gui));

	return dialog;
}

/**
 * cong_document_find:
 *
 * @doc: the #CongDocument for which the find dialog is to be run
 *
 * Opens the Find dialog for this #CongDocument
 *
 */
void
cong_document_find (CongDocument *doc)
{
	CongDialogReplace *dialog;
	CongFindDialogData *data;
	gchar *selected_text;
	
	dialog = dialog_find_get_dialog (doc);
	if (!dialog)
		return;

	if (GTK_WIDGET_VISIBLE (dialog->dialog))
		return;

	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog->dialog), 
					   CONG_RESPONSE_FIND, FALSE);


      
	selected_text = cong_selection_get_selected_text (doc);
        data = cong_document_get_find_dialog_data (doc);

	if (selected_text && !
	      (selected_text[0]=='<' && selected_text[strlen(selected_text) - 1]=='>')) 
	{		
         	gtk_entry_set_text (GTK_ENTRY (dialog->search_entry), selected_text);
		
		g_free (selected_text);
	} 
	else 
	{
		if (data->last_find != NULL)
		{
			gtk_entry_set_text (GTK_ENTRY (dialog->search_entry), data->last_find);
		}
	}
      
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->match_case_checkbutton), 
				      data->is_match_case);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->entire_word_checkbutton),
				      data->is_entire_word);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->wrap_around_checkbutton),
				      data->is_wrap_around);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->search_backwards_checkbutton),
				      data->is_search_backwards);

	gtk_widget_grab_focus (dialog->search_entry);
	
	gtk_widget_show (dialog->dialog);
}


void
cong_document_replace (CongDocument *doc)
{
	CongDialogReplace *dialog;
	CongFindDialogData *data;
	gchar *selected_text;
	
	dialog = dialog_replace_get_dialog (doc);
	if (!dialog)
		return;

	if (GTK_WIDGET_VISIBLE (dialog->dialog))
		return;

	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog->dialog), 
					   CONG_RESPONSE_REPLACE, FALSE);

	selected_text = cong_selection_get_selected_text (doc);
        data = cong_document_get_find_dialog_data (doc);

	if (selected_text && !
	      (selected_text[0]=='<' && selected_text[strlen(selected_text) - 1] == '>')) 
	{		
         	gtk_entry_set_text (GTK_ENTRY (dialog->search_entry), selected_text);
		
		g_free (selected_text);
	} 
	else 
	{
		if (data->last_find != NULL)
		{
			gtk_entry_set_text (GTK_ENTRY (dialog->search_entry), data->last_find);
		}
	}
      
	if (data->last_replace != NULL)
	{
		gtk_entry_set_text (GTK_ENTRY (dialog->replace_entry), data->last_replace);
	}
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->match_case_checkbutton), 
				      data->is_match_case);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->entire_word_checkbutton),
				      data->is_entire_word);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->wrap_around_checkbutton),
				      data->is_wrap_around);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->search_backwards_checkbutton),
				      data->is_search_backwards);

	gtk_widget_grab_focus (dialog->search_entry);
	
	gtk_widget_show (dialog->dialog);
}

static void
setup_find_data_from_dialog (CongDialogReplace *dialog)
{
        CongFindDialogData *data;
	const gchar *search_string = NULL;
	const gchar *replace_string = NULL;

	gboolean case_sensitive;
	gboolean entire_word;
	gboolean wrap_around;
	gboolean search_backwards;

        data = cong_document_get_find_dialog_data (dialog->doc);
	search_string = gtk_entry_get_text (GTK_ENTRY (dialog->search_entry));		
	replace_string = gtk_entry_get_text (GTK_ENTRY (dialog->replace_entry));		

        if (search_string && strlen(search_string) > 0)
  	  gnome_entry_prepend_history (GNOME_ENTRY (dialog->search_entry_list), TRUE, search_string);

        if (replace_string && strlen(replace_string) > 0)
  	  gnome_entry_prepend_history (GNOME_ENTRY (dialog->replace_entry_list), TRUE, replace_string);
		
	/* retrieve search settings from the dialog */
	case_sensitive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->match_case_checkbutton));
	entire_word = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->entire_word_checkbutton));
	wrap_around = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->wrap_around_checkbutton));
	search_backwards = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->search_backwards_checkbutton));

        /* Setup setting for next invocation */
	
        if (search_string && strlen(search_string) > 0)
	 {
		g_free(data->last_find);
		data->last_find = g_strdup (search_string);
	 }

        if (replace_string && strlen(replace_string) > 0)
	 {
		g_free(data->last_replace);
		data->last_replace = g_strdup (replace_string);
	 }
	data->is_match_case = case_sensitive;
	data->is_entire_word = entire_word;
	data->is_wrap_around = wrap_around;
	data->is_search_backwards = search_backwards;

        return;
}

static gchar*
dlg_strstr (gchar *heystack, gchar *needle, CongFindDialogData *data)
{
   gchar *content;
   gchar *str;
   gchar *result;
   gchar *found;
   gchar *p;
   gint bytes_added;
   
   if (!data->is_match_case)
     {
       content = g_utf8_casefold (heystack, -1);
       str = g_utf8_casefold (needle, -1);
     }
   else
     {
       content = g_strdup (heystack);
       str = g_strdup (needle);
     }

   bytes_added = 0;
   while (1)
   {     
       if (data->is_search_backwards)
        {
	      found = g_strrstr (content, str);
        }
       else
        {
	      found = g_strstr_len (content, strlen (content), str);
        }

        if (!found)
         {
	     g_free (content);
             g_free (str);
	     return NULL;
         }

      if (data->is_entire_word)
	{
	  gchar *prev;
	  gchar *next;
      
	  prev = g_utf8_find_prev_char (content, found);
	  next = g_utf8_find_next_char (found + strlen (str) - 1, NULL);
      
          if ((prev != NULL && g_unichar_isalpha (g_utf8_get_char(prev))) ||
	   (next != NULL && g_unichar_isalpha (g_utf8_get_char(next))) )
	      {
	          if (data->is_search_backwards)
		   {
		    p = g_strndup (content, found - content);
		   }
		  else
		   {
		    p = g_strdup (found + strlen(str));
		    bytes_added += (found - content)+ strlen(str);
		   }
		  g_free (content);
		  content = p;
    		  continue;
	      }
         }
	 
	 break;
   }
        
   result = heystack + (found - content) + bytes_added;
     
   g_free (content);
   g_free (str);
   
   return result;
}

static gboolean 
contains_search_string (CongNodePtr node,
			gpointer user_data)
{
	CongFindDialogData *data = (CongFindDialogData *)user_data;
	
	g_assert (node);

        if (cong_node_type (node)==CONG_NODE_TYPE_TEXT) {
	  if (dlg_strstr (node->content, data->last_find, data))
	    return TRUE;
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
