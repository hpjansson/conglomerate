/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-importer.c
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
#include "cong-service-importer.h"

struct CongServiceImporterPrivate
{
	CongServiceImporterMakeFilterCallback filter_factory_callback;
	CongServiceImporterActionCallback action_callback;
	gpointer user_data;
};

CONG_DEFINE_CLASS (CongServiceImporter, cong_service_importer, CONG_SERVICE_IMPORTER, CongService, CONG_SERVICE_TYPE)

/**
 * cong_service_importer_construct:
 * @importer:
 * @name:
 * @description:
 * @id:
 * @filter_factory_callback:
 * @action_callback:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceImporter*
cong_service_importer_construct (CongServiceImporter *importer,
				 const gchar *name, 
				 const gchar *description,
				 const gchar *id,
				 CongServiceImporterMakeFilterCallback filter_factory_callback,
				 CongServiceImporterActionCallback action_callback,
				 gpointer user_data)
{
	g_return_val_if_fail (IS_CONG_SERVICE_IMPORTER (importer), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (id, NULL);
	g_return_val_if_fail (filter_factory_callback, NULL);
	g_return_val_if_fail (action_callback, NULL);

	cong_service_construct (CONG_SERVICE (importer),
				name,
				description,
				id);
		
	PRIVATE (importer)->filter_factory_callback = filter_factory_callback;
	PRIVATE (importer)->action_callback = action_callback;
	PRIVATE (importer)->user_data = user_data;

	return importer;
}

/* Implementation of CongServiceImporter: */
/**
 * cong_importer_make_file_filter:
 * @importer: the importer
 *
 * Run this importer's callback to create a GtkFileFilter for the File->Importer dialog
 * Returns:
 */
GtkFileFilter*
cong_importer_make_file_filter (CongServiceImporter *importer)
{
	g_return_val_if_fail (IS_CONG_SERVICE_IMPORTER (importer), NULL);

	g_assert (PRIVATE (importer)->filter_factory_callback);

	return PRIVATE (importer)->filter_factory_callback (importer);
}

/**
 * cong_importer_invoke:
 * @importer:
 * @filename:
 * @mime_type:
 * @toplevel_window:
 *
 * TODO: Write me
 */
void 
cong_importer_invoke (CongServiceImporter *importer, 
		      const gchar *filename, 
		      const gchar *mime_type, 
		      GtkWindow *toplevel_window)
{
	g_return_if_fail (IS_CONG_SERVICE_IMPORTER (importer));
	g_return_if_fail (filename);
	g_return_if_fail (mime_type);
	
	g_assert(PRIVATE (importer)->action_callback);

	return PRIVATE (importer)->action_callback (importer, 
						    filename, 
						    mime_type, 
						    PRIVATE (importer)->user_data, toplevel_window);
}


/**
 * cong_importer_make_basic_file_filter:
 * @importer: the importer
 *
 * Utility function for writing implementations of the filter factory callback.
 * Creates a file filter with the name set to the description text of the importer.
 */
GtkFileFilter*
cong_service_importer_make_basic_filter (CongServiceImporter *importer)
{
	GtkFileFilter *filter;
	
	g_return_val_if_fail (IS_CONG_SERVICE_IMPORTER (importer), NULL);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter,
				  cong_service_get_description ( CONG_SERVICE(importer)));

	return filter;
}
