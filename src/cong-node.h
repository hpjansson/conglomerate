/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-node.h
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

#ifndef __CONG_NODE_H__
#define __CONG_NODE_H__

G_BEGIN_DECLS

typedef enum
{
	CONG_NODE_TYPE_UNKNOWN,

	CONG_NODE_TYPE_ELEMENT,
	CONG_NODE_TYPE_ATTRIBUTE,
	CONG_NODE_TYPE_TEXT,
	CONG_NODE_TYPE_CDATA_SECTION,
	CONG_NODE_TYPE_ENTITY_REF,
	CONG_NODE_TYPE_ENTITY_NODE,
	CONG_NODE_TYPE_PI,
	CONG_NODE_TYPE_COMMENT,
	CONG_NODE_TYPE_DOCUMENT,
	CONG_NODE_TYPE_DOCUMENT_TYPE,
	CONG_NODE_TYPE_DOCUMENT_FRAG,
	CONG_NODE_TYPE_NOTATION,
	CONG_NODE_TYPE_HTML_DOCUMENT,
	CONG_NODE_TYPE_DTD,
	CONG_NODE_TYPE_ELEMENT_DECL,
	CONG_NODE_TYPE_ATRRIBUTE_DECL,
	CONG_NODE_TYPE_ENTITY_DECL,
	CONG_NODE_TYPE_NAMESPACE_DECL,
	CONG_NODE_TYPE_XINCLUDE_START,
	CONG_NODE_TYPE_XINCLUDE_END,

	CONG_NODE_TYPE_NUM
}  CongNodeType;

typedef enum 
{
	CONG_FLOW_TYPE_BLOCK,
	CONG_FLOW_TYPE_INLINE
} CongFlowType;

typedef enum  {
	CONG_WHITESPACE_NORMALIZE,
	CONG_WHITESPACE_PRESERVE
} CongWhitespaceHandling;

typedef struct CongDocument CongDocument;
typedef struct CongView CongView;
typedef struct CongViewClass CongViewClass;
typedef struct CongDispspec CongDispspec;
typedef struct CongDispspecElement CongDispspecElement;
typedef struct CongDispspecElementHeaderInfo CongDispspecElementHeaderInfo;
typedef struct CongDispspecRegistry CongDispspecRegistry;

typedef struct CongFont CongFont;

typedef struct CongCursor CongCursor;
typedef struct CongSelection CongSelection;
typedef struct CongPrimaryWindow CongPrimaryWindow;
typedef struct CongEditorView CongEditorView;
typedef struct CongSpanEditor CongSpanEditor; 

typedef xmlNodePtr CongNodePtr;
typedef xmlChar CongXMLChar;

CongNodePtr cong_node_prev(CongNodePtr node);
CongNodePtr cong_node_next(CongNodePtr node);
CongNodePtr cong_node_first_child(CongNodePtr node);
CongNodePtr cong_node_parent(CongNodePtr node);

CongNodeType cong_node_type(CongNodePtr node);

gboolean 
cong_node_is_element (CongNodePtr node, 
		      const gchar *ns_uri, 
		      const gchar *local_name);

gboolean 
cong_node_is_element_from_set (CongNodePtr node, 
			       const gchar *ns_uri,
			       const gchar **local_name_array,
			       guint num_local_names,
			       guint *output_index);

xmlNsPtr
cong_node_get_ns (CongNodePtr node);

const gchar*
cong_node_get_ns_uri (CongNodePtr node);

const gchar*
cong_node_get_ns_prefix (CongNodePtr node);

const gchar*
cong_node_get_local_name (CongNodePtr node);

gchar*
cong_node_get_qualified_name (CongNodePtr node);

xmlNsPtr
cong_node_get_ns_for_uri (CongNodePtr node, 
			  const gchar *ns_uri);

xmlNsPtr
cong_node_get_ns_for_prefix (CongNodePtr node, 
			     const gchar *prefix);

xmlNsPtr
cong_node_get_attr_ns(CongNodePtr node, 
		      const char *qualified_name, 
		      const char **output_name);

gchar*
cong_node_get_path (CongNodePtr node);

/* Handy debug methods for writing log info: */
gchar *cong_node_debug_description(CongNodePtr node);

const gchar *cong_node_type_description(CongNodeType node_type);

/* Methods for accessing attribute values: */

CongXMLChar* cong_node_get_attribute(CongNodePtr node,
				     xmlNs* ns_ptr, 
				     const CongXMLChar* local_attribute_name);
gboolean cong_node_has_attribute(CongNodePtr node,
				 xmlNs* ns_ptr, 
				 const CongXMLChar* local_attribute_name);

/* Selftest methods: */
void cong_node_self_test(CongNodePtr node);
void cong_node_self_test_recursive(CongNodePtr node);

void
cong_node_recursive_set_doc (CongNodePtr node, 
			     xmlDocPtr xml_doc);

CongNodePtr
cong_node_recursive_dup (CongNodePtr node);

gboolean
cong_node_is_descendant_of (CongNodePtr node,
			    CongNodePtr potential_ancestor);

/* Method to test if it's safe to recurse over the children of this node.  It's generally not a good idea to recurse over the children of an entity reference.  
   FIXME: why? 
*/
gboolean cong_node_should_recurse(CongNodePtr node);

#if 1
#define CONG_NODE_SELF_TEST(node) cong_node_self_test(node)
#else
#define CONG_NODE_SELF_TEST(node) ((void)0)
#endif

int 
cong_node_get_length (CongNodePtr node); /* get length of content; does not include the zero terminator (to correspond to the TTREE size field) */

CongNodePtr
cong_node_new_element (xmlNsPtr ns,
		       const gchar *tagname, 
		       CongDocument *doc);
CongNodePtr 
cong_node_new_element_from_dispspec (CongDispspecElement *element,
				     CongDocument *doc);

CongNodePtr
cong_node_new_text (const gchar *text, 
		    CongDocument *doc);
CongNodePtr 
cong_node_new_text_len (const gchar *text, 
			int len, 
			CongDocument *doc); /* FIXME: what character type ? */
CongNodePtr
cong_node_new_comment (const gchar *comment, 
		       CongDocument *doc);

/* Destruction: (the node has to have been unlinked from the tree already): */
void 
cong_node_free (CongNodePtr node);


gchar*
cong_node_generate_source (CongNodePtr node);

gchar*
cong_node_generate_child_source (CongNodePtr node);

/* Generate XML source from TEXT and COMMENT nodes as a UTF8 string, from the byte offset into the UTF-8: */
gchar*
cong_node_generate_source_from_byte_offset (CongNodePtr node, 
					    int byte_offset);

/* Generate XML source from TEXT and COMMENT nodes as a UTF8 string, up to the byte offset into the UTF-8: 
   FIXME: specify the end-point more precisely
 */
gchar*
cong_node_generate_source_up_to_byte_offset (CongNodePtr node,
					     int byte_offset);

/* Generate XML source from TEXT and COMMENT nodes as a UTF8 string, between the given byte offset into the UTF-8: 
   FIXME: specify the end-point more precisely
 */
gchar*
cong_node_generate_source_between_byte_offsets (CongNodePtr node,
						int start_byte_offset,
						int end_byte_offset);

gboolean
cong_node_is_descendant_of (CongNodePtr node,
			    CongNodePtr potential_ancestor);


/* 
   Direct tree manipulation; these functions are "private" and should only be called by the cong_document_ versions below, which send notifications
   to views of the document.
   (Eventually we will deprecate those as well and convert to an apporach involving atomic and compound modification objects, which will give us Undo/Redo)
*/
void cong_node_private_make_orphan(CongNodePtr node);
void cong_node_private_add_after(CongNodePtr node, CongNodePtr older_sibling);
void cong_node_private_add_before(CongNodePtr node, CongNodePtr younger_sibling);
void cong_node_private_set_parent(CongNodePtr node, CongNodePtr adoptive_parent, gboolean add_to_end);
void cong_node_private_set_text(CongNodePtr node, const xmlChar *new_content);
void cong_node_private_set_attribute(CongNodePtr node,
				     xmlNs *ns_ptr, 
				     const xmlChar *local_attribute_name,
				     const xmlChar *value);
void cong_node_private_remove_attribute(CongNodePtr node,
					xmlNs *ns_ptr, 
					const xmlChar *local_attribute_name);

/* Utilities: */

CongNodePtr 
cong_node_get_child_by_name (CongNodePtr node, 
			     const gchar *ns_uri, 
			     const gchar *local_name);

CongNodePtr 
cong_node_get_first_text_node_descendant (CongNodePtr node);

CongWhitespaceHandling
cong_node_get_whitespace_handling (CongDocument *doc,
				   CongNodePtr text_node);

gboolean
cong_node_should_be_visible_in_editor (CongNodePtr node);

gboolean
cong_node_is_valid_cursor_location (CongNodePtr node);

gboolean
cong_node_supports_byte_offsets (CongNodePtr node);

gboolean
cong_node_can_be_cut (CongNodePtr node);

gboolean
cong_node_can_be_copied (CongNodePtr node);

int 
cong_node_get_ordering (CongNodePtr n0,
			CongNodePtr n1);


/**
 * CongNodePredicate:
 *
 * A predicate function for searching for nodes in various ways
 */
typedef gboolean 
(*CongNodePredicate) (CongNodePtr node,
		      gpointer user_data);
		      

CongNodePtr
cong_node_calc_first_node_in_subtree_satisfying (CongNodePtr node,
						 CongNodePredicate predicate,
						 gpointer user_data);

CongNodePtr
cong_node_calc_final_node_in_subtree_satisfying (CongNodePtr node, 
						 CongNodePredicate predicate,
						 gpointer user_data);

CongNodePtr
cong_node_calc_prev_node_satisfying (CongNodePtr node, 
				     CongNodePredicate predicate,
				     gpointer user_data);

CongNodePtr
cong_node_calc_next_node_satisfying (CongNodePtr node,
				     CongNodePredicate predicate,
				     gpointer user_data);

G_END_DECLS

#endif
