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

	CONG_ELEMENT_TYPE_PARAGRAPH,

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

const gchar* cong_node_name(CongNodePtr node);
const gchar* cong_node_xmlns(CongNodePtr node);
CongNodePtr cong_node_prev(CongNodePtr node);
CongNodePtr cong_node_next(CongNodePtr node);
CongNodePtr cong_node_first_child(CongNodePtr node);
CongNodePtr cong_node_parent(CongNodePtr node);

enum CongNodeType cong_node_type(CongNodePtr node);

/** 
    Handy method for deciding if you've found a tag with the given name, as opposed to text nodes, comments, tags with other names etc.
    Ignores namespaces at the moment...
*/
gboolean cong_node_is_tag(CongNodePtr node, const CongXMLChar *tagname);

/* Method for getting an XPath to the node: */
gchar *cong_node_get_path(CongNodePtr node);

/* Handy debug methods for writing log info: */
gchar *cong_node_debug_description(CongNodePtr node);

const gchar *cong_node_type_description(enum CongNodeType node_type);

/* Methods for accessing attribute values: */
CongXMLChar* cong_node_get_attribute(CongNodePtr node, const CongXMLChar* attribute_name);
/* caller responsible for freeing; will be NULL if not found in node and no default in DTD available */

/* Selftest methods: */
void cong_node_self_test(CongNodePtr node);
void cong_node_self_test_recursive(CongNodePtr node);

void cong_node_recursive_delete(CongDocument *doc, CongNodePtr node);
CongNodePtr cong_node_recursive_dup(CongNodePtr node);

/* Method to test if it's safe to recurse over the children of this node.  It's generally not a good idea to recurse over the children of an entity reference.  
   FIXME: why? 
*/
gboolean cong_node_should_recurse(CongNodePtr node);

#if 1
#define CONG_NODE_SELF_TEST(node) cong_node_self_test(node)
#else
#define CONG_NODE_SELF_TEST(node) ((void)0)
#endif

int cong_node_get_length(CongNodePtr node); /* get length of content; does not include the zero terminator (to correspond to the TTREE size field) */

/* Construction: */
CongNodePtr cong_node_new_element(const gchar* xmlns, const gchar *tagname, CongDocument *doc);
CongNodePtr cong_node_new_element_from_dispspec(CongDispspecElement *element, CongDocument *doc);
CongNodePtr cong_node_new_text(const gchar *text, CongDocument *doc);
CongNodePtr cong_node_new_text_len(const gchar *text, int len, CongDocument *doc); /* FIXME: what character type ? */

/* Destruction: (the node has to have been unlinked from the tree already): */
void cong_node_free(CongNodePtr node);

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
void cong_node_private_set_attribute(CongNodePtr node, const xmlChar *name, const xmlChar *value);
void cong_node_private_remove_attribute(CongNodePtr node, const xmlChar *name);

#endif
