/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-relaxng.c
 *
 * Plugin for manipulating RELAX NG files
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
#include "cong-error-dialog.h"
#include "cong-dispspec.h"

#include "cong-fake-plugin-hooks.h"

#include "cong-primary-window.h"

#include "cong-vfs.h"
#include "cong-util.h"
#include "cong-dtd.h"

#define RELAX_NG_NS_URI ("http://relaxng.org/ns/structure/1.0")

gboolean 
doc_filter_is_rng (CongServiceDocTool *doc_tool, 
		   CongDocument *doc, 
		   gpointer user_data)
{
	g_return_val_if_fail (IS_CONG_SERVICE_DOC_TOOL (doc_tool), FALSE);
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), FALSE);

	CongNodePtr root;

	g_return_val_if_fail(doc, FALSE);

	root = cong_document_get_root(doc);

	/* FIXME: what are the valid roots of a RELAX NG document? 
	   We only look at the namespace for the moment.
	 */
	if (cong_util_ns_uri_equality (RELAX_NG_NS_URI, cong_node_get_ns_uri (root))) {
		return TRUE;
	}

	return FALSE;
}



static void
regenerate_rnc (CongDocument *doc,
		GtkTextBuffer *text_buffer)
{
	xmlChar *doc_txt_ptr;
	int doc_txt_len;

	xmlDocDumpFormatMemoryEnc (cong_document_get_xml (doc),
				   &doc_txt_ptr,
				   &doc_txt_len, 
				   "UTF-8",
#if 1
				   0);
#else
				   (details->format ? 1 : 0));
#endif
	
	gtk_text_buffer_set_text (GTK_TEXT_BUFFER(text_buffer),
				  doc_txt_ptr,
				  doc_txt_len);
	
	xmlFree (doc_txt_ptr);

/* FIXME: need to implement dumping the XML as compact RNG */
#if 1
	gtk_text_buffer_set_text (GTK_TEXT_BUFFER (text_buffer),
				 "fubar",
				 -1);
#endif
}

static GtkWidget*
make_rnc_view (CongDocument *doc)
{
#if 1
	return cong_source_view_new_full (doc,
					  "text/rnc", /* FIXME: check this type! */
					  regenerate_rnc);
#else
	return gtk_label_new ("fubar");
#endif
}

void 
view_as_rnc_action_callback (CongServiceDocTool *doc_tool, 
			     CongPrimaryWindow *primary_window, 
			     gpointer user_data)
{
	CongDocument *doc;
	GtkWidget *rnc_view;
	

	g_return_if_fail (IS_CONG_SERVICE_DOC_TOOL (doc_tool));
	g_return_if_fail (primary_window);

	doc = cong_primary_window_get_document (primary_window);	
	rnc_view = make_rnc_view (doc);

	cong_util_show_in_window (rnc_view,
				  _("Compact Syntax View"));
}

/* would be exposed as "plugin_register"? */
gboolean 
plugin_relaxng_plugin_register (CongPlugin *plugin)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), FALSE);

	/* FIXME: CVS version of libxml2 has this function:

	   xmlDocPtr
	   xmlConvertCRNG (const char *schemas, int len, const char *encoding);

	   which could be wrapped as an importer.
	*/
#if 0
	cong_plugin_register_doc_tool (plugin,
				       _("View as Compact RELAX NG"),
				       _("Display the compact syntax form of a RELAX NG schema"),
				       "view-as-rnc",
				       _("View as Compact RELAX NG"),
				       NULL,
				       NULL,
				       doc_filter_is_rng,
				       view_as_rnc_action_callback,
				       NULL);
#endif

	
	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean 
plugin_relaxng_plugin_configure (CongPlugin *plugin)
{
	g_return_val_if_fail (IS_CONG_PLUGIN (plugin), FALSE);

	return TRUE;
}
