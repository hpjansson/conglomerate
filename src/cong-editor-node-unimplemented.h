/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-node-unimplemented.h
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

#ifndef __CONG_EDITOR_NODE_UNIMPLEMENTED_H__
#define __CONG_EDITOR_NODE_UNIMPLEMENTED_H__

#include "cong-editor-node.h"

G_BEGIN_DECLS

#define CONG_EDITOR_NODE_UNIMPLEMENTED_TYPE	      (cong_editor_node_unimplemented_get_type ())
#define CONG_EDITOR_NODE_UNIMPLEMENTED(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_NODE_UNIMPLEMENTED_TYPE, CongEditorNodeUnimplemented)
#define CONG_EDITOR_NODE_UNIMPLEMENTED_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_NODE_UNIMPLEMENTED_TYPE, CongEditorNodeUnimplementedClass)
#define IS_CONG_EDITOR_NODE_UNIMPLEMENTED(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_NODE_UNIMPLEMENTED_TYPE)

typedef struct CongEditorNodeUnimplemented CongEditorNodeUnimplemented;
typedef struct CongEditorNodeUnimplementedClass CongEditorNodeUnimplementedClass;
typedef struct CongEditorNodeUnimplementedDetails CongEditorNodeUnimplementedDetails;

struct CongEditorNodeUnimplemented
{
	CongEditorNode editor_node;

	CongEditorNodeUnimplementedDetails *private;
};

struct CongEditorNodeUnimplementedClass
{
	CongEditorNodeClass klass;

	/* Methods? */
};

GType
cong_editor_node_unimplemented_get_type (void);

CongEditorNodeUnimplemented*
cong_editor_node_unimplemented_construct (CongEditorNodeUnimplemented *editor_node_unimplemented,
					  CongEditorWidget3* widget,
					  CongNodePtr node,
					  CongEditorNode *traversal_parent,
					  const gchar *description);

CongEditorNode*
cong_editor_node_unimplemented_new (CongEditorWidget3* widget,
				    CongNodePtr node,
				    CongEditorNode *traversal_parent,
				    const gchar *description);

G_END_DECLS

#endif
