/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"
#include <string.h>
#include <stdlib.h>

void cong_location_set(CongLocation *loc, CongNodePtr tt, int offset)
{
	g_return_if_fail(loc != NULL);
	
	loc->tt_loc=tt;
	loc->char_loc=offset;
	
}

void
cong_location_nullify(CongLocation *loc)
{
	g_return_if_fail(loc != NULL);
	
	loc->tt_loc=NULL;
}


gboolean cong_location_exists(CongLocation *loc)
{
	g_return_val_if_fail(loc != NULL, FALSE);
	
	return (loc->tt_loc!=NULL);
}

gboolean
cong_location_equals(const CongLocation *loc0, const CongLocation *loc1)
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


#if 0
#if NEW_XML_IMPLEMENTATION
int cong_location_frag_type(CongLocation *loc)
{
	g_return_val_if_fail(loc != NULL, 0);
	g_return_val_if_fail(loc->tt_loc != NULL, 0);
	
	return XML_DATA;
}
#else
int cong_location_frag_type(CongLocation *loc)
{
	g_return_val_if_fail(loc != NULL, 0);
	g_return_val_if_fail(loc->tt_loc != NULL, 0);
	
	return xml_frag_type(loc->tt_loc);
}
#endif
#endif

enum CongNodeType
cong_location_node_type(CongLocation *loc)
{
	g_return_val_if_fail(loc != NULL, 0);

	return cong_node_type(loc->tt_loc);
}

char cong_location_get_char(CongLocation *loc)
{
	g_return_val_if_fail(loc != NULL, '\0');
	g_return_val_if_fail(loc->tt_loc != NULL, '\0');
	g_return_val_if_fail(cong_location_node_type(loc) == CONG_NODE_TYPE_TEXT, '\0');
	
	return *(xml_frag_data_nice(loc->tt_loc) + loc->char_loc);
}

CongNodePtr
cong_location_xml_frag_data_nice_split2(CongDocument *doc, CongLocation *loc)
{
	g_return_val_if_fail(loc != NULL, NULL);
	g_return_val_if_fail(cong_location_exists(loc), NULL);
	g_return_val_if_fail( (cong_location_node_type(loc) == CONG_NODE_TYPE_TEXT), NULL);
	
	/* GREP FOR MVC */

	return xml_frag_data_nice_split2(doc, loc->tt_loc, loc->char_loc);
}

void
cong_location_insert_chars(CongDocument *doc, CongLocation *loc, const char* s)
{
#if NEW_XML_IMPLEMENTATION
	xmlChar *new_content;
#else
	TTREE *n;
#endif

	int len;

	g_return_if_fail(cong_location_exists(loc));
	g_return_if_fail(cong_location_node_type(loc) == CONG_NODE_TYPE_TEXT);
	g_return_if_fail(s!=NULL);

	len = strlen(s);

	/* GREP FOR MVC */

#if NEW_XML_IMPLEMENTATION
	new_content = xmlStrndup(loc->tt_loc->content, loc->char_loc);
	new_content = xmlStrcat(new_content, s); /* FIXME: xmlChar versus char */
	new_content = xmlStrcat(new_content, loc->tt_loc->content+loc->char_loc); /* FIXME: pointer arithmetic will fail with UTF8 etc */

	#if 1
	cong_document_node_set_text(doc, loc->tt_loc, new_content);
	#else
	xmlNodeSetContent(loc->tt_loc, new_content);
	#endif

	
	xmlFree(new_content);

	loc->char_loc += len;		
#else	

	n = loc->tt_loc->child;
	n->data = realloc(n->data, (n->size + 1) + len);
	
	memmove(n->data + loc->char_loc + len, n->data + loc->char_loc, (n->size + 1) - loc->char_loc);
	memcpy(n->data + loc->char_loc, s, len);
	n->size += len;
	loc->char_loc += len;
#endif
}

void
cong_location_del_next_char(CongDocument *doc, CongLocation *loc)
{
	g_return_if_fail(cong_location_exists(loc));

	/* GREP FOR MVC */

#if NEW_XML_IMPLEMENTATION
	/* FIXME: what should we do about "empty" tags?  Better para support instead? */
	if (cong_location_get_char(loc))
	{
		/* FIXME:  audit the char types and ptr arithmetic here: */
#if 1
		char *new_text;

		new_text = xmlStrndup(xml_frag_data_nice(loc->tt_loc), loc->char_loc);
		new_text = xmlStrcat(new_text, xml_frag_data_nice(loc->tt_loc) + loc->char_loc + 1); 

		cong_document_node_set_text(doc, loc->tt_loc, new_text);

		xmlFree(new_text);
#else
		memmove(xml_frag_data_nice(loc->tt_loc) + loc->char_loc, 
			xml_frag_data_nice(loc->tt_loc) + loc->char_loc + 1,
			strlen(xml_frag_data_nice(loc->tt_loc) + loc->char_loc));
#endif
	}
#else

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
#endif
}

CongNodePtr
cong_location_xml_frag_prev(CongLocation *loc)
{
	return cong_node_prev(loc->tt_loc);
}

CongNodePtr
cong_location_xml_frag_next(CongLocation *loc)
{
	return cong_node_next(loc->tt_loc);
}

CongNodePtr
cong_location_node(CongLocation *loc)
{
	return loc->tt_loc;
}

CongNodePtr
cong_location_parent(CongLocation *loc)
{
	g_return_val_if_fail(loc != NULL, NULL);
	g_return_val_if_fail(loc->tt_loc != NULL, NULL);

	return loc->tt_loc->parent;
}


void
cong_location_copy(CongLocation *dst, const CongLocation *src)
{
	g_return_if_fail(dst != NULL);
	g_return_if_fail(src != NULL);
	
	*dst = *src;
}
