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

static void
on_remove_dtd_button_clicked (GtkButton *button,
			       CongDocument *doc);
static void
on_specify_dtd_button_clicked (GtkButton *button,
			       CongDocument *doc);

static void
add_dtd_info (CongDialogCategory *dtd_category,
	      const gchar *ExternalID,
	      const gchar *SystemID)
{
	g_assert (dtd_category);

	if (NULL==ExternalID) {
		ExternalID=_("None");
	}
	if (NULL==SystemID) {
		SystemID=_("None");
	}
	cong_dialog_category_add_field(dtd_category, _("External ID"), make_uneditable_text(ExternalID), FALSE);
	cong_dialog_category_add_field(dtd_category, _("System ID"), make_uneditable_text(SystemID), FALSE);
}


GtkWidget*
cong_file_properties_dialog_new (CongDocument *doc, 
				 GtkWindow *parent_window)
{
	xmlDocPtr xml_doc;
	CongDispspec* ds;
	GtkWidget *dialog;
	GtkNotebook *notebook;
	CongDialogContent *basic_content;
	CongDialogCategory *general_category;
	CongDialogCategory *doctype_category;
	CongDialogContent *advanced_content;
	CongDialogCategory *header_category;
	CongDialogCategory *dtd_category;
	gchar *filename, *path;

	struct _xmlDtd  *extSubset;     /* the document external subset */
	struct _xmlDtd  *intSubset;     /* the document internal subset */

	g_return_val_if_fail(doc, NULL);

	xml_doc = cong_document_get_xml(doc);
	ds = cong_document_get_dispspec(doc);

	dialog = gtk_dialog_new_with_buttons(_("Properties"),
					     parent_window,
					     0,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_ACCEPT,
					     NULL);

	gtk_container_set_border_width(GTK_CONTAINER(dialog), 12);

	/* Basic content: */
	basic_content = cong_dialog_content_new(TRUE);
	general_category = cong_dialog_content_add_category(basic_content, _("General"));
	doctype_category = cong_dialog_content_add_category(basic_content, _("Type"));

	filename = cong_document_get_filename(doc);
	path = cong_document_get_parent_uri(doc);

	cong_dialog_category_add_field(general_category, _("Name"), make_uneditable_text(filename), FALSE);
	cong_dialog_category_add_field(general_category, _("Location"), make_uneditable_text(path), FALSE);
	cong_dialog_category_add_field(general_category, _("Modified"), make_uneditable_text(cong_document_is_modified(doc)?"Yes":"No"), FALSE);

	cong_dialog_category_add_field(doctype_category, _("Name"), make_uneditable_text(cong_dispspec_get_name(ds)), FALSE);
	cong_dialog_category_add_field(doctype_category, _("Description"), make_uneditable_text(cong_dispspec_get_description(ds)), FALSE);

	g_free(filename);
	g_free(path);

	/* Advanced content: */
	advanced_content = cong_dialog_content_new(TRUE);
	header_category = cong_dialog_content_add_category(advanced_content, _("XML Header"));
	dtd_category = cong_dialog_content_add_category(advanced_content, _("Document Type Declaration"));

	cong_dialog_category_add_field(header_category, _("Version"), make_uneditable_text(xml_doc->version), FALSE);

	{
		const gchar *encoding_text = xml_doc->encoding;
		if (NULL==encoding_text) {
			encoding_text = _("Unspecified");
		}
		cong_dialog_category_add_field(header_category, _("Encoding"), make_uneditable_text(encoding_text), FALSE);	
	}
	cong_dialog_category_add_field(header_category, _("Standalone"), make_uneditable_text(xml_doc->standalone?"yes":"no"), FALSE);

	if (xml_doc->extSubset) {
		GtkWidget *remove_dtd_button = gtk_button_new_with_mnemonic (_("_Remove this DTD"));

		add_dtd_info (dtd_category,
			      xml_doc->extSubset->ExternalID,
			      xml_doc->extSubset->SystemID);

		cong_dialog_category_add_selflabelled_field (dtd_category, 
							     remove_dtd_button,
							     FALSE);			
		
		g_signal_connect (G_OBJECT (remove_dtd_button),
				  "clicked",
				  G_CALLBACK (on_remove_dtd_button_clicked),
				  doc);
		
	} else {
		const CongExternalDocumentModel* model_dtd;

		model_dtd = cong_dispspec_get_external_document_model (ds,
								       CONG_DOCUMENT_MODE_TYPE_DTD);
		
 		if (model_dtd) {
			GtkWidget *specify_dtd_button = gtk_button_new_with_mnemonic (_("_Associate this DTD"));
			GtkWidget *unspecified_label = gtk_label_new (_("The document does not specify an external DTD, but Conglomerate believes the following information is appropriate.  Click on \"Associate this DTD\" to specify this information explicitly in the document."));

			gtk_label_set_line_wrap (GTK_LABEL (unspecified_label),
						 TRUE);

			cong_dialog_category_add_selflabelled_field(dtd_category, unspecified_label, FALSE);

			add_dtd_info (dtd_category,
				      cong_external_document_model_get_public_id (model_dtd),
				      cong_external_document_model_get_system_id (model_dtd));

			cong_dialog_category_add_selflabelled_field (dtd_category, 
								     specify_dtd_button,
								     FALSE);			

			g_signal_connect (G_OBJECT (specify_dtd_button),
					  "clicked",
					  G_CALLBACK (on_specify_dtd_button_clicked),
					  doc);
					  
		} else {
			cong_dialog_category_add_selflabelled_field (dtd_category, 
								     gtk_label_new (_("No External Subset")),
								     FALSE);
		}
	}

	if (xml_doc->intSubset) {
	} else {
	}

#if 1
	notebook = GTK_NOTEBOOK(gtk_notebook_new());
	
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   GTK_WIDGET(notebook));

	gtk_notebook_append_page(notebook,
				 cong_dialog_content_get_widget(basic_content),
				 gtk_label_new(_("Basic")));
	gtk_notebook_append_page(notebook,
				 cong_dialog_content_get_widget(advanced_content),
				 gtk_label_new(_("Advanced")));
#else
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   cong_dialog_content_get_widget(content));
#endif

	g_signal_connect_swapped (G_OBJECT(dialog), 
				  "response", 
				  G_CALLBACK (gtk_widget_destroy),
				  GTK_OBJECT (dialog));

	gtk_widget_show_all(dialog);

	return dialog;
}

static void
on_remove_dtd_button_clicked (GtkButton *button,
			      CongDocument *doc)
{
	g_assert (IS_CONG_DOCUMENT (doc));

	{
		CongCommand *cmd = cong_document_begin_command (doc,
								_("Remove DTD"),
								NULL);
		cong_command_add_set_external_dtd (cmd,
						   NULL,
						   NULL,
						   NULL);
		cong_document_end_command (doc,
					   cmd);
	}
}

static void
on_specify_dtd_button_clicked (GtkButton *button,
			       CongDocument *doc)
{
	const CongExternalDocumentModel* model_dtd;
	CongDispspec* ds;

	g_assert (IS_CONG_DOCUMENT (doc));


	ds = cong_document_get_dispspec(doc);

	model_dtd = cong_dispspec_get_external_document_model (ds,
							       CONG_DOCUMENT_MODE_TYPE_DTD);
	
	g_assert (model_dtd);

	{
		CongCommand *cmd = cong_document_begin_command (doc,
								_("Associate with DTD"),
								NULL);
		cong_command_add_set_external_dtd (cmd,
						   cong_document_get_root(doc)->name,
						   cong_external_document_model_get_public_id (model_dtd),
						   cong_external_document_model_get_system_id (model_dtd));
		cong_document_end_command (doc,
					   cmd);
	}
}
