/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * xmlload.c
 *
 * Copyright (C) 2002 David Malcolm
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

/*
  xmlload.c
  
  This traverses the libxml structures below a xmlDocPtr and generates a TTREE*
  corresponding to that which would have been created by the old flux xml loaders.
  
  The code is a hybrid of the libxml tree traversal code in
  libxml's xmlDebugDumpDocument, and the TTREE creation on code found in
  flux-0.2.8/src/xml/xmltree-rxp.c.
*/

#include <glib.h>
#include <libxml/tree.h>

#include "global.h" 
/* purely to get at build settings */

#if 0
#include <ttree.h>

void
convert_libxml_to_ttree_node_list(TTREE *tt, xmlNodePtr node);

TTREE*
convert_libxml_to_ttree_node(TTREE *tt, xmlNodePtr node)
{
	TTREE* new_tt = NULL;

	if (node == NULL) {
		return NULL;
	}

	switch (node->type) {
	case XML_ELEMENT_NODE:
		{
			TTREE *node_name;
			/* Create TTREE for element: */
			new_tt = ttree_node_add(tt, "tag_span", 8);
			node_name = ttree_node_add(new_tt, 
						   node->name,
						   strlen(node->name));
			
			/* Add attributes */
			if (node->properties != NULL) {
				xmlAttrPtr attr = node->properties;
				
				while (attr != NULL) {
					TTREE *node_cur = ttree_node_add(node_name, "attr", 4);
					node_cur = ttree_node_add(node_cur, 
								  attr->name,
								  strlen(attr->name));
					
					if (attr->children != NULL) {
						g_assert(attr->children->type==XML_TEXT_NODE);
						
						ttree_node_add(node_cur, attr->children->content, strlen(attr->children->content));
					}     
					
					attr = attr->next;
				}
			}
			
			/* Recursively process children */
			if ((node->children != NULL) && (node->type != XML_ENTITY_REF_NODE)) {
				convert_libxml_to_ttree_node_list(node_name, node->children);
			}
		}
		break;
		
	case XML_TEXT_NODE:
		{
			if (node->content != NULL) {
				new_tt = ttree_node_add(tt, "data", 4);
				ttree_node_add(new_tt, node->content, strlen(node->content));
			}
		}
		break;
		
	case XML_COMMENT_NODE:
		{
			if (node->content != NULL) {
				new_tt = ttree_node_add(tt, "comment", 7);
				ttree_node_add(new_tt, node->content, strlen(node->content));
			}
		}
		break;
		
	default:
	}

	return new_tt;
}

void
convert_libxml_to_ttree_node_list(TTREE *tt, xmlNodePtr node)
{
	while (node != NULL) {
		convert_libxml_to_ttree_node(tt, node);
		node = node->next;
	}
}

TTREE*
convert_libxml_to_ttree_doc(xmlDocPtr doc)
{
	TTREE *node_root = NULL;
	if (doc == NULL) {
		return NULL;
	}
	
	if (((doc->type == XML_DOCUMENT_NODE) ||
	     (doc->type == XML_HTML_DOCUMENT_NODE)) && (doc->children != NULL)) {
		node_root = ttree_node_add(NULL, "xml", 3);
		convert_libxml_to_ttree_node_list(node_root, doc->children);
	}
	
	return node_root;
}
#endif
