/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-text-span.c
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
#include "cong-text-cache.h"
#include "cong-util.h"

#define DEBUG_TEXT_STRIPPING 0

typedef struct CongTextCacheSpan CongTextCacheSpan;

struct CongTextCacheSpan
{
	int original_first_byte_offset; /* offset into the text within the string */
	int stripped_first_byte_offset; /* offset into the stripped plaintext cache */
	int byte_count; /* number of bytes within the stripped plaintext cache */
};

struct CongTextCache
{
	gboolean strip_whitespace;
	gchar *stripped_string;
	GList *list_of_span;
};

/* Internal function declarations: */
static CongTextCacheSpan* 
cong_text_cache_span_new(int original_first_byte_offset,
			 int stripped_first_byte_offset,
			 int byte_count);

static CongTextCacheSpan *
get_text_span_at_stripped_byte_offset (CongTextCache *text_cache, 
				       int byte_offset);

/* Exported function definitions: */
CongTextCache*
cong_text_cache_new (gboolean strip_whitespace,
		     const gchar *string)
{
	CongTextCache* cache = g_new0(CongTextCache,1);

	cache->strip_whitespace = strip_whitespace;

	cong_text_cache_set_text (cache,
				  string);

	return cache;
}

void
cong_text_cache_free (CongTextCache* text_cache)
{
	GList *iter;

	g_return_if_fail (text_cache);
	
	for (iter=text_cache->list_of_span; iter; iter=iter->next) {
		g_assert(iter->data);
		g_free (iter->data);
	}
	
	g_list_free (text_cache->list_of_span);
	
	g_free (text_cache);
}

const gchar*
cong_text_cache_get_text (CongTextCache* text_cache)
{
	g_return_val_if_fail (text_cache, NULL);

	return text_cache->stripped_string;
}

void
cong_text_cache_set_text (CongTextCache* text_cache,
			  const gchar* input_string)
{
	g_return_if_fail (text_cache);
	g_return_if_fail (input_string);

	if (text_cache->stripped_string) {
		g_free (text_cache->stripped_string);
		text_cache->stripped_string = NULL;
	}

	if (text_cache->list_of_span) {
		GList *iter;

		for (iter=text_cache->list_of_span; iter; iter=iter->next) {
			g_assert(iter->data);
			g_free (iter->data);
		}

		g_list_free(text_cache->list_of_span);

		text_cache->list_of_span = NULL;
	}

	if (text_cache->strip_whitespace) {
		gunichar *unichar_string;
		glong num_chars;
		gchar *dst;
		int i;
		gboolean last_char_was_space=FALSE;
		CongTextCacheSpan *text_span;
		int original_byte_offset_start_of_span = 0;
		int stripped_byte_offset_start_of_span = 0;

		
		unichar_string = g_utf8_to_ucs4_fast(input_string,
						     -1,
						     &num_chars);

		g_assert (NULL==text_cache->stripped_string);
		text_cache->stripped_string = g_malloc((num_chars*8)+1);
		
		dst = text_cache->stripped_string;
		
		for (i=0;i<num_chars;i++) {
			gunichar c = unichar_string[i];
			gboolean this_char_is_space = g_unichar_isspace(c);
			
			if (this_char_is_space) {
				
				if (!last_char_was_space) {
					
					/* Write a space into the buffer: */
					*(dst++) = ' ';
					
					/* Add stuff to the list of spans */
					text_span = cong_text_cache_span_new (original_byte_offset_start_of_span,
									      stripped_byte_offset_start_of_span,
									      (i+1-original_byte_offset_start_of_span));
					text_cache->list_of_span = g_list_append(text_cache->list_of_span, 
										 text_span);
					
				}
				
			} else {
				
				if (last_char_was_space) {
					/* We're starting what will be a new span; record where we've got to: */
					original_byte_offset_start_of_span = i;
					stripped_byte_offset_start_of_span = (dst-text_cache->stripped_string);
				}
				
				/* Write character as utf-8 into buffer: */
				dst += g_unichar_to_utf8(c, dst);
			}
			
			last_char_was_space = this_char_is_space;
		}
		
		g_free(unichar_string);
		
		if (!last_char_was_space) {
			/* Add stuff to the list of spans */
			text_span = cong_text_cache_span_new (original_byte_offset_start_of_span,
							      stripped_byte_offset_start_of_span,
							      (i-original_byte_offset_start_of_span));
			text_cache->list_of_span = g_list_append (text_cache->list_of_span, 
								  text_span);
		}
		
		/* Terminate the string: */
		*dst = '\0';
	} else {
		g_assert(0);
	}

#if DEBUG_TEXT_STRIPPING
	{
		gchar *cleaned_input = cong_util_cleanup_text(input_string);
		gchar *cleaned_output = cong_util_cleanup_text(text_cache->stripped_string);

		g_message("stripped \"%s\" into \"%s\"", cleaned_input, cleaned_output);

		g_free(cleaned_input);
		g_free(cleaned_output);
	}
#endif
}

gboolean
cong_text_cache_convert_stripped_byte_offset_to_original (CongTextCache *text_cache,
							  int stripped_byte_offset,
							  int *original_byte_offset)
{
	CongTextCacheSpan *text_span;

	g_return_val_if_fail(text_cache, FALSE);
	g_return_val_if_fail(original_byte_offset, FALSE);

	text_span = get_text_span_at_stripped_byte_offset(text_cache, stripped_byte_offset);
	
	if (text_span) {
		*original_byte_offset = text_span->original_first_byte_offset + (stripped_byte_offset - text_span->stripped_first_byte_offset);

		g_message("stripped byte offset %i -> original offset %i", stripped_byte_offset, *original_byte_offset);
		return TRUE;
	}

	return FALSE;
}


/* Internal function definitions: */
static CongTextCacheSpan* 
cong_text_cache_span_new(int original_first_byte_offset,
			 int stripped_first_byte_offset,
			 int byte_count)
{
	CongTextCacheSpan *text_span = g_new0(CongTextCacheSpan,1);
	text_span->original_first_byte_offset = original_first_byte_offset;
	text_span->stripped_first_byte_offset = stripped_first_byte_offset;
	text_span->byte_count = byte_count;

	return text_span;
}

static CongTextCacheSpan *
get_text_span_at_stripped_byte_offset (CongTextCache *text_cache, 
				       int byte_offset)
{
	GList *iter;

	g_return_val_if_fail(text_cache, NULL);
	
	/* Scan through the text spans, looking for the byte offset: */
	for (iter = text_cache->list_of_span; iter; iter = iter->next) {
		CongTextCacheSpan *text_span = iter->data;
		g_assert(text_span);
		
		g_assert(byte_offset >= text_span->stripped_first_byte_offset);

		if (byte_offset < (text_span->stripped_first_byte_offset + text_span->byte_count) ) {
			return text_span;
		}
	}

	return NULL;
}


