/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-document-traversal.h
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

#ifndef __CONG_DOCUMENT_TRAVERSAL_H__
#define __CONG_DOCUMENT_TRAVERSAL_H__

#include "cong-document.h"

G_BEGIN_DECLS

/**
   CongDocumentTraversal functions
 */

#define CONG_DOCUMENT_TRAVERSAL_TYPE         (cong_document_traversal_get_type ())
#define CONG_DOCUMENT_TRAVERSAL(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_DOCUMENT_TRAVERSAL_TYPE, CongDocumentTraversal)
#define CONG_DOCUMENT_TRAVERSAL_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_DOCUMENT_TRAVERSAL_TYPE, CongDocumentTraversalClass)
#define IS_CONG_DOCUMENT_TRAVERSAL(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_DOCUMENT_TRAVERSAL_TYPE)

typedef struct _CongDocumentTraversalDetails CongDocumentTraversalDetails;

struct _CongDocumentTraversal
{
	GObject object;

	CongDocumentTraversalDetails *private;
};

struct _CongDocumentTraversalClass
{
	GObjectClass klass;

	/* Methods? */
};

GType
cong_document_traversal_get_type (void);

CongDocumentTraversal*
cong_document_traversal_construct (CongDocumentTraversal *doc_traversal,
				   CongDocument *doc);

CongDocumentTraversal*
cong_document_traversal_new (CongDocument *doc);

CongDocument*
cong_document_traversal_get_document (CongDocumentTraversal *doc_traversal);

/**
 * cong_document_get_root_traversal_node:
 * @doc:  The #CongDocumentTraversal of interest
 *
 * The #CongDocumentTraversal maintains a tree of #CongTraversalNode objects corresponding to a depth-first traversal of its xml tree,
 * but with the entity references having only the entity definition as their sole child.
 *
 * Returns: the #CongTraversalNode corresponding to the root xml node of the document
 */
CongTraversalNode*
cong_document_traversal_get_root_traversal_node (CongDocumentTraversal *doc_traversal);

void
cong_document_traversal_for_each_traversal_node (CongDocumentTraversal *doc_traversal,
						 CongNodePtr xml_node,
						 void (*callback) (CongDocumentTraversal *doc_traversal, 
								   CongTraversalNode *traversal_node, 
								   gpointer user_data),
						 gpointer user_data);

gboolean
cong_document_traversal_has_traversal_node_for_node (CongDocumentTraversal *doc_traversal,
						     CongNodePtr node);

CongTraversalNode*
cong_document_traversal_get_traversal_node (CongDocumentTraversal *doc_traversal,
					    CongNodePtr xml_node,
					    CongTraversalNode *traversal_parent);

CongTraversalNode*
cong_document_traversal_get_a_traversal_node (CongDocumentTraversal *doc_traversal,
					      CongNodePtr xml_node);




G_END_DECLS

#endif
