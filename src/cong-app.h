/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-app.h
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
 */

#ifndef __CONG_APP_H__
#define __CONG_APP_H__

G_BEGIN_DECLS

typedef struct CongApp CongApp;
typedef struct CongAppPrivate CongAppPrivate;

struct CongApp
{
	GtkWidget *popup;
	GList *primary_windows;

	CongAppPrivate *private;
};

/* Accessing the singleton; (asserts internally that it has been constructed): */
CongApp*
cong_app_singleton(void);

void
cong_app_construct_singleton(int   argc,
			     char *argv[]);

void
cong_app_destroy_singleton(void);

int
cong_app_post_init_hack (CongApp *app);


gchar*
cong_app_get_clipboard_xml_source (CongApp *app,
				   GdkAtom selection,
				   CongDocument *target_doc);


void
cong_app_set_clipboard_from_xml_fragment (CongApp *app,
					  GdkAtom selection,
					  const gchar* xml_fragment,
					  CongDocument *source_doc);

GnomeProgram*
cong_app_get_gnome_program (CongApp *app);

GtkTooltips*
cong_app_get_tooltips (CongApp *app);

CongFont*
cong_app_get_font (CongApp *app,
		   CongFontRole role);

CongPluginManager*
cong_app_get_plugin_manager (CongApp *app);

CongDispspecRegistry*
cong_app_get_dispspec_registry (CongApp *app);

GConfClient*
cong_app_get_gconf_client (CongApp *app);

const GList*
cong_app_get_language_list (CongApp *app);

G_END_DECLS

#endif



