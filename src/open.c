/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <string.h>
#include <gtk/gtk.h>
#include <ttree.h>
#include <xml.h>
#include "global.h"

#include <unistd.h> /* for chdir */

#include <libxml/tree.h>
#include <libxml/parserInternals.h>

#if 1
/* New implementation: */

gchar*
get_toplevel_tag(xmlDocPtr doc)
{
	xmlNodePtr xml_toplevel;

	g_return_val_if_fail(doc,NULL);
	
	for (xml_toplevel=doc->children; xml_toplevel; xml_toplevel = xml_toplevel->next) {
		if (xml_toplevel->type==XML_ELEMENT_NODE) {
			return g_strdup(xml_toplevel->name);
		}
	}

	return NULL;
}

/**
 * Routine to figure out an appropriate dispspec for use with this file.
 * Looks for a DTD; if found, it looks up the DTD in a mapping.
 * If this fails, it looks at the top-level tag and makes a guess, but asks the user for confirmation.
 * If this fails, it asks the user.
 */
const CongDispspec*
get_appropriate_dispspec(xmlDocPtr doc)
{
	gchar* toplevel_tag;

	g_return_val_if_fail(doc,NULL);

	/* FIXME: check for a DTD */
#if 0
	if (doc->) {
	}
#endif

	toplevel_tag = get_toplevel_tag(doc);

	if (toplevel_tag) {
		int i;
		
		g_message("Searching for a match against top-level tag <%s>\n", toplevel_tag);

		for (i=0;i<cong_dispspec_registry_get_num(the_globals.ds_registry);i++) {
			const CongDispspec* ds = cong_dispspec_registry_get(the_globals.ds_registry, i);

			CongDispspecElement* element = cong_dispspec_lookup_element(ds, toplevel_tag);
			
			if (element) {
				/* FIXME: check for appropriateness */
				g_free(toplevel_tag);
				return ds;
			}
		}
		
		/* No match */

		g_free(toplevel_tag);
	}

	/* Do a selection dialog for the user (including the ability to generate a new dispspec): */

	return NULL;
}

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
					     NULL,
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
				
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   error_list_view
			   );
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

GtkDialog*
cong_error_dialog_new_file_open_failed_from_parser_error(const GnomeVFSURI* file_uri, CongParserResult *parser_result)
{
	GtkDialog* dialog = NULL;

	gchar* app_name = cong_error_get_appname();
	
	gchar* why_failed = g_strdup_printf("%s cannot understand the internal format of the file.", app_name);

	g_assert(parser_result);

	dialog =  cong_error_dialog_new_file_open_failed_with_convenience(file_uri, FALSE,
									  why_failed,
									  "FIXME",
									  "Show Details",
									  on_parser_error_details,
									  parser_result);

	g_free(why_failed);
	g_free(app_name);

	return dialog;
}

xmlDocPtr parse_buffer(const char* buffer, GnomeVFSFileSize size, GnomeVFSURI* file_uri)
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
	
	ctxt = xmlCreateMemoryParserCtxt(buffer, size);
	if (ctxt == NULL) return(NULL);
	
	g_assert(ctxt->sax);
	ctxt->sax->error=on_sax_error;
	ctxt->sax->warning=on_sax_warning;

	g_assert(global_parser_result==NULL);

	global_parser_result = &parser_result;

	xmlParseDocument(ctxt);

	global_parser_result = NULL;

	if (ctxt->wellFormed) {
		ret = ctxt->myDoc;
	} else {
		GtkDialog* dialog = cong_error_dialog_new_file_open_failed_from_parser_error(file_uri, &parser_result);
	
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

void open_document_do(const gchar* doc_name)
{
	char *p;
	TTREE *xml_in;
	FILE *xml_f;
	CongDispspec *ds;

	/* Use libxml to load the doc: */
	{
		xmlDocPtr doc=NULL;

		GnomeVFSURI* file_uri = gnome_vfs_uri_new(doc_name);

		/* Load using GnomeVFS: */
		{
			char* buffer;
			GnomeVFSFileSize size;
			GnomeVFSResult vfs_result = cong_vfs_new_buffer_from_file(doc_name, &buffer, &size);

			if (vfs_result!=GNOME_VFS_OK) {
				GtkDialog* dialog = cong_error_dialog_new_file_open_failed_from_vfs_result(file_uri, vfs_result);
			
				cong_error_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(GTK_WIDGET(dialog));

				gnome_vfs_uri_unref(file_uri);

				return;
			}

			g_assert(buffer);

			/* Parse the file from the buffer: */
			doc = parse_buffer(buffer, size, file_uri);

			g_free(buffer);
		}


		if (NULL==doc) {
			gnome_vfs_uri_unref(file_uri);
			return;
		}

		ds = get_appropriate_dispspec(doc);

		if (ds==NULL) {
			GtkDialog *dialog;
			gchar *what_failed;
			struct force_dialog the_dlg;
			the_dlg.was_forced=FALSE;
			the_dlg.doc=doc;

			what_failed = cong_error_what_failed_on_file_open_failure(file_uri, FALSE);

			dialog = cong_error_dialog_new_with_convenience(what_failed,
									"The internal structure of the document does not match any of the types known to Conglomerate.", 
									"You can force Conglomerate to load the document by clicking on the \"Force\" button below, but results may not be ideal.",
									"Force",
									force_load,
									&the_dlg);

			g_free(what_failed);

			cong_error_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(GTK_WIDGET(dialog));

			if (the_dlg.was_forced) {
				/* We carry on, with the auto-generated dispspec: */
				ds = the_dlg.ds;
				/* FIXME:  This will eventually leak; the ds is never released and is never part of the global registry */
			} else {
				xmlFreeDoc(doc);
				gnome_vfs_uri_unref(file_uri);
				return;
			}
		}

		gnome_vfs_uri_unref(file_uri);

		if (doc->intSubset) {
			xmlDebugDumpDTD(stdout,  doc->intSubset);
		} else {
			printf("NULL interior subset DTD\n");
		}
		if (doc->extSubset) {
			xmlDebugDumpDTD(stdout,  doc->extSubset);
		} else {
			printf("NULL exterior subset DTD\n");
		}

		
		xml_in = convert_libxml_to_ttree_doc(doc);

		xmlFreeDoc(doc);
	}

	g_assert(ds);
	
	the_globals.xv = xmlview_new(cong_document_new_from_ttree(xml_in, ds));
	gtk_box_pack_start(GTK_BOX(cong_gui_get_root(&the_gui)), the_globals.xv->w, FALSE, FALSE, 0);

}

gint open_document(GtkWidget *w, gpointer data)
{
	const char *doc_name, *ds_name;
	
	doc_name = get_file_name("Select an XML document");
	if (!doc_name) return(TRUE);

	open_document_do(doc_name);
	return(TRUE);  
}
#else
int open_document_do(const char *doc_name, const char *ds_name)
{
	char *p;
	TTREE *xml_in;
	FILE *xml_f;

	the_globals.ds = cong_dispspec_new_from_ds_file(ds_name);
	if (the_globals.ds==NULL) {
	  return(TRUE);  /* Invalid displayspec. */	  
	}

	xml_f = fopen(doc_name, "rt");
	if (!xml_f) {
	  GtkWidget* dialog;

	  g_warning("Problem opening doc file \"%s\"\n", doc_name);

	  dialog = cong_error_dialog_new_file_open_failed(doc_name);
	  cong_error_dialog_run(GTK_DIALOG(dialog));
	  gtk_widget_destroy(dialog);

	  return(TRUE);
	}

	p = strrchr(doc_name, '/');
	if (p)
	{
		*p = 0;
		chdir(doc_name);
	}
	
	xml_in = xml_f_to_ttree(xml_f, 0);
	if (!xml_in) {
	  g_warning("Problem parsing doc file \"%s\"\n", doc_name);
	  return(TRUE);  /* Invalid XML document. */
	}

	fclose(xml_f);	

	xml_t_trim(xml_in);
	the_globals.xv = xmlview_new(cong_document_new_from_ttree(xml_in), the_globals.ds);
	gtk_box_pack_start(GTK_BOX(cong_gui_get_root(&the_gui)), the_globals.xv->w, FALSE, FALSE, 0);

	return (TRUE);
}

/* Old implementation: */
gint open_document(GtkWidget *w, gpointer data)
{
	const char *doc_name, *ds_name;
	
	doc_name = get_file_name("Select an XML document");
	if (!doc_name) return(TRUE);

	ds_name = get_file_name("Select a matching displayspec");
	if (!ds_name) return(TRUE);

	open_document_do(doc_name, ds_name);
	return(TRUE);
}
#endif
