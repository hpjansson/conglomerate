/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-node.c
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
#include "cong-node.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-util.h"

#define LOG_CONG_NODE_PRIVATE_MODIFICATIONS 0
#if LOG_CONG_NODE_PRIVATE_MODIFICATIONS
#define LOG_NODE_PRIVATE_MODIFICATION(x) g_message(x)
#else
#define LOG_NODE_PRIVATE_MODIFICATION(x) ((void)0)
#endif

/* Internal types: */

/* Internal function declarations: */

/* Exported function definitions: */
const gchar* cong_node_name(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return node->name;
}

const gchar* cong_node_xmlns(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	if (node->ns) {
		return node->ns->prefix;
	} else {
		return NULL;
	}
}

CongNodePtr cong_node_prev(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return node->prev;	
}

CongNodePtr cong_node_next(CongNodePtr node)
{
	g_assert(node);
	g_return_val_if_fail(node, NULL);

	return node->next;
}

CongNodePtr cong_node_first_child(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return node->children;
}

CongNodePtr cong_node_parent(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return node->parent;
}

enum CongNodeType cong_node_type(CongNodePtr node)
{
	g_return_val_if_fail(node, CONG_NODE_TYPE_UNKNOWN);

	switch (node->type) {

	default: g_assert_not_reached();

	case XML_ELEMENT_NODE: 
		return CONG_NODE_TYPE_ELEMENT;
	case XML_ATTRIBUTE_NODE:
		return CONG_NODE_TYPE_ATTRIBUTE;
	case XML_TEXT_NODE: 
		return CONG_NODE_TYPE_TEXT;
	case XML_CDATA_SECTION_NODE:
		return CONG_NODE_TYPE_CDATA_SECTION;
	case XML_ENTITY_REF_NODE:
		return CONG_NODE_TYPE_ENTITY_REF;
	case XML_ENTITY_NODE:
		return CONG_NODE_TYPE_ENTITY_NODE;
	case XML_PI_NODE:
		return CONG_NODE_TYPE_PI;
	case XML_COMMENT_NODE: 
		return CONG_NODE_TYPE_COMMENT;
	case XML_DOCUMENT_NODE:
		return CONG_NODE_TYPE_DOCUMENT;
	case XML_DOCUMENT_TYPE_NODE:
		return CONG_NODE_TYPE_DOCUMENT_TYPE;
	case XML_DOCUMENT_FRAG_NODE:
		return CONG_NODE_TYPE_DOCUMENT_FRAG;
	case XML_NOTATION_NODE:
		return CONG_NODE_TYPE_NOTATION;
	case XML_HTML_DOCUMENT_NODE:
		return CONG_NODE_TYPE_HTML_DOCUMENT;
	case XML_DTD_NODE:
		return CONG_NODE_TYPE_DTD;
	case XML_ELEMENT_DECL:
		return CONG_NODE_TYPE_ELEMENT_DECL;
	case XML_ATTRIBUTE_DECL:
		return CONG_NODE_TYPE_ATRRIBUTE_DECL;
	case XML_ENTITY_DECL:
		return CONG_NODE_TYPE_ENTITY_DECL;
	case XML_NAMESPACE_DECL:
		return CONG_NODE_TYPE_NAMESPACE_DECL;
	case XML_XINCLUDE_START:
		return CONG_NODE_TYPE_XINCLUDE_START;
	case XML_XINCLUDE_END:
		return CONG_NODE_TYPE_XINCLUDE_END;
#ifdef LIBXML_DOCB_ENABLED
	case XML_DOCB_DOCUMENT_NODE:
#endif
		return CONG_NODE_TYPE_UNKNOWN;
	}

}

gboolean 
cong_node_is_tag (CongNodePtr node, 
		  const gchar *xmlns,
		  const gchar *tagname)
{
	/* FIXME: what about namespaces? */

	g_return_val_if_fail(node, FALSE);
	g_return_val_if_fail(tagname, FALSE);

	if (node->type==XML_ELEMENT_NODE) {
		const gchar *node_xmlns = cong_node_get_xmlns (node);

		if (xmlns) {
			if (node_xmlns) {
				if (0!=strcmp(xmlns, node_xmlns)) {
					return FALSE;
				}
			} else {
				return FALSE;
			}
		} else {
			if (node_xmlns) {
				return FALSE;
			}
		}
		
		return 0==strcmp(tagname, node->name);
	}

	return FALSE;
}

const gchar*
cong_node_get_xmlns (CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);
	
	if (node->ns) {
		return node->ns->prefix;
	} else {
		return NULL;
	}
}


/* Method for getting an XPath to the node: */
gchar *cong_node_get_path(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return xmlGetNodePath(node);
}

gchar *cong_node_debug_description(CongNodePtr node)
{
	gchar *result = NULL; 
	gchar *xpath;
	gchar *cleaned_text = NULL; 

	g_return_val_if_fail(node, NULL);

	xpath = cong_node_get_path(node);

	switch (node->type) {

	default: g_assert_not_reached();

	case XML_ELEMENT_NODE: 
		result = g_strdup_printf("%s at %p, name=\"%s\", path=\"%s\"",
					 cong_node_type_description(cong_node_type(node)),
					 node,
					 node->name,
					 xpath);
		break;
	case XML_ATTRIBUTE_NODE:
	case XML_TEXT_NODE: 
		cleaned_text = cong_util_cleanup_text(node->content);
		result = g_strdup_printf("%s at %p, name=\"%s\", content=\"%s\", path=\"%s\"",
					 cong_node_type_description(cong_node_type(node)),
					 node,
					 node->name,
					 cleaned_text,
					 xpath);
		break;
	case XML_CDATA_SECTION_NODE:
	case XML_ENTITY_REF_NODE:
	case XML_ENTITY_NODE:
	case XML_PI_NODE:
	case XML_COMMENT_NODE: 
		cleaned_text = cong_util_cleanup_text(node->content);
		result = g_strdup_printf("%s at %p, name=\"%s\", content=\"%s\", path=\"%s\"",
					 cong_node_type_description(cong_node_type(node)),
					 node,
					 node->name,
					 cleaned_text,
					 xpath);
		break;
	case XML_DOCUMENT_NODE:
	case XML_DOCUMENT_TYPE_NODE:
	case XML_DOCUMENT_FRAG_NODE:
	case XML_NOTATION_NODE:
	case XML_HTML_DOCUMENT_NODE:
	case XML_DTD_NODE:
	case XML_ELEMENT_DECL:
	case XML_ATTRIBUTE_DECL:
	case XML_ENTITY_DECL:
	case XML_NAMESPACE_DECL:
	case XML_XINCLUDE_START:
	case XML_XINCLUDE_END:
#ifdef LIBXML_DOCB_ENABLED
	case XML_DOCB_DOCUMENT_NODE:
#endif
		result = g_strdup_printf("%s at %p, name=\"%s\", path=\"%s\"",
					 cong_node_type_description(cong_node_type(node)),
					 node,
					 node->name,
					 xpath);
		break;
		
	}

	if (cleaned_text) {
		g_free(cleaned_text);
	}
	g_free(xpath);

	return result;
}

static const gchar* node_type_names[CONG_NODE_TYPE_NUM]=
{
	"CONG_NODE_TYPE_UNKNOWN",

	"CONG_NODE_TYPE_ELEMENT",
	"CONG_NODE_TYPE_ATTRIBUTE",
	"CONG_NODE_TYPE_TEXT",
	"CONG_NODE_TYPE_CDATA_SECTION",
	"CONG_NODE_TYPE_ENTITY_REF",
	"CONG_NODE_TYPE_ENTITY_NODE",
	"CONG_NODE_TYPE_PI",
	"CONG_NODE_TYPE_COMMENT",
	"CONG_NODE_TYPE_DOCUMENT",
	"CONG_NODE_TYPE_DOCUMENT_TYPE",
	"CONG_NODE_TYPE_DOCUMENT_FRAG",
	"CONG_NODE_TYPE_NOTATION",
	"CONG_NODE_TYPE_HTML_DOCUMENT",
	"CONG_NODE_TYPE_DTD",
	"CONG_NODE_TYPE_ELEMENT_DECL",
	"CONG_NODE_TYPE_ATTRIBUTE_DECL",
	"CONG_NODE_TYPE_ENTITY_DECL",
	"CONG_NODE_TYPE_NAMESPACE_DECL",
	"CONG_NODE_TYPE_XINCLUDE_START",
	"CONG_NODE_TYPE_XINCLUDE_END"
};
	


const gchar *cong_node_type_description(enum CongNodeType node_type)
{
	g_return_val_if_fail(node_type<CONG_NODE_TYPE_NUM,"(invalid type)");

	return node_type_names[node_type];

}

/* Methods for accessing attribute values: */
CongXMLChar* cong_node_get_attribute(CongNodePtr node, const CongXMLChar* attribute_name)
{
	g_return_val_if_fail(node, NULL);
	g_return_val_if_fail(attribute_name, NULL);

	return xmlGetProp(node, attribute_name);
}

void cong_node_self_test(CongNodePtr node)
{
	CongNodePtr iter;

	g_return_if_fail(node);

	g_assert(node->doc);

	/* Test content for valid UTF-8, if appropriate for this node type: */
	switch (node->type) {
	case XML_DOCUMENT_NODE:
	case XML_DTD_NODE:
		/* The "content" field is meaningless; don't test */
		break;

	default:
		if (node->content) {
			g_assert(g_utf8_validate(node->content,-1,NULL));
		}
		break;
	}

	if (node->prev) {
		g_assert(node->prev->next==node);
		g_assert(node->prev->parent == node->parent);
		g_assert(node->parent);
		g_assert(node->parent->children!=node);
	} else {
		if (node->parent) {
			g_assert(node->parent->children==node);
		}
	}

	if (node->next) {
		g_assert(node->next->prev==node);
		g_assert(node->next->parent == node->parent);
		g_assert(node->parent);
		g_assert(node->parent->last!=node);
	} else {
		if (node->parent) {
			g_assert(node->parent->last==node);
		}
	}

	if (node->type==XML_ENTITY_REF_NODE) {
		/* Special case:

		FIXME: Then the child of this node is the entity decl, and hence has a different node as their parent (the DTD): */

		/* FIXME: are boththese cases needed?  try examples/file-roller.xml */
		for (iter=node->children; iter; iter=iter->next) {
			switch (iter->type) {
			case XML_COMMENT_NODE:
			case XML_ENTITY_NODE:
			case XML_ENTITY_DECL:
				g_assert (iter->parent->type == XML_DTD_NODE);
				g_assert (iter->parent!=node);
				break;
			case XML_ENTITY_REF_NODE:				
				break;
			default:
				g_assert_not_reached();
			}
		}
	} else {
		/* Normally, all children of this node should hade this node as their parent: */
		for (iter=node->children; iter; iter=iter->next) {
			g_assert (iter->parent == node);
		}
	}

	if (node->parent) {
		/* Check forwards for node under parent: */
		for (iter=node->parent->children; iter; iter=iter->next) {
			if (iter==node) {
				break;
			}
		}
		g_assert(iter==node);

		/* Check backwards for node under parent: */
		for (iter=node->parent->last; iter; iter=iter->prev) {
			if (iter==node) {
				break;
			}
		}
		g_assert(iter==node);
	}
}

void cong_node_self_test_recursive(CongNodePtr node)
{
	CongNodePtr iter;

	g_return_if_fail(node);

	cong_node_self_test(node);

	for (iter=node->children; iter; iter=iter->next) {
		cong_node_self_test_recursive(iter);
	}
}

int cong_node_get_length(CongNodePtr node)
{
	/* get length of content; does not include the zero terminator */
	g_return_val_if_fail( (cong_node_type(node) == CONG_NODE_TYPE_TEXT) || (cong_node_type(node) == CONG_NODE_TYPE_COMMENT), 0);

	return xmlStrlen(node->content);
	
}

gboolean cong_node_should_recurse(CongNodePtr node)
{
	g_return_val_if_fail(node, FALSE);

	switch (node->type) {
	default: g_assert_not_reached();
	case XML_ELEMENT_NODE:
	case XML_ATTRIBUTE_NODE:
		return TRUE;
	case XML_TEXT_NODE:
	case XML_CDATA_SECTION_NODE:
		return TRUE;

	case XML_ENTITY_REF_NODE:
		return FALSE;
	case XML_ENTITY_NODE:
	case XML_PI_NODE:
	case XML_COMMENT_NODE:
	case XML_DOCUMENT_NODE:
	case XML_DOCUMENT_TYPE_NODE:
	case XML_DOCUMENT_FRAG_NODE:
	case XML_NOTATION_NODE:
	case XML_HTML_DOCUMENT_NODE:
	case XML_DTD_NODE:
	case XML_ELEMENT_DECL:
	case XML_ATTRIBUTE_DECL:
	case XML_ENTITY_DECL:
	case XML_NAMESPACE_DECL:
	case XML_XINCLUDE_START:
	case XML_XINCLUDE_END:
#ifdef LIBXML_DOCB_ENABLED
	case XML_DOCB_DOCUMENT_NODE:
#endif
		return TRUE;
	}
}

/* Construction: */
CongNodePtr cong_node_new_element(const gchar *xmlns, const gchar *tagname, CongDocument *doc)
{
	/* xmlns can be NULL */
	g_return_val_if_fail(tagname, NULL);
	g_return_val_if_fail(doc, NULL);

	if (xmlns) {
		return xmlNewDocNode (cong_document_get_xml(doc), 
				      cong_document_get_nsptr (doc, xmlns), 
				      tagname, 
				      NULL); /* FIXME: audit the character types here */
	} else {
		return xmlNewDocNode (cong_document_get_xml(doc), 
				      NULL,
				      tagname, 
				      NULL); /* FIXME: audit the character types here */
	}
}

CongNodePtr cong_node_new_element_from_dispspec(CongDispspecElement *element, CongDocument *doc)
{
	const gchar *xmlns;

	g_return_val_if_fail (element, NULL);
	g_return_val_if_fail (doc, NULL);

	xmlns = cong_dispspec_element_get_xmlns(element);

	if (xmlns) {
		return xmlNewDocNode (cong_document_get_xml (doc), 
				      cong_document_get_nsptr (doc, xmlns), 
				      cong_dispspec_element_tagname(element), 
				      NULL);
	} else {
		return xmlNewDocNode (cong_document_get_xml (doc), 
				      NULL, 
				      cong_dispspec_element_tagname(element), 
				      NULL);
	}

}

CongNodePtr cong_node_new_text(const char *text, CongDocument *doc)
{
	return cong_node_new_text_len(text, strlen(text),doc);
}

CongNodePtr cong_node_new_text_len(const char *text, int len, CongDocument *doc)
{
	g_return_val_if_fail(text, NULL);
	g_return_val_if_fail(doc, NULL);

	return xmlNewDocTextLen(cong_document_get_xml(doc), text, len); /* FIXME: audit the character types here */
}

/* Destruction: (the node has to have been unlinked from the tree already): */

void cong_node_free(CongNodePtr node)
{
	g_return_if_fail(node);

	xmlFreeNode(node);
}

gchar*
cong_node_generate_source (CongNodePtr node)
{
	xmlBufferPtr xml_buffer;
	gchar *result;

	g_return_val_if_fail (node, NULL);


	g_assert (node->doc);

	xml_buffer = xmlBufferCreate();

	xmlNodeDump (xml_buffer,
		     node->doc,
		     node,
		     0,
		     FALSE);

	result = g_strdup (xmlBufferContent (xml_buffer));

	xmlBufferFree (xml_buffer);

	return result;
}

gchar*
cong_node_generate_source_from_byte_offset (CongNodePtr node,
					    int start_byte_offset)
{
	if (node->content) {
		g_assert (start_byte_offset<=strlen(node->content));

		return cong_node_generate_source_between_byte_offsets (node,
								       start_byte_offset,
								       strlen(node->content));

	} else {
		return g_strdup("");
	}
}

/* Generate XML source from TEXT and COMMENT nodes as a UTF8 string, up to the byte offset into the UTF-8: 
   FIXME: specify the end-point more precisely
 */
gchar*
cong_node_generate_source_up_to_byte_offset (CongNodePtr node,
					     int end_byte_offset)
{
	if (node->content) {
		g_assert (end_byte_offset<=strlen(node->content));

		return cong_node_generate_source_between_byte_offsets (node,
								       0,
								       end_byte_offset);

	} else {
		return g_strdup("");
	}
}

/* Generate XML source from TEXT and COMMENT nodes as a UTF8 string, between the given byte offset into the UTF-8: 
   FIXME: specify the end-point more precisely
 */
gchar*
cong_node_generate_source_between_byte_offsets (CongNodePtr node,
						int start_byte_offset,
						int end_byte_offset)
{
	gchar *result;
	gchar *clipped_content;

	g_return_val_if_fail (node, NULL);
	g_return_val_if_fail (start_byte_offset<=end_byte_offset, NULL);

	g_assert (node->doc);

	if (node->content) {
		g_assert (start_byte_offset<=strlen(node->content));
		g_assert (end_byte_offset<=strlen(node->content));
	} else {
		return g_strdup("");
	}

	clipped_content = g_strndup (node->content + start_byte_offset, end_byte_offset - start_byte_offset);

	g_message("clipped content = \"%s\"", clipped_content);

	switch (node->type) {
	default: g_assert_not_reached();
	case XML_TEXT_NODE:
		result = xmlEncodeSpecialChars(node->doc, clipped_content);
		break;

	case XML_COMMENT_NODE:
		result = g_strdup_printf("<!--%s-->" , clipped_content);
		break;
	}

	g_assert(result);

	g_message("result =\"%s\"", result);

	g_free (clipped_content);

	return result;
}

void
cong_node_recursive_set_doc(CongNodePtr node, xmlDocPtr xml_doc)
{
	CongNodePtr iter;

	node->doc = xml_doc;

	for (iter = node->children; iter; iter=iter->next) {
		cong_node_recursive_set_doc(iter, xml_doc);
	}
}

CongNodePtr cong_node_recursive_dup(CongNodePtr node)
{
	CongNodePtr new_node = xmlCopyNode(node, TRUE);

	/* Unfortunately, this doesn't preserve the doc ptrs, so we must reconstruct these: */
	cong_node_recursive_set_doc(new_node, node->doc);

	/* FIXME: this is an evil hack, and this whole function should be deprecated */

	return new_node;
}

gboolean
cong_node_is_descendant_of (CongNodePtr node,
			    CongNodePtr potential_ancestor)
{
	g_return_val_if_fail (node, FALSE);
	g_return_val_if_fail (potential_ancestor, FALSE);
	
	if (node->parent) {
		if (node->parent == potential_ancestor) {
			return TRUE;
		} else {
			return cong_node_is_descendant_of (node->parent,
							   potential_ancestor);
		}
	} else {
		return FALSE;
	}
}


/* Tree manipulation: */
void cong_node_private_make_orphan(CongNodePtr node)
{
	CongNodePtr former_parent;
	CongNodePtr former_prev;
	CongNodePtr former_next;

	LOG_NODE_PRIVATE_MODIFICATION("cong_node_private_make_orphan");

	g_return_if_fail(node);

	CONG_NODE_SELF_TEST(node);

	former_parent = node->parent;
	former_prev = node->prev;
	former_next = node->next;

	if (former_parent) {
		CONG_NODE_SELF_TEST(former_parent);
	}
	
	if (former_prev) {
		CONG_NODE_SELF_TEST(former_prev);
	}

	if (former_next) {
		CONG_NODE_SELF_TEST(former_next);	
	}

	if (node->parent) {

		CONG_NODE_SELF_TEST(node->parent);

		if (node->prev) {
			g_assert(node->parent->children != node);
			g_assert(former_prev->next == node);

			former_prev->next = former_next;
			node->prev = NULL;
		} else {
			g_assert(node->parent->children == node);
			node->parent->children = former_next;
		}
		
		if (node->next) {
			g_assert(node->parent->last != node);
			g_assert(former_next->prev == node);

			former_next->prev = former_prev;
			node->next = NULL;
		} else {
			g_assert(node->parent->last == node);
			node->parent->last = former_prev;
		}

		node->parent = NULL;
	 
	} else {
		g_assert(node->prev == NULL);
		g_assert(node->next == NULL);
	}


	/* Postconditions: */
	{
		g_assert(node->parent == NULL);
		g_assert(node->prev == NULL);
		g_assert(node->next == NULL);

		CONG_NODE_SELF_TEST(node);

		if (former_parent) {
			CONG_NODE_SELF_TEST(former_parent);
		}

		if (former_prev) {
			g_assert(former_prev->next == former_next);
			CONG_NODE_SELF_TEST(former_prev);
		}
		if (former_next) {
			g_assert(former_next->prev == former_prev);
			CONG_NODE_SELF_TEST(former_next);
		}
	}
}

void cong_node_private_add_after(CongNodePtr node, CongNodePtr older_sibling)
{
	LOG_NODE_PRIVATE_MODIFICATION("cong_node_private_add_after");

	g_return_if_fail(node);
	g_return_if_fail(older_sibling);
	g_return_if_fail(older_sibling->parent);
	g_return_if_fail(node!=older_sibling);

	CONG_NODE_SELF_TEST(node);
	CONG_NODE_SELF_TEST(older_sibling);

	cong_node_private_make_orphan(node);
#if 1
	node->parent = older_sibling->parent;
	node->prev = older_sibling;

	if (older_sibling->next) {
		g_assert(older_sibling->parent->last!=older_sibling);
		g_assert(older_sibling->next->prev==older_sibling);

		node->next = older_sibling->next;
		older_sibling->next->prev = node;

	} else {
		g_assert(older_sibling->parent->last==older_sibling);

		older_sibling->parent->last = node;
	}

	older_sibling->next = node;

	/* Postconditions: */
	{
		g_assert( older_sibling->next == node );
		g_assert( older_sibling == node->prev );
		g_assert( older_sibling->parent == node->parent );

		CONG_NODE_SELF_TEST(node);
		CONG_NODE_SELF_TEST(older_sibling);
		CONG_NODE_SELF_TEST(older_sibling->parent);

		if (node->next) {
			CONG_NODE_SELF_TEST(node->next);
		}
	}
#endif
#if 0
	xmlAddNextSibling(older_sibling, node);
#endif

}

void cong_node_private_add_before(CongNodePtr node, CongNodePtr younger_sibling)
{
	LOG_NODE_PRIVATE_MODIFICATION("cong_node_private_add_before");

	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);
	g_return_if_fail(younger_sibling->parent);
	g_return_if_fail(node!=younger_sibling);

	CONG_NODE_SELF_TEST(node);
	CONG_NODE_SELF_TEST(younger_sibling);

	cong_node_private_make_orphan(node);

#if 1
	node->parent = younger_sibling->parent;
	node->next = younger_sibling;

	if (younger_sibling->prev) {
		g_assert(younger_sibling->parent->children!=younger_sibling);
		g_assert(younger_sibling->prev->next==younger_sibling);

		node->prev = younger_sibling->prev;
		younger_sibling->prev->next = node;

	} else {
		g_assert(younger_sibling->parent->children==younger_sibling);

		younger_sibling->parent->children = node;
	}

	younger_sibling->prev = node;

	/* Postconditions: */
	{
		g_assert( younger_sibling->prev == node );
		g_assert( younger_sibling == node->next );
		g_assert( younger_sibling->parent == node->parent );

		CONG_NODE_SELF_TEST(node);
		CONG_NODE_SELF_TEST(younger_sibling);
		CONG_NODE_SELF_TEST(younger_sibling->parent);

		if (node->prev) {
			CONG_NODE_SELF_TEST(node->prev);
		}
	}
#endif
#if 0
	xmlAddPrevSibling(younger_sibling, node);
#endif

}

void cong_node_private_set_parent(CongNodePtr node, CongNodePtr adoptive_parent)
{
	LOG_NODE_PRIVATE_MODIFICATION("cong_node_private_set_parent");

	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);
	g_return_if_fail(node!=adoptive_parent);

	CONG_NODE_SELF_TEST(node);
	CONG_NODE_SELF_TEST(adoptive_parent);

	cong_node_private_make_orphan(node);

#if 1
	if (adoptive_parent->last) {
		cong_node_private_add_after(node, adoptive_parent->last);
	} else {
		g_assert(adoptive_parent->children == NULL);

		adoptive_parent->children = node;
		adoptive_parent->last = node;
		node->parent = adoptive_parent;
	}

	/* Postconditions: */
	{
		g_assert(node->parent == adoptive_parent);
		g_assert(adoptive_parent->last == node);
		g_assert(node->next == NULL);
		CONG_NODE_SELF_TEST(node);
		CONG_NODE_SELF_TEST(adoptive_parent);
	}
#endif
#if 0
	xmlAddChild(adoptive_parent, node);
#endif

}

void cong_node_private_set_text(CongNodePtr node, const xmlChar *new_content)
{
	LOG_NODE_PRIVATE_MODIFICATION("cong_node_private_set_text");

	g_return_if_fail(node);
	g_return_if_fail(new_content);

	xmlNodeSetContent(node, new_content);
}

void cong_node_private_set_attribute(CongNodePtr node, const xmlChar *name, const xmlChar *value)
{
	LOG_NODE_PRIVATE_MODIFICATION("cong_node_private_set_attribute");

	g_return_if_fail(node);
	g_return_if_fail(name);
	g_return_if_fail(value);

	xmlSetProp(node, name, value);
}

void cong_node_private_remove_attribute(CongNodePtr node, const xmlChar *name)
{
	LOG_NODE_PRIVATE_MODIFICATION("cong_node_private_remove_attribute");

	g_return_if_fail(node);
	g_return_if_fail(name);

	xmlUnsetProp(node, name);
}

/* Utilities: */
CongNodePtr cong_node_get_child_by_name (CongNodePtr node, 
					 const gchar *xmlns, 
					 const gchar *tagname)
{
	CongNodePtr iter;

	g_return_val_if_fail(node, NULL);
	g_return_val_if_fail(tagname, NULL);

	for (iter=node->children; iter; iter=iter->next) {
		if (cong_node_is_tag (iter, xmlns, tagname)) {
			return iter;
		}
	}

	return NULL;
}

CongNodePtr 
cong_node_get_first_text_node_descendant (CongNodePtr node)
{
	g_return_val_if_fail (node, NULL);

	if (cong_node_type(node)==CONG_NODE_TYPE_TEXT) {
		return node;
	} else {
		CongNodePtr iter;

		for (iter = node->children; iter; iter=iter->next) {
			CongNodePtr iter_result = cong_node_get_first_text_node_descendant (iter);

			if (iter_result) {
				return iter_result;
			}
		}

		return NULL;
	}
}

/* Internal function definitions: */
