/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-file-selection.h
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

#ifndef __CONG_FILE_SELECTION_H__
#define __CONG_FILE_SELECTION_H__

G_BEGIN_DECLS

typedef enum
{
  CONG_FILE_CHOOSER_ACTION_OPEN,
  CONG_FILE_CHOOSER_ACTION_SAVE
} CongFileChooserAction;

gchar*
cong_get_file_name (const gchar *title, 
		    const gchar *filename,
		    GtkWindow *parent_window,
		    CongFileChooserAction action,
		    GList *list_of_filters);

gchar*
cong_get_file_name_with_filter (const gchar *title, 
				const gchar *filename,
				GtkWindow *parent_window,
				CongFileChooserAction action,
				GList *list_of_filters,
				GtkFileFilter **output_filter);

GList*
cong_file_selection_make_xml_filter_list (void);

G_END_DECLS

#endif
