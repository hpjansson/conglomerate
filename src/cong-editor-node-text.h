/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node-text.h
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

#ifndef __CONG_EDITOR_NODE_TEXT_H__
#define __CONG_EDITOR_NODE_TEXT_H__

#include "cong-editor-node.h"

G_BEGIN_DECLS

#define CONG_EDITOR_NODE_TEXT_TYPE	      (cong_editor_node_text_get_type ())
#define CONG_EDITOR_NODE_TEXT(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_NODE_TEXT_TYPE, CongEditorNodeText)
#define CONG_EDITOR_NODE_TEXT_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_NODE_TEXT_TYPE, CongEditorNodeTextClass)
#define IS_CONG_EDITOR_NODE_TEXT(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_NODE_TEXT_TYPE)

CONG_EDITOR_NODE_DECLARE_SUBCLASS(Text, text)

gboolean
cong_editor_node_text_convert_original_byte_offset_to_stripped (CongEditorNodeText *editor_node_text,
								int original_byte_offset,
								int *stripped_byte_offset);


/* Utility for handling cursor movement: */
gboolean
cong_editor_node_text_calc_up (CongEditorNodeText *editor_node_text,
			       int input_byte_offset,
			       int* output_byte_offset);

/* Utility for handling cursor movement: */
gboolean
cong_editor_node_text_calc_down (CongEditorNodeText *editor_node_text,
				 int input_byte_offset,
				 int* output_byte_offset);


G_END_DECLS

#endif
