/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-docbook.c
 *
 * Plugin for DocBook support
 *
 * Copyright (C) 2002 David Malcolm
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
#include "cong-plugin.h"
#include "cong-error-dialog.h"
#include "fo.h"
#include "cong-progress-checklist.h"

#if 0
struct DocBookAuthorInfo
{
};

struct DocBookCreationInfo
{
	const gchar *
};
#endif

void factory_page_creation_callback_unified(CongDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
#if 0
	GnomeDruidPageStandard *which_settings_page;
	GnomeDruidPageStandard *create_new_settings_authorship_page;
	GnomeDruidPageStandard *create_new_settings_legal_page;
	GnomeDruidPageStandard *create_new_settings_name_the_settings_page;

	g_message("factory_page_creation_callback_unified <%s>", (char*)user_data);

	{
		GtkWidget *radio1, *radio2, *radio3, *box;
		which_settings_page = cong_new_file_assistant_new_page(assistant, 
								       factory, 
								       TRUE,
								       FALSE);
		box = gtk_vbox_new (TRUE, 2);
   
		radio1 = gtk_radio_button_new_with_label (NULL,
							  "None");   
   
		radio2 = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio1),
								      "Create and use new authorship information.");

		radio3 = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio1),
								      "Use existing authorship information.");

		gtk_widget_set_sensitive(GTK_WIDGET(radio3), FALSE); /* FIXME: this is unimplemented */
		
		/* Pack them into a box, then show all the widgets */
		gtk_box_pack_start (GTK_BOX (box), radio1, TRUE, TRUE, 2);
		gtk_box_pack_start (GTK_BOX (box), radio2, TRUE, TRUE, 2);
		gtk_box_pack_start (GTK_BOX (box), radio3, TRUE, TRUE, 2);
		gtk_widget_show_all (box);
		gnome_druid_page_standard_append_item(GNOME_DRUID_PAGE_STANDARD(which_settings_page),
						      "Conglomerate can automatically add \"metadata\" to the document \ni.e. information about the author and copyright status.\n\nWhat metadata would you like the document to contain?",
						      box,
						      user_data);
	}

	/* 
	   Authorship page to contain info on firstname, surname, optional affiliation and optional email 
	*/
	{
		create_new_settings_authorship_page = cong_new_file_assistant_new_page(assistant, 
										       factory, 
										       FALSE,
										       FALSE);
		
		gnome_druid_page_standard_append_item(GNOME_DRUID_PAGE_STANDARD(create_new_settings_authorship_page),
						      "authorship page",
						      gtk_calendar_new(),
						      user_data);
	}

	/* 
	   Legal page to have option to add a copyright of current year. with a holder string based on name from previous page.
	   Also, a choice of different licenses, and a button to preview the licenses.
	*/
	{
		create_new_settings_legal_page = cong_new_file_assistant_new_page(assistant, 
										  factory, 
										  FALSE,
										  FALSE);
		
		gnome_druid_page_standard_append_item(GNOME_DRUID_PAGE_STANDARD(create_new_settings_legal_page),
						      "Would you like Conglomerate to add a copyright notice to the doccument?",
						      gtk_calendar_new(),
						      user_data);
	}

	{
		create_new_settings_name_the_settings_page = cong_new_file_assistant_new_page(assistant, 
											      factory, 
											      FALSE,
											      TRUE);
		
		gnome_druid_page_standard_append_item(GNOME_DRUID_PAGE_STANDARD(create_new_settings_name_the_settings_page),
						      "If you wish, you may now name these settings so that you can reuse them at a later date (NOT YET IMPLEMENED)",
						      gtk_calendar_new(),
						      user_data);
	}
#endif
}

/**
   Manufactures appropriate DocBook DTD, and assigns it to the given document; doesn't add it to the node tree.
 */
static xmlDtdPtr make_docbook_declaration(xmlDocPtr xml_doc, const xmlChar *root_element)
{
	g_return_val_if_fail(xml_doc, NULL);
	g_return_val_if_fail(root_element, NULL);

	return xmlNewDtd(xml_doc,
			 root_element,
			 "-//OASIS//DTD DocBook XML V4.1.2//EN",
			 "http://www.oasis-open.org/docbook/xml/4.1.2/docbookx.dtd");
	
	/* An example of the desired output:
	   <!DOCTYPE article PUBLIC "-//OASIS//DTD DocBook XML V4.1.2//EN" 
	   "http://www.oasis-open.org/docbook/xml/4.1.2/docbookx.dtd" []>
	*/

}

/**
   Manufactures appropriate DocBook DTD, and assigns it to the given document, then adds it to the node tree (so it should show up when serialised)
 */
static xmlDtdPtr add_docbook_declaration(xmlDocPtr xml_doc, const xmlChar *root_element)
{
	xmlDtdPtr xml_dtd;

	g_return_val_if_fail(xml_doc, NULL);
	g_return_val_if_fail(root_element, NULL);

	xml_dtd = make_docbook_declaration(xml_doc,root_element);

	/* The following line appears to be the correct one to get the doctype declaration to appear in the node hierarchy and hence get serialised */
	xmlAddChild((xmlNodePtr)xml_doc,
		    (xmlNodePtr)xml_dtd);

	return xml_dtd;
}

xmlDocPtr make_article(const xmlChar *title)
{
	xmlDocPtr xml_doc;
	xmlNodePtr root_node;

	/* Build up the document and its content: */
	xml_doc = xmlNewDoc("1.0");

	add_docbook_declaration(xml_doc, "article");

	root_node = xmlNewDocNode(xml_doc,
				  NULL, /* xmlNsPtr ns, */
				  "article",
				  NULL);

	xmlDocSetRootElement(xml_doc,
			     root_node);

	xmlAddChild(root_node,
		    xmlNewDocNode(xml_doc,
				  NULL,
				  "title",
				  title)
		    );

	xmlAddChild(root_node, 
		    xmlNewDocNode(xml_doc,
				  NULL,
				  "para",
				  _("Text of the article goes here"))
		    );

	return xml_doc;
}

void factory_action_callback_article(CongDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
	xmlDocPtr xml_doc;

	xml_doc = make_article(_("Untitled Article"));

	cong_ui_new_document_from_manufactured_xml(xml_doc,
						   cong_new_file_assistant_get_toplevel(assistant));
}


xmlDocPtr make_book(const xmlChar *title)
{
	xmlDocPtr xml_doc;
	xmlNodePtr root_node;
	xmlNodePtr chapter_node;

	/* Build up the document and its content: */
	xml_doc = xmlNewDoc("1.0");

	add_docbook_declaration(xml_doc, "book");	

	root_node = xmlNewDocNode(xml_doc,
				  NULL, /* xmlNsPtr ns, */
				  "book",
				  NULL);

	xmlDocSetRootElement(xml_doc,
			     root_node);

	xmlAddChild(root_node,
		    xmlNewDocNode(xml_doc,
				  NULL,
				  "title",
				  title)
		    );

	chapter_node = xmlNewDocNode(xml_doc,
				     NULL,
				     "chapter",
				     "");
	xmlAddChild(root_node, chapter_node);

	xmlAddChild(chapter_node, 
		    xmlNewDocNode(xml_doc,
				  NULL,
				  "para",
				  _("Text of the first chapter goes here"))
		    );

	return xml_doc;
}

void factory_action_callback_book(CongDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
	xmlDocPtr xml_doc;

	xml_doc = make_book(_("Untitled Book"));

	cong_ui_new_document_from_manufactured_xml(xml_doc,
						   cong_new_file_assistant_get_toplevel(assistant));
}


xmlDocPtr make_set(const xmlChar *title)
{
	xmlDocPtr xml_doc;
	xmlNodePtr root_node;

	/* Build up the document and its content: */
	xml_doc = xmlNewDoc("1.0");

	add_docbook_declaration(xml_doc, "set");	

	root_node = xmlNewDocNode(xml_doc,
				  NULL, /* xmlNsPtr ns, */
				  "set",
				  NULL);

	xmlDocSetRootElement(xml_doc,
			     root_node);

	xmlAddChild(root_node,
		    xmlNewDocNode(xml_doc,
				  NULL,
				  "title",
				  title)
		    );

	return xml_doc;
}

void factory_action_callback_set(CongDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
	xmlDocPtr xml_doc;

	xml_doc = make_set(_("Untitled Set of Documents"));

	cong_ui_new_document_from_manufactured_xml(xml_doc,
						   cong_new_file_assistant_get_toplevel(assistant));
}

gboolean text_importer_mime_filter(CongImporter *importer, const gchar *mime_type, gpointer user_data)
{
	g_return_val_if_fail(importer, FALSE);
	g_return_val_if_fail(mime_type, FALSE);

	if (0==strcmp(mime_type,"text/plain")) {
		return TRUE;
	} else if (0==strcmp(mime_type,"text/x-readme")) {
		return TRUE;
	} else if (0==strcmp(mime_type,"application/octet-stream")) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void text_importer_action_callback(CongImporter *importer, const gchar *uri, const gchar *mime_type, gpointer user_data, GtkWindow *toplevel_window)
{
	char* buffer;
	GnomeVFSFileSize size;
	xmlDocPtr xml_doc;

	g_message("text_importer_action_callback");

	if (cong_ui_load_imported_file_content(uri, &buffer, &size)) {
		xmlNodePtr root_node;

		g_assert(buffer);

		/* Build up the document and its content: */
		xml_doc = xmlNewDoc("1.0");
			
		root_node = xmlNewDocNode(xml_doc,
					  NULL, /* xmlNsPtr ns, */
					  "article",
					  buffer);

		xmlDocSetRootElement(xml_doc,
				     root_node);

		/* Finished building content: */
		g_free(buffer);

		/* Do appropriate UI stuff: */
		cong_ui_new_document_from_imported_xml(xml_doc,
						       toplevel_window);
	}
}

gboolean sourcecode_importer_mime_filter(CongImporter *importer, const gchar *mime_type, gpointer user_data)
{
	g_return_val_if_fail(importer, FALSE);
	g_return_val_if_fail(mime_type, FALSE);

	if (0==strcmp(mime_type,"text/x-c-header")) {
		return TRUE;
	} else if (0==strcmp(mime_type,"text/x-c")) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void sourcecode_importer_action_callback(CongImporter *importer, const gchar *uri, const gchar *mime_type, gpointer user_data, GtkWindow *toplevel_window)
{
	char* buffer;
	GnomeVFSFileSize size;
	xmlDocPtr xml_doc;
	xmlNodePtr root_node;

	g_message("sourcecode_importer_action_callback");

	if (cong_ui_load_imported_file_content(uri, &buffer, &size)) {
		g_assert(buffer);

		/* Build up the document and its content: */
		xml_doc = xmlNewDoc("1.0");
			
		root_node = xmlNewDocNode(xml_doc,
					  NULL, /* xmlNsPtr ns, */
					  "article",
					  NULL);

		xmlDocSetRootElement(xml_doc,
				     root_node);

		xmlAddChild(root_node,
			    xmlNewDocNode(xml_doc,
					  NULL,
					  "title",
					  uri)
			    );

		xmlAddChild(root_node,
			    xmlNewDocNode(xml_doc,
					  NULL,
					  "programlisting",
					  buffer)
			    );

		/* Finished building content: */
		g_free(buffer);

		/* Do appropriate UI stuff: */
		cong_ui_new_document_from_imported_xml(xml_doc,
						       toplevel_window);
	}
}

gboolean docbook_exporter_fpi_filter(CongExporter *exporter, const gchar *fpi, gpointer user_data)
{
	g_return_val_if_fail(exporter, FALSE);
	g_return_val_if_fail(fpi, FALSE);

	return TRUE; /* for now */
}

void html_exporter_action_callback(CongExporter *exporter, CongDocument *doc, const gchar *uri, gpointer user_data, GtkWindow *toplevel_window)
{
	gchar *stylesheet_path;

	g_return_if_fail(exporter);
	g_return_if_fail(doc);
	g_return_if_fail(uri);

	g_message("html_exporter_action_callback");
	
	stylesheet_path = cong_utils_get_norman_walsh_stylesheet("html/docbook.xsl");
	g_assert(stylesheet_path);

	cong_ui_transform_doc_to_uri(doc,
				     stylesheet_path,
				     uri,
				     toplevel_window);

	g_free(stylesheet_path);
}

void pdf_exporter_action_callback(CongExporter *exporter, CongDocument *doc, const gchar *uri, gpointer user_data, GtkWindow *toplevel_window)
{
#if 1
	GtkWidget *progress_checklist_dialog;
	CongProgressChecklist *progress_checklist;
	xmlDocPtr fo_doc;
	gchar *stylesheet_path;

	g_message("pdf_exporter_action_callback");

	g_return_if_fail(exporter);
	g_return_if_fail(doc);
	g_return_if_fail(uri);

	stylesheet_path = cong_utils_get_norman_walsh_stylesheet("fo/docbook.xsl");
	g_assert(stylesheet_path);

	progress_checklist_dialog = cong_progress_checklist_dialog_new(_("Exporting PDF file"), toplevel_window);
	progress_checklist = cong_progress_checklist_dialog_get_progress_checklist(CONG_PROGRESS_CHECKLIST_DIALOG(progress_checklist_dialog));

	cong_progress_checklist_add_stage(progress_checklist,
					  _("Transforming DocBook into XSL Formatting Objects"));
	cong_progress_checklist_add_stage(progress_checklist,
					  _("Laying out XSL Formatting Objects as a PDF file"));

	gtk_widget_show(progress_checklist_dialog);

	fo_doc = cong_ui_transform_doc(doc,
				       stylesheet_path,
				       toplevel_window);

	g_free(stylesheet_path);

	if (fo_doc) {
		cong_progress_checklist_complete_stage(progress_checklist);

		CONG_DO_UNIMPLEMENTED_DIALOG_WITH_BUGZILLA_ID(toplevel_window, _("Converting XSL Formatting Objects to PDF"), 108467);
		/* FIXME: ultimately we probably want to use xmlroff to do this stage */
		
		xmlFreeDoc(fo_doc);
	}


	gtk_widget_destroy(progress_checklist_dialog);
#else
	g_message("pdf_exporter_action_callback");

	CONG_DO_UNIMPLEMENTED_DIALOG(toplevel_window, "Exporting DocBook as PDF");	
#endif
}

void fo_exporter_action_callback(CongExporter *exporter, CongDocument *doc, const gchar *uri, gpointer user_data, GtkWindow *toplevel_window)
{
	gchar *stylesheet_path;

	g_return_if_fail(exporter);
	g_return_if_fail(doc);
	g_return_if_fail(uri);

	g_message("fo_exporter_action_callback");

	stylesheet_path = cong_utils_get_norman_walsh_stylesheet("fo/docbook.xsl");
	g_assert(stylesheet_path);

	cong_ui_transform_doc_to_uri(doc,
				     stylesheet_path,
				     uri,
				     toplevel_window);

	g_free(stylesheet_path);
}

#if ENABLE_PRINTING
gboolean docbook_print_method_fpi_filter(CongPrintMethod *print_method, const gchar *fpi, gpointer user_data)
{
	g_return_val_if_fail(print_method, FALSE);
	g_return_val_if_fail(fpi, FALSE);

	return TRUE; /* for now */
}

void docbook_print_method_action_callback(CongPrintMethod *print_method, CongDocument *doc, GnomePrintContext *gpc, gpointer user_data, GtkWindow *toplevel_window)
{
	GtkWidget *progress_checklist_dialog;
	CongProgressChecklist *progress_checklist;
	xmlDocPtr fo_doc;

	g_message("docbook_print_method_action_callback");

	progress_checklist_dialog = cong_progress_checklist_dialog_new(_("Printing DocBook file"), toplevel_window);
	progress_checklist = cong_progress_checklist_dialog_get_progress_checklist(CONG_PROGRESS_CHECKLIST_DIALOG(progress_checklist_dialog));

	cong_progress_checklist_add_stage(progress_checklist,
					  _("Transforming DocBook into XSL Formatting Objects"));
	cong_progress_checklist_add_stage(progress_checklist,
					  _("Printing XSL Formatting Objects"));

	gtk_widget_show(progress_checklist_dialog);

	{
		gchar *stylesheet_path = cong_utils_get_norman_walsh_stylesheet("fo/docbook.xsl");
		g_assert(stylesheet_path);

		fo_doc = cong_ui_transform_doc(doc,
					       stylesheet_path,
					       toplevel_window);

		g_free(stylesheet_path);
	}

	if (fo_doc) {
		cong_progress_checklist_complete_stage(progress_checklist);

		CONG_DO_UNIMPLEMENTED_DIALOG_WITH_BUGZILLA_ID(toplevel_window, _("Printing XSL Formatting Objects"), 108468);

#if 0
		/* FIXME: ultimately we probably want to use xmlroff to do this stage */
		{
			FoPrintContext *fpc;
			FoParserResult *parser_result;
			FoSolverResult *solver_result;

			fpc = fo_print_context_new_from_gnome_print(gpc);

			parser_result = fo_parser_result_new_from_xmldoc(fo_doc);
			
			if (parser_result) {
				
#if 1
				/* View solver result: */
				solver_result = fo_solver_result_new_from_parser_result(parser_result);
				
				if (solver_result) {
					fo_solver_result_render(solver_result, fpc);
					
					fo_solver_result_delete(solver_result);
				}
#else
				/* View parser result: */
				fo_parser_result_test_render(parser_result, fpc);
#endif
				
				fo_parser_result_delete(parser_result);
				
			}

			fo_print_context_delete(fpc);
		}
#else
		/* Some test code: */
		{
			GnomeFont *font;
			font = gnome_font_find_closest ("Helvetica", 12);
			
			gnome_print_beginpage (gpc, "1");
			
			gnome_print_setfont (gpc, font);
			gnome_print_moveto (gpc, 100, 400);
			gnome_print_show (gpc, _("This will eventually be the text from DocBook print method"));
			
			gnome_print_moveto (gpc, 100, 200);
			gnome_print_lineto (gpc, 200, 200);
			gnome_print_stroke (gpc);
			
			gnome_print_showpage (gpc);
		}
#endif
		
		xmlFreeDoc(fo_doc);
	}


	gtk_widget_destroy(progress_checklist_dialog);


}
#endif /* #if ENABLE_PRINTING */

 /* would be exposed as "plugin_register"? */
gboolean plugin_docbook_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);
	
	cong_plugin_register_document_factory(plugin, 
					      _("DocBook Article"), 
					      _("Create an article, perhaps for a website or a magazine, using the \"DocBook\" format"),
					      "docbook-article-factory",
					      factory_page_creation_callback_unified,
					      factory_action_callback_article,
					      "cong-docbook-article",
					      "article");
	cong_plugin_register_document_factory(plugin, 
					      _("DocBook Book"), 
					      _("Create a book, using the \"DocBook\" format"),
					      "docbook-book-factory",
					      factory_page_creation_callback_unified,
					      factory_action_callback_book,
					      "cong-docbook-book",
					      "book");
	cong_plugin_register_document_factory(plugin, 
					      _("DocBook Set"), 
					      _("Create a set of related books, using the \"DocBook\" format"),
					      "docbook-set-factory",
					      factory_page_creation_callback_unified,
					      factory_action_callback_set,
					      "cong-docbook-set",
					      "set");	

	cong_plugin_register_importer(plugin, 
				      _("Import text as a DocBook article"), 
				      _("Import a plain text file into the \"DocBook\" format, as an article."),
				      "docbook-plaintext-import",
				      text_importer_mime_filter,
				      text_importer_action_callback,
				      NULL);

#if 0
	cong_plugin_register_importer(plugin, 
				      "Import program code as a DocBook article", 
				      "Import program source code into the \"DocBook\" format, as an article.",
				      "docbook-sourcecode-import",
				      sourcecode_importer_mime_filter,
				      sourcecode_importer_action_callback,
				      NULL);
#endif

	cong_plugin_register_exporter(plugin, 
				      _("Export DocBook as HTML"), 
				      "",
				      "docbook-HTML-export",
				      docbook_exporter_fpi_filter,
				      html_exporter_action_callback,
				      NULL);

	cong_plugin_register_exporter(plugin, 
				      _("Export DocBook as PDF"), 
				      "",
				      "docbook-PDF-export",
				      docbook_exporter_fpi_filter,
				      pdf_exporter_action_callback,
				      NULL);

	cong_plugin_register_exporter(plugin, 
				      _("Export DocBook as XSL:FO"),
				      "",
				      "docbook-XSLFO-export",
				      docbook_exporter_fpi_filter,
				      fo_exporter_action_callback,
				      NULL);

#if ENABLE_PRINTING
	cong_plugin_register_print_method(plugin, 
					  _("Print DocBook"),
					  "",
					  "docbook-print",
					  docbook_print_method_fpi_filter,
					  docbook_print_method_action_callback,
					  NULL);
#endif

	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean plugin_docbook_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
