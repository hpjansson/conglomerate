/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"
#include <string.h>
#include <stdlib.h>
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-error-dialog.h"
#include "cong-util.h"
#include "cong-command.h"

 
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

static gboolean
has_same_namespace (CongNodePtr n1,
		    CongNodePtr n2)
{
	g_return_val_if_fail (n1, FALSE);
	g_return_val_if_fail (n2, FALSE);

	/* FIXME: */
	return TRUE;
}

/*
  Are both ELEMENT nodes of the same tag type?
*/
gboolean
cong_node_is_same_tag (CongNodePtr n1, 
		       CongNodePtr n2)
{
	g_return_val_if_fail (n1, FALSE);
	g_return_val_if_fail (n2, FALSE);

	if (CONG_NODE_TYPE_ELEMENT == cong_node_type(n1)) {
		if (CONG_NODE_TYPE_ELEMENT == cong_node_type(n2)) {
			if (has_same_namespace(n1,n2)) {
				if (0==strcmp(n1->name, n2->name)) {
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

gboolean
cong_node_is_pure_whitespace_text_node (CongNodePtr node)
{
	g_return_val_if_fail (node, FALSE);

	if (CONG_NODE_TYPE_TEXT == cong_node_type(node)) {
		if (cong_util_is_pure_whitespace (node->content)) {
			return TRUE;
		}		
	}

	return FALSE;
}

static void
merge_tags (CongCommand *cmd,
	    CongNodePtr predator,
	    CongNodePtr victim)
{
	CongDocument *doc;
	CongNodePtr iter, next;

	g_return_if_fail (IS_CONG_COMMAND(cmd));
	g_return_if_fail (predator);
	g_return_if_fail (victim);

	doc = cong_command_get_document (cmd);

	g_message("merging <%s> tags", predator->name);

	cong_document_begin_edit (doc);
	
	/* Move all children of victim into predator: */
	for (iter=victim->children; iter; iter = next) {
		next = iter->next;
		
		cong_command_add_node_set_parent (cmd,
						  iter,
						  predator);
	}
	
	/* Merge any text nodes as necessary: */
	cong_command_add_merge_adjacent_text_children_of_node (cmd, 
							       predator);

	/* Victim is now empty; remove it and delete it: */
	cong_command_add_node_recursive_delete (cmd, 
						victim);

	cong_document_end_edit (doc);
}

static void
handle_tag_merging (CongDocument *doc,
		    CongNodePtr node)
{			
	g_assert (CONG_NODE_TYPE_ELEMENT == cong_node_type(node));

	if (node->next) {
				
		/* Handle the case of a single intermediate text node of pure whitespace (added by formatting routine) separating two identical tags: */
		if (CONG_NODE_TYPE_TEXT == cong_node_type (node->next)) {
			if (node->next->next) {
				if (cong_node_is_same_tag (node, 
							   node->next->next)) {
					if (cong_node_is_pure_whitespace_text_node (node->next)) {
						CongCommand *cmd = cong_document_begin_command (doc, _("Merge tags"), NULL);

						/* Do the merge: */
						merge_tags (cmd,
							    node,
							    node->next->next);

						g_message ("deleting intermediate whitespace");

						/* Delete the intermediate whitespace (NB this pointer should still be valid): */
						cong_command_add_node_recursive_delete (cmd, 
											node->next);
						
						cong_document_end_command (doc, cmd);
					}
				}				
			}
		} else {
			/* Handle the case of two adjacent, identical tags: */
			if (cong_node_is_same_tag (node, 
						   node->next)) {
				CongCommand *cmd = cong_document_begin_command (doc, _("Merge tags"), NULL);
				
				/* Do the merge: */
				merge_tags (cmd,
					    node,
					    node->next);

				cong_document_end_command (doc, cmd);
			}			
		}
	}

}

void
cong_location_del_next_char (CongDocument *doc, 
			     const CongLocation *loc)
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

		{
			CongCommand *cmd = cong_document_begin_command (doc, 
									_("Delete character"), 
									"cong-delete-character");

			cong_command_add_node_set_text(cmd, loc->node, new_text);
			
			cong_document_end_command (doc, cmd);
		}

		xmlFree(new_text);
	} else {
		/* 
		   We're at the end of a text node, trying to delete...
		*/
		g_assert (CONG_NODE_TYPE_TEXT==cong_node_type(loc->node));
		
		if (loc->node->next) {
			/* A tag of some kind is about to begin: */
			/* FIXME: what to do? */
		} else {
			/* 
			   We're at the end of tag, trying to delete...
			   Interpret this as an attempt to merge the tag containing this text with its next sibling.
			   See http://bugzilla.gnome.org/show_bug.cgi?id=121970
			*/

			handle_tag_merging (doc,
					    loc->node->parent);
		}
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

typedef gboolean (*CongNodePredicate) (CongDispspec *dispspec,
				       CongNodePtr node);

/* Search for prev node, assuming a depth-first traversal: */
static CongNodePtr
calc_final_node_in_subtree_satisfying (CongDispspec *dispspec,
				       CongNodePtr node, 
				       CongNodePredicate predicate)
{
	CongNodePtr iter;

	/* "node" is treated as being in its own subtree */

	for (iter = node->last; iter; iter=iter->prev) {
		CongNodePtr final = calc_final_node_in_subtree_satisfying (dispspec,
									   iter,
									   predicate);
		
		if (final) {
			return final;
		}		
	}

	/* Not found in any children of this node, try this node: */
	if (predicate (dispspec, node)) {
		return node;
	} else {
		return NULL;
	}
}

static CongNodePtr
calc_prev_node_satisfying (CongDispspec *dispspec,
			   CongNodePtr node, 
			   CongNodePredicate predicate)
{
	g_return_val_if_fail (dispspec, NULL);
	g_return_val_if_fail (node, NULL);
	g_return_val_if_fail (predicate, NULL);

	/* Search through subtrees of siblings to the left of this node: */
	{
		CongNodePtr iter;

		for (iter = node->prev; iter; iter = iter->prev) {
			CongNodePtr final = calc_final_node_in_subtree_satisfying (dispspec,
										   iter, 
										   predicate);
			
			if (final) {
				return final;
			}
		}
	}

	/* If not found, try parent node, and then recurse: */
	if (node->parent) {
		if (predicate(dispspec, node->parent)) {
			return node->parent;
		} else {
			return calc_prev_node_satisfying (dispspec,
							  node->parent, 
							  predicate);				
		} 
	} else {
		return NULL;
	}
}

static gboolean
is_valid_cursor_node (CongDispspec *dispspec,
		      CongNodePtr node) 
{
	g_return_val_if_fail (node, FALSE);

	return cong_node_is_valid_cursor_location (node);
}

gboolean cong_location_calc_prev_char(const CongLocation *input_loc, 
				      CongDispspec *dispspec,
				      CongLocation *output_loc)
{
	CongNodePtr n;

#ifndef RELEASE	
	printf("<- [curs]\n");
#endif

	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);
	
	n = input_loc->node;
	if (is_valid_cursor_node(dispspec, input_loc->node) && input_loc->byte_offset) { 

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
	n = calc_prev_node_satisfying (dispspec, n, is_valid_cursor_node);

	if (n) {
		g_assert (is_valid_cursor_node (dispspec, n));

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
				if (cong_dispspec_element_structural(dispspec, cong_node_get_xmlns(n), xml_frag_name_nice(n)))
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
			if (cong_dispspec_element_structural(dispspec, cong_node_get_xmlns(n), xml_frag_name_nice(n))) { n = 0; break; }
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
		
		if (char_index==attrs_len) {
			if (attrs_len>0) {
				char_index=attrs_len-1;
			} else {
				g_free(pango_log_attr);
				return FALSE;	
			}
		}

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

		if (char_index==attrs_len) {
			return FALSE;
		}

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

#if 0
	g_message ("cong_location_calc_word_extent at offset %i for content \"%s\"",
		   input_loc->byte_offset,
		   input_loc->node->content);
#endif

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

#if 0
		g_message ("succeeeded with offsets (%i->%i)",
			   output_start_of_word->byte_offset,
			   output_end_of_word->byte_offset);
#endif

		return TRUE;

	} else {
#if 0
		g_message ("failed");
#endif
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

void
cong_location_copy_with_ref (CongDocument *doc, 
			     CongLocation *dst,
			     const CongLocation *src)
{
	CongNodePtr old_node;

	old_node = dst->node;

	cong_location_copy (dst, src);

	if (dst->node) {
		cong_document_node_ref (doc, dst->node);
	}

	if (old_node) {
		cong_document_node_unref (doc, old_node);
	}
}

void
cong_location_nullify_with_ref (CongDocument *doc, 
				CongLocation *loc)
{
	CongNodePtr old_node;

	if (loc->node) {
		cong_document_node_unref (doc, loc->node);
	}

	cong_location_nullify (loc);
}

