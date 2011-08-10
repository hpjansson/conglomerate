/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-arbitrary.c
 *
 * Plugin for handling arbitrary documents
 *
 * Copyright (C) 2004 David Malcolm
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
#include "cong-util.h"
#include "cong-app.h"
#include "cong-dispspec.h"
#include "cong-dispspec-element.h"
#include "cong-dispspec-registry.h"
#include "cong-dtd.h"
#include "cong-eel.h"
#include "cong-glade.h"

#include "cong-fake-plugin-hooks.h"

typedef struct ArbitraryCreationInfo ArbitraryCreationInfo;

typedef struct ArbitraryGUI ArbitraryGUI;

struct ArbitraryGUI
{
	GtkWidget *page;
	GladeXML *xml;
	GtkWidget *middle_page;
	GtkEntry *entry_ns_uri;
	GtkEntry *entry_local_name;
	GtkTextBuffer *preview_text_buffer;
};

struct ArbitraryCreationInfo
{
	const gchar *ns_uri;
	const gchar *local_name;
};

static void
refresh_preview (ArbitraryGUI *arbitrary_gui);

static void
get_aci (ArbitraryGUI *arbitrary_gui,
	 ArbitraryCreationInfo *output_aci);

static xmlDocPtr
make_arbitrary_doc (ArbitraryCreationInfo *aci);


/* GUI functions */
static void
entry_changed_cb (GtkEditable *editable,
		  gpointer user_data)
{
	ArbitraryGUI *arbitrary_gui = (ArbitraryGUI *)user_data;

	refresh_preview (arbitrary_gui); 
}

static void
refresh_preview (ArbitraryGUI *arbitrary_gui)
{
	/* Generate doc, then use libxml to generate a UTF-8 string representation into the text view'sbuffer: */
	ArbitraryCreationInfo aci;
	xmlDocPtr xml_doc;
	xmlChar *doc_txt_ptr;
	int doc_txt_len;

	g_assert (arbitrary_gui);

	get_aci (arbitrary_gui,
		 &aci);

	xml_doc = make_arbitrary_doc (&aci);
	g_assert (xml_doc);

	xmlDocDumpFormatMemoryEnc (xml_doc,
				   &doc_txt_ptr,
				   &doc_txt_len, 
				   "UTF-8",
				   1);
	
	gtk_text_buffer_set_text (arbitrary_gui->preview_text_buffer,
				  (const gchar*)doc_txt_ptr,
				  doc_txt_len);	
	xmlFree (doc_txt_ptr);
	xmlFreeDoc (xml_doc);
}

static void
free_gui (gpointer factory_data)
{
	ArbitraryGUI *arbitrary_gui;

	g_assert (factory_data);

	arbitrary_gui = (ArbitraryGUI*)factory_data;

	g_object_unref (G_OBJECT (arbitrary_gui->xml));
	g_free (arbitrary_gui);	
}

static void
get_aci (ArbitraryGUI *gui,
	 ArbitraryCreationInfo *output_aci)
{
	g_assert (gui);
	g_assert (output_aci);

	output_aci->local_name = gtk_entry_get_text (gui->entry_local_name);
	output_aci->ns_uri = gtk_entry_get_text (gui->entry_ns_uri);
}

/**
 * factory_page_creation_callback_arbitrary:
 * @factory:
 * @assistant:
 * @user_data:
 *
 * TODO: Write me
 */
void 
factory_page_creation_callback_arbitrary(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
	ArbitraryGUI *arbitrary_gui;
#if 0
	g_message("factory_page_creation_callback_arbitrary");
#endif

	arbitrary_gui = g_new0 (ArbitraryGUI, 1);
	
	arbitrary_gui->xml = cong_util_load_glade_file ("conglomerate/glade/plugin-arbitrary.glade",
							"middle_page",
							NULL,
							NULL);
	
	arbitrary_gui->middle_page = glade_xml_get_widget (arbitrary_gui->xml, "middle_page");
	
	
	arbitrary_gui->page = cong_new_file_assistant_new_page (assistant, 
								factory, 
	                                                        arbitrary_gui->middle_page,
	                                                        _("The XML document needs a top-level element."),
	                                                        NULL,
								TRUE,
								TRUE);

	arbitrary_gui->entry_ns_uri = GTK_ENTRY (glade_xml_get_widget (arbitrary_gui->xml, "entry_ns_uri"));
	g_assert (arbitrary_gui->entry_ns_uri);

	arbitrary_gui->entry_local_name = GTK_ENTRY (glade_xml_get_widget (arbitrary_gui->xml, "entry_local_name"));
	g_assert (arbitrary_gui->entry_local_name);

	arbitrary_gui->preview_text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (glade_xml_get_widget (arbitrary_gui->xml, "textview_preview")));
	g_assert (arbitrary_gui->preview_text_buffer);

	g_signal_connect (arbitrary_gui->entry_ns_uri,
			  "changed",
			  G_CALLBACK (entry_changed_cb),
			  arbitrary_gui);
	g_signal_connect (arbitrary_gui->entry_local_name,
			  "changed",
			  G_CALLBACK (entry_changed_cb),
			  arbitrary_gui);

	refresh_preview (arbitrary_gui);

	cong_new_file_assistant_set_data_for_factory (assistant,
						      factory,
						      arbitrary_gui,
						      free_gui);
}

static xmlDocPtr
make_arbitrary_doc (ArbitraryCreationInfo *aci)
{
	xmlDocPtr xml_doc;
	xmlNodePtr root_node;

	g_assert (aci);
	g_assert (aci->ns_uri);
	g_assert (aci->local_name);

	xml_doc = xmlNewDoc ((const xmlChar*)"1.0");

	root_node = xmlNewDocNode (xml_doc,
				   NULL,
				   (const xmlChar*)aci->local_name,
				   NULL);
	if (aci->ns_uri[0]!='\0') {
		xmlNsPtr xml_ns = xmlNewNs (root_node, 
					    (const xmlChar*)aci->ns_uri, 
					    NULL);
		xmlSetNs (root_node, 
			  xml_ns);	
	}
	xmlDocSetRootElement (xml_doc,
			      root_node);
	
	return xml_doc;
}

/**
 * factory_action_callback_arbitrary:
 * @factory:
 * @assistant:
 * @user_data:
 *
 * TODO: Write me
 */
void 
factory_action_callback_arbitrary(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
	ArbitraryCreationInfo aci;
	xmlDocPtr xml_doc;
	ArbitraryGUI *gui;

	gui = (ArbitraryGUI*)cong_new_file_assistant_get_data_for_factory (assistant,
									   factory);
	g_assert (gui);

	get_aci (gui,
		 &aci);

	xml_doc = make_arbitrary_doc (&aci);
	g_assert (xml_doc);

	cong_ui_new_document_from_manufactured_xml (xml_doc,
						    cong_new_file_assistant_get_toplevel (assistant));
}

/* would be exposed as "plugin_register"? */
/**
 * plugin_arbitrary_plugin_register:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_arbitrary_plugin_register (CongPlugin *plugin)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), FALSE);

	cong_plugin_register_document_factory(plugin, 
					      _("Empty XML Document"), 
					      _("Create an empty XML document of an arbitrary type."),
					      "arbitrary-factory",
					      factory_page_creation_callback_arbitrary,
					      factory_action_callback_arbitrary,
					      NULL,
					      NULL);
	
	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
/**
 * plugin_arbitrary_plugin_configure:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_arbitrary_plugin_configure (CongPlugin *plugin)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), FALSE);

	return TRUE;
}
