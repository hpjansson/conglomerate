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
#if 1
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
#else
	TTREE *n0;
	UNUSED_VAR(TTREE *n1)
	char *s = 0, *s_sub;

	n0 = x->child->child;
	if (!n0) return(0);

	s = malloc(1);
	*s = 0;
	
	for ( ; n0; n0 = n0->next)
	{
		if (xml_frag_type(n0) == XML_DATA)
		{
			s = strdcat(s, xml_frag_data_nice(n0));
		}
		else if (xml_frag_type(n0) == XML_TAG_SPAN)
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
#endif
}


TTREE *xml_inner_span_element(CongDispspec *ds, TTREE *x)
{
	if (x->parent) x = x->parent;
	else return(0);

	if (x->parent) x = x->parent;
	else return(0);

	if (!strcmp("tag_span", x->data) && cong_dispspec_element_span(ds, x->child->data))
		return(x);

	return(0);
}


TTREE *xml_outer_span_element(CongDispspec *ds, TTREE *x)
{
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
}


void xml_tag_remove(TTREE *x)
{
	TTREE *n0;
	
	if (!x) return;  /* He. */
	
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
}
