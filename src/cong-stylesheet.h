/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-stylesheet.h
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

#ifndef __CONG_STYLESHEET_H__
#define __CONG_STYLESHEET_H__

G_BEGIN_DECLS

typedef struct _CongStylesheetParameter CongStylesheetParameter;
struct _CongStylesheetParameter
{
	gchar *name;
	gchar *value;
};

CongStylesheetParameter*
cong_stylesheet_parameter_new (const gchar *name,
			       const gchar *value);

void
cong_stylesheet_parameter_free (CongStylesheetParameter *parameter);

void
cong_stylesheet_parameter_list_free (GList *list_of_parameters);

void
cong_stylesheet_parameter_list_debug (GList *list_of_parameters);

G_END_DECLS

#endif
