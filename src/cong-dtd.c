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


enum CongElementType
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

gboolean
cong_dtd_element_content_can_contain_pcdata (xmlElementContentPtr content)
{
	g_return_val_if_fail (content, FALSE);
  
	switch (content->type) {
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

