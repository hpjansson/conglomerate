/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"
#include "cong-document.h"
#include "cong-view.h"
#include "cong-error-dialog.h"
#include "cong-dispspec.h"
#include "cong-dispspec-element.h"
#include "cong-selection.h"
#include <libgnome/gnome-macros.h>
#include "cong-util.h"
#include "cong-primary-window.h"
#include "cong-command.h"
#include "cong-command-history.h"
#include "cong-marshal.h"
#include "cong-vfs.h"
#include "cong-app.h"
#include "cong-document-traversal.h"
#include "cong-edit-find-and-replace.h"
#include "cong-dispspec-registry.h"

#if ENABLE_PRINTING
#include "cong-service-print-method.h"
#include "cong-plugin-manager.h"
#endif 

/* Internal functions: */
static void
cong_document_finalize (GObject *object);

static void
cong_document_dispose (GObject *object);

/* Default signal handlers: */
static void 
cong_document_handle_begin_edit(CongDocument *doc);

static void 
cong_document_handle_end_edit(CongDocument *doc);

static void 
cong_document_handle_node_make_orphan(CongDocument *doc, CongNodePtr node);

static void
cong_document_handle_node_add_after(CongDocument *doc, CongNodePtr node, CongNodePtr older_sibling);

static void
cong_document_handle_node_add_before(CongDocument *doc, CongNodePtr node, CongNodePtr younger_sibling);

static void
cong_document_handle_node_set_parent(CongDocument *doc, CongNodePtr node, CongNodePtr adoptive_parent, gboolean add_to_end);

static void
cong_document_handle_node_set_text(CongDocument *doc, CongNodePtr node, const xmlChar *new_content);

static void
cong_document_handle_node_set_attribute(CongDocument *doc, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name, const xmlChar *value);

static void
cong_document_handle_node_remove_attribute(CongDocument *doc, CongNodePtr node, xmlNs *ns_ptr, const xmlChar *name);

static void
cong_document_handle_selection_change(CongDocument *doc);

static void
cong_document_handle_cursor_change(CongDocument *doc);

static void
cong_document_handle_set_dtd_ptr (CongDocument *doc,
				  xmlDtdPtr dtd_ptr);

static void
cong_document_handle_set_url (CongDocument *doc,
			      const gchar *new_url);

#define TEST_VIEW 0
#define TEST_EDITOR_VIEW 0
#define DEBUG_MVC 0
#define LOG_TRAVERSAL_NODES 1

#if LOG_TRAVERSAL_NODES
#define LOG_TRAVERSAL_NODE1(x) g_message(x)
#define LOG_TRAVERSAL_NODE2(x, a) g_message((x), (a))
#else
#define LOG_TRAVERSAL_NODE1(x) ((void)0)
#define LOG_TRAVERSAL_NODE2(x, a) ((void)0)
#endif

#undef PRIVATE
#define PRIVATE(x) ((x)->private)

enum {
	BEGIN_EDIT,
	END_EDIT,
	NODE_MAKE_ORPHAN,
	NODE_ADD_AFTER,
	NODE_ADD_BEFORE,
	NODE_SET_PARENT,
	NODE_SET_TEXT,
	NODE_SET_ATTRIBUTE,
	NODE_REMOVE_ATTRIBUTE,
	SELECTION_CHANGE,
	CURSOR_CHANGE,
	SET_DTD_PTR,
	SET_URL,

	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

struct CongDocumentDetails
{
	xmlDocPtr xml_doc;

	CongDispspec *default_ds;

	CongDocumentTraversal *traversal;

	gchar *url;

	GList *views; /* a list of CongView* */

	/* cursor and selections are now properties of the document: */
	CongCursor cursor;
	CongSelection *selection;

	gboolean modified; /* has the document been modified since it was last loaded/saved? */
	GTimeVal time_of_last_save;

	CongCommandHistory *history;
	CongCommand *current_command;

	/* We have an SDI interface, so there should be just one primary window associated with each doc.
	   Knowing this lets us update the window title when it changes (eventually do as a signal on the document).
	*/
	CongPrimaryWindow *primary_window;

	/* Amortisation of updates: */
	guint edit_depth;
	gboolean selection_has_changed;

	/* Stats about the document: */
	gboolean num_nodes_valid;	
	guint num_nodes;
	
	/* Search data */
	CongFindDialogData *find_data;
};

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongDocument, 
			cong_document,
			GObject,
			G_TYPE_OBJECT );

static void
cong_document_class_init (CongDocumentClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = cong_document_finalize;
	G_OBJECT_CLASS (klass)->dispose = cong_document_dispose;

	/* Set up default handlers: */
	klass->begin_edit = cong_document_handle_begin_edit;
	klass->end_edit = cong_document_handle_end_edit;
	klass->node_make_orphan = cong_document_handle_node_make_orphan;
	klass->node_add_after = cong_document_handle_node_add_after;
	klass->node_add_before = cong_document_handle_node_add_before;
	klass->node_set_parent = cong_document_handle_node_set_parent;
	klass->node_set_text = cong_document_handle_node_set_text;
	klass->node_set_attribute = cong_document_handle_node_set_attribute;
	klass->node_remove_attribute = cong_document_handle_node_remove_attribute;
	klass->selection_change = cong_document_handle_selection_change;
	klass->cursor_change = cong_document_handle_cursor_change;
	klass->set_dtd_ptr = cong_document_handle_set_dtd_ptr;
	klass->set_url = cong_document_handle_set_url;

	/* Set up the various signals: */
	signals[BEGIN_EDIT] = g_signal_new ("begin_edit",
					    CONG_DOCUMENT_TYPE,
					    G_SIGNAL_RUN_LAST,
					    G_STRUCT_OFFSET(CongDocumentClass, begin_edit),
					    NULL, NULL,
					    g_cclosure_marshal_VOID__VOID,
					    G_TYPE_NONE, 
					    0);
	
	signals[END_EDIT] = g_signal_new ("end_edit",
					  CONG_DOCUMENT_TYPE,
					  G_SIGNAL_RUN_LAST,
					  G_STRUCT_OFFSET(CongDocumentClass, end_edit),
					  NULL, NULL,
					  g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 
					  0);
	
	signals[NODE_MAKE_ORPHAN] = g_signal_new ("node_make_orphan",
						  CONG_DOCUMENT_TYPE,
						  G_SIGNAL_RUN_LAST,
						  G_STRUCT_OFFSET(CongDocumentClass, node_make_orphan),
						  NULL, NULL,
						  cong_cclosure_marshal_VOID__POINTER,
						  G_TYPE_NONE, 
						  1, G_TYPE_POINTER);
	
	signals[NODE_ADD_AFTER] = g_signal_new ("node_add_after",
						CONG_DOCUMENT_TYPE,
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(CongDocumentClass, node_add_after),
						NULL, NULL,
						cong_cclosure_marshal_VOID__POINTER_POINTER,
						G_TYPE_NONE, 
						2, G_TYPE_POINTER, G_TYPE_POINTER);
	
	signals[NODE_ADD_BEFORE] = g_signal_new ("node_add_before",
						 CONG_DOCUMENT_TYPE,
						 G_SIGNAL_RUN_LAST,
						 G_STRUCT_OFFSET(CongDocumentClass, node_add_before),
						 NULL, NULL,
						 cong_cclosure_marshal_VOID__POINTER_POINTER,
						 G_TYPE_NONE, 
						 2, G_TYPE_POINTER, G_TYPE_POINTER);
	
	signals[NODE_SET_PARENT] = g_signal_new ("node_set_parent",
						 CONG_DOCUMENT_TYPE,
						 G_SIGNAL_RUN_LAST,
						 G_STRUCT_OFFSET(CongDocumentClass, node_set_parent),
						 NULL, NULL,
						 cong_cclosure_marshal_VOID__POINTER_POINTER_BOOLEAN,
						 G_TYPE_NONE, 
						 3, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_BOOLEAN);
	
	signals[NODE_SET_TEXT] = g_signal_new ("node_set_text",
					       CONG_DOCUMENT_TYPE,
					       G_SIGNAL_RUN_LAST,
					       G_STRUCT_OFFSET(CongDocumentClass, node_set_text),
					       NULL, NULL,
					       cong_cclosure_marshal_VOID__POINTER_STRING,
					       G_TYPE_NONE, 
					       2, G_TYPE_POINTER, G_TYPE_STRING);
	

	signals[NODE_SET_ATTRIBUTE] = g_signal_new ("node_set_attribute",
						    CONG_DOCUMENT_TYPE,
						    G_SIGNAL_RUN_LAST,
						    G_STRUCT_OFFSET(CongDocumentClass, node_set_attribute),
						    NULL, NULL,
						    cong_cclosure_marshal_VOID__POINTER_POINTER_STRING_STRING,
						    G_TYPE_NONE, 
						    4, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING);

	signals[NODE_REMOVE_ATTRIBUTE] = g_signal_new ("node_remove_attribute",
						       CONG_DOCUMENT_TYPE,
						       G_SIGNAL_RUN_LAST,
						       G_STRUCT_OFFSET(CongDocumentClass, node_remove_attribute),
						       NULL, NULL,
						       cong_cclosure_marshal_VOID__POINTER_POINTER_STRING,
						       G_TYPE_NONE, 
						       3, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_STRING);

	signals[SELECTION_CHANGE] = g_signal_new ("selection_change",
						  CONG_DOCUMENT_TYPE,
						  G_SIGNAL_RUN_LAST,
						  G_STRUCT_OFFSET(CongDocumentClass, selection_change),
						  NULL, NULL,
						  g_cclosure_marshal_VOID__VOID,
						  G_TYPE_NONE, 
						  0);

	signals[CURSOR_CHANGE] = g_signal_new ("cursor_change",
					       CONG_DOCUMENT_TYPE,
					       G_SIGNAL_RUN_LAST,
					       G_STRUCT_OFFSET(CongDocumentClass, cursor_change),
					       NULL, NULL,
					       g_cclosure_marshal_VOID__VOID,
					       G_TYPE_NONE, 
					       0);
	signals[SET_DTD_PTR] = g_signal_new ("set_dtd_ptr",
					     CONG_DOCUMENT_TYPE,
					     G_SIGNAL_RUN_LAST,
					     G_STRUCT_OFFSET(CongDocumentClass, set_dtd_ptr),
					     NULL, NULL,
					     cong_cclosure_marshal_VOID__POINTER,
					     G_TYPE_NONE, 
					     1, G_TYPE_POINTER);
	signals[SET_URL] = g_signal_new ("set_url",
					 CONG_DOCUMENT_TYPE,
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(CongDocumentClass, set_url),
					 NULL, NULL,
					 g_cclosure_marshal_VOID__STRING,
					 G_TYPE_NONE, 
					 1, G_TYPE_STRING);
}

static void
cong_document_instance_init (CongDocument *doc)
{
	doc->private = g_new0(CongDocumentDetails,1);
}

#if 0
typedef void (*CongXMLSelfTestCallback)(xmlNodePtr node, const gchar *error_message);

gboolean cong_xml_selftest_node(xmlNodePtr node, CongXMLSelfTestCallback selftest_callback);
gboolean cong_xml_selftest_doc(xmlDocPtr xml_doc, CongXMLSelfTestCallback selftest_callback);

gboolean cong_xml_selftest_node(xmlNodePtr node, CongXMLSelfTestCallback selftest_callback)
{
	/* Test this node: */
	if (node->content && (node->type!=XML_ATTRIBUTE_DECL)) {
		/* g_message("testing node content\"%s\"", node->content); */
		if (!g_utf8_validate(node->content, -1, NULL)) {
			if (selftest_callback) {
				(*selftest_callback)(node, "Invalid UTF-8 data");
			}
			return FALSE;
		}
	}

	/* Recurse through children: */
	{
		xmlNodePtr iter;
		
		for (iter = node->children; iter; iter=iter->next) {
			if (!cong_xml_selftest_node(iter, selftest_callback)) {
				return FALSE;
			}
		}
	}

	/* All tests passed: */
	return TRUE;
}

gboolean cong_xml_selftest_doc(xmlDocPtr xml_doc, CongXMLSelfTestCallback selftest_callback)
{
	xmlNodePtr iter;

	g_return_val_if_fail(xml_doc, FALSE);

	/* Traverse the document: */
	for (iter = xml_doc->children; iter; iter=iter->next) {
		if (!cong_xml_selftest_node(iter, selftest_callback)) {
			return FALSE;
		}
	}
}
#endif

/**
 * cong_document_construct:
 * @doc:
 * @xml_doc:
 * @ds:
 * @url:
 *
 * TODO: Write me
 * Returns:
 */
CongDocument*
cong_document_construct (CongDocument *doc,
			 xmlDocPtr xml_doc,
			 CongDispspec *ds, 
			 const gchar *url)
{
	CongNodePtr initial_cursor_node;

	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	g_return_val_if_fail (xml_doc, NULL);

	PRIVATE(doc)->xml_doc = xml_doc;
	PRIVATE(doc)->default_ds = ds;
	PRIVATE(doc)->url = g_strdup(url);

	g_get_current_time(&PRIVATE(doc)->time_of_last_save);

	#if TEST_VIEW
	{
		GtkWindow *window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
		GtkWidget *test_view = cong_test_view_new(doc);
		gtk_container_add(GTK_CONTAINER(window), test_view);
		gtk_widget_show(GTK_WIDGET(window));		
	}
	#endif

	#if TEST_EDITOR_VIEW
	{
		GtkWindow *window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
		GtkWidget *test_editor_view = cong_editor_widget_new(doc);
		GtkWidget *scroller;

		scroller = gtk_scrolled_window_new(NULL, NULL);

		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), 
					       GTK_POLICY_AUTOMATIC,
					       GTK_POLICY_ALWAYS);

		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroller), 
						      test_editor_view);

		gtk_container_add(GTK_CONTAINER(window), scroller);

		gtk_widget_show_all(GTK_WIDGET(window));		
	}
	#endif

	cong_cursor_init(&PRIVATE(doc)->cursor, doc);
	PRIVATE(doc)->selection = cong_selection_new();

	initial_cursor_node = cong_node_get_first_text_node_descendant ((CongNodePtr)PRIVATE(doc)->xml_doc);
	
	if (initial_cursor_node) {
		cong_location_set_to_start_of_node (&PRIVATE(doc)->cursor.location,
						    initial_cursor_node);
	}

	PRIVATE(doc)->history = cong_command_history_new();

	PRIVATE(doc)->traversal = cong_document_traversal_new (doc);
	
	PRIVATE(doc)->find_data = g_new0 (CongFindDialogData, 1);

	return doc;
}

/**
 * cong_document_new_from_xmldoc:
 * @xml_doc:  The #xmlDocPtr which the #CongDocument is to wrap.  It takes ownership of it.
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
			       const gchar *url)
{
	g_return_val_if_fail(xml_doc!=NULL, NULL);
#if 0
	g_return_val_if_fail(cong_xml_selftest_doc(xml_doc, NULL), NULL);
#endif

	return cong_document_construct (g_object_new (CONG_DOCUMENT_TYPE, NULL),
					xml_doc,
					ds,
					url);
}

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
cong_document_get_xml(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return PRIVATE(doc)->xml_doc;
}

/**
 * cong_document_get_root_element:
 * @doc:
 *
 * Returns: the root element of the document, if any (as opposed to the root node)
 */
CongNodePtr
cong_document_get_root_element(CongDocument *doc)
{
	CongNodePtr iter;

	g_return_val_if_fail(doc, NULL);

	for (iter=PRIVATE(doc)->xml_doc->children; iter; iter=iter->next) {
		if (iter->type==XML_ELEMENT_NODE) {
			return iter;
		}
	}

	return NULL;
}

/**
 * cong_document_get_traversal:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
CongDocumentTraversal*
cong_document_get_traversal (CongDocument *doc)
{
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	
	return PRIVATE(doc)->traversal;
}

/**
 * cong_document_get_root_traversal_node:
 * @doc:  The #CongDocument of interest
 *
 * The #CongDocument maintains #CongDocumentTraversal corresponding to a depth-first traversal of its xml tree,
 * but with the entity references having only the entity definition as their sole child.
 *
 * Returns: the #CongDocumentTraversal owned by the document
 */
CongTraversalNode*
cong_document_get_root_traversal_node (CongDocument *doc)
{
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);

	return cong_document_traversal_get_root_traversal_node (PRIVATE(doc)->traversal);
}

/**
 * cong_document_get_default_dispspec:
 * @doc:
 *
 * Get the CongDispspec in use for all non-namespaced elements in this document
 * Returns:
 */
CongDispspec*
cong_document_get_default_dispspec(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return PRIVATE(doc)->default_ds;
}

/**
 * cong_document_get_root_dispspec:
 * @doc:
 *
 * Get the CongDispspec corresponding to the root element of the document (if any)
 * Returns:
 */
CongDispspec*
cong_document_get_root_dispspec(CongDocument *doc)
{
	CongNodePtr root_element;
	g_return_val_if_fail(doc, NULL);

	root_element = cong_document_get_root_element (doc);

	if (root_element) {
		return cong_document_get_dispspec_for_node (doc, 
							    root_element);
	} else {
		return NULL;
	}
}

/**
 * cong_document_get_dtd_dispspec:
 * @doc:
 *
 * Get the CongDispspec corresponding to the document's DTD (if any)
 * Returns:
 */
CongDispspec*
cong_document_get_dtd_dispspec(CongDocument *doc)
{
	g_return_val_if_fail (doc, NULL);

	return cong_dispspec_registry_get_dispspec_for_dtd (cong_app_get_dispspec_registry (cong_app_singleton ()),
							    PRIVATE(doc)->xml_doc);
}

/**
 * cong_document_get_dispspec_element_for_node:
 * @doc:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspecElement*
cong_document_get_dispspec_element_for_node(CongDocument *doc, CongNodePtr node)
{
	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);

	return cong_dispspec_registry_get_dispspec_element_for_node (cong_app_get_dispspec_registry (cong_app_singleton ()),
								     PRIVATE(doc)->default_ds,
								     node);
}

/**
 * cong_document_get_dispspec_for_node:
 * @doc:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspec*
cong_document_get_dispspec_for_node(CongDocument *doc, CongNodePtr node)
{
	CongDispspecElement* ds_element;

	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);

	ds_element = cong_document_get_dispspec_element_for_node (doc, node);

	if (ds_element) {
		return cong_dispspec_element_get_dispspec (ds_element);
	} else {
		return NULL;
	}
}

/**
 * cong_document_get_filename:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
gchar*
cong_document_get_filename(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	if (PRIVATE(doc)->url) {
		gchar *filename;
		gchar *path;
		GnomeVFSURI *uri = gnome_vfs_uri_new(PRIVATE(doc)->url);
		
		cong_vfs_split_vfs_uri (uri, &filename, &path);

		gnome_vfs_uri_unref(uri);

		g_free(path);
		
		return filename;

	} else {
		return g_strdup(_("(Untitled)"));
	}
}

/**
 * cong_document_get_full_uri:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
gchar*
cong_document_get_full_uri(CongDocument *doc) 
{
	g_return_val_if_fail(doc, NULL);

	if (PRIVATE(doc)->url) {
		return g_strdup(PRIVATE(doc)->url);
	}
	else {
		return NULL;
	}		    
}

/**
 * cong_document_get_parent_uri:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
gchar*
cong_document_get_parent_uri(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	if (PRIVATE(doc)->url) {
		gchar *filename;
		gchar *path;
		GnomeVFSURI *uri = gnome_vfs_uri_new(PRIVATE(doc)->url);
		
		cong_vfs_split_vfs_uri (uri, &filename, &path);

		gnome_vfs_uri_unref(uri);

		g_free(filename);
		
		return path;

	} else {
		return g_strdup(".");
	}
}

/**
 * cong_document_get_dtd_public_identifier:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
const CongXMLChar*
cong_document_get_dtd_public_identifier(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	g_assert(PRIVATE(doc)->xml_doc);

	if (NULL==PRIVATE(doc)->xml_doc->extSubset) {
		return NULL;
	}

	return PRIVATE(doc)->xml_doc->extSubset->ExternalID;
}

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
			  const gchar* ns_uri)
{
	xmlNsPtr ns;

	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	g_return_val_if_fail (ns_uri, NULL);

	ns = xmlSearchNsByHref (PRIVATE(doc)->xml_doc,
				(xmlNodePtr)PRIVATE(doc)->xml_doc, /* FIXME: is this correct? */
				ns_uri);

	return ns;
}

/**
 * cong_document_save:
 * @doc:
 * @filename:
 * @parent_window:
 *
 * TODO: Write me
 */
void
cong_document_save(CongDocument *doc, 
		   const char* filename, 
		   GtkWindow *toplevel_window)
{

	GnomeVFSURI *file_uri;
	GnomeVFSResult vfs_result;
	GnomeVFSFileSize file_size;

	g_return_if_fail(doc);
	g_return_if_fail(filename);

	if (!g_path_is_absolute (filename) && !(g_str_has_prefix (filename, "file:"))) {

		gchar *absolute_path = g_strconcat (g_get_current_dir(), GNOME_VFS_URI_PATH_STR, filename, NULL);
    		file_uri = gnome_vfs_uri_new (absolute_path);
		g_free (absolute_path);
	} else {
	file_uri = gnome_vfs_uri_new(filename);
	}
	
	vfs_result = cong_vfs_save_xml_to_uri (PRIVATE(doc)->xml_doc, 
					       file_uri,	
					       &file_size);

	if (vfs_result != GNOME_VFS_OK) {
		GtkDialog* dialog = cong_error_dialog_new_from_file_save_failure (toplevel_window,
										  filename, 
										  vfs_result, 
										  &file_size);
			
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));

		gnome_vfs_uri_unref(file_uri);

		return;
	}

	cong_document_set_url(doc, filename);

	cong_document_set_modified(doc, FALSE);

	g_get_current_time(&PRIVATE(doc)->time_of_last_save);

	gnome_vfs_uri_unref(file_uri);
}

/**
 * cong_document_is_modified:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_document_is_modified(CongDocument *doc)
{
	g_return_val_if_fail(doc, FALSE);

	return PRIVATE(doc)->modified;
}

/**
 * cong_document_set_modified:
 * @doc:
 * @modified:
 *
 * TODO: Write me
 */
void
cong_document_set_modified(CongDocument *doc, gboolean modified)
{
	g_return_if_fail(doc);

	if (PRIVATE(doc)->modified != modified) {

		PRIVATE(doc)->modified = modified;

		/* get at primary window; set title */
		if (PRIVATE(doc)->primary_window) {
			cong_primary_window_update_title(PRIVATE(doc)->primary_window);
		}
	}
}

/**
 * cong_document_get_primary_window:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
CongPrimaryWindow*
cong_document_get_primary_window(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);
	return PRIVATE(doc)->primary_window;
}

/**
 * cong_document_set_primary_window:
 * @doc:
 * @window:
 *
 * TODO: Write me
 */
void
cong_document_set_primary_window(CongDocument *doc, CongPrimaryWindow *window)
{
	g_return_if_fail(doc);
	g_return_if_fail(window);

	g_assert(PRIVATE(doc)->primary_window==NULL);
	PRIVATE(doc)->primary_window = window;
}

/**
 * cong_document_set_url:
 * @doc:
 * @url:
 *
 * TODO: Write me
 */
void 
cong_document_set_url(CongDocument *doc, const gchar *url) 
{
	g_return_if_fail (IS_CONG_DOCUMENT (doc));
	g_return_if_fail (url);

	g_signal_emit (G_OBJECT(doc),
		       signals[SET_URL], 0,
		       url);
}

/**
 * cong_document_get_seconds_since_last_save_or_load:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
glong
cong_document_get_seconds_since_last_save_or_load(const CongDocument *doc)
{
	GTimeVal current_time;

	g_return_val_if_fail(doc, 0);

	g_get_current_time(&current_time);

	return current_time.tv_sec - PRIVATE(doc)->time_of_last_save.tv_sec;
}

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
			     CongNodePtr node)
{

	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	g_return_val_if_fail (node, NULL);

	switch (cong_node_type (node)) {
	default: g_assert_not_reached();
	case CONG_NODE_TYPE_ELEMENT:
		{
			CongDispspecElement *ds_element = cong_document_get_dispspec_element_for_node (doc,
												       node);
			if (ds_element) {
				return g_strdup (cong_dispspec_element_username (ds_element));
			} else {
				if (cong_node_get_ns_prefix (node)) {
					return g_strdup_printf ("<%s:%s>", cong_node_get_ns_prefix (node), node->name);
				} else {
					return g_strdup_printf ("<%s>", node->name);
				}
			}
		}
		break;

	case CONG_NODE_TYPE_ATTRIBUTE:
		return g_strdup_printf ( _("Attribute \"%s\""), node->name);

	case CONG_NODE_TYPE_TEXT:
		{
			#define TRUNCATION_LENGTH 15

			gchar *content_header = cong_util_text_header (node->content, TRUNCATION_LENGTH);
			gchar *result = g_strdup_printf( _("Text \"%s\""), content_header);
			g_free (content_header);
			return result;
		}

	case CONG_NODE_TYPE_CDATA_SECTION:
		return g_strdup_printf( _("Character Data"));

	case CONG_NODE_TYPE_ENTITY_REF:
		return g_strdup_printf (_("Reference to Entity \"%s\""), node->name);

	case CONG_NODE_TYPE_ENTITY_NODE:
		return g_strdup_printf (_("Entity \"%s\""), node->name);;

	case CONG_NODE_TYPE_PI:
		return g_strdup_printf (_("XML Processing Instruction"));

	case CONG_NODE_TYPE_COMMENT:
		{
			gchar *content_header = cong_util_text_header (node->content, TRUNCATION_LENGTH);
			gchar *result = g_strdup_printf( _("Comment \"%s\""), content_header);
			g_free (content_header);
			return result;
		}

	case CONG_NODE_TYPE_DOCUMENT:
		return g_strdup_printf (_("Document"));		

	case CONG_NODE_TYPE_DOCUMENT_TYPE:
		return g_strdup_printf (_("Document Type"));

	case CONG_NODE_TYPE_DOCUMENT_FRAG:
		return g_strdup_printf (_("Document Fragment"));

	case CONG_NODE_TYPE_NOTATION:
		return g_strdup_printf (_("Notation"));

	case CONG_NODE_TYPE_HTML_DOCUMENT:
		return g_strdup_printf (_("HTML Document"));

	case CONG_NODE_TYPE_DTD:
		return g_strdup_printf (_("Document Type Declaration"));

	case CONG_NODE_TYPE_ELEMENT_DECL:
		return g_strdup_printf (_("Element Declaration"));

	case CONG_NODE_TYPE_ATRRIBUTE_DECL:
		return g_strdup_printf (_("Attribute Declaration"));

	case CONG_NODE_TYPE_ENTITY_DECL:
		return g_strdup_printf (_("Entity Declaration"));

	case CONG_NODE_TYPE_NAMESPACE_DECL:
		return g_strdup_printf (_("Namespace Declaration"));

	case CONG_NODE_TYPE_XINCLUDE_START:
		return g_strdup_printf (_("XInclude Start"));

	case CONG_NODE_TYPE_XINCLUDE_END:
		return g_strdup_printf (_("XInclude End"));
	}

	g_assert_not_reached();
}

static gboolean 
node_count_callback (CongDocument *doc, CongNodePtr node, gpointer user_data, guint recursion_level)
{
	(*((int*)user_data))++;

	return FALSE;		
}

/**
 * cong_document_get_num_nodes:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
guint
cong_document_get_num_nodes (CongDocument *doc)
{
	g_return_val_if_fail(doc, 0);

	if (!PRIVATE(doc)->num_nodes_valid) {

		PRIVATE(doc)->num_nodes = 0;
		cong_document_for_each_node(doc, node_count_callback, &(PRIVATE(doc)->num_nodes));
		PRIVATE(doc)->num_nodes_valid = TRUE;

	}
	
	return PRIVATE(doc)->num_nodes;
	
}

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
			CongNodePtr node)
{
	g_return_if_fail (IS_CONG_DOCUMENT (doc));
	g_return_if_fail (node);

	/* unwritten */
}

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
			  CongNodePtr node)
{
	g_return_if_fail (IS_CONG_DOCUMENT (doc));
	g_return_if_fail (node);

	/* unwritten */
}

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
			    CongNodePtr node)
{
	g_return_if_fail (IS_CONG_DOCUMENT (doc));
	g_return_if_fail (node_ptr);
	
	/* re the new value first in case they are equal: */
	if (node) {
		cong_document_node_ref (doc,
					node);
	}

	if (*node_ptr) {
		cong_document_node_unref (doc,
					  *node_ptr);
	}
	
	*node_ptr = node;
}

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
 */
CongCommand*
cong_document_begin_command (CongDocument *doc,
			     const gchar *description,
			     const gchar *consolidation_id)
{
	CongCommand *cmd;

	g_return_val_if_fail (IS_CONG_DOCUMENT(doc), NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (NULL==PRIVATE(doc)->current_command, NULL);

	cmd = cong_command_private_new (doc, description, consolidation_id);
	
	PRIVATE(doc)->current_command = cmd;

	cong_document_begin_edit (doc);

	return cmd;
}

static CongCommand*
should_merge_commands (CongDocument *doc,
		       CongCommand *cmd)
{
	g_return_val_if_fail (IS_CONG_DOCUMENT(doc), NULL);
	g_return_val_if_fail (IS_CONG_COMMAND(cmd), NULL);
	
	/* Merge if applicable: */
	if (cong_command_get_consolidation_id (cmd)) {
		/* If there is redo history, then we shouldn't merge: */
		if (NULL == cong_command_history_get_next_redo_command (PRIVATE(doc)->history)) {
			CongCommand *last_cmd = cong_command_history_get_next_undo_command (PRIVATE(doc)->history);

			if (last_cmd) {
				if (!cong_command_has_ever_been_undone (last_cmd)) {
					if (cong_command_get_consolidation_id (last_cmd)) {
						/* Only merge if they both have the same consolidation ID; both must be non-NULL: */
						if (0==strcmp(cong_command_get_consolidation_id (last_cmd), cong_command_get_consolidation_id (cmd))) {
							return last_cmd;
						}
					}
				}
			}
		}
	}
	
	return NULL;
}

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
			   CongCommand *cmd)
{
	CongCommand *cmd_to_merge_into;
	g_return_if_fail (IS_CONG_DOCUMENT(doc));
	g_return_if_fail (IS_CONG_COMMAND(cmd));
	g_return_if_fail (doc == cong_command_get_document (cmd));
	g_return_if_fail (cmd==PRIVATE(doc)->current_command);

	cong_document_end_edit (doc);

	PRIVATE(doc)->current_command = NULL;

	cmd_to_merge_into = should_merge_commands (doc,
						   cmd);
		
	if (cmd_to_merge_into) {
#if 0
		g_message ("Merging command \"%s\" into existing undo history", cong_command_get_description(cmd));
#endif

		cong_command_merge (cmd_to_merge_into,
				    cmd);
	} else {
		cong_command_history_add_command (PRIVATE(doc)->history,
						  cmd);
	}

	g_object_unref (G_OBJECT (cmd));
}

/**
 * cong_document_abort_command:
 * @doc: The #CongDocument upon which the command acted.
 * @cmd: The #CongCommand which should be aborted
 * 
 * Finish command without applying it to document
 */
void
cong_document_abort_command (CongDocument *doc,
		     	      CongCommand *cmd)
{
	g_return_if_fail (IS_CONG_DOCUMENT(doc));
	g_return_if_fail (IS_CONG_COMMAND(cmd));
	g_return_if_fail (doc == cong_command_get_document (cmd));
	g_return_if_fail (cmd==PRIVATE(doc)->current_command);

	cong_command_undo (cmd);

	PRIVATE(doc)->current_command = NULL;

	g_object_unref (G_OBJECT (cmd));

	cong_document_end_edit (doc);
}			      

/* Public MVC hooks: */
/**
 * cong_document_begin_edit:
 * @doc:
 *
 * TODO: Write me
 */
void 
cong_document_begin_edit (CongDocument *doc)
{
	g_return_if_fail (doc);

	PRIVATE(doc)->edit_depth++;

	/* If we've just started the outermost level of a series of nested edits, then notify all the views: */
	if (1 == PRIVATE(doc)->edit_depth) {

		/* Emit signal: */
		g_signal_emit (G_OBJECT(doc),
			       signals[BEGIN_EDIT], 0);
			       
	}
}

/**
 * cong_document_end_edit:
 * @doc:
 *
 * TODO: Write me
 */
void 
cong_document_end_edit (CongDocument *doc)
{
	g_return_if_fail (doc);

	g_assert(PRIVATE(doc)->edit_depth>0 && "If this assertion fails, then there is a mismatched pair of begin/end edit calls on the document");

	PRIVATE(doc)->edit_depth--;

	/* If we've just finished the outermost level of a series of nested edits, then notify all the views: */
	if (0 == PRIVATE(doc)->edit_depth) {

		/* Emit signal: */
		g_signal_emit (G_OBJECT(doc),
			       signals[END_EDIT], 0);
	}
}

/**
 * cong_document_is_within_edit:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
cong_document_is_within_edit(CongDocument *doc)
{
	g_return_val_if_fail (doc, FALSE);

	return (PRIVATE(doc)->edit_depth>0);
}

/**
 * cong_document_private_node_make_orphan:
 * @doc:
 * @node:
 *
 * TODO: Write me
 */
void 
cong_document_private_node_make_orphan(CongDocument *doc, CongNodePtr node)
{
#if DEBUG_MVC
	g_message("cong_document_private_node_make_orphan");
#endif

	g_assert (cong_document_is_within_edit(doc));

	if (node->parent) {
		/* Emit signal: */
		g_signal_emit (G_OBJECT(doc),
			       signals[NODE_MAKE_ORPHAN], 0,
			       node);
	}
}

/**
 * cong_document_private_node_add_after:
 * @doc:
 * @node:
 * @older_sibling:
 *
 * TODO: Write me
 */
void 
cong_document_private_node_add_after(CongDocument *doc, CongNodePtr node, CongNodePtr older_sibling)
{
	g_return_if_fail(doc);
	g_return_if_fail(node);

	#if DEBUG_MVC
	g_message("cong_document_private_node_add_after");
	#endif

	g_assert (cong_document_is_within_edit(doc));

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[NODE_ADD_AFTER], 0,
		       node,
		       older_sibling);
}

/**
 * cong_document_private_node_add_before:
 * @doc:
 * @node:
 * @younger_sibling:
 *
 * TODO: Write me
 */
void 
cong_document_private_node_add_before(CongDocument *doc, CongNodePtr node, CongNodePtr younger_sibling)
{
	g_return_if_fail(doc);
	g_return_if_fail(node);

	#if DEBUG_MVC
	g_message("cong_document_private_node_add_before");
	#endif

	g_assert (cong_document_is_within_edit(doc));

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[NODE_ADD_BEFORE], 0,
		       node,
		       younger_sibling);
}

/**
 * cong_document_private_node_set_parent:
 * @doc:
 * @node:
 * @adoptive_parent:
 * @add_to_end:  if TRUE, add the node to the end of the parent's list; if FALSE add to the start
 *
 * TODO: Write me
 */
void 
cong_document_private_node_set_parent (CongDocument *doc, 
				       CongNodePtr node, 
				       CongNodePtr adoptive_parent,
				       gboolean add_to_end)
{
	g_return_if_fail(doc);
	g_return_if_fail(node);

	#if DEBUG_MVC
	g_message("cong_document_private_node_set_parent");
	#endif

	g_assert (cong_document_is_within_edit(doc));

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[NODE_SET_PARENT], 0,
		       node,
		       adoptive_parent,
		       add_to_end);
}

/**
 * cong_document_private_node_set_text:
 * @doc:
 * @node:
 * @new_content:
 *
 * TODO: Write me
 */
void 
cong_document_private_node_set_text(CongDocument *doc, CongNodePtr node, const xmlChar *new_content)
{
	g_return_if_fail(doc);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_MVC
	g_message("cong_document_private_node_set_text");
	#endif

	g_assert (cong_document_is_within_edit(doc));

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[NODE_SET_TEXT], 0,
		       node,
		       new_content);
}

/**
 * cong_document_private_node_set_attribute:
 * @doc:
 * @node:
 * @ns_ptr:
 * @name:
 * @value:
 *
 * TODO: Write me
 */
void 
cong_document_private_node_set_attribute(CongDocument *doc, 
					 CongNodePtr node, 
					 xmlNs *ns_ptr,
					 const xmlChar *name, 
					 const xmlChar *value)
{
	g_return_if_fail(doc);
	g_return_if_fail(node);
	g_return_if_fail(name);
	g_return_if_fail(value);

	#if DEBUG_MVC
	g_message("cong_document_private_node_set_attribute");
	#endif

	g_assert (cong_document_is_within_edit(doc));

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[NODE_SET_ATTRIBUTE], 0,
		       node,
		       ns_ptr,
		       name,
		       value);
}

/**
 * cong_document_private_node_remove_attribute:
 * @doc:
 * @node:
 * @ns_ptr:
 * @name:
 *
 * TODO: Write me
 */
void 
cong_document_private_node_remove_attribute(CongDocument *doc, 
					    CongNodePtr node, 
					    xmlNs *ns_ptr,
					    const xmlChar *name)
{
	g_return_if_fail(doc);
	g_return_if_fail(node);
	g_return_if_fail(name);

	#if DEBUG_MVC
	g_message("cong_document_private_node_remove_attribute");
	#endif

	g_assert (cong_document_is_within_edit(doc));

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[NODE_REMOVE_ATTRIBUTE], 0,
		       node,
		       ns_ptr,
		       name);
}

/**
 * cong_document_private_on_selection_change:
 * @doc:
 *
 * TODO: Write me
 */
void 
cong_document_private_on_selection_change(CongDocument *doc)
{
	GList *iter;

	g_return_if_fail(doc);

	#if DEBUG_MVC
	g_message("cong_document_private_on_selection_change");
	#endif

	g_assert (cong_document_is_within_edit(doc));

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[SELECTION_CHANGE], 0);

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_selection_change) {
			view->klass->on_selection_change(view);
		}
	}
}

/**
 * cong_document_private_on_cursor_change:
 * @doc:
 *
 * TODO: Write me
 */
void 
cong_document_private_on_cursor_change(CongDocument *doc)
{
	GList *iter;

	g_return_if_fail(doc);

#if 0
	#if DEBUG_MVC
	g_message("cong_document_private_node_on_cursor_change");
	#endif
#endif

	g_assert (cong_document_is_within_edit(doc));

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[CURSOR_CHANGE], 0);

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_cursor_change) {
			view->klass->on_cursor_change(view);
		}
	}
}

/**
 * cong_document_private_set_dtd_ptr:
 * @doc:
 * @dtd_ptr:
 *
 * TODO: Write me
 */
void 
cong_document_private_set_dtd_ptr (CongDocument *doc,
				   xmlDtdPtr dtd_ptr)
{
	g_return_if_fail(doc);

	g_return_if_fail(doc);

#if 0
	#if DEBUG_MVC
	g_message("cong_document_set_dtd_ptr");
	#endif
#endif

	g_assert (cong_document_is_within_edit(doc));

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[SET_DTD_PTR], 0,
		       dtd_ptr);
}
/* end of MVC user hooks */

/**
 * cong_document_register_view:
 * @doc:
 * @view:
 *
 * TODO: Write me
 */
void 
cong_document_register_view(CongDocument *doc, CongView *view)
{
	g_return_if_fail(doc);
	g_return_if_fail(view);

	PRIVATE(doc)->views = g_list_prepend(PRIVATE(doc)->views, view);
	g_object_ref (G_OBJECT(doc));
}

/**
 * cong_document_unregister_view:
 * @doc:
 * @view:
 *
 * TODO: Write me
 */
void 
cong_document_unregister_view(CongDocument *doc, CongView *view)
{
	g_return_if_fail(doc);
	g_return_if_fail(view);

	PRIVATE(doc)->views = g_list_remove(PRIVATE(doc)->views, view); 
	g_object_unref (G_OBJECT(doc));
}

/**
 * cong_document_get_cursor:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
CongCursor* 
cong_document_get_cursor(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return &PRIVATE(doc)->cursor;
}

/**
 * cong_document_get_selection:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
CongSelection* 
cong_document_get_selection(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return PRIVATE(doc)->selection;
}

/** 
 * cong_document_get_selected_node
 * @doc: the document
 * 
 * Convenience wrapper around CongSelection for dealing with selection of specific nodes (as opposed to text ranges).
 * 
 * Returns: If a specific node is selected, returns that node.  If a text range is selected instead, or there is no selection, NULL is returned.
 */
CongNodePtr
cong_document_get_selected_node (CongDocument *doc)
{
	CongLocation *start, *end;

	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);

	start = cong_selection_get_logical_start (PRIVATE(doc)->selection);
	end = cong_selection_get_logical_start (PRIVATE(doc)->selection);
	g_assert (start);
	g_assert (end);

	if (start->node == end->node) {
		return start->node;
	}

	return NULL;
}


/** 
 * cong_document_get_language_for_node
 * @doc:
 * @node:
 * 
 * Every node within a document has an implied language, although the logic to determine 
 * this may depend on the dispspec, on attributes of the nodes, and perhaps even on user prefererences (and hence the value may change unexpectedly).
 * 
 * Returns: a #PangoLanguage ptr which will exist for the whole lifetime of the program
 */
PangoLanguage*
cong_document_get_language_for_node(CongDocument *doc, 
				    CongNodePtr node)
{
	g_return_val_if_fail (doc, NULL);
	g_return_val_if_fail (node, NULL);

	/* We special-case DocBook here; perhaps this should involve a plugin mechanism instead?: */
	if (cong_util_is_docbook (doc)) {
		/* All DocBook elements have an optional "lang" attribute which appears to be drawn from ISO 639, potentially extended with a country code from ISO 3166 */
		/* Search in this node and above for such an attribute: */
		
		while (node != NULL) {
			
			if (cong_node_type (node)==CONG_NODE_TYPE_ELEMENT) {
				xmlChar *lang = xmlGetProp (node, "lang");
				if (lang != NULL) {
					PangoLanguage *result = pango_language_from_string (lang);		
					xmlFree (lang);
					
					return result;
				}
			}
			node = node->parent;
		}
		
		return NULL;
	}

	/* Otherwise, use the standard xml:lang attribute (if any), searching parents up to the root as appropriate: */
	{
		xmlChar *xml_lang = xmlNodeGetLang (node);

		if (xml_lang) {
			PangoLanguage *result = pango_language_from_string (xml_lang);		
			xmlFree (xml_lang);
			
			return result;
		}

		/* Not found: */
		return NULL;
	}
}

/**
 * cong_document_make_pango_log_attr_for_node:
 * @doc:
 * @node: a text or comment node
 * @pango_log_attrs: pointer for output
 * @attrs_len: pointer for output
 *
 * Generate Pango logical attributes for the given node, which must be either
 * text or a comment.
 */
void 
cong_document_make_pango_log_attr_for_node (CongDocument *doc,
					    CongNodePtr node,
					    PangoLogAttr **pango_log_attrs,
					    int *attrs_len)
{
	PangoLanguage *language;

	g_return_if_fail (doc);
	g_return_if_fail (node);
	g_return_if_fail (node->content);
	g_return_if_fail (pango_log_attrs);
	g_return_if_fail (attrs_len);

	language = cong_document_get_language_for_node (doc, node);

	*attrs_len = g_utf8_strlen (node->content,-1)+1;

	*pango_log_attrs = g_new (PangoLogAttr, (*attrs_len));
	
	pango_get_log_attrs (node->content,
			     strlen(node->content), /* length in bytes */
			     -1,
			     language,
			     *pango_log_attrs,
			     *attrs_len);
}

/** 
 * cong_document_should_spellcheck_node:
 * @doc: a CongDocument
 * @node: a text or comment node in the document
 * 
 * Determine whether the node should be spell-checked or not.  This depends on the
 * dispspec; for example, you probably don't want to spell-check filenames or environment
 * variables in a DocBook document.
 * 
 * Returns: TRUE iff the node should be spellchecked
 */
gboolean
cong_document_should_spellcheck_node (CongDocument *doc, 
				      CongNodePtr node)
{
	CongDispspecElement *ds_element;

	g_return_val_if_fail(doc, FALSE);
	g_return_val_if_fail(node, FALSE);

	/* Always spellcheck inside comments? */
	if (cong_node_type (node)==CONG_NODE_TYPE_COMMENT) {
		return TRUE;
	}

	/* Traverse up the tree until you find an element: */
	while (cong_node_type (node)!=CONG_NODE_TYPE_ELEMENT) {
		node=node->parent;

		if (node==NULL) {
			return FALSE;
		}
	}

	ds_element = cong_document_get_dispspec_element_for_node (doc, node);

	if (ds_element) {
		return cong_dispspec_element_should_spellcheck (ds_element);
	} else {
		/* If no dispspec, then don't spellcheck: */
		return FALSE;
	}

}


static gboolean 
cong_document_for_each_node_recurse (CongDocument *doc, 
				     CongNodePtr node, 
				     CongDocumentRecursionCallback callback, 
				     gpointer user_data, 
				     guint recursion_level)
{
	g_assert(doc);
	g_assert(node);
	g_assert(callback);

	if ((*callback)(doc, node, user_data, recursion_level)) {
		return TRUE;
	}
	    
	/* Recurse over children: */
	{
		CongNodePtr child_iter;

		for (child_iter = node->children; child_iter; child_iter=child_iter->next) {
			if (cong_document_for_each_node_recurse(doc, child_iter, callback, user_data, recursion_level+1)) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

/**
 * cong_document_for_each_node:
 * @doc:
 * @callback:
 * @callback_data:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_document_for_each_node (CongDocument *doc, 
			     CongDocumentRecursionCallback callback, 
			     gpointer user_data)
{
	g_return_val_if_fail (doc, TRUE);
	g_return_val_if_fail (callback, TRUE);

	return cong_document_for_each_node_recurse (doc,
						    (CongNodePtr)cong_document_get_xml(doc), 
						    callback, 
						    user_data, 
						    0);
}

/**
 * cong_document_for_each_child_of_node:
 * @doc:
 * @parent:
 * @callback:
 * @callback_data:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_document_for_each_child_of_node (CongDocument *doc, 
				      CongNodePtr parent, 
				      CongDocumentRecursionCallback callback, 
				      gpointer callback_data)
{
	CongNodePtr child_iter;

	g_return_val_if_fail (doc, TRUE);
	g_return_val_if_fail (parent, TRUE);
	g_return_val_if_fail (callback, TRUE);

	for (child_iter = parent->children; child_iter; child_iter=child_iter->next) {
		if ((*callback)(doc, child_iter, callback_data, 0)) {
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * cong_document_make_nodes_from_source_fragment:
 * @doc:
 * @source_fragment:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr
cong_document_make_nodes_from_source_fragment (CongDocument *doc, 
					       const gchar *source_fragment)
{
	gchar *fake_document;
	xmlDocPtr xml_doc;
	CongNodePtr result;

	g_return_val_if_fail (doc, NULL);
	g_return_val_if_fail (source_fragment, NULL);

	fake_document = g_strdup_printf ("<?xml version=\"1.0\"?>\n<placeholder>%s</placeholder>", source_fragment);
	
	xml_doc = xmlParseMemory (fake_document, 
				  strlen(fake_document));
	
	if (NULL==xml_doc) {
		/* Then the parsing failed: */
		g_free (fake_document);
		return NULL;
	}

	g_assert(xml_doc->children);
	g_assert(cong_node_is_element (xml_doc->children, NULL, "placeholder"));

	result = cong_node_recursive_dup (xml_doc->children);
		
	cong_node_recursive_set_doc (result, 
				     cong_document_get_xml (doc));	

	xmlFreeDoc (xml_doc);
	g_free (fake_document);

	g_assert(cong_node_is_element (result, NULL, "placeholder"));
	
	return result;

}

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
			       CongNodePtr node)
{
	xmlDocPtr doc;
	xmlElementPtr elemDecl = NULL;
	const xmlChar *prefix = NULL;
	gboolean extsubset = FALSE;

	/*  check cong_doc */
	g_return_val_if_fail(cong_doc, NULL);

	/*  check node */
	g_return_val_if_fail(node, NULL);
	
	/*  check that node has embedded document */
	g_return_val_if_fail(node->doc, NULL);

	/*  set document */
	doc = node->doc;
	
	/*  check that document has DTD */
	g_return_val_if_fail(doc->intSubset || doc->extSubset, NULL);

	if (node->type != XML_ELEMENT_NODE) { return NULL; }

	/*  ensure element has a name */
	g_return_val_if_fail(node->name, NULL);

	/*
	 * Fetch the declaration for the qualified name.
	 */
	if ((node->ns != NULL) && (node->ns->prefix != NULL)) {
		prefix = node->ns->prefix;
	}
	
	/*  search the internal subset DTD for a description of this elemenet */
	if (prefix != NULL) {
		elemDecl = xmlGetDtdQElementDesc(doc->intSubset,
						 node->name, prefix);
	}
	
	/*  if that didn't work, try the external subset */
	if ((elemDecl == NULL) && (doc->extSubset != NULL)) {
	    elemDecl = xmlGetDtdQElementDesc(doc->extSubset,
		                             node->name, prefix);
	    if (elemDecl != NULL) {
		    extsubset = TRUE;
	    }
	}

	/*
	 * If the qualified name didn't work, try the
	 * non-qualified name.
	 * Fetch the declaration for the non qualified name
	 * This is "non-strict" validation should be done on the
	 * full QName but in that case being flexible makes sense.
	 */
	if (elemDecl == NULL) {
		elemDecl = xmlGetDtdElementDesc(doc->intSubset, node->name);
	}

	if ((elemDecl == NULL) && (doc->extSubset != NULL)) {
		elemDecl = xmlGetDtdElementDesc(doc->extSubset, node->name);
		if (elemDecl != NULL) {
			extsubset = TRUE;
		}
	}

	return elemDecl;
}

/**
 * cong_document_get_command_history:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
CongCommandHistory*
cong_document_get_command_history (CongDocument *doc)
{
	g_return_val_if_fail (IS_CONG_DOCUMENT(doc), NULL);

	return PRIVATE(doc)->history;
}

/**
 * cong_document_undo:
 * @doc:
 *
 * TODO: Write me
 */
void
cong_document_undo (CongDocument *doc)
{
	g_return_if_fail (IS_CONG_DOCUMENT(doc));

#if 0
	g_message ("cong_document_undo");
#endif

	cong_command_history_undo (PRIVATE(doc)->history);
}

/**
 * cong_document_redo:
 * @doc:
 *
 * TODO: Write me
 */
void
cong_document_redo (CongDocument *doc)
{
	g_return_if_fail (IS_CONG_DOCUMENT(doc));

#if 0
	g_message ("cong_document_redo");
#endif

	cong_command_history_redo (PRIVATE(doc)->history);
}

/**
 * cong_document_select_node:
 * @doc:
 * @node:
 *
 * TODO: Write me
 */
void
cong_document_select_node (CongDocument *doc,
			   CongNodePtr node)
{	
	g_return_if_fail (IS_CONG_DOCUMENT (doc));
	g_return_if_fail (node);

	if (!cong_selection_is_node (cong_document_get_selection (doc),node)) {
		gchar *node_name = cong_document_get_node_name (doc, node);
		gchar *cmd_name = g_strdup_printf (_("Select %s"), node_name);
		CongCommand *cmd = cong_document_begin_command (doc, cmd_name, NULL);
		CongLocation new_selection_start;
		CongLocation new_selection_end;

		cong_location_set_to_start_of_node(&new_selection_start, node);
		cong_location_set_to_end_of_node(&new_selection_end, node);
		
		cong_command_add_selection_change (cmd,
						   &new_selection_start,
						   &new_selection_end);
		
		cong_document_end_command (doc, cmd);
		
		g_free (node_name);
		g_free (cmd_name);
	}	
}

/* Helper function for cong_document_node_can_be_deleted
   Is the location within the subtree of the test node? */
static gboolean
is_location_in_node_subtree (const CongLocation *loc,
			     CongNodePtr test_node)
{
	g_assert (loc);
	g_assert (test_node);

	if (loc->node) {
		if (loc->node == test_node) {
			return TRUE;
		}

		if (cong_node_is_descendant_of (loc->node, test_node)) {
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * cong_document_node_can_be_deleted:
 * @doc:
 * @node:
 * 
 * Helper function  that determines if a node can safely be deleted.
 * It will fail if the node or its subtree contains the cursor or the start/end of the selection,
 * to avoid these containing dangling pointers to dead memory.
 * Used to fix bug #108530, to ensure that the cursor and selection are always moved out of the way.
 *
 * Returns: TRUE if the node can be safely deleted, FALSE otherwise
 */
gboolean
cong_document_node_can_be_deleted (CongDocument *cong_doc,
				   CongNodePtr node)
{
	CongCursor* cursor;
	CongSelection* selection;

	g_return_val_if_fail (IS_CONG_DOCUMENT (cong_doc), FALSE);
	g_return_val_if_fail (node, FALSE);

	cursor = cong_document_get_cursor (cong_doc);
	selection = cong_document_get_selection (cong_doc);

	/* Is the cursor's node within the subtree of the test node?  If so, return FALSE: */
	if (is_location_in_node_subtree (&cursor->location, node)) {
		return FALSE;
	}

	/* Similar tests for the selection start/end: */
	if (is_location_in_node_subtree (cong_selection_get_logical_start (selection), node)) {
		return FALSE;
	}

	if (is_location_in_node_subtree (cong_selection_get_logical_end (selection), node)) {
		return FALSE;
	}

	/* Passed all tests; it's save to delete this node: */
	return TRUE; 
}

/**
 * cong_document_can_paste:
 * @doc:
 *
 * Returns: %TRUE
 */
gboolean
cong_document_can_paste(CongDocument *doc)
{
	return TRUE;
	/* FIXMEPCS: conditions? */
}

#if ENABLE_PRINTING
struct can_print_data
{
	CongDocument *doc;
	gint num_print_methods;
};

static void
callback_can_print (CongServicePrintMethod *print_method, 
		    gpointer user_data)
{
	struct can_print_data *print_data = (struct can_print_data*)user_data;

	if (cong_print_method_supports_document(print_method, print_data->doc)) {
		print_data->doc++;
	}
}

/**
 * cong_document_can_print:
 * @doc:
 *
 * Returns:
 */
gboolean
cong_document_can_print (CongDocument *doc)
{
	struct can_print_data print_data;
	print_data.doc = doc;
	print_data.num_print_methods=0;

	cong_plugin_manager_for_each_print_method (cong_app_get_plugin_manager (cong_app_singleton ()),
						   callback_can_print,
						   &print_data);

	return print_data.num_print_methods>0;
}
#endif

/* Internal function definitions: */
static void
cong_document_finalize (GObject *object)
{
	CongDocument *doc = CONG_DOCUMENT (object);

	g_message ("cong_document_finalize");
	
	g_free (doc->private);
	doc->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
cong_document_dispose (GObject *object)
{
	CongDocument *doc = CONG_DOCUMENT (object);

	g_message ("cong_document_dispose");

	g_assert (doc->private);
	g_assert (PRIVATE(doc)->views == NULL); /* There must not be any views left referencing this document; views are supposed to hold references to the doc */
	g_assert (0==PRIVATE(doc)->edit_depth); /* We musn't be in the middle of an edit, or things will go badly wrong */

	if (PRIVATE(doc)->xml_doc) {
		xmlFreeDoc(PRIVATE(doc)->xml_doc);
		PRIVATE(doc)->xml_doc = NULL;
	}

#if 1
	if (PRIVATE(doc)->default_ds) {
		/* FIXME: need to make CongDispsec into a GObject */
#if 0
		g_object_unref (G_OBJECT (doc->default_ds));
#endif
		PRIVATE(doc)->default_ds = NULL;
	}
#endif
	
	if (PRIVATE(doc)->url) {
		g_free (PRIVATE(doc)->url);
		PRIVATE(doc)->url = NULL;
	}
	
	cong_cursor_uninit(&PRIVATE(doc)->cursor);

	if (PRIVATE(doc)->selection) {
		cong_selection_free (PRIVATE(doc)->selection);
		PRIVATE(doc)->selection = NULL;
	}

	if (PRIVATE(doc)->traversal) {
		g_object_unref (G_OBJECT (PRIVATE(doc)->traversal));
		PRIVATE(doc)->traversal = NULL;
	}

	if (PRIVATE(doc)->find_data) {
		g_free (PRIVATE(doc)->find_data);
		PRIVATE(doc)->traversal = NULL;
	}

	/* FIXME: the primary_window doesn't hold a ref to the document; should it?  
	   Should we hold a ref to the primary_window? 
	   Should the primary_window be a GObject subclass
	*/
	
	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

/* Implementation of default signal handlers: */
static void 
cong_document_handle_begin_edit(CongDocument *doc)
{
	GList *iter;

#if DEBUG_MVC
	g_message("cong_document_handle_begin_edit");
#endif

	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_begin_edit) {
			view->klass->on_document_begin_edit(view);
		}
	}

}

static gchar*
generate_source_for_PRIMARY (CongDocument *doc)
{
	CongSelection *selection;

	g_assert (IS_CONG_DOCUMENT (doc));

	selection = cong_document_get_selection(doc);
	
	if (!(cong_range_exists (cong_selection_get_logical_range (selection)) &&
	      cong_range_is_valid (cong_selection_get_logical_range (selection)))) { 
		return NULL;
	}
	
	if (cong_range_is_empty (cong_selection_get_logical_range (selection))) {
		return NULL;
	}

	if (!cong_range_can_be_copied (cong_selection_get_ordered_range (selection))) {
		return NULL;
	}

	return cong_range_generate_source (cong_selection_get_ordered_range (selection));		
}

static void 
cong_document_handle_end_edit(CongDocument *doc)
{
	GList *iter;

#if DEBUG_MVC
	g_message("cong_document_handle_end_edit");
#endif

	if (PRIVATE (doc)->selection_has_changed) {

		gchar *source;

		/* The selection changed at some point within the edit; refresh PRIMARY: */
		PRIVATE (doc)->selection_has_changed = FALSE;

		source = generate_source_for_PRIMARY (doc);

		if (source) {
			cong_app_set_clipboard_from_xml_fragment (cong_app_singleton(),
								  GDK_SELECTION_PRIMARY,
								  source,
								  doc);			
			g_free (source);
		} else {
			/* Set PRIMARY to be empty - FIXME: is this really appropriate? */
			cong_app_set_clipboard_from_xml_fragment (cong_app_singleton(),
								  GDK_SELECTION_PRIMARY,
								  "",
								  doc);
		}
	}


	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_begin_edit) {
			view->klass->on_document_end_edit(view);
		}
	}
	
}

static void 
cong_document_handle_node_make_orphan(CongDocument *doc, CongNodePtr node)
{
	GList *iter;
	CongNodePtr former_parent;
	
	/* This is a special case, in that doc is allowed to be NULL (to handle the clipboard) */
	
	g_return_if_fail(node);
	
#if DEBUG_MVC
	g_message("cong_document_handle_node_make_orphan");
#endif

	former_parent = node->parent;

	/* If the cursor or a selection were present, nullify them: */
	if (doc) {
#if 0
		CongCursor *cursor = cong_document_get_cursor(doc);
		CongSelection *selection = cong_document_get_selection(doc);

		if (cursor->location.node) {
			if (cong_node_is_descendant_of (cursor->location.node,
							node)) {
				cong_location_nullify (&cursor->location);

				cong_document_on_cursor_change (doc);
			}
		}

		if (cong_selection_get_logical_start (selection)->node) {
			if (cong_node_is_descendant_of (cong_selection_get_logical_start (selection)->node,
							node)) {
				cong_selection_nullify (selection);
				cong_document_on_selection_change (doc);
			}
		}
#endif
	}
	
	if (doc) {
		/* Notify listeners: */
		for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
			CongView *view = CONG_VIEW(iter->data);
			
			g_assert(view->klass);
			if (view->klass->on_document_node_make_orphan) {
				view->klass->on_document_node_make_orphan(view, TRUE, node, former_parent);
			}
		}
		
		cong_document_set_modified(doc, TRUE);
	}

	/* Make the change: */
	PRIVATE(doc)->num_nodes_valid = FALSE;
	cong_node_private_make_orphan(node);

	if (doc) {
		/* Notify listeners: */
		for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
			CongView *view = CONG_VIEW(iter->data);
			
			g_assert(view->klass);
			if (view->klass->on_document_node_make_orphan) {
				view->klass->on_document_node_make_orphan(view, FALSE, node, former_parent);
			}
		}
		
		cong_document_set_modified(doc, TRUE);
	}

}

static void
cong_document_handle_node_add_after(CongDocument *doc, CongNodePtr node, CongNodePtr older_sibling)
{
	GList *iter;

	g_assert(doc);
	g_return_if_fail(doc);
	g_return_if_fail(node);

	#if DEBUG_MVC
	g_message("cong_document_handle_node_add_after");
	#endif

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_add_after) {
			view->klass->on_document_node_add_after(view, TRUE, node, older_sibling);
		}
	}

	/* Make the change: */
	PRIVATE(doc)->num_nodes_valid = FALSE;
	cong_node_private_add_after(node, older_sibling);

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_add_after) {
			view->klass->on_document_node_add_after(view, FALSE, node, older_sibling);
		}
	}

	cong_document_set_modified(doc, TRUE);
}

static void
cong_document_handle_node_add_before(CongDocument *doc, CongNodePtr node, CongNodePtr younger_sibling)
{
	GList *iter;

	#if DEBUG_MVC
	g_message("cong_document_handle_node_add_before");
	#endif

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_add_before) {
			view->klass->on_document_node_add_before(view, TRUE, node, younger_sibling);
		}
	}

	/* Make the change: */
	PRIVATE(doc)->num_nodes_valid = FALSE;
	cong_node_private_add_before(node, younger_sibling);

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_add_before) {
			view->klass->on_document_node_add_before(view, FALSE, node, younger_sibling);
		}
	}

	cong_document_set_modified(doc, TRUE);
}

static void
cong_document_handle_node_set_parent(CongDocument *doc, CongNodePtr node, CongNodePtr adoptive_parent, gboolean add_to_end)
{
	GList *iter;

	g_return_if_fail(doc);
	g_return_if_fail(node);

	#if DEBUG_MVC
	g_message("cong_document_handle_node_set_parent");
	#endif

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_set_parent) {
			view->klass->on_document_node_set_parent(view, TRUE, node, adoptive_parent, add_to_end);
		}
	}

	/* Make the change: */
	PRIVATE(doc)->num_nodes_valid = FALSE;
	cong_node_private_set_parent(node, adoptive_parent, add_to_end);

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_set_parent) {
			view->klass->on_document_node_set_parent(view, FALSE, node, adoptive_parent, add_to_end);
		}
	}

	cong_document_set_modified(doc, TRUE);
}

static void
cong_document_handle_node_set_text(CongDocument *doc, CongNodePtr node, const xmlChar *new_content)
{
	GList *iter;

	g_return_if_fail(doc);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_MVC
	g_message("cong_document_handle_node_set_text");
	#endif

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_set_text) {
			view->klass->on_document_node_set_text(view, TRUE, node, new_content);
		}
	}

	/* Make the change: */
	cong_node_private_set_text(node, new_content);

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_set_text) {
			view->klass->on_document_node_set_text(view, FALSE, node, new_content);
		}
	}

	cong_document_set_modified(doc, TRUE);
}

static void
cong_document_handle_node_set_attribute(CongDocument *doc, 
					CongNodePtr node, 
					xmlNs *ns_ptr,
					const xmlChar *name, 
					const xmlChar *value)
{
	GList *iter;

	g_return_if_fail(doc);
	g_return_if_fail(node);
	g_return_if_fail(name);
	g_return_if_fail(value);

	#if DEBUG_MVC
	g_message("cong_document_node_handle_set_attribute");
	#endif

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_set_attribute) {
			view->klass->on_document_node_set_attribute(view, 
								    TRUE, 
								    node, 
								    ns_ptr,
								    name, 
								    value);
		}
	}

	/* Make the change: */
	cong_node_private_set_attribute(node, ns_ptr, name, value);

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_set_attribute) {
			view->klass->on_document_node_set_attribute(view, 
								    FALSE, 
								    node, 
								    ns_ptr,
								    name, 
								    value);
		}
	}

	cong_document_set_modified(doc, TRUE);
}

static void
cong_document_handle_node_remove_attribute(CongDocument *doc, 
					   CongNodePtr node, 
					   xmlNs *ns_ptr,
					   const xmlChar *name)
{
	GList *iter;

	g_return_if_fail(doc);
	g_return_if_fail(node);
	g_return_if_fail(name);

	#if DEBUG_MVC
	g_message("cong_document_handle_node_remove_attribute");
	#endif

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_remove_attribute) {
			view->klass->on_document_node_remove_attribute(view, 
								       TRUE, 
								       node, 
								       ns_ptr,
								       name);
		}
	}

	/* Make the change: */
	cong_node_private_remove_attribute(node, ns_ptr, name);

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_remove_attribute) {
			view->klass->on_document_node_remove_attribute(view, 
								       FALSE, 
								       node, 
								       ns_ptr,
								       name);
		}
	}

	cong_document_set_modified(doc, TRUE);
}

static void
cong_document_handle_selection_change(CongDocument *doc)
{
	#if DEBUG_MVC
	g_message("cong_document_handle_selection_change");
	#endif

	PRIVATE (doc)->selection_has_changed = TRUE;
}

static void
cong_document_handle_cursor_change(CongDocument *doc)
{
#if 0
	#if DEBUG_MVC
	g_message("cong_document_handle_cursor_change");
	#endif
#endif
}


static void
cong_document_handle_set_dtd_ptr (CongDocument *doc,
				  xmlDtdPtr dtd_ptr)
{
	GList *iter;

	#if DEBUG_MVC
	g_message("cong_document_handle_set_dtd_ptr");
	#endif

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_set_dtd_ptr) {
			view->klass->on_document_set_dtd_ptr (view, 
							      TRUE, 
							      dtd_ptr);
		}
	}

	/* Make the change: */
	{
		xmlDocPtr xml_doc = PRIVATE(doc)->xml_doc;

		xml_doc->extSubset = dtd_ptr;
	}

	/* Document is now modified: */
	cong_document_set_modified(doc, TRUE);

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_set_dtd_ptr) {
			view->klass->on_document_set_dtd_ptr (view, 
							      FALSE,
							      dtd_ptr);
		}
	}
}

static void
cong_document_handle_set_url (CongDocument *doc,
			      const gchar *new_url)
{
	if (PRIVATE(doc)->url) {
		g_free(PRIVATE(doc)->url);
	}
	PRIVATE(doc)->url = g_strdup(new_url);

	/* FIXME: replace this with signal handler? */
	/* get at primary window; set title */
	if (PRIVATE(doc)->primary_window) {
		cong_primary_window_update_title(PRIVATE(doc)->primary_window);
	}
}

CongFindDialogData *
cong_document_get_find_dialog_data  (CongDocument *doc)
{ 
       g_return_val_if_fail(doc, NULL);
       return PRIVATE (doc)->find_data;
}
