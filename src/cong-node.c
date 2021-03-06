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
#include "cong-dispspec-element.h"
#include "cong-util.h"
#include "cong-dtd.h"

#include <string.h>

#define LOG_CONG_NODE_PRIVATE_MODIFICATIONS 0
#if LOG_CONG_NODE_PRIVATE_MODIFICATIONS
#define LOG_NODE_PRIVATE_MODIFICATION(x) g_message(x)
#else
#define LOG_NODE_PRIVATE_MODIFICATION(x) ((void)0)
#endif

/* Internal types: */

/* Internal function declarations: */

/* Exported function definitions: */
/**
 * cong_node_prev:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr 
cong_node_prev(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return node->prev;	
}

/**
 * cong_node_next:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr 
cong_node_next(CongNodePtr node)
{
	g_assert(node);
	g_return_val_if_fail(node, NULL);

	return node->next;
}

/**
 * cong_node_first_child:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr 
cong_node_first_child(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return node->children;
}

/**
 * cong_node_parent:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr 
cong_node_parent(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return node->parent;
}

/**
 * cong_node_type:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongNodeType 
cong_node_type(CongNodePtr node)
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

/** 
 * cong_node_is_element:
 * @node:
 * @ns_uri:
 * @local_name:
 *
 * Handy method for deciding if you've found a element with the given name, as opposed to text nodes, comments, elements with other names etc.
 *
 * Returns: TRUE if the node is an element with the correct name, FALSE otherwise
 */
gboolean 
cong_node_is_element (CongNodePtr node, 
		      const gchar *ns_uri,
		      const gchar *local_name)
{
	g_return_val_if_fail (node, FALSE);
	g_return_val_if_fail (local_name, FALSE);

	if (node->type==XML_ELEMENT_NODE) {
		const gchar *node_ns_uri = cong_node_get_ns_uri (node);

		if (!cong_util_ns_uri_equality (ns_uri, node_ns_uri)) {
			return FALSE;
		}
		
		return 0==strcmp (local_name, (const char*)node->name);
	}

	return FALSE;
}

/** 
 * cong_node_is_element_from_set:
 * @node:
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
			       guint *output_index)
{
	g_return_val_if_fail (node, FALSE);
	g_return_val_if_fail (local_name_array, FALSE);

	if (node->type==XML_ELEMENT_NODE) {
		const gchar *node_ns_uri = cong_node_get_ns_uri (node);
		guint i;

		if (!cong_util_ns_uri_equality (ns_uri, node_ns_uri)) {
			return FALSE;
		}

		for (i=0;i<num_local_names;i++) {
			g_assert (local_name_array[i]);

			if (0==strcmp (local_name_array[i], (const char*)node->name)) {

				if (output_index) {
					*output_index = i;					
				}

				return TRUE;
			}
		}
	}

	return FALSE;
	
}

/**
 * cong_node_get_ns:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
xmlNsPtr
cong_node_get_ns (CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);
	
	return node->ns;
}

/**
 * cong_node_get_ns_uri:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_node_get_ns_uri (CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);
	
	if (node->ns) {
		return (const gchar*)node->ns->href;
	} else {
		return NULL;
	}
}

/**
 * cong_node_get_ns_prefix:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_node_get_ns_prefix (CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);
	
	if (node->ns) {
		return (const gchar*)node->ns->prefix;
	} else {
		return NULL;
	}
}

/**
 * cong_node_get_local_name:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
const gchar* 
cong_node_get_local_name (CongNodePtr node)
{
	g_return_val_if_fail (node, NULL);
	g_return_val_if_fail (node->type==XML_ELEMENT_NODE, NULL);

	return (const gchar*)node->name;
}

/**
 * cong_node_get_qualified_name:
 * @node: an XML element
 *
 * Builds a string of the form "ns_prefix:local_name" for an element inside a namespace
 * or simply "local_name" for the rest.
 *
 * Returns: a freshly-allocated string which the caller must g_free
 */
gchar*
cong_node_get_qualified_name (CongNodePtr node)
{
	g_return_val_if_fail (node, NULL);
	g_return_val_if_fail (node->type==XML_ELEMENT_NODE, NULL);

	if (cong_node_get_ns_prefix (node)) {				
		return g_strdup_printf ("%s:%s", 
					cong_node_get_ns_prefix (node), 
					cong_node_get_local_name (node));
	} else {
		return g_strdup (cong_node_get_local_name (node));
	}	
}

/*
 * cong_node_get_ns_for_uri:
 * @node:  the context in which to look for the prefix
 * @ns_uri: the namespace URI
 *
 * Lookup a namespace URI; find the appropriate xmlNsPtr defined, 
 * or NULL if not found.
 *
 * Returns:  the #xmlNsPtr if found, or NULL if not found.
 */
xmlNsPtr
cong_node_get_ns_for_uri (CongNodePtr node, 
			  const gchar *ns_uri)
{
	return xmlSearchNsByHref (node->doc,
				  node,
				  (const xmlChar*)ns_uri);
}

/*
 * cong_node_get_ns_for_prefix:
 * @node:  the context in which to look for the prefix
 * @prefix: the prefix
 *
 * Lookup a namespace prefix; find the appropriate xmlNsPtr defined
 * for that prefix, or NULL if not found.
 *
 * Returns:  the #xmlNsPtr if found, or NULL if not found.
 */
xmlNsPtr
cong_node_get_ns_for_prefix (CongNodePtr node, 
			     const gchar *prefix)
{
	g_return_val_if_fail (node, NULL);
	g_return_val_if_fail (prefix, NULL); /* FIXME: really? */

	return xmlSearchNs (node->doc,
			    node, /* FIXME: is this correct? */
			    (const xmlChar*)prefix);
}

/**
 * conf_node_get_attr_ns:
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
		      const char **output_name)
{
	g_return_val_if_fail(node != NULL, NULL);
	g_return_val_if_fail(qualified_name != NULL, NULL);
	g_return_val_if_fail(output_name != NULL, NULL);

	/* get namespace prefix */
		
	*output_name = strchr(qualified_name, ':');

	if(*output_name == NULL) {
		*output_name = qualified_name;
		return NULL;
	} else {
		gchar *prefix;
		xmlNsPtr ns;

		prefix = g_strndup(qualified_name, 
				   (*output_name) - qualified_name);

		/* go after the colon. */
		(*output_name) ++;

		ns = cong_node_get_ns_for_prefix(node,
						 prefix);

		g_free(prefix);

		return ns;
	}	
}

/* Method for getting an XPath to the node: */
/**
 * cong_node_get_path:
 * @node: an XML node
 *
 * Method for getting an XPath to the node.
 *
 * Returns: the XPath as a freshly allocated string, which must be freed using g_free
 */
gchar *
cong_node_get_path(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return cong_util_dup_and_free_xml_string (xmlGetNodePath(node));
}

/**
 * cong_node_debug_description:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
gchar *
cong_node_debug_description(CongNodePtr node)
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
		cleaned_text = cong_util_cleanup_text((const gchar*)node->content);
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
		cleaned_text = cong_util_cleanup_text((const gchar*)node->content);
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
	
/**
 * cong_node_type_description:
 * @node_type:
 *
 * TODO: Write me
 * Returns:
 */
const gchar *
cong_node_type_description(CongNodeType node_type)
{
	g_return_val_if_fail(node_type<CONG_NODE_TYPE_NUM,"(invalid type)");

	return node_type_names[node_type];

}

/**
 * cong_node_type_is_textual:
 * @node_type:
 *
 * Returns: TRUE iff the node type is text, a CDATA section, or a comment.
 */
gboolean
cong_node_type_is_textual (CongNodeType node_type)
{
	switch (node_type) {
	default: 
		return FALSE;

	case CONG_NODE_TYPE_TEXT:
	case CONG_NODE_TYPE_CDATA_SECTION:
	case CONG_NODE_TYPE_COMMENT:
		return TRUE;
	}
}

/**
 * cong_node_type_is_textual_content:
 * @node_type:
 *
 * Returns: TRUE iff the node type is text, a CDATA section (not comments).
 */
gboolean
cong_node_type_is_textual_content (CongNodeType node_type)
{
	switch (node_type) {
	default: 
		return FALSE;

	case CONG_NODE_TYPE_TEXT:
	case CONG_NODE_TYPE_CDATA_SECTION:
		return TRUE;
	}
}

/* Methods for accessing attribute values: */
/**
 * cong_node_get_attribute:
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
gchar* 
cong_node_get_attribute(CongNodePtr node,
			xmlNs* ns_ptr, 
			const gchar* local_attribute_name)
{
	g_return_val_if_fail(node, NULL);
	g_return_val_if_fail(local_attribute_name, NULL);
	
	if(ns_ptr == NULL) {
		return cong_util_dup_and_free_xml_string (xmlGetNoNsProp(node, (const xmlChar*)local_attribute_name));		
	} else {
		return cong_util_dup_and_free_xml_string (xmlGetNsProp(node, (const xmlChar*)local_attribute_name, ns_ptr->href));
	}
}

/**
 * cong_node_has_attribute:
 * @node: XML node which has the attribute.
 * @ns_ptr: Attribute's namespace, can be NULL
 *             (if it is the default namespace it MUST NOT be NULL but the coresponding
 *              xmlNs).
 * @local_attribute_name: Name of the attribute, without namespace prefix.
 *
 * Returns: Returns TRUE if the attribute specified through @local_attribute_name and
 *          @ns_ptr is found in the node or as default in the DTD.
 */
gboolean 
cong_node_has_attribute(CongNodePtr node,
			xmlNs* ns_ptr, 
			const gchar* local_attribute_name)
{
	g_return_val_if_fail(node, FALSE);
	g_return_val_if_fail(local_attribute_name, FALSE);
	
	if(ns_ptr == NULL) {
		xmlAttr *attr = xmlHasProp(node, (const xmlChar*)local_attribute_name);

		/* now check, if there is any namespace,
		 * if there is a prefix. */
		if (attr == NULL) {
			return FALSE;
		} else if (attr->ns != NULL &&
			   attr->ns->prefix != NULL) {
			if (strcmp((const char*)attr->ns->prefix, "") == 0) {
				return TRUE;
			} else {
				return FALSE;
			}
		} else {
			return TRUE;
		}
	} else {		
		return xmlHasNsProp(node, (const xmlChar*)local_attribute_name, ns_ptr->href) != NULL;
	}
}

/**
 * cong_node_self_test:
 * @node:
 *
 * TODO: Write me
 */
void 
cong_node_self_test(CongNodePtr node)
{
	CongNodePtr iter;

	g_return_if_fail(node);

	g_assert(node->doc);

	/* Test content for valid UTF-8, if appropriate for this node type: */
	switch (node->type) {
	case XML_DOCUMENT_NODE:
	case XML_DTD_NODE:
	case XML_ATTRIBUTE_DECL:
		/* The "content" field is meaningless; don't test */
		break;

	default:
		if (node->content) {
			g_assert(g_utf8_validate((const gchar*)node->content,-1,NULL));
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
		g_assert (node->children == node->last);

#if 1
		g_assert (node->children->type == XML_ENTITY_DECL);
#else
		/* FIXME: are boththese cases needed?  try examples/file-roller.xml */
		for (iter=node->children; iter; iter=iter->next) {
			switch (iter->type) {
			case XML_COMMENT_NODE:
			case XML_ENTITY_NODE:
			case XML_ENTITY_DECL:
			case XML_ATTRIBUTE_DECL:
			case XML_ELEMENT_DECL:
				g_assert (iter->parent->type == XML_DTD_NODE);
				g_assert (iter->parent!=node);
				break;
			case XML_ENTITY_REF_NODE:				
				break;
			default:
				g_assert_not_reached();
			}
		}
#endif
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

/**
 * cong_node_self_test_recursive:
 * @node:
 *
 * TODO: Write me
 */
void 
cong_node_self_test_recursive(CongNodePtr node)
{

	g_return_if_fail(node);

	cong_node_self_test(node);

	if (node->type==XML_ENTITY_REF_NODE) {
		cong_node_self_test_recursive(node->children);
	} else {
		CongNodePtr iter;

		for (iter=node->children; iter; iter=iter->next) {
			cong_node_self_test_recursive(iter);
		}
	}
}

/**
 * cong_node_get_length:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
int 
cong_node_get_length(CongNodePtr node)
{
	/* get length of content; does not include the zero terminator */
	g_return_val_if_fail( cong_node_type_is_textual (cong_node_type(node)), 0);

	return xmlStrlen(node->content);
	
}

/**
 * cong_node_should_recurse:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
cong_node_should_recurse(CongNodePtr node)
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
/**
 * cong_node_new_element:
 * @ns:
 * @tagname:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr 
cong_node_new_element (xmlNsPtr xml_ns, 
		       const gchar *local_name, 
		       CongDocument *doc)
{
	/* xml_ns can be NULL */
	g_return_val_if_fail (local_name, NULL);
	g_return_val_if_fail (doc, NULL);

	return xmlNewDocNode (cong_document_get_xml (doc),
			      xml_ns,
			      (const xmlChar*)local_name, 
			      NULL); /* FIXME: audit the character types here */
}

/**
 * cong_node_new_element_from_dispspec:
 * @element:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr
cong_node_new_element_from_dispspec (CongDispspecElement *element, 
				     CongDocument *doc)
{
	xmlNsPtr xml_ns;

	g_return_val_if_fail (element, NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);

	xml_ns = cong_document_get_xml_ns (doc,
					   cong_dispspec_element_get_ns_uri (element));

	return xmlNewDocNode (cong_document_get_xml (doc), 
			      xml_ns,
			      (const xmlChar*)cong_dispspec_element_get_local_name (element), 
			      NULL);
}

CongNodePtr 
cong_node_new_element_full (xmlDocPtr xml_doc, 
			    const gchar *ns_uri,
			    const gchar *local_name)
{
	xmlNsPtr xml_ns;

	g_return_val_if_fail (xml_doc, NULL);
	g_return_val_if_fail (local_name, NULL);

	xml_ns = xmlSearchNsByHref (xml_doc,
				    (xmlNodePtr)xml_doc, /* FIXME: is this correct? */
				    (const xmlChar*)ns_uri);
	return xmlNewDocNode (xml_doc,
			      xml_ns,
			      (const xmlChar*)local_name, 
			      NULL);
}

CongNodePtr 
cong_node_new_element_full_with_content (xmlDocPtr xml_doc, 
					 const gchar *ns_uri,
					 const gchar *local_name,
					 const gchar *content)
{
	xmlNsPtr xml_ns;

	g_return_val_if_fail (xml_doc, NULL);
	g_return_val_if_fail (local_name, NULL);
	g_return_val_if_fail (content, NULL);

	xml_ns = xmlSearchNsByHref (xml_doc,
				    (xmlNodePtr)xml_doc, /* FIXME: is this correct? */
				    (const xmlChar*)ns_uri);
	return xmlNewDocNode (xml_doc,
			      xml_ns,
			      (const xmlChar*)local_name, 
			      (const xmlChar*)content);
}

/**
 * cong_node_new_text:
 * @text:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr 
cong_node_new_text (const char *text, 
		    CongDocument *doc)
{
	return cong_node_new_text_len(text, strlen(text),doc);
}

/**
 * cong_node_new_text_len:
 * @text:
 * @len:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr 
cong_node_new_text_len (const char *text, 
			int len, 
			CongDocument *doc)
{
	g_return_val_if_fail(text, NULL);
	g_return_val_if_fail(doc, NULL);

	return xmlNewDocTextLen (cong_document_get_xml(doc), 
				 (const xmlChar*)text, 
				 len);
}

CongNodePtr
cong_node_new_cdata_section (const gchar *text, 
			     CongDocument *doc)
{
	return cong_node_new_cdata_section_len (text, strlen(text), doc);
}

CongNodePtr
cong_node_new_cdata_section_len (const gchar *text, 
				 int len, 
				 CongDocument *doc)
{
	return xmlNewCDataBlock	(cong_document_get_xml(doc),
				 (const xmlChar*)text,
				 len);
}

CongNodePtr
cong_node_new_comment (const gchar *comment, 
		       CongDocument *doc)
{
	g_return_val_if_fail (comment, NULL);
	g_return_val_if_fail (doc, NULL);

	return xmlNewDocComment (cong_document_get_xml(doc), 
				 (const xmlChar*)comment);
}

CongNodePtr
cong_node_new_comment_len (const gchar *comment, 
			   int len,	
			   CongDocument *doc)
{
	gchar *tmp_comment;
	CongNodePtr result;

	g_return_val_if_fail (comment, NULL);
	g_return_val_if_fail (doc, NULL);

	/* There doesn't seem to be a xmlNewDocCommentLen function so we fake it: */
	tmp_comment = g_strndup (comment, len);

	result = xmlNewDocComment (cong_document_get_xml (doc), 
				   (const xmlChar*)tmp_comment);

	g_free (tmp_comment);

	return result;
}

CongNodePtr
cong_node_new_textual (CongNodeType textual_node_type,
		       const gchar *content,
		       CongDocument *doc)
{
	g_return_val_if_fail (cong_node_type_is_textual (textual_node_type), NULL);

	switch (textual_node_type) {
	default: g_assert_not_reached ();
	case CONG_NODE_TYPE_TEXT:
		return cong_node_new_text (content, doc);
	case CONG_NODE_TYPE_CDATA_SECTION:
		return cong_node_new_cdata_section (content, doc);
	case CONG_NODE_TYPE_COMMENT:
		return cong_node_new_comment (content, doc);
	}
}

CongNodePtr
cong_node_new_textual_len (CongNodeType textual_node_type,
			   const gchar *content,
			   int len,
			   CongDocument *doc)
{
	g_return_val_if_fail (cong_node_type_is_textual (textual_node_type), NULL);

	switch (textual_node_type) {
	default: g_assert_not_reached ();
	case CONG_NODE_TYPE_TEXT:
		return cong_node_new_text_len (content, len, doc);
	case CONG_NODE_TYPE_CDATA_SECTION:
		return cong_node_new_cdata_section_len (content, len, doc);
	case CONG_NODE_TYPE_COMMENT:
		return cong_node_new_comment_len (content, len, doc);
	}
}


/* Destruction: (the node has to have been unlinked from the tree already): */

/**
 * cong_node_free:
 * @node:
 *
 * TODO: Write me
 */
void 
cong_node_free(CongNodePtr node)
{
	g_return_if_fail(node);

	xmlFreeNode(node);
}

/**
 * cong_node_generate_source:
 * @node:  The node for which the XML source is to be generated
 * 
 * Generate XML source for the node 
 *
 * Returns: the XML source for the node as a UTF8 string.  The caller is responsible for freeing this with g_free
 */
gchar*
cong_node_generate_source (CongNodePtr node)
{
	xmlBufferPtr xml_buffer;
	gchar *result;

	g_return_val_if_fail (node, NULL);


	g_assert (node->doc);

	xml_buffer = xmlBufferCreate();

	switch (cong_node_type (node)) {
	case CONG_NODE_TYPE_DOCUMENT:
		g_assert_not_reached();
		break;
	default:
		xmlNodeDump (xml_buffer,
			     node->doc,
			     node,
			     0,
			     FALSE);
		break;
	}

	result = g_strdup ((const gchar*)xmlBufferContent (xml_buffer));

	xmlBufferFree (xml_buffer);

	return result;
}

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
cong_node_generate_child_source (CongNodePtr node)
{
	xmlBufferPtr xml_buffer;
	gchar *result;
	CongNodePtr iter;

	g_return_val_if_fail (node, NULL);

	g_assert (node->doc);

	xml_buffer = xmlBufferCreate();

	iter = node->children;

	do {
		if (iter) {
			switch (cong_node_type (iter)) {
			case CONG_NODE_TYPE_DOCUMENT:
				g_assert_not_reached();
				break;
			default:
				xmlNodeDump (xml_buffer,
					     iter->doc,
					     iter,
					     0,
					     FALSE);
				break;
			}
		}

		if (iter==node->last) {
			break;
		} else {
			iter=iter->next;
		}

	} while (1);

	result = g_strdup ((const gchar*)xmlBufferContent (xml_buffer));

	xmlBufferFree (xml_buffer);

	return result;	
}

/**
 * cong_node_generate_source_from_byte_offset:
 * @node:
 * @byte_offset:
 *
 * TODO: Write me
 * Returns:
 */
gchar*
cong_node_generate_source_from_byte_offset (CongNodePtr node,
					    int start_byte_offset)
{
	if (node->content) {
		g_assert (start_byte_offset<=strlen((const char*)node->content));

		return cong_node_generate_source_between_byte_offsets (node,
								       start_byte_offset,
								       strlen((const gchar*)node->content));

	} else {
		return g_strdup("");
	}
}

/* Generate XML source from TEXT and COMMENT nodes as a UTF8 string, up to the byte offset into the UTF-8: 
   FIXME: specify the end-point more precisely
 */
/**
 * cong_node_generate_source_up_to_byte_offset:
 * @node:
 * @byte_offset:
 *
 * TODO: Write me
 * Returns:
 */
gchar*
cong_node_generate_source_up_to_byte_offset (CongNodePtr node,
					     int end_byte_offset)
{
	if (node->content) {
		g_assert (end_byte_offset<=xmlStrlen(node->content));

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
/**
 * cong_node_generate_source_between_byte_offsets:
 * @node:
 * @start_byte_offset:
 * @end_byte_offset:
 *
 * TODO: Write me
 * Returns:
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
		g_assert (start_byte_offset<=xmlStrlen(node->content));
		g_assert (end_byte_offset<=xmlStrlen(node->content));
	} else {
		return g_strdup("");
	}

	clipped_content = g_strndup ((const gchar*)node->content + start_byte_offset, end_byte_offset - start_byte_offset);

	/* g_message("clipped content = \"%s\"", clipped_content); */

	switch (node->type) {
	default: g_assert_not_reached();
	case XML_TEXT_NODE:
		result = cong_util_dup_and_free_xml_string (xmlEncodeSpecialChars(node->doc, (const xmlChar*)clipped_content));
		break;

	case XML_COMMENT_NODE:
		result = g_strdup_printf("<!--%s-->" , clipped_content);
		break;
	}

	g_assert(result);

	/* g_message("result =\"%s\"", result); */

	g_free (clipped_content);

	return result;
}

/**
 * cong_node_recursive_set_doc:
 * @node:
 * @xml_doc:
 *
 * TODO: Write me
 */
void
cong_node_recursive_set_doc(CongNodePtr node, xmlDocPtr xml_doc)
{
	CongNodePtr iter;

	node->doc = xml_doc;

	for (iter = node->children; iter; iter=iter->next) {
		cong_node_recursive_set_doc(iter, xml_doc);
	}
}

/**
 * cong_node_recursive_dup:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr 
cong_node_recursive_dup(CongNodePtr node)
{
	CongNodePtr new_node = xmlCopyNode(node, TRUE);

	/* Unfortunately, this doesn't preserve the doc ptrs, so we must reconstruct these: */
	cong_node_recursive_set_doc(new_node, node->doc);

	/* FIXME: this is an evil hack, and this whole function should be deprecated */

	return new_node;
}

/**
 * cong_node_is_descendant_of:
 * @node:
 * @potential_ancestor:
 *
 * TODO: Write me
 * Returns:
 */
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

/* Utility for tree manipulation: */
static void
update_entities (CongNodePtr node)
{
	g_return_if_fail (node);

	if (cong_node_type (node)==CONG_NODE_TYPE_ENTITY_DECL) {
		xmlEntityPtr ent = (xmlEntityPtr)node;
		gchar *child_source;

		switch (ent->etype) {
		case XML_INTERNAL_GENERAL_ENTITY:
		case XML_INTERNAL_PARAMETER_ENTITY:
			/* refresh entity source */
			#if 0
			g_message ("need to refresh entity \"%s\"", node->name);
			#endif

			if (ent->orig) {
				xmlFree (ent->orig);
			}
			
			child_source = cong_node_generate_child_source (node);
			ent->orig = xmlStrdup ((const xmlChar*)child_source);
			g_free (child_source);
			break;

		case XML_INTERNAL_PREDEFINED_ENTITY:
		case XML_EXTERNAL_GENERAL_PARSED_ENTITY:
		case XML_EXTERNAL_GENERAL_UNPARSED_ENTITY:
		case XML_EXTERNAL_PARAMETER_ENTITY:
			/* FIXME: potentially update the document's file's "modified" information:  move to CongDocument level? */
			break;

		}

		/* FIXME: amortize these updates... move to the CongDocument level? */
	}

	if (node->parent) {
		update_entities (node->parent);
	}
}

/* Tree manipulation: */
/**
 * cong_node_private_make_orphan:
 * @node:
 *
 * TODO: Write me
 */
void 
cong_node_private_make_orphan(CongNodePtr node)
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

	if (former_parent) {
		update_entities (former_parent);
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

/**
 * cong_node_private_add_after:
 * @node:
 * @older_sibling:
 *
 * TODO: Write me
 */
void 
cong_node_private_add_after(CongNodePtr node, CongNodePtr older_sibling)
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

	update_entities (node);

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

/**
 * cong_node_private_add_before:
 * @node:
 * @younger_sibling:
 *
 * TODO: Write me
 */
void 
cong_node_private_add_before(CongNodePtr node, CongNodePtr younger_sibling)
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

	update_entities (node);

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

/**
 * cong_node_private_set_parent:
 * @node:
 * @adoptive_parent:
 * @add_to_end:
 *
 * TODO: Write me
 */
void 
cong_node_private_set_parent(CongNodePtr node, CongNodePtr adoptive_parent, gboolean add_to_end)
{
	LOG_NODE_PRIVATE_MODIFICATION("cong_node_private_set_parent");

	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);
	g_return_if_fail(node!=adoptive_parent);

	CONG_NODE_SELF_TEST(node);
	CONG_NODE_SELF_TEST(adoptive_parent);

	cong_node_private_make_orphan(node);

#if 1
	if (add_to_end) {
		if (adoptive_parent->last) {
			cong_node_private_add_after(node, adoptive_parent->last);
		} else {
			g_assert(adoptive_parent->children == NULL);
			
			adoptive_parent->children = node;
			adoptive_parent->last = node;
			node->parent = adoptive_parent;
		}
	} else {
		if (adoptive_parent->children) {
			cong_node_private_add_before(node, adoptive_parent->children);
		} else {
			g_assert(adoptive_parent->last == NULL);
			
			adoptive_parent->children = node;
			adoptive_parent->last = node;
			node->parent = adoptive_parent;
		}
	}

	update_entities (node);

	/* Postconditions: */
	{
		g_assert(node->parent == adoptive_parent);
		if (add_to_end) {
			g_assert(adoptive_parent->last == node);
			g_assert(node->next == NULL);
		} else {
			g_assert(adoptive_parent->children == node);
			g_assert(node->prev == NULL);
		}
		CONG_NODE_SELF_TEST(node);
		CONG_NODE_SELF_TEST(adoptive_parent);
	}
#endif
#if 0
	xmlAddChild(adoptive_parent, node);
#endif

}

/**
 * cong_node_private_set_text:
 * @node:
 * @new_content:
 *
 * TODO: Write me
 */
void 
cong_node_private_set_text(CongNodePtr node, const gchar *new_content)
{
	LOG_NODE_PRIVATE_MODIFICATION("cong_node_private_set_text");

	g_return_if_fail(node);
	g_return_if_fail(new_content);

	xmlNodeSetContent(node, (const xmlChar*)new_content);

	update_entities (node);
}

/**
 * cong_node_private_set_attribute:
 * @node:
 * @ns_ptr:
 * @local_attribute_name:
 * @value:
 *
 * TODO: Write me
 */
void 
cong_node_private_set_attribute(CongNodePtr node,
				xmlNs *ns_ptr, 
				const gchar *local_attribute_name,
				const gchar *value)
{
	LOG_NODE_PRIVATE_MODIFICATION("cong_node_private_set_attribute");

	g_return_if_fail(node);
	g_return_if_fail(local_attribute_name);
	g_return_if_fail(value);
	
	if(ns_ptr == NULL)
		xmlSetProp(node, (const xmlChar*)local_attribute_name, (const xmlChar*)value);
	else
		xmlSetNsProp(node, ns_ptr, (const xmlChar*)local_attribute_name, (const xmlChar*)value);

	update_entities (node);
}

/**
 * cong_node_private_remove_attribute:
 * @node:
 * @ns_ptr:
 * @local_attribute_name:
 *
 * TODO: Write me
 */
void 
cong_node_private_remove_attribute(CongNodePtr node, 
				   xmlNs *ns_ptr,
				   const gchar *local_attribute_name)
{
	LOG_NODE_PRIVATE_MODIFICATION("cong_node_private_remove_attribute");

	g_return_if_fail(node);
	g_return_if_fail(local_attribute_name);

	if(ns_ptr == NULL)
		xmlUnsetProp(node, (const xmlChar*)local_attribute_name);
	else
		xmlUnsetNsProp(node, ns_ptr, (const xmlChar*)local_attribute_name);

	update_entities (node);
}

/* Utilities: */
/**
 * cong_node_get_child_by_name:
 * @node:  the parent node
 * @ns_uri: URI of namespace to search for, or NULL
 * @local_name: the local name within any namespace of the element to search for
 *
 * This function searches the children of @node looking for elements of the given name.
 *
 * Returns: the first child element matching the given name, or NULL if there are none
 */
CongNodePtr 
cong_node_get_child_by_name (CongNodePtr node, 
			     const gchar *ns_uri, 
			     const gchar *local_name)
{
	CongNodePtr iter;

	g_return_val_if_fail (node, NULL);
	g_return_val_if_fail (local_name, NULL);

	for (iter=node->children; iter; iter=iter->next) {
		if (cong_node_is_element (iter, ns_uri, local_name)) {
			return iter;
		}
	}

	return NULL;
}

/**
 * cong_node_get_first_text_node_descendant:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
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

/**
 * cong_node_get_whitespace_handling:
 * @doc:
 * @text_node:
 *
 * TODO: Write me
 * Returns:
 */
CongWhitespaceHandling
cong_node_get_whitespace_handling (CongDocument *doc,
				   CongNodePtr text_node)
{
	g_return_val_if_fail (doc, CONG_WHITESPACE_NORMALIZE);
	g_return_val_if_fail (text_node, CONG_WHITESPACE_NORMALIZE);
	g_return_val_if_fail (cong_node_type_is_textual_content (cong_node_type (text_node)), CONG_WHITESPACE_NORMALIZE);

	if (cong_node_type (text_node->parent)==CONG_NODE_TYPE_ELEMENT) {
		CongDispspecElement *ds_element = cong_document_get_dispspec_element_for_node  (doc, text_node->parent);

		if (ds_element) {
			return cong_dispspec_element_get_whitespace (ds_element);
		}
	}
	
	return CONG_WHITESPACE_NORMALIZE;
}

/**
 * cong_node_should_be_visible_in_editor:
 * @node:  a node to be tested
 *
 * The function determines if the node ought to be visible in the main editor view.
 *
 * As described in bug #123367, TEXT nodes that are either empty or purely whitespace
 * should only appear in the main editor view if the DTD allows PCDATA at the location in the
 * document.  Otherwise the text is probably merely formatting to prettify the source view. *
 *
 * Returns: a #gboolean which is TRUE iff the node ought to be visible in the main editor view
 */
gboolean
cong_node_should_be_visible_in_editor (CongNodePtr node)
{
	g_return_val_if_fail (node, FALSE);

	if (cong_node_type (node) == CONG_NODE_TYPE_TEXT) {
		if (node->parent) {
			xmlElementPtr dtd_entry = NULL;
			
			/* If the DTD doesn't allow #PCDATA in this node and it only
			   contains whitespace we should ignore it (if it does it
			   shouldn't validate we should add an error marked element).
			*/
			/* Bulletproof the routine to deal with TEXT nodes immediately below DOCUMENT (which ould have a NULL name) and similar cases: */
			if (node->parent->name) {
				dtd_entry = xmlGetDtdElementDesc (node->doc->extSubset, node->parent->name);
			}
			if (dtd_entry) {
				if (cong_dtd_element_content_can_contain_pcdata (dtd_entry->content)) {
					return TRUE;
				} else if (!cong_util_is_pure_whitespace ((const gchar*)node->content)) {
					return TRUE;
				} else {
					return FALSE;
				}
			} else {
				return TRUE;
			}
		} else {
			/* Node isn't yet part of tree, so we can't tell; assume TRUE to stop cursor tests failing: */
			return TRUE;
		}
	} else {
		return TRUE;
	}
}

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
cong_node_is_valid_cursor_location (CongNodePtr node)
{
	g_return_val_if_fail (node, FALSE);

	switch (node->type) {
	default: return FALSE;
	case XML_TEXT_NODE:
		return cong_node_should_be_visible_in_editor (node);

	case XML_CDATA_SECTION_NODE:
		return TRUE;
		
	case XML_COMMENT_NODE:
		/* Eventually allow comment editing: */		
		return FALSE;
	}
}

/**
 * cong_node_supports_byte_offsets:
 * @node:  a node to be tested
 *
 * The function determines if #CongLocation objects that reference this node can have meaningful byte offsets
 *
 * Only TEXT, COMMENT and CDATA_SECTION nodes can currently have meaningful byte offsets.
 * 
 * Returns: a #gboolean which is TRUE if #CongLocations that reference this node can have a meaningful byte offset 
 */
gboolean
cong_node_supports_byte_offsets (CongNodePtr node)
{
	g_return_val_if_fail (node, FALSE);
	
	return cong_node_type_is_textual (node->type);
}

/**
 * cong_node_can_be_cut:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_node_can_be_cut (CongNodePtr node)
{
	g_return_val_if_fail (node, FALSE);

	g_assert (node->doc);

#if 0
	/* Forbid cutting the root element of the document: */
	if (node->doc->==node) {
		return FAlSE;
	}
#endif

	/* Forbid all but the easy cases for now: we want stability rather than features: */
	switch (cong_node_type (node)) {
	default: 
		return FALSE;

	case CONG_NODE_TYPE_TEXT:
	case CONG_NODE_TYPE_ELEMENT:
	case CONG_NODE_TYPE_COMMENT:
		return TRUE;
	}
}

/**
 * cong_node_can_be_copied:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_node_can_be_copied (CongNodePtr node)
{
	/* FIXME: should be different conditions (fix after 0.8.0) */
	return cong_node_can_be_cut(node);
}

/**
 * cong_node_get_deepest_common_parent:
 * @n0:
 * @n1:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr 
cong_node_get_deepest_common_parent (CongNodePtr n0, 
				     CongNodePtr n1)
{
	CongNodePtr iter0, iter1;

	g_return_val_if_fail (n0, NULL);
	g_return_val_if_fail (n1, NULL);
	g_return_val_if_fail (n0->doc == n1->doc, NULL);

	for (iter0=n0; iter0; iter0=iter0->parent) {
		for (iter1=n1; iter1; iter1=iter1->parent) {
			if (iter0==iter1) {
				return iter0;
			}
		}
	}

	return NULL;
}

/**
 * cong_node_get_ordering:
 * @n0:  first node to be compared
 * @n1:  second node to be compared
 *
 * This functions compares the location of two nodes in the xml tree and returns a numeric comparsion representing
 * their locations in a depth-first traversal.
 *
 * Returns: negative if n0 is reached before n1, zero if they are the same node, positive if n0 is reached after n1
 */
int 
cong_node_get_ordering (CongNodePtr n0,
			CongNodePtr n1)
{
	CongNodePtr deepest_common_parent;

#if 0
	g_assert (n0);
	g_assert (n1);
	g_assert (n0->parent || cong_node_type (n0)==CONG_NODE_TYPE_DOCUMENT);
	g_assert (n1->parent || cong_node_type (n1)==CONG_NODE_TYPE_DOCUMENT);
#endif

	g_return_val_if_fail (n0, 0);
	g_return_val_if_fail (n1, 0);
	g_return_val_if_fail (n0->doc == n1->doc, 0);
	g_return_val_if_fail (n0->parent || cong_node_type (n0)==CONG_NODE_TYPE_DOCUMENT, 0);
	g_return_val_if_fail (n1->parent || cong_node_type (n1)==CONG_NODE_TYPE_DOCUMENT, 0);

	deepest_common_parent = cong_node_get_deepest_common_parent (n0, n1);

	while (1) {
		if (n0==n1) {
			return 0;
		}

		/* Easy case: the same parent: */
		if (n0->parent == n1->parent) {
			CongNodePtr iter;
			
			/* Scan forward from n0 looking for n1: */
			for (iter = n0; iter && iter != n1; iter = iter->next) ;
			
			if (NULL==iter) {
				/* Didn't find n1?  Must have been earlier */
				return 1;
			} else {
				return -1;
			}
		}
		
		if (n0->parent == n1) {
			return -1;
		}
		
		if (n1->parent == n0) {
			return 1;
		}
		
		/* Different parents, and the nodes are not each other's parents - ordering is determined by both nodes parents provided we don't go to their deepest common parent (or above): */
		if (n0->parent) {
			if (n0!=deepest_common_parent && n0->parent!=deepest_common_parent) {
				n0=n0->parent;
			}
		}
		if (n1->parent) {
			if (n1!=deepest_common_parent && n1->parent!=deepest_common_parent) {
				n1=n1->parent;
			}
		}
	}
}

/**
 * cong_node_calc_first_node_in_subtree_satisfying:
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
 */
CongNodePtr
cong_node_calc_first_node_in_subtree_satisfying (CongNodePtr node,
						 CongNodePredicate predicate,
						 gpointer user_data)
{
	CongNodePtr iter;

	g_return_val_if_fail (node, NULL);
	g_return_val_if_fail (predicate, NULL);

	/* If the current node matches the predicates, return it. */
	if (predicate (node, user_data)) {
		return node;
	}

	/* Otherwise run through its children, and recursively find the first
	 * satisfying node. */
	for (iter = node->children; iter; iter = iter->next) {
		CongNodePtr first = cong_node_calc_first_node_in_subtree_satisfying (iter, 
										     predicate,
										     user_data);

		if (first) {
			return first;
		}
	}

	return NULL;
}

/**
 * cong_node_calc_final_node_in_subtree_satisfying:
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
 */
CongNodePtr
cong_node_calc_final_node_in_subtree_satisfying (CongNodePtr node, 
						 CongNodePredicate predicate,
						 gpointer user_data)
{
	CongNodePtr iter;

	g_return_val_if_fail (node, NULL);
	g_return_val_if_fail (predicate, NULL);

	/* "node" is treated as being in its own subtree */

	for (iter = node->last; iter; iter=iter->prev) {
		CongNodePtr final = cong_node_calc_final_node_in_subtree_satisfying (iter,
										     predicate,
										     user_data);
		
		if (final) {
			return final;
		}		
	}

	/* Not found in any children of this node, try this node: */
	if (predicate (node, user_data)) {
		return node;
	} else {
		return NULL;
	}
}

/**
 * cong_node_calc_prev_node_satisfying:
 * @node: the start of the search
 * @predicate: the #CongNodePredicate to test nodes for
 * @user_data: user-supplied data passed to the predicate
 *
 * Finds the first preceding node relative to the input that satisfies the predicate,
 * in an imagined depth-first traversal of the document.  Includes ancestors.
 *
 * Returns: the appropriate node satisfying @predicate, or NULL if there are none
 */
CongNodePtr
cong_node_calc_prev_node_satisfying (CongNodePtr node, 
				     CongNodePredicate predicate,
				     gpointer user_data)
{
	g_return_val_if_fail (node, NULL);
	g_return_val_if_fail (predicate, NULL);

	/* Search through subtrees of siblings to the left of this node: */
	{
		CongNodePtr iter;

		for (iter = node->prev; iter; iter = iter->prev) {
			CongNodePtr final = cong_node_calc_final_node_in_subtree_satisfying (iter, 
											     predicate,
											     user_data);
			
			if (final) {
				return final;
			}
		}
	}

	/* If not found, try parent node, and then recurse: */
	if (node->parent) {
		if (predicate(node->parent, user_data)) {
			return node->parent;
		} else {
			return cong_node_calc_prev_node_satisfying (node->parent, 
								    predicate,
								    user_data);
		} 
	} else {
		return NULL;
	}
}

/**
 * cong_node_calc_next_node_satisfying:
 * @node: the start of the search
 * @predicate: the #CongNodePredicate to test nodes for
 * @user_data: user-supplied data passed to the predicate
 *
 * Finds the first following node relative to the input that satisfies the predicate,
 * in an imagined depth-first traversal of the document.   Includes ancestors.
 *
 * Returns: the appropriate node satisfying @predicate, or NULL if there are none
 */
CongNodePtr
cong_node_calc_next_node_satisfying (CongNodePtr node,
				     CongNodePredicate predicate,
				     gpointer user_data)
{
	CongNodePtr iter;
	
	g_return_val_if_fail (node, NULL);
	g_return_val_if_fail (predicate, NULL);

	/* Search through subtrees of siblings to the right of this node */
	for (iter = node->next; iter; iter = iter->next) {
		CongNodePtr first = cong_node_calc_first_node_in_subtree_satisfying (iter, 
										     predicate,
										     user_data);

		if (first) {
			return first;
		}
	}

	/* If not found, try parent node, and then recurse: */
	if (node->parent) {
		if (predicate (node->parent, user_data)) {
			return node->parent;
		} else {
			return cong_node_calc_next_node_satisfying (node->parent,
								    predicate,
								    user_data);
		}
	} else {
		return NULL;
	}
}
