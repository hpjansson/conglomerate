/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-doc-from-xds.c
 *
 * Plugin for Creating Stub Documents from Dispspecs
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
 * Authors: Jeff Martin <jeff@custommonkey.org>
 */

#include "global.h"
#include "cong-plugin.h"
#include "cong-app.h"
#include "cong-fake-plugin-hooks.h"
#include "cong-dispspec.h"
#include "cong-dispspec-registry.h"
#include "cong-util.h"

/**
 * factory_action_callback_doc_from_xds:
 * @factory:
 * @assistant:
 * @user_data:
 *
 * TODO: Write me
 */
void 
factory_action_callback_doc_from_xds(CongServiceDocumentFactory *factory,
	CongNewFileAssistant *assistant, gpointer user_data)
{
	xmlDocPtr xml_doc;
	CongDispspec* dispspec;
	xmlNodePtr template;

	dispspec = (CongDispspec*)user_data;

	xml_doc = xmlNewDoc("1.0");

	template = cong_dispspec_get_template(dispspec);
	if(template)
	{
		xmlNodePtr clone;
		const CongExternalDocumentModel* model_dtd;

		clone = xmlCopyNode(template, TRUE);

		cong_node_recursive_set_doc (clone, 
					     xml_doc);

		model_dtd = cong_dispspec_get_external_document_model (dispspec,
								       CONG_DOCUMENT_MODE_TYPE_DTD);

		if (model_dtd) {
			cong_util_add_external_dtd (xml_doc, 
						    clone->name,
						    cong_external_document_model_get_public_id (model_dtd),
						    cong_external_document_model_get_system_id (model_dtd));
		}

		xmlDocSetRootElement(xml_doc, clone);
	}

	cong_ui_new_document_from_manufactured_xml(xml_doc,
		cong_new_file_assistant_get_toplevel(assistant));
}

/**
 * factory_page_creation_callback_doc_from_xds:
 * @factory:
 * @assistant:
 * @user_data:
 *
 * TODO: Write me
 */
void 
factory_page_creation_callback_doc_from_xds(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data)
{
}



/* would be exposed as "plugin_register"? */
/**
 * plugin_doc_from_xds_plugin_register:
 * @plugin:
 *
 * TODO: Write me
 */
gboolean 
plugin_doc_from_xds_plugin_register(CongPlugin *plugin)
{
	int i;
	CongDispspecRegistry* registry;

	g_return_val_if_fail(plugin, FALSE);

	registry = cong_app_get_dispspec_registry (cong_app_singleton());
	

	for (i = 0;i < cong_dispspec_registry_get_num(registry);i++)
	{
		CongDispspec* dispspec;

		dispspec = cong_dispspec_registry_get(registry, i);

		if( cong_dispspec_get_template(dispspec))
		{

			cong_plugin_register_document_factory(plugin, 
				_(cong_dispspec_get_name(dispspec)), 
				_(cong_dispspec_get_description(dispspec)),
				cong_dispspec_get_name(dispspec), 
				factory_page_creation_callback_doc_from_xds,
				factory_action_callback_doc_from_xds,
				"cong-docbook-article",
				dispspec);
		}

	}

	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
/**
 * plugin_doc_from_xds_plugin_configure:
 * @plugin:
 *
 * TODO: Write me
 */
gboolean 
plugin_doc_from_xds_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
