/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-font.c
 *
 * Copyright (C) 2002 David Malcolm
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
 * Based on code by Hans Petter Jansson <hpj@ximian.com>
 */

#include "global.h"

const char font_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                          "abcdefghijklmnopqrstuvwxyz"
                          "0123456789!@\"'/()[]{}*&#~";

CongFont*
cong_font_load(const gchar *font_name)
{
	CongFont *font;
	g_return_val_if_fail(font_name, NULL);

	font = g_new0(CongFont,1);

	font->font_desc = pango_font_description_from_string (font_name);
	g_assert (font->font_desc);

	font->pango_font = pango_context_load_font(gdk_pango_context_get(),
						   font->desc);
	
	font->gdk_font = gdk_font_from_description (font->font_desc);
	/* FIXME: add expection handling when font is not on system */
	g_assert(font->gdk_font);

	gdk_string_extents(font->gdk_font, font_chars, 0, 0, 0, &font->asc, &font->desc);                  

	return font;	
}

void
cong_font_delete(CongFont *font)
{
	g_assert(0); /* unimplemented */
}



