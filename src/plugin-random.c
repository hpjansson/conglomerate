/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-random.c
 *
 * Plugin for testing service of various kinds.  Not really intended for end-users.
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

#include "cong-fake-plugin-hooks.h"

struct RandomCreationInfo
{
	CongDispspec *dispspec;
	gboolean ensure_valid;
	int depth;
	GRand *random;
};

struct RandomGUI
{
	GnomeDruidPageStandard *page;
	GladeXML *xml;
	GtkWidget *middle_page;
	GtkOptionMenu *dispspec_option_menu;
	GtkMenu *dispspec_menu;
};

static void
add_option_for_dispspec (struct RandomGUI *random_gui,
			 CongDispspec *dispspec)
{
	GtkMenuItem *menu_item;

	g_assert (random_gui);
	g_assert (dispspec);

	menu_item = GTK_MENU_ITEM (gtk_menu_item_new_with_label ( cong_dispspec_get_name (dispspec)));
	gtk_widget_show (GTK_WIDGET (menu_item));
	
	gtk_menu_shell_append (GTK_MENU_SHELL (random_gui->dispspec_menu),
			       GTK_WIDGET (menu_item));
	
	g_object_set_data (G_OBJECT (menu_item),
			   "dispspec",
			   dispspec);
}

void factory_page_creation_callback_random(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
	struct RandomGUI random_gui;
#if 0
	g_message("factory_page_creation_callback_random");
#endif
	
	random_gui.xml = cong_util_load_glade_file ("glade/plugin-random.glade",
						    "middle_page",
						    NULL,
						    NULL);
	
	random_gui.middle_page = glade_xml_get_widget (random_gui.xml, "middle_page");
	
	
	random_gui.page = cong_new_file_assistant_new_page (assistant, 
							    factory, 
							    TRUE,
							    TRUE);	

	gnome_druid_page_standard_append_item (GNOME_DRUID_PAGE_STANDARD(random_gui.page),
					       _("What kind of random XML document would you like to create?"),
					       random_gui.middle_page,
					       "");

	random_gui.dispspec_option_menu = GTK_OPTION_MENU (glade_xml_get_widget (random_gui.xml, "optionmenu_doctype"));
	random_gui.dispspec_menu = GTK_MENU (gtk_menu_new());
	gtk_option_menu_set_menu (random_gui.dispspec_option_menu,
				  GTK_WIDGET (random_gui.dispspec_menu));
	gtk_widget_show (GTK_WIDGET (random_gui.dispspec_menu));

	/* Generate the available document types from the dispspecs that are known */
	{
		unsigned int i;	
		CongDispspecRegistry* ds_registry;		

		ds_registry = cong_app_get_dispspec_registry (cong_app_singleton ());;
		
		for (i=0; i<cong_dispspec_registry_get_num (ds_registry); i++) {			
			add_option_for_dispspec (&random_gui,
						 cong_dispspec_registry_get (ds_registry, i));
		}

		gtk_option_menu_set_history (random_gui.dispspec_option_menu,
					     0);
	}

	/* FIXME: this will leak various things stored in random_gui */
}

static void
populate_element (struct RandomCreationInfo *rci,
		  xmlDocPtr xml_doc,
		  xmlNodePtr xml_node,
		  int depth);

gboolean
generate_bool_for_opt (struct RandomCreationInfo *rci)
{
	g_assert (rci);

	return g_rand_boolean (rci->random);
}

gint
generate_count_for_mult (struct RandomCreationInfo *rci)
{
	g_assert (rci);

	return g_rand_int_range (rci->random,
				 0,
				 6);
}

gint
generate_count_for_plus (struct RandomCreationInfo *rci)
{
	g_assert (rci);

	return g_rand_int_range (rci->random,
				 1,
				 7);
}

gint
generate_count_for_ocur (struct RandomCreationInfo *rci,
			 xmlElementContentOccur ocur)
{
	g_assert (rci);

	switch (ocur) {
	default: g_assert_not_reached ();
	case XML_ELEMENT_CONTENT_ONCE:
		return 1;

	case XML_ELEMENT_CONTENT_OPT:
		return generate_bool_for_opt (rci)?1:0;

	case XML_ELEMENT_CONTENT_MULT:
		return generate_count_for_mult (rci);

	case XML_ELEMENT_CONTENT_PLUS:
		return generate_count_for_plus (rci);
	}
	
}

static void
populate_element_from_content (struct RandomCreationInfo *rci,
			       xmlDocPtr xml_doc,
			       xmlNodePtr xml_node,
			       int depth,
			       xmlElementContentPtr content)
{
	g_assert (content);

	switch (content->type) {
	default: g_assert_not_reached ();
	case XML_ELEMENT_CONTENT_PCDATA:
		{
			xmlNodePtr child_node = xmlNewDocText (xml_doc,
							       "the quick brown fox jumps over the lazy dog");
			xmlAddChild (xml_node, 
				     child_node);
		}
		break;
	case XML_ELEMENT_CONTENT_ELEMENT:
		{
			xmlNodePtr child_node = xmlNewDocNode (xml_doc,
							       NULL,
							       content->name,
							       ""); /* FIXME: namespace? */
			xmlAddChild (xml_node, 
				     child_node);
			populate_element (rci,
					  xml_doc,
					  child_node,
					  depth+1);
		}
		break;
	case XML_ELEMENT_CONTENT_SEQ:
		/* Do both c1 and c2 in sequence: */
		populate_element_from_content (rci,
					       xml_doc,
					       xml_node,
					       depth,
					       content->c1);
		populate_element_from_content (rci,
					       xml_doc,
					       xml_node,
					       depth,
					       content->c2);
		break;
	case XML_ELEMENT_CONTENT_OR:
		/* Do one of c1 or c2: */
		if (generate_bool_for_opt (rci)) {
			populate_element_from_content (rci,
						       xml_doc,
						       xml_node,
						       depth,
						       content->c1);
		} else {
			populate_element_from_content (rci,
						       xml_doc,
						       xml_node,
						       depth,
						       content->c2);
		}
		break;
	}
}

static void
populate_element_from_dtd (struct RandomCreationInfo *rci,
			   xmlDocPtr xml_doc,
			   xmlNodePtr xml_node,
			   int depth,
			   xmlElementPtr element)
{
	g_assert (rci);
	g_assert (xml_doc);
	g_assert (xml_node);
	g_assert (element);

	switch (element->etype) {
	default: g_assert_not_reached ();
	case XML_ELEMENT_TYPE_UNDEFINED:
	case XML_ELEMENT_TYPE_EMPTY:
		/* do nothing */
		break;

	case XML_ELEMENT_TYPE_ANY:
		break;

	case XML_ELEMENT_TYPE_MIXED:
		break;

	case XML_ELEMENT_TYPE_ELEMENT:
		break;
	}

	if (element->content) {
		guint count = generate_count_for_ocur (rci,
						       element->content->ocur);
		gint i;
		
		for (i=0;i<count;i++) {
			populate_element_from_content (rci,
						       xml_doc,
						       xml_node,
						       depth+1,
						       element->content);
		}				
	}


	/* FIXME: set up attributes! */
}

static void
populate_element (struct RandomCreationInfo *rci,
		  xmlDocPtr xml_doc,
		  xmlNodePtr xml_node,
		  int depth)
{
	int i=0;

	g_assert (rci);
	g_assert (xml_doc);
	g_assert (xml_node);

	/* Safety cutoffs */
	{ 
		/* Stop if we've reached the maximum depth */
		if (depth>=rci->depth) {
			return;
		}
	}

	if (xml_doc->extSubset) {
		xmlElementPtr element = cong_dtd_element_get_element_for_node (xml_doc->extSubset,
									       xml_node);
		if (element) {
			populate_element_from_dtd (rci,
						   xml_doc,
						   xml_node,
						   depth,
						   element);
			return;
		}
	} 

	/* No DTD information was available for this node; randomly add content: */
	for (i=0;i<(rci->depth-depth);i++) {
		xmlNodePtr child_node = xmlNewDocNode (xml_doc,
						       NULL,
						       "chapter",
						       "");
		xmlAddChild (xml_node, 
			     child_node);
		
		populate_element (rci,
				  xml_doc,
				  child_node,
				  depth+1);
	}
}

static xmlDocPtr
make_random_doc (struct RandomCreationInfo *rci)
{
	xmlDocPtr xml_doc;
	xmlNodePtr root_node;
	const CongExternalDocumentModel* dtd_model;
	CongDispspecElement *ds_element_root;
	const xmlChar *root_element;

	g_assert (rci);
	g_assert (rci->dispspec);

	ds_element_root = cong_dispspec_get_first_element (rci->dispspec); /* FIXME */
	g_assert (ds_element_root);

	root_element = cong_dispspec_element_tagname (ds_element_root);
	g_assert (root_element);

	dtd_model = cong_dispspec_get_external_document_model (rci->dispspec,
							       CONG_DOCUMENT_MODE_TYPE_DTD);

	xml_doc = xmlNewDoc ("1.0");
	root_node = xmlNewDocNode (xml_doc,
				   NULL, /* xmlNsPtr ns, */
				   root_element,
				   NULL);
	xmlDocSetRootElement (xml_doc,
			      root_node);

	if (dtd_model) {
		cong_util_add_external_dtd (xml_doc, 
					    root_element,
					    cong_external_document_model_get_public_id (dtd_model),
					    cong_external_document_model_get_system_id (dtd_model));
	}
	
	populate_element (rci,
			  xml_doc,
			  root_node,
			  0);
	return xml_doc;
}


void factory_action_callback_random(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
	struct RandomCreationInfo rci;
	xmlDocPtr xml_doc;

	rci.dispspec = cong_dispspec_registry_get (cong_app_get_dispspec_registry (cong_app_singleton ()), 1); /* FIXME */
	rci.depth = 4; /* FIXME */
	rci.random = g_rand_new ();

	xml_doc = make_random_doc (&rci);
	g_assert (xml_doc);

	cong_ui_new_document_from_manufactured_xml (xml_doc,
						    cong_new_file_assistant_get_toplevel (assistant));
}

 /* would be exposed as "plugin_register"? */
gboolean plugin_random_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);
	
	cong_plugin_register_document_factory(plugin, 
					      _("Random Document"), 
					      _("Create an XML document containing random content."),
					      "random-factory",
					      factory_page_creation_callback_random,
					      factory_action_callback_random,
					      NULL,
					      NULL);
	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean plugin_random_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
