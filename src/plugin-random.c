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
#include "cong-random-document.h"

#include "cong-fake-plugin-hooks.h"

#define LOG_RANDOM1(x)       (g_message ((x)))
#define LOG_RANDOM2(x, a)    (g_message ((x), (a)))
#define LOG_RANDOM3(x, a, b) (g_message ((x), (a), (b)))

typedef struct RandomGUI RandomGUI;

struct RandomGUI
{
	GtkWidget *page;
	GladeXML *xml;
	GtkWidget *middle_page;
	GtkWidget *combo_box;
	GPtrArray *combo_array;
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
	                                                    random_gui->middle_page,
	                                                    _("What kind of random XML document would you like to create?"),
	                                                    NULL,
							    TRUE,
							    TRUE);	

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
	xmlDocPtr xml_doc;
	RandomGUI *gui;

	gui = (RandomGUI*)cong_new_file_assistant_get_data_for_factory (assistant,
									factory);
	g_assert (gui);

	xml_doc = cong_make_random_doc (random_gui_get_selected_dispspec (gui),
					TRUE,
					random_gui_get_depth (gui));
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
