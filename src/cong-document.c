/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"
#include "cong-document.h"
#include "cong-view.h"
#include "cong-error-dialog.h"
#include "cong-dispspec.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "cong-marshal.h"

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

#define TEST_VIEW 0
#define TEST_EDITOR_VIEW 0
#define DEBUG_MVC 1

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
	CongCursor curs;
	CongSelection selection;

	gboolean modified; /* has the document been modified since it was last loaded/saved? */
	GTimeVal time_of_last_save;

	/* We have an SDI interface, so there should be just one primary window associated with each doc.
	   Knowing this lets us update the window title when it changes (eventually do as a signal on the document).
	*/
	CongPrimaryWindow *primary_window;

	/* Amortisation of updates: */
	guint edit_depth;
};

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongDocument, 
			cong_document,
			GObject,
			G_TYPE_OBJECT );

static void
cong_document_class_init (CongDocumentClass *klass)
{
#if 0
	G_OBJECT_CLASS (klass)->finalize = cong_document_finalize;
	G_OBJECT_CLASS (klass)->dispose = cong_document_dispose;
#endif

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

	cong_cursor_init(&PRIVATE(doc)->curs, doc);
	cong_selection_init(&PRIVATE(doc)->selection);

#if 0
	PRIVATE(doc)->curs.set = 0;
	PRIVATE(doc)->curs.w = 0;
#endif

	cong_location_nullify(&PRIVATE(doc)->selection.loc0);
	cong_location_nullify(&PRIVATE(doc)->selection.loc1);

	return doc;
}


CongDocument*
cong_document_new_from_xmldoc (xmlDocPtr xml_doc,
			       CongDispspec *ds, 
			       const gchar *url)
{
	CongDocument *doc;

	g_return_val_if_fail(xml_doc!=NULL, NULL);
#if 0
	g_return_val_if_fail(cong_xml_selftest_doc(xml_doc, NULL), NULL);
#endif

	return cong_document_construct (g_object_new (CONG_DOCUMENT_TYPE, NULL),
					xml_doc,
					ds,
					url);
}

#if 0
void
cong_document_ref(CongDocument *doc)
{
	g_return_if_fail(doc);

	PRIVATE(doc)->ref_count++;
}


void
cong_document_unref(CongDocument *doc)
{
	g_return_if_fail(doc);

	if ((--PRIVATE(doc)->ref_count)==0) {
		g_assert(PRIVATE(doc)->views == NULL); /* There must not be any views left referencing this document; views are supposed to hold references to the doc */

		cong_cursor_uninit(&PRIVATE(doc)->curs);
	
		xmlFreeDoc(PRIVATE(doc)->xml_doc);

		if (PRIVATE(doc)->url) {
			g_free(PRIVATE(doc)->url);
		}
	
		g_free(doc);
	}
}
#endif

xmlDocPtr
cong_document_get_xml(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return PRIVATE(doc)->xml_doc;
}

CongNodePtr
cong_document_get_root(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return PRIVATE(doc)->xml_doc->children;

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
		
		cong_error_split_uri(uri, &filename, &path);

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
		
		cong_error_split_uri(uri, &filename, &path);

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

	PRIVATE(doc)->modified = modified;

	/* get at primary window; set title */
	if (PRIVATE(doc)->primary_window) {
		cong_primary_window_update_title(PRIVATE(doc)->primary_window);
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


/* Public MVC hooks: */
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


void cong_document_node_make_orphan(CongDocument *doc, CongNodePtr node)
{
#if DEBUG_MVC
	g_message("cong_document_node_make_orphan");
#endif

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[NODE_MAKE_ORPHAN], 0,
		       node);
}

void cong_document_node_add_after(CongDocument *doc, CongNodePtr node, CongNodePtr older_sibling)
{
	g_return_if_fail(doc);
	g_return_if_fail(node);

	#if DEBUG_MVC
	g_message("cong_document_node_add_after");
	#endif

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[NODE_ADD_AFTER], 0,
		       node,
		       older_sibling);
}

void cong_document_node_add_before(CongDocument *doc, CongNodePtr node, CongNodePtr younger_sibling)
{
	GList *iter;

	g_return_if_fail(doc);
	g_return_if_fail(node);

	#if DEBUG_MVC
	g_message("cong_document_node_add_before");
	#endif

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[NODE_ADD_BEFORE], 0,
		       node,
		       younger_sibling);
}

void cong_document_node_set_parent(CongDocument *doc, CongNodePtr node, CongNodePtr adoptive_parent)
{
	GList *iter;

	g_return_if_fail(doc);
	g_return_if_fail(node);

	#if DEBUG_MVC
	g_message("cong_document_node_set_parent");
	#endif

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[NODE_SET_PARENT], 0,
		       node,
		       adoptive_parent);
}

void cong_document_node_set_text(CongDocument *doc, CongNodePtr node, const xmlChar *new_content)
{
	GList *iter;

	g_return_if_fail(doc);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_MVC
	g_message("cong_document_node_set_text");
	#endif

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[NODE_SET_TEXT], 0,
		       node,
		       new_content);
}

void cong_document_node_set_attribute(CongDocument *doc, CongNodePtr node, const xmlChar *name, const xmlChar *value)
{
	g_return_if_fail(doc);
	g_return_if_fail(node);
	g_return_if_fail(name);
	g_return_if_fail(value);

	#if DEBUG_MVC
	g_message("cong_document_node_set_attribute");
	#endif

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[NODE_SET_ATTRIBUTE], 0,
		       node,
		       name,
		       value);
}

void cong_document_node_remove_attribute(CongDocument *doc, CongNodePtr node, const xmlChar *name)
{
	g_return_if_fail(doc);
	g_return_if_fail(node);
	g_return_if_fail(name);

	#if DEBUG_MVC
	g_message("cong_document_node_remove_attribute");
	#endif

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc),
		       signals[NODE_REMOVE_ATTRIBUTE], 0,
		       node,
		       name);
}

void cong_document_on_selection_change(CongDocument *doc)
{
	GList *iter;

	g_return_if_fail(doc);

	#if DEBUG_MVC
	g_message("cong_document_on_selection_change");
	#endif

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

void cong_document_on_cursor_change(CongDocument *doc)
{
	GList *iter;

	g_return_if_fail(doc);

#if 0
	#if DEBUG_MVC
	g_message("cong_document_node_on_cursor_change");
	#endif
#endif

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

/* end of MVC user hooks */



void cong_document_tag_remove(CongDocument *doc, CongNodePtr x)
{
	GList *iter;

	g_return_if_fail(doc);
	g_return_if_fail(x);

	#if DEBUG_MVC
	g_message("cong_document_tag_remove");
	#endif

	xml_tag_remove(doc, x); /* this is now a compound operation */
}

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

	return &PRIVATE(doc)->curs;
}

CongSelection* cong_document_get_selection(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return &PRIVATE(doc)->selection;
}

PangoLanguage*
cong_document_get_language_for_node(CongDocument *doc, 
				    CongNodePtr node)
{
	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(node, NULL);

	return NULL; /* for now */
}

void cong_document_delete_selection(CongDocument *doc)
{
	CongNodePtr t;
	CongSelection *selection;
	CongCursor *curs;

	g_return_if_fail(doc);

#if 0
	CONG_DO_UNIMPLEMENTED_DIALOG(NULL,
				     "Deletion of selection");
#else
	cong_selection_delete(cong_document_get_selection(doc), doc);
#endif
}

static gboolean cong_document_for_each_node_recurse(CongDocument *doc, CongNodePtr node, CongDocumentRecursionCallback callback, gpointer user_data, guint recursion_level)
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

gboolean cong_document_for_each_node(CongDocument *doc, CongDocumentRecursionCallback callback, gpointer user_data)
{
	g_return_val_if_fail (doc, TRUE);
	g_return_val_if_fail (callback, TRUE);

	return cong_document_for_each_node_recurse (doc,
						    (CongNodePtr)cong_document_get_xml(doc), 
						    callback, 
						    user_data, 
						    0);
}

static gboolean merge_adjacent_text_callback(CongDocument *doc, CongNodePtr node, gpointer user_data, guint recursion_level)
{
	/* We have to "look behind" at the previous sibling, since the iteration moes forward: */
	if (node->prev) {
		if (cong_node_type(node)==CONG_NODE_TYPE_TEXT) {
			if (cong_node_type(node->prev)==CONG_NODE_TYPE_TEXT) {
				/* Merge preceding node's text into this one, then delete it: */
				gchar *new_text = g_strdup_printf("%s%s", node->prev->content, node->content);

				cong_document_node_set_text (doc, node, new_text);
				g_free (new_text);

				cong_document_tag_remove (doc, node->prev);
			}			
		}
	}

	/* Keep going: */
	return FALSE;
}

void cong_document_merge_adjacent_text_nodes(CongDocument *doc)
{
	g_return_if_fail (doc);

	cong_document_begin_edit (doc);

	cong_document_for_each_node (doc, merge_adjacent_text_callback, NULL);

	cong_document_end_edit (doc);

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
	g_message("coeng_document_node_handle_set_attribute");
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



