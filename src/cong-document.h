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

typedef struct CongDocumentTraversal CongDocumentTraversal;
typedef struct CongDocumentTraversalClass CongDocumentTraversalClass;

typedef struct CongTraversalNode CongTraversalNode;
typedef struct CongTraversalNodeClass CongTraversalNodeClass;

typedef struct CongCommand CongCommand;
typedef struct CongCommandClass CongCommandClass;

typedef struct CongCommandHistory CongCommandHistory;
typedef struct CongCommandHistoryClass CongCommandHistoryClass;

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
				    xmlNs *ns_ptr,
				    const xmlChar *name, 
				    const xmlChar *value);

	void (*node_remove_attribute) (CongDocument *doc, 
				       CongNodePtr node, 
				       xmlNs *ns_ptr,
				       const xmlChar *name);
	
	void (*selection_change) (CongDocument *doc);

	void (*cursor_change) (CongDocument *doc);

	void (*set_dtd_ptr) (CongDocument *doc,
			     xmlDtdPtr dtd_ptr);

	void (*set_url) (CongDocument *doc,
			 const gchar *new_url);
};

GType
cong_document_get_type (void);

CongDocument*
cong_document_construct (CongDocument *doc,
			 xmlDocPtr xml_doc,
			 CongDispspec *ds, 
			 const gchar *url);

/**
 * cong_document_new_from_xmldoc:
 * @xmldoc:  The #xmlDocPtr which the #CongDocument is to wrap.  It takes ownership of it.
 * @ds:
 * @url:
 *
 * The new #CongDocument is created with a reference count of 1; 
 * any views constructed of the document will increment its reference count 
 *
 * Returns: the new #CongDocument
 */
CongDocument*
cong_document_new_from_xmldoc (xmlDocPtr xml_doc, 
			       CongDispspec *ds, 
			       const gchar *url);

/**
 * cong_document_get_xml:
 * @doc:  The #CongDocument.
 *
 * Retrieve the #xmlDocPtr wrapped by a #CongDocument.  You should not attempt to modify it directly, but instead
 * use methods of the #CongDocument
 *
 * Returns: the #xmlDocPtr wrapped by the #CongDocument
 */
xmlDocPtr
cong_document_get_xml(CongDocument *doc);

CongNodePtr
cong_document_get_root(CongDocument *doc);

/**
 * cong_document_get_root_traversal_node:
 * @doc:  The #CongDocument of interest
 *
 * The #CongDocument maintains #CongDocumentTraversal corresponding to a depth-first traversal of its xml tree,
 * but with the entity references having only the entity definition as their sole child.
 *
 * Returns: the #CongDocumentTraversal owned by the document
 */
CongDocumentTraversal*
cong_document_get_traversal (CongDocument *doc);


/**
 * cong_document_get_root_traversal_node:
 * @doc:  The #CongDocument of interest
 *
 * The #CongDocument maintains a tree of #CongTraversalNode objects corresponding to a depth-first traversal of its xml tree,
 * but with the entity references having only the entity definition as their sole child.
 *
 * Returns: the #CongTraversalNode corresponding to the root xml node of the document
 */
CongTraversalNode*
cong_document_get_root_traversal_node (CongDocument *doc);

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

/**
 * cong_document_get_xml_ns:
 * @doc:  The #CongDocument of interest
 * @ns_uri:  The URI of the namespace
 *
 * Locates a namespace by URI within a CongDocument
 *
 * Returns: the #xmlNsPtr for the namespace, or NULL if not found
 */
xmlNsPtr
cong_document_get_xml_ns (CongDocument *doc, 
			  const gchar* ns_uri);

#if 0
xmlNsPtr
cong_document_get_nsptr (CongDocument *doc, const gchar* xmlns);
#endif

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

/**
 * cong_document_get_node_name
 * @doc: 
 * @node:
 * 
 * Generate a user-visible, translated string that is a name for this node.
 * 
 * Returns:  the node name, which must be deleted by the caller.
 */
gchar*
cong_document_get_node_name (CongDocument *doc, 
			     CongNodePtr node);

/* Statistics about the document.  These are cached internally */
guint
cong_document_get_num_nodes (CongDocument *doc);

/**
 * cong_document_node_ref:
 * @doc:
 * @node:
 * 
 * Add an "external reference" to the node; it will not be deleted until it has been both removed from the tree
 * AND has lost all external references.
 *
 */
void 
cong_document_node_ref (CongDocument *doc,
			CongNodePtr node);

/**
 * cong_document_node_unref:
 * @doc:
 * @node:
 * 
 * Remove an "external reference" to the node; it will be deleted if it has been both removed from the tree
 * AND this was its last external reference.
 *
 */
void 
cong_document_node_unref (CongDocument *doc,
			  CongNodePtr node);

/**
 * cong_document_set_with_ref:
 * @doc:
 * @node_ptr: A pointer to a #CongNodePtr
 * @node: a ptr to a node, or NULL
 * 
 * Sets @node_ptr to @node, doing any necessary reference count modifications to the old and new value
 *
 */
void
cong_document_set_with_ref (CongDocument *doc,
			    CongNodePtr *node_ptr,
			    CongNodePtr node);

/**
 * cong_document_begin_command:
 * @doc: The #CongDocument upon which the command is to act.
 * @description: Human-readable, translated name for this command, as it will appear in the undo/redo history
 * widget
 * @consolidation_id: A string ID (or NULL) for this command to allow multiple similar commands to be consolidated into 
 * a single command.  For example, multiple characters being typed at the keboard can be merged into a single "Typing" command.
 * 
 * Begins creating a named command which will act upon the document.  You can then add modifications to this command,
 * and then finish with a call to cong_document_end_command()
 *
 * Returns:  a #CongCommand to which modifications should be added
 *
 */
CongCommand*
cong_document_begin_command (CongDocument *doc,
			     const gchar *description,
			     const gchar *consolidation_id);

/**
 * cong_document_end_command:
 * @doc: The #CongDocument upon which the command acted.
 * @cmd: The #CongCommand which is now complete
 * 
 * Finish creating a command which has acted upon the document.  You should have created this command using a call
 * to cong_document_begin_command()
 *
 */
void
cong_document_end_command (CongDocument *doc,
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
void cong_document_private_node_set_parent(CongDocument *doc, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
void cong_document_private_node_set_text(CongDocument *doc, CongNodePtr node, const xmlChar *new_content);
void cong_document_private_tag_remove(CongDocument *doc, CongNodePtr x);
void cong_document_private_node_set_attribute(CongDocument *doc, CongNodePtr node,  xmlNs *ns_ptr, const xmlChar *name, const xmlChar *value);
void cong_document_private_node_remove_attribute(CongDocument *doc, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name);
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


/** 
 * cong_document_get_selected_node
 * @doc: the document
 * 
 * Convenience wrapper around CongSelection for dealing with selection of specific nodes (as opposed to text ranges).
 * 
 * Returns: If a specific node is selected, returns that node.  If a text range is selected instead, or there is no selection, NULL is returned.
 * 
 */
CongNodePtr
cong_document_get_selected_node (CongDocument *doc);


/** 
 * cong_document_get_language_for_node
 * @doc:
 * @node:
 * 
 * Every node within a document has an implied language, although the logic to determine 
 * this may depend on the dispspec, on attributes of the nodes, and perhaps even on user prefererences (and hence the value may change unexpectedly).
 * 
 * FIXME: what are the ownership/ref count semantics of this function?
 * 
 * Returns:
 */
PangoLanguage*
cong_document_get_language_for_node(CongDocument *doc, 
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

void 
cong_document_paste_clipboard_or_selection (CongDocument *doc, 
					    GtkWidget *widget);

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


/**
 * cong_document_get_dtd_element
 * @cong_doc:
 * @node:
 * 
 * Helper function to get the xmlElementPtr within the DTD for a node
 * Currently only looks at the actual DTD, but in future might attempt
 * to use the dispspec to infer an DTD and return that
 * So don't store the return value; it might get deleted etc
 * 
 * Returns:
 */
xmlElementPtr
cong_document_get_dtd_element (CongDocument *cong_doc, 
			       CongNodePtr node);


GList* 
cong_document_get_valid_new_child_elements (CongDocument *doc,
					    CongNodePtr node, 
					    enum CongElementType tag_type);
GList* 
cong_document_get_valid_new_previous_sibling_elements (CongDocument *doc,
						       CongNodePtr node, 
						       enum CongElementType tag_type);
GList* 
cong_document_get_valid_new_next_sibling_elements (CongDocument* doc, 
						   CongNodePtr node, 
						   enum CongElementType tag_type);

/**
 * cong_document_node_can_be_deleted:
 * @cong_doc:
 * @node:
 * 
 * Helper function  that determines if a node can safely be deleted.
 * It will fail if the node or its subtree contains the cursor or the start/end of the selection,
 * to avoid these containing dangling pointers to dead memory.
 * Used to fix bug #108530, to ensure that the cursor and selection are always moved out of the way.
 *
 * Returns: TRUE if the node can be safely deleted, FALSE otherwise
 * 
 */
gboolean
cong_document_node_can_be_deleted (CongDocument *doc,
				   CongNodePtr node);

G_END_DECLS

#endif
