/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-dtd.c
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
#include "cong-dtd.h"
#include <libxml/hash.h>

/* Internal types: */
struct dtd_callback_marshall
{
	CongDtdElementCallback callback;
	gpointer user_data;
};

/* Internal function declarations: */
static void
element_callback_marshall (void *payload, 
			   void *data,
			   xmlChar * name);

/* Exported function definitions: */
/**
 * cong_dtd_for_each_element:
 * @dtd:
 * @callback:
 * @user_data:
 *
 * TODO: Write me
 */
void
cong_dtd_for_each_element (xmlDtdPtr dtd,
			   CongDtdElementCallback callback,
			   gpointer user_data)
{
	struct dtd_callback_marshall marshall;

	g_return_if_fail (dtd);
	g_return_if_fail (callback);

	marshall.callback = callback;
	marshall.user_data = user_data;

	xmlHashScan (dtd->elements, 
		     element_callback_marshall, 
		     &marshall);
}

/**
 * cong_dtd_for_each_attribute:
 * @dtd_element:
 * @callback:
 * @user_data:
 *
 * TODO: Write me
 */
void 
cong_dtd_for_each_attribute (xmlElementPtr dtd_element,
			     CongDtdAttributeCallback callback,
			     gpointer user_data)
{
	xmlAttributePtr attr;

	g_return_if_fail (dtd_element);
	g_return_if_fail (callback);

	for (attr=dtd_element->attributes; attr; attr=attr->nexth) {
		(*callback) (dtd_element,
			     attr,
			     user_data);
	}
}

/**
 * cong_dtd_element_guess_dispspec_type
 * @element:
 * 
 * Given a DTD element, make a guess as to an appropriate xds element type.
 * Useful when autogenerating CongDispspec from DTD files
 * 
 * Returns: 
 */
CongElementType
cong_dtd_element_guess_dispspec_type (xmlElementPtr element)
{
	g_return_val_if_fail (element, CONG_ELEMENT_TYPE_UNKNOWN);

	if (!element->content || cong_dtd_element_content_can_contain_pcdata (element->content))
	{
		return CONG_ELEMENT_TYPE_SPAN;
	}
	else
	{
		return CONG_ELEMENT_TYPE_STRUCTURAL;
	}
}

/**
 * cong_dtd_element_content_can_contain_pcdata:
 * @content:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_dtd_element_content_can_contain_pcdata (xmlElementContentPtr content)
{
	g_return_val_if_fail (content, FALSE);
  
	switch (content->type) {
	default: break;
	case XML_ELEMENT_CONTENT_PCDATA:
		return TRUE;
	}

	/* Recurse over descendents: */
	if (content->c1) {
		if (cong_dtd_element_content_can_contain_pcdata (content->c1)) {
			return TRUE;
		}
	}

	if (content->c2) {
		if (cong_dtd_element_content_can_contain_pcdata (content->c2)) {
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * cong_dtd_get_element_for_node
 * @dtd: the DTD
 * @node: an xml node
 * 
 * Given a DTD and an XML node, try to find the #xmlElementPtr for that node in the DTD
 * 
 * Returns: the #xmlElementPtr if found, or NULL otherwise
 */
xmlElementPtr
cong_dtd_element_get_element_for_node (xmlDtdPtr dtd,
				       xmlNodePtr xml_node)
{
	g_return_val_if_fail (dtd, NULL);
	g_return_val_if_fail (xml_node, NULL);
	g_return_val_if_fail (XML_ELEMENT_NODE==xml_node->type, NULL);
	g_return_val_if_fail (xml_node->name, NULL);

	/* FIXME: doesn't handle namespaces yet: */
	return xmlGetDtdElementDesc (dtd,
				     xml_node->name);
}

/**
 * cong_dtd_content_model_node_is_element:
 * @content: a node in the content model tree for one of the elements in a DTD
 * @dtd_element: one of the elements in a DTD (usually a different one)
 *
 * Determine if this node in the content model is a direct cross-reference
 * to the given element.  Useful for determining recursion, nesting of elements, etc
 * 
 * Returns: TRUE if it is the element, FALSE if it's another element, PCDATA, etc
 */
gboolean
cong_dtd_content_model_node_is_element (xmlElementContentPtr content,
					xmlElementPtr dtd_element)
{
	g_return_val_if_fail (content, FALSE);
	g_return_val_if_fail (dtd_element, FALSE);

	if (content->type==XML_ELEMENT_CONTENT_ELEMENT) {
		if (0==strcmp (content->name, dtd_element->name)) {
			if (content->prefix) {
				if (dtd_element->prefix) {
					if (0==strcmp (content->prefix, dtd_element->prefix)) {
						return TRUE;
					}
				}
			} else {
				if (NULL==dtd_element->prefix) {
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

struct find_content_data
{
	xmlElementContentPtr content;
	xmlElementPtr result;
	
};

static void 
find_content_callback (xmlElementPtr dtd_element,
		       gpointer user_data)
{
	struct find_content_data *cb_data = (struct find_content_data *)user_data;

	if (cong_dtd_content_model_node_is_element (cb_data->content,
						    dtd_element)) {
		cb_data->result = dtd_element;
		/* FIXME: ought to terminate search here! */
	}
}


/**
 * cong_dtd_get_element_for_content:
 * @dtd:
 * @content:
 *
 * Get the element in the DTD corresponding to a node in the content model (if any)
 *
 * Returns:
 */
xmlElementPtr 
cong_dtd_get_element_for_content (xmlDtdPtr dtd,
				  xmlElementContentPtr content)
{
	g_return_val_if_fail (dtd, NULL);
	g_return_val_if_fail (content, NULL);

	if (content->type==XML_ELEMENT_CONTENT_ELEMENT) {
		struct find_content_data cb_data;
		cb_data.content = content;
		cb_data.result = NULL;
		cong_dtd_for_each_element (dtd,
					   find_content_callback,
					   &cb_data);
		return cb_data.result;
	}

	return NULL;
}


struct cross_reference_data
{
	xmlDtdPtr dtd;
	xmlElementPtr dtd_element;
	CongDtdElementReferenceCallback callback;
	gpointer user_data;
};

static void
visit_element_references_for_content_subtree (xmlElementContentPtr content,
					      struct cross_reference_data *cb_data)
{
	g_assert (content);
	g_assert (cb_data);

	/* content->ocur is irrelvant */
	switch (content->type) {
	default: g_assert_not_reached ();
	case XML_ELEMENT_CONTENT_PCDATA:
		break;
	case XML_ELEMENT_CONTENT_ELEMENT:
		if (cong_dtd_content_model_node_is_element (content, cb_data->dtd_element)) {
			/* We've found a reference to the search element: */
			cb_data->callback (cb_data->dtd,
					   cb_data->dtd_element,
					   content,
					   cb_data->user_data);
		}
		break;
	case XML_ELEMENT_CONTENT_SEQ:
	case XML_ELEMENT_CONTENT_OR:
		/* recurse down subtrees: */
		visit_element_references_for_content_subtree (content->c1,
							      cb_data);
		visit_element_references_for_content_subtree (content->c2,
							      cb_data);
		break;
	}
}



static void 
element_callback_visit_cross_references (xmlElementPtr dtd_element,
					 gpointer user_data)
{
	struct cross_reference_data *cb_data = (struct cross_reference_data *)user_data;

	g_assert (dtd_element);
	g_assert (user_data);

	g_assert (cb_data->dtd_element);

	if (dtd_element->content) {
		visit_element_references_for_content_subtree (dtd_element->content,
							      cb_data);
	}
}

/**
 * cong_dtd_for_each_reference_to_element:
 * @dtd:
 * @dtd_element:
 * @callback:
 * @user_data:
 *
 * Function to search through the content models in the DTD, calling the callback for any reference to the given element.
 *
 * Handles the case where the element is part of a recursive content model (which would allow arbitrary
 * numbers of that element to be added) by treating each as a single cross-reference.
 *
 */
void
cong_dtd_for_each_reference_to_element (xmlDtdPtr dtd,
					xmlElementPtr dtd_element,
					CongDtdElementReferenceCallback callback,
					gpointer user_data)
{
	struct cross_reference_data cb_data;

	g_return_if_fail (dtd);
	g_return_if_fail (dtd_element);
	g_return_if_fail (callback);

	cb_data.dtd = dtd;
	cb_data.dtd_element = dtd_element;
	cb_data.callback = callback;
	cb_data.user_data = user_data;

	cong_dtd_for_each_element (dtd,
				   element_callback_visit_cross_references,
				   &cb_data);
}

static void
element_reference_callback_count (xmlDtdPtr dtd,
				  xmlElementPtr dtd_element,
				  xmlElementContentPtr content,
				  gpointer user_data)
{
	guint *count = (guint *)user_data;

	(*count)++;
}

guint
cong_dtd_count_references_to_element (xmlDtdPtr dtd,
				      xmlElementPtr dtd_element)
{
	guint result = 0;

	cong_dtd_for_each_reference_to_element (dtd_element->parent,
						dtd_element,
						element_reference_callback_count,
						&result);

	return result;
}			  

static void 
element_callback_add_candidates (xmlElementPtr dtd_element,
				 gpointer user_data)
{
	if (0==cong_dtd_count_references_to_element (dtd_element->parent,
						     dtd_element)) {
		/* If no references, then perhaps this is the element: */
		GList** list_ptr = (GList**)user_data;

		*list_ptr = g_list_append (*list_ptr, dtd_element);
	}
}

/*
 * cong_dtd_guess_start_elements:
 *
 * Try to guess the most likely root elements of the DTD.
 * Currently implemented by finding those that aren't cross-referenced elsewhere in the DTD.
 *
 * Returns: a freshly allocated GList of xmlElementPtr
 */
GList*
cong_dtd_guess_start_elements (xmlDtdPtr dtd)
{
	GList *list = NULL;

	g_return_val_if_fail (dtd, NULL);

	cong_dtd_for_each_element (dtd,
				   element_callback_add_candidates,
				   &list);
	return list;
}


/* Internal function definitions: */
static void
element_callback_marshall (void *payload, 
			   void *data,
			   xmlChar * name)
{
	struct dtd_callback_marshall* marshall = (struct dtd_callback_marshall*)data;

	g_assert (marshall);
	g_assert (marshall->callback);

	(marshall->callback) ((xmlElementPtr)payload,
			      marshall->user_data);
}

