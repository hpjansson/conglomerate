/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-exporter.c
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
#include "cong-service-exporter.h"
#include "cong-app.h"

struct CongServiceExporterPrivate
{
	CongServiceExporterDocumentFilter doc_filter;
	CongServiceExporterActionCallback action_callback;
	gpointer user_data;
};

CONG_DEFINE_CLASS (CongServiceExporter, cong_service_exporter, CONG_SERVICE_EXPORTER, CongService, CONG_SERVICE_TYPE)

/**
 * cong_service_exporter_construct:
 * @exporter:
 * @name:
 * @description:
 * @service_id:
 * @doc_filter:
 * @action_callback:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceExporter*
cong_service_exporter_construct (CongServiceExporter *exporter,
				 const gchar *name, 
				 const gchar *description,
				 const gchar *service_id,
				 CongServiceExporterDocumentFilter doc_filter,
				 CongServiceExporterActionCallback action_callback,
				 gpointer user_data)
{
	g_return_val_if_fail (IS_CONG_SERVICE_EXPORTER (exporter), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);
	g_return_val_if_fail (doc_filter, NULL);
	g_return_val_if_fail (action_callback, NULL);

	cong_service_construct (CONG_SERVICE (exporter),				
				name,
				description,
				service_id);
	PRIVATE (exporter)->doc_filter = doc_filter;
	PRIVATE (exporter)->action_callback = action_callback;
	PRIVATE (exporter)->user_data = user_data;

	return exporter;
}

/* Implementation of CongServiceExporter: */
/**
 * cong_exporter_supports_document:
 * @exporter:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
cong_exporter_supports_document(CongServiceExporter *exporter, CongDocument *doc)
{
	g_return_val_if_fail (IS_CONG_SERVICE_EXPORTER (exporter), FALSE);
	g_return_val_if_fail (doc, FALSE);

	g_assert (PRIVATE (exporter)->doc_filter);

	return PRIVATE (exporter)->doc_filter (exporter, 
					       doc, 
					       PRIVATE (exporter)->user_data);
}

/**
 * cong_exporter_invoke:
 * @exporter:
 * @doc:
 * @uri:
 * @toplevel_window:
 *
 * TODO: Write me
 */
void 
cong_exporter_invoke(CongServiceExporter *exporter, CongDocument *doc, const gchar *uri, GtkWindow *toplevel_window)
{
	g_return_if_fail (IS_CONG_SERVICE_EXPORTER (exporter));
	g_return_if_fail (doc);
	g_return_if_fail (uri);
	
	g_assert (PRIVATE (exporter)->action_callback);

	return PRIVATE (exporter)->action_callback (exporter, 
						    doc, 
						    uri, 
						    PRIVATE (exporter)->user_data, 
						    toplevel_window);
}

/**
 * cong_exporter_get_preferred_uri:
 * @exporter:
 *
 * TODO: Write me
 * Returns:
 */
gchar *
cong_exporter_get_preferred_uri(CongServiceExporter *exporter)
{
	gchar *gconf_key;
	gchar *preferred_uri;

	g_return_val_if_fail (IS_CONG_SERVICE_EXPORTER (exporter), NULL);

	gconf_key = cong_service_get_gconf_key (CONG_SERVICE(exporter),
						"preferred-uri");
	
	preferred_uri = gconf_client_get_string (cong_app_get_gconf_client (cong_app_singleton()),
						 gconf_key,
						 NULL);

	g_free(gconf_key);

	return preferred_uri;
}

/**
 * cong_exporter_set_preferred_uri:
 * @exporter:
 * @uri:
 *
 * TODO: Write me
 */
void 
cong_exporter_set_preferred_uri(CongServiceExporter *exporter, const gchar *uri)
{
	gchar *gconf_key;

	g_return_if_fail (IS_CONG_SERVICE_EXPORTER (exporter));
	g_return_if_fail (uri);

	gconf_key = cong_service_get_gconf_key(CONG_SERVICE(exporter), "preferred-uri");

	gconf_client_set_string (cong_app_get_gconf_client (cong_app_singleton()),
				 gconf_key,
				 uri,
				 NULL);
	
	g_free(gconf_key);
}

