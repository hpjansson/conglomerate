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
	GnomeProgram *gnome_program;

	CongPluginManager *plugin_manager;

	GList *primary_windows;

	CongFont *fonts[CONG_FONT_ROLE_NUM];

	GdkGC *insert_element_gc;

	CongDispspecRegistry* ds_registry;

	GtkWidget *popup;

	GConfClient* gconf_client;

	GtkTooltips *tooltips;

	CongNodePtr clipboard;

	CongAppPrivate *private;
};

/* Accessing the singleton; (asserts internally that it has been constructed): */
CongApp*
cong_app_singleton(void);

void
cong_app_construct_singleton(void);

void
cong_app_destroy_singleton(void);


#if 0
const gchar*
cong_app_get_clipboard (CongApp *app);

void
cong_app_set_clipboard (CongApp *app, 
			const gchar* text);
#endif



G_END_DECLS

#endif



