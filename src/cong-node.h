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

enum CongNodeType
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
};


enum CongElementType
{
	CONG_ELEMENT_TYPE_STRUCTURAL,
	CONG_ELEMENT_TYPE_SPAN,
	CONG_ELEMENT_TYPE_INSERT,

	CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE,

	/* Other types?  Table? Plugin widget/Bonobo control? */

	CONG_ELEMENT_TYPE_PLUGIN,

	CONG_ELEMENT_TYPE_UNKNOWN,

	CONG_ELEMENT_TYPE_ALL
};

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

enum CongNodeType cong_node_type(CongNodePtr node);

/** 
 * cong_node_is_element:
 *
 * Handy method for deciding if you've found a element with the given name, as opposed to text nodes, comments, elements with other names etc.
 *
 * Returns: TRUE if the node is an element with the correct name, FALSE otherwise
 */
gboolean 
cong_node_is_element (CongNodePtr node, 
		      const gchar *ns_uri, 
		      const gchar *local_name);

/** 
 * cong_node_is_element_from_set:
 *
 * @ns_uri: URI of the namespace shared by all the element in the search set
 * @local_name_array: array of element local names within the namespace
 * @num_local_names: size of search array
 * @output_index: pointer to write index of result to, or NULL if you don't care
 *
 * Handy method for deciding if you've found a element with one of the given names in the set, as opposed to text nodes, comments, elements with other names etc.
 *
 * Returns: TRUE if the node is an element with the correct name, FALSE otherwise
 */
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

/**
 * cong_node_get_qualified_name:
 *
 * @node: an XML element
 *
 * Builds a string of the form "ns_prefix:local_name" for an element inside a namespace
 * or simply "local_name" for the rest.
 *
 * Returns: a freshly-allocated string which the caller must g_free
 *
 */
gchar*
cong_node_get_qualified_name (CongNodePtr node);

/*
 * cong_node_get_ns_for_uri:
 *
 * @node:  the context in which to look for the prefix
 * @ns_uri: the namespace URI
 *
 * Lookup a namespace URI; find the appropriate xmlNsPtr defined, 
 * or NULL if not found.
 *
 * Returns:  the #xmlNsPtr if found, or NULL if not found.
 *
 */
xmlNsPtr
cong_node_get_ns_for_uri (CongNodePtr node, 
			  const gchar *ns_uri);

/*
 * cong_node_get_ns_for_prefix:
 *
 * @node:  the context in which to look for the prefix
 * @prefix: the prefix
 *
 * Lookup a namespace prefix; find the appropriate xmlNsPtr defined
 * for that prefix, or NULL if not found.
 *
 * Returns:  the #xmlNsPtr if found, or NULL if not found.
 *
 */
xmlNsPtr
cong_node_get_ns_for_prefix (CongNodePtr node, 
			     const gchar *prefix);

/**
 * conf_node_get_attr_ns:
 *
 * @node: an XML element
 * @qualified_attr_name: An qualified attribute name with an optional namespace prefix.
 * @output_attr_name: Stores the location of the local_name in @qualified_name.
 *
 * Splits the qualified name into the prefix and the local name,
 * and searches the namespace of the prefix. All namespaces of the
 * @node are searched. If no prefix is availible NULL is returned.
 * see: Namespaces in XML / 5.3 Uniqueness of Attributes.
 *
 * Returns: A pointer to the namespace, can be NULL.
 */
xmlNsPtr
cong_node_get_attr_ns(CongNodePtr node, 
		      const char *qualified_name, 
		      const char **output_name);

/**
 * cong_node_get_path:
 *
 * @node: an XML node
 *
 * Method for getting an XPath to the node.
 *
 * Returns: the XPath as a freshly allocated string, which must be freed using g_free
 * 
 */
gchar*
cong_node_get_path (CongNodePtr node);

/* Handy debug methods for writing log info: */
gchar *cong_node_debug_description(CongNodePtr node);

const gchar *cong_node_type_description(enum CongNodeType node_type);

/* Methods for accessing attribute values: */

/**
 * cong_node_get_attribute
 *
 * @node: XML node which has the attribute.
 * @ns_ptr: Attribute's namespace, can be NULL
 *          (if it is the default namespace it MUST NOT be NULL but the coresponding
 *          xmlNs).
 * @local_attribute_name: Name of the attribute, without namespace prefix.
 *
 * Returns the content of the attribute specified through @local_attribute_name and
 * @ns_ptr.
 *
 * Returns: The content of the attribute, to be freed by the caller.
 *          Will be NULL if not found in node and no default in DTD available
 */
CongXMLChar* cong_node_get_attribute(CongNodePtr node,
				     xmlNs* ns_ptr, 
				     const CongXMLChar* local_attribute_name);
/**
 * cong_node_has_attribute
 *
 * @node: XML node which has the attribute.
 * @ns_ptr: Attribute's namespace, can be NULL
 *             (if it is the default namespace it MUST NOT be NULL but the coresponding
 *              xmlNs).
 * @local_attribute_name: Name of the attribute, without namespace prefix.
 *
 * Returns: Returns TRUE if the attribute specified through @local_attribute_name and
 *          @ns_ptr is found in the node or as default in the DTD.
 */
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

/* Construction: */
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

/* Destruction: (the node has to have been unlinked from the tree already): */
void 
cong_node_free (CongNodePtr node);


/**
 * cong_node_generate_source:
 * @node:  The node for which the XML source is to be generated
 * 
 * Generate XML source for the node 
 *
 * Returns: the XML source for the node as a UTF8 string.  The caller is responsible for freeing this with g_free
 */
gchar*
cong_node_generate_source (CongNodePtr node);

/**
 * cong_node_generate_child_source:
 * @node:  The parent node
 * 
 * Generate XML source for the node's children, concatenated together as a UTF8 string.  Should handle entity references correctly.
 * The result does not include the XML source for the node itself.
 *
 * Returns: the XML source for the node's children as a UTF8 string.  The caller is responsible for freeing this with g_free
 */
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
void cong_node_private_set_parent(CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
void cong_node_private_set_text(CongNodePtr node, const xmlChar *new_content);
void cong_node_private_set_attribute(CongNodePtr node,
				     xmlNs *ns_ptr, 
				     const xmlChar *local_attribute_name,
				     const xmlChar *value);
void cong_node_private_remove_attribute(CongNodePtr node,
					xmlNs *ns_ptr, 
					const xmlChar *local_attribute_name);

/* Utilities: */

/**
 * cong_node_get_child_by_name:
 * @node:  the parent node
 * @ns_uri: URI of namespace to search for, or NULL
 * @tagname: the local name within any namespace of the element to search for
 *
 * This function searches the children of @node looking for elements of the given name.
 *
 * Returns: the first child element matching the given name, or NULL if there are none
 */
CongNodePtr 
cong_node_get_child_by_name (CongNodePtr node, 
			     const gchar *ns_uri, 
			     const gchar *local_name);

CongNodePtr 
cong_node_get_first_text_node_descendant (CongNodePtr node);

enum CongWhitespaceHandling
cong_node_get_whitespace_handling (CongDocument *doc,
				   CongNodePtr text_node);

/**
 * cong_node_should_be_visible_in_editor:
 * @node:  a node to be tested
 *
 * The function detemines if the node ought to be visible in the main editor view.
 *
 * As described in bug #123367, TEXT nodes that are either empty or purely whitespace
 * should only appear in the main editor view if the DTD allows PCDATA at the location in the
 * document.  Otherwise the text is probably merely formatting to prettify the source view. *
 *
 * Returns: a #gboolean which is TRUE iff the node ought to be visible in the main editor view
 */
gboolean
cong_node_should_be_visible_in_editor (CongNodePtr node);

/**
 * cong_node_is_valid_cursor_location:
 * @node:  a node to be tested
 *
 * The function detemines if the node is a suitable location for the cursor.
 *
 * It currently only tests for TEXT nodes, but will eventually be expanded to
 * allow COMMENT nodes as well.
 *
 * Returns: a #gboolean which is TRUE iff it is safe to put the cursor inside this node
 */
gboolean
cong_node_is_valid_cursor_location (CongNodePtr node);

/**
 * cong_node_supports_byte_offsets:
 * @node:  a node to be tested
 *
 * The function determines if #CongLocation objects that reference this node can have meaningful byte offsets
 *
 * Only TEXT and COMMENT nodes can currently have meaningful byte offsets.
 * 
 * Returns: a #gboolean which is TRUE iff #CongLocations that reference this node can have a meaningful byte offset 
 */
gboolean
cong_node_supports_byte_offsets (CongNodePtr node);

gboolean
cong_node_can_be_cut (CongNodePtr node);

gboolean
cong_node_can_be_copied (CongNodePtr node);

/**
 * cong_node_get_ordering:
 * @n0:  first node to be compared
 * @n1:  second node to be compared
 *
 * This functions compares the location of two nodes in the xml tree and returns a numeric comparsion representing
 * their locations in a depth-first traversal.
 *
 * Returns: negative if n0 is reached before n1, zero if they are the same node, positive if n0 is reached after n1
 * 
 */
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
		      

/**
 * cong_node_calc_first_node_in_subtree_satisfying:
 *
 * @node: the top of the subtree
 * @predicate: the #CongNodePredicate to test nodes for
 * @user_data: user-supplied data passed to the predicate
 *
 * Finds the first node in a depth-first traversal of the subtree below this node
 * that satisfies the predicate.
 *
 * Note that @node is the initial node of the tree (and hence is tested first)
 *
 * Returns: the appropriate node satisfying @predicate, or NULL if there are none
 *
 */
CongNodePtr
cong_node_calc_first_node_in_subtree_satisfying (CongNodePtr node,
						 CongNodePredicate predicate,
						 gpointer user_data);

/**
 * cong_node_calc_final_node_in_subtree_satisfying:
 *
 * @node: the top of the subtree
 * @predicate: the #CongNodePredicate to test nodes for
 * @user_data: user-supplied data passed to the predicate
 *
 * Finds the final node in a depth-first traversal of the subtree below this node
 * that satisfies the predicate.
 *
 * Note that @node is the initial node of the tree (and hence is tested last)
 *
 * Returns: the appropriate node satisfying @predicate, or NULL if there are none
 *
 */
CongNodePtr
cong_node_calc_final_node_in_subtree_satisfying (CongNodePtr node, 
						 CongNodePredicate predicate,
						 gpointer user_data);

/**
 * cong_node_calc_prev_node_satisfying:
 *
 * @node: the start of the search
 * @predicate: the #CongNodePredicate to test nodes for
 * @user_data: user-supplied data passed to the predicate
 *
 * Finds the first preceding node relative to the input that satisfies the predicate,
 * in an imagined depth-first traversal of the document.  Includes ancestors.
 *
 * Returns: the appropriate node satisfying @predicate, or NULL if there are none
 *
 */
CongNodePtr
cong_node_calc_prev_node_satisfying (CongNodePtr node, 
				     CongNodePredicate predicate,
				     gpointer user_data);

/**
 * cong_node_calc_next_node_satisfying:
 *
 * @node: the start of the search
 * @predicate: the #CongNodePredicate to test nodes for
 * @user_data: user-supplied data passed to the predicate
 *
 * Finds the first following node relative to the input that satisfies the predicate,
 * in an imagined depth-first traversal of the document.   Includes ancestors.
 *
 * Returns: the appropriate node satisfying @predicate, or NULL if there are none
 *
 */
CongNodePtr
cong_node_calc_next_node_satisfying (CongNodePtr node,
				     CongNodePredicate predicate,
				     gpointer user_data);

G_END_DECLS

#endif
