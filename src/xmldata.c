/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>                                                            
#include <gtk/gtk.h>                                                            

#include <stdlib.h>
#include <string.h>

#include "global.h"

char fake_data[] = "";

const char* cong_node_name(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return node->name;
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

	default: g_assert(0);

	case XML_ELEMENT_NODE: 
		return CONG_NODE_TYPE_ELEMENT;

	case XML_TEXT_NODE: 
		return CONG_NODE_TYPE_TEXT;

	case XML_COMMENT_NODE: 
		return CONG_NODE_TYPE_COMMENT;

	case XML_ATTRIBUTE_NODE:
	case XML_CDATA_SECTION_NODE:
	case XML_ENTITY_REF_NODE:
	case XML_ENTITY_NODE:
	case XML_PI_NODE:
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
		return CONG_NODE_TYPE_UNKNOWN;
	}

}

static const gchar* node_type_names[CONG_NODE_TYPE_NUM]=
{
	"CONG_NODE_TYPE_UNKNOWN",
	"CONG_NODE_TYPE_ELEMENT",
	"CONG_NODE_TYPE_TEXT",
	"CONG_NODE_TYPE_COMMENT"
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

	for (iter=node->children; iter; iter=iter->next) {
		g_assert(iter->parent == node);
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

/* Construction: */
CongNodePtr cong_node_new_element(const char *tagname)
{
	g_return_val_if_fail(tagname, NULL);

	return xmlNewNode(NULL, tagname); /* FIXME: audit the character types here */
}

CongNodePtr cong_node_new_text(const char *text)
{
	return cong_node_new_text_len(text, strlen(text));
}

CongNodePtr cong_node_new_text_len(const char *text, int len)
{
	g_return_val_if_fail(text, NULL);

	return xmlNewTextLen(text, len); /* FIXME: audit the character types here */
}

/* Destruction: (the node has to have been unlinked from the tree already): */

void cong_node_free(CongNodePtr node)
{
	g_return_if_fail(node);

	xmlFreeNode(node);
}


void cong_node_recursive_delete(CongDocument *doc, CongNodePtr node)
{
	CongNodePtr iter, next;

	CONG_NODE_SELF_TEST(node);

	iter = node->children; 

	while (iter) {
		next = iter->next;

		CONG_NODE_SELF_TEST(iter);
		
		cong_node_recursive_delete(doc, iter);

		iter = next;
	}

	g_assert(node->children==NULL);
	g_assert(node->last==NULL);

	cong_document_node_make_orphan(doc, node);

	cong_node_free(node);
}

CongNodePtr cong_node_recursive_dup(CongNodePtr node)
{
	return xmlCopyNode(node, TRUE);
}

/* Tree manipulation: */
void cong_node_make_orphan(CongNodePtr node)
{
	CongNodePtr former_parent;
	CongNodePtr former_prev;
	CongNodePtr former_next;

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

void cong_node_add_after(CongNodePtr node, CongNodePtr older_sibling)
{
	g_return_if_fail(node);
	g_return_if_fail(older_sibling);
	g_return_if_fail(older_sibling->parent);
	g_return_if_fail(node!=older_sibling);

	CONG_NODE_SELF_TEST(node);
	CONG_NODE_SELF_TEST(older_sibling);

	cong_node_make_orphan(node);

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
}

void cong_node_add_before(CongNodePtr node, CongNodePtr younger_sibling)
{
	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);
	g_return_if_fail(younger_sibling->parent);
	g_return_if_fail(node!=younger_sibling);

	CONG_NODE_SELF_TEST(node);
	CONG_NODE_SELF_TEST(younger_sibling);

	cong_node_make_orphan(node);

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
}

void cong_node_set_parent(CongNodePtr node, CongNodePtr adoptive_parent)
{
	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);
	g_return_if_fail(node!=adoptive_parent);

	CONG_NODE_SELF_TEST(node);
	CONG_NODE_SELF_TEST(adoptive_parent);

	cong_node_make_orphan(node);

	if (adoptive_parent->last) {
		cong_node_add_after(node, adoptive_parent->last);
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
}

void cong_node_set_text(CongNodePtr node, const xmlChar *new_content)
{
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	xmlNodeSetContent(node, new_content);
}

const char *xml_frag_data_nice(CongNodePtr x)
{
	const char *s;
	
	g_return_val_if_fail(x->type==XML_TEXT_NODE, NULL);
	
	s = x->content; /* FIXME:  hackish cast from xmlChar* to char* */
	if (!s) s = fake_data;
	
	return(s);
}


const char *xml_frag_name_nice(CongNodePtr x)
{
	const char *s;
	
	s = cong_node_name(x);
	if (!s) s = fake_data;

	return(s);
}


/* Tested and works */

static char *cat_string(char *head, const char *tail)
{
	char *new = g_malloc(strlen(head) + strlen(tail) + 1);
	strcpy(new, head);
	strcat(new, tail);
	g_free(head);

	return(new);
}

/* Recursively traverses the document from that node concatenating the character data into a string */
char *xml_fetch_clean_data(CongNodePtr x)
{
	CongNodePtr n0;
	char *s = 0, *s_sub;

	n0 = cong_node_first_child(x);
	if (!n0) return NULL;

	s = malloc(1);
	*s = '\0';
	
	for ( ; n0; n0 = cong_node_next(n0))
	{
		if (cong_node_type(n0) == CONG_NODE_TYPE_TEXT)
		{
			s = cat_string(s, xml_frag_data_nice(n0));
		}
		else if (cong_node_type(n0) == CONG_NODE_TYPE_ELEMENT)
		{
			s_sub = xml_fetch_clean_data(n0);
			if (s_sub)
			{
				s = cat_string(s, s_sub);
				free(s_sub);
			}
		}
	}
	
	return(s);
}


CongNodePtr xml_inner_span_element(CongDispspec *ds, CongNodePtr x)
{
	g_return_val_if_fail(ds, NULL);
	g_return_val_if_fail(x, NULL);

	if (cong_node_parent(x)) {

		x = cong_node_parent(x);

	} else {
		return NULL;
	}
	
	if (cong_node_type(x)==CONG_NODE_TYPE_ELEMENT) {
		CongDispspecElement* element = cong_dispspec_lookup_node(ds, x);

		if (element) {
			if (CONG_ELEMENT_TYPE_SPAN == cong_dispspec_element_type(element)) {
				return(x);
			}
		}
	}

	return NULL;

}


CongNodePtr xml_outer_span_element(CongDispspec *ds, CongNodePtr x)
{
	CongNodePtr n0 = NULL;

	g_return_val_if_fail(ds, NULL);
	g_return_val_if_fail(x, NULL);

	if (x->parent) {
		x = x->parent;
	} else {
		return NULL;
	}

	for (;;)
	{
		if ( cong_node_type(x) == CONG_NODE_TYPE_ELEMENT && cong_dispspec_element_span(ds, cong_node_name(x))) {
			n0 = x;
		} else { 
			break;
		}
		
		if (x->parent) {
			x = x->parent;
		} else {
			break;
		}
	}

	return(n0);
}

/*
  Function to remove a node x from the tree; all its children become children of x's parents in the natural place in the tree.
 */
void xml_tag_remove(CongDocument *doc, CongNodePtr x)
{
	CongNodePtr n0;
	CongNodePtr n0_next;

	g_return_if_fail(x);

	/* GREP FOR MVC */

#if 1
	for (n0 = x->children; n0; n0 = n0_next) {
		n0_next = n0->next;
		
		cong_document_node_add_before(doc, n0, x);
	}

	cong_document_node_make_orphan(doc, x);

	cong_node_free(x);
#else
	n0 = cong_node_first_child(x);

	if (n0) {
		n0->prev = x->prev;
	}

	if (NULL==x->prev) {
		x->parent->children = n0;
	} else {
		x->prev->next = n0;
	}

	for (; n0->next; n0 = n0->next) {
		n0->parent = x->parent;
	}
	n0->parent = x->parent;

	n0->next = x->next;
	if (x->next) {
		x->next->prev = n0;
	} else {
		x->parent->last = n0;
	}
	
	x->next = NULL;
	x->prev = NULL;
	x->parent = NULL;
	x->children = NULL;
	x->last = NULL;

	xmlFreeNode(x);
#endif
}






