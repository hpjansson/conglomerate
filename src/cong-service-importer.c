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
	CongServiceImporterMimeFilter mime_filter;
	CongServiceImporterActionCallback action_callback;
	gpointer user_data;
};

CONG_DEFINE_CLASS (CongServiceImporter, cong_service_importer, CONG_SERVICE_IMPORTER, CongService, CONG_SERVICE_TYPE)

CongServiceImporter*
cong_service_importer_construct (CongServiceImporter *importer,
				 const gchar *name, 
				 const gchar *description,
				 const gchar *id,
				 CongServiceImporterMimeFilter mime_filter,
				 CongServiceImporterActionCallback action_callback,
				 gpointer user_data)
{
	g_return_val_if_fail (IS_CONG_SERVICE_IMPORTER (importer), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (id, NULL);
	g_return_val_if_fail (mime_filter, NULL);
	g_return_val_if_fail (action_callback, NULL);

	cong_service_construct (CONG_SERVICE (importer),
				name,
				description,
				id);
		
	PRIVATE (importer)->mime_filter = mime_filter;
	PRIVATE (importer)->action_callback = action_callback;
	PRIVATE (importer)->user_data = user_data;

	return importer;
}

/* Implementation of CongServiceImporter: */
gboolean cong_importer_supports_mime_type(CongServiceImporter *importer, const gchar *mime_type)
{
	g_return_val_if_fail (IS_CONG_SERVICE_IMPORTER (importer), FALSE);
	g_return_val_if_fail (mime_type, FALSE);

	g_assert (PRIVATE (importer)->mime_filter);

	return PRIVATE (importer)->mime_filter (importer, 
						mime_type, 
						PRIVATE (importer)->user_data);

}

void cong_importer_invoke(CongServiceImporter *importer, const gchar *filename, const gchar *mime_type, GtkWindow *toplevel_window)
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

