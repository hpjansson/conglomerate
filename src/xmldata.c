/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>                                                            
#include <gtk/gtk.h>                                                            

#include <stdlib.h>

#include <ttree.h>
#include <xml.h>
#include <strtool.h>
#include "global.h"

char fake_data[] = "";

char *xml_frag_data_nice(TTREE *x)
{
	char *s;
	
	s = xml_frag_data(x);
	if (!s) s = fake_data;
	
	return(s);
}


char *xml_frag_name_nice(TTREE *x)
{
	char *s;
	
	s = xml_frag_name(x);
	if (!s) s = fake_data;

	return(s);
}


/* Tested and works */

char *xml_fetch_clean_data(TTREE *x)
{
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
}


TTREE *xml_inner_span_element(TTREE *x)
{
	if (x->parent) x = x->parent;
	else return(0);

	if (x->parent) x = x->parent;
	else return(0);

	if (!strcmp("tag_span", x->data) && cong_dispspec_element_span(the_globals.ds, x->child->data))
		return(x);

	return(0);
}


TTREE *xml_outer_span_element(TTREE *x)
{
	TTREE *n0 = 0;
	
	if (x->parent) x = x->parent;
	else return(0);

	if (x->parent) x = x->parent;
	else return(0);

	for (;;)
	{
	  if (!strcmp("tag_span", x->data) && cong_dispspec_element_span(the_globals.ds, x->child->data))
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
