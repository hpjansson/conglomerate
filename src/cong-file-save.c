/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-file-save.c
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
 *          Hans Petter Jansson <hpj@ximian.com>
 */

#include <gtk/gtk.h>

#include "global.h"

#include "cong-document.h"
#include "cong-primary-window.h"
#include "cong-file-selection.h"
#include "cong-ui-hooks.h"

/**
 * toolbar_callback_save:
 * @w:
 * @data:
 *
 * TODO: Write me
 * Returns:
 */
gint 
toolbar_callback_save(GtkWidget *w, gpointer data)
{
	CongPrimaryWindow *primary_window = data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	return save_document(doc, cong_primary_window_get_toplevel(primary_window));
}

/**
 * save_document_as:
 * @doc:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
gint 
save_document_as(CongDocument *doc, GtkWindow *parent_window)
{
	char *current_doc_name;
	char *new_doc_name;

	g_return_val_if_fail(doc, FALSE);
	g_return_val_if_fail(parent_window, FALSE);

	current_doc_name = cong_document_get_full_uri(doc);
	
	new_doc_name = cong_get_file_name(_("Save XML as..."), 
					  current_doc_name,
					  parent_window,
					  CONG_FILE_CHOOSER_ACTION_SAVE,
					  cong_file_selection_make_xml_filter_list ());
	if (current_doc_name) {
		g_free(current_doc_name);
	}

	if (!new_doc_name) {
		return TRUE;
	}
	
	cong_document_save(doc, new_doc_name, parent_window);

	g_free(new_doc_name);
	
	return TRUE;
}

/**
 * save_document:
 * @doc:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
gint 
save_document(CongDocument *doc, GtkWindow *parent_window) 
{
	gchar *doc_name;

	g_return_val_if_fail(doc, FALSE);

	doc_name = cong_document_get_full_uri(doc);

	if (!doc_name) {
		return save_document_as(doc, parent_window);
	}

	cong_document_save(doc, doc_name, parent_window);
	
	g_free(doc_name);

	return TRUE;
}
