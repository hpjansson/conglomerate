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
 *
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

	dialog_details->xml = cong_util_load_glade_file ("glade/cong-find-replace.glade", 
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

void
cong_document_find_next (CongDocument *doc)
{
	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	CONG_DO_UNIMPLEMENTED_DIALOG (NULL, "Find Next");
}

void
cong_document_find_prev (CongDocument *doc)
{
	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	CONG_DO_UNIMPLEMENTED_DIALOG (NULL, "Find Previous");
}

void
cong_document_replace (CongDocument *doc)
{
	GladeXML *xml;
	GtkDialog *dialog;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	xml = cong_util_load_glade_file ("glade/cong-find-replace.glade", 
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
	   const CongFindParameters *params,
	   CongLocation *output)
{
	const gchar *result;
	g_return_val_if_fail (start_loc, FALSE);
	g_return_val_if_fail (params, FALSE);
	g_return_val_if_fail (output, FALSE);

	/* Search in current node: */
	result = cong_util_strstr_with_case (start_loc->node->content + start_loc->byte_offset,
					     params->search_string,
					     params->match_case);

	if (result) {
		cong_location_set_node_and_byte_offset (output, 
							start_loc->node, 
							result - (const gchar*)start_loc->node->content);
		return TRUE;
	} else {
		/* Not found in this node; scan through other nodes: */
		CongNodePtr next_node;

		next_node = cong_node_calc_next_node_satisfying (start_loc->node, 
								 contains_search_string,
								 (gpointer)params);

		if (next_node) {
			result = cong_util_strstr_with_case (next_node->content, 
							     params->search_string,
							     params->match_case);
			g_assert (result);

			cong_location_set_node_and_byte_offset (output, 
								next_node, 
								result - (const gchar*)next_node->content);
			return TRUE;
		} else {
			return FALSE;
		}
	}
}

/* Find dialog implementation details: */
static void
do_find_next (CongDocument *doc,
	      const CongFindParameters *params)
{
	CongCursor *cursor;
	CongLocation result;

	g_assert (IS_CONG_DOCUMENT (doc));

	g_message ("find \"%s\" with match_case: %s", params->search_string, ( params->match_case?"TRUE":"FALSE"));

	cursor = cong_document_get_cursor (doc);

	if (find_next (&cursor->location,
		       params,
		       &result)) {
		/* "result" contains start of an occurrence of a search_string */
		CongLocation string_end;

		CongCommand *cmd = cong_document_begin_command (doc,
								_("Find"),
								NULL);

		cong_location_set_node_and_byte_offset (&string_end,
							result.node,
							result.byte_offset+strlen (params->search_string));

		cong_command_add_selection_change (cmd,
						   &result,
						   &string_end);

		cong_command_add_cursor_change (cmd,
						&string_end);
		
		cong_document_end_command (doc,
					   cmd);
	} else {
		/* No matches found */
		/* FIXME: provide a useful dialog to handle this case */
		g_message ("no match found");
	}
}

static void
on_find_dialog_find (GtkWidget *widget,
		     CongFindDialogDetails *dialog_details)
{
	CongFindParameters params;

	g_assert (dialog_details);

	params.search_string = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget(dialog_details->xml, "finddlg_combo-entry_find")));
	params.match_case = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget(dialog_details->xml, "checkbutton_finddlg_matchcase")));

	do_find_next (dialog_details->doc,
		      &params);
}

static gboolean
on_find_dialog_destroy (GtkWidget *widget,
			CongFindDialogDetails *dialog_details)
{
	g_assert (dialog_details);

	g_object_unref (G_OBJECT (dialog_details->doc));
	g_object_unref (G_OBJECT (dialog_details->xml));

	g_free (dialog_details);

	return FALSE;

}

/* Replace dialog implementation details: */
