/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-location.h
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

#ifndef __CONG_LOCATION_H__
#define __CONG_LOCATION_H__


#define CONG_LOCATION_BYTE_OFFSET_NULL_NODE (-2)
#define CONG_LOCATION_BYTE_OFFSET_MEANINGLESS (-1)

/**
 * CongLocation
 * 
 * Struct representing a location within a document, with both a node ptr and a byte offset into the text.
 * The text is stored as UTF-8, and the offset is a byte offset, not a character offset.
 */
typedef struct _CongLocation
{
	CongNodePtr node;
	int byte_offset; /* byte offset, not a character offset; 
			    required to be CONG_LOCATION_BYTE_OFFSET_MEANINGLESS for nodes for which the offset is meaningless, 
			    and CONG_LOCATION_BYTE_OFFSET_NULL_NODE for a NULL node ptr */
} CongLocation;

/**
   Selftest routine:
*/
gboolean
cong_location_is_valid(const CongLocation *loc);

void
cong_location_nullify(CongLocation *loc);

void
cong_location_set_to_start_of_node(CongLocation *loc, CongNodePtr node);

void
cong_location_set_to_end_of_node(CongLocation *loc, CongNodePtr node);

void
cong_location_set_node_and_byte_offset(CongLocation *loc, CongNodePtr node, int byte_offset);

void
cong_location_set_node_and_char_offset(CongLocation *loc, CongNodePtr node, glong char_offset);

gboolean
cong_location_exists(const CongLocation *loc);

gboolean
cong_location_equals(const CongLocation *loc0, const CongLocation *loc1);

CongNodeType
cong_location_node_type(const CongLocation *loc);

gunichar
cong_location_get_unichar(const CongLocation *loc);

gchar*
cong_location_get_utf8_pointer(const CongLocation *loc);

void
cong_location_del_next_char(CongDocument *doc, const CongLocation *loc);

CongNodePtr
cong_location_xml_frag_prev(const CongLocation *loc);

CongNodePtr
cong_location_xml_frag_next(const CongLocation *loc);

CongNodePtr
cong_location_node(const CongLocation *loc);

CongNodePtr
cong_location_parent(const CongLocation *loc);

void
cong_location_copy(CongLocation *dst, const CongLocation *src);

gboolean
cong_location_calc_prev_char(const CongLocation *input_loc, 
			     CongDispspec *dispspec,
			     CongLocation *output_loc);
gboolean
cong_location_calc_next_char(const CongLocation *input_loc,
			     CongDispspec *dispspec,
			     CongLocation *output_loc);
gboolean
cong_location_calc_prev_word(const CongLocation *input_loc, 
			     CongDocument *doc,
			     CongLocation *output_loc);
gboolean
cong_location_calc_next_word(const CongLocation *input_loc, 
			     CongDocument *doc,
			     CongLocation *output_loc);
gboolean
cong_location_calc_document_start(const CongLocation *input_loc, 
				  CongDispspec *dispspec,
				  CongLocation *output_loc);
gboolean
cong_location_calc_line_start(const CongLocation *input_loc, 
			      CongDispspec *dispspec,
			      CongLocation *output_loc);
gboolean
cong_location_calc_document_end(const CongLocation *input_loc, 
				CongDispspec *dispspec,
				CongLocation *output_loc);
gboolean
cong_location_calc_line_end(const CongLocation *input_loc, 
			    CongDispspec *dispspec,
			    CongLocation *output_loc);
gboolean
cong_location_calc_prev_page(const CongLocation *input_loc, 
			     CongDispspec *dispspec,
			     CongLocation *output_loc);
gboolean
cong_location_calc_next_page(const CongLocation *input_loc, 
			     CongDispspec *dispspec,
			     CongLocation *output_loc);
gboolean
cong_location_calc_word_extent(const CongLocation *input_loc,
			       CongDocument *doc,
			       CongLocation *output_start_of_word,
			       CongLocation *output_end_of_word);
gboolean
cong_location_calc_prev_text_node (const CongLocation *input_loc, 
				   CongDispspec *dispspec,
				   CongLocation *output_loc);
gboolean
cong_location_calc_next_text_node (const CongLocation *input_loc,
				   CongDispspec *dispspec,
				   CongLocation *output_loc);

void
cong_location_copy_with_ref (CongDocument *doc, 
			     CongLocation *dst,
			     const CongLocation *src);
void
cong_location_nullify_with_ref (CongDocument *doc, 
				CongLocation *loc);

#endif
