/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"
#include "cong-document.h"
#include "cong-view.h"
#include "cong-error-dialog.h"
#include "cong-dispspec.h"
#include "cong-selection.h"
#include <libgnome/gnome-macros.h>
#include "cong-util.h"
#include "cong-primary-window.h"
#include "cong-command.h"
#include "cong-command-history.h"
#include "cong-marshal.h"

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
cong_document_handle_node_set_parent(CongDocument *doc, CongNodePtr node, CongNodePtr adoptive_parent);

static void
cong_document_handle_node_set_text(CongDocument *doc, CongNodePtr node, const xmlChar *new_content);

static void
cong_document_handle_node_set_attribute(CongDocument *doc, CongNodePtr node, const xmlChar *name, const xmlChar *value);

static void
cong_document_handle_node_remove_attribute(CongDocument *doc, CongNodePtr node, const xmlChar *name);

static void
cong_document_handle_selection_change(CongDocument *doc);

static void
cong_document_handle_cursor_change(CongDocument *doc);

static void
cong_document_handle_set_external_dtd (CongDocument *doc,
				       const gchar* root_element,
				       const gchar* public_id,
				       const gchar* system_id);

#define TEST_VIEW 0
#define TEST_EDITOR_VIEW 0
#define DEBUG_MVC 0

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
	SET_EXTERNAL_DTD,

	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

struct CongDocumentDetails
{
	xmlDocPtr xml_doc;

	CongDispspec *ds;

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

	/* Stats about the document: */
	gboolean num_nodes_valid;	
	guint num_nodes;
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
	klass->set_external_dtd = cong_document_handle_set_external_dtd;

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
						  cong_cclosure_marshal_VOID__CONGNODEPTR,
						  G_TYPE_NONE, 
						  1, G_TYPE_POINTER);
	
	signals[NODE_ADD_AFTER] = g_signal_new ("node_add_after",
						CONG_DOCUMENT_TYPE,
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(CongDocumentClass, node_add_after),
						NULL, NULL,
						cong_cclosure_marshal_VOID__CONGNODEPTR_CONGNODEPTR,
						G_TYPE_NONE, 
						2, G_TYPE_POINTER, G_TYPE_POINTER);
	
	signals[NODE_ADD_BEFORE] = g_signal_new ("node_add_before",
						 CONG_DOCUMENT_TYPE,
						 G_SIGNAL_RUN_LAST,
						 G_STRUCT_OFFSET(CongDocumentClass, node_add_before),
						 NULL, NULL,
						 cong_cclosure_marshal_VOID__CONGNODEPTR_CONGNODEPTR,
						 G_TYPE_NONE, 
						 2, G_TYPE_POINTER, G_TYPE_POINTER);
	
	signals[NODE_SET_PARENT] = g_signal_new ("node_set_parent",
						 CONG_DOCUMENT_TYPE,
						 G_SIGNAL_RUN_LAST,
						 G_STRUCT_OFFSET(CongDocumentClass, node_set_parent),
						 NULL, NULL,
						 cong_cclosure_marshal_VOID__CONGNODEPTR_CONGNODEPTR,
						 G_TYPE_NONE, 
						 2, G_TYPE_POINTER, G_TYPE_POINTER);
	
	signals[NODE_SET_TEXT] = g_signal_new ("node_set_text",
					       CONG_DOCUMENT_TYPE,
					       G_SIGNAL_RUN_LAST,
					       G_STRUCT_OFFSET(CongDocumentClass, node_set_text),
					       NULL, NULL,
					       cong_cclosure_marshal_VOID__CONGNODEPTR_STRING,
					       G_TYPE_NONE, 
					       2, G_TYPE_POINTER, G_TYPE_STRING);
	

	signals[NODE_SET_ATTRIBUTE] = g_signal_new ("node_set_attribute",
						    CONG_DOCUMENT_TYPE,
						    G_SIGNAL_RUN_LAST,
						    G_STRUCT_OFFSET(CongDocumentClass, node_set_attribute),
						    NULL, NULL,
						    cong_cclosure_marshal_VOID__CONGNODEPTR_STRING_STRING,
						    G_TYPE_NONE, 
						    3, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING);

	signals[NODE_REMOVE_ATTRIBUTE] = g_signal_new ("node_remove_attribute",
						       CONG_DOCUMENT_TYPE,
						       G_SIGNAL_RUN_LAST,
						       G_STRUCT_OFFSET(CongDocumentClass, node_remove_attribute),
						       NULL, NULL,
						       cong_cclosure_marshal_VOID__CONGNODEPTR_STRING,
						       G_TYPE_NONE, 
						       2, G_TYPE_POINTER, G_TYPE_STRING);

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
	signals[SET_EXTERNAL_DTD] = g_signal_new ("set_external_dtd",
						  CONG_DOCUMENT_TYPE,
						  G_SIGNAL_RUN_LAST,
						  G_STRUCT_OFFSET(CongDocumentClass, set_external_dtd),
						  NULL, NULL,
						  cong_cclosure_marshal_VOID__STRING_STRING_STRING,
						  G_TYPE_NONE, 
						  3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
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

CongDocument*
cong_document_construct (CongDocument *doc,
			 xmlDocPtr xml_doc,
			 CongDispspec *ds, 
			 const gchar *url)
{
	CongNodePtr initial_cursor_node;

	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	g_return_val_if_fail (xml_doc, NULL);
	g_return_val_if_fail (ds, NULL);

	PRIVATE(doc)->xml_doc = xml_doc;
	PRIVATE(doc)->ds = ds;
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

	return doc;
}


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

xmlDocPtr
cong_document_get_xml(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return PRIVATE(doc)->xml_doc;
}

CongNodePtr
cong_document_get_root(CongDocument *doc)
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

CongDispspec*
cong_document_get_dispspec(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return PRIVATE(doc)->ds;
}

CongDispspecElement*
cong_document_get_dispspec_element_for_node(CongDocument *doc, CongNodePtr node)
{
	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);

	return cong_dispspec_lookup_node(PRIVATE(doc)->ds, node);
}

gchar*
cong_document_get_filename(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	if (PRIVATE(doc)->url) {
		gchar *filename;
		gchar *path;
		GnomeVFSURI *uri = gnome_vfs_uri_new(PRIVATE(doc)->url);
		
		cong_util_split_uri(uri, &filename, &path);

		gnome_vfs_uri_unref(uri);

		g_free(path);
		
		return filename;

	} else {
		return g_strdup("(Untitled)");
	}
}

gchar*
cong_document_get_full_uri(CongDocument *doc) {
	g_return_val_if_fail(doc, NULL);

	if (PRIVATE(doc)->url) {
		return g_strdup(PRIVATE(doc)->url);
	}
	else {
		return NULL;
	}		    
}

gchar*
cong_document_get_parent_uri(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	if (PRIVATE(doc)->url) {
		gchar *filename;
		gchar *path;
		GnomeVFSURI *uri = gnome_vfs_uri_new(PRIVATE(doc)->url);
		
		cong_util_split_uri(uri, &filename, &path);

		gnome_vfs_uri_unref(uri);

		g_free(filename);
		
		return path;

	} else {
		return g_strdup(".");
	}
}

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

xmlNsPtr
cong_document_get_nsptr (CongDocument *doc, const gchar *xmlns)
{
	xmlNsPtr ns;

	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(xmlns, NULL);

	ns = xmlSearchNs(PRIVATE(doc)->xml_doc,
			 (xmlNodePtr)PRIVATE(doc)->xml_doc, /* FIXME: is this correct? */
			 xmlns);

	return ns;
}

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

	file_uri = gnome_vfs_uri_new(filename);
	
	vfs_result = cong_xml_save_to_vfs(PRIVATE(doc)->xml_doc, 
					  file_uri,	
					  &file_size);

	if (vfs_result != GNOME_VFS_OK) {
		GtkDialog* dialog = cong_error_dialog_new_from_file_save_failure(toplevel_window,
										 file_uri, 
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

gboolean
cong_document_is_modified(CongDocument *doc)
{
	g_return_val_if_fail(doc, FALSE);

	return PRIVATE(doc)->modified;
}

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

void
cong_document_set_primary_window(CongDocument *doc, CongPrimaryWindow *window)
{
	g_return_if_fail(doc);
	g_return_if_fail(window);

	g_assert(PRIVATE(doc)->primary_window==NULL);
	PRIVATE(doc)->primary_window = window;
}

void 
cong_document_set_url(CongDocument *doc, const gchar *url) 
{
	g_return_if_fail(doc);

	if (PRIVATE(doc)->url) {
		g_free(PRIVATE(doc)->url);
	}
	PRIVATE(doc)->url = g_strdup(url);

	/* get at primary window; set title */
	if (PRIVATE(doc)->primary_window) {
		cong_primary_window_update_title(PRIVATE(doc)->primary_window);
	}
}

glong
cong_document_get_seconds_since_last_save_or_load(const CongDocument *doc)
{
	GTimeVal current_time;

	g_return_val_if_fail(doc, 0);

	g_get_current_time(&current_time);

	return current_time.tv_sec - PRIVATE(doc)->time_of_last_save.tv_sec;
}

static gboolean 
node_count_callback (CongDocument *doc, CongNodePtr node, gpointer user_data, guint recursion_level)
{
	((int*)user_data)++;

	return FALSE;		
}


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

void 
cong_document_node_ref (CongDocument *doc,
			CongNodePtr node)
{
	g_return_if_fail (IS_CONG_DOCUMENT (doc));
	g_return_if_fail (node);

	/* unwritten */
}

void 
cong_document_node_unref (CongDocument *doc,
			  CongNodePtr node)
{
	g_return_if_fail (IS_CONG_DOCUMENT (doc));
	g_return_if_fail (node);

	/* unwritten */
}

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

void
cong_document_end_command (CongDocument *doc,
			   CongCommand *cmd)
{
	g_return_if_fail (IS_CONG_DOCUMENT(doc));
	g_return_if_fail (IS_CONG_COMMAND(cmd));
	g_return_if_fail (doc == cong_command_get_document (cmd));
	g_return_if_fail (cmd==PRIVATE(doc)->current_command);

	cong_document_end_edit (doc);

	PRIVATE(doc)->current_command = NULL;

	cong_command_history_add_command (PRIVATE(doc)->history,
					  cmd);
	g_object_unref (G_OBJECT (cmd));
}


/* Public MVC hooks: */
#if 1
void cong_document_begin_edit (CongDocument *doc)
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

void cong_document_end_edit (CongDocument *doc)
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

gboolean cong_document_is_within_edit(CongDocument *doc)
{
	g_return_val_if_fail (doc, FALSE);

	return (PRIVATE(doc)->edit_depth>0);
}


void cong_document_private_node_make_orphan(CongDocument *doc, CongNodePtr node)
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

void cong_document_private_node_add_after(CongDocument *doc, CongNodePtr node, CongNodePtr older_sibling)
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

void cong_document_private_node_add_before(CongDocument *doc, CongNodePtr node, CongNodePtr younger_sibling)
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

void cong_document_private_node_set_parent(CongDocument *doc, CongNodePtr node, CongNodePtr adoptive_parent)
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
		       adoptive_parent);
}

void cong_document_private_node_set_text(CongDocument *doc, CongNodePtr node, const xmlChar *new_content)
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

void cong_document_private_node_set_attribute(CongDocument *doc, CongNodePtr node, const xmlChar *name, const xmlChar *value)
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
		       name,
		       value);
}

void cong_document_private_node_remove_attribute(CongDocument *doc, CongNodePtr node, const xmlChar *name)
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
		       name);
}

void cong_document_private_on_selection_change(CongDocument *doc)
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

void cong_document_private_on_cursor_change(CongDocument *doc)
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

void 
cong_document_set_external_dtd (CongDocument *doc,
				const xmlChar *root_element,
				const xmlChar *public_id, 
				const xmlChar *system_id)
{
	g_return_if_fail(doc);

	g_return_if_fail(doc);

#if 0
	#if DEBUG_MVC
	g_message("cong_document_set_external_dtd");
	#endif
#endif

	g_assert (cong_document_is_within_edit(doc));

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[SET_EXTERNAL_DTD], 0,
		       root_element,
		       public_id, 
		       system_id);
}
#endif

/* end of MVC user hooks */


#if !SUPPORT_UNDO
void cong_document_tag_remove(CongDocument *doc, CongNodePtr x)
{
	CongNodePtr parent;

	g_return_if_fail(doc);
	g_return_if_fail(x);

	#if DEBUG_MVC
	g_message("cong_document_tag_remove");
	#endif

	parent = x->parent;

	cong_document_begin_edit(doc);

	cong_util_remove_tag (doc, 
			      x); /* this is now a compound operation */

	cong_document_merge_adjacent_text_children_of_node (doc, 
							    parent);

	cong_document_end_edit(doc);
}
#endif /* #if !SUPPORT_UNDO */

void cong_document_register_view(CongDocument *doc, CongView *view)
{
	g_return_if_fail(doc);
	g_return_if_fail(view);

	PRIVATE(doc)->views = g_list_prepend(PRIVATE(doc)->views, view);
	g_object_ref (G_OBJECT(doc));
}

void cong_document_unregister_view(CongDocument *doc, CongView *view)
{
	g_return_if_fail(doc);
	g_return_if_fail(view);

	PRIVATE(doc)->views = g_list_remove(PRIVATE(doc)->views, view); 
	g_object_unref (G_OBJECT(doc));
}


CongCursor* cong_document_get_cursor(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return &PRIVATE(doc)->cursor;
}

CongSelection* cong_document_get_selection(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return PRIVATE(doc)->selection;
}

PangoLanguage*
cong_document_get_language_for_node(CongDocument *doc, 
				    CongNodePtr node)
{
	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);

	return NULL; /* for now */
}

#if !SUPPORT_UNDO
void
cong_document_delete_selection (CongDocument *doc)
{
	CongSelection *selection;
	CongRange *range;

	g_return_if_fail(doc);

	selection = cong_document_get_selection(doc);
	range = cong_selection_get_ordered_range (selection);

	cong_document_begin_edit (doc);

	cong_document_delete_range (doc, 
				    cong_selection_get_ordered_range (selection));

	cong_selection_nullify (selection);

	cong_document_on_selection_change (doc);

	cong_document_end_edit (doc);
}
#endif /* #if !SUPPORT_UNDO */

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

#if !SUPPORT_UNDO
static gboolean
merge_text_update_location_callback (CongDocument *doc,
				     CongLocation *location, 
				     gpointer user_data)
{
	CongNodePtr affected_node = user_data;

	g_assert (affected_node);
	g_assert (affected_node->prev);

	if (location->node == affected_node->prev) {
		
		location->node = affected_node;

		return TRUE;

	} else if (location->node == affected_node) {

		location->byte_offset += strlen(affected_node->prev->content);

		return TRUE;
	}

	return FALSE;
}

static gboolean merge_adjacent_text_callback(CongDocument *doc, CongNodePtr node, gpointer user_data, guint recursion_level)
{
	/* We have to "look behind" at the previous sibling, since the iteration moes forward: */
	if (node->prev) {
		if (cong_node_type(node)==CONG_NODE_TYPE_TEXT) {
			if (cong_node_type(node->prev)==CONG_NODE_TYPE_TEXT) {
				/* Merge preceding node's text into this one, then delete it: */
				gchar *new_text;

				new_text = g_strdup_printf("%s%s", node->prev->content, node->content);

				/* Update cursor and selection if necessary: */
				cong_document_for_each_location (doc,
								 merge_text_update_location_callback, 
								 node);

				cong_document_node_set_text (doc, node, new_text);
				g_free (new_text);

				cong_document_node_recursive_delete (doc, node->prev);
			}			
		}
	}

	/* Keep going: */
	return FALSE;
}

void 
cong_document_merge_adjacent_text_nodes (CongDocument *doc)
{
	g_return_if_fail (doc);

	cong_document_begin_edit (doc);

	cong_document_for_each_node (doc, merge_adjacent_text_callback, NULL);

	cong_document_end_edit (doc);

}

void 
cong_document_merge_adjacent_text_children_of_node (CongDocument *doc, 
						    CongNodePtr parent)
{
	g_return_if_fail (doc);
	g_return_if_fail (parent);

	cong_document_begin_edit (doc);

	cong_document_for_each_child_of_node (doc, parent, merge_adjacent_text_callback, NULL);

	cong_document_end_edit (doc);
}
#endif /* #if !SUPPORT_UNDO */

#if !SUPPORT_UNDO
void 
cong_document_node_recursive_delete (CongDocument *doc, 
				     CongNodePtr node)
{
	CongNodePtr iter, next;

	CONG_NODE_SELF_TEST(node);

	/* You must ensure the cursor and/or selection locations don't retain a pointer to this node: */
	if (doc) {
		g_assert (PRIVATE(doc)->cursor.location.node!=node);
		g_assert (cong_selection_get_logical_start (PRIVATE(doc)->selection)->node!=node);
		g_assert (cong_selection_get_logical_end (PRIVATE(doc)->selection)->node!=node);
	}

	iter = node->children; 

	while (iter) {
		next = iter->next;

		CONG_NODE_SELF_TEST(iter);
		
		cong_document_node_recursive_delete(doc, iter);

		iter = next;
	}

	g_assert(node->children==NULL);
	g_assert(node->last==NULL);

	if (node->parent) {
		cong_document_node_make_orphan(doc, node);
	}

	cong_node_free(node);
}

static gboolean
pre_node_deletion_update_location_callback (CongDocument *doc,
					   CongLocation *location, 
					   gpointer user_data)
{
	CongNodePtr node = user_data;
	if (location->node == node) {

#if 1
		/* FIXME: do this for now: */
		cong_location_nullify (location);
		return TRUE;
#else
		if (node->prev) {
			/* Find text node: */
#error
			return TRUE;

		} else {
			if (node->next) {
				/* Find text node: */
#error

				return TRUE;

			} else {
				cong_location_nullify (location);
				return TRUE;
			}
		}
#endif
	}

	return FALSE;
}

struct text_deletion_userdata
{
	CongNodePtr node;
	int start_byte_offset;
	int end_byte_offset;
};

static gboolean
text_deletion_update_location_callback (CongDocument *doc,
					CongLocation *location, 
					gpointer user_data)
{
	struct text_deletion_userdata *text_deletion_userdata = user_data;

	if (location->node == text_deletion_userdata->node) {
		if (location->byte_offset <= text_deletion_userdata->start_byte_offset) {
			return FALSE;
		} else {
			if (location->byte_offset < text_deletion_userdata->end_byte_offset) {
				cong_location_nullify (location);
				return TRUE;
			} else {
				location->byte_offset -= (text_deletion_userdata->end_byte_offset - text_deletion_userdata->start_byte_offset);
				
				return TRUE;				
			}
		}
	} 

	return FALSE;
}

void cong_document_delete_range (CongDocument *doc, 
				 CongRange *range)
{
	CongLocation loc0, loc1;
	CongNodePtr n0, n1, n2;
	
	g_return_if_fail(doc);
	g_return_if_fail(range);

	/* Validate range */
	g_return_if_fail( cong_location_exists(&range->loc0) );
	g_return_if_fail( cong_location_exists(&range->loc1) );
	g_return_if_fail( cong_location_parent(&range->loc0) == cong_location_parent(&range->loc1) );
	/* both must be children of the same parent to maintain proper nesting */

	cong_document_begin_edit (doc);

	/* --- Processing for multiple nodes --- */
	if (range->loc0.node != range->loc1.node)
	{
		CongNodePtr prev_node;
	
		/* Range is valid, now order first/last nodes */
		
		for (n0 = range->loc0.node; n0 && n0 != range->loc1.node; n0 = n0->next) ;
		
		if (!n0)
		{
			cong_location_copy(&loc0, &range->loc1);
			cong_location_copy(&loc1, &range->loc0);
		}
		else
		{
			cong_location_copy(&loc0, &range->loc0);
			cong_location_copy(&loc1, &range->loc1);
		}

		/* Split, first */

		if (loc0.byte_offset && cong_node_type(loc0.node) == CONG_NODE_TYPE_TEXT)
		{
			prev_node = cong_location_xml_frag_data_nice_split2(doc, &loc0);
			g_assert(prev_node);

			loc0.node = range->loc0.node = prev_node->next;
		} else {
			prev_node = loc0.node;
		}
		
		/* prev_node holds the previous node */

		/* Reparent, first & middle */
		for (n0 = loc0.node; n0 != loc1.node; n0 = n2) {
			n2 = n0->next;

			CONG_NODE_SELF_TEST(n0);

			cong_document_for_each_location (doc, 
							 pre_node_deletion_update_location_callback, 
							 n0);

			cong_document_node_recursive_delete (doc,
							     n0);
		}

		/* Split, last */

		if (loc1.byte_offset && cong_node_type(loc1.node) == CONG_NODE_TYPE_TEXT)
		{
			loc1.node = cong_location_xml_frag_data_nice_split2(doc, &loc1);
		}

		/* Delete last */
		cong_document_for_each_location (doc, 
						 pre_node_deletion_update_location_callback, 
						 loc1.node);

		/* FIXME: should this be a cong_document_node_recursive_delete instead? */
		cong_util_remove_tag (doc, 
				      loc1.node);
	}

	/* --- Processing for single node (loc0.node == loc1.node) --- */

	else
	{
		/* Sort out the ordering: */
		if (range->loc0.byte_offset < range->loc1.byte_offset)
		{
			cong_location_copy(&loc0,&range->loc0);
			cong_location_copy(&loc1,&range->loc1);
		}
		else
		{
			cong_location_copy(&loc0,&range->loc1);
			cong_location_copy(&loc1,&range->loc0);
		}

		if (cong_node_type(loc0.node) == CONG_NODE_TYPE_TEXT)
		{
			if (loc0.byte_offset == loc1.byte_offset) {
				/* The end is the beginning is the end */
			} else {
			
				/* Split up textual content of node: */
				gchar *text_before = g_strndup (loc0.node->content, loc0.byte_offset);

				gchar *new_text = g_strdup_printf("%s%s",text_before, loc1.node->content + loc1.byte_offset);

				struct text_deletion_userdata text_deletion_userdata;				
				text_deletion_userdata.node = loc0.node;
				text_deletion_userdata.start_byte_offset = loc0.byte_offset;
				text_deletion_userdata.end_byte_offset = loc1.byte_offset;

				cong_document_for_each_location (doc, 
								 text_deletion_update_location_callback, 
								 &text_deletion_userdata);

				cong_document_node_set_text (doc,
							     loc0.node,
							     new_text);

				g_free (text_before);
				g_free (new_text);

				/* what should happen to cursor? */
			}
		} else {
			/* Delete entire node: */
#if 1
			CONG_DO_UNIMPLEMENTED_DIALOG(NULL, "Deletion of a single non-textual node");
#else
			/* what should happen to cursor? */
			cong_util_remove_tag (doc, 
					      loc0.node);
#endif
		}
	}

	cong_document_end_edit (doc);
}
#endif /* #if !SUPPORT_UNDO */

#if !SUPPORT_UNDO
void
cong_document_insert_text (CongDocument *doc, 
			   CongLocation *loc, 
			   const gchar* insertion)
{
	xmlChar *new_content;

	int byte_length;

	g_return_if_fail(cong_location_exists(loc));
	g_return_if_fail (cong_node_is_valid_cursor_location(loc->node));
	g_return_if_fail(insertion!=NULL);
	g_return_if_fail(g_utf8_validate(insertion, -1, NULL));

	byte_length = strlen(insertion);

	new_content = xmlStrndup(loc->node->content, loc->byte_offset);
	CONG_VALIDATE_UTF8(new_content);

	new_content = xmlStrcat(new_content, insertion);
	CONG_VALIDATE_UTF8(new_content);

	CONG_VALIDATE_UTF8(loc->node->content+loc->byte_offset);
	new_content = xmlStrcat(new_content, loc->node->content+loc->byte_offset);
	CONG_VALIDATE_UTF8(new_content);

	cong_document_node_set_text(doc, loc->node, new_content);
	
	xmlFree(new_content);

	/* GREP FOR MVC: the location is updated here */
	loc->byte_offset += byte_length;		
	CONG_VALIDATE_UTF8(loc->node->content+loc->byte_offset);
}
#endif /* #if !SUPPORT_UNDO */

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
	
	g_message (fake_document);
	
	xml_doc = xmlParseMemory (fake_document, 
				  strlen(fake_document));
	
	g_assert(xml_doc);
	g_assert(xml_doc->children);
	g_assert(cong_node_is_tag (xml_doc->children, NULL, "placeholder"));

	result = cong_node_recursive_dup (xml_doc->children);
		
	cong_node_recursive_set_doc (result, 
				     cong_document_get_xml (doc));	

	xmlFreeDoc (xml_doc);
	g_free (fake_document);

	g_assert(cong_node_is_tag (result, NULL, "placeholder"));
	
	return result;

}

#if !SUPPORT_UNDO
void
cong_document_for_each_location (CongDocument *doc, 
				 CongUpdateLocationCallback callback, 
				 gpointer user_data)
{
	gboolean selection_change = FALSE;
	CongLocation logical_sel_start;
	CongLocation logical_sel_end;

	g_return_if_fail (doc);
	g_return_if_fail (callback);

	cong_location_copy (&logical_sel_start, cong_selection_get_logical_start (PRIVATE(doc)->selection));
	cong_location_copy (&logical_sel_end, cong_selection_get_logical_end (PRIVATE(doc)->selection));

#if 0
	g_message ("test for update of cursor location from (%p,%i)",PRIVATE(doc)->cursor.location.node, PRIVATE(doc)->cursor.location.byte_offset);
#endif

	if ( callback (doc,
		       &PRIVATE(doc)->cursor.location,
		       user_data)) {
#if 0
		g_message ("update of cursor location to (%p,%i)",PRIVATE(doc)->cursor.location.node, PRIVATE(doc)->cursor.location.byte_offset);
#endif
		cong_document_on_cursor_change (doc);
	}

	if ( callback (doc,
		       &logical_sel_start,
		       user_data)) {
		selection_change = TRUE;
	}
	
	if (callback (doc,
		      &logical_sel_end,
		      user_data)) {
		selection_change = TRUE;
	}
	
	if (selection_change) {
		cong_selection_set_logical_start (PRIVATE(doc)->selection,
						  &logical_sel_start);
		
		cong_selection_set_logical_end (PRIVATE(doc)->selection,
						&logical_sel_end);
		
		cong_document_on_selection_change (doc);
	}
}
#endif /* #if !SUPPORT_UNDO */

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

CongCommandHistory*
cong_document_get_command_history (CongDocument *doc)
{
	g_return_val_if_fail (IS_CONG_DOCUMENT(doc), NULL);

	return PRIVATE(doc)->history;
}


void
cong_document_undo (CongDocument *doc)
{
	g_return_if_fail (IS_CONG_DOCUMENT(doc));

	g_message ("cong_document_undo");

	cong_command_history_undo (PRIVATE(doc)->history);
}

void
cong_document_redo (CongDocument *doc)
{
	g_return_if_fail (IS_CONG_DOCUMENT(doc));

	g_message ("cong_document_redo");

	cong_command_history_redo (PRIVATE(doc)->history);
}


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
	if (PRIVATE(doc)->ds) {
		/* FIXME: need to make CongDispsec into a GObject */
#if 0
		g_object_unref (G_OBJECT (doc->ds));
#endif
		PRIVATE(doc)->ds = NULL;
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

static void 
cong_document_handle_end_edit(CongDocument *doc)
{
	GList *iter;

#if DEBUG_MVC
	g_message("cong_document_handle_end_edit");
#endif

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
cong_document_handle_node_set_parent(CongDocument *doc, CongNodePtr node, CongNodePtr adoptive_parent)
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
			view->klass->on_document_node_set_parent(view, TRUE, node, adoptive_parent);
		}
	}

	/* Make the change: */
	PRIVATE(doc)->num_nodes_valid = FALSE;
	cong_node_private_set_parent(node, adoptive_parent);

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_set_parent) {
			view->klass->on_document_node_set_parent(view, FALSE, node, adoptive_parent);
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
cong_document_handle_node_set_attribute(CongDocument *doc, CongNodePtr node, const xmlChar *name, const xmlChar *value)
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
			view->klass->on_document_node_set_attribute(view, TRUE, node, name, value);
		}
	}

	/* Make the change: */
	cong_node_private_set_attribute(node, name, value);

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_set_attribute) {
			view->klass->on_document_node_set_attribute(view, FALSE, node, name, value);
		}
	}

	cong_document_set_modified(doc, TRUE);
}

static void
cong_document_handle_node_remove_attribute(CongDocument *doc, CongNodePtr node, const xmlChar *name)
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
			view->klass->on_document_node_remove_attribute(view, TRUE, node, name);
		}
	}

	/* Make the change: */
	cong_node_private_remove_attribute(node, name);

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_node_remove_attribute) {
			view->klass->on_document_node_remove_attribute(view, FALSE, node, name);
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
cong_document_handle_set_external_dtd (CongDocument *doc,
				       const gchar* root_element,
				       const gchar* public_id,
				       const gchar* system_id)
{
	GList *iter;

	#if DEBUG_MVC
	g_message("cong_document_handle_set_external_dtd");
	#endif

#if SUPPORT_UNDO
	g_message ("FIXME: undo/redo not implemented for set_external_dtd");
#else
	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_set_external_dtd) {
			view->klass->on_document_set_external_dtd (view, 
								   TRUE, 
								   root_element,
								   public_id,
								   system_id);
		}
	}

	/* Make the change: */
	{
		xmlDocPtr xml_doc = PRIVATE(doc)->xml_doc;

		/* Remove any existing DTD: */
		{
			if (xml_doc->extSubset) {
				cong_document_private_node_make_orphan (doc,
									(xmlNodePtr)xml_doc->extSubset);

				cong_document_node_recursive_delete (doc, 
								     (xmlNodePtr)xml_doc->extSubset);

				xml_doc->extSubset = NULL;
			}
		}

		/* Add the new DTD (if any): */
		if (root_element) {
			xmlDtdPtr dtd_ptr = cong_util_make_dtd (xml_doc,
								root_element,
								public_id, 
								system_id);
			
			if (dtd_ptr) {			
				if (xml_doc->children) {
					cong_document_private_node_add_before (doc,
									       (xmlNodePtr)dtd_ptr,
									       (xmlNodePtr)xml_doc->children);
				} else {
					cong_document_private_node_set_parent (doc,
									       (xmlNodePtr)dtd_ptr,
									       (xmlNodePtr)xml_doc);
				}
				
				/* Ensure the DTD ptr is still set up within the xml_doc; the tree manipulation seems to make it lose the extSubset pointer: */
				xml_doc->extSubset = dtd_ptr;
			}
		}
	}

	/* Document is now modified: */
	cong_document_set_modified(doc, TRUE);

	/* Notify listeners: */
	for (iter = PRIVATE(doc)->views; iter; iter = g_list_next(iter) ) {
		CongView *view = CONG_VIEW(iter->data);
		
		g_assert(view->klass);
		if (view->klass->on_document_set_external_dtd) {
			view->klass->on_document_set_external_dtd (view, 
								   FALSE,
								   root_element,
								   public_id,
								   system_id);
		}
	}
#endif

}



