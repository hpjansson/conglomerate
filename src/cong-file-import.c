/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-file-import.c
 *
 * Copyright (C) 2003 David Malcolm
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
 * Based on code by Hans Petter Jansson <hpj@ximian.com>
 */

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "global.h"
#include "../config.h"
#include "cong-dispspec.h"
#include "cong-document.h"
#include "cong-error-dialog.h"
#include "cong-dialog.h"
#include "cong-app.h"
#include "cong-eel.h"
#include <glade/glade-xml.h>
#include "cong-vfs.h"
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#include "cong-file-selection.h"
#include "cong-service-importer.h"
#include "cong-plugin-manager.h"
#include <libgnomevfs/gnome-vfs-mime-handlers.h>

static void add_importer_to_list(CongServiceImporter *importer, gpointer user_data)
{
	GList **list_of_filters = user_data;
	GtkFileFilter *filter = cong_importer_make_file_filter (importer);

	*list_of_filters = g_list_append (*list_of_filters, 
					  filter);

	g_object_set_data (G_OBJECT (filter),
			   "importer",
			   importer);					 
}


/**
 * cong_ui_hook_file_import:
 * @toplevel_window:
 *
 * TODO: Write me
 */
void
cong_ui_hook_file_import (GtkWindow *toplevel_window)
{
	gchar *filename;
	GList *list_of_filters = NULL;
	GtkFileFilter *filter = NULL;
	GList *iter;

	cong_plugin_manager_for_each_importer( cong_app_get_plugin_manager (cong_app_singleton()), 
					       add_importer_to_list, 
					       &list_of_filters);

	for (iter=list_of_filters;iter;iter=iter->next) {
		g_object_ref (G_OBJECT (iter->data));
	}

	filename = cong_get_file_name_with_filter (_("Import file..."), 
						   NULL, 
						   toplevel_window,
						   CONG_FILE_CHOOSER_ACTION_OPEN,
						   list_of_filters,
						   &filter);

	if (filename) {
		CongServiceImporter *importer;
		char* mime_type = gnome_vfs_get_mime_type (filename);

		g_assert (filter);
		importer = g_object_get_data (G_OBJECT (filter),
					      "importer");
		g_assert (importer);

		cong_importer_invoke (importer, 
				      filename, 
				      mime_type, 
				      toplevel_window);
		g_free (mime_type);
		g_free (filename);
	}

	for (iter=list_of_filters;iter;iter=iter->next) {
		g_object_unref (G_OBJECT (iter->data));
	}
}
