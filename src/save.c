/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-primary-window.c
 *
 * Copyright (C) 2002 David Malcolm
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
 *          Hans Petter Jansson <hpj@ximian.com>
 */

#include <gtk/gtk.h>

#include "global.h"

gint save_document(GtkWidget *w, gpointer data)
{
	CongDocument *doc = data;
	const char *doc_name;

	g_assert(doc);

	doc_name = get_file_name("Save XML as...");
	if (!doc_name) return(TRUE);
	
	cong_document_save(doc, doc_name);

	return(TRUE);
}
