/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-edit-find-and-replace.h
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

#ifndef __CONG_EDIT_FIND_AND_REPLACE_H__
#define __CONG_EDIT_FIND_AND_REPLACE_H__

G_BEGIN_DECLS


struct CongFindDialogData
{
   gchar *last_find;
   gchar *last_replace;
   
   gboolean  is_wrap_around;
   gboolean  is_entire_word;
   gboolean  is_match_case;
   gboolean  is_search_backwards;
};

/**
 * cong_document_find:
 * @doc: the #CongDocument for which the find dialog is to be run
 *
 * Presents the Find dialog for this #CongDocument
 *
 */
void
cong_document_find (CongDocument *doc);

/**
 * cong_document_find_next:
 * @doc: the #CongDocument
 *
 * Perform an "Edit->Find Next" for this #CongDocument
 *
 */
void
cong_document_find_next (CongDocument *doc);

/**
 * cong_document_find_prev:
 * @doc: the #CongDocument
 *
 * Perform an "Edit->Find Next" for this #CongDocument
 *
 */
void
cong_document_find_prev (CongDocument *doc);

/**
 * cong_document_replace:
 * @doc: the #CongDocument for which the dialog is to be run
 *
 * Presents the Replace dialog for this #CongDocument
 *
 */
void
cong_document_replace (CongDocument *doc);


G_END_DECLS

#endif
