/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-cleanup-source.c
 *
 * Tools for cleaning up the XML source of the document
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

#include "global.h"
#include "cong-plugin.h"
#include "cong-document.h"
#include "cong-error-dialog.h"
#include "cong-dialog.h"

#include "cong-fake-plugin-hooks.h"

static gboolean doc_filter(CongTool *tool, CongDocument *doc, gpointer user_data)
{
	/* Always appropriate: */
	return TRUE;
}


typedef struct CongSourceCleanupOptions CongSourceCleanupOptions;

struct CongSourceCleanupOptions
{
	gboolean use_tabs; /* if false then use spaces */
	guint num_spaces_per_indent; /* only if not using tabs */

	gboolean wrap_text; /* if true then wrap to a column */
	guint num_text_columns; /* only relevant if wrap_text is true */
};



static gchar*
generate_indentation(const CongSourceCleanupOptions *options, guint indent_level)
{
	g_return_val_if_fail(options, NULL);

	if (options->use_tabs) {
		return g_strnfill(indent_level, '\t');
	} else {
		return g_strnfill((indent_level*options->num_spaces_per_indent), ' ');
	}

}

/* Return TRUE to stop the traversal */
typedef gboolean (*CongDocumentRecursionCallback)(CongDocument *doc, CongNodePtr node, gpointer user_data, guint recursion_level);

/* Return TRUE if the traversal was stopped prematurely */
gboolean cong_document_for_each_node(CongDocument *doc, CongDocumentRecursionCallback callback, gpointer callback_data);


gboolean cong_util_is_pure_whitespace (const gchar *utf8_text)
{
	gunichar ch;

	g_return_val_if_fail(utf8_text, FALSE);

	while (ch = g_utf8_get_char(utf8_text)) {
		if (!g_unichar_isspace(ch)) {
			return FALSE;
		}

		utf8_text = g_utf8_next_char(utf8_text);
	}
	
	return TRUE;
}

gchar* cong_util_strip_whitespace_from_string (const gchar* input_string)
{
	gunichar *unichar_string;
	glong num_chars;
	gchar *result_string;
	gchar *dst;
	int i;
	gboolean last_char_was_space=TRUE;

	g_return_val_if_fail(input_string, NULL);

	unichar_string = g_utf8_to_ucs4_fast(input_string,
                                             -1,
                                             &num_chars);

	result_string = g_malloc((num_chars*8)+1);

	dst = result_string;

	for (i=0;i<num_chars;i++) {
		gunichar c = unichar_string[i];
		gboolean this_char_is_space = g_unichar_isspace(c);

		if (this_char_is_space) {

			if (!last_char_was_space) {
				
				/* Write a space into the buffer: */
				*(dst++) = ' ';
			}

		} else {

			/* Write character as utf-8 into buffer: */
			dst += g_unichar_to_utf8(c, dst);
		}
		
		last_char_was_space = this_char_is_space;
	}

	g_free(unichar_string);

	/* Terminate the string: */
	*dst = '\0';

	return result_string;	
}

static gboolean cong_document_for_each_node_recurse(CongDocument *doc, CongNodePtr node, CongDocumentRecursionCallback callback, gpointer user_data, guint recursion_level)
{
	g_assert(doc);
	g_assert(node);
	g_assert(callback);

	if ((*callback)(doc, node, user_data, recursion_level)) {
		return TRUE;
	}
	    
	/* Recurse over children: */
	{
		CongNodePtr child_iter;

		for (child_iter = node->children; child_iter; child_iter=child_iter->next) {
			if (cong_document_for_each_node_recurse(doc, child_iter, callback, user_data, recursion_level+1)) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

gboolean cong_document_for_each_node(CongDocument *doc, CongDocumentRecursionCallback callback, gpointer user_data)
{
	g_return_val_if_fail (doc, TRUE);
	g_return_val_if_fail (callback, TRUE);

	return cong_document_for_each_node_recurse (doc,
						    (CongNodePtr)cong_document_get_xml(doc), 
						    callback, 
						    user_data, 
						    0);
}



static gboolean strip_whitespace_callback(CongDocument *doc, CongNodePtr node, gpointer user_data, guint recursion_level)
{
	if (cong_node_type(node)==CONG_NODE_TYPE_TEXT) {
		gchar *new_content = cong_util_strip_whitespace_from_string(node->content);
		cong_document_node_set_text (doc, node, new_content);
		g_free(new_content);
	}

	return FALSE;
}


static gboolean add_indentation_callback(CongDocument *doc, CongNodePtr node, gpointer user_data, guint recursion_level)
{
	const CongSourceCleanupOptions *options = user_data;

	g_assert (doc);
	g_assert (options);
	g_assert (node);

/* 	g_message("cleanup_source_callback(%s)", cong_node_debug_description(node)); */

	switch (cong_node_type(node)) {
	default: g_assert_not_reached();
	case CONG_NODE_TYPE_UNKNOWN:
		break;

	case CONG_NODE_TYPE_ELEMENT:
		{
#if 0
			CongDispspecElement* ds_element = cong_document_get_dispspec_element_for_node(doc, node);

			if (ds_element) {
				switch (cong_dispspec_element_type(ds_element)) {
				default: g_assert_not_reached();
				case CONG_ELEMENT_TYPE_STRUCTURAL:
					/* Insert indentation here: */
					
					#error
				case CONG_ELEMENT_TYPE_SPAN:
				case CONG_ELEMENT_TYPE_INSERT:					
				case CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE:					
				case CONG_ELEMENT_TYPE_PARAGRAPH:					
				case CONG_ELEMENT_TYPE_PLUGIN:					
				case CONG_ELEMENT_TYPE_UNKNOWN:
					break;
				}
			} else {
				#error
			}
#else
			g_assert(node->parent);
			
			{
				enum CongNodeType parent_type = cong_node_type (node->parent);
				if (parent_type!=CONG_NODE_TYPE_DOCUMENT) {

					/* Add indentation before this element: */
					{
						gchar *indentation_text = generate_indentation (options, recursion_level);
						CongNodePtr indentation_node = cong_node_new_text (indentation_text, doc);
						g_free (indentation_text);
						
						cong_document_node_add_before (doc, indentation_node, node);
					}
#if 1
					/* Add a carriage return after this element: */
					{
						CongNodePtr cr_node = cong_node_new_text ("\n", doc);
						cong_document_node_add_after (doc, cr_node, node);
					}
#endif

				}
				
				/* If this node has children; add a carriage returns as the new first child, and a CR+indent as the new last child: */
				if (node->children) {
					CongNodePtr new_first_child = cong_node_new_text ("\n", doc);
					gchar *indentation_text = generate_indentation (options, recursion_level);
#if 1
					CongNodePtr new_last_child = cong_node_new_text (indentation_text, doc);
#else
					gchar *new_last_child_text = g_strdup_printf ("\n%s", indentation_text);
					CongNodePtr new_last_child = cong_node_new_text (new_last_child_text, doc);
					g_free (new_last_child_text);
#endif
					g_free (indentation_text);
						
					cong_document_node_add_before (doc, new_first_child, node->children);
					cong_document_node_add_after (doc, new_last_child, node->last);
				}
			}

#endif
		}
		break;

	case CONG_NODE_TYPE_ATTRIBUTE:
		break;

	case CONG_NODE_TYPE_TEXT:
		/* Clean up a text node, provided it's not one of the whitespace ones we've added ourselves: */
		if (!cong_util_is_pure_whitespace (node->content))
		{
			gchar *indentation = generate_indentation (options, recursion_level);
			gchar *new_content = g_strdup_printf ("%s%s\n", indentation, node->content);

			cong_document_node_set_text (doc, node, new_content);

			g_free (indentation);
			g_free (new_content);
		}
		break;

	case CONG_NODE_TYPE_CDATA_SECTION:
	case CONG_NODE_TYPE_ENTITY_REF:
	case CONG_NODE_TYPE_ENTITY_NODE:
	case CONG_NODE_TYPE_PI:
	case CONG_NODE_TYPE_COMMENT:
	case CONG_NODE_TYPE_DOCUMENT:
	case CONG_NODE_TYPE_DOCUMENT_TYPE:
	case CONG_NODE_TYPE_DOCUMENT_FRAG:
	case CONG_NODE_TYPE_NOTATION:
	case CONG_NODE_TYPE_HTML_DOCUMENT:
	case CONG_NODE_TYPE_DTD:
	case CONG_NODE_TYPE_ELEMENT_DECL:
	case CONG_NODE_TYPE_ATRRIBUTE_DECL:
	case CONG_NODE_TYPE_ENTITY_DECL:
	case CONG_NODE_TYPE_NAMESPACE_DECL:
	case CONG_NODE_TYPE_XINCLUDE_START:
	case CONG_NODE_TYPE_XINCLUDE_END:
		break;
	}

	/* Keep going: */
	return FALSE;
}

static void cong_util_cleanup_source(CongDocument *doc, const CongSourceCleanupOptions *options)
{
	g_return_if_fail(doc);
	g_return_if_fail(options);

#if 1
	/* FIXME: knowledge about where whitespace should be preserved? */
 	/* Traverse the document:
	   - every text node should have its internal whitespace cleaned, then begin/end with an appropriate amount of indentation
	   - every element node inside another element node should have whitespace containing a carriage return and then indentation...
	*/

	/* Stage 1:  strip out all non-significant whitespace: */
	cong_document_for_each_node(doc, strip_whitespace_callback, &options);

	/* Stage 2:  add back whitespace to indicate the structure of the document: */
	cong_document_for_each_node(doc, add_indentation_callback, &options);
#else
	CONG_DO_UNIMPLEMENTED_DIALOG(primary_window, "Cleanup XML source");
#endif
}

static void action_callback(CongTool *tool, CongPrimaryWindow *primary_window, gpointer user_data)
{
	CongSourceCleanupOptions options;

	/* FIXME: present a dialog box; use GConf etc */
	options.use_tabs = TRUE;
	options.wrap_text = TRUE;
	options.num_text_columns = 80;

	cong_util_cleanup_source(cong_primary_window_get_document(primary_window), &options);
}

 /* would be exposed as "plugin_register"? */
gboolean plugin_cleanup_source_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	cong_plugin_register_tool(plugin, 
				  _("Cleanup XML Source"),
				  _("Cleans up the XML source of the document, so that it is easy to read in a code editor."),
				  "cleanup",
				  _("_Clean the XML Source"),
				  _("Cleans up the XML source of the document, so that it is easy to read in a code editor."),
				  _("Cleans up the XML source of the document, so that it is easy to read in a code editor."),
				  doc_filter,
				  action_callback,
				  NULL);

	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
gboolean plugin_cleanup_source_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
