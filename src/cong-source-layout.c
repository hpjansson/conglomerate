/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-source-layout.c
 *
 * Tools for cleaning up the XML source of the document
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
#include "cong-document.h"
#include "cong-error-dialog.h"
#include "cong-dialog.h"
#include "cong-primary-window.h"
#include "cong-dispspec.h"
#include "cong-dispspec-element.h"
#include "cong-command.h"
#include "cong-util.h"
#include "cong-source-layout.h"

typedef struct CongSourceCleanupData CongSourceCleanupData;

struct CongSourceCleanupData
{
	CongCommand *cmd;
	const CongSourceCleanupOptions *options;
};

static gchar*
generate_indentation(const CongSourceCleanupOptions *options, guint indent_level)
{
	g_return_val_if_fail(options, NULL);

	if (indent_level>0) {
		indent_level--;
	}

	if (options->use_tabs) {
		return g_strnfill(indent_level, '\t');
	} else {
		return g_strnfill((indent_level*options->num_spaces_per_indent), ' ');
	}

}

/**
 * cong_util_is_recursively_inline:
 * @doc:
 * @node:
 *
 * TODO: Write me
 */
gboolean 
cong_util_is_recursively_inline (CongDocument *doc,
				 CongNodePtr node)
{
	CongNodePtr iter;

	g_return_val_if_fail (doc, FALSE);
	g_return_val_if_fail (node, FALSE);

	for (iter=node->children; iter; iter=iter->next) {
		switch (cong_node_type (iter)) {
		default: g_assert_not_reached();
		case CONG_NODE_TYPE_UNKNOWN:
			break;
			
		case CONG_NODE_TYPE_ELEMENT:
			{
				CongDispspecElement* ds_element = cong_document_get_dispspec_element_for_node(doc, 
													      iter);
				
				if (ds_element) {
					switch (cong_dispspec_element_type(ds_element)) {
					default: g_assert_not_reached();
					case CONG_ELEMENT_TYPE_STRUCTURAL:
						return FALSE;
						/* Insert indentation here: */
						
					case CONG_ELEMENT_TYPE_SPAN:
						break;
						
					case CONG_ELEMENT_TYPE_INSERT:
					case CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE:					
					case CONG_ELEMENT_TYPE_PLUGIN:
					case CONG_ELEMENT_TYPE_UNKNOWN:
						return FALSE;
					}
				} else {
					return FALSE;
				}
			}
			break;
			
		case CONG_NODE_TYPE_ATTRIBUTE:
			break;
			
		case CONG_NODE_TYPE_TEXT:
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
			return FALSE;
		}
	}
	
	
	return TRUE;
}

/**
 * cong_util_strip_whitespace_from_string:
 * @input_string:
 * @strip_all_initial_whitespace:
 *
 * TODO: Write me
 */
gchar* 
cong_util_strip_whitespace_from_string (const gchar* input_string, 
					gboolean strip_all_initial_whitespace)
{
	gunichar *unichar_string;
	glong num_chars;
	gchar *result_string;
	gchar *dst;
	int i;
	gboolean last_char_was_space=strip_all_initial_whitespace;

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

typedef struct _CongCleanupSourceUpdateLocationData {
	CongNodePtr node;
	
	/* Used in whitespace stripping */
	gchar *old_content;
	gchar *new_content;
	
	/* Used in indentation */
	gint indent;

} CongCleanupSourceUpdateLocationData;

static gboolean
strip_whitespace_update_location_callback (CongDocument *doc,
		    			   CongLocation *location, 
					   gpointer user_data)
{
	CongCleanupSourceUpdateLocationData *location_data = (CongCleanupSourceUpdateLocationData *) user_data;
	gchar *old_content, *new_content;
	gunichar old_char, new_next_char;
	gboolean valid = TRUE;

	g_assert (location_data->node);
	g_assert (cong_node_type (location_data->node)==CONG_NODE_TYPE_TEXT);
	
	old_content = location_data->old_content;
	new_content = location_data->new_content;

	if (location->node == location_data->node) {
		if (*new_content != ' ') {
			new_next_char = g_utf8_get_char (new_content);
			while (g_utf8_get_char (old_content) != new_next_char && *old_content != 0)
				old_content = g_utf8_next_char (old_content);
		}	
		
		while (*old_content != 0 && *new_content != 0 ) {

		    if (valid) {
			if (old_content - location_data->old_content >= location->byte_offset) 
			    break;
		    } else {
			valid = TRUE;    
		    }
		
		    if (*new_content == ' ') {
			new_next_char = g_utf8_get_char (new_content + 1);
			old_char = g_utf8_get_char (old_content);

			if (new_next_char != old_char) {
			    old_content = g_utf8_next_char (old_content);
			    valid = FALSE;
			} else {
			    new_content = new_content + 1;
			}
		    } else {
			    old_content = g_utf8_next_char (old_content);
			    new_content = g_utf8_next_char (new_content);
		    }
		} 

		location->byte_offset = new_content - location_data->new_content;	
		
		return TRUE;
	}

	return FALSE;
}

static gboolean strip_whitespace_callback(CongDocument *doc, CongNodePtr node, gpointer user_data, guint recursion_level)
{
	CongSourceCleanupData *cleanup_data = user_data;
	CongCleanupSourceUpdateLocationData update_location_data;

	if (cong_node_type(node)==CONG_NODE_TYPE_TEXT) {
		if (cong_node_get_whitespace_handling (doc, node)==CONG_WHITESPACE_NORMALIZE) {
			gboolean strip_all_initial_whitespace = TRUE;
			gchar *new_content;

			/* Don't strip all initial whitespace if this node was preceded by an span tag: */
			if (node->prev) {
				if (cong_node_type (node->prev)==CONG_NODE_TYPE_ELEMENT) {
					CongDispspecElement* ds_element = cong_document_get_dispspec_element_for_node(doc, 
														      node->prev);
				
					if (ds_element) {
						if (cong_dispspec_element_type(ds_element)==CONG_ELEMENT_TYPE_SPAN) {
							strip_all_initial_whitespace = FALSE;
						}
					}
				}
			}

			new_content = cong_util_strip_whitespace_from_string (node->content,
									      strip_all_initial_whitespace);
			
			update_location_data.node = node;
			update_location_data.old_content = node->content;
			update_location_data.new_content = new_content;

			cong_command_for_each_location (cleanup_data->cmd,
							strip_whitespace_update_location_callback, 
							&update_location_data);
			
			cong_command_add_node_set_text (cleanup_data->cmd,
							node,
							new_content);
			g_free(new_content);
		}
	}

	return FALSE;
}

static void 
add_indentation_and_cr_nodes (CongDocument *doc, 
			      CongNodePtr node,
			      CongSourceCleanupData *cleanup_data,
			      guint recursion_level)
{
	/* Add indentation before this element: */
	{
		gchar *indentation_text = generate_indentation (cleanup_data->options, recursion_level);
		CongNodePtr indentation_node = cong_node_new_text (indentation_text, doc);
		g_free (indentation_text);

		cong_command_add_node_add_before (cleanup_data->cmd,
						  indentation_node,
						  node);
	}

	/* Add a carriage return after this element: */
	{
		CongNodePtr cr_node = cong_node_new_text ("\n", doc);

		cong_command_add_node_add_after (cleanup_data->cmd,
						 cr_node,
						 node);
	}
}

static gboolean
add_indentation_update_location_callback (CongDocument *doc,
		    			   CongLocation *location, 
					     gpointer user_data)
{
	CongCleanupSourceUpdateLocationData *location_data = (CongCleanupSourceUpdateLocationData *) user_data;

	g_assert (location_data->node);
	g_assert (cong_node_type (location_data->node)==CONG_NODE_TYPE_TEXT);
	
	if (location->node == location_data->node) {
		location->byte_offset += location_data->indent;	
		return TRUE;
	}

	return FALSE;
}

static gboolean add_indentation_callback(CongDocument *doc, CongNodePtr node, gpointer user_data, guint recursion_level)
{
	CongSourceCleanupData *cleanup_data = user_data;
	const CongSourceCleanupOptions *options = cleanup_data->options;

	g_assert (doc);
	g_assert (options);
	g_assert (node);

	if (node->parent) {
		if (cong_util_is_recursively_inline (doc, node)) {
			if (cong_util_is_recursively_inline (doc, node->parent)) {
				return FALSE;
			}
		}
	}						

	switch (cong_node_type(node)) {
	default: g_assert_not_reached();
	case CONG_NODE_TYPE_UNKNOWN:
		break;

	case CONG_NODE_TYPE_ELEMENT:
		{
			g_assert(node->parent);
			
			{
				CongNodeType parent_type = cong_node_type (node->parent);
				if (parent_type!=CONG_NODE_TYPE_DOCUMENT) {
					add_indentation_and_cr_nodes (doc, 
								      node,
								      cleanup_data,
								      recursion_level);
				}
				
				if (node->children) {
					
					if (cong_util_is_recursively_inline (doc, node)) {
						/* 
						   Do nothing
						*/						
					} else {
						/* Add a carriage returns as the new first child, and a CR+indent as the new last child: */
						CongNodePtr new_first_child = cong_node_new_text ("\n", doc);
						gchar *indentation_text = generate_indentation (options, recursion_level);
						CongNodePtr new_last_child = cong_node_new_text (indentation_text, doc);
						g_free (indentation_text);

						cong_command_add_node_add_before (cleanup_data->cmd, new_first_child, node->children);
						cong_command_add_node_add_after (cleanup_data->cmd, new_last_child, node->last);
					}
				}
			}
		}
		break;

	case CONG_NODE_TYPE_ATTRIBUTE:
		break;

	case CONG_NODE_TYPE_TEXT:
		/* Clean up a text node, provided it's not one of the whitespace ones we've added ourselves: */
		if (!cong_util_is_pure_whitespace (node->content))
		{
			if (cong_util_is_recursively_inline (doc, node->parent)) {
				/* Do nothing: */
			} else {
				CongCleanupSourceUpdateLocationData update_location_data;
				
				gchar *indentation = generate_indentation (options, recursion_level);
				gchar *new_content = g_strconcat (indentation, node->content, NULL);
				
				cong_command_add_node_set_text (cleanup_data->cmd, node, new_content);
				
				update_location_data.node = node;
				update_location_data.indent = strlen (indentation);
				
				cong_command_for_each_location (cleanup_data->cmd,
		    						add_indentation_update_location_callback, 
	    							&update_location_data);
				g_free (indentation);
				g_free (new_content);
			}
		}
		break;

	case CONG_NODE_TYPE_CDATA_SECTION:
	case CONG_NODE_TYPE_ENTITY_REF:
	case CONG_NODE_TYPE_ENTITY_NODE:
		break;

	case CONG_NODE_TYPE_PI:
	case CONG_NODE_TYPE_COMMENT:
		add_indentation_and_cr_nodes (doc, 
					      node,
					      cleanup_data,
					      recursion_level);
		break;
		
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

void 
cong_command_add_cleanup_source (CongCommand *command,
				 const CongSourceCleanupOptions *options)
{
	CongDocument *doc;
	CongSourceCleanupData cleanup_data;
	
	doc = cong_command_get_document (command);

	/* FIXME: knowledge about where whitespace should be preserved? */
 	/* Traverse the document:
	   - every text node should have its internal whitespace cleaned, then begin/end with an appropriate amount of indentation
	   - every element node inside another element node should have whitespace containing a carriage return and then indentation...
	*/

	cleanup_data.options = options;
	cleanup_data.cmd = command;

	/* Stage 1:  strip out all non-significant whitespace: */
	cong_document_for_each_node (doc, strip_whitespace_callback, &cleanup_data);

	/* Stage 2:  add back whitespace to indicate the structure of the document: */
	cong_document_for_each_node (doc, add_indentation_callback, &cleanup_data);

	/* Stage 3: merge adjacent text nodes: */
	cong_command_add_merge_adjacent_text_nodes (cleanup_data.cmd);
}

void 
cong_util_cleanup_source (CongDocument *doc, 
			  const CongSourceCleanupOptions *options)
{
	CongCommand *command;

	g_return_if_fail(doc);
	g_return_if_fail(options);

	command = cong_document_begin_command (doc,
					       _("Cleanup XML Source"),
					       NULL);

	cong_command_add_cleanup_source (command,
					 options);

	cong_document_end_command (doc,
				   command);
}

