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

#include "cong-plugin.h"

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


/**
 * cong_app_get_clipboard_xml_source:
 * @app: the #CongApp
 * @selection: should be either GDK_SELECTION_CLIPBOARD or GDK_SELECTION_PRIMARY
 * @target_doc: the #CongDocument into which the source is to be pasted
 * 
 * This function interrogates the appropriate clipboard and attempts to get the content
 * in the best possible format for the target document.  It may attempt to do conversions,
 * for example, converting <li> elements into <listitem> when pasting HTML into a DocBook document.
 *
 * Returns: a newly-allocated UTF-8 fragment of XML source, wrapped in a <placeholder> element,
 * suitable for parsing and adding to the document (which must then be freed using g_free) or NULL
 */
gchar*
cong_app_get_clipboard_xml_source (CongApp *app,
				   GdkAtom selection,
				   CongDocument *target_doc);


/**
 * cong_app_set_clipboard_from_xml_fragment:
 * @app: the #CongApp
 * @selection: should be either GDK_SELECTION_CLIPBOARD or GDK_SELECTION_PRIMARY
 * @xml_fragment: a fragment of XML source
 * @source_doc: the #CongDocument from which the source has been cut
 * 
 * This function takes the XML source and attempts to place it into the appropriate clipboard.  It will attempt to make it 
 * available in a number of formats, and in the best possible way for each format.  It may attempt to do conversions when it does 
 * this, e.g. generating pretty versions of bulleted lists for text, or converting to an HTML representation
 * where appropriate.
 *
 * The XML form of the source is not converted, but is wrapped in an <xml-fragment> top-level element and given a DOCTYPE declaration if available in the source document.
 * So it should be well-formed, but not valid.
 * Haven't yet specified what happens to entities.
 */
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
		   enum CongFontRole role);

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



