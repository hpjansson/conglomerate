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
#include "cong-progress-checklist.h"
#include "cong-document.h"
#include "cong-app.h"
#include "cong-util.h"
#include "cong-fake-plugin-hooks.h"
#include "cong-attribute-editor.h"

/* Splits input UTF8 into a GList of nul-terminated GUnichar strings */
static GList*
split_utf8_into_unichar_lines (const gchar *utf8_input,
			       GnomeVFSFileSize size);

static void
parse_text_buffer_into_docbook (CongDocument *doc,
				CongNodePtr root_node, 
				const char* buffer,
				GnomeVFSFileSize size);

#if 0
struct DocBookAuthorInfo
{
};

struct DocBookCreationInfo
{
	const gchar *
};
#endif

void factory_page_creation_callback_unified(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
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
   Manufactures appropriate DocBook DTD, and assigns it to the given document, then adds it to the node tree (so it should show up when serialised)
 */
static xmlDtdPtr add_docbook_declaration(xmlDocPtr xml_doc, const xmlChar *root_element)
{
	xmlDtdPtr xml_dtd;

	g_return_val_if_fail(xml_doc, NULL);
	g_return_val_if_fail(root_element, NULL);

	xml_dtd = cong_util_add_external_dtd (xml_doc, 
					      root_element,
					      "-//OASIS//DTD DocBook XML V4.1.2//EN",
					      "http://www.oasis-open.org/docbook/xml/4.1.2/docbookx.dtd");

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

void factory_action_callback_article(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
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

void factory_action_callback_book(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
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

void factory_action_callback_set(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
	xmlDocPtr xml_doc;

	xml_doc = make_set(_("Untitled Set of Documents"));

	cong_ui_new_document_from_manufactured_xml(xml_doc,
						   cong_new_file_assistant_get_toplevel(assistant));
}

gboolean text_importer_mime_filter(CongServiceImporter *importer, const gchar *mime_type, gpointer user_data)
{
	g_return_val_if_fail(importer, FALSE);
	g_return_val_if_fail(mime_type, FALSE);

	if (0==strcmp(mime_type,"text/plain")) {
		return TRUE;
	} else if (0==strcmp(mime_type,"text/x-readme")) {
		return TRUE;
	} else if (0==strcmp(mime_type,"text/x-install")) {
		return TRUE;
	} else if (0==strcmp(mime_type,"text/x-copying")) {
		return TRUE;
	} else if (0==strcmp(mime_type,"application/octet-stream")) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/* Internal stuff for the text importer: */
gboolean
cong_unichar_is_line_break(gunichar c)
{
	switch (g_unichar_break_type (c)) {
	default: return FALSE;

	case G_UNICODE_BREAK_CARRIAGE_RETURN:
	case G_UNICODE_BREAK_LINE_FEED:
		return TRUE;
	}
}

static gunichar*
make_unichar_line (gunichar *start, 
		   gunichar *beyond_end)
{
	gunichar *duplicate;

	g_return_val_if_fail (start, NULL);
	g_return_val_if_fail (beyond_end, NULL);
	g_return_val_if_fail (beyond_end>=start, NULL);

	duplicate = g_memdup (start, ((beyond_end-start)+1)*sizeof(gunichar));

	duplicate[(beyond_end-start)] = 0;

	return duplicate;
}

static void
append_line (GList **result,
	     gunichar *start, 
	     gunichar *beyond_end)
{
	gunichar* line = make_unichar_line (start, 
					    beyond_end);
	
	if (line) {
		*result = g_list_append (*result, line);
	}	
}

static GList*
split_utf8_into_unichar_lines (const gchar *utf8_input,
			       GnomeVFSFileSize size)
{
	GList *result = NULL;
	gunichar* ucs4_full_string;
	gunichar* line_start;
	gunichar* iter;

	g_return_val_if_fail (utf8_input, NULL);

	ucs4_full_string = g_utf8_to_ucs4_fast (utf8_input,
						size,
						NULL);
	
	line_start = ucs4_full_string;

	for (iter=ucs4_full_string; *iter; iter++) {
		if (cong_unichar_is_line_break(*iter)) {
			append_line (&result, line_start, iter);
			line_start = iter+1;
		}
	}

	append_line (&result, line_start, iter-1);
	
	g_free (ucs4_full_string);

	return result;
}

static gboolean
cong_ucs4_is_bold_caps (gunichar *ucs4_input)
{
	g_return_val_if_fail (ucs4_input, FALSE);

	if (0 == *ucs4_input) {
		return FALSE;
	}

	while (*ucs4_input) {
		if (g_unichar_islower (*ucs4_input)) {
			return FALSE;
		}

		ucs4_input++;
	}

	return TRUE;
}

static gboolean
cong_unichar_is_underline (gunichar c)
{
	switch (c) {
	default: return FALSE;
	case '-': return TRUE;
	case '=': return TRUE;
	case '_': return TRUE;
	}
}

static gboolean
cong_ucs4_is_underline (gunichar *ucs4_input)
{
	g_return_val_if_fail (ucs4_input, FALSE);

	if (0 == *ucs4_input) {
		return FALSE;
	}

	while (*ucs4_input) {
		if (!cong_unichar_is_underline (*ucs4_input)) {
			return FALSE;
		}

		ucs4_input++;
	}

	return TRUE;
}

static gboolean
cong_ucs4_is_list_item (gunichar *ucs4_input, 
			gunichar **output_text)
{
	/* For now, detect "- ": */
	if ('-'==ucs4_input[0]) {
		if (' '==ucs4_input[1]) {
			gunichar *iter = &ucs4_input[2];
			while (0!=*iter) {
				iter++;				
			}

			if (output_text) {
				*output_text = g_memdup (&ucs4_input[2], sizeof(gunichar)*(1+(iter-&ucs4_input[2])));
			}
			
			return TRUE;
		}
	}
	return FALSE;
}

gboolean
should_merge (gunichar *ucs4_input_1,
	      gunichar *ucs4_input_2)
{
	if (0==ucs4_input_2[0]) {
		return FALSE;
	}

	if (cong_ucs4_is_bold_caps (ucs4_input_1)) {
		return cong_ucs4_is_bold_caps (ucs4_input_2);
	} else {
		if (cong_ucs4_is_bold_caps (ucs4_input_2)) {
			return FALSE;
		}
	}

	if (cong_ucs4_is_list_item (ucs4_input_2, NULL)) {
		return FALSE;
	}

	return TRUE;
}

gunichar*
cong_ucs4_concat (gunichar *ucs4_input_1,
		  gunichar *ucs4_input_2)
{
	gchar *utf8_input_1 = g_ucs4_to_utf8 (ucs4_input_1,
					      -1,
					      NULL,
					      NULL,
					      NULL);
	gchar *utf8_input_2 = g_ucs4_to_utf8 (ucs4_input_2,
					      -1,
					      NULL,
					      NULL,
					      NULL);
	gchar *utf8_result = g_strdup_printf("%s %s", utf8_input_1, utf8_input_2);

	gunichar *ucs4_result = g_utf8_to_ucs4_fast (utf8_result,
						     -1,
						     NULL);
	g_assert (ucs4_result);
	
	g_free (utf8_input_1);
	g_free (utf8_input_2);
	g_free (utf8_result);

	return ucs4_result;
}


GList*
merge_lines (GList *list)
{
#if 0	
	GList *iter;
	GList *next;

	for (iter=list;iter;iter=next) {
		gunichar *string = iter->data;

		g_assert (string);

		next = iter->next;

		if (0==string[0]) {
			/* Delete empty lines: */
			g_free (iter->data);
			list = g_list_delete_link (list,
						   iter->next);

			
		} else {
			/* Merge with next line, if both are non-empty: */
			if (next) {
				g_assert (next->data);
				if (should_merge(iter->data, next->data)) {
					
					gunichar *new_string = cong_ucs4_concat (iter->data, next->data);
					g_free (iter->data);
					iter->data = new_string;

					g_assert (next==iter->next);
					next = next->next;

					g_free (iter->next->data);
					list = g_list_delete_link (list,
								   iter->next);
				}
			}
		}
	}	
#endif
	return list;
}

#if 0
/**
   Traverse document, looking for <para> tags containing empty text.

   Go back and merge the 
 */
void
cong_util_merge_paras (CongDocument *doc)
{
	cong_document_begin_edit (doc);
	cong_document_end_edit (doc);
}
#endif

static void
parse_text_buffer_into_docbook (CongDocument *doc,
				CongNodePtr root_node, 
				const char* buffer,
				GnomeVFSFileSize size)
{
	/* Note: this routine uses the private CongDocument methods which are normally reserved for the undo/redo system.
	   It is safe here, since we are not interacting with that - the document has only just been created. */
	g_return_if_fail (doc);
	g_return_if_fail (root_node);
	g_return_if_fail (buffer);

	cong_document_begin_edit (doc);

	#if 1
	/* Split buffer into lines; add lines individually as paras, with some heuristics: */
	{
		GList* list = split_utf8_into_unichar_lines (buffer,
							     size);
		GList *iter;
		CongNodePtr current_sect = NULL;
		CongNodePtr current_list = NULL;

		list = merge_lines (list);
		
		for (iter=list;iter;iter=iter->next) {
			gboolean is_heading = FALSE;
			gunichar *ucs4_listitem_text = NULL;

			g_assert (iter->data);
			
			/* Individual fully bold lines are probably headings: */
			if (cong_ucs4_is_bold_caps (iter->data) ) {
				if (iter->next) {
					if (!cong_ucs4_is_bold_caps (iter->next->data) ) {
						is_heading = TRUE;
					}					
				}
			} 

			/* Detect if the next line is merely an underline of some kind: */
			if (iter->next) {
				
				g_assert (iter->next->data);
				if (cong_ucs4_is_underline (iter->next->data)) {
					is_heading = TRUE;
					
					list = g_list_delete_link (list,
								   iter->next);
				}
			}

			/* Reject blanks lines here: */
			if (0==((gunichar*)iter->data)[0]) {
				continue;
			}
			
			/* Attempt to merge with the next line: */
			while (iter->next) {
				if (should_merge (iter->data, iter->next->data)) {
					gunichar *new_string = cong_ucs4_concat (iter->data, iter->next->data);
					g_free (iter->data);
					iter->data = new_string;
					
					g_free (iter->next->data);
					list = g_list_delete_link (list,
								   iter->next);
				} else {
					break;
				}
			}

				

			if (cong_ucs4_is_list_item (iter->data,
						    &ucs4_listitem_text) ) {
				CongNodePtr listitem;
				CongNodePtr para;
				CongNodePtr text_node;


				gchar *utf8_listitem_text = g_ucs4_to_utf8 (ucs4_listitem_text,
									    -1,
									    NULL,
									    NULL,
									    NULL);
				
				text_node = cong_node_new_text (utf8_listitem_text, 
								doc);
				
				g_free (utf8_listitem_text);
				g_free (ucs4_listitem_text);


				if (NULL==current_list) {
					/* Create a <itemizedlist> below the root/currect sect: */
					current_list = cong_node_new_element(NULL, "itemizedlist", doc);
					
					
					if (current_sect) {
						cong_document_private_node_set_parent (doc, 
									       current_list,
									       current_sect);
					} else {
						cong_document_private_node_set_parent (doc, 
									       current_list, 
									       root_node);
					}
				}

				listitem = cong_node_new_element(NULL, "listitem", doc);

				para = cong_node_new_element(NULL, "para", doc);
					
				cong_document_private_node_set_parent (doc, 
							       listitem,
							       current_list);

				cong_document_private_node_set_parent (doc, 							       
							       para,
							       listitem);

				cong_document_private_node_set_parent (doc,
							       text_node,
 							       para);
			} else {
				CongNodePtr text_node;

				gchar *utf8_line;

				utf8_line = g_ucs4_to_utf8 (iter->data,
							    -1,
							    NULL,
							    NULL,
							    NULL);
				
				text_node = cong_node_new_text (utf8_line, 
								doc);
				
				g_free (utf8_line);

				current_list = NULL;

				if (is_heading) {
					CongNodePtr title;
					
					/* Create a <sect1> below the root and add as a title */
					current_sect = cong_node_new_element(NULL, "sect1", doc);
					
					title = cong_node_new_element(NULL, "title", doc);
					
					cong_document_private_node_set_parent (doc, 
								       current_sect, 
								       root_node);
					
					cong_document_private_node_set_parent (doc, 
								       title,
								       current_sect);
					
					cong_document_private_node_set_parent (doc, 							       
								       text_node,
								       title);
				} else {
					/* FIXME: spot text of the form: "fubar: more text"; turn it into a <formalpara> tag instead */
					CongNodePtr para_node;
					
					/* Create a <para>; either below the currect <sect1> or below the root */
					para_node = cong_node_new_element(NULL, "para", doc);				
					
					if (current_sect) {
						cong_document_private_node_set_parent (doc, 
									       para_node, 
									       current_sect);
					} else {
						cong_document_private_node_set_parent (doc, 
									       para_node, 
									       root_node);
					}
					cong_document_private_node_set_parent (doc, 
								       text_node, 
								       para_node);
				}
			}
		}

		/* FIXME: this leaks the lines */
	}
	#else
	/* Add raw to doc: */
	{
		CongNodePtr text_node = cong_node_new_text (buffer, 
							    doc);
		
		cong_document_private_node_set_parent (doc, 
					       text_node, 
					       root_node);
	}
	#endif

	cong_document_end_edit (doc);
}

void text_importer_action_callback(CongServiceImporter *importer, const gchar *uri, const gchar *mime_type, gpointer user_data, GtkWindow *toplevel_window)
{
	char* buffer;
	GnomeVFSFileSize size;
	xmlDocPtr xml_doc;

	g_message("text_importer_action_callback");

	if (cong_ui_load_imported_file_content(uri, &buffer, &size, toplevel_window)) {
		CongDocument *doc;
		xmlNodePtr root_node;

		g_assert(buffer);

		/* Build up the document and its content: */
		xml_doc = xmlNewDoc("1.0");

		add_docbook_declaration(xml_doc, "article");
			
		root_node = xmlNewDocNode(xml_doc,
					  NULL, /* xmlNsPtr ns, */
					  "article",
					  NULL);

		xmlDocSetRootElement(xml_doc,
				     root_node);

		/* Do appropriate UI stuff: */
		doc = cong_ui_new_document_from_imported_xml(xml_doc,
							     toplevel_window);

		if (doc) {
			parse_text_buffer_into_docbook (doc,
							root_node, 
							buffer,
							size);
		}

		/* Finished building content: */
		g_free(buffer);

	}
}

gboolean sourcecode_importer_mime_filter(CongServiceImporter *importer, const gchar *mime_type, gpointer user_data)
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

void sourcecode_importer_action_callback(CongServiceImporter *importer, const gchar *uri, const gchar *mime_type, gpointer user_data, GtkWindow *toplevel_window)
{
	char* buffer;
	GnomeVFSFileSize size;
	xmlDocPtr xml_doc;
	xmlNodePtr root_node;

	g_message("sourcecode_importer_action_callback");

	if (cong_ui_load_imported_file_content(uri, &buffer, &size, toplevel_window)) {
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

gboolean docbook_exporter_document_filter(CongServiceExporter *exporter, CongDocument *doc, gpointer user_data)
{
	g_return_val_if_fail(exporter, FALSE);
	g_return_val_if_fail(doc, FALSE);

	return cong_util_is_docbook(doc);
}

void html_exporter_action_callback(CongServiceExporter *exporter, CongDocument *doc, const gchar *uri, gpointer user_data, GtkWindow *toplevel_window)
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

void pdf_exporter_action_callback(CongServiceExporter *exporter, CongDocument *doc, const gchar *uri, gpointer user_data, GtkWindow *toplevel_window)
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

void fo_exporter_action_callback(CongServiceExporter *exporter, CongDocument *doc, const gchar *uri, gpointer user_data, GtkWindow *toplevel_window)
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

#if (ENABLE_PRINTING &&ENABLE_LIBFO)
gboolean 
docbook_print_method_document_filter (CongServicePrintMethod *print_method, 
				      CongDocument *doc, 
				      gpointer user_data)
{
	g_return_val_if_fail(print_method, FALSE);
	g_return_val_if_fail(doc, FALSE);

	return cong_util_is_docbook(doc);
}

void 
docbook_print_method_action_callback (CongServicePrintMethod *print_method, 
				      CongDocument *doc, 
				      GnomePrintContext *gpc, 
				      gpointer user_data, 
				      GtkWindow *toplevel_window)
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

		cong_util_print_xslfo(toplevel_window, 
				      gpc, 
				      fo_doc);

		xmlFreeDoc(fo_doc);
	}


	gtk_widget_destroy(progress_checklist_dialog);


}
#endif /* #if ENABLE_PRINTING */

GtkWidget* docbook_generic_node_factory_method(CongServiceNodePropertyDialog *custom_property_dialog, CongDocument *doc, CongNodePtr node)
{
	gchar* glade_filename;
	GladeXML *xml;
	GtkWidget *notebook1;

	g_message("docbook_generic_node_factory_method");

	g_return_val_if_fail(custom_property_dialog, NULL);
	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);

	glade_filename = gnome_program_locate_file (cong_app_get_gnome_program (cong_app_singleton()),
						    GNOME_FILE_DOMAIN_APP_DATADIR,
						    "glade/docbook-common-properties.glade",
						    FALSE,
						    NULL);

	global_glade_doc_ptr = doc;
	global_glade_node_ptr = node;

	xml = glade_xml_new(glade_filename, NULL, NULL);
	glade_xml_signal_autoconnect(xml);

	global_glade_doc_ptr = NULL;
	global_glade_node_ptr = NULL;

	/* FIXME: wire stuff up! */
	
	g_free(glade_filename);

	/* Add the advanced properties tab: */
	notebook1 = glade_xml_get_widget(xml, "notebook1");
	cong_ui_append_advanced_node_properties_page(GTK_NOTEBOOK(notebook1),
						     doc, 
						     node);
	
	return glade_xml_get_widget(xml, "common_dialog");
}

GtkWidget* docbook_orderedlist_properties_factory_method(CongServiceNodePropertyDialog *custom_property_dialog, CongDocument *doc, CongNodePtr node)
{
	gchar* glade_filename;
	GladeXML *xml;
	GtkWidget *notebook1;

	g_message("docbook_orderedlist_properties_factory_method");

	g_return_val_if_fail(custom_property_dialog, NULL);
	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);

	glade_filename = gnome_program_locate_file (cong_app_get_gnome_program (cong_app_singleton()),
						    GNOME_FILE_DOMAIN_APP_DATADIR,
						    "glade/docbook-orderedlist-properties.glade",
						    FALSE,
						    NULL);

	global_glade_doc_ptr = doc;
	global_glade_node_ptr = node;

	xml = glade_xml_new(glade_filename, NULL, NULL);
	glade_xml_signal_autoconnect(xml);

	global_glade_doc_ptr = NULL;
	global_glade_node_ptr = NULL;

	g_free(glade_filename);

	/* Wire stuff up: */
	{
		/* The numeration radio buttons: */
		cong_bind_radio_button (GTK_RADIO_BUTTON (glade_xml_get_widget(xml, "arabic")),
					doc,
					node,
					NULL,
					"numeration",
					"arabic");
		cong_bind_radio_button (GTK_RADIO_BUTTON (glade_xml_get_widget(xml, "loweralpha")),
					doc,
					node,
					NULL,
					"numeration",
					"loweralpha");
		cong_bind_radio_button (GTK_RADIO_BUTTON (glade_xml_get_widget(xml, "lowerroman")),
					doc,
					node,
					NULL,
					"numeration",
					"lowerroman");
		cong_bind_radio_button (GTK_RADIO_BUTTON (glade_xml_get_widget(xml, "upperalpha")),
					doc,
					node,
					NULL,
					"numeration",
					"upperalpha");
		cong_bind_radio_button (GTK_RADIO_BUTTON (glade_xml_get_widget(xml, "upperroman")),
					doc,
					node,
					NULL,
					"numeration",
					"upperroman");
		
		/* The checkboxes: */
		cong_bind_check_button (GTK_CHECK_BUTTON (glade_xml_get_widget(xml, "inheritnum")),
					doc,
					node,
					NULL,
					"inheritnum",
					"ignore",
					"inherit");
		cong_bind_check_button (GTK_CHECK_BUTTON (glade_xml_get_widget(xml, "spacing")),
					doc,
					node,
					NULL,
					"spacing",
					"normal",
					"compact");
		cong_bind_check_button (GTK_CHECK_BUTTON (glade_xml_get_widget(xml, "continuation")),
					doc,
					node,
					NULL,
					"continuation",
					"restart",
					"continues");
		
	}
	
	/* Add the advanced properties tab: */
	notebook1 = glade_xml_get_widget(xml, "notebook1");
	cong_ui_append_advanced_node_properties_page(GTK_NOTEBOOK(notebook1),
						     doc, 
						     node);
	
	return glade_xml_get_widget(xml, "common_dialog");
}

static void
open_ulink_in_browser (CongNodePtr node)
{
	gchar *url = cong_node_get_attribute (node, NULL, "url");

	if (url) {	
		/* FIXME: should we have some error handling? */
		gnome_url_show (url,
				NULL);
	}

	g_free (url);
}

static void
on_test_link_pressed (GtkButton *button,
		      gpointer user_data)
{
	CongNodePtr node = user_data;

	open_ulink_in_browser (node);
}

GtkWidget* docbook_ulink_properties_factory_method(CongServiceNodePropertyDialog *custom_property_dialog, CongDocument *doc, CongNodePtr node)
{
	gchar* glade_filename;
	GladeXML *xml;
	GtkWidget *notebook1;

	g_message("docbook_ulink_properties_factory_method");

	g_return_val_if_fail(custom_property_dialog, NULL);
	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);

	glade_filename = gnome_program_locate_file (cong_app_get_gnome_program (cong_app_singleton()),
						    GNOME_FILE_DOMAIN_APP_DATADIR,
						    "glade/docbook-ulink-properties.glade",
						    FALSE,
						    NULL);

	global_glade_doc_ptr = doc;
	global_glade_node_ptr = node;

	xml = glade_xml_new(glade_filename, NULL, NULL);
	glade_xml_signal_autoconnect(xml);

	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "test_link_button")),
			  "pressed",
			  G_CALLBACK (on_test_link_pressed),
			  node);

	global_glade_doc_ptr = NULL;
	global_glade_node_ptr = NULL;

	g_free(glade_filename);

	/* Add the advanced properties tab: */
	notebook1 = glade_xml_get_widget(xml, "notebook1");
	cong_ui_append_advanced_node_properties_page(GTK_NOTEBOOK(notebook1),
						     doc, 
						     node);
	
	return glade_xml_get_widget(xml, "common_dialog");
}

static enum NodeToolFilterResult 
node_filter_browse_url (CongServiceNodeTool *node_tool, 
			CongDocument *doc, 
			CongNodePtr node,
			gpointer user_data)
{
	if (cong_node_is_element (node,
				  NULL,
				  "ulink")) {
		gchar *url = cong_node_get_attribute (node, NULL, "url");
		
		if (url) {
			g_free (url);

			return NODE_TOOL_AVAILABLE;
		} else {
			return NODE_TOOL_INSENSITIVE;
		}
	} else {
		return NODE_TOOL_HIDDEN;
	}
}

static void 
action_callback_browse_url (CongServiceNodeTool *tool, 
			    CongDocument *doc, 
			    CongNodePtr node,
			    GtkWindow *parent_window,
			    gpointer user_data)
{
	open_ulink_in_browser (node);
}

static enum NodeToolFilterResult
node_filter_promote (CongServiceNodeTool *node_tool, 
		     CongDocument *doc, 
		     CongNodePtr node,
		     gpointer user_data)
{
	if (cong_util_is_docbook(doc)) {
		if (cong_node_is_element(node, NULL, "sect2")) {
			return NODE_TOOL_AVAILABLE;
		} else if (cong_node_is_element(node, NULL, "sect3")) {
			return NODE_TOOL_AVAILABLE;
		} else if (cong_node_is_element(node, NULL, "sect4")) {
			return NODE_TOOL_AVAILABLE;
		} else if (cong_node_is_element(node, NULL, "sect5")) {
			return NODE_TOOL_AVAILABLE;
		}

		/* FIXME: handle <sect> tags */
	}

	return NODE_TOOL_HIDDEN;
}

static void
action_callback_promote (CongServiceNodeTool *tool, 
			 CongDocument *doc, 
			 CongNodePtr node,
			 GtkWindow *parent_window,
			 gpointer user_data)
{
	g_message ("action_callback_promote");

	/* Unwritten */
	CONG_DO_UNIMPLEMENTED_DIALOG (parent_window,
				      "promote DocBook");
}

static enum NodeToolFilterResult
node_filter_demote (CongServiceNodeTool *node_tool, 
		    CongDocument *doc, 
		    CongNodePtr node,
		    gpointer user_data)
{
	if (cong_util_is_docbook(doc)) {
		if (cong_node_is_element(node, NULL, "sect1")) {
			return NODE_TOOL_AVAILABLE;
		} else if (cong_node_is_element(node, NULL, "sect2")) {
			return NODE_TOOL_AVAILABLE;
		} else if (cong_node_is_element(node, NULL, "sect3")) {
			return NODE_TOOL_AVAILABLE;
		} else if (cong_node_is_element(node, NULL, "sect4")) {
			return NODE_TOOL_AVAILABLE;
		}

		/* FIXME: handle <sect> tags */
	}

	return NODE_TOOL_HIDDEN;
}

static void
action_callback_demote (CongServiceNodeTool *tool, 
			CongDocument *doc, 
			CongNodePtr node,
			GtkWindow *parent_window,
			gpointer user_data)
{
	g_message ("action_callback_demote");

	/* Unwritten */
	CONG_DO_UNIMPLEMENTED_DIALOG (parent_window,
				      "demote DocBook");
}



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
				      _("Use Norman Walsh's DocBook stylesheets to create a webpage from this DocBook file"),
				      "docbook-HTML-export",
				      docbook_exporter_document_filter,
				      html_exporter_action_callback,
				      NULL);

	cong_plugin_register_exporter(plugin, 
				      _("Export DocBook as PDF"), 
				      _("Use Norman Walsh's DocBook stylesheets to create a PDF file from this DocBook file"),
				      "docbook-PDF-export",
				      docbook_exporter_document_filter,
				      pdf_exporter_action_callback,
				      NULL);

	cong_plugin_register_exporter(plugin, 
				      _("Export DocBook as XSL:FO"),
				      _("Use Norman Walsh's DocBook stylesheets to create an XSL:FO file from this DocBook file that can be printed or converted to PDF at a later date"),
				      "docbook-XSLFO-export",
				      docbook_exporter_document_filter,
				      fo_exporter_action_callback,
				      NULL);

#if (ENABLE_PRINTING && ENABLE_LIBFO)
	cong_plugin_register_print_method(plugin, 
					  _("Print DocBook"),
					  "",
					  "docbook-print",
					  docbook_print_method_document_filter,
					  docbook_print_method_action_callback,
					  NULL);
#endif

	cong_plugin_register_custom_property_dialog(plugin,
						    _("Generic DocBook property dialog"), 
						    _("Provides a Properties dialog for most DocBook nodes"),
						    "docbook-generic-node-properties",
						    docbook_generic_node_factory_method,
						    NULL);

	cong_plugin_register_custom_property_dialog_for_element (plugin,
								 "orderedlist",
								 "docbook-orderedlist-properties",
								 docbook_orderedlist_properties_factory_method,
								 NULL);

	cong_plugin_register_custom_property_dialog_for_element (plugin,
								 "ulink",
								 "docbook-ulink-properties",
								 docbook_ulink_properties_factory_method,
								 NULL);

	cong_plugin_register_node_tool (plugin,
					_("Open Link in Browser"), 
					"",
					"docbook-browse-to-url",
					_("Open Link in Browser"),
					NULL,
					NULL,
					node_filter_browse_url,
					action_callback_browse_url,
					NULL);

#if 1
	cong_plugin_register_node_tool (plugin,
					_("Promote Section"), 
					_("Promotes a DocBook section to a higher organisational level within the document"),
					"docbook-promote",
					_("Promote"),
					_("Promote the section to a higher organisational level within the document"),
					_("Promote the section to a higher organisational level within the document"),
					node_filter_promote,
					action_callback_promote,
					NULL);

	cong_plugin_register_node_tool (plugin,
					_("Demote Section"), 
					_("Demotes a DocBook section to a lower organisational level within the document"),
					"docbook-demote",
					_("Demote"),
					_("Demote the section to a lower organisational level within the document"),
					_("Demote the section to a lower organisational level within the document"),
					node_filter_demote,
					action_callback_demote,
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
