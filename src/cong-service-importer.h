/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-importer.h
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

#ifndef __CONG_SERVICE_IMPORTER_H__
#define __CONG_SERVICE_IMPORTER_H__

#include "cong-service.h"
#include "cong-plugin.h"

G_BEGIN_DECLS

#define CONG_SERVICE_IMPORTER_TYPE	  (cong_service_importer_get_type ())
#define CONG_SERVICE_IMPORTER(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_SERVICE_IMPORTER_TYPE, CongServiceImporter)
#define CONG_SERVICE_IMPORTER_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_SERVICE_IMPORTER_TYPE, CongServiceImporterClass)
#define IS_CONG_SERVICE_IMPORTER(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_SERVICE_IMPORTER_TYPE)

CONG_DECLARE_CLASS_BEGIN (CongServiceImporter, cong_service_importer, CongService)
CONG_DECLARE_CLASS_END ()


/* Function pointers that are registered by plugins: */
typedef GtkFileFilter*
(*CongServiceImporterMakeFilterCallback) (CongServiceImporter *importer);
typedef void 
(*CongServiceImporterActionCallback) (CongServiceImporter *importer, 
				      const gchar *uri, 
				      const gchar *mime_type, 
				      gpointer user_data, 
				      GtkWindow *toplevel_window);

CongServiceImporter*
cong_service_importer_construct (CongServiceImporter *importer,
				 const gchar *name, 
				 const gchar *description,
				 const gchar *id,
				 CongServiceImporterMakeFilterCallback filter_factory_callback,
				 CongServiceImporterActionCallback action_callback,
				 gpointer user_data);

CongServiceImporter*
cong_plugin_register_importer (CongPlugin *plugin, 
			       const gchar *name, 
			       const gchar *description,
			       const gchar *id,
			       CongServiceImporterMakeFilterCallback filter_factory_callback,
			       CongServiceImporterActionCallback action_callback,
			       gpointer user_data);

GtkFileFilter*
cong_importer_make_file_filter (CongServiceImporter *importer);

void 
cong_importer_invoke (CongServiceImporter *importer, 
		      const gchar *filename, 
		      const gchar *mime_type, 
		      GtkWindow *toplevel_window);

void 
cong_plugin_for_each_importer (CongPlugin *plugin, 
			       void 
			       (*callback) (CongServiceImporter *importer, 
					    gpointer user_data), 
			       gpointer user_data);

GtkFileFilter* 
cong_service_importer_make_basic_filter (CongServiceImporter *importer);


G_END_DECLS

#endif



