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
	int stripped_byte_count; /* number of bytes within the stripped plaintext cache */
};

struct CongTextCache
{
	/* Behaviour: */
	gboolean strip_whitespace;

	/*  Inputs: */
	gchar *input_string;
	PangoAttrList *input_attr_list; /* can be NULL if client code isn't interested in attributes */

	/* Outputs: */
	gchar *output_string; /* NULL if it needs regenerating */
	PangoAttrList *output_attr_list; /* NULL if it needs regenerating */
	GList *list_of_span; /* NULL if it needs regenerating */
};

/* Internal function declarations: */
static void
regenerate_output_text (CongTextCache *text_cache);

static void
regenerate_output_attr_list (CongTextCache *text_cache);

static void
clear_cache (CongTextCache *text_cache);

static CongTextCacheSpan* 
cong_text_cache_span_new(int original_first_byte_offset,
			 int stripped_first_byte_offset,
			 int stripped_byte_count);


static CongTextCacheSpan *
get_text_span_at_stripped_byte_offset (CongTextCache *text_cache, 
				       int byte_offset);

static CongTextCacheSpan *
get_text_span_at_original_byte_offset (CongTextCache *text_cache, 
				       int byte_offset);

/* Exported function definitions: */
/**
 * cong_text_cache_new:
 * @strip_whitespace:  Should whitespace be stripped ("normalised") if TRUE, or preserved if FALSE
 * @string: the UTF8 string, must be non-NULL
 * @attr_list:  Pango attributes for the string; can be NULL if you don't care about them
 *
 * Create a new #CongTextCache.
 *
 * Returns: the new #CongTextCache
 */
CongTextCache*
cong_text_cache_new (gboolean strip_whitespace,
		     const gchar *string,
		     PangoAttrList *attr_list)
{
	CongTextCache* cache;

	g_return_val_if_fail (string, NULL);

	cache = g_new0(CongTextCache,1);

	cache->strip_whitespace = strip_whitespace;

	cong_text_cache_set_input_text (cache,
					string);
	cong_text_cache_set_input_attributes (cache,
					      attr_list);
	return cache;
}

/**
 * cong_text_cache_free:
 * @text_cache:
 *
 * TODO: Write me
 */
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

/**
 * cong_text_cache_get_output_text:
 * @text_cache: the text cache from which to get the result
 *
 * Get the text from the cache, which will have had the appropriate operation performed on it.
 *
 * Returns: the result of the operation as a UTF-8 string (owned by the #CongTextCache)
 */
const gchar*
cong_text_cache_get_output_text (CongTextCache* text_cache)
{
	g_return_val_if_fail (text_cache, NULL);

	if (NULL==text_cache->output_string) {
		regenerate_output_text (text_cache);
	}

	g_assert (text_cache->output_string);
	return text_cache->output_string;
}

/**
 * cong_text_cache_get_output_attributes:
 * @text_cache:
 *
 * Get the text attributes from the cache, which will have had the appropriate operation performed on it.
 *
 * Returns: the attributes, which you must unref when you are finished.
 */
PangoAttrList*
cong_text_cache_get_output_attributes (CongTextCache* text_cache)
{
	g_return_val_if_fail (text_cache, NULL);

	if (NULL==text_cache->input_attr_list) {
		return NULL;
	}

	if (NULL==text_cache->output_attr_list) {
		regenerate_output_attr_list (text_cache);
	}

	g_assert (text_cache->output_attr_list);
	return text_cache->output_attr_list;
}

/**
 * cong_text_cache_set_input_text:
 * @text_cache:
 * @input_string:
 *
 * TODO: Write me
 */
void
cong_text_cache_set_input_text (CongTextCache* text_cache,
				const gchar* input_string)
{
	g_return_if_fail (text_cache);
	g_return_if_fail (input_string);

	if (text_cache->input_string) {
		g_free (text_cache->input_string);
	}
	text_cache->input_string = g_strdup (input_string);

	clear_cache (text_cache);
}

/**
 * cong_text_cache_set_input_attributes:
 * @text_cache:
 * @attr_list:
 *
 * TODO: Write me
 */
void
cong_text_cache_set_input_attributes (CongTextCache* text_cache,
				      PangoAttrList *attr_list)
{
	g_return_if_fail (text_cache);
	g_return_if_fail (attr_list);

	pango_attr_list_ref (attr_list);
	if (text_cache->input_attr_list) {
		pango_attr_list_unref (text_cache->input_attr_list);
	}

	text_cache->input_attr_list = attr_list;

	if (text_cache->output_attr_list) {
		pango_attr_list_unref (text_cache->output_attr_list);
		text_cache->output_attr_list = NULL;
	}
}

/**
 * cong_text_cache_convert_stripped_byte_offset_to_original:
 * @text_cache:
 * @stripped_byte_offset:
 * @original_byte_offset:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_text_cache_convert_stripped_byte_offset_to_original (CongTextCache *text_cache,
							  int stripped_byte_offset,
							  int *original_byte_offset)
{
	CongTextCacheSpan *text_span;

	g_return_val_if_fail(text_cache, FALSE);
	g_return_val_if_fail(original_byte_offset, FALSE);
	g_return_val_if_fail(stripped_byte_offset>=0, FALSE);
	g_return_val_if_fail(stripped_byte_offset<=strlen(text_cache->input_string), FALSE);

	text_span = get_text_span_at_stripped_byte_offset(text_cache, stripped_byte_offset);
	
	if (text_span) {
		g_assert (stripped_byte_offset >= text_span->stripped_first_byte_offset);
		g_assert (stripped_byte_offset <= (text_span->stripped_first_byte_offset + text_span->stripped_byte_count));
		*original_byte_offset = text_span->original_first_byte_offset + (stripped_byte_offset - text_span->stripped_first_byte_offset);

		g_assert (*original_byte_offset>=0);
		g_assert (*original_byte_offset<=strlen(text_cache->input_string));

#if DEBUG_TEXT_STRIPPING
		g_message("stripped byte offset %i -> original offset %i", stripped_byte_offset, *original_byte_offset);
#endif
		return TRUE;
	} else {
#if DEBUG_TEXT_STRIPPING
		g_message("stripped byte offset %i could not be converted", stripped_byte_offset);
#endif
	}

	return FALSE;
}

/**
 * cong_text_cache_convert_original_byte_offset_to_stripped:
 * @text_cache:
 * @original_byte_offset:
 * @stripped_byte_offset:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_text_cache_convert_original_byte_offset_to_stripped (CongTextCache *text_cache,
							  int original_byte_offset,
							  int *stripped_byte_offset)
{
	CongTextCacheSpan *text_span;

	g_return_val_if_fail(text_cache, FALSE);
	g_return_val_if_fail(stripped_byte_offset, FALSE);
	g_return_val_if_fail(original_byte_offset>=0, FALSE);
	g_return_val_if_fail(original_byte_offset<=strlen(text_cache->input_string), FALSE);

	text_span = get_text_span_at_original_byte_offset(text_cache, original_byte_offset);
	
	if (text_span) {
		int byte_count_into_span;

		g_assert (original_byte_offset >= text_span->original_first_byte_offset);

		byte_count_into_span = original_byte_offset - text_span->original_first_byte_offset;
		if (byte_count_into_span <= text_span->stripped_byte_count) {
			*stripped_byte_offset = text_span->stripped_first_byte_offset + byte_count_into_span;
		} else {
			*stripped_byte_offset = text_span->stripped_first_byte_offset + text_span->stripped_byte_count;
		}

		g_assert (*stripped_byte_offset>=0);
		g_assert (*stripped_byte_offset<=strlen(text_cache->output_string));

#if DEBUG_TEXT_STRIPPING
		g_message("original byte offset %i -> stripped offset %i", original_byte_offset, *stripped_byte_offset);
#endif
		return TRUE;
	} else {
#if DEBUG_TEXT_STRIPPING
		g_message("original byte offset %i could not be converted", original_byte_offset);
#endif
	}

	return FALSE;
}


/* Internal function definitions: */
static void
regenerate_output_text (CongTextCache *text_cache)
{
	g_assert (text_cache);

	g_assert (text_cache->input_string);

	clear_cache (text_cache);

	if (text_cache->strip_whitespace) {
		gchar *dst;
		const gchar *src = text_cache->input_string;
		gboolean last_char_was_space=FALSE;
 		CongTextCacheSpan *text_span;
		int original_byte_offset_start_of_span = 0;
		int stripped_byte_offset_start_of_span = 0;
		
		g_assert (NULL==text_cache->output_string);
		text_cache->output_string = g_malloc((strlen(text_cache->input_string)*8)+1);
		
		dst = text_cache->output_string;
		
		while (*src) {			
			gunichar c = g_utf8_get_char (src);
			gboolean this_char_is_space = g_unichar_isspace(c);
			
			if (this_char_is_space) {
				
				if (!last_char_was_space) {
					
					/* Write a space into the buffer: */
					*(dst++) = ' ';
					
					/* Add stuff to the list of spans */
					text_span = cong_text_cache_span_new (original_byte_offset_start_of_span,
									      stripped_byte_offset_start_of_span,
									      (dst-text_cache->output_string) - stripped_byte_offset_start_of_span);
					text_cache->list_of_span = g_list_append(text_cache->list_of_span, 
										 text_span);

				}
				
			} else {
				
				if (last_char_was_space) {
					/* We're starting what will be a new span; record where we've got to: */
					original_byte_offset_start_of_span = (src-text_cache->input_string); 
					stripped_byte_offset_start_of_span = (dst-text_cache->output_string);
				}
				
				/* Write character as utf-8 into buffer: */
				dst += g_unichar_to_utf8(c, dst);
			}
			
			last_char_was_space = this_char_is_space;

			src = g_utf8_next_char(src);
		}
		
		if (!last_char_was_space) {
			/* Add stuff to the list of spans */
			text_span = cong_text_cache_span_new (original_byte_offset_start_of_span,
							      stripped_byte_offset_start_of_span,
							      (dst-text_cache->output_string) - stripped_byte_offset_start_of_span);
			text_cache->list_of_span = g_list_append (text_cache->list_of_span, 
								  text_span);
		}
		
		/* Terminate the string: */
		*dst = '\0';
	} else {
		/* Just generate one big span: */
 		CongTextCacheSpan *text_span;

		text_cache->output_string = g_strdup (text_cache->input_string);

		text_span = cong_text_cache_span_new (0,
						      0,
						      strlen (text_cache->input_string));
		text_cache->list_of_span = g_list_append (text_cache->list_of_span, 
							  text_span);

	}

#if DEBUG_TEXT_STRIPPING
	{
		gchar *cleaned_input = cong_util_cleanup_text(input_string);
		gchar *cleaned_output = cong_util_cleanup_text(text_cache->stripped_string);

		g_message("stripped \"%s\" into \"%s\"", cleaned_input, cleaned_output);

		g_free(cleaned_input);
		g_free(cleaned_output);		
	}
	{
		GList *iter;

		for (iter = text_cache->list_of_span; iter; iter = iter->next) {
			CongTextCacheSpan *text_span = iter->data;
			g_assert(text_span);

			g_printf ("[%i->%i/%i->%i)", 
				  text_span->original_first_byte_offset, text_span->original_first_byte_offset+text_span->stripped_byte_count,
				  text_span->stripped_first_byte_offset, text_span->stripped_first_byte_offset+text_span->stripped_byte_count);
		}		
		g_printf("\n");
	}
#endif
}

static void
regenerate_output_attr_list (CongTextCache *text_cache)
{
	PangoAttrIterator* iter;

	g_assert (text_cache);
	g_assert (NULL==text_cache->output_attr_list);
	g_assert (text_cache->input_attr_list);



#if 1
	text_cache->output_attr_list = pango_attr_list_new ();

	iter = pango_attr_list_get_iterator (text_cache->input_attr_list);

	do {
		GSList *first_attr;
		GSList *inner_iter;

		first_attr = pango_attr_iterator_get_attrs (iter);

		for (inner_iter = first_attr; inner_iter; inner_iter=inner_iter->next) {

			/* Clone, with modified range: */
			PangoAttribute *cloned_attr = pango_attribute_copy (inner_iter->data);
			
			if (!cong_text_cache_convert_original_byte_offset_to_stripped (text_cache,
										       cloned_attr->start_index,
										       &cloned_attr->start_index)){
				g_warning ("unable to convert attribute start index %i", cloned_attr->start_index);
			}
			if (!cong_text_cache_convert_original_byte_offset_to_stripped (text_cache,
										       cloned_attr->end_index,
										       &cloned_attr->end_index)){
				g_warning ("unable to convert attribute end index %i", cloned_attr->end_index);
			}
			
			pango_attr_list_insert (text_cache->output_attr_list,
						cloned_attr);
			
			pango_attribute_destroy (inner_iter->data);
		}
		
		g_slist_free (first_attr);

	} while (pango_attr_iterator_next (iter));
	
	pango_attr_iterator_destroy (iter);
#else
	/* For now: */
	text_cache->output_attr_list = pango_attr_list_copy (text_cache->input_attr_list);
#endif

}

static void
clear_cache (CongTextCache *text_cache)
{
	g_assert (text_cache);

	/* Clear any results; they will need recalculating: */

	if (text_cache->output_string) {
		g_free (text_cache->output_string);
		text_cache->output_string = NULL;
	}

	if (text_cache->output_attr_list) {
		pango_attr_list_unref (text_cache->output_attr_list);
		text_cache->output_attr_list = NULL;
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
}


static CongTextCacheSpan* 
cong_text_cache_span_new(int original_first_byte_offset,
			 int stripped_first_byte_offset,			 
			 int stripped_byte_count)
{
	CongTextCacheSpan *text_span;

	text_span = g_new0(CongTextCacheSpan,1);
	text_span->original_first_byte_offset = original_first_byte_offset;
	text_span->stripped_first_byte_offset = stripped_first_byte_offset;
	text_span->stripped_byte_count = stripped_byte_count;

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

		if (byte_offset <= (text_span->stripped_first_byte_offset + text_span->stripped_byte_count) ) {
			return text_span;
		}
	}

	return NULL;
}

static CongTextCacheSpan *
get_text_span_at_original_byte_offset (CongTextCache *text_cache, 
				       int byte_offset)
{
	GList *iter;

	g_return_val_if_fail(text_cache, NULL);
	
	/* Scan through the text spans, looking for the byte offset: */
	for (iter = text_cache->list_of_span; iter; iter = iter->next) {
		CongTextCacheSpan *text_span = iter->data;
		g_assert(text_span);

		if (byte_offset < text_span->original_first_byte_offset) {
			/* Then we are within the stripped region of whitespace at the end of the prior text span: */
			g_assert (iter->prev);
			g_assert (iter->prev->data);
			return 	(CongTextCacheSpan *)iter->prev->data;
		}

		if (byte_offset <= (text_span->original_first_byte_offset + text_span->stripped_byte_count) ) {
			return text_span;
		}
	}

	return NULL;
}
