/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-primary-window.h
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

#ifndef __CONG_PRIMARY_WINDOW_H__
#define __CONG_PRIMARY_WINDOW_H__

G_BEGIN_DECLS

/**
 * cong_primary_window_new:
 * @doc: the document that should be loaded into the new window; if NULL, a document-less
 * window is created
 * 
 * Creates a new conglomerate window; If @doc is NULL, only the GUI is created, if not,
 * the @doc is added to the gui by calling cong_primary_window_add_doc().
 * 
 * Returns: The new created window
 */
CongPrimaryWindow*
cong_primary_window_new (CongDocument *doc);


void
cong_primary_window_add_doc (CongPrimaryWindow *primary_window, CongDocument *doc);

void
cong_primary_window_free (CongPrimaryWindow *primary_window);

CongDocument*
cong_primary_window_get_document (CongPrimaryWindow *primary_window);

void
cong_primary_window_update_title (CongPrimaryWindow *primary_window);

GtkWindow*
cong_primary_window_get_toplevel (CongPrimaryWindow *primary_window);

gboolean cong_primary_window_can_close(CongPrimaryWindow *primary_window);

G_END_DECLS

#endif
