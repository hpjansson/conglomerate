/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"
#include <string.h>
#include <stdlib.h>
#include "cong-document.h"

void cong_location_set(CongLocation *loc, CongNodePtr tt, int offset)
{
	g_return_if_fail(loc != NULL);
	
	loc->node=tt;
	loc->byte_offset=offset;
	
}

void
cong_location_nullify(CongLocation *loc)
{
	g_return_if_fail(loc != NULL);
	
	loc->node=NULL;
}


gboolean cong_location_exists(const CongLocation *loc)
{
	g_return_val_if_fail(loc != NULL, FALSE);
	
	return (loc->node!=NULL);
}

gboolean
cong_location_equals(const CongLocation *loc0, const CongLocation *loc1)
{
	g_return_val_if_fail(loc0 != NULL, FALSE);
	g_return_val_if_fail(loc1 != NULL, FALSE);
	
	if (loc0->node == loc1->node) {
		if (loc0->byte_offset == loc1->byte_offset) {
			return TRUE;
		}		
	}

	return FALSE;	
}


enum CongNodeType
cong_location_node_type(const CongLocation *loc)
{
	g_return_val_if_fail(loc != NULL, 0);

	return cong_node_type(loc->node);
}

gunichar cong_location_get_unichar(const CongLocation *loc)
{
	g_return_val_if_fail(loc != NULL, '\0');
	g_return_val_if_fail(loc->node != NULL, '\0');
	g_return_val_if_fail(cong_location_node_type(loc) == CONG_NODE_TYPE_TEXT, '\0');
	
	return g_utf8_get_char(xml_frag_data_nice(loc->node) + loc->byte_offset);
}

gchar*
cong_location_get_utf8_pointer(const CongLocation *loc)
{
	g_return_val_if_fail(loc, NULL);

	g_assert(0); /* unimplemented */

	return NULL;
}


CongNodePtr
cong_location_xml_frag_data_nice_split2(CongDocument *doc, const CongLocation *loc)
{
	g_return_val_if_fail(loc != NULL, NULL);
	g_return_val_if_fail(cong_location_exists(loc), NULL);
	g_return_val_if_fail( (cong_location_node_type(loc) == CONG_NODE_TYPE_TEXT), NULL);
	
	/* GREP FOR MVC */

	return xml_frag_data_nice_split2(doc, loc->node, loc->byte_offset);
}

void
cong_location_insert_chars(CongDocument *doc, CongLocation *loc, const gchar* insertion)
{
	xmlChar *new_content;

	int byte_length;

	g_return_if_fail(cong_location_exists(loc));
	g_return_if_fail(cong_location_node_type(loc) == CONG_NODE_TYPE_TEXT);
	g_return_if_fail(insertion!=NULL);
	g_return_if_fail(g_utf8_validate(insertion, -1, NULL));

	byte_length = strlen(insertion);

	new_content = xmlStrndup(loc->node->content, loc->byte_offset);
	CONG_VALIDATE_UTF8(new_content);

	new_content = xmlStrcat(new_content, insertion);
	CONG_VALIDATE_UTF8(new_content);

	CONG_VALIDATE_UTF8(loc->node->content+loc->byte_offset);
	new_content = xmlStrcat(new_content, loc->node->content+loc->byte_offset);
	CONG_VALIDATE_UTF8(new_content);

	cong_document_node_set_text(doc, loc->node, new_content);
	
	xmlFree(new_content);

	/* GREP FOR MVC: the location is updated here */
	loc->byte_offset += byte_length;		
	CONG_VALIDATE_UTF8(loc->node->content+loc->byte_offset);
}

void
cong_location_del_next_char(CongDocument *doc, const CongLocation *loc)
{
	g_return_if_fail(cong_location_exists(loc));

	/* GREP FOR MVC */

	/* FIXME: what should we do about "empty" tags?  Better para support instead? */
	if (cong_location_get_unichar(loc))
	{
		gchar *new_text;
		gchar *next_char;
		gchar *char_after_next;

		new_text = xmlStrndup(xml_frag_data_nice(loc->node), loc->byte_offset);
		CONG_VALIDATE_UTF8(new_text);

		next_char = xml_frag_data_nice(loc->node) + loc->byte_offset;
		CONG_VALIDATE_UTF8(next_char);

		char_after_next = g_utf8_find_next_char(next_char, NULL);
		if (char_after_next) {
			CONG_VALIDATE_UTF8(char_after_next);
			new_text = xmlStrcat(new_text, char_after_next);
			CONG_VALIDATE_UTF8(new_text);
		}

		cong_document_node_set_text(doc, loc->node, new_text);

		xmlFree(new_text);
	}
}

CongNodePtr
cong_location_xml_frag_prev(const CongLocation *loc)
{
	return cong_node_prev(loc->node);
}

CongNodePtr
cong_location_xml_frag_next(const CongLocation *loc)
{
	return cong_node_next(loc->node);
}

CongNodePtr
cong_location_node(const CongLocation *loc)
{
	return loc->node;
}

CongNodePtr
cong_location_parent(const CongLocation *loc)
{
	g_return_val_if_fail(loc != NULL, NULL);
	g_return_val_if_fail(loc->node != NULL, NULL);

	return loc->node->parent;
}


void
cong_location_copy(CongLocation *dst, const CongLocation *src)
{
	g_return_if_fail(dst != NULL);
	g_return_if_fail(src != NULL);
	
	*dst = *src;
}
