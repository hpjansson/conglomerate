/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-css.c
 *
 * CSS plugin
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

#include "cong-fake-plugin-hooks.h"

#if ENABLE_SEWFOX
#include "cong-app.h"
#include "cong-primary-window.h"

#include <sewfox/sx-box-view.h>

gboolean 
css_doc_filter (CongServiceDocTool *doc_tool, 
		CongDocument *doc, 
		gpointer user_data)
{
	return TRUE;
}

static CRStyleSheet *
load_stylesheet (const gchar *filename) 
{
        enum CRStatus status;
	CRStyleSheet *sheet;
	status = cr_om_parser_simply_parse_file	(filename, 
						 CR_ASCII, 
						 &sheet);
	if (status == CR_OK) {
		return sheet;
	} else {
		return NULL;
	}
}
	
static 
CRCascade*
make_cascade (void)
{
        CRCascade *cascade;
	gchar *css_filename = gnome_program_locate_file (cong_app_get_gnome_program (cong_app_singleton()),
							 GNOME_FILE_DOMAIN_APP_DATADIR,
							 "conglomerate/css/docbook.css", /* FIXME: why do I need the conglomerate prefix? */
							 FALSE,
							 NULL);
	g_message (css_filename);

        cascade = cr_cascade_new (load_stylesheet (css_filename),
                                  NULL,
                                  NULL);
	g_free (css_filename);

	return cascade;
}

void 
css_action_callback (CongServiceDocTool *doc_tool, 
		     CongPrimaryWindow *primary_window, 
		     gpointer user_data)
{
	CRCascade *cascade;	
	SXBoxView *box_view;
	GtkWidget *scroll;

	cascade = make_cascade ();	

	box_view = sx_box_view_new (cascade, 
				    cong_document_get_xml (cong_primary_window_get_document (primary_window)));
	gtk_widget_show (GTK_WIDGET (box_view));

        scroll = gtk_scrolled_window_new (NULL, NULL);
        gtk_container_add (GTK_CONTAINER (scroll), 
			   GTK_WIDGET (box_view));	

	cong_util_show_in_window (GTK_WIDGET (scroll),
				  "foo");
}
#endif

/* would be exposed as "plugin_register"? */
/**
 * plugin_css_plugin_register:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_css_plugin_register (CongPlugin *plugin)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), FALSE);

#if ENABLE_SEWFOX
	cong_plugin_register_doc_tool(plugin, 
				      "View with stylesheet",
				      "View with a CSS stylesheet applied to the document.",
				      "cleanup",
				      "_CSS test view",
				      NULL,
				      NULL,
				      css_doc_filter,
				      css_action_callback,
				      NULL);
#endif

	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
/**
 * plugin_css_plugin_configure:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_css_plugin_configure (CongPlugin *plugin)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), FALSE);

	return TRUE;
}
