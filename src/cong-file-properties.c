/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-file-properties.c
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

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "global.h"
#include <string.h>
#include "cong-plugin.h"
#include "cong-app.h"
#include "cong-dialog.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-command.h"
#include "cong-util.h"
#include "cong-glade.h"

typedef struct CongFilePropertiesDialogDetails CongFilePropertiesDialogDetails;

struct CongFilePropertiesDialogDetails
{
	GladeXML *xml;
	CongDocument *doc;

	gulong sigid_end_edit;
	gulong sigid_set_dtd;
	gulong sigid_set_url;
};

static gboolean
on_dialog_destroy (GtkWidget *widget,
		   gpointer user_data);

static void
on_dtd_button_clicked (GtkButton *button,
		       CongDocument *doc);

static void
on_doc_end_edit (CongDocument *doc,
		 gpointer user_data);

static void
on_doc_set_dtd_ptr (CongDocument *doc,
		    xmlDtdPtr dtd_ptr,
		    gpointer user_data);

static void
on_doc_set_url (CongDocument *doc,
		const gchar *new_url,
		gpointer user_data);


static void
refresh_filename_and_location (CongFilePropertiesDialogDetails *dialog_details,
			       CongDocument *doc)
{
	/* Filename: */
	{
		gchar *filename;
		filename = cong_document_get_filename (doc);
		
		gtk_label_set_text ( GTK_LABEL (glade_xml_get_widget (dialog_details->xml,"label_name")), 
				     filename);
		g_free (filename);			
		
	}
	
	/* Location: */
	{
		gchar *path;
		path = cong_document_get_parent_uri (doc);
		
		gtk_label_set_text ( GTK_LABEL (glade_xml_get_widget (dialog_details->xml,"label_location")), 
				     path);
		g_free (path);
	}
}

static const gchar*
get_modified_string (CongDocument *doc) 
{
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);

	return (cong_document_is_modified(doc)? _("Yes") : _("No"));
}

static void
refresh_modified (CongFilePropertiesDialogDetails *dialog_details,
		  CongDocument *doc)
{			
	gtk_label_set_text ( GTK_LABEL (glade_xml_get_widget (dialog_details->xml,"label_modified")), 
			     get_modified_string (doc) );
}

static void
set_dtd_info (GladeXML *xml,
	      const gchar *ExternalID,
	      const gchar *SystemID)
{
	g_assert (xml);

	if (NULL==ExternalID) {
		ExternalID=_("None");
	}
	if (NULL==SystemID) {
		SystemID=_("None");
	}
	
	gtk_label_set_text ( GTK_LABEL (glade_xml_get_widget (xml,"label_dtd_external_id")), 
			     ExternalID);
	gtk_label_set_text ( GTK_LABEL (glade_xml_get_widget (xml,"label_dtd_system_id")),
			     SystemID);
}

static void
refresh_dtd_stuff (CongFilePropertiesDialogDetails *dialog_details,
		   CongDocument *doc)
{
	xmlDocPtr xml_doc;
	CongDispspec* ds;

	GtkButton *button_dtd;
	GtkLabel *label_dtd_notes;

	gchar * text;

	g_assert (dialog_details);
	g_assert (dialog_details->xml);
	g_assert (IS_CONG_DOCUMENT (doc));

	xml_doc = cong_document_get_xml (doc);
	ds = cong_document_get_dispspec(doc);

	g_message ("refresh_dtd_stuff, extSubset=%p", xml_doc->extSubset);

	button_dtd = GTK_BUTTON (glade_xml_get_widget (dialog_details->xml,"button_dtd"));
	g_assert (button_dtd);

	label_dtd_notes = GTK_LABEL (glade_xml_get_widget (dialog_details->xml,"label_dtd_notes"));
	g_assert (label_dtd_notes);

	if (xml_doc->extSubset) {
		gtk_button_set_label (button_dtd,
				      _("_Remove this DTD"));
		
		set_dtd_info (dialog_details->xml,
			      xml_doc->extSubset->ExternalID,
			      xml_doc->extSubset->SystemID);	

		gtk_label_set_text ( label_dtd_notes, 
				     "");
	
	} else {
		const CongExternalDocumentModel* model_dtd = NULL;

		if (ds) {		
			model_dtd = cong_dispspec_get_external_document_model (ds,
									       CONG_DOCUMENT_MODE_TYPE_DTD);
		}
		
		if (model_dtd) {
			gtk_button_set_label (button_dtd,
					      _("_Associate this DTD"));
			set_dtd_info (dialog_details->xml,
				      cong_external_document_model_get_public_id (model_dtd),
				      cong_external_document_model_get_system_id (model_dtd));

			text = g_strdup_printf("<small>%s</small>",
					       _("The document does not specify an external DTD, but Conglomerate believes the above information is appropriate.  Click on \"Associate this DTD\" to specify this information explicitly in the document."));
					       
			gtk_label_set_markup ( label_dtd_notes, text );
			g_free(text);
		} else {
			set_dtd_info (dialog_details->xml,
				      "",
				      "");
			gtk_label_set_text ( label_dtd_notes,
					     _("No External Subset"));
			gtk_button_set_label (button_dtd,
					      _("_Add a DTD"));
		}
	}	
}

/**
 * cong_file_properties_dialog_new:
 * @doc:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget*
cong_file_properties_dialog_new (CongDocument *doc, 
				 GtkWindow *parent_window)
{
	xmlDocPtr xml_doc;
	CongDispspec* ds;
	GtkWidget *dialog;
#if 0
	GtkNotebook *notebook;
	CongDialogContent *basic_content;
	CongDialogCategory *general_category;
	CongDialogCategory *doctype_category;
	CongDialogContent *advanced_content;
	CongDialogCategory *header_category;
	CongDialogCategory *dtd_category;
#endif
	CongFilePropertiesDialogDetails *dialog_details;
#if 0
	struct _xmlDtd  *extSubset;     /* the document external subset */
	struct _xmlDtd  *intSubset;     /* the document internal subset */
#endif

	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);

	dialog_details = g_new0 (CongFilePropertiesDialogDetails,1);

	xml_doc = cong_document_get_xml(doc);
	ds = cong_document_get_dispspec(doc);

	dialog_details->doc = doc; 
	g_object_ref (G_OBJECT (doc));

	dialog_details->xml = cong_util_load_glade_file ("conglomerate/glade/cong-file-properties.glade",
							 NULL,
							 doc,
							 NULL);		

	dialog = glade_xml_get_widget(dialog_details->xml, "common_dialog");

	/* Filename & Location: */
	refresh_filename_and_location (dialog_details, doc);
	
	dialog_details->sigid_set_url =  g_signal_connect_after (G_OBJECT (doc),
								 "set_url",
								 G_CALLBACK (on_doc_set_url),
								 dialog_details);
	
	/* Modified: */
	{
		refresh_modified (dialog_details, doc);
	}
	
	dialog_details->sigid_end_edit =  g_signal_connect_after (G_OBJECT (doc),
								  "end_edit",
								  G_CALLBACK (on_doc_end_edit),
								  dialog_details);	
	/* Fields from dispspec: */
	{
		gtk_label_set_text ( GTK_LABEL (glade_xml_get_widget (dialog_details->xml,"label_typename")),
				     ds ? cong_dispspec_get_name (ds) : _("Unknown"));
		gtk_label_set_text ( GTK_LABEL (glade_xml_get_widget (dialog_details->xml,"label_typedesc")),
				     ds ? cong_dispspec_get_description (ds) : _("Unknown"));
	}
	
	/* XML Header: */
	{
		gtk_label_set_text ( GTK_LABEL (glade_xml_get_widget (dialog_details->xml,"label_xml_version")), 
				     xml_doc->version);
		
		{
			const gchar *encoding_text = xml_doc->encoding;
			if (NULL==encoding_text) {
				encoding_text = _("Unspecified");
			}
			gtk_label_set_text ( GTK_LABEL (glade_xml_get_widget (dialog_details->xml,"label_xml_encoding")), 
					     encoding_text);
		}
		
		gtk_label_set_text ( GTK_LABEL (glade_xml_get_widget (dialog_details->xml,"label_xml_standalone")), 
				     xml_doc->standalone?"yes":"no");
		/* FIXME: should this be localised? */
	}
	
	/* DTD Stuff: */
	{
		refresh_dtd_stuff (dialog_details, doc);
		
		g_signal_connect (G_OBJECT (glade_xml_get_widget (dialog_details->xml,"button_dtd")),
				  "clicked",
				  G_CALLBACK (on_dtd_button_clicked),
				  doc);
		
		dialog_details->sigid_set_dtd  = g_signal_connect_after (G_OBJECT (doc),
									 "set_dtd_ptr",
									 G_CALLBACK (on_doc_set_dtd_ptr),
									 dialog_details);
	}

	/* Add a cleanup hookup: */
	g_signal_connect (G_OBJECT (dialog),
			  "destroy",
			  G_CALLBACK (on_dialog_destroy),
			  dialog_details);

	gtk_widget_show_all(dialog);

	return dialog;
}

static gboolean
on_dialog_destroy (GtkWidget *widget,
		   gpointer user_data)
{
	CongFilePropertiesDialogDetails *dialog_details = (CongFilePropertiesDialogDetails*)user_data;

	g_signal_handler_disconnect (G_OBJECT (dialog_details->doc),
				     dialog_details->sigid_set_url);
	g_signal_handler_disconnect (G_OBJECT (dialog_details->doc),
				     dialog_details->sigid_end_edit);
	g_signal_handler_disconnect (G_OBJECT (dialog_details->doc),
				     dialog_details->sigid_set_dtd);

	g_object_unref (G_OBJECT (dialog_details->doc));

	g_object_unref (G_OBJECT (dialog_details->xml));

	g_free (dialog_details);

	return FALSE;
}

static void
on_dtd_button_clicked (GtkButton *button,
		       CongDocument *doc)
{
	/* Choices in this function mirror that in the function refresh_dtd_stuff: */
	xmlDocPtr xml_doc;
	CongDispspec* ds;

	g_assert (IS_CONG_DOCUMENT (doc));

	xml_doc = cong_document_get_xml (doc);
	ds = cong_document_get_dispspec(doc);

	if (xml_doc->extSubset) {
		/* Then button is "Remove this DTD": */
		CongCommand *cmd = cong_document_begin_command (doc,
								_("Remove DTD"),
								NULL);
		cong_command_add_set_external_dtd (cmd,
						   NULL,
						   NULL,
						   NULL);
		cong_document_end_command (doc,
					   cmd);
	} else {
		const CongExternalDocumentModel* model_dtd;
		
		model_dtd = cong_dispspec_get_external_document_model (ds,
								       CONG_DOCUMENT_MODE_TYPE_DTD);
		
		if (model_dtd) {
			/* Then button is "Associate this DTD": */
			CongCommand *cmd = cong_document_begin_command (doc,
									_("Associate with DTD"),
									NULL);
			cong_command_add_set_external_dtd (cmd,
							   cong_document_get_root(doc)->name,
							   cong_external_document_model_get_public_id (model_dtd),
							   cong_external_document_model_get_system_id (model_dtd));
			cong_document_end_command (doc,
						   cmd);
			
		} else {
			/* Then button is "Add a DTD": */
			cong_util_run_add_dtd_dialog (doc,
						      GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(button))));
		}
	}
}

static void
on_doc_end_edit (CongDocument *doc,
		 gpointer user_data)
{
	CongFilePropertiesDialogDetails *dialog_details = (CongFilePropertiesDialogDetails*)user_data;
	g_assert (IS_CONG_DOCUMENT (doc));

	refresh_modified (dialog_details, 
			  doc);
}


static void
on_doc_set_dtd_ptr (CongDocument *doc,
		    xmlDtdPtr dtd_ptr,
		    gpointer user_data)
{
	CongFilePropertiesDialogDetails *dialog_details = (CongFilePropertiesDialogDetails*)user_data;
	g_assert (IS_CONG_DOCUMENT (doc));

	refresh_dtd_stuff (dialog_details, 
			   doc);
}

static void
on_doc_set_url (CongDocument *doc,
		const gchar *new_url,
		gpointer user_data)
{
	CongFilePropertiesDialogDetails *dialog_details = (CongFilePropertiesDialogDetails*)user_data;
	g_assert (IS_CONG_DOCUMENT (doc));
	g_assert (new_url);

	refresh_filename_and_location (dialog_details,
				       doc);
}
