/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gtk/gtk.h>

#ifdef ENABLE_GTKSOURCEVIEW
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#endif

#include "global.h"
#include "cong-dispspec.h"
#include "cong-dispspec-registry.h"
#include "cong-document.h"
#include "cong-error-dialog.h"
#include "cong-parser-error.h"
#include "cong-util.h"
#include "cong-vfs.h"

#include <libxml/catalog.h>

/**
 * cong_parser_result_add_issue:
 * @result:
 * @type:
 * @linenum:
 * @description:
 *
 * TODO: Write me
 */
void 
cong_parser_result_add_issue(CongParserResult *result, CongIssueType type, int linenum, gchar *description)
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

#include <libxml/tree.h>
#include <libxml/parserInternals.h>

CongParserResult *global_parser_result=NULL;

/**
 * on_issue:
 * @result:
 * @type:
 * @ctxt:
 * @description:
 *
 * TODO: Write me
 */
void 
on_issue(CongParserResult *result, CongIssueType type, xmlParserCtxtPtr ctxt, gchar* description)
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

/**
 * on_sax_error:
 * @ctx:
 * @msg:
 * @...: FIXME: Check how gtk-doc wants to handle var-args
 *
 * TODO: Write me
 */
void 
on_sax_error(void *ctx, const char *msg, ...)
{
	gchar *str;
	printf("on_sax_error\n");

	g_assert(global_parser_result);

	CONG_GET_VAR_STR(msg,str)

	on_issue(global_parser_result, CONG_ISSUE_ERROR, ctx, str);

	g_free(str);
}

/**
 * on_sax_warning:
 * @ctx:
 * @msg:
 * @...: FIXME: Check how gtk-doc wants to handle var args
 *
 * TODO: Write me
 */
void 
on_sax_warning(void *ctx, const char *msg, ...)
{
	gchar *str;
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
	CongParserResult *parser_result;
	GtkListStore *store;
#ifdef ENABLE_GTKSOURCEVIEW
	GtkSourceBuffer *text_buffer;
#else
        GtkTextBuffer *text_buffer;
#endif
	GtkWidget *text_view;
} CongErrorReport;

static void        
on_selection_changed (GtkTreeSelection *treeselection,
		      CongErrorReport *report)
{
	GtkTreeIter tree_iter;
	GtkTextIter text_iter;
	GtkTextIter text_iter_next_line;

	g_message("on_selection_changed");

	if ( gtk_tree_selection_get_selected (treeselection, NULL, &tree_iter) ) {

		gint line_number;

		gtk_tree_model_get(GTK_TREE_MODEL(report->store), &tree_iter, PARSER_ERROR_LINENUM_NUMERIC_COLUMN, &line_number, -1);

		/* Error reporting from libxml2 has line numbers starting at 1; gtk_text_buffer lines start at 0 */
		g_assert (line_number>=1);
		
		gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (report->text_buffer),
						  &text_iter,
						  line_number-1);
		gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (report->text_buffer),
						  &text_iter_next_line,
						  line_number);

		gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(report->text_view),
                                             &text_iter,
                                             0.1, /* gdouble within_margin, */
                                             FALSE, /* gboolean use_align, */
                                             0.5, /* gdouble xalign, */
                                             0.5 /* gdouble yalign */);
		gtk_text_buffer_select_range (GTK_TEXT_BUFFER (report->text_buffer),
					      &text_iter,
					      &text_iter_next_line);
	}
}

enum {
	CONG_PARSER_ERROR_RESPONSE_RETRY = 1
};

/**
 * cong_parser_result_dialog_new:
 * @parser_result:
 *
 * TODO: Write me
 * Returns:
 */
GtkDialog *
cong_parser_result_dialog_new(CongParserResult *parser_result)
{
	GtkWidget *dialog;
	gchar *title, *filename;
	GtkWidget *text_view;
#ifdef ENABLE_GTKSOURCEVIEW
        GtkSourceBuffer *text_buffer;
        GtkSourceLanguageManager *lang_manager;
        GtkSourceLanguage *lang;
#else
        GtkTextBuffer *text_buffer;
#endif
	GtkWidget *scrolled_window;
	GtkWidget *scrolled_window2;
	GtkListStore *store;
	GtkWidget *error_list_view;
	GtkTreeViewColumn *column;
	GtkCellRenderer *text_renderer;
	CongErrorReport *report;

	g_return_val_if_fail(parser_result, NULL);

	g_assert(parser_result);
	g_assert(parser_result->file);

	filename = cong_vfs_extract_short_name (parser_result->file);

	title = g_strdup_printf(_("Parse errors loading %s"), filename);

	g_free(filename);

	dialog = gtk_dialog_new_with_buttons(title,
					     parser_result->parent_window,
					     0,
#if 0
					     _("_Retry"),
					     CONG_PARSER_ERROR_RESPONSE_RETRY,
#endif
					     GTK_STOCK_CANCEL,
					     GTK_RESPONSE_CANCEL,
					     NULL);

	g_free(title);


#ifdef ENABLE_GTKSOURCEVIEW
        lang_manager = gtk_source_language_manager_get_default();
        lang = gtk_source_language_manager_get_language(lang_manager, "xml");
        text_buffer = gtk_source_buffer_new_with_language(lang);
        text_view = gtk_source_view_new_with_buffer(text_buffer);
        gtk_source_buffer_set_highlight_syntax(text_buffer, TRUE);
        gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(text_view), TRUE);
#else
	text_view = gtk_text_view_new ();
	text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
#endif

#if 1
	gtk_text_buffer_set_text (GTK_TEXT_BUFFER(text_buffer), parser_result->buffer, parser_result->size);
#else
	gtk_text_buffer_set_text (GTK_TEXT_BUFFER(text_buffer), "Hello, this is some text", -1);
#endif

        gtk_text_view_set_editable(GTK_TEXT_VIEW (text_view), FALSE);
        gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW (text_view), FALSE);

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
		GSList *iter = parser_result->issues;
		
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

	column = gtk_tree_view_column_new_with_attributes (_("Line Number"), text_renderer,
							   "text", PARSER_ERROR_LINENUM_TEXT_COLUMN,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (error_list_view), column);

	column = gtk_tree_view_column_new_with_attributes (_("Description"), text_renderer,
							   "text", PARSER_ERROR_DESCRIPTION_TEXT_COLUMN,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (error_list_view), column);

	/* FIXME: this will leak memory */
	report = g_new0(CongErrorReport,1);
	report->parser_result = parser_result;
	report->store = store;
	report->text_buffer = text_buffer;
	report->text_view = text_view;

	g_signal_connect (G_OBJECT(gtk_tree_view_get_selection (GTK_TREE_VIEW (error_list_view))),
			  "changed",
			  G_CALLBACK (on_selection_changed),
			  report);

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

	return GTK_DIALOG(dialog);
}

/**
 * on_parser_error_details:
 * @data:
 *
 * TODO: Write me
 */
void 
on_parser_error_details(gpointer data)
{
	GtkDialog * dialog;

	g_message("on_parser_error_details");

	dialog = cong_parser_result_dialog_new((CongParserResult*)data);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

/**
 * cong_error_dialog_new_file_open_failed_from_parser_error:
 * @file:
 * @parser_result:
 *
 * TODO: Write me
 * Returns:
 */
GtkDialog*
cong_error_dialog_new_file_open_failed_from_parser_error (GFile *file,
							  CongParserResult *parser_result)
{
	GtkDialog* dialog = NULL;

	gchar* app_name = cong_error_get_appname();
	
	gchar* why_failed = g_strdup_printf(_("%s cannot understand the internal format of the file."), app_name);

	g_assert(parser_result);

	dialog = cong_error_dialog_new_from_file_open_failure_with_convenience (parser_result->parent_window,
										file,
										FALSE,
										why_failed,
										_("Conglomerate currently requires documents to be \"well-formed\"; it has much stricter rules than most web browsers.  It also does not yet support SGML.  We hope to fix these problems in a later release."),
										_("Show Details"),
										on_parser_error_details,
										parser_result);
	
	g_free(why_failed);
	g_free(app_name);

	return dialog;
}

/**
 * cong_ui_parse_buffer:
 * @buffer:
 * @size:
 * @string_uri:
 * @parent_window:
 *
 * TODO: Write me
 * Returns:
 */
xmlDocPtr 
cong_ui_parse_buffer (const char* buffer,
		      gsize size,
		      GFile *file,
		      GtkWindow *parent_window)
{
#if 1
	xmlDocPtr ret;
	xmlParserCtxtPtr ctxt;

	CongParserResult parser_result;

	g_return_val_if_fail(buffer, NULL);
	g_return_val_if_fail(file, NULL);

	parser_result.buffer=buffer;
	parser_result.size=size;
	parser_result.issues=NULL;
	parser_result.parent_window=parent_window;

	parser_result.file = g_file_dup(file);
	
	ctxt = cong_parse_from_memory (buffer, 
				       size,
				       &parser_result);

	if (ctxt->wellFormed) {
		ret = ctxt->myDoc;
	} else {
		GtkDialog* dialog = cong_error_dialog_new_file_open_failed_from_parser_error (file,
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

/**
 * cong_parse_from_filename:
 * @string_uri:
 * @parser_result:
 *
 * TODO: Write me
 * Returns:
 */
xmlParserCtxtPtr
cong_parse_from_filename (const gchar* string_uri,
			  CongParserResult *parser_result)
{
	xmlParserCtxtPtr ctxt;

	g_return_val_if_fail (string_uri, NULL);
	g_return_val_if_fail (parser_result, NULL);

	ctxt = xmlCreateFileParserCtxt	(string_uri);
	if (ctxt == NULL) return(NULL);
	
	g_assert(ctxt->sax);
	ctxt->sax->error=on_sax_error;
	ctxt->sax->warning=on_sax_warning;
	ctxt->loadsubset = TRUE; /* try to get DTDs to be loaded */
	
	xmlCatalogSetDebug(TRUE);
	
	g_assert(global_parser_result==NULL);
	
	global_parser_result = parser_result;
	
	xmlParseDocument(ctxt);
	
	global_parser_result = NULL;		

	return ctxt;
}

/**
 * cong_parse_from_memory:
 * @buffer:
 * @size:
 * @parser_result:
 *
 * TODO: Write me
 * Returns:
 */
xmlParserCtxtPtr
cong_parse_from_memory (const char* buffer, 
			gsize size,
			CongParserResult *parser_result)
{
	xmlParserCtxtPtr ctxt;

	g_return_val_if_fail (buffer, NULL);
	g_return_val_if_fail (parser_result, NULL);

	ctxt = xmlCreateMemoryParserCtxt(buffer, size);
	if (ctxt == NULL) return(NULL);
	
	g_assert(ctxt->sax);
	ctxt->sax->error=on_sax_error;
	ctxt->sax->warning=on_sax_warning;
	ctxt->loadsubset = TRUE; /* try to get DTDs to be loaded */

	xmlCatalogSetDebug(TRUE);

	g_assert(global_parser_result==NULL);

	global_parser_result = parser_result;

	xmlParseDocument(ctxt);

	global_parser_result = NULL;

	return ctxt;
}
