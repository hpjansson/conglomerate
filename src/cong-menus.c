/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-menus.c
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
 * Based on code by Hans Petter Jansson <hpj@ximian.com>
 */

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "global.h"
#include "../config.h"
#include "cong-dispspec.h"
#include "cong-document.h"
#include "cong-error-dialog.h"
#include "cong-dialog.h"
#include "cong-plugin-manager.h"
#include "cong-service-tool.h"
#include "cong-progress-checklist.h"
#include "cong-app.h"
#include "cong-util.h"
#include "cong-command.h"
#include "cong-command-history.h"
#include "cong-selection.h"
#include "cong-range.h"
#include "cong-ui-hooks.h"

#if 1
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>
#endif

#include <libxml/tree.h>

#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>

#if PRINT_TESTS
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprintui/gnome-print-master-preview.h>
#endif

#include "cong-dispspec-registry.h"
#include <glade/glade.h>
#include "cong-primary-window.h"

#include "cong-edit-find-and-replace.h"

extern char *ilogo_xpm[];

#define ENABLE_DEBUG_MENU 0
#define ENABLE_UNIMPLEMENTED_MENUS 1

GtkWidget* make_uneditable_text(const gchar* text)
{
#if 1
	GtkWidget *widget = gtk_label_new(text);
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap(GTK_LABEL(widget), TRUE);
	return widget;
#else
	GtkEntry *entry = GTK_ENTRY(gtk_entry_new());

	gtk_entry_set_text(entry, text);
	gtk_entry_set_editable(entry, FALSE);

	return GTK_WIDGET(entry);
#endif
}



/* Handy routines for implementing menu callbacks: */
void unimplemented_menu_item(gpointer callback_data,
			     guint callback_action,
			     GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CONG_DO_UNIMPLEMENTED_DIALOG(cong_primary_window_get_toplevel(primary_window), 
				     _("The selected menu item has not yet been implemented."));
}

static void dispatch_document_command(void (*document_command)(CongDocument *doc), gpointer callback_data)
{
	CongPrimaryWindow *primary_window = callback_data;

	g_assert(document_command);
	g_assert(primary_window);

	g_return_if_fail(cong_primary_window_get_document(primary_window));

	(*document_command)(cong_primary_window_get_document(primary_window));	
}

static void dispatch_document_command2(void (*document_command)(CongDocument *doc, GtkWidget *widget), gpointer callback_data, GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;

	g_assert(document_command);
	g_assert(primary_window);

	g_return_if_fail(cong_primary_window_get_document(primary_window));

	(*document_command)(cong_primary_window_get_document(primary_window), widget);	
}


/* Callbacks for "File" menu: */
static void menu_callback_file_new(gpointer callback_data,
				   guint callback_action,
				   GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	new_document(cong_primary_window_get_toplevel(primary_window));
}

static void menu_callback_file_open(gpointer callback_data,
				    guint callback_action,
				    GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	open_document(cong_primary_window_get_toplevel(primary_window));
}

static void menu_callback_file_save(gpointer callback_data,
				    guint callback_action,
				    GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	save_document(doc, cong_primary_window_get_toplevel(primary_window));
}

static void menu_callback_file_save_as(gpointer callback_data,
				       guint callback_action,
				       GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	save_document_as(doc, cong_primary_window_get_toplevel(primary_window));
}

static void menu_callback_file_save_copy(gpointer callback_data,
					 guint callback_action,
					 GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;

	CONG_DO_UNIMPLEMENTED_DIALOG(cong_primary_window_get_toplevel(primary_window), 
				     _("The selected menu item has not yet been implemented."));
}

static void menu_callback_file_revert(gpointer callback_data,
				      guint callback_action,
				      GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	g_return_if_fail(doc);

	if (cong_document_is_modified(doc)) {
		gchar* filename;
		GtkDialog *dialog;
		gint result;

		filename = cong_document_get_filename(doc);

		dialog = cong_dialog_revert_confirmation_alert_new(cong_primary_window_get_toplevel(primary_window),
								   filename,
								   cong_document_get_seconds_since_last_save_or_load(doc));

		g_free(filename);

		result = gtk_dialog_run(dialog);

		gtk_widget_destroy(GTK_WIDGET(dialog));

		if (result != CONG_REVERT_CONFIRMATION_RESULT_REVERT) {
			return;
		}

		CONG_DO_UNIMPLEMENTED_DIALOG_WITH_BUGZILLA_ID(NULL, 
							      _("The selected menu item has not yet been implemented."),
							      118770);
	} 
}

static void menu_callback_file_import(gpointer callback_data,
				      guint callback_action,
				      GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;

	/* FIXME: this option should be disabled if there are no importers installed */

	cong_ui_hook_file_import (cong_primary_window_get_toplevel (primary_window));
}

static void menu_callback_file_export(gpointer callback_data,
				      guint callback_action,
				      GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	/* FIXME: this option should be disabled if there are no exporters installed that are appropriate for this FPI */

	cong_ui_hook_file_export (doc,
				  cong_primary_window_get_toplevel (primary_window));
}

#if ENABLE_PRINTING
static void menu_callback_file_print_preview(gpointer callback_data,
					     guint callback_action,
					     GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	/* FIXME: this option should be disabled if there are no print routines installed that are appropriate for this FPI */

	cong_ui_hook_file_print_preview (doc,
					 cong_primary_window_get_toplevel (primary_window));
}

static void menu_callback_file_print(gpointer callback_data,
				      guint callback_action,
				      GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	/* FIXME: this option should be disabled if there are no print routines installed that are appropriate for this FPI */

	cong_ui_hook_file_print (doc,
				 cong_primary_window_get_toplevel (primary_window));
}
#endif /* #if ENABLE_PRINTING */


static void menu_callback_file_properties(gpointer callback_data,
					  guint callback_action,
					  GtkWidget *widget)
{
	GtkWidget *dialog;

	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	g_return_if_fail(doc);

	dialog = cong_file_properties_dialog_new(doc,
						 cong_primary_window_get_toplevel(primary_window));

	gtk_widget_show_all(dialog);

	/* FIXME: memory leaks */
}

static void menu_callback_file_close(gpointer callback_data, 
				     guint callback_action, 
				     GtkWidget *widget) {
	CongPrimaryWindow *primary_window = callback_data;

	if (cong_primary_window_can_close(primary_window)) {
		gtk_widget_destroy(GTK_WIDGET(cong_primary_window_get_toplevel(primary_window)));
	}
}

static void menu_callback_file_quit(gpointer callback_data, 
				    guint callback_action, 
				    GtkWidget *widget) {

	CongPrimaryWindow *primary_window = callback_data;
	GList *current;
	gboolean canceled = FALSE;

	(void)primary_window; /* suppress warnings */

	current = g_list_first(cong_app_singleton()->primary_windows);

	while (current) {
		if (!cong_primary_window_can_close((struct CongPrimaryWindow*)(current->data))) {
			canceled = TRUE;
			break;
		}
		current = g_list_next(current);
	}
	
	if (!canceled) {
		/* FIXME: This is probably leaking memory by not
		   freeing the primary windows...*/
		gtk_main_quit();
	}
}

/* Callbacks for "Edit" menu: */
static void menu_callback_undo (gpointer callback_data,
				guint callback_action,
				GtkWidget *widget)
{
	dispatch_document_command(cong_document_undo, callback_data);
}

static void menu_callback_redo (gpointer callback_data,
				guint callback_action,
				GtkWidget *widget)
{
	dispatch_document_command(cong_document_redo, callback_data);
}

static void menu_callback_cut(gpointer callback_data,
			      guint callback_action,
			      GtkWidget *widget)
{
	dispatch_document_command(cong_document_cut_selection, callback_data);
}

static void menu_callback_copy(gpointer callback_data,
			       guint callback_action,
			       GtkWidget *widget)
{
	dispatch_document_command(cong_document_copy_selection, callback_data);
}

static void menu_callback_paste(gpointer callback_data,
				guint callback_action,
				GtkWidget *widget)
{
	dispatch_document_command2(cong_document_paste_clipboard_or_selection, callback_data, widget);
}

static void 
menu_callback_find (gpointer callback_data,
		    guint callback_action,
		    GtkWidget *widget)
{
	dispatch_document_command (cong_document_find, callback_data);
}
static void 
menu_callback_find_next (gpointer callback_data,
			 guint callback_action,
			 GtkWidget *widget)
{
	dispatch_document_command (cong_document_find_next, callback_data);
}
static void 
menu_callback_find_prev (gpointer callback_data,
			 guint callback_action,
			 GtkWidget *widget)
{
	dispatch_document_command (cong_document_find_prev, callback_data);
}
static void 
menu_callback_replace (gpointer callback_data,
		       guint callback_action,
		       GtkWidget *widget)
{
	dispatch_document_command (cong_document_replace, callback_data);
}

static void menu_callback_view_source(gpointer callback_data,
				guint callback_action,
				GtkWidget *widget)
{
	dispatch_document_command(cong_document_view_source, callback_data);
}

static void
menu_callback_preferences (gpointer callback_data,
			   guint callback_action,
			   GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	g_assert(primary_window);

	cong_ui_hook_edit_preferences (cong_primary_window_get_toplevel (primary_window));
}

/* Callbacks for "Debug" menu: */
#if ENABLE_DEBUG_MENU
void debug_error(CongPrimaryWindow *primary_window)
{
	cong_error_tests(cong_primary_window_get_toplevel(primary_window));
}

void menu_callback_debug_error(gpointer callback_data,
			       guint callback_action,
			       GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;

	debug_error(primary_window); 
}

void open_preview_window_for_doc(xmlDocPtr doc)
{
#if 1
	/* Save it to a temp file and invoke user's favourite browser: */
	g_assert(0);
#else
	GtkWidget* html_view;
	HtmlDocument *html_document;

	g_return_if_fail(doc);

	html_view = html_view_new();

	html_document = ;

	html_view_set_document(HTML_VIEW(html_view), html_document);
#endif
}

void open_transformed_window_for_doc(xmlDocPtr doc, 
				     GtkWindow *parent_window)
{
	/* Hackish test: */

	CongDispspec *ds;
	CongDocument *cong_doc;

	g_return_if_fail(doc);

	ds = cong_dispspec_registry_get_appropriate_dispspec (cong_app_get_dispspec_registry (cong_app_singleton()), 
							      doc,
							      NULL);

	if (ds==NULL) {
		ds = query_for_forced_dispspec(_("Conglomerate cannot open the result of the transformation"), 
					       doc,
					       parent_window,
					       NULL);

		if (NULL==ds) {
			xmlFreeDoc(doc);
			return;
		}
	}

	g_assert(ds);
	cong_doc = cong_document_new_from_xmldoc(doc, ds, NULL); /* takes ownership of doc */

	cong_node_self_test_recursive(cong_document_get_root(cong_doc));

	g_assert(cong_doc);

	cong_primary_window_new(cong_doc);

	g_object_unref(G_OBJECT(cong_doc));
}


void debug_transform(CongPrimaryWindow *primary_window,
		    const gchar *stylesheet_filename)
{
	CongDocument *doc;

	/* Hackish test of libxslt */
	xsltStylesheetPtr xsl;
	xmlDocPtr input_clone;
	xmlDocPtr result;
	GtkWindow *parent_window;

	g_return_if_fail(stylesheet_filename);


	parent_window = cong_primary_window_get_toplevel(primary_window);
#if 0
	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue(1);
#endif
	
	doc = cong_primary_window_get_document(primary_window);

	g_return_if_fail(doc);

	{
		gchar* stylesheet_path = cong_utils_get_norman_walsh_stylesheet(stylesheet_filename);
		g_assert(stylesheet_path);

		xsl = xsltParseStylesheetFile(stylesheet_filename);

		g_free(stylesheet_path);
	}

	if (NULL==xsl) {
		gchar *why_failed = g_strdup_printf(_("There was a problem reading the stylesheet file \"%s\""),stylesheet_filename);

		GtkDialog* dialog = cong_error_dialog_new(parent_window,
							  _("Conglomerate could not transform the document"),
							  why_failed,
							  "FIXME");
	
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		return;
	}

	/* DHM 14/11/2002:  document nodes seemed to being corrupted when applying the stylesheet.
	   So We now work with a clone of the document
	*/
	input_clone = xmlCopyDoc(cong_document_get_xml(doc), TRUE);
	g_assert(input_clone);

	result = xsltApplyStylesheet(xsl, input_clone, NULL);
	g_assert(result);

	xmlFreeDoc(input_clone);

	if (result->children==NULL) {
		gchar *why_failed = g_strdup_printf(_("There was a problem applying the stylesheet file"));

		GtkDialog* dialog = cong_error_dialog_new(parent_window,
							  _("Conglomerate could not transform the document"),
							  why_failed,
							  "FIXME");
	
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		return;
	}

#if 0
	open_preview_window_for_doc(result); /* takes ownership of the result */
#else
	open_transformed_window_for_doc(result, parent_window); /* takes ownership of the result */
	/* FIXME: do as a document?  or have a special preview window? */
#endif
	
	xsltFreeStylesheet(xsl);

	/* do we need to clean up the globals? */
#if 0
	xmlSubstituteEntitiesDefault(0);
	xmlLoadExtDtdDefaultValue(0);
#endif
}

#define DOCBOOK_TO_HTML_STYLESHEET_FILE ("html/docbook.xsl")
#define DOCBOOK_TO_XHTML_STYLESHEET_FILE ("xhtml/docbook.xsl")
#define DOCBOOK_TO_HTML_HELP_STYLESHEET_FILE ("htmlhelp/htmlhelp.xsl")
#define DOCBOOK_TO_JAVAHELP_STYLESHEET_FILE ("javahelp/javahelp.xsl")
#define DOCBOOK_TO_FO_STYLESHEET_FILE ("fo/docbook.xsl")

void menu_callback_debug_transform_docbook_to_html(gpointer callback_data,
				  guint callback_action,
				  GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;

	debug_transform (primary_window,
			 DOCBOOK_TO_HTML_STYLESHEET_FILE);
}
void menu_callback_debug_transform_docbook_to_xhtml(gpointer callback_data,
						  guint callback_action,
						  GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;

	debug_transform (primary_window,
			 DOCBOOK_TO_XHTML_STYLESHEET_FILE);
}
void menu_callback_debug_transform_docbook_to_html_help(gpointer callback_data,
						  guint callback_action,
						  GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;

	debug_transform (primary_window,
			 DOCBOOK_TO_HTML_HELP_STYLESHEET_FILE);
}
void menu_callback_debug_transform_docbook_to_javahelp(gpointer callback_data,
						  guint callback_action,
						  GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;

	debug_transform (primary_window,
			 DOCBOOK_TO_JAVAHELP_STYLESHEET_FILE);
}
void menu_callback_debug_transform_docbook_to_fo(gpointer callback_data,
				  guint callback_action,
				  GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;

	debug_transform (primary_window,
			 DOCBOOK_TO_FO_STYLESHEET_FILE);
}

#if PRINT_TESTS
static void
my_draw (GnomePrintContext *gpc)
{
	GnomeFont *font;

	font = gnome_font_find_closest ("Helvetica", 12);

	gnome_print_beginpage (gpc, "1");

	gnome_print_setfont (gpc, font);
	gnome_print_moveto (gpc, 100, 400);
	gnome_print_show (gpc, "Hello world\nThis is a test");

	gnome_print_moveto (gpc, 100, 200);
	gnome_print_lineto (gpc, 200, 200);
	gnome_print_stroke (gpc);

	gnome_print_showpage (gpc);
}

void menu_callback_debug_preview_fo(gpointer callback_data,
				  guint callback_action,
				  GtkWidget *widget)
{
	/* Open a GnomePrint preview */
	xmlDocPtr xml_doc;
	GnomePrintMaster *gpm;
	GnomePrintContext *gpc;
	GtkWidget *preview_widget;
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc;

	gpm = gnome_print_master_new ();
	gpc = gnome_print_master_get_context (gpm);
#if 1

#if 1
	/* Grab XML from document; assume it's FO for now: */
	doc = cong_primary_window_get_document(primary_window);
	g_return_if_fail(doc);


	xml_doc = cong_document_get_xml(doc);
#else
	/* Load a hardcoded file off my hardddrive (to make it easier to test): */
	xml_doc = xmlParseFile("/home/david/coding/conge-cvs-dhm3/conge/examples/test-fo.xml");
#endif
	g_assert(xml_doc);

	/* Render the FO document to gnome_print: */
	cong_gnome_print_render_xslfo(xml_doc, gpm);
	
	xmlFreeDoc(xml_doc);
#else
	/* Just render some test stuff: */
	my_draw (gpc);
#endif

	gnome_print_master_close (gpm);

	preview_widget = gnome_print_master_preview_new (gpm, _("Print Preview"));
	gtk_widget_show(preview_widget);
}
#endif /* #if PRINT_TESTS */

void my_hash_scanner(void *payload, void *data, xmlChar *name)
{
	g_message("got name \"%s\"", name);
}

#if 0
gchar *get_element_content_string(xmlElementContentPtr content)
{
	g_return_val_if_fail(content, NULL);

	switch (content->type) {
	default: g_assert(0);
	case XML_ELEMENT_CONTENT_PCDATA:
	case XML_ELEMENT_CONTENT_ELEMENT:
	case XML_ELEMENT_CONTENT_SEQ:
	case XML_ELEMENT_CONTENT_OR:
		{
			gchar *lhs = c1->name;
		}


	}
}
#endif

gchar *get_enumeration_details(xmlEnumerationPtr enum_ptr)
{
	gchar *temp, *temp2;
	temp = g_strdup("(");

	while (enum_ptr) {
		temp2 = g_strdup_printf("%s %s%s", temp, enum_ptr->name, (enum_ptr->next!=NULL)?",":"");
		g_free(temp);
		temp = temp2;

		enum_ptr=enum_ptr->next;
	}

	temp2 = g_strdup_printf("%s )", temp);
	g_free(temp);
	return temp2;
}

gchar *get_attribute_debug_details(xmlAttributePtr attr)
{
	gchar *type_str = NULL;
	g_assert(attr);

	switch (attr->atype) {
	default: g_assert(0);
	case XML_ATTRIBUTE_CDATA:
		type_str = g_strdup("CDATA");
		break;

	case XML_ATTRIBUTE_ID:
		type_str = g_strdup("ID");
		break;

	case XML_ATTRIBUTE_IDREF:
		type_str = g_strdup("IDREF");
		break;

	case XML_ATTRIBUTE_IDREFS:
		type_str = g_strdup("IDREFS");
		break;

	case XML_ATTRIBUTE_ENTITY:
		type_str = g_strdup("ENTITY");
		break;

	case XML_ATTRIBUTE_ENTITIES:
		type_str = g_strdup("ENTITIES");
		break;

	case XML_ATTRIBUTE_NMTOKEN:
		type_str = g_strdup("NMTOKEN");
		break;

	case XML_ATTRIBUTE_NMTOKENS:
		type_str = g_strdup("NMTOKENS");
		break;

	case XML_ATTRIBUTE_ENUMERATION:
		type_str = get_enumeration_details(attr->tree);
		break;

	case XML_ATTRIBUTE_NOTATION:
		type_str = g_strdup("NOTATION");
		break;
	}

#if 0
	switch (attr->def) {
	}
#endif

	return type_str;

#if 0
    xmlAttributeType       atype;	/* The attribute type */
    xmlAttributeDefault      def;	/* the default */
    const xmlChar  *defaultValue;	/* or the default value */
    xmlEnumerationPtr       tree;       /* or the enumeration tree if any */
#endif

}

void element_hash_scanner(void *payload, void *data, xmlChar *name)
{
	xmlElementPtr element = payload;
	xmlAttributePtr attr;

	g_message("got element <%s>", name);
/*  	g_message("content = "); */

	/* List the attributes that apply to this element: */
	for (attr=element->attributes; attr; attr=attr->nexth) {
		gchar *details = get_attribute_debug_details(attr);
		g_message("attribute \"%s\": %s",attr->name, details);
		g_free(details);
	}
}

void entity_hash_scanner(void *payload, void *data, xmlChar *name)
{
	g_message("got entity \"%s\"", name);
}

void debug_log_dtd(xmlDtdPtr dtd)
{
	g_message("Name \"%s\"\n", dtd->name);
	g_message("ExternalID:\"%s\"\n", dtd->ExternalID);
	g_message("SystemID:\"%s\"\n", dtd->SystemID);

#if 0
	g_message("notations\n");
	xmlHashScan(dtd->notations,
		    my_hash_scanner,
		    NULL);
#endif

	g_message("\nelements\n");
	xmlHashScan(dtd->elements,
		    element_hash_scanner,
		    NULL);

#if 0
	g_message("\nattributes\n");
	xmlHashScan(dtd->attributes,
		    my_hash_scanner,
		    NULL);
#endif

#if 0
	g_message("\nentities\n");
	xmlHashScan(dtd->entities,
		    entity_hash_scanner,
		    NULL);
#endif

#if 0
	g_message("\npentities\n");
	xmlHashScan(dtd->pentities,
		    my_hash_scanner,
		    NULL);
#endif
}

void menu_callback_debug_dtd(gpointer callback_data,
			    guint callback_action,
			    GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc;
	xmlDocPtr xml_doc;


	doc = cong_primary_window_get_document(primary_window);
	g_return_if_fail(doc);

	xml_doc = cong_document_get_xml(doc);
	g_assert(xml_doc);
 
	if (xml_doc->intSubset) {
		g_message("Interior subset:\n");
		debug_log_dtd(xml_doc->intSubset);
	} else {
		g_message("No interior subset\n");
	}
	
	if (xml_doc->extSubset) {
		g_message("Exterior subset:\n");
		debug_log_dtd(xml_doc->extSubset);
	} else {
		g_message("No exterior subset\n");
	}
}

GtkWidget *test_dialog_new(GtkWindow *parent_window)
{
	GtkWidget *dialog;
	GtkWidget *tabs;
	CongDialogContent *basic_content;
	CongDialogCategory *general_category;
	CongDialogCategory *fubar_category;
	CongDialogContent *advanced_content;
	CongDialogCategory *morestuff_category;
	CongDialogCategory *yetmorestuff_category;

	dialog = gtk_dialog_new_with_buttons("Test Dialog",
					     parent_window,
					     0,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_ACCEPT,
					     NULL);

	gtk_container_set_border_width(GTK_CONTAINER(dialog), 12);

	tabs = gtk_notebook_new();

	basic_content = cong_dialog_content_new(TRUE);
	general_category = cong_dialog_content_add_category(basic_content, "General");
	fubar_category = cong_dialog_content_add_category(basic_content, "Fubar");

	advanced_content = cong_dialog_content_new(TRUE);
	morestuff_category = cong_dialog_content_add_category(advanced_content, "More Stuff");
	yetmorestuff_category = cong_dialog_content_add_category(advanced_content, "Yet More Stuff");

	cong_dialog_category_add_field(general_category, "Foo", gtk_entry_new(), FALSE);
	cong_dialog_category_add_field(general_category, "A very long label", gtk_entry_new(), FALSE);
	cong_dialog_category_add_field(general_category, "Another label", gtk_entry_new(), FALSE);
	cong_dialog_category_add_selflabelled_field(general_category, gtk_toggle_button_new_with_label("Bar"), FALSE);
	cong_dialog_category_add_selflabelled_field(general_category, gtk_check_button_new_with_label("Crikey"), FALSE);

	cong_dialog_category_add_field(fubar_category, "Foo", gtk_entry_new(), FALSE);
	cong_dialog_category_add_field(fubar_category, "Foo", gtk_entry_new(), FALSE);
	cong_dialog_category_add_field(fubar_category, "Here we go again", gtk_entry_new(), FALSE);
	cong_dialog_category_add_selflabelled_field(fubar_category, gtk_toggle_button_new_with_label("Bar"), FALSE);
	cong_dialog_category_add_selflabelled_field(fubar_category, gtk_check_button_new_with_label("Crikey"), FALSE);

	cong_dialog_category_add_field(morestuff_category, "Foo", gtk_entry_new(), FALSE);
	cong_dialog_category_add_field(morestuff_category, "Foo", gtk_entry_new(), FALSE);
	cong_dialog_category_add_field(morestuff_category, "Foo", gtk_entry_new(), FALSE);
	cong_dialog_category_add_selflabelled_field(morestuff_category, gtk_toggle_button_new_with_label("Bar"), FALSE);
	cong_dialog_category_add_selflabelled_field(morestuff_category, gtk_check_button_new_with_label("Crikey"), FALSE );

	cong_dialog_category_add_field(yetmorestuff_category, "Foo", gtk_entry_new(), FALSE);
	cong_dialog_category_add_field(yetmorestuff_category, "Foo", gtk_entry_new(), FALSE);
	cong_dialog_category_add_field(yetmorestuff_category, "Foo", gtk_entry_new(), FALSE);
	cong_dialog_category_add_selflabelled_field(yetmorestuff_category, gtk_toggle_button_new_with_label("Bar"), FALSE);
	cong_dialog_category_add_selflabelled_field(yetmorestuff_category, gtk_check_button_new_with_label("Crikey"), FALSE );
	
	gtk_notebook_append_page( GTK_NOTEBOOK(tabs),
				  cong_dialog_content_get_widget(basic_content),
				  gtk_label_new("Basic"));

	gtk_notebook_append_page( GTK_NOTEBOOK(tabs),
				  cong_dialog_content_get_widget(advanced_content),
				  gtk_label_new("Advanced"));

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   tabs);

	gtk_widget_show_all(dialog);

	return dialog;
}

void menu_callback_debug_dialog(gpointer callback_data,
				guint callback_action,
				GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;

	GtkWidget *dialog = test_dialog_new(cong_primary_window_get_toplevel(primary_window));

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

struct debug_progress_checklist
{
	CongProgressChecklistDialog *dialog;
	CongProgressChecklist *progress_checklist;
	guint timeout_id;
};

static gboolean on_timeout(gpointer user_data)
{
	struct debug_progress_checklist* debug_data = user_data;

	g_message("on_timeout");

	cong_progress_checklist_complete_stage(debug_data->progress_checklist);

	return TRUE;
}


void menu_callback_debug_progress_checklist(gpointer callback_data,
					    guint callback_action,
					    GtkWidget *widget)
{
	struct debug_progress_checklist debug_data;
	CongPrimaryWindow *primary_window = callback_data;
	int i;

	debug_data.dialog = CONG_PROGRESS_CHECKLIST_DIALOG(cong_progress_checklist_dialog_new(_("Test Progress Checklist"), cong_primary_window_get_toplevel(primary_window)));
	debug_data.progress_checklist = cong_progress_checklist_dialog_get_progress_checklist(debug_data.dialog);
	
	for (i=0;i<10;i++) {
		gchar *stage_name = g_strdup_printf(_("This is stage %i"), i);
		cong_progress_checklist_add_stage(debug_data.progress_checklist,
						  stage_name);
		g_free(stage_name);
	}
	
	debug_data.timeout_id =gtk_timeout_add(1000,
					       on_timeout,                                             
					       &debug_data);

	gtk_dialog_run(GTK_DIALOG(debug_data.dialog));

	gtk_timeout_remove (debug_data.timeout_id);
	gtk_widget_destroy(GTK_WIDGET(debug_data.dialog));
	
}

static void make_debug_log_window (CongPrimaryWindow *primary_window,
				   GtkWidget* log_widget, 
				   const gchar *window_title)
{
	GtkWidget *window;

	window = gnome_app_new(PACKAGE_NAME,
			       window_title);

	gnome_app_set_contents(GNOME_APP(window), log_widget);

	gtk_window_set_default_size(GTK_WINDOW(window),
				    500,
				    400);

	gtk_widget_show(GTK_WIDGET(window));
}


void menu_callback_debug_document_message_log(gpointer callback_data,
					      guint callback_action,
					      GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	make_debug_log_window (primary_window,
			       cong_debug_message_log_view_new(doc),
			       _("Message Log - Conglomerate"));
}

void menu_callback_debug_document_signal_log(gpointer callback_data,
					      guint callback_action,
					      GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	make_debug_log_window (primary_window,
			       cong_debug_signal_log_view_new(doc),
			       _("Signal Log - Conglomerate"));
}

void menu_callback_debug_information_alert(gpointer callback_data,
					   guint callback_action,
					   GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	GtkDialog *dialog = cong_dialog_information_alert_new(cong_primary_window_get_toplevel(primary_window),
							      "This is a test information alert.  Hopefully it complies with the GNOME HIG.");
	gtk_dialog_run(dialog);
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

void menu_callback_debug_glade_test(gpointer callback_data,
					   guint callback_action,
					   GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;

	gchar* glade_filename = gnome_program_locate_file(cong_app_get_gnome_program (cong_app_singleton()),
							  GNOME_FILE_DOMAIN_APP_DATADIR,
							  "test.glade",
							  FALSE,
							  NULL);

	GladeXML *xml = glade_xml_new(glade_filename, NULL, NULL);
	glade_xml_signal_autoconnect(xml);

	g_free(glade_filename);

	(void)primary_window; /* suppress warnings */
}

/* Code for dumping an XML representation of the loaded plugins: */
static void 
add_xml_for_service (CongService *service,
		     gpointer user_data)
{
	xmlNodePtr plugin_node = (xmlNodePtr)user_data;
	xmlNodePtr service_node;
	
	g_assert (IS_CONG_SERVICE (service));

	service_node = xmlNewDocNode (plugin_node->doc,
				      NULL,
				      "CongService",
				      NULL);	
	xmlAddChild (plugin_node,
		     service_node);


	xmlSetProp (service_node, 
		    "serviceID", 
		    cong_service_get_id (service));

	xmlSetProp (service_node, 
		    "serviceClass", 
		    G_OBJECT_TYPE_NAME (G_OBJECT (service)));

	xmlAddChild (service_node,
		     xmlNewDocNode (service_node->doc,
				    NULL,
				    "Name",
				    cong_service_get_name (service)));

	xmlAddChild (service_node,
		     xmlNewDocNode (service_node->doc,
				    NULL,
				    "Description",
				    cong_service_get_description (service)));
}

static void
add_xml_for_plugin (CongPlugin *plugin,
		    gpointer user_data)
{
	xmlNodePtr root_node = (xmlNodePtr)user_data;
	xmlNodePtr plugin_node;

	g_assert (IS_CONG_PLUGIN (plugin));

	plugin_node = xmlNewDocNode (root_node->doc,
				     NULL,
				     "CongPlugin",
				     NULL);	
	xmlAddChild (root_node, 
		     plugin_node);

	xmlSetProp (plugin_node, 
		    "pluginID", 
		    cong_plugin_get_id (plugin));

	cong_plugin_for_each_service (plugin, 
				      add_xml_for_service,
				      plugin_node);
}

static xmlDocPtr
make_plugin_info_xml (CongPluginManager *plugin_manager)
{
	xmlDocPtr xml_doc;
	xmlNodePtr root_node;

	g_return_val_if_fail (plugin_manager, NULL);

	xml_doc = xmlNewDoc("1.0");
	
	root_node = xmlNewDocNode(xml_doc,
				  NULL, /* xmlNsPtr ns, */
				  "CongPluginData",
				  NULL);
	
	xmlDocSetRootElement (xml_doc,
			      root_node);

	cong_plugin_manager_for_each_plugin (plugin_manager, 
					     add_xml_for_plugin,
					     root_node);
	return xml_doc;	

	
}

static void 
menu_callback_debug_plugin_info (gpointer callback_data,
				 guint callback_action,
				 GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;

	cong_ui_new_document_from_imported_xml (make_plugin_info_xml (cong_app_get_plugin_manager (cong_app_singleton ())),
						cong_primary_window_get_toplevel (primary_window));
}


static gchar*
get_test_fragment(CongPrimaryWindow *primary_window)
{
#if 1
	return g_strdup("foo<emphasis>Hello world</emphasis>bar");
#else
	return g_strdup("foo<some-random-unknown-tag>Hello world</some-random-unknown-tag>bar");
#endif
}

void menu_callback_debug_insert_xml_fragment(gpointer callback_data,
					     guint callback_action,
					     GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);
	gchar *source_fragment;
	g_assert(doc);

	source_fragment = get_test_fragment(primary_window);
	g_assert(source_fragment);

	cong_document_paste_source_at (doc, 
				       &cong_document_get_cursor(doc)->location, 
				       source_fragment);
	
	g_free(source_fragment);
}

void menu_callback_debug_command_test (gpointer callback_data,
				       guint callback_action,
				       GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	CongCommand* cmd = cong_document_begin_command (doc,
							"test command",
							NULL);

	cong_document_end_command (doc,
				   cmd);
}
#endif /* #if ENABLE_DEBUG_MENU */

/* Callbacks for "Help" menu: */
static void menu_callback_about(gpointer callback_data,
				guint callback_action,
				GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;

	GdkPixbuf *logo_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)ilogo_xpm);
	gchar* authors[] = {"Hans Petter Jansson", 
			    "David Malcolm", 
			    "Joakim Ziegler", 
			    "Steinar Bang", 
			    "Geert Stappers", 
			    "Brent Hendricks", 
			    "Doug Daniels", 
			    "Dmitry Mastrukov", 
			    "Jeff Martin", 
			    NULL};

	gchar* documenters[] = { NULL };

 	gchar* translator_credits = _("translator_credits");
  
 	GtkWidget *about = gnome_about_new(_("Conglomerate XML Editor"),
 					   PACKAGE_VERSION,
 					   _("(C) 1999 Hans Petter Jansson\n(C) 2004 David Malcolm"),
 					   _("Conglomerate: a free, user-friendly XML editor"),
 					   (const char **)authors,
 					   (const char **)documenters,
 					   strcmp(translator_credits, "translator_credits") != 0 ?
 						    translator_credits : NULL,
  					   logo_pixbuf);
	gdk_pixbuf_unref(logo_pixbuf);

	gtk_window_set_transient_for(GTK_WINDOW(about), 
				     cong_primary_window_get_toplevel(primary_window));

	gtk_dialog_run(GTK_DIALOG(about));

}

static void menu_callback_help(gpointer callback_data,
				guint callback_action,
				GtkWidget *widget)
{
	GError *error = NULL;

	gnome_help_display("conglomerate.xml", NULL, &error);

	if(error!=NULL)
	{
		g_warning(error->message);
		g_error_free(error);
	}
}

/* The menus, for a window that contains a document: */
enum ActionMarkers {
	ACTION_MARKER_UNDO = 1,
	ACTION_MARKER_REDO,
	ACTION_MARKER_CUT,
	ACTION_MARKER_COPY,
	ACTION_MARKER_PASTE,
	ACTION_MARKER_TOOLS_MENU
};


static GtkItemFactoryEntry menu_items_with_doc[] =
{
	{ N_("/_File"),             NULL, NULL, 0, "<Branch>" },
	{ N_("/File/_New..."),       NULL, menu_callback_file_new, 0, "<StockItem>", GTK_STOCK_NEW },
	{ N_("/File/_Open..."),      NULL, menu_callback_file_open, 0, "<StockItem>", GTK_STOCK_OPEN },
	{ N_("/File/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/File/_Save"),           "<control>S", menu_callback_file_save, 0, "<StockItem>", GTK_STOCK_SAVE },
	{ N_("/File/Save _As..."),     "<shift><control>S", menu_callback_file_save_as, 0, "<StockItem>", GTK_STOCK_SAVE_AS },
	{ N_("/File/Sa_ve a Copy..."), NULL, menu_callback_file_save_copy, 0, "<Item>" },
#if ENABLE_PRINTING
	{ N_("/File/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/File/Print Previe_w..."),   "<shift><control>P", menu_callback_file_print_preview, 0, "<StockItem>", GTK_STOCK_PRINT_PREVIEW },
	{ N_("/File/_Print..."),           "<control>P", menu_callback_file_print, 0, "<StockItem>", GTK_STOCK_PRINT },
#endif
#if ENABLE_UNIMPLEMENTED_MENUS
	{ N_("/File/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/File/_Revert"),         NULL, menu_callback_file_revert, 0, "<StockItem>", GTK_STOCK_REVERT_TO_SAVED },
#endif /* #if ENABLE_UNIMPLEMENTED_MENUS */
	{ N_("/File/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/File/_Import..."),           NULL, menu_callback_file_import, 0, "<Item>" },
	{ N_("/File/_Export..."),           NULL, menu_callback_file_export, 0, "<Item>" },
	{ N_("/File/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/File/Proper_ties"),     NULL, menu_callback_file_properties, 0, "<StockItem>", GTK_STOCK_PROPERTIES },
	{ N_("/File/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/File/_Close"),         "<control>W", menu_callback_file_close, 0, "<StockItem>", GTK_STOCK_CLOSE },
	{ N_("/File/_Quit"),         "<control>Q", menu_callback_file_quit, 0, "<StockItem>", GTK_STOCK_QUIT },

	{ N_("/_Edit"),                 NULL, 0, 0, "<Branch>" },
	{ N_("/Edit/_Undo"),              "<control>Z", menu_callback_undo, ACTION_MARKER_UNDO, "<StockItem>", GTK_STOCK_UNDO },
	{ N_("/Edit/_Redo"),              "<shift><control>Z", menu_callback_redo, ACTION_MARKER_REDO, "<StockItem>", GTK_STOCK_REDO },
	{ N_("/Edit/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/Edit/Cu_t"),              "<control>X", menu_callback_cut, ACTION_MARKER_CUT, "<StockItem>", GTK_STOCK_CUT },
	{ N_("/Edit/_Copy"),             "<control>C", menu_callback_copy, ACTION_MARKER_COPY, "<StockItem>", GTK_STOCK_COPY },
	{ N_("/Edit/_Paste"),            "<control>V", menu_callback_paste, ACTION_MARKER_PASTE, "<StockItem>", GTK_STOCK_PASTE },
	{ N_("/Edit/"), NULL, NULL, 0, "<Separator>" },
#if ENABLE_UNIMPLEMENTED_MENUS
	{ N_("/Edit/_Find..."),         "<control>F", menu_callback_find, 0, "<StockItem>", GTK_STOCK_FIND },
	{ N_("/Edit/Find Ne_xt"),       "<control>G", menu_callback_find_next, 0, "<Item>" },
	{ N_("/Edit/Find Pre_vious"),   "<shift><control>G", menu_callback_find_prev, 0, "<Item>" },
	{ N_("/Edit/R_eplace..."),      "<control>R", menu_callback_replace, 0, "<StockItem>", GTK_STOCK_FIND_AND_REPLACE },
	{ N_("/Edit/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/Edit/_Insert..."),       NULL, unimplemented_menu_item, 0, "<Item>" },
	{ N_("/Edit/"), NULL, NULL, 0, "<Separator>" },
#endif /* #if ENABLE_UNIMPLEMENTED_MENUS */
	{ N_("/Edit/View _Source"),     "<control>U", menu_callback_view_source, 0, NULL },
	{ N_("/Edit/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/Edit/Prefere_nces"),     NULL, menu_callback_preferences, 0, NULL },

#if ENABLE_DEBUG_MENU
	{ N_("/Debug"),                 NULL, NULL, 0, "<Branch>" },
	{ N_("/Debug/Begin self-test of error-reporting system..."),           NULL, menu_callback_debug_error, 0, NULL },
	{ N_("/Debug/Transform DocBook to HTML"),       NULL, menu_callback_debug_transform_docbook_to_html, 0, NULL },
	{ N_("/Debug/Transform DocBook to XHTML"),       NULL, menu_callback_debug_transform_docbook_to_xhtml, 0, NULL },
	{ N_("/Debug/Transform DocBook to HTML Help"),       NULL, menu_callback_debug_transform_docbook_to_html_help, 0, NULL },
	{ N_("/Debug/Transform DocBook to Java Help"),       NULL, menu_callback_debug_transform_docbook_to_javahelp, 0, NULL },
	{ N_("/Debug/Transform DocBook to FO"),       NULL, menu_callback_debug_transform_docbook_to_fo, 0, NULL },
#if PRINT_TESTS
	{ N_("/Debug/Preview XSL:FO"),       NULL, menu_callback_debug_preview_fo, 0, NULL },
#endif /* #if PRINT_TESTS */
	{ N_("/Debug/DTD"),             NULL, menu_callback_debug_dtd, 0, NULL },
	{ N_("/Debug/Dialog"),             NULL, menu_callback_debug_dialog, 0, NULL },
	{ N_("/Debug/Progress Checklist"),             NULL, menu_callback_debug_progress_checklist, 0, NULL },
	{ N_("/Debug/Document Message Log"),           NULL, menu_callback_debug_document_message_log, 0, NULL },	
	{ N_("/Debug/Document Signal Log"),           NULL, menu_callback_debug_document_signal_log, 0, NULL },	
	{ N_("/Debug/Information Alert"),           NULL, menu_callback_debug_information_alert, 0, NULL },	
	{ N_("/Debug/Glade Test"),           NULL, menu_callback_debug_glade_test, 0, NULL },	
	{ N_("/Debug/Insert XML Fragment"),           NULL, menu_callback_debug_insert_xml_fragment, 0, NULL },	
	{ N_("/Debug/Command Test"),           NULL, menu_callback_debug_command_test, 0, NULL },	
#endif /* #if ENABLE_DEBUG_MENU */

	{ N_("/_Tools"),        NULL, NULL, ACTION_MARKER_TOOLS_MENU, "<Branch>" },

	{ N_("/_Help"),        NULL, NULL, 0, "<Branch>" },
	{ N_("/Help/_Contents"), "F1", menu_callback_help, 0, "<StockItem>",GTK_STOCK_HELP },
	{ N_("/Help/_About"),    NULL, menu_callback_about, 0, "<StockItem>", GNOME_STOCK_ABOUT }

};

/* The menus, for a window that doesn't contain a document (i.e. the initial window, when launching the application): */
static GtkItemFactoryEntry menu_items_without_doc[] =
{
	{ N_("/_File"),             NULL, NULL, 0, "<Branch>" },
	{ N_("/File/_New..."),       NULL, menu_callback_file_new, 0, "<StockItem>", GTK_STOCK_NEW },
	{ N_("/File/_Open..."),      NULL, menu_callback_file_open, 0, "<StockItem>", GTK_STOCK_OPEN },
	{ N_("/File/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/File/_Import..."),           NULL, menu_callback_file_import, 0, "<Item>" },
	{ N_("/File/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/File/_Close"),         "<control>W", menu_callback_file_close, 0, "<StockItem>", GTK_STOCK_CLOSE },
	{ N_("/File/_Quit"),         "<control>Q", menu_callback_file_quit, 0, "<StockItem>", GTK_STOCK_QUIT },

	{ N_("/_Edit"),                 NULL, 0, 0, "<Branch>" },
	{ N_("/Edit/Prefere_nces"),     NULL, menu_callback_preferences, 0, NULL },

#if ENABLE_DEBUG_MENU
	{ ("/Debug"),                 NULL, NULL, 0, "<Branch>" },
	{ ("/Debug/Begin self-test of error-reporting system..."),           NULL, menu_callback_debug_error, 0, NULL },
	{ ("/Debug/Dialog"),             NULL, menu_callback_debug_dialog, 0, NULL },
	{ ("/Debug/Progress Checklist"),             NULL, menu_callback_debug_progress_checklist, 0, NULL },
	{ ("/Debug/Information Alert"),           NULL, menu_callback_debug_information_alert, 0, NULL },	
	{ ("/Debug/Glade Test"),           NULL, menu_callback_debug_glade_test, 0, NULL },	
	{ ("/Debug/Get Plugin Information as XML"),           NULL, menu_callback_debug_plugin_info, 0, NULL },	
#endif /* #if ENABLE_DEBUG_MENU */

	{ N_("/_Help"),        NULL, NULL, 0, "<Branch>" },
	{ N_("/Help/_Contents"), "F1", menu_callback_help, 0, "<StockItem>",GTK_STOCK_HELP },
	{ N_("/Help/_About"),    NULL, menu_callback_about, 0, "<StockItem>", GNOME_STOCK_ABOUT }

};

struct add_tool_callback_data
{
	CongPrimaryWindow *primary_window;
	GtkMenuShell *tools_menu;
	GtkTooltips *menu_tips;
};

static void on_tool_menu_item_activation(GtkMenuItem *menuitem,
					 gpointer user_data)
{
	CongPrimaryWindow *primary_window = user_data;
	CongServiceDocTool *tool = g_object_get_data(G_OBJECT(menuitem), "cong-tool");
	
	g_assert(primary_window);
	g_assert(tool);

	cong_doc_tool_invoke(tool, primary_window);
}

static void add_tool_callback(CongServiceDocTool *tool, gpointer user_data)
{
	struct add_tool_callback_data *callback_data = user_data;

	if (cong_doc_tool_supports_document(tool, cong_primary_window_get_document(callback_data->primary_window))) {
		GtkMenuItem *menu_item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(cong_service_tool_get_menu_text(CONG_SERVICE_TOOL (tool))));

		g_signal_connect(G_OBJECT(menu_item), 
				 "activate", 
				 G_CALLBACK(on_tool_menu_item_activation),
				 callback_data->primary_window);

		g_object_set_data(G_OBJECT(menu_item), 
				  "cong-tool",
				  tool);

		gtk_tooltips_set_tip(callback_data->menu_tips,
				     GTK_WIDGET(menu_item),
				     cong_service_tool_get_tip_text(CONG_SERVICE_TOOL (tool)),
				     cong_service_tool_get_tip_further_text(CONG_SERVICE_TOOL (tool)));

		gtk_widget_show(GTK_WIDGET(menu_item));

		gtk_menu_shell_append(callback_data->tools_menu,
				      GTK_WIDGET(menu_item));
	}
}

static void 
on_history_changed_undo_menu (CongCommandHistory *history,
			      gpointer user_data)
{
	GtkWidget *undo_menu_item = GTK_WIDGET (user_data);

	gtk_widget_set_sensitive (undo_menu_item,
				  cong_command_history_can_undo (history));

	/* FIXME: is there a way to set the menu item text to reflect the new content? */
}

static void 
on_history_changed_redo_menu (CongCommandHistory *history,
			      gpointer user_data)
{
	GtkWidget *redo_menu_item = GTK_WIDGET (user_data);

	gtk_widget_set_sensitive (redo_menu_item,
				  cong_command_history_can_redo (history));

	/* FIXME: is there a way to set the menu item text to reflect the new content? */
}

static void 
on_selection_changed_copy_menu (CongDocument *document,
			      gpointer user_data)
{
	GtkWidget *copy = GTK_WIDGET (user_data);

	CongSelection *selection = cong_document_get_selection(document);
	CongRange *range = cong_selection_get_ordered_range(selection);
	gtk_widget_set_sensitive (copy,
				  cong_range_can_be_copied (range));
}

static void 
on_selection_changed_cut_menu (CongDocument *document,
			      gpointer user_data)
{
	GtkWidget *cut = GTK_WIDGET (user_data);

	CongSelection *selection = cong_document_get_selection(document);
	CongRange *range = cong_selection_get_ordered_range(selection);
	gtk_widget_set_sensitive (cut,
				  cong_range_can_be_cut (range));
}

void cong_menus_create_items(GtkItemFactory *item_factory, 
			     CongPrimaryWindow *primary_window)
{
	g_return_if_fail(item_factory);
	g_return_if_fail(primary_window);

	if (cong_primary_window_get_document(primary_window)) {
		gtk_item_factory_delete_entries (item_factory,
					 sizeof(menu_items_without_doc) / sizeof (menu_items_without_doc[0]),
					 menu_items_without_doc);
		
		gtk_item_factory_create_items(item_factory, 
					      sizeof(menu_items_with_doc) / sizeof(menu_items_with_doc[0]),
					      menu_items_with_doc, 
					      primary_window /* so that all menu callbacks receive the CongPrimaryWindow ptr as their callback_data */);

		/* Now add any plugin tools below the "Tools" menu: */
		{
			struct add_tool_callback_data callback_data;
			GtkWidget *tools_menu =  gtk_item_factory_get_widget_by_action(item_factory,
										       ACTION_MARKER_TOOLS_MENU);
			g_assert(tools_menu);

			callback_data.primary_window = primary_window;
			callback_data.tools_menu = GTK_MENU_SHELL(tools_menu);
			g_assert(callback_data.tools_menu);
			callback_data.menu_tips = gtk_tooltips_new();

			cong_plugin_manager_for_each_doc_tool (cong_app_get_plugin_manager (cong_app_singleton()), 
							       add_tool_callback, 
							       &callback_data);
		}

		/* Connect undo/redo sensitivity to the command history: */
		/* Connect Cut/Copy sensitivity to CongDocument "selection change" signal PCS*/
		{
			CongDocument *doc = cong_primary_window_get_document (primary_window);
			CongCommandHistory *history = cong_document_get_command_history (doc);

			GtkWidget *undo =  gtk_item_factory_get_widget_by_action(item_factory,
										 ACTION_MARKER_UNDO);
			GtkWidget *redo =  gtk_item_factory_get_widget_by_action(item_factory,
										 ACTION_MARKER_REDO);
			GtkWidget *copy =  gtk_item_factory_get_widget_by_action(item_factory,
										 ACTION_MARKER_COPY);
			GtkWidget *cut =  gtk_item_factory_get_widget_by_action(item_factory,
										 ACTION_MARKER_CUT);
			GtkWidget *paste =  gtk_item_factory_get_widget_by_action(item_factory,
										 ACTION_MARKER_PASTE);
			/* FIXME: What signal to connect paste to in order to update sensitivity? */

			g_signal_connect (G_OBJECT(history),
					  "changed",
					  G_CALLBACK(on_history_changed_undo_menu),
					  undo);
			g_signal_connect (G_OBJECT(history),
					  "changed",
					  G_CALLBACK(on_history_changed_redo_menu),
					  redo);
			g_signal_connect (G_OBJECT(doc),
					  "selection_change",
					  G_CALLBACK(on_selection_changed_copy_menu),
					  copy);
			g_signal_connect (G_OBJECT(doc),
					  "selection_change",
					  G_CALLBACK(on_selection_changed_cut_menu),
					  cut);

			gtk_widget_set_sensitive (undo, FALSE);
			gtk_widget_set_sensitive (redo, FALSE);
			gtk_widget_set_sensitive (paste, cong_document_can_paste(doc));
			gtk_widget_set_sensitive (copy, FALSE);
			gtk_widget_set_sensitive (cut, FALSE);
		}

	} else {
		gtk_item_factory_create_items(item_factory,
					      sizeof(menu_items_without_doc) / sizeof(menu_items_without_doc[0]),
					      menu_items_without_doc, 
					      primary_window /* so that all menu callbacks receive the CongPrimaryWindow ptr as their callback_data */);
	}
}
