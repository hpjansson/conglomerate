/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-selection.h
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

#ifndef __CONG_SELECTION_H__
#define __CONG_SELECTION_H__

#include "cong-range.h"

G_BEGIN_DECLS

/* Selection methods: */
CongSelection*
cong_selection_new (void);
void 
cong_selection_free (CongSelection *selection);

void
cong_selection_import (CongSelection *selection, 
		       GtkWidget* widget);

void 
cong_selection_start_from_curs (CongSelection *selection, 
				CongCursor *curs);

void 
cong_selection_end_from_curs (CongSelection *selection, 
			      CongCursor *curs);

CongRange*
cong_selection_get_logical_range (CongSelection *selection);

CongRange*
cong_selection_get_ordered_range (CongSelection *selection);

CongLocation*
cong_selection_get_logical_start (CongSelection *selection);

CongLocation*
cong_selection_get_logical_end (CongSelection *selection);

CongLocation*
cong_selection_get_ordered_start (CongSelection *selection);

CongLocation*
cong_selection_get_ordered_end (CongSelection *selection);

void
cong_selection_set_logical_start (CongSelection *selection,
				  const CongLocation *location);
void
cong_selection_set_logical_end (CongSelection *selection,
				const CongLocation *location);

void
cong_selection_set_logical_range (CongSelection *selection,
				  const CongLocation *start_loc,
				  const CongLocation *end_loc);

gchar* 
cong_selection_get_selected_text (CongDocument *doc);

gboolean
cong_selection_is_node (CongSelection *selection,
			CongNodePtr node);

void
cong_selection_nullify (CongSelection *selection);

gboolean
cong_selection_is_valid (CongSelection *selection);

GdkGC*
cong_selection_legacy_get_gc_valid(CongSelection *selection);
GdkGC*
cong_selection_legacy_get_gc_invalid(CongSelection *selection);

G_END_DECLS

#endif
