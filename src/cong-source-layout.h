/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-source-layout.h
 *
 * Tools for cleaning up the XML source of the document
 *
 * Copyright (C) 2004 David Malcolm
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

#ifndef __CONG_SOURCE_LAYOUT_H__
#define __CONG_SOURCE_LAYOUT_H__

G_BEGIN_DECLS

typedef struct CongSourceCleanupOptions CongSourceCleanupOptions;

struct CongSourceCleanupOptions
{
	gboolean use_tabs; /* if false then use spaces */
	guint num_spaces_per_indent; /* only if not using tabs */

	gboolean wrap_text; /* if true then wrap to a column */
	guint num_text_columns; /* only relevant if wrap_text is true */
};

void 
cong_util_cleanup_source (CongDocument *doc, 
			  const CongSourceCleanupOptions *options);

G_END_DECLS

#endif

