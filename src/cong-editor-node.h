/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node.h
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

#ifndef __CONG_EDITOR_NODE_H__
#define __CONG_EDITOR_NODE_H__

G_BEGIN_DECLS


#define CONG_EDITOR_NODE_TYPE	      (cong_editor_node_get_type ())
#define CONG_EDITOR_NODE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_NODE_TYPE, CongEditorNode)
#define CONG_EDITOR_NODE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_NODE_TYPE, CongEditorNodeClass)
#define IS_CONG_EDITOR_NODE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_NODE_TYPE)

typedef struct CongEditorNode CongEditorNode;
typedef struct CongEditorNodeClass CongEditorNodeClass;
typedef struct CongEditorNodeDetails CongEditorNodeDetails;

struct CongEditorNode
{
	GObject object;

	CongEditorNodeDetails *private;
};

struct CongElementEditorClass
{
	GObjectClass klass;

	/* Methods? */
#if 0
	void (*on_recursive_delete)(CongElementEditor *element_editor);
	void (*on_recursive_self_test)(CongElementEditor *element_editor);
	gboolean (*on_document_event)(CongElementEditor *element_editor, CongDocumentEvent *event);
	void (*get_size_requisition)(CongElementEditor *element_editor, int width_hint);
	void (*allocate_child_space)(CongElementEditor *element_editor);
	void (*recursive_render)(CongElementEditor *element_editor, const GdkRectangle *window_rect);
	void (*on_button_press)(CongElementEditor *element_editor, GdkEventButton *event);
	void (*on_motion_notify)(CongElementEditor *element_editor, GdkEventMotion *event);
	void (*on_key_press)(CongElementEditor *element_editor, GdkEventKey *event);
#endif
};

GType
cong_editor_node_get_type (void);

CongEditorWidget3*
cong_editor_node_get_widget (CongEditorNode *node);

CongNodePtr
cong_editor_node_get_node (CongEditorNode *node);


G_END_DECLS

#endif
