/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-admonition-node-element.c
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
#include "plugin-admonition-node-element.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"
#include "cong-util.h"
#include "cong-enum-mapping.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-dispspec-element.h"

#include "cong-editor-area-pixbuf.h"
#include "cong-editor-area-labelled.h"

#include "cong-app.h"

CONG_EDITOR_NODE_DEFINE_PLUGIN_SUBCLASS_SPECIAL(Admonition, admonition, CONG_EDITOR_NODE_ELEMENT_ADMONITION)

static const gchar*
get_icon_filename (CongEditorNodeElementAdmonition *editor_node_element_admonition);

static GdkPixbuf*
load_icon (const gchar *icon_filename);

static CongEditorArea*
create_block_area (CongEditorNodeElementAdmonition *editor_node_element_admonition)
{
	CongEditorArea *area_label;
	GdkPixbuf* pixbuf;

	/* FIXME cache the pixbufs; only load once */
	pixbuf = load_icon (get_icon_filename (editor_node_element_admonition));

	area_label = cong_editor_area_pixbuf_new (cong_editor_node_get_widget (CONG_EDITOR_NODE (editor_node_element_admonition)),
						  pixbuf);
	g_object_unref (G_OBJECT (pixbuf));

	return cong_editor_area_labelled_new (cong_editor_node_get_widget (CONG_EDITOR_NODE (editor_node_element_admonition)),
					      area_label);
}

static const gchar*
get_icon_filename (CongEditorNodeElementAdmonition *editor_node_element_admonition)
{
	CongDispspecElement *element;

	element = cong_editor_node_element_get_dispspec_element (CONG_EDITOR_NODE_ELEMENT (editor_node_element_admonition));

	return cong_dispspec_element_get_value_for_key ("icon", element);
}

static GdkPixbuf*
load_icon (const gchar *icon_filename)
{
	gchar *full_path;
	gchar *modified_icon_filename; /* FIXME bugzilla 136287 */
	GdkPixbuf *pixbuf;

	g_return_val_if_fail(icon_filename, NULL);

	modified_icon_filename = g_strconcat(PACKAGE_NAME,"/",icon_filename, NULL);
	full_path = gnome_program_locate_file (cong_app_get_gnome_program (cong_app_singleton()),
					       GNOME_FILE_DOMAIN_APP_PIXMAP,
					       modified_icon_filename,
					       FALSE,
					       NULL);

	g_message ("Trying to load \"%s\"", full_path);

	pixbuf = gdk_pixbuf_new_from_file(full_path, NULL);
	
	g_free(full_path);
	g_free(modified_icon_filename);

	return pixbuf;
}
