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
	GnomeDruidPageStandard *page;

	g_message("factory_page_creation_callback_unified <%s>", (char*)user_data);

	page = cong_new_file_assistant_new_page(assistant, 
						factory, 
						TRUE,
						TRUE);

	gnome_druid_page_standard_append_item(GNOME_DRUID_PAGE_STANDARD(page),
					      "This is a dummy calendar control; it's a placeholder and will eventually be replaced with some useful options",
					      gtk_calendar_new(),
					      user_data);

#if 0
	cong_new_file_assistant_set_page(assistant, GNOME_DRUID_PAGE(page));
#endif
}


xmlDocPtr make_article(const xmlChar *title)
{
	xmlDocPtr xml_doc;
	xmlNodePtr root_node;

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
				  title)
		    );

	xmlAddChild(root_node, 
		    xmlNewDocNode(xml_doc,
				  NULL,
				  "para",
				  "")
		    );

	return xml_doc;
}

void factory_action_callback_article(CongDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
	xmlDocPtr xml_doc;

	xml_doc = make_article("Untitled Article");

	cong_ui_new_document_from_manufactured_xml(xml_doc);	
}


xmlDocPtr make_book(const xmlChar *title)
{
	xmlDocPtr xml_doc;
	xmlNodePtr root_node;
	xmlNodePtr chapter_node;

	/* Build up the document and its content: */
	xml_doc = xmlNewDoc("1.0");
	
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
				  "")
		    );

	return xml_doc;
}

void factory_action_callback_book(CongDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
	xmlDocPtr xml_doc;

	xml_doc = make_book("Untitled Book");

	cong_ui_new_document_from_manufactured_xml(xml_doc);	
}


xmlDocPtr make_set(const xmlChar *title)
{
	xmlDocPtr xml_doc;
	xmlNodePtr root_node;

	/* Build up the document and its content: */
	xml_doc = xmlNewDoc("1.0");
	
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

	xml_doc = make_set("Untitled Set");

	cong_ui_new_document_from_manufactured_xml(xml_doc);	
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

void text_importer_action_callback(CongImporter *importer, const gchar *uri, const gchar *mime_type, gpointer user_data)
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
		cong_ui_new_document_from_imported_xml(xml_doc);	
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

void sourcecode_importer_action_callback(CongImporter *importer, const gchar *uri, const gchar *mime_type, gpointer user_data)
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
		cong_ui_new_document_from_imported_xml(xml_doc);	
	}
}

gboolean docbook_exporter_fip_filter(CongExporter *exporter, const gchar *fip, gpointer user_data)
{
	g_return_val_if_fail(exporter, FALSE);
	g_return_val_if_fail(fip, FALSE);

	return TRUE; /* for now */
}

void html_exporter_action_callback(CongExporter *exporter, const gchar *uri, gpointer user_data)
{
	g_return_if_fail(exporter);
	g_return_if_fail(uri);

	CONG_DO_UNIMPLEMENTED_DIALOG("Export DocBook to HTML");
}

void pdf_exporter_action_callback(CongExporter *exporter, const gchar *uri, gpointer user_data)
{
	g_return_if_fail(exporter);
	g_return_if_fail(uri);

	CONG_DO_UNIMPLEMENTED_DIALOG("Export DocBook to PDF");
}

void fo_exporter_action_callback(CongExporter *exporter, const gchar *uri, gpointer user_data)
{
	g_return_if_fail(exporter);
	g_return_if_fail(uri);

	CONG_DO_UNIMPLEMENTED_DIALOG("Export DocBook to FO");
}


 /* would be exposed as "plugin_register"? */
gboolean plugin_docbook_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);
	
	cong_plugin_register_document_factory(plugin, 
					      "DocBook Article", 
					      "Create an article, perhaps for a website or a magazine, using the \"DocBook\" format",
					      factory_page_creation_callback_unified,
					      factory_action_callback_article,
					      "article");
	cong_plugin_register_document_factory(plugin, 
					      "DocBook Book", 
					      "Create a book, using the \"DocBook\" format",
					      factory_page_creation_callback_unified,
					      factory_action_callback_book,
					      "book");
	cong_plugin_register_document_factory(plugin, 
					      "DocBook Set", 
					      "Create a set of related books, using the \"DocBook\" format",
					      factory_page_creation_callback_unified,
					      factory_action_callback_set,
					      "set");	

	cong_plugin_register_importer(plugin, 
				      "Import text as a DocBook article", 
				      "Import a plain text file into the \"DocBook\" format, as an article.",
				      text_importer_mime_filter,
				      text_importer_action_callback,
				      NULL);


	cong_plugin_register_importer(plugin, 
				      "Import program code as a DocBook article", 
				      "Import program source code into the \"DocBook\" format, as an article.",
				      sourcecode_importer_mime_filter,
				      sourcecode_importer_action_callback,
				      NULL);

	cong_plugin_register_exporter(plugin, 
				      "Export DocBook as HTML", 
				      "",
				      docbook_exporter_fip_filter,
				      html_exporter_action_callback,
				      NULL);

	cong_plugin_register_exporter(plugin, 
				      "Export DocBook as PDF", 
				      "",
				      docbook_exporter_fip_filter,
				      pdf_exporter_action_callback,
				      NULL);

	cong_plugin_register_exporter(plugin, 
				      "Export DocBook as XSL:FO",
				      "",
				      docbook_exporter_fip_filter,
				      fo_exporter_action_callback,
				      NULL);

	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean plugin_docbook_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
