/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-range.c
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
#include "cong-range.h"
#include "cong-eel.h"

/**
 * cong_range_init:
 * @range:
 *
 * TODO: Write me
 */
void
cong_range_init (CongRange *range)
{
	g_return_if_fail (range);

	cong_location_nullify(&range->loc0);
	cong_location_nullify(&range->loc1);
}

/**
 * cong_range_nullify:
 * @range:
 *
 * TODO: Write me
 */
void
cong_range_nullify (CongRange *range)
{
	g_return_if_fail (range);

	cong_location_nullify(&range->loc0);
	cong_location_nullify(&range->loc1);
}

/**
 * cong_range_is_valid:
 * @range:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_range_is_valid (CongRange *range)
{
	g_return_val_if_fail (range, FALSE);

	if (range->loc0.node==NULL) {
		return (range->loc1.node==NULL);
	}

	if (!cong_location_is_valid(&range->loc0)) {
		return FALSE;
	}

	if (!cong_location_is_valid(&range->loc1)) {
		return FALSE;
	}

	/* For this to work; the range has to have the start/end at the same nesting level i.e. they must be siblings 
	   Is this a hard restriction?  Or can ranges be either way (perhaps with a boolean test function to determine well-formedness?
	   What about ordering of the range?
	 */
	return (range->loc0.node->parent == range->loc1.node->parent);
}

/**
 * cong_range_generate_source:
 * @range:
 *
 * TODO: Write me
 * Returns:
 */
gchar*
cong_range_generate_source (CongRange *range)
{
	CongNodePtr iter;
	gchar *result = NULL;

	g_return_val_if_fail (range, NULL);
	g_return_val_if_fail (cong_range_is_valid (range),
			      NULL);

	if (NULL == range->loc0.node) {
		return NULL;
	}
	
	g_assert (cong_range_is_ordered (range));


	/* We have a single node: */
	if (range->loc0.node==range->loc1.node) {
		if (cong_node_supports_byte_offsets (range->loc0.node)) {
			return cong_node_generate_source_between_byte_offsets (range->loc0.node,
									       range->loc0.byte_offset,
									       range->loc1.byte_offset);
		} else {
			return cong_node_generate_source (range->loc0.node);
		}
	} else {
		result = cong_node_generate_source_from_byte_offset (range->loc0.node, 
								     range->loc0.byte_offset);
		
		for (iter = range->loc0.node->next; TRUE; iter=iter->next) {
			gchar *source_for_iter;
			
			g_assert(iter);

			if (iter==range->loc1.node) {
				source_for_iter = cong_node_generate_source_up_to_byte_offset (iter,
											       range->loc1.byte_offset);
			} else {
				source_for_iter = cong_node_generate_source (iter);				
			}

			cong_eel_set_string(&result, g_strdup_printf("%s%s", result, source_for_iter));
			g_free(source_for_iter);

			if (iter==range->loc1.node) {
				break;
			}
		}
	}

	return result;
}

/**
 * cong_range_exists:
 * @range:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_range_exists (CongRange *range)
{
	g_return_val_if_fail (range, FALSE);

	if (cong_location_exists(&range->loc0) && cong_location_exists(&range->loc1)) {
		return TRUE;
	} else {
		return FALSE;
	}	
}

/**
 * cong_range_is_empty:
 * @range:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_range_is_empty (CongRange *range)
{
	g_return_val_if_fail (range, TRUE);
	
	if (cong_location_equals(&range->loc0, &range->loc1)) {
		/* The range is empty iff we're expecting byte offsets to be valid: */
		return cong_node_supports_byte_offsets (range->loc0.node);
	} else {
		/* Non-equal start/end: range is non-empty: */
		return FALSE;
	}


}

/**
 * cong_range_is_ordered:
 * @range:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_range_is_ordered (CongRange *range)
{
	g_return_val_if_fail (range, FALSE);

	if (range->loc0.node==range->loc1.node) {

		return (range->loc0.byte_offset<=range->loc1.byte_offset);

	} else {
		/* Non-equal node ptrs: */

		/* FIXME: unwritten */
		return TRUE;

#if 0
		if (range->loc0.node->parent == range->loc1.node->parent) {
		}
#endif

	}

}

/**
 * cong_range_can_be_cut:
 * @range:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_range_can_be_cut (CongRange *range)
{
	g_return_val_if_fail (range, FALSE);

	if (!(cong_range_exists (range) &&
	      cong_range_is_valid (range))) { 
		return FALSE;
	}

	if (cong_range_is_empty (range)) {
		return FALSE;
	}


	if (!cong_range_is_ordered (range)) {
		return FALSE;
	}


	/* If we have a single node: */
	if (range->loc0.node==range->loc1.node) {
		return cong_node_can_be_cut (range->loc0.node);
	} else {
		return TRUE;
	}
}

/**
 * cong_range_can_be_copied:
 * @range:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_range_can_be_copied (CongRange *range)
{
	/* FIXME: Should be different conditions, this is in Bugzilla
	   as Bug #126091 - to be fixed after 0.8.0 */
	return cong_range_can_be_cut(range);
}

/**
 * cong_range_make_ordered:
 * @range:
 *
 * TODO: Write me
 */
void
cong_range_make_ordered (CongRange *range)
{
	g_return_if_fail (range);

	if (range->loc0.node==range->loc1.node) {

		/* Equal node ptrs: swap byte offsets if necessary: */
		if (range->loc0.byte_offset>range->loc1.byte_offset) {
			int tmp = range->loc0.byte_offset;
			range->loc0.byte_offset = range->loc1.byte_offset;
			range->loc1.byte_offset = tmp;
		}

	} else {
		/* Non-equal node ptrs: swap node ptrs if necessary*/
		int ordering = cong_node_get_ordering (range->loc0.node, range->loc1.node);

#if 0
		{
			gchar *desc0 = cong_node_debug_description (range->loc0.node);
			gchar *desc1 = cong_node_debug_description (range->loc1.node);
			g_message ("comparing %s and %s got %i", desc0, desc1, ordering);

			g_free (desc0);
			g_free (desc1);
		}
#endif
		
		if (ordering>0) {
			/* Swap the node ptrs: */
			CongLocation tmp;
			
			cong_location_copy(&tmp, &range->loc0);
			cong_location_copy(&range->loc0, &range->loc1);
			cong_location_copy(&range->loc1, &tmp);
		}
	}
	
}

/**
 * cong_range_is_node:
 * @range:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_range_is_node (CongRange *range,
		    CongNodePtr node)
{
	g_return_val_if_fail (range, FALSE);

	if (range->loc0.node==node && range->loc0.byte_offset==CONG_LOCATION_BYTE_OFFSET_MEANINGLESS) {
		if (range->loc1.node==node && range->loc1.byte_offset==CONG_LOCATION_BYTE_OFFSET_MEANINGLESS) {
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * cong_range_copy:
 * @dst:
 * @src:
 *
 * TODO: Write me
 */
void
cong_range_copy(CongRange *dst, const CongRange *src)
{
	g_return_if_fail (dst);
	g_return_if_fail (src);

	*dst = *src;
}
