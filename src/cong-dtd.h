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

#include "cong-dispspec-element.h"

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

CongElementType
cong_dtd_element_guess_dispspec_type (xmlElementPtr element);

/**
 * Can this element or its descendents ever contain PCDATA?
 */
gboolean
cong_dtd_element_content_can_contain_pcdata (xmlElementContentPtr content);

xmlElementPtr
cong_dtd_element_get_element_for_node (xmlDtdPtr dtd,
				       xmlNodePtr xml_node);

gboolean
cong_dtd_content_model_node_is_element (xmlElementContentPtr content,
					xmlElementPtr dtd_element);

 
xmlElementPtr 
cong_dtd_get_element_for_content (xmlDtdPtr dtd,
				  xmlElementContentPtr content);

typedef void
(*CongDtdElementReferenceCallback) (xmlDtdPtr dtd,
				    xmlElementPtr dtd_element,
				    xmlElementContentPtr content,
				    gpointer user_data);

void
cong_dtd_for_each_reference_to_element (xmlDtdPtr dtd,
					xmlElementPtr dtd_element,
					CongDtdElementReferenceCallback callback,
					gpointer user_data);

guint
cong_dtd_count_references_to_element (xmlDtdPtr dtd,
				      xmlElementPtr dtd_element);

GList*
cong_dtd_guess_start_elements (xmlDtdPtr dtd);

G_END_DECLS

#endif



