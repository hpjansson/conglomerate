/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"
#include <string.h>
#include <stdlib.h>
#include "cong-document.h"
#include "cong-error-dialog.h"

gboolean
cong_location_is_valid(const CongLocation *loc)
{
	enum CongNodeType type;
	
	g_return_val_if_fail(loc != NULL, FALSE);

	if (loc->node==NULL) {
		return (loc->byte_offset==0);
	} else {
		type = cong_location_node_type(loc);

		switch (type) {
		default: g_assert_not_reached();

		case CONG_NODE_TYPE_UNKNOWN:
		case CONG_NODE_TYPE_ELEMENT:
			return (loc->byte_offset==0);
	
		case CONG_NODE_TYPE_TEXT:
		case CONG_NODE_TYPE_COMMENT:
			{	   
				/* Test that the byte offset is at the start of a character, in range, etc... */
				if (loc->byte_offset<0) {
					return FALSE;
				}

				/* FIXME: add more tests here */
				
				return TRUE;
			}			
		}
	}
}

void
cong_location_nullify(CongLocation *loc)
{
	g_return_if_fail(loc != NULL);
	
	loc->node=NULL;
	loc->byte_offset=0; /* for good measure */
}

void
cong_location_set_to_start_of_node(CongLocation *loc, CongNodePtr node)
{
	g_return_if_fail(loc != NULL);
	g_return_if_fail(node != NULL);

	loc->node=node;
	loc->byte_offset=0;
}

void 
cong_location_set_node_and_byte_offset(CongLocation *loc, CongNodePtr node, int offset)
{
	g_return_if_fail(loc != NULL);
	g_assert(node);
	g_return_if_fail(node != NULL);

	/* FIXME: 
	   We can do some self-testing here.
	   Test that the node is of a sane type for there to be an offset i.e. text or a comment.
	   Test that the byte offset is at the start of a character, in range, etc...
	 */
	
	loc->node=node;
	loc->byte_offset=offset;

	g_assert(cong_location_is_valid(loc));
}

void
cong_location_set_node_and_char_offset(CongLocation *loc, CongNodePtr node, glong char_offset)
{
	gchar *result_pos;

	g_return_if_fail(loc);
	g_return_if_fail(node);

	/* FIXME: 
	   We can do some self-testing here.
	   Test that the node is of a sane type for there to be an offset i.e. text or a comment.
	   Test that the char offset is in range, etc...
	 */

	result_pos = g_utf8_offset_to_pointer(node->content, char_offset);
	g_assert(result_pos);
	
	loc->node = node;
	loc->byte_offset = result_pos - (gchar*)node->content;

	g_assert(cong_location_is_valid(loc));
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

	g_assert(cong_location_is_valid(loc0));
	g_assert(cong_location_is_valid(loc1));
	
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
	g_return_val_if_fail(loc, CONG_NODE_TYPE_UNKNOWN);
	g_return_val_if_fail(loc->node, CONG_NODE_TYPE_UNKNOWN);

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

	return cong_document_node_split2 (doc, 
					  loc->node, 
					  loc->byte_offset);
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

	cong_document_begin_edit (doc);
	cong_document_node_set_text(doc, loc->node, new_content);
	cong_document_end_edit (doc);
	
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

		cong_document_begin_edit (doc);
		cong_document_node_set_text(doc, loc->node, new_text);
		cong_document_end_edit (doc);

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

gboolean cong_location_calc_prev_char(const CongLocation *input_loc, 
				      CongDispspec *dispspec,
				      CongLocation *output_loc)
{
	CongNodePtr n;
	CongNodePtr n0;

#ifndef RELEASE	
	printf("<- [curs]\n");
#endif

	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);
	
	n = input_loc->node;
	if (cong_location_node_type(input_loc) == CONG_NODE_TYPE_TEXT && input_loc->byte_offset) { 

		gchar *this_char;
		gchar *prev_char;

		g_assert(input_loc->node);
		g_assert(input_loc->node->content);
		g_assert(g_utf8_validate(input_loc->node->content,-1,NULL));

		/* FIXME: Should we be looking at a PangoLogAttrs "is_cursor_position" instead? */

		this_char = input_loc->node->content+input_loc->byte_offset;
		prev_char = g_utf8_find_prev_char(input_loc->node->content, this_char);
		g_assert(prev_char);

		cong_location_set_node_and_byte_offset(output_loc,input_loc->node, prev_char - (gchar*)input_loc->node->content);

		return TRUE;
	}

	do
	{
		n0 = n;
		if (n) n = cong_node_prev(n);
		
		for ( ; n; )
		{
			if (cong_node_type(n) == CONG_NODE_TYPE_TEXT) break;
			else if (cong_node_type(n) == CONG_NODE_TYPE_ELEMENT)
			{
				if (!strcmp(cong_node_name(n), "table")) break;
				if (cong_dispspec_element_structural(dispspec, xml_frag_name_nice(n)))
				{
					n = n0 = 0;
					break;
				}
				else if (cong_node_first_child(n))
				{
#ifndef RELEASE					
					printf("Entering tag: %s.\n", xml_frag_name_nice(n));
#endif					
					n = cong_node_first_child(n);
					continue;
				}
			}
			
			n0 = n;
			n = cong_node_prev(n);
		}

		if (!n) n = n0;
		else if (cong_node_type(n) == CONG_NODE_TYPE_TEXT) break;

		while (n)
		{
			if (cong_dispspec_element_structural(dispspec, xml_frag_name_nice(n))) { n = 0; break; }
			if (!cong_node_prev(n)) n = n0 = cong_node_parent(n);
			else break;
		}
	}
	while (n);

	if (n) {
		/* FIXME: UTF-8 issues here! */
		cong_location_set_node_and_byte_offset(output_loc,n, strlen(xml_frag_data_nice(n)));
		return TRUE;
	} else {
		return FALSE;
	}
}


gboolean cong_location_calc_next_char(const CongLocation *input_loc,
				      CongDispspec *dispspec,
				      CongLocation *output_loc)
{
	CongNodePtr n;
	CongNodePtr n0;

#ifndef RELEASE	
	printf("[curs] ->\n");
#endif

	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);
	

	n = input_loc->node;
	if (cong_location_node_type(input_loc) == CONG_NODE_TYPE_TEXT && cong_location_get_unichar(input_loc))
	{ 
		gchar *this_char;
		gchar *next_char;

		g_assert(input_loc->node);
		g_assert(input_loc->node->content);
		g_assert(g_utf8_validate(input_loc->node->content,-1,NULL));

		/* FIXME: Should we be looking at a PangoLogAttrs "is_cursor_position" instead? */

		this_char = input_loc->node->content+input_loc->byte_offset;
		next_char = g_utf8_find_next_char(this_char, NULL);
		g_assert(next_char);

		cong_location_set_node_and_byte_offset(output_loc,input_loc->node, next_char - (gchar*)input_loc->node->content);

		return TRUE; 
	}

	do
	{
		n0 = n;
		if (n) n = cong_node_next(n);

		for ( ; n; )
		{
			if (cong_node_type(n) == CONG_NODE_TYPE_TEXT) break;
			else if (cong_node_type(n) == CONG_NODE_TYPE_ELEMENT)
			{				 
				if (!strcmp(cong_node_name(n), "table")) break;
				if (cong_dispspec_element_structural(dispspec, xml_frag_name_nice(n)))
				{
					n = n0 = 0;
					break;
				}
				else if (cong_node_first_child(n))
				{
#ifndef RELEASE					
					printf("Entering tag: %s.\n", xml_frag_name_nice(n));
#endif
					n = cong_node_first_child(n);
					continue;
				}
			}
			
			n0 = n;
			n = cong_node_next(n);
		}

		if (!n) n = n0;
		else if (cong_node_type(n) == CONG_NODE_TYPE_TEXT) break;

		while (n)
		{
			if (cong_dispspec_element_structural(dispspec, xml_frag_name_nice(n))) { n = 0; break; }
			if (!cong_node_next(n)) n = n0 = cong_node_parent(n);
			else break;
		}
	}
	while (n);

	if (n) {
		cong_location_set_to_start_of_node(output_loc, n);
		return TRUE;
	} else {
		return FALSE;
	}
}

static void make_pango_log_attr_for_node(CongDocument *doc,
					 CongNodePtr node,
					 PangoLogAttr **pango_log_attrs,
					 int *attrs_len)
{
	PangoLanguage *language;

	g_return_if_fail(doc);
	g_return_if_fail(node);
	g_return_if_fail(node->content);
	g_return_if_fail(pango_log_attrs);
	g_return_if_fail(attrs_len);

	language = cong_document_get_language_for_node(doc, node);

	*attrs_len = g_utf8_strlen(node->content,-1)+1;

	*pango_log_attrs = g_new(PangoLogAttr, (*attrs_len));
	
	pango_get_log_attrs(node->content,
			    strlen(node->content), /* length in bytes */
			    -1,
			    language,
			    *pango_log_attrs,
			    *attrs_len);

}

gboolean cong_location_calc_prev_word(const CongLocation *input_loc, 
				      CongDocument *doc,
				      CongLocation *output_loc)
{
	PangoLogAttr *pango_log_attr=NULL;
	int attrs_len=0;

	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(doc, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	make_pango_log_attr_for_node(doc,
				     input_loc->node,
				     &pango_log_attr,
				     &attrs_len);

	if (pango_log_attr) {
		glong char_index = g_utf8_pointer_to_offset(input_loc->node->content, input_loc->node->content+input_loc->byte_offset);

		g_assert(attrs_len>0);
		g_assert(char_index<attrs_len);

		/* Scan backwards to next is_word_start: */
		while (char_index>0) {

			if (pango_log_attr[--char_index].is_word_start) {
				cong_location_set_node_and_char_offset(output_loc, input_loc->node, char_index);
				g_free(pango_log_attr);
				return TRUE;
			}
		}

		cong_location_set_to_start_of_node(output_loc,input_loc->node);
		g_free(pango_log_attr);
		return TRUE;

	} else {
		return FALSE;
	}
}

gboolean cong_location_calc_next_word(const CongLocation *input_loc, 
				      CongDocument *doc,
				      CongLocation *output_loc)
{
	PangoLogAttr *pango_log_attr=NULL;
	int attrs_len=0;

	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(doc, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	make_pango_log_attr_for_node(doc, 
				     input_loc->node,
				     &pango_log_attr,
				     &attrs_len);

	if (pango_log_attr) {
		glong char_index = g_utf8_pointer_to_offset(input_loc->node->content, input_loc->node->content+input_loc->byte_offset);

		g_assert(attrs_len>0);
		g_assert(char_index<attrs_len);

		/* Scan forwards to next is_word_start: */
		while (char_index<attrs_len) {

			if (pango_log_attr[++char_index].is_word_start) {
				cong_location_set_node_and_char_offset(output_loc, input_loc->node, char_index);
				g_free(pango_log_attr);
				return TRUE;
			}
		}

		/* FIXME: is this logic correct??? */
		cong_location_set_node_and_char_offset(output_loc, input_loc->node, char_index-1);
		g_free(pango_log_attr);
		return TRUE;

	} else {
		return FALSE;
	}
}

gboolean cong_location_calc_document_start(const CongLocation *input_loc, 
					   CongDispspec *dispspec,
					   CongLocation *output_loc)
{
	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	CONG_DO_UNIMPLEMENTED_DIALOG(NULL, _("Calculating document start"));

	return FALSE;
}
gboolean cong_location_calc_line_start(const CongLocation *input_loc, 
				      CongDispspec *dispspec,
				      CongLocation *output_loc)
{
	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	CONG_DO_UNIMPLEMENTED_DIALOG(NULL, _("Calculating line start"));

	return FALSE;
}

gboolean cong_location_calc_document_end(const CongLocation *input_loc, 
					 CongDispspec *dispspec,
					 CongLocation *output_loc)
{
	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	CONG_DO_UNIMPLEMENTED_DIALOG(NULL, _("Calculating document end"));

	return FALSE;
}

gboolean cong_location_calc_line_end(const CongLocation *input_loc, 
				     CongDispspec *dispspec,
				     CongLocation *output_loc)
{
	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	CONG_DO_UNIMPLEMENTED_DIALOG(NULL, _("Calculating line end"));

	return FALSE;
}

gboolean cong_location_calc_prev_page(const CongLocation *input_loc, 
				      CongDispspec *dispspec,
				      CongLocation *output_loc)
{
	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	CONG_DO_UNIMPLEMENTED_DIALOG(NULL, _("Calculating previous page"));

	return FALSE;
}

gboolean cong_location_calc_next_page(const CongLocation *input_loc, 
				      CongDispspec *dispspec,
				      CongLocation *output_loc)
{
	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	CONG_DO_UNIMPLEMENTED_DIALOG(NULL, _("Calculating next page"));

	return FALSE;
}

gboolean
cong_location_calc_word_extent(const CongLocation *input_loc, 
			       CongDocument *doc,
			       CongLocation *output_start_of_word, 
			       CongLocation *output_end_of_word)
{
	PangoLogAttr *pango_log_attr=NULL;
	int attrs_len=0;

	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(doc, FALSE);
	g_return_val_if_fail(output_start_of_word, FALSE);
	g_return_val_if_fail(output_end_of_word, FALSE);

	make_pango_log_attr_for_node(doc,
				     input_loc->node,
				     &pango_log_attr,
				     &attrs_len);

	if (pango_log_attr) {
		glong char_index = g_utf8_pointer_to_offset(input_loc->node->content, input_loc->node->content+input_loc->byte_offset);

		g_assert(attrs_len>0);
		g_assert(char_index<attrs_len);

		/* Scan backwards to next is_word_start: */
		{
			glong start_char_index = char_index;
			while (start_char_index>0) {
				
				if (pango_log_attr[--start_char_index].is_word_start) {
					break;
				}
			}
			cong_location_set_node_and_char_offset(output_start_of_word, input_loc->node, start_char_index);
		}

		
		/* Scan forwards to next is_word_end: */
		{
			glong end_char_index = char_index;
			while (end_char_index<attrs_len) {
				
				if (pango_log_attr[++end_char_index].is_word_end) {
					break;
				}
			}
			cong_location_set_node_and_char_offset(output_end_of_word, input_loc->node, end_char_index);
		}

		g_free(pango_log_attr);
		return TRUE;

	} else {
		return FALSE;
	}
}

#if 0
gboolean
cong_location_calc_prev_text_node (const CongLocation *input_loc, 
				   CongDispspec *dispspec,
				   CongLocation *output_loc)
{
#error
}

gboolean
cong_location_calc_next_text_node (const CongLocation *input_loc,
				   CongDispspec *dispspec,
				   CongLocation *output_loc)
{
#error
}
#endif

