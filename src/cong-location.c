/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"
#include <string.h>
#include <stdlib.h>
#include "xml.h"

void cong_location_set(cong_location *loc, TTREE *tt, int offset)
{
	g_return_if_fail(loc != NULL);
	
	loc->tt_loc=tt;
	loc->char_loc=offset;
	
}

void
cong_location_nullify(cong_location *loc)
{
	g_return_if_fail(loc != NULL);
	
	loc->tt_loc=NULL;
}


gboolean cong_location_exists(cong_location *loc)
{
	g_return_val_if_fail(loc != NULL, FALSE);
	
	return (loc->tt_loc!=NULL);
}

gboolean
cong_location_equals(const cong_location *loc0, const cong_location *loc1)
{
	g_return_val_if_fail(loc0 != NULL, FALSE);
	g_return_val_if_fail(loc1 != NULL, FALSE);
	
	if (loc0->tt_loc == loc1->tt_loc) {
		if (loc0->char_loc == loc1->char_loc) {
			return TRUE;
		}		
	}

	return FALSE;	
}


int cong_location_frag_type(cong_location *loc)
{
	g_return_val_if_fail(loc != NULL, 0);
	g_return_val_if_fail(loc->tt_loc != NULL, 0);
	
	return xml_frag_type(loc->tt_loc);
}

char cong_location_get_char(cong_location *loc)
{
	g_return_val_if_fail(loc != NULL, '\0');
	g_return_val_if_fail(loc->tt_loc != NULL, '\0');
	g_return_val_if_fail(cong_location_frag_type(loc) == XML_DATA, '\0');
	
	return *(xml_frag_data_nice(loc->tt_loc) + loc->char_loc);
}

TTREE*
cong_location_xml_frag_data_nice_split2(cong_location *loc)
{
	g_return_val_if_fail(loc != NULL, NULL);
	g_return_val_if_fail(cong_location_exists(loc), NULL);
	g_return_val_if_fail( (cong_location_frag_type(loc) == XML_DATA), NULL);
	
	return xml_frag_data_nice_split2(loc->tt_loc, loc->char_loc);
}

void
cong_location_insert_chars(cong_location *loc, const char* s)
{
	TTREE *n;
	int len;

	g_return_if_fail(cong_location_exists(loc));
	g_return_if_fail(cong_location_frag_type(loc) == XML_DATA);
	g_return_if_fail(s!=NULL);

	len = strlen(s);
	
	n = loc->tt_loc->child;
	n->data = realloc(n->data, (n->size + 1) + len);
	
	memmove(n->data + loc->char_loc + len, n->data + loc->char_loc, (n->size + 1) - loc->char_loc);
	memcpy(n->data + loc->char_loc, s, len);
	n->size += len;
	loc->char_loc += len;
}

void
cong_location_del_next_char(cong_location *loc)
{
	g_return_if_fail(cong_location_exists(loc));

	if (cong_location_get_char(loc))
	{
		memmove(xml_frag_data_nice(loc->tt_loc) + loc->char_loc, 
			xml_frag_data(loc->tt_loc) + loc->char_loc + 1,
			strlen(xml_frag_data_nice(loc->tt_loc) + loc->char_loc));
		loc->tt_loc->child->size--;
	}
	else if (loc->tt_loc->next && xml_frag_type(loc->tt_loc->next) == XML_TAG_EMPTY) {
		ttree_branch_remove(loc->tt_loc->next);	
	}
}

TTREE*
cong_location_xml_frag_prev(cong_location *loc)
{
	return xml_frag_prev(loc->tt_loc);
}

TTREE*
cong_location_xml_frag_next(cong_location *loc)
{
	return xml_frag_next(loc->tt_loc);
}

TTREE*
cong_location_node(cong_location *loc)
{
	return loc->tt_loc;
}

TTREE*
cong_location_parent(cong_location *loc)
{
	g_return_val_if_fail(loc != NULL, NULL);
	g_return_val_if_fail(loc->tt_loc != NULL, NULL);

	return loc->tt_loc->parent;
}


void
cong_location_copy(cong_location *dst, const cong_location *src)
{
	g_return_if_fail(dst != NULL);
	g_return_if_fail(src != NULL);
	
	*dst = *src;
}
