/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-document.h
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

#ifndef __CONG_DOCUMENT_H__
#define __CONG_DOCUMENT_H__

G_BEGIN_DECLS

/**
   CongDocument functions
 */

#define CONG_DOCUMENT_TYPE	   (cong_document_get_type ())
#define CONG_DOCUMENT(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_DOCUMENT_TYPE, CongDocument)
#define CONG_DOCUMENT_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_DOCUMENT_TYPE, CongDocumentClass)
#define IS_CONG_DOCUMENT(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_DOCUMENT_TYPE)

typedef struct CongDocumentClass CongDocumentClass;
typedef struct CongDocumentDetails CongDocumentDetails;

struct CongDocument
{
	GObject object;

	CongDocumentDetails *private;
};

struct CongDocumentClass
{
	GObjectClass klass;

	/* Methods? */
	void (*begin_edit) (CongDocument *doc);

	void (*end_edit) (CongDocument *doc);

	void (*node_make_orphan) (CongDocument *doc, 
				  CongNodePtr node);

	void (*node_add_after) (CongDocument *doc, 
			   CongNodePtr node, 
			   CongNodePtr older_sibling);

	void (*node_add_before) (CongDocument *doc, 
			    CongNodePtr node, 
			    CongNodePtr younger_sibling);

	void (*node_set_parent) (CongDocument *doc, 
			    CongNodePtr node, 
			    CongNodePtr adoptive_parent); /* added to end of child list */

	void (*node_set_text) (CongDocument *doc, 
			  CongNodePtr node, 
			  const xmlChar *new_content);

	void (*node_set_attribute) (CongDocument *doc, 
				    CongNodePtr node, 
				    const xmlChar *name, 
				    const xmlChar *value);

	void (*node_remove_attribute) (CongDocument *doc, 
				       CongNodePtr node, 
				       const xmlChar *name);
	
	void (*selection_change) (CongDocument *doc);

	void (*cursor_change) (CongDocument *doc);
};

GType
cong_document_get_type (void);

CongDocument*
cong_document_construct (CongDocument *doc,
			 xmlDocPtr xml_doc,
			 CongDispspec *ds, 
			 const gchar *url);

/* takes ownership of xml_doc; the new CongDocument is created with a reference count of 1; any views constructed of the document will increment its reference count */
CongDocument*
cong_document_new_from_xmldoc (xmlDocPtr xml_doc, 
			       CongDispspec *ds, 
			       const gchar *url);

#if 0
/* FIXME: eventually this will be a GObject, and the ref/unref functions will be redundant: */
void
cong_document_ref(CongDocument *doc);

void
cong_document_unref(CongDocument *doc);
#endif

xmlDocPtr
cong_document_get_xml(CongDocument *doc);

CongNodePtr
cong_document_get_root(CongDocument *doc);

CongDispspec*
cong_document_get_dispspec(CongDocument *doc);

CongDispspecElement*
cong_document_get_dispspec_element_for_node(CongDocument *doc, CongNodePtr node);

gchar*
cong_document_get_filename(CongDocument *doc);
/* caller is responsible for freeeing */

gchar*
cong_document_get_full_uri(CongDocument *doc);
/* caller is responsible for freeeing */

gchar*
cong_document_get_parent_uri(CongDocument *doc);
/* caller is responsible for freeeing */

const CongXMLChar*
cong_document_get_dtd_public_identifier(CongDocument *doc);
/* NULL if not present */

xmlNsPtr
cong_document_get_nsptr (CongDocument *doc, const gchar* xmlns);

void
cong_document_save(CongDocument *doc, 
		   const char* filename,
		   GtkWindow *parent_window);

gboolean
cong_document_is_modified(CongDocument *doc);

void
cong_document_set_modified(CongDocument *doc, gboolean modified);

void
cong_document_set_primary_window(CongDocument *doc, CongPrimaryWindow *window);

void 
cong_document_set_url(CongDocument *doc, const gchar *url);

glong
cong_document_get_seconds_since_last_save_or_load(const CongDocument *doc);

/* Statistics about the document.  These are cached internally */
guint
cong_document_get_num_nodes (CongDocument *doc);

/** 
 * Update amortisation
 *
 * When making a large number of changes to a document, you can wrap them inside a begin_edit/end_edit pair
 * Begin/End edits can be nested; the document keeps track of the depth.
 * Views are notified when editing actually begins/ends; they are only notified on the outermost level of nesting.
 */
void cong_document_begin_edit(CongDocument *doc);
void cong_document_end_edit(CongDocument *doc);
gboolean cong_document_is_within_edit(CongDocument *doc);

/* MVC: Change signals on the document: *
void cong_document_node_make_orphan(CongDocument *doc, CongNodePtr node);
void cong_document_node_add_after(CongDocument *doc, CongNodePtr node, CongNodePtr older_sibling);
void cong_document_node_add_before(CongDocument *doc, CongNodePtr node, CongNodePtr younger_sibling);
void cong_document_node_set_parent(CongDocument *doc, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
void cong_document_node_set_text(CongDocument *doc, CongNodePtr node, const xmlChar *new_content);
void cong_document_tag_remove(CongDocument *doc, CongNodePtr x);
void cong_document_node_set_attribute(CongDocument *doc, CongNodePtr node, const xmlChar *name, const xmlChar *value);
void cong_document_node_remove_attribute(CongDocument *doc, CongNodePtr node, const xmlChar *name);
void cong_document_on_selection_change(CongDocument *doc);
void cong_document_on_cursor_change(CongDocument *doc);

/* These functions internally ref and unref the document:, as well as adding the view to the doc's list */
void cong_document_register_view(CongDocument *doc, CongView *view);
void cong_document_unregister_view(CongDocument *doc, CongView *view);

/* cursor and selections are now properties of the document: */
CongCursor* cong_document_get_cursor(CongDocument *doc);
CongSelection* cong_document_get_selection(CongDocument *doc);

/** 
    Every node within a document has an implied language, although the logic to determine 
    this may depend on the dispspec, on attributes of the nodes, and perhaps even on user prefererences (and hence the value may change unexpectedly): 

    FIXME: what are the ownership/ref count semantics of this function?
*/
PangoLanguage*
cong_document_get_language_for_node(CongDocument *doc, 
				    CongNodePtr node);

/* Various UI hooks: */
void cong_document_cut_selection(CongDocument *doc);
void cong_document_copy_selection(CongDocument *doc);
void cong_document_paste_selection(CongDocument *doc, GtkWidget *widget);
void cong_document_view_source(CongDocument *doc);

/* Handy ways to traverse the document, with a callback: */

/* Return TRUE to stop the traversal. */
typedef gboolean (*CongDocumentRecursionCallback)(CongDocument *doc, CongNodePtr node, gpointer user_data, guint recursion_level);

/* Return TRUE if the traversal was stopped prematurely */
gboolean cong_document_for_each_node(CongDocument *doc, CongDocumentRecursionCallback callback, gpointer callback_data);

/* Handy utilities for manipulating the document: */
void cong_document_merge_adjacent_text_nodes(CongDocument *doc);

G_END_DECLS

#endif
