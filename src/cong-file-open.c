/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <string.h>
#include <gtk/gtk.h>
#include "global.h"
#include "cong-dispspec.h"
#include "cong-dispspec-registry.h"
#include "cong-document.h"
#include "cong-error-dialog.h"

#include <unistd.h> /* for chdir */

#include <libxml/tree.h>
#include <libxml/parserInternals.h>

/* Towards improved error handling: */
enum CongIssueType
{
	CONG_ISSUE_ERROR,
	CONG_ISSUE_WARNING
};

typedef struct CongParserIssue
{
	enum CongIssueType type;
	gchar *filename;
	int linenum;
	gchar *description;
} CongParserIssue;

typedef struct CongParserResult
{
	GnomeVFSURI *file_uri;
	const char *buffer;
	GnomeVFSFileSize size;
#if 0
	xmlDocPtr doc;
#endif
	GSList *issues; /* list of CongParserIssues */

	GtkWindow *parent_window;
} CongParserResult;

void cong_parser_result_add_issue(CongParserResult *result, enum CongIssueType type, int linenum, gchar *description)
{
	CongParserIssue *issue;

	g_return_if_fail(result);
	g_return_if_fail(description);

	issue = g_new0(CongParserIssue,1);

	issue->type = type;
	issue->linenum = linenum;
	issue->description = g_strdup(description);

	result->issues = g_slist_append(result->issues, (gpointer)issue);

	/* FIXME: we never release these at the moment */
}

CongParserResult *global_parser_result=NULL;

void on_issue(CongParserResult *result, enum CongIssueType type, xmlParserCtxtPtr ctxt, gchar* description)
{
	g_return_if_fail(description);

	if (ctxt != NULL) {
		xmlParserInputPtr input = ctxt->input;
		if ((input != NULL) && (input->filename == NULL) &&
		    (ctxt->inputNr > 1)) {
			input = ctxt->inputTab[ctxt->inputNr - 2];
		}

		if (input) {
			cong_parser_result_add_issue(result, type, input->line, description);
		} else {
			cong_parser_result_add_issue(result, type, -1, description);
		}		
	}
}

/* macro adapted from libxml's error.c */
#define CONG_GET_VAR_STR(msg, str) {				\
    int       size;						\
    int       chars;						\
    char      *larger;						\
    va_list   ap;						\
								\
    str = (char *) g_malloc(150);				\
								\
    size = 150;							\
								\
    while (1) {							\
	va_start(ap, msg);					\
  	chars = vsnprintf(str, size, msg, ap);			\
	va_end(ap);						\
	if ((chars > -1) && (chars < size))			\
	    break;						\
	if (chars > -1)						\
	    size += chars + 1;					\
	else							\
	    size += 100;					\
	if ((larger = (char *) g_realloc(str, size)) == NULL) { \
	    g_free(str);					\
	    return;						\
	}							\
	str = larger;						\
    }								\
}

void on_sax_error(void *ctx, const char *msg, ...)
{
	char *str;
	printf("on_sax_error\n");

	g_assert(global_parser_result);

	CONG_GET_VAR_STR(msg,str)

	on_issue(global_parser_result, CONG_ISSUE_ERROR, ctx, str);

	g_free(str);
}

void on_sax_warning(void *ctx, const char *msg, ...)
{
	char *str;
	printf("on_sax_warning\n");

	g_assert(global_parser_result);

	CONG_GET_VAR_STR(msg,str)

	on_issue(global_parser_result, CONG_ISSUE_WARNING, ctx, str);

	g_free(str);
}

enum
{
	PARSER_ERROR_LINENUM_NUMERIC_COLUMN,
	PARSER_ERROR_LINENUM_TEXT_COLUMN,
	PARSER_ERROR_DESCRIPTION_TEXT_COLUMN,
	PARSER_ERROR_NUM_COLUMNS
};

/* Data and callback for showing the details of a parser error: */

typedef struct CongErrorReport
{
	CongParserResult *result;
	GtkListStore *store;
	GtkTextBuffer *text_buffer;
	GtkWidget *text_view;
} CongErrorReport;

/* The "row-activated" signal */

void  on_row_activated(GtkTreeView *treeview,
		       GtkTreePath *arg1,
		       GtkTreeViewColumn *arg2,
		       gpointer user_data)
{
	CongErrorReport *report = (CongErrorReport*)user_data;
	GtkTreeIter tree_iter;
	GtkTextIter text_iter;

	printf("on_row_activated\n");

	if ( gtk_tree_model_get_iter(GTK_TREE_MODEL(report->store), &tree_iter, arg1) ) {

		gint line_number;

		gtk_tree_model_get(GTK_TREE_MODEL(report->store), &tree_iter, PARSER_ERROR_LINENUM_NUMERIC_COLUMN, &line_number, -1);
		
		gtk_text_buffer_get_iter_at_line(report->text_buffer,
						 &text_iter,
						 line_number);

		gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(report->text_view),
                                             &text_iter,
                                             0.1, /* gdouble within_margin, */
                                             FALSE, /* gboolean use_align, */
                                             0.5, /* gdouble xalign, */
                                             0.5 /* gdouble yalign */);

		/* FIXME:  better to highlight the line; how do we do this? */
		/* FIXME: would this better to do on single-click, rather than double-click? */
	}
}

void on_parser_error_details(gpointer data)
{
	CongParserResult *result = (CongParserResult*)data;

	GtkWidget *dialog;
	gchar *title, *filename;
	GtkWidget *text_view;
	GtkTextBuffer *text_buffer;
	GtkWidget *scrolled_window;
	GtkWidget *scrolled_window2;
	GtkListStore *store;
	GtkWidget *error_list_view;
	GtkTreeViewColumn *column;
	GtkCellRenderer *text_renderer;
	CongErrorReport report;

	printf("on_parser_error_details\n");

	g_assert(result);
	g_assert(result->file_uri);

	filename = gnome_vfs_uri_extract_short_name(result->file_uri);
	title = g_strdup_printf("Parse errors loading %s", filename);

	g_free(filename);

	dialog = gtk_dialog_new_with_buttons(title,
					     result->parent_window,
					     0,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	g_free(title);


	text_view = gtk_text_view_new ();

	text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

#if 1
	gtk_text_buffer_set_text (text_buffer, result->buffer, result->size);
#else
	gtk_text_buffer_set_text (text_buffer, "Hello, this is some text", -1);
#endif


	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
		
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   scrolled_window);
	
			
	gtk_container_add (GTK_CONTAINER (scrolled_window),
			   text_view);


	store = gtk_list_store_new (PARSER_ERROR_NUM_COLUMNS,
				    G_TYPE_INT,
				    G_TYPE_STRING,
				    G_TYPE_STRING);

	/* Populate the store: */
	{
		GSList *iter = result->issues;
		
		while (iter) {
			GtkTreeIter tree_iter;
			CongParserIssue *issue = (CongParserIssue*)iter->data;

			char tmp[256];
			sprintf(tmp,"%i",issue->linenum);

			gtk_list_store_insert_before(store,&tree_iter, NULL);

			gtk_list_store_set(store,
					   &tree_iter, 
					   PARSER_ERROR_LINENUM_NUMERIC_COLUMN, issue->linenum,
					   PARSER_ERROR_LINENUM_TEXT_COLUMN, tmp,
					   PARSER_ERROR_DESCRIPTION_TEXT_COLUMN, issue->description,
					   -1);

			iter = g_slist_next(iter);
		}
	}

	error_list_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

	
	g_object_unref (G_OBJECT (store));

	text_renderer = gtk_cell_renderer_text_new ();

	column = gtk_tree_view_column_new_with_attributes ("Line Number", text_renderer,
							   "text", PARSER_ERROR_LINENUM_TEXT_COLUMN,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (error_list_view), column);

	column = gtk_tree_view_column_new_with_attributes ("Description", text_renderer,
							   "text", PARSER_ERROR_DESCRIPTION_TEXT_COLUMN,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (error_list_view), column);


	report.result = result;
	report.store = store;
	report.text_buffer = text_buffer;
	report.text_view = text_view;
	gtk_signal_connect(GTK_OBJECT(error_list_view),
			   "row-activated",
			   GTK_SIGNAL_FUNC(on_row_activated),
			   &report);

	scrolled_window2 = gtk_scrolled_window_new(NULL, NULL);
		
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   scrolled_window2);
	
			
	gtk_container_add (GTK_CONTAINER (scrolled_window2),
			   error_list_view);				

#if 1
	gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

	gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 400);
#endif



	gtk_widget_show_all(dialog);
#if 1
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
#endif
	
}

static GtkDialog*
cong_error_dialog_new_file_open_failed_from_parser_error(const GnomeVFSURI* file_uri, 
							 CongParserResult *parser_result)
{
	GtkDialog* dialog = NULL;

	gchar* app_name = cong_error_get_appname();
	
	gchar* why_failed = g_strdup_printf("%s cannot understand the internal format of the file.", app_name);

	g_assert(parser_result);

	dialog =  cong_error_dialog_new_file_open_failed_with_convenience(parser_result->parent_window,
									  file_uri, 
									  FALSE,
									  why_failed,
									  "FIXME",
									  "Show Details",
									  on_parser_error_details,
									  parser_result);

	g_free(why_failed);
	g_free(app_name);

	return dialog;
}

static xmlDocPtr parse_buffer(const char* buffer, GnomeVFSFileSize size, GnomeVFSURI* file_uri, GtkWindow *parent_window)
{
#if 1
	xmlDocPtr ret;
	xmlParserCtxtPtr ctxt;

	CongParserResult parser_result;

	g_return_val_if_fail(buffer, NULL);
	g_return_val_if_fail(file_uri, NULL);

	parser_result.buffer=buffer;
	parser_result.size=size;
	parser_result.file_uri=file_uri;
	parser_result.issues=NULL;
	parser_result.parent_window=parent_window;
	
	ctxt = xmlCreateMemoryParserCtxt(buffer, size);
	if (ctxt == NULL) return(NULL);
	
	g_assert(ctxt->sax);
	ctxt->sax->error=on_sax_error;
	ctxt->sax->warning=on_sax_warning;
	ctxt->loadsubset = TRUE; /* try to get DTDs to be loaded */

	xmlCatalogSetDebug(TRUE);

	g_assert(global_parser_result==NULL);

	global_parser_result = &parser_result;

	xmlParseDocument(ctxt);

	global_parser_result = NULL;

	if (ctxt->wellFormed) {
		ret = ctxt->myDoc;
	} else {
		GtkDialog* dialog = cong_error_dialog_new_file_open_failed_from_parser_error(file_uri,
											     &parser_result);
	
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));

		/* FIXME: It will often be possible to continue; we should give the user the option of carrying on with what we've got */

		ret = NULL;
		xmlFreeDoc(ctxt->myDoc);
		ctxt->myDoc = NULL;

	}
	xmlFreeParserCtxt(ctxt);

	
	return(ret);
#else
	xmlDocPtr doc = xmlParseMemory(buffer, size);

	return doc;
#endif

	
}

/* Data and callback for handling the forced loading of a file, autogenerating a dispspec: */
struct force_dialog
{
	int was_forced;
	xmlDocPtr doc;
	CongDispspec *ds;
};

void force_load(gpointer data)
{
	struct force_dialog *the_dlg = (struct force_dialog*)data;

	g_assert(the_dlg);
	g_assert(the_dlg->doc);

	the_dlg->was_forced=TRUE;
	the_dlg->ds = cong_dispspec_new_from_xml_file(the_dlg->doc);

	g_assert(the_dlg->ds);
}

CongDispspec* query_for_forced_dispspec(gchar *what_failed, xmlDocPtr doc, GtkWindow *parent_window)
{
	GtkDialog *dialog;
	struct force_dialog the_dlg;
	the_dlg.was_forced=FALSE;
	the_dlg.doc=doc;


	dialog = cong_error_dialog_new_with_convenience(parent_window,
							what_failed,
							"The internal structure of the document does not match any of the types known to Conglomerate.", 
							"You can force Conglomerate to load the document by clicking on the \"Force\" button below, but results may not be ideal.",
							"Force",
							force_load,
							&the_dlg);
	
	cong_error_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));

	if (the_dlg.was_forced) {
		/* We carry on, with the auto-generated dispspec: */
		return the_dlg.ds;
		/* FIXME:  This will eventually leak; the ds is never released and is never part of the global registry */
	} else {
		return NULL;
	}
}

void open_document_do(const gchar* doc_name, GtkWindow *parent_window)
{
	char *p;
	FILE *xml_f;
	CongDispspec *ds;
	CongDocument *cong_doc;
	xmlDocPtr doc = NULL;

	/* Use libxml to load the doc: */
	{
		GnomeVFSURI* file_uri = gnome_vfs_uri_new(doc_name);

		/* Load using GnomeVFS: */
		{
			char* buffer;
			GnomeVFSFileSize size;
			GnomeVFSResult vfs_result = cong_vfs_new_buffer_from_file(doc_name, &buffer, &size);

			if (vfs_result!=GNOME_VFS_OK) {
				GtkDialog* dialog = cong_error_dialog_new_file_open_failed_from_vfs_result(parent_window,
													   file_uri, 
													   vfs_result);
			
				cong_error_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(GTK_WIDGET(dialog));

				gnome_vfs_uri_unref(file_uri);

				return;
			}

			g_assert(buffer);

			/* Parse the file from the buffer: */
			doc = parse_buffer(buffer, size, file_uri, parent_window);

			g_free(buffer);
		}


		if (NULL==doc) {
			gnome_vfs_uri_unref(file_uri);
			return;
		}

		ds = cong_dispspec_registry_get_appropriate_dispspec(doc);

		if (ds==NULL) {
			gchar *what_failed;

			what_failed = cong_error_what_failed_on_file_open_failure(file_uri, FALSE);

			ds = query_for_forced_dispspec(what_failed, doc, parent_window);
			
			g_free(what_failed);

			if (NULL==ds) {
				xmlFreeDoc(doc);
				gnome_vfs_uri_unref(file_uri);
				return;
			}
		}

		gnome_vfs_uri_unref(file_uri);
		
	}

	g_assert(ds);
	cong_doc = cong_document_new_from_xmldoc(doc, ds, doc_name); /* takes ownership of doc */

	cong_node_self_test_recursive(cong_document_get_root(cong_doc));

	g_assert(cong_doc);

#if 1
	cong_primary_window_new(cong_doc);
	cong_document_unref(cong_doc);
#else
	the_globals.xv = xmlview_new(cong_doc);
	gtk_box_pack_start(GTK_BOX(cong_gui_get_root(&the_gui)), the_globals.xv->w, FALSE, FALSE, 0);
#endif

}



void open_document(GtkWindow *parent_window)
{
	char *doc_name;

	g_return_if_fail(parent_window);
	
	doc_name = cong_get_file_name("Select an XML document",
				      NULL,
				      parent_window);

	if (!doc_name) {
		return;
	}

	open_document_do(doc_name, parent_window);

	g_free(doc_name);
}

gint toolbar_callback_open(GtkWidget *widget, gpointer data)
{
	CongPrimaryWindow *primary_window = data;

	open_document(cong_primary_window_get_toplevel(primary_window));

	return TRUE;
}
