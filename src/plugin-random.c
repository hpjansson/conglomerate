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
#include "cong-eel.h"
#include "cong-glade.h"

#include "cong-fake-plugin-hooks.h"

#define LOG_RANDOM1(x)       (g_message ((x)))
#define LOG_RANDOM2(x, a)    (g_message ((x), (a)))
#define LOG_RANDOM3(x, a, b) (g_message ((x), (a), (b)))

typedef struct RandomGUI RandomGUI;
typedef struct RandomCreationInfo RandomCreationInfo;

struct RandomGUI
{
	GnomeDruidPageStandard *page;
	GladeXML *xml;
	GtkWidget *middle_page;
	GtkWidget *combo_box;
	GPtrArray *combo_array;
};

struct RandomCreationInfo
{
	CongDispspec *dispspec;
	gboolean ensure_valid;
	int depth;
	GRand *random;
};

/* GUI functions */
static CongDispspec*
random_gui_get_selected_dispspec (RandomGUI *random_gui)
{
	gint selected;
	g_assert (random_gui);
	g_assert (random_gui->combo_box);

	selected = gtk_combo_box_get_active( GTK_COMBO_BOX(random_gui->combo_box) );
	g_return_val_if_fail (selected != -1, NULL);
	return (CongDispspec*)g_ptr_array_index (random_gui->combo_array, selected );

}

static gint
random_gui_get_depth (RandomGUI *random_gui)
{
	g_assert (random_gui);
	g_assert (random_gui->xml);
	
	return gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (glade_xml_get_widget (random_gui->xml, "spinbutton_depth")));	
}

static void
random_gui_add_option_for_dispspec (RandomGUI *random_gui,
				    CongDispspec *dispspec)
{
	g_assert (random_gui);
	g_assert (dispspec);

	gtk_combo_box_append_text (GTK_COMBO_BOX (random_gui->combo_box),
				   cong_dispspec_get_name (dispspec) );
	g_ptr_array_add (random_gui->combo_array, (gpointer) dispspec);
}

static void
free_gui (gpointer factory_data)
{
	RandomGUI *random_gui;

	g_assert (factory_data);

	random_gui = (RandomGUI*)factory_data;

	if ( random_gui->combo_array )
		g_ptr_array_free (random_gui->combo_array, FALSE);

	g_object_unref (G_OBJECT (random_gui->xml));
	g_free (random_gui);	
}

/**
 * factory_page_creation_callback_random:
 * @factory:
 * @assistant:
 * @user_data:
 *
 * TODO: Write me
 */
void 
factory_page_creation_callback_random(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
	RandomGUI *random_gui;
#if 0
	g_message("factory_page_creation_callback_random");
#endif

	random_gui = g_new0 (RandomGUI, 1);
	
	random_gui->xml = cong_util_load_glade_file ("conglomerate/glade/plugin-random.glade",
						     "middle_page",
						     NULL,
						     NULL);
	
	random_gui->middle_page = glade_xml_get_widget (random_gui->xml, "middle_page");
	
	
	random_gui->page = cong_new_file_assistant_new_page (assistant, 
							    factory, 
							    TRUE,
							    TRUE);	

	gnome_druid_page_standard_append_item (GNOME_DRUID_PAGE_STANDARD(random_gui->page),
					       _("What kind of random XML document would you like to create?"),
					       random_gui->middle_page,
					       "");

	/*
	 * In order to get the combo box to work in its "simple" mode - i.e.
	 * without having to create a list store - I seem to have to have at
	 * least one "items" entry in Glade. So we delete it here. Which is
	 * plain ugly. Is there a way to tell glade that we want the "simple"
	 * form of the GtkComboBox widget? Aha. I can hack the glade file so that
	 * it has an items property for the widget, but that the contents are
	 * empty. This could be fragile (I don't know what glade will do it saves
         * the file), so I leave in the "dummy-item" which we have to delete.
         * Doug Burke
	 */
	random_gui->combo_box = glade_xml_get_widget (random_gui->xml, "combobox_doctype");
	gtk_combo_box_remove_text (GTK_COMBO_BOX(random_gui->combo_box), 0 );
	random_gui->combo_array = g_ptr_array_new ();
	
	/* Generate the available document types from the dispspecs that are known */
	{
		unsigned int i;	
		CongDispspecRegistry* ds_registry;		

		ds_registry = cong_app_get_dispspec_registry (cong_app_singleton ());;
		
		for (i=0; i<cong_dispspec_registry_get_num (ds_registry); i++) {			
			random_gui_add_option_for_dispspec (random_gui,
							    cong_dispspec_registry_get (ds_registry, i));
		}

		gtk_combo_box_set_active (GTK_COMBO_BOX(random_gui->combo_box), 0);
		gtk_widget_show (random_gui->combo_box);

	}

	cong_new_file_assistant_set_data_for_factory (assistant,
						      factory,
						      random_gui,
						      free_gui);
}

static void
populate_element (RandomCreationInfo *rci,
		  xmlDocPtr xml_doc,
		  xmlNodePtr xml_node,
		  int depth);

/**
 * generate_bool_for_opt:
 * @rci:
 *
 * TODO: Write me
 */
gboolean
generate_bool_for_opt (RandomCreationInfo *rci)
{
	g_assert (rci);

	return g_rand_boolean (rci->random);
}

/**
 * generate_count_for_mult:
 * @rci:
 *
 * TODO: Write me
 */
gint
generate_count_for_mult (RandomCreationInfo *rci)
{
	g_assert (rci);

	return g_rand_int_range (rci->random,
				 0,
				 6);
}

/**
 * generate_count_for_plus:
 * @rci:
 *
 * TODO: Write me
 */
gint
generate_count_for_plus (RandomCreationInfo *rci)
{
	g_assert (rci);

	return g_rand_int_range (rci->random,
				 1,
				 7);
}

/**
 * generate_count_for_ocur:
 * @rci:
 * @ocur:
 *
 * TODO: Write me
 */
gint
generate_count_for_ocur (RandomCreationInfo *rci,
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

#if 0
gchar*
cong_dtd_generate_source_for_content (xmlElementContentPtr content)
{
	g_return_val_if_fail (content, NULL);

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
#endif

/**
 * random_unichar:
 * @rci:
 *
 * TODO: Write me
 */
gunichar
random_unichar (RandomCreationInfo *rci)
{
	/* FIXME: probably should have a smarter system... */
	gunichar result;

	/* Have a high chance of spaces, to create word-breaking opportunities: */
	if (0==g_rand_int_range (rci->random, 0, 10)) {
		return ' ';
	}
	
	while (1) {
		result = g_rand_int_range (rci->random, 1, 65535);

		if (g_unichar_isdefined (result)) {
			if (!g_unichar_iscntrl (result)) {

#define UNICODE_VALID(Char)                   \
    ((Char) < 0x110000 &&                     \
     (((Char) & 0xFFFFF800) != 0xD800) &&     \
     ((Char) < 0xFDD0 || (Char) > 0xFDEF) &&  \
     ((Char) & 0xFFFE) != 0xFFFE)

				if (UNICODE_VALID (result)) {
					return result;
				}
			}
		}
	}
}

/**
 * random_text:
 * @rci:
 *
 * TODO: Write me
 */
gchar*
random_text (RandomCreationInfo *rci)
{
	/* FIXME: should we translate the various strings in this function? */
	switch (g_rand_int_range (rci->random, 0, 3)) {
	default: g_assert_not_reached ();
	case 0:
		return g_strdup ("the quick brown fox jumps over the lazy dog");

	case 1:
		return g_strdup ("lore ipsum");

	case 2:
		/* Generate an entirely random unicode string: */
		{
			#define MAX_LENGTH (50)
			gint count = g_rand_int_range (rci->random, 1, MAX_LENGTH);
			gint i;
			gunichar tmp_str[MAX_LENGTH+1];
			gchar *utf8_text;

			for (i=0;i<count;i++) {
				tmp_str[i]= random_unichar (rci);
			}
			tmp_str[i]=0;

			utf8_text = g_ucs4_to_utf8 (tmp_str,
						    count,
						    NULL,
						    NULL,
						    NULL);

			if (g_utf8_validate (utf8_text, -1, NULL)) {
				return utf8_text;
			} else {
				g_free (utf8_text);
				return g_strdup ("fubar");
			}

		}
	}
}

static void
populate_element_from_content (RandomCreationInfo *rci,
			       xmlDocPtr xml_doc,
			       xmlNodePtr xml_node,
			       int depth,
			       xmlElementContentPtr content)
{
	gint i;
	guint count;

	g_assert (content);

	count = generate_count_for_ocur (rci, content->ocur);
	
#if 0
	{
		gchar *frag = cong_dtd_generate_source_for_content (content);
		g_message ("got count of %i for %s", count, frag);
		g_free (frag);
	}
#endif

	for (i=0;i<count;i++) {	
		switch (content->type) {
		default: g_assert_not_reached ();
		case XML_ELEMENT_CONTENT_PCDATA:
			{
				gchar *text = random_text (rci);
				xmlNodePtr child_node = xmlNewDocText (xml_doc,
								       text);
				g_free (text);

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
}

static void
populate_element_from_dtd (RandomCreationInfo *rci,
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
		populate_element_from_content (rci,
					       xml_doc,
					       xml_node,
					       depth+1,
					       element->content);
	}



	/* FIXME: set up attributes! */
}

static void
populate_element (RandomCreationInfo *rci,
		  xmlDocPtr xml_doc,
		  xmlNodePtr xml_node,
		  int depth)
{
	g_assert (rci);
	g_assert (xml_doc);
	g_assert (xml_node);

	LOG_RANDOM3 ("populate_element (below <%s>, %i)", xml_node->name, depth);

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
	{
		gint child_count;
		gint i;
		
		/* Slow algorithm */
		guint num_elements = cong_dispspec_get_num_elements (rci->dispspec);

		child_count = g_rand_int_range (rci->random, 
						0,
						(rci->depth-depth));

		for (i=0;i<child_count;i++) {
			CongDispspecElement* ds_element;
			xmlNodePtr child_node;

			ds_element = cong_dispspec_get_element (rci->dispspec,
								g_rand_int_range (rci->random, 
										  0,
										  num_elements));
			g_assert (ds_element);

			child_node = xmlNewDocNode (xml_doc,
						    NULL,
						    cong_dispspec_element_get_local_name (ds_element),
						    "");
			if (cong_dispspec_element_get_ns_uri (ds_element)) {
				xmlNsPtr xml_ns = xmlNewNs (child_node, 
							    cong_dispspec_element_get_ns_uri (ds_element),
							    NULL);
				xmlSetNs (child_node, 
					  xml_ns);	
			}

			xmlAddChild (xml_node,
				     child_node);
			
			populate_element (rci,
					  xml_doc,
					  child_node,
					  depth+1);
		}
	}
}

static xmlDocPtr
make_random_doc (RandomCreationInfo *rci)
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

	root_element = cong_dispspec_element_get_local_name (ds_element_root);
	g_assert (root_element);

	dtd_model = cong_dispspec_get_external_document_model (rci->dispspec,
							       CONG_DOCUMENT_MODE_TYPE_DTD);

	xml_doc = xmlNewDoc ("1.0");

	root_node = xmlNewDocNode (xml_doc,
				   NULL, /* xmlNsPtr ns, */
				   root_element,
				   NULL);
	if (cong_dispspec_element_get_ns_uri (ds_element_root)) {
		xmlNsPtr xml_ns = xmlNewNs (root_node, 
					    cong_dispspec_element_get_ns_uri (ds_element_root),
					    NULL);
		xmlSetNs (root_node, 
			  xml_ns);	
	}
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

/**
 * factory_action_callback_random:
 * @factory:
 * @assistant:
 * @user_data:
 *
 * TODO: Write me
 */
void 
factory_action_callback_random(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
	RandomCreationInfo rci;
	xmlDocPtr xml_doc;
	RandomGUI *gui;

	gui = (RandomGUI*)cong_new_file_assistant_get_data_for_factory (assistant,
									factory);
	g_assert (gui);

#if 1
	rci.dispspec = random_gui_get_selected_dispspec (gui);
	rci.depth = random_gui_get_depth (gui);
#else
	rci.dispspec = cong_dispspec_registry_get (cong_app_get_dispspec_registry (cong_app_singleton ()), 1); /* FIXME */
	rci.depth = 10; /* FIXME */
#endif
	rci.random = g_rand_new ();

	xml_doc = make_random_doc (&rci);
	g_assert (xml_doc);

	cong_ui_new_document_from_manufactured_xml (xml_doc,
						    cong_new_file_assistant_get_toplevel (assistant));
}

/* would be exposed as "plugin_register"? */
/**
 * plugin_random_plugin_register:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_random_plugin_register(CongPlugin *plugin)
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
/**
 * plugin_random_plugin_configure:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_random_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
