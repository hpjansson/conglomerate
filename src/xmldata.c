/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>                                                            
#include <gtk/gtk.h>                                                            

#include <stdlib.h>

#include <ttree.h>
#include <xml.h>
#include <strtool.h>
#include "global.h"

char fake_data[] = "";

#if NEW_XML_IMPLEMENTATION
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
#endif

enum CongNodeType cong_node_type(CongNodePtr node)
{
	g_return_val_if_fail(node, CONG_NODE_TYPE_UNKNOWN);

#if NEW_XML_IMPLEMENTATION
	switch (node->type) {

	default: g_assert(0);

	case XML_ELEMENT_NODE: 
		return CONG_NODE_TYPE_ELEMENT;

	case XML_TEXT_NODE: 
		return CONG_NODE_TYPE_TEXT;

	case XML_COMMENT_NODE: 
		return CONG_NODE_TYPE_COMMENT;
	}
#else
	switch (xml_frag_type(node)) {

	default: g_assert(0);

	case XML_UNKNOWN:
		return CONG_NODE_TYPE_UNKNOWN;

	case XML_TAG_EMPTY:
		/* the code shouldn't support these TTREE node types as CongNode */
		g_assert(0);

	case XML_ATTR:
		/* the code shouldn't support these TTREE node types as CongNode; however they do occur at the moment, so we handle them gracefully */
		return CONG_NODE_TYPE_UNKNOWN;		

	case XML_DATA:
		return CONG_NODE_TYPE_TEXT;

	case XML_TAG_SPAN:
		return CONG_NODE_TYPE_ELEMENT;

	}
#endif

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

void cong_node_self_test(CongNodePtr node)
{
#if NEW_XML_IMPLEMENTATION
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
#endif
}

void cong_node_self_test_recursive(CongNodePtr node)
{
#if NEW_XML_IMPLEMENTATION
	CongNodePtr iter;
	cong_node_self_test(node);

	for (iter=node->children; iter; iter=iter->next) {
		cong_node_self_test_recursive(iter);
	}
#endif
}

int cong_node_get_length(CongNodePtr node)
{
	/* get length of content; does not include the zero terminator (to correspond to the TTREE size field) */
	g_return_val_if_fail( (cong_node_type(node) == CONG_NODE_TYPE_TEXT) || (cong_node_type(node) == CONG_NODE_TYPE_COMMENT), 0);

#if NEW_XML_IMPLEMENTATION
	return xmlStrlen(node->content);
#else
	return node->child->size;
#endif
	
}

/* Construction: */
CongNodePtr cong_node_new_element(const char *tagname)
{
#if !NEW_XML_IMPLEMENTATION
	TTREE *dummy;
	CongNodePtr new_node;
#endif
	g_return_val_if_fail(tagname, NULL);

#if NEW_XML_IMPLEMENTATION
	return xmlNewNode(NULL, tagname); /* FIXME: audit the character types here */
#else
	/* Have to create as a child of something, for some reason: */
	dummy = ttree_node_add(0, "d", 1);

	new_node = ttree_node_add(dummy, "tag_span", 8);
	ttree_node_add(new_node, tagname, strlen(tagname));

	dummy->child = 0;
	ttree_branch_remove(dummy);

	return new_node;
#endif
}

CongNodePtr cong_node_new_text(const char *text)
{
	return cong_node_new_text_len(text, strlen(text));
}

CongNodePtr cong_node_new_text_len(const char *text, int len)
{
#if !NEW_XML_IMPLEMENTATION
	TTREE *dummy;
	CongNodePtr new_node;
#endif

	g_return_val_if_fail(text, NULL);

#if NEW_XML_IMPLEMENTATION
	return xmlNewTextLen(text, len); /* FIXME: audit the character types here */
#else
	/* Have to create as a child of something, for some reason: */
	dummy = ttree_node_add(0, "d", 1);

	new_node = ttree_node_add(dummy, "data", 4);
	ttree_node_add(new_node, text, len);

	dummy->child = 0;
	ttree_branch_remove(dummy);

	return new_node;
#endif
	

}

/* Destruction: (the node has to have been unlinked from the tree already): */

void cong_node_free(CongNodePtr node)
{
	g_return_if_fail(node);

#if NEW_XML_IMPLEMENTATION
	xmlFreeNode(node);
#else
	g_assert(cong_node_type(node)==CONG_NODE_TYPE_TEXT); /* for now */
	if (node->child)
	{
		if (node->child->data) free(node->child->data);
		free(node->child);
	}

	if (node->data) free(node->data);
	free(node);
#endif
	
}

/* Tree manipulation: */
#if NEW_XML_IMPLEMENTATION
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
#endif

#if NEW_XML_IMPLEMENTATION
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
#endif

#if NEW_XML_IMPLEMENTATION
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
#endif

#if NEW_XML_IMPLEMENTATION
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
#endif


const char *xml_frag_data_nice(CongNodePtr x)
{
	const char *s;
	
#if NEW_XML_IMPLEMENTATION
	g_return_val_if_fail(x->type==XML_TEXT_NODE, NULL);
	
	s = x->content; /* FIXME:  hackish cast from xmlChar* to char* */
	if (!s) s = fake_data;
#else
	s = xml_frag_data(x);
	if (!s) s = fake_data;
#endif
	
	return(s);
}


const char *xml_frag_name_nice(CongNodePtr x)
{
	const char *s;
	
#if NEW_XML_IMPLEMENTATION
	s = cong_node_name(x);
	if (!s) s = fake_data;
#else
	s = xml_frag_name(x);
	if (!s) s = fake_data;
#endif

	return(s);
}


/* Tested and works */

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
			s = strdcat(s, xml_frag_data_nice(n0));
		}
		else if (cong_node_type(n0) == CONG_NODE_TYPE_ELEMENT)
		{
			s_sub = xml_fetch_clean_data(n0);
			if (s_sub)
			{
				s = strdcat(s, s_sub);
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

#if NEW_XML_IMPLEMENTATION
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

#else
	if (x->parent) x = x->parent;
	else return(0);

	if (x->parent) x = x->parent;
	else return(0);

	if (!strcmp("tag_span", x->data) && cong_dispspec_element_span(ds, x->child->data))
		return(x);

#endif

	return NULL;

}


CongNodePtr xml_outer_span_element(CongDispspec *ds, CongNodePtr x)
{
#if NEW_XML_IMPLEMENTATION
	g_assert(0);
	return NULL;
#else
	TTREE *n0 = 0;
	
	if (x->parent) x = x->parent;
	else return(0);

	if (x->parent) x = x->parent;
	else return(0);

	for (;;)
	{
	  if (!strcmp("tag_span", x->data) && cong_dispspec_element_span(ds, x->child->data))
		  n0 = x;
		else break;
		
	  if (x->parent) x = x->parent;
	  else break;

	  if (x->parent) x = x->parent;
	  else break;
	}

	return(n0);
#endif
}

/*
  Function to remove a node x from the tree; all its children become children of x's parents in the natural place in the tree.
 */
void xml_tag_remove(CongNodePtr x)
{
	CongNodePtr n0;

	g_return_if_fail(x);

	/* GREP FOR MVC */

#if NEW_XML_IMPLEMENTATION
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
#else
	n0 = x->child->child;
	n0->prev = x->prev;
	if (!x->prev) x->parent->child = n0;
	else x->prev->next = n0;
	
	for (; n0->next; n0 = n0->next) n0->parent = x->parent;
	
	n0->parent = x->parent;
	n0->next = x->next;
	if (x->next) x->next->prev = n0;
	
	x->next = 0;
	x->prev = 0;
	x->parent = 0;
	if (x->child) x->child->child = 0;
	ttree_branch_remove(x);
#endif
}






