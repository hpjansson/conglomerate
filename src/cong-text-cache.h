/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-text-span.h
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

#ifndef __CONG_TEXT_SPAN_H__
#define __CONG_TEXT_SPAN_H__

G_BEGIN_DECLS

typedef struct CongTextCache CongTextCache;

#if 0
/**
 * Struct representing a run of characters within a plaintext cache from a specific text node; 
 * useful for converting from PangoLayoutLines back to the underlying XML: 
 * There can be more than one of these for a particular text node; each is a subset of the characters
 * within the text node - this is to cope with the case where surplus whitespace characters are
 * stripped when the text cache is built.
 */
struct CongTextCacheSpan
{
	int original_first_byte_offset; /* offset into the text within the original node */
	int stripped_first_byte_offset; /* offset into the stripped plaintext cache */
	int byte_count; /* number of bytes within the stripped plaintext cache */
};


CongTextCacheSpan* 
cong_text_cache_span_new (int original_first_byte_offset,
			  int stripped_first_byte_offset,
			  int byte_count);
#endif

CongTextCache*
cong_text_cache_new (gboolean strip_whitespace,
		     const gchar *string);

void
cong_text_cache_free (CongTextCache* text_cache);

const gchar*
cong_text_cache_get_text (CongTextCache* text_cache);

void
cong_text_cache_set_text (CongTextCache* text_cache,
			  const gchar* input_string);

gboolean
cong_text_cache_convert_stripped_byte_offset_to_original (CongTextCache *text_cache,
							  int stripped_byte_offset,
							  int *original_byte_offset);

G_END_DECLS

#endif
