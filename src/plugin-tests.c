/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-tests.c
 *
 * Plugin for testing service of various kinds.  Not really intended for end-users.
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

#include "cong-fake-plugin-hooks.h"

struct UnicodeCreationInfo
{
	int num_tables;
	int rows_per_table;
	int cols_per_table;
	int starting_character;
};

void factory_page_creation_callback_unicode(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
	GnomeDruidPageStandard *page;

#if 0
	g_message("factory_page_creation_callback_unicode");
#endif

	page = cong_new_file_assistant_new_page(assistant, 
						factory, 
						TRUE,
						TRUE);

	gnome_druid_page_standard_append_item(GNOME_DRUID_PAGE_STANDARD(page),
					      _("This is a dummy calendar control; it's a placeholder and will eventually be replaced with some useful options"),
					      gtk_calendar_new(),
					      user_data);

#if 0
	cong_new_file_assistant_set_page(assistant, GNOME_DRUID_PAGE(page));
#endif
}

static xmlNodePtr make_head_row(xmlDocPtr xml_doc, const struct UnicodeCreationInfo* uci, int table_index)
{
	int i;

	xmlNodePtr row = xmlNewDocNode(xml_doc,
				       NULL,
				       "row",
				       NULL);

	for (i=0;i<uci->cols_per_table;i++) {
		xmlNodePtr entry;
		gchar *text;

		text = g_strdup_printf("%X", i);

		entry = xmlNewDocNode(xml_doc,
				      NULL,
				      "entry",
				      text);
		xmlAddChild(row, entry);

		g_free(text);

	}

	return row;
}

static xmlNodePtr make_body_row(xmlDocPtr xml_doc, const struct UnicodeCreationInfo* uci, int table_index, int row_index)
{
	int i;

	xmlNodePtr row = xmlNewDocNode(xml_doc,
				       NULL,
				       "row",
				       NULL);

	for (i=0;i<uci->cols_per_table;i++) {
		xmlNodePtr entry;
		gchar *text;

		text = g_strdup_printf("&#x%X;", (table_index * uci->rows_per_table * uci->cols_per_table ) + (row_index * uci->cols_per_table) + i);

		entry = xmlNewDocNode(xml_doc,
				      NULL,
				      "entry",
				      text);
		xmlAddChild(row, entry);

		g_free(text);

	}

	return row;
}

static xmlNodePtr make_unicode_table(xmlDocPtr xml_doc, const struct UnicodeCreationInfo* uci, int table_index)
{
	xmlNodePtr table_node;

	g_return_val_if_fail(uci, NULL);

	table_node = xmlNewDocNode(xml_doc,
				   NULL,
				   "table",
				   NULL);

	/* Add title for this table: */
	{
		gchar *title = g_strdup_printf(_("Unicode Characters 0x%04X-0x%04X"), 
						uci->starting_character + (table_index*uci->rows_per_table*uci->cols_per_table), 
						uci->starting_character + ((table_index+1)*uci->rows_per_table*uci->cols_per_table)-1);
		
		xmlAddChild(table_node,
			    xmlNewDocNode(xml_doc,
					  NULL,
					  "title",
					  title)
			    );

		g_free(title);
	}

	/* Add <tgroup>: */
	{
		xmlNodePtr tgroup = xmlNewDocNode(xml_doc,
						  NULL,
						  "tgroup",
						  NULL);
		
		xmlAddChild(table_node, tgroup);
		
		xmlNewProp_NUMBER(tgroup, 
				  "cols",
				  uci->cols_per_table);

		/* Add <thead> */
		{
			xmlNodePtr thead = xmlNewDocNode(xml_doc,
							 NULL,
							 "thead",
							NULL);
			xmlAddChild(tgroup, thead);
			xmlAddChild(thead, 
				    make_head_row(xml_doc, uci, table_index)
				    );
		}

		/* Add <tbody> */
		{
			int row;
			xmlNodePtr tbody = xmlNewDocNode(xml_doc,
							 NULL,
							 "tbody",
							 NULL);
				
			xmlAddChild(tgroup, tbody);

			for (row=0;row<uci->rows_per_table;row++) { 
				xmlAddChild(tbody, 
					    make_body_row(xml_doc, uci, table_index, row)
					    );
			}
		}
	}
	
	return table_node;
}

static xmlDocPtr make_unicode_tables(const struct UnicodeCreationInfo* uci)
{
	xmlDocPtr xml_doc;
	xmlNodePtr root_node;

	g_return_val_if_fail(uci, NULL);

	/* Build up the document and its content: */
	xml_doc = xmlNewDoc("1.0");
	
	root_node = xmlNewDocNode(xml_doc,
				  NULL, /* xmlNsPtr ns, */
				  "article",
				  NULL);

	xmlDocSetRootElement(xml_doc,
			     root_node);

	{
		gchar *title = g_strdup_printf(_("Unicode Characters 0x%04X-0x%04X"), 
						uci->starting_character, 
						uci->starting_character + (uci->num_tables*uci->rows_per_table*uci->cols_per_table));
		
		xmlAddChild(root_node,
			    xmlNewDocNode(xml_doc,
					  NULL,
					  "title",
					  title)
			    );

		g_free(title);
	}

	/* Add the tables: */
	{
		int i;
		for (i=0;i<uci->num_tables;i++) {
			xmlAddChild(root_node, 
				    make_unicode_table(xml_doc, uci, i)
				    );
		}
	}


	return xml_doc;
}

#if 0
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
#endif

void factory_action_callback_unicode(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
	xmlDocPtr xml_doc;
	struct UnicodeCreationInfo uci;
	
	uci.num_tables = 16;
	uci.rows_per_table = 16; 
	uci.cols_per_table = 16;
	uci.starting_character = 0;

	xml_doc = make_unicode_tables(&uci);

	cong_ui_new_document_from_manufactured_xml(xml_doc,
						   cong_new_file_assistant_get_toplevel(assistant));	
}

 /* would be exposed as "plugin_register"? */
gboolean plugin_tests_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	/* Disable this service; it's confusing to end-users and unstable: */
#if 0	
	cong_plugin_register_document_factory(plugin, 
					      _("Unicode Character Reference"), 
					      _("Create a table listing a part of the Unicode character set as a DocBook article"),
					      "tests-unicode-character-reference-factory",
					      factory_page_creation_callback_unicode,
					      factory_action_callback_unicode,
					      NULL,
					      NULL);
#endif

	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean plugin_tests_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
