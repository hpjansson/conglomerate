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

