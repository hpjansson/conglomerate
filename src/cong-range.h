/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-range.h
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

#ifndef __CONG_RANGE_H__
#define __CONG_RANGE_H__

G_BEGIN_DECLS

/* FIXME: we need to precisely specify where the endpoint of a range is; does it specify
   (i) the byte offset to the final character in the range
   (ii) the byte offset to the first character after the end of the range

   I believe we want it to be (ii)

   --------------------------
   This is a complicated test
   01234567890123456789012345
   ----------1111111111222222

   If the word "complicated" is highlighted (without the spaces), then the
   start location should have offset 10 and the end location have offset 21

   FIXME: we then need to go through all the code checking this!
   FIXME: do we support "backward" ranges?  How does it impact on all this?
 */
struct CongRange
{
	CongLocation loc0;
	CongLocation loc1;
};

void
cong_range_init (CongRange *range);

void
cong_range_nullify (CongRange *range);

gchar*
cong_range_generate_source (CongRange *range);

gboolean
cong_range_exists (CongRange *range);

gboolean
cong_range_is_empty (CongRange *range);

gboolean
cong_range_is_ordered (CongRange *range);

void
cong_range_make_ordered (CongRange *range);

void
cong_range_copy(CongRange *dst, const CongRange *src);

G_END_DECLS

#endif
