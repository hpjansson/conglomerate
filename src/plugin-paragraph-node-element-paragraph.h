/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-paragraph-node-element-paragraph.h
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

#ifndef __PLUGIN_PARAGRAPH_NODE_ELEMENT_PARAGRAPH_H__
#define __PLUGIN_PARAGRAPH_NODE_ELEMENT_PARAGRAPH_H__

#include "cong-editor-node-element.h"

G_BEGIN_DECLS

#define CONG_EDITOR_NODE_ELEMENT_PARAGRAPH_TYPE	      (cong_editor_node_element_paragraph_get_type ())
#define CONG_EDITOR_NODE_ELEMENT_PARAGRAPH(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_NODE_ELEMENT_PARAGRAPH_TYPE, CongEditorNodeElementParagraph)
#define CONG_EDITOR_NODE_ELEMENT_PARAGRAPH_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_NODE_ELEMENT_PARAGRAPH_TYPE, CongEditorNodeElementParagraphClass)
#define IS_CONG_EDITOR_NODE_ELEMENT_PARAGRAPH(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_NODE_ELEMENT_PARAGRAPH_TYPE)

CONG_EDITOR_NODE_DECLARE_PLUGIN_SUBCLASS(Paragraph, paragraph)

G_END_DECLS

#endif
