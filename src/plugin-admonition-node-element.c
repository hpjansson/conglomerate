/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-admonition-node-element.c
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
 * Fragments of code based upon libxslt: numbers.c
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


#define PRIVATE(x) ((x)->private)

struct CongEditorNodeElementAdmonitionDetails
{
	int dummy;
};

/* Internal function declarations: */
static void
finalize (GObject *object);

static void
dispose (GObject *object);

static CongEditorArea*
generate_block_area (CongEditorNode *editor_node);

static const gchar*
get_icon_filename (CongEditorNodeElementAdmonition *editor_node_element_admonition);

static GdkPixbuf*
load_icon (const gchar *icon_filename);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeElementAdmonition, 
			cong_editor_node_element_admonition,
			CongEditorNodeElement,
			CONG_EDITOR_NODE_ELEMENT_TYPE );

static void
cong_editor_node_element_admonition_class_init (CongEditorNodeElementAdmonitionClass *klass)
{
	CongEditorNodeClass *node_klass = CONG_EDITOR_NODE_CLASS(klass);

	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	node_klass->generate_block_area = generate_block_area;
}

static void
cong_editor_node_element_admonition_instance_init (CongEditorNodeElementAdmonition *node_element_admonition)
{
	node_element_admonition->private = g_new0(CongEditorNodeElementAdmonitionDetails,1);
}

/**
 * cong_editor_node_element_admonition_construct:
 * @editor_node_element_admonition:
 * @editor_widget:
 * @traversal_node:
 *
 * TODO: Write me
 */
CongEditorNodeElementAdmonition*
cong_editor_node_element_admonition_construct (CongEditorNodeElementAdmonition *editor_node_element_admonition,
					       CongEditorWidget3* editor_widget,
					       CongTraversalNode *traversal_node)
{
	cong_editor_node_element_construct (CONG_EDITOR_NODE_ELEMENT (editor_node_element_admonition),
					    editor_widget,
					    traversal_node);

	return editor_node_element_admonition;
}

/**
 * cong_editor_node_element_admonition_new:
 * @widget:
 * @traversal_node:
 *
 * TODO: Write me
 */
CongEditorNode*
cong_editor_node_element_admonition_new (CongEditorWidget3* widget,
					 CongTraversalNode *traversal_node)
{
#if DEBUG_EDITOR_NODE_LIFETIMES
	g_message("cong_editor_node_element_admonition_new(%s)", node->name);
#endif

	return CONG_EDITOR_NODE( cong_editor_node_element_admonition_construct
				 (g_object_new (CONG_EDITOR_NODE_ELEMENT_ADMONITION_TYPE, NULL),
				  widget,
				  traversal_node));
}
/* Internal function definitions: */
static void
finalize (GObject *object)
{
	CongEditorNodeElementAdmonition *editor_node_element_admonition = CONG_EDITOR_NODE_ELEMENT_ADMONITION (object);

	g_free (editor_node_element_admonition->private);
	editor_node_element_admonition->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);

}

static void
dispose (GObject *object)
{
	CongEditorNodeElementAdmonition *editor_node_element_admonition = CONG_EDITOR_NODE_ELEMENT_ADMONITION (object);

	g_assert (editor_node_element_admonition->private);
	
	/* Cleanup: */

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static CongEditorArea*
generate_block_area (CongEditorNode *editor_node)
{
	CongEditorArea *new_area;
	CongEditorNodeElementAdmonition *editor_node_element_admonition = CONG_EDITOR_NODE_ELEMENT_ADMONITION (editor_node);
	CongEditorArea *area_label;
	GdkPixbuf* pixbuf;

	g_return_val_if_fail (editor_node, NULL);

	/* FIXME cache the pixbufs; only load once */
	pixbuf = load_icon (get_icon_filename (editor_node_element_admonition));

	area_label = cong_editor_area_pixbuf_new (cong_editor_node_get_widget (editor_node),
						  pixbuf);

	g_object_unref (G_OBJECT (pixbuf));	

	new_area = cong_editor_area_labelled_new (cong_editor_node_get_widget (editor_node),
						  area_label);

	cong_editor_area_connect_node_signals (new_area,
					       editor_node);

	return new_area;
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
