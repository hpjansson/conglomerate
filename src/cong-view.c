/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-view.c
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

#include "global.h"
#include "cong-view.h"
#include "cong-document.h"

/**
 * cong_view_get_document:
 * @view:
 *
 * TODO: Write me
 * Returns:
 */
CongDocument*
cong_view_get_document(CongView *view)
{
	g_return_val_if_fail(view, NULL);

	return view->doc;
}

/**
 * cong_view_get_dispspec
 * @view:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspec*
cong_view_get_dispspec(CongView *view)
{
	g_return_val_if_fail(view, NULL);

	return cong_document_get_dispspec(view->doc);
}
