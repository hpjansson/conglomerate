/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-dtd.h
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

#ifndef __CONG_DTD_H__
#define __CONG_DTD_H__

G_BEGIN_DECLS

/* Callback for traversing DTD elements: */
typedef void (*CongDtdElementCallback) (xmlElementPtr dtd_element,
					gpointer user_data);

void
cong_dtd_for_each_element (xmlDtdPtr dtd,
			   CongDtdElementCallback callback,
			   gpointer user_data);

/* Callback for traversing DTD attributes: */
typedef void (*CongDtdAttributeCallback) (xmlElementPtr dtd_element,
					  xmlAttributePtr attr,
					  gpointer user_data);
					  

void 
cong_dtd_for_each_attribute (xmlElementPtr dtd_element,
			     CongDtdAttributeCallback callback,
			     gpointer user_data);

/**
 * cong_dtd_element_guess_dispspec_type
 * @element:
 * 
 * Given a DTD element, make a guess as to an appropriate xds element type.
 * Useful when autogenerating CongDispspec from DTD files
 * 
 * Returns: 
 */
enum CongElementType
cong_dtd_element_guess_dispspec_type (xmlElementPtr element);

/**
 * Can this element or its descendents ever contain PCDATA?
 */
gboolean
cong_dtd_element_content_can_contain_pcdata (xmlElementContentPtr content);

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
				       xmlNodePtr xml_node);

/**
 * cong_dtd_content_model_node_is_element:
 *
 * @content: a node in the content model tree for one of the elements in a DTD
 * @dtd_element: one of the elements in a DTD (usually a different one)
 *
 * Determine if this node in the content model is a direct cross-reference
 * to the given element.  Useful for determining recursion, nesting of elements, etc
 * 
 * Returns: TRUE if it is the element, FALSE if it's another element, PCDATA, etc
 *
 */
gboolean
cong_dtd_content_model_node_is_element (xmlElementContentPtr content,
					xmlElementPtr dtd_element);

/**
 * cong_dtd_get_element_for_content:
 *
 * Get the element in the DTD corresponding to a node in the content model (if any)
 *
 */
xmlElementPtr 
cong_dtd_get_element_for_content (xmlDtdPtr dtd,
				  xmlElementContentPtr content);

typedef void
(*CongDtdElementReferenceCallback) (xmlDtdPtr dtd,
				    xmlElementPtr dtd_element,
				    xmlElementContentPtr content,
				    gpointer user_data);

/**
 * cong_dtd_for_each_reference_to_element:
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
					gpointer user_data);

guint
cong_dtd_count_references_to_element (xmlDtdPtr dtd,
				      xmlElementPtr dtd_element);

/*
 * cong_dtd_guess_start_elements:
 *
 * Try to guess the most likely root elements of the DTD.
 * Currently implemented by finding those that aren't cross-referenced elsewhere in the DTD.
 *
 * Returns: a freshly allocated GList of xmlElementPtr
 */
GList*
cong_dtd_guess_start_elements (xmlDtdPtr dtd);

G_END_DECLS

#endif



