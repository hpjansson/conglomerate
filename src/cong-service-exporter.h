/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-stub-exporter.h
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

#ifndef __CONG_STUB_EXPORTER_H__
#define __CONG_STUB_EXPORTER_H__

#include "cong-plugin.h"

G_BEGIN_DECLS

#define CONG_SERVICE_EXPORTER_TYPE	  (cong_service_exporter_get_type ())
#define CONG_SERVICE_EXPORTER(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_SERVICE_EXPORTER_TYPE, CongServiceExporter)
#define CONG_SERVICE_EXPORTER_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_SERVICE_EXPORTER_TYPE, CongServiceExporterClass)
#define IS_CONG_SERVICE_EXPORTER(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_SERVICE_EXPORTER_TYPE)
CONG_DECLARE_CLASS (CongServiceExporter, cong_service_exporter, CongService)

typedef gboolean 
(*CongServiceExporterDocumentFilter) (CongServiceExporter *exporter, 
				      CongDocument *doc, 
				      gpointer user_data);

typedef GtkWidget* 
(*CongServiceExporterOptionsWidgetCallback) (CongServiceExporter *exporter, 
					     CongDocument *doc, 
					     gpointer user_data);

typedef void 
(*CongServiceExporterActionCallback) (CongServiceExporter *exporter, 
				      CongDocument *doc, 
				      const gchar *uri, 
				      gpointer user_data, 
				      GtkWindow *toplevel_window);

CongServiceExporter*
cong_service_exporter_construct (CongServiceExporter *exporter,
				 const gchar *name, 
				 const gchar *description,
				 const gchar *service_id,
				 CongServiceExporterDocumentFilter doc_filter,
				 CongServiceExporterOptionsWidgetCallback options_widget_callback,
				 CongServiceExporterActionCallback action_callback,
				 gpointer user_data);

CongServiceExporter*
cong_plugin_register_exporter (CongPlugin *plugin, 
			       const gchar *name, 
			       const gchar *description,
			       const gchar *id,
			       CongServiceExporterDocumentFilter doc_filter,
			       CongServiceExporterOptionsWidgetCallback options_widget_callback,
			       CongServiceExporterActionCallback action_callback,
			       gpointer user_data);

gboolean 
cong_exporter_supports_document (CongServiceExporter *exporter, 
				 CongDocument *doc);
void 
cong_exporter_invoke (CongServiceExporter *exporter, 
		      CongDocument *doc, 
		      const gchar *uri, 
		      GtkWindow *toplevel_window);
gchar*
cong_exporter_get_preferred_uri (CongServiceExporter *exporter);

void
cong_exporter_set_preferred_uri (CongServiceExporter *exporter, 
				 const gchar *uri);

GtkWidget* 
cong_exporter_make_options_widget (CongServiceExporter *exporter, 
				   CongDocument *doc);

void 
cong_plugin_for_each_exporter (CongPlugin *plugin, 
			       void (*callback) (CongServiceExporter *exporter, 
						 gpointer user_data), 
			       gpointer user_data);

G_END_DECLS

#endif



