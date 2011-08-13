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

#include "cong-dispspec-element.h"

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

typedef struct _CongDocumentClass CongDocumentClass;
typedef struct _CongDocumentDetails CongDocumentDetails;

typedef struct _CongDocumentTraversal CongDocumentTraversal;
typedef struct _CongDocumentTraversalClass CongDocumentTraversalClass;

typedef struct _CongTraversalNode CongTraversalNode;
typedef struct _CongTraversalNodeClass CongTraversalNodeClass;

typedef struct _CongCommand CongCommand;
typedef struct _CongCommandClass CongCommandClass;

typedef struct _CongCommandHistory CongCommandHistory;
typedef struct _CongCommandHistoryClass CongCommandHistoryClass;

typedef struct _CongFindDialogData CongFindDialogData;

struct _CongDocument
{
	GObject object;

	CongDocumentDetails *private;
};

struct _CongDocumentClass
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
				 CongNodePtr adoptive_parent,
				 gboolean add_to_end);

	void (*node_set_text) (CongDocument *doc, 
			  CongNodePtr node, 
			  const gchar *new_content);

	void (*node_set_attribute) (CongDocument *doc, 
				    CongNodePtr node, 
				    xmlNs *ns_ptr,
				    const gchar *name, 
				    const gchar *value);

	void (*node_remove_attribute) (CongDocument *doc, 
				       CongNodePtr node, 
				       xmlNs *ns_ptr,
				       const gchar *name);
	
	void (*selection_change) (CongDocument *doc);

	void (*cursor_change) (CongDocument *doc);

	void (*set_dtd_ptr) (CongDocument *doc,
			     xmlDtdPtr dtd_ptr);

	void (*set_file) (CongDocument *doc,
			  GFile *new_file);
};

GType
cong_document_get_type (void);

CongDocument*
cong_document_construct (CongDocument *doc,
			 xmlDocPtr xml_doc,
			 CongDispspec *ds, 
			 GFile *file);

CongDocument*
cong_document_new_from_xmldoc (xmlDocPtr xml_doc, 
			       CongDispspec *ds, 
			       GFile *file);

xmlDocPtr
cong_document_get_xml(CongDocument *doc);

CongNodePtr
cong_document_get_root_element(CongDocument *doc);

CongDocumentTraversal*
cong_document_get_traversal (CongDocument *doc);


CongTraversalNode*
cong_document_get_root_traversal_node (CongDocument *doc);

CongFindDialogData *
cong_document_get_find_dialog_data  (CongDocument *doc);

CongDispspec*
cong_document_get_default_dispspec(CongDocument *doc);

CongDispspec*
cong_document_get_root_dispspec(CongDocument *doc);

CongDispspec*
cong_document_get_dtd_dispspec(CongDocument *doc);

CongDispspecElement*
cong_document_get_dispspec_element_for_node(CongDocument *doc, CongNodePtr node);

CongDispspec*
cong_document_get_dispspec_for_node(CongDocument *doc, CongNodePtr node);

gchar*
cong_document_get_filename(CongDocument *doc);
/* caller is responsible for freeeing */

GFile *
cong_document_get_file(CongDocument *doc);
/* caller is responsible for unreffing */

GFile *
cong_document_get_parent(CongDocument *doc);
/* caller is responsible for unreffing */

const gchar*
cong_document_get_dtd_public_identifier(CongDocument *doc);
/* NULL if not present */

xmlNsPtr
cong_document_get_xml_ns (CongDocument *doc, 
			  const gchar* ns_uri);

#if 0
xmlNsPtr
cong_document_get_nsptr (CongDocument *doc, const gchar* xmlns);
#endif

void
cong_document_save(CongDocument *doc, 
		   GFile *file,
		   GtkWindow *parent_window);

gboolean
cong_document_is_modified(CongDocument *doc);

void
cong_document_set_modified(CongDocument *doc, gboolean modified);

void
cong_document_set_primary_window(CongDocument *doc, CongPrimaryWindow *window);

CongPrimaryWindow*
cong_document_get_primary_window(CongDocument *doc);

void 
cong_document_set_file(CongDocument *doc, GFile *file);

glong
cong_document_get_seconds_since_last_save_or_load(const CongDocument *doc);

gchar*
cong_document_get_node_name (CongDocument *doc, 
			     CongNodePtr node);

/* Statistics about the document.  These are cached internally */
guint
cong_document_get_num_nodes (CongDocument *doc);

void 
cong_document_node_ref (CongDocument *doc,
			CongNodePtr node);

void 
cong_document_node_unref (CongDocument *doc,
			  CongNodePtr node);
			  
CongCommand*
cong_document_begin_command (CongDocument *doc,
			     const gchar *description,
			     const gchar *consolidation_id);

void
cong_document_end_command (CongDocument *doc,
			   CongCommand *cmd);

void
cong_document_abort_command (CongDocument *doc,
		     	      CongCommand *cmd);

void
cong_document_end_preprocessor_command (CongDocument *doc,
					CongCommand *cmd);

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

/* MVC: Atomic change signals on the document; only CongModification subclasses should call these directly: */
void cong_document_private_node_make_orphan(CongDocument *doc, CongNodePtr node);
void cong_document_private_node_add_after(CongDocument *doc, CongNodePtr node, CongNodePtr older_sibling);
void cong_document_private_node_add_before(CongDocument *doc, CongNodePtr node, CongNodePtr younger_sibling);
void cong_document_private_node_set_parent (CongDocument *doc, 
					    CongNodePtr node, 
					    CongNodePtr adoptive_parent,
					    gboolean add_to_end); /* adds to either the end (TRUE) or the start (FALSE) */
void cong_document_private_node_set_text(CongDocument *doc, CongNodePtr node, const gchar *new_content);
void cong_document_private_tag_remove(CongDocument *doc, CongNodePtr x);
void cong_document_private_node_set_attribute(CongDocument *doc, CongNodePtr node,  xmlNs *ns_ptr, const gchar *name, const gchar *value);
void cong_document_private_node_remove_attribute(CongDocument *doc, CongNodePtr node, xmlNs *ns_ptr, const gchar *name);
void cong_document_private_on_selection_change(CongDocument *doc);
void cong_document_private_on_cursor_change(CongDocument *doc);
void 
cong_document_private_set_dtd_ptr (CongDocument *doc,
				   xmlDtdPtr dtd_ptr);

/* These functions internally ref and unref the document:, as well as adding the view to the doc's list */
void cong_document_register_view(CongDocument *doc, CongView *view);
void cong_document_unregister_view(CongDocument *doc, CongView *view);

/* cursor and selections are now properties of the document: */
CongCursor* cong_document_get_cursor(CongDocument *doc);
CongSelection* cong_document_get_selection(CongDocument *doc);


CongNodePtr
cong_document_get_selected_node (CongDocument *doc);


PangoLanguage*
cong_document_get_language_for_node(CongDocument *doc, 
				    CongNodePtr node);

void 
cong_document_make_pango_log_attr_for_node (CongDocument *doc,
					    CongNodePtr node,
					    PangoLogAttr **pango_log_attrs,
					    int *attrs_len);

gboolean
cong_document_should_spellcheck_node (CongDocument *doc, 
				      CongNodePtr node);

CongCommandHistory*
cong_document_get_command_history (CongDocument *doc);

/* Various UI hooks: */
void
cong_document_undo (CongDocument *doc);

void
cong_document_redo (CongDocument *doc);

void
cong_document_select_node (CongDocument *doc,
			   CongNodePtr node);

void
cong_document_cut_selection (CongDocument *doc);

void
cong_document_copy_selection (CongDocument *doc);

gboolean
cong_document_can_paste (CongDocument *doc);

gboolean
cong_document_can_export (CongDocument *doc);

#if ENABLE_PRINTING
gboolean
cong_document_can_print (CongDocument *doc);
#endif

void 
cong_document_paste_clipboard (CongDocument *doc);

void
cong_document_paste_source_at (CongDocument *doc, 
			       CongLocation *insert_loc, 
			       const gchar *source_fragment);

void
cong_document_paste_source_under (CongDocument *doc, 
				  CongNodePtr relative_to_node,
				  const gchar *source_fragment);

void
cong_document_paste_source_before (CongDocument *doc, 
				   CongNodePtr relative_to_node,
				   const gchar *source_fragment);

void
cong_document_paste_source_after (CongDocument *doc, 
				  CongNodePtr relative_to_node,
				  const gchar *source_fragment);

void
cong_document_view_source (CongDocument *doc);

/* Handy ways to traverse the document, with a callback: */

/* Return TRUE to stop the traversal. */
typedef gboolean (*CongDocumentRecursionCallback)(CongDocument *doc, CongNodePtr node, gpointer user_data, guint recursion_level);

/* Return TRUE if the traversal was stopped prematurely */
gboolean
cong_document_for_each_node (CongDocument *doc, 
			     CongDocumentRecursionCallback callback, 
			     gpointer callback_data);

/* This one doesn't recurse; it merely visits the direct dsescendents of parent */
gboolean 
cong_document_for_each_child_of_node (CongDocument *doc, 
				      CongNodePtr parent, 
				      CongDocumentRecursionCallback callback, 
				      gpointer callback_data);

/* Tries to parse the source fragment, places the result under a <placeholder> tag, which is returned */
CongNodePtr
cong_document_make_nodes_from_source_fragment (CongDocument *doc, 
					       const gchar *source_fragment);


xmlElementPtr
cong_document_get_dtd_element (CongDocument *cong_doc, 
			       CongNodePtr node);


GList* 
cong_document_get_valid_new_child_elements (CongDocument *doc,
					    CongNodePtr node, 
					    CongElementType tag_type);
GList* 
cong_document_get_valid_new_previous_sibling_elements (CongDocument *doc,
						       CongNodePtr node, 
						       CongElementType tag_type);
GList* 
cong_document_get_valid_new_next_sibling_elements (CongDocument* doc, 
						   CongNodePtr node, 
						   CongElementType tag_type);

gboolean
cong_document_node_can_be_deleted (CongDocument *doc,
				   CongNodePtr node);

G_END_DECLS

#endif
