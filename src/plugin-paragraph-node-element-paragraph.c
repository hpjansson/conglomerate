/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-paragraph-node-element-paragraph.c
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

#include "global.h"
#include "plugin-paragraph-node-element-paragraph.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "plugin-paragraph-area-paragraph.h"

CONG_EDITOR_NODE_DEFINE_PLUGIN_SUBCLASS(Paragraph, paragraph, CONG_EDITOR_NODE_ELEMENT_PARAGRAPH, cong_editor_area_paragraph_new)
