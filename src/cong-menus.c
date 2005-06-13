/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-menus.c
 *
 * Copyright (C) 2004 David Malcolm
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

#define ENABLE_DEBUG_MENU 0
#define ENABLE_UNIMPLEMENTED_MENUS 0

/**
 * make_uneditable_text:
 * @text:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget* 
make_uneditable_text(const gchar* text)
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
/**
 * unimplemented_menu_item:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
void 
action_callback_unimplemented (GtkAction *action,
			       CongPrimaryWindow *primary_window)
{
	CONG_DO_UNIMPLEMENTED_DIALOG (cong_primary_window_get_toplevel (primary_window), 
				      _("The selected menu item has not yet been implemented."));
}

static void
dispatch_document_command (void (*document_command)(CongDocument *doc), 
			   CongPrimaryWindow *primary_window)
{
	g_assert(document_command);
	g_assert(primary_window);

	g_return_if_fail (cong_primary_window_get_document (primary_window));

	(*document_command)(cong_primary_window_get_document (primary_window));	
}

/* Callbacks for "File" menu: */
static void 
action_callback_file_new (GtkAction *action,
			  CongPrimaryWindow *primary_window)
{
	new_document(cong_primary_window_get_toplevel(primary_window));
}

static void 
action_callback_file_open (GtkAction *action,
			   CongPrimaryWindow *primary_window)
{
	open_document(cong_primary_window_get_toplevel(primary_window));
}

static void 
action_callback_file_save (GtkAction *action,
			   CongPrimaryWindow *primary_window)
{
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	save_document(doc, cong_primary_window_get_toplevel(primary_window));
}

static void 
action_callback_file_save_as (GtkAction *action,
			      CongPrimaryWindow *primary_window)
{
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	save_document_as(doc, cong_primary_window_get_toplevel(primary_window));
}

static void 
action_callback_file_save_copy (GtkAction *action,
			        CongPrimaryWindow *primary_window)
{
	CONG_DO_UNIMPLEMENTED_DIALOG(cong_primary_window_get_toplevel(primary_window), 
				     _("The selected menu item has not yet been implemented."));
}

#if ENABLE_UNIMPLEMENTED_MENUS
static void 
action_callback_file_revert (GtkAction *action,
			     CongPrimaryWindow *primary_window)
{
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
#endif /* ENABLE_UNIMPLEMENTED_MENUS */

static void 
action_callback_file_import (GtkAction *action,
			     CongPrimaryWindow *primary_window)
{
	/*
	 * FIXME: this option should be disabled if there are no importers installed
	 *  - or we could assume that there is always going to be an importer, since
	 *    some already come with Conglomerate.
	 */

	cong_ui_hook_file_import (cong_primary_window_get_toplevel (primary_window));
}

static void
action_callback_file_export (GtkAction *action,
			     CongPrimaryWindow *primary_window)
{
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	cong_ui_hook_file_export (doc,
				  cong_primary_window_get_toplevel (primary_window));
}

#if ENABLE_PRINTING
static void 
action_callback_file_print_preview (GtkAction *action,
				    CongPrimaryWindow *primary_window)
{
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	cong_ui_hook_file_print_preview (doc,
					 cong_primary_window_get_toplevel (primary_window));
}

static void
action_callback_file_print (GtkAction *action,
			    CongPrimaryWindow *primary_window)
{
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	cong_ui_hook_file_print (doc,
				 cong_primary_window_get_toplevel (primary_window));
}
#endif /* #if ENABLE_PRINTING */


static void 
action_callback_file_properties (GtkAction *action,
				 CongPrimaryWindow *primary_window)
{
	GtkWidget *dialog;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	g_return_if_fail(doc);

	dialog = cong_file_properties_dialog_new(doc,
						 cong_primary_window_get_toplevel(primary_window));

	gtk_widget_show_all(dialog);

	/* FIXME: memory leaks */
}

static void
action_callback_file_close (GtkAction *action,
			    CongPrimaryWindow *primary_window)
{
	if (cong_primary_window_can_close(primary_window)) {
		gtk_widget_destroy(GTK_WIDGET(cong_primary_window_get_toplevel(primary_window)));
	}
}

static void
action_callback_file_quit (GtkAction *action,
			   CongPrimaryWindow *primary_window)
{
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
static void 
action_callback_undo (GtkAction *action,
		      CongPrimaryWindow *primary_window)
{
	dispatch_document_command(cong_document_undo, primary_window);
}

static void
action_callback_redo (GtkAction *action,
		      CongPrimaryWindow *primary_window)
{
	dispatch_document_command(cong_document_redo, primary_window);
}

static void
action_callback_cut (GtkAction *action,
		     CongPrimaryWindow *primary_window)
{
	dispatch_document_command(cong_document_cut_selection, primary_window);
}

static void
action_callback_copy (GtkAction *action,
		      CongPrimaryWindow *primary_window)
{
	dispatch_document_command(cong_document_copy_selection, primary_window);
}

static void
action_callback_paste (GtkAction *action,
		       CongPrimaryWindow *primary_window)
{
	dispatch_document_command(cong_document_paste_clipboard, primary_window);
}

static void 
action_callback_find (GtkAction *action,
		      CongPrimaryWindow *primary_window)
{
	dispatch_document_command (cong_document_find, primary_window);
}

static void 
action_callback_find_next (GtkAction *action,
			   CongPrimaryWindow *primary_window)
{
	dispatch_document_command (cong_document_find_next, primary_window);
}

static void 
action_callback_find_prev (GtkAction *action,
			   CongPrimaryWindow *primary_window)
{
	dispatch_document_command (cong_document_find_prev, primary_window);
}

static void 
action_callback_replace (GtkAction *action,
			 CongPrimaryWindow *primary_window)
{
	dispatch_document_command (cong_document_replace, primary_window);
}

static void
action_callback_view_source (GtkAction *action,
			     CongPrimaryWindow *primary_window)
{
	dispatch_document_command(cong_document_view_source, primary_window);
}

static void
action_callback_preferences (GtkAction *action,
			     CongPrimaryWindow *primary_window)
{
	g_assert (primary_window);

	cong_ui_hook_edit_preferences (cong_primary_window_get_toplevel (primary_window));
}

/* Callbacks for "Debug" menu: */
#if ENABLE_DEBUG_MENU
/**
 * debug_error:
 * @primary_window:
 *
 * TODO: Write me
 */
static void 
debug_error (GtkAction *action,
	     CongPrimaryWindow *primary_window)
{
	cong_error_tests(cong_primary_window_get_toplevel(primary_window));
}

/**
 * action_callback_debug_error:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
static void
action_callback_debug_error (GtkAction *action,
			     CongPrimaryWindow *primary_window)
{
	debug_error(action,
		    primary_window); 
}

/**
 * open_preview_window_for_doc:
 * @doc:
 *
 * TODO: Write me
 */
void 
open_preview_window_for_doc(xmlDocPtr doc)
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

/**
 * open_transformed_window_for_doc:
 * @doc:
 * @parent_window:
 *
 * TODO: Write me
 */
void 
open_transformed_window_for_doc(xmlDocPtr doc, 
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

	cong_node_self_test_recursive(cong_document_get_root_element(cong_doc));

	g_assert(cong_doc);

	cong_primary_window_new(cong_doc);

	g_object_unref(G_OBJECT(cong_doc));
}

/**
 * debug_transform:
 * @primary_window:
 * @stylesheet_filename:
 *
 * TODO: Write me
 */
void 
debug_transform(CongPrimaryWindow *primary_window,
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

/**
 * action_callback_debug_transform_docbook_to_html:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
static void 
action_callback_debug_transform_docbook_to_html (GtkAction *action,
						 CongPrimaryWindow *primary_window)
{
	debug_transform (primary_window,
			 DOCBOOK_TO_HTML_STYLESHEET_FILE);
}

/**
 * action_callback_debug_transform_docbook_to_xhtml:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
static void 
action_callback_debug_transform_docbook_to_xhtml (GtkAction *action,
						  CongPrimaryWindow *primary_window)
{
	debug_transform (primary_window,
			 DOCBOOK_TO_XHTML_STYLESHEET_FILE);
}

/**
 * action_callback_debug_transform_docbook_to_html_help:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
static void 
action_callback_debug_transform_docbook_to_html_help (GtkAction *action,
						      CongPrimaryWindow *primary_window)
{
	debug_transform (primary_window,
			 DOCBOOK_TO_HTML_HELP_STYLESHEET_FILE);
}

/**
 * action_callback_debug_transform_docbook_to_javahelp:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
static void 
action_callback_debug_transform_docbook_to_javahelp (GtkAction *action,
						     CongPrimaryWindow *primary_window)
{
	debug_transform (primary_window,
			 DOCBOOK_TO_JAVAHELP_STYLESHEET_FILE);
}

/**
 * action_callback_debug_transform_docbook_to_fo:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
static void 
action_callback_debug_transform_docbook_to_fo(GtkAction *action,
					      CongPrimaryWindow *primary_window)
{
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

static void 
action_callback_debug_preview_fo(GtkAction *action,
				 CongPrimaryWindow *primary_window)
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

/**
 * my_hash_scanner:
 * @payload:
 * @data:
 * @name:
 *
 * TODO: Write me
 */
void 
my_hash_scanner(void *payload, void *data, xmlChar *name)
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

/**
 * get_enumeration_details:
 * @enum_ptr:
 *
 * TODO: Write me
 */
gchar *
get_enumeration_details(xmlEnumerationPtr enum_ptr)
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

/**
 * get_attribute_debug_details:
 * @attr:
 *
 * TODO: Write me
 */
gchar *
get_attribute_debug_details(xmlAttributePtr attr)
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

/**
 * element_hash_scanner:
 * @payload:
 * @data:
 * @name:
 *
 * TODO: Write me
 */
void 
element_hash_scanner(void *payload, void *data, xmlChar *name)
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

/**
 * entity_hash_scanner:
 * @payload:
 * @data:
 * @name:
 *
 * TODO: Write me
 */
void 
entity_hash_scanner(void *payload, void *data, xmlChar *name)
{
	g_message("got entity \"%s\"", name);
}

/**
 * debug_log_dtd:
 * @dtd:
 *
 * TODO: Write me
 */
void 
debug_log_dtd(xmlDtdPtr dtd)
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

/**
 * action_callback_debug_dtd:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
void 
action_callback_debug_dtd (GtkAction *action,
			   CongPrimaryWindow *primary_window)
{
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

/**
 * test_dialog_new:
 * @parent_window:
 *
 * TODO: Write me
 */
GtkWidget *
test_dialog_new(GtkWindow *parent_window)
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

/**
 * action_callback_debug_dialog:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
void 
action_callback_debug_dialog (GtkAction *action,
			      CongPrimaryWindow *primary_window)
{
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

/**
 * action_callback_debug_progress_checklist:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
static void 
action_callback_debug_progress_checklist (GtkAction *action,
					  CongPrimaryWindow *primary_window)
{
	struct debug_progress_checklist debug_data;
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

/**
 * action_callback_debug_document_message_log:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
static void 
action_callback_debug_document_message_log (GtkAction *action,
					    CongPrimaryWindow *primary_window)
{
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	make_debug_log_window (primary_window,
			       cong_debug_message_log_view_new(doc),
			       _("Message Log - Conglomerate"));
}

/**
 * action_callback_debug_document_signal_log:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
static void 
action_callback_debug_document_signal_log(GtkAction *action,
					  CongPrimaryWindow *primary_window)
{
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	make_debug_log_window (primary_window,
			       cong_debug_signal_log_view_new(doc),
			       _("Signal Log - Conglomerate"));
}

/**
 * action_callback_debug_information_alert:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
static void 
action_callback_debug_information_alert (GtkAction *action,
					 CongPrimaryWindow *primary_window)
{
	GtkDialog *dialog = cong_dialog_information_alert_new(cong_primary_window_get_toplevel(primary_window),
							      "This is a test information alert.  Hopefully it complies with the GNOME HIG.");
	gtk_dialog_run(dialog);
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

/**
 * action_callback_debug_glade_test:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
static void 
action_callback_debug_glade_test (GtkAction *action,
				  CongPrimaryWindow *primary_window)
{
	gchar* glade_filename = gnome_program_locate_file(cong_app_get_gnome_program (cong_app_singleton()),
							  GNOME_FILE_DOMAIN_APP_DATADIR,
							  "conglomerate/glade/test.glade",
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
action_callback_debug_plugin_info (GtkAction *action,
				   CongPrimaryWindow *primary_window)
{
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

/**
 * action_callback_debug_insert_xml_fragment:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
static void 
action_callback_debug_insert_xml_fragment (GtkAction *action,
					   CongPrimaryWindow *primary_window)
{
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

/**
 * action_callback_debug_command_test:
 * @callback_data:
 * @callback_action:
 * @widget:
 *
 * TODO: Write me
 */
static void 
action_callback_debug_command_test (GtkAction *action,
				    CongPrimaryWindow *primary_window)
{
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	CongCommand* cmd = cong_document_begin_command (doc,
							"test command",
							NULL);

	cong_document_end_command (doc,
				   cmd);
}

static void 
action_callback_debug_gnome_spell (GtkAction *action,
				   CongPrimaryWindow *primary_window)
{
	/* CongPrimaryWindow *primary_window = callback_data; */
	/* CongDocument *doc = cong_primary_window_get_document(primary_window); */

	g_message("fubar");
}
#endif /* #if ENABLE_DEBUG_MENU */

/* Callbacks for "Help" menu: */
static void 
action_callback_homepage (GtkAction *action,
			  CongPrimaryWindow *primary_window)
{
   gnome_url_show ("http://www.conglomerate.org", NULL);
}				   

static void 
action_callback_about (GtkAction *action,
		       CongPrimaryWindow *primary_window)
{
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

        gchar *logo_path;

	GdkPixbuf *logo_pixbuf;
  
 	GtkWidget *about;
	
	logo_path = gnome_program_locate_file (cong_app_get_gnome_program (cong_app_singleton()),
					       GNOME_FILE_DOMAIN_APP_PIXMAP,
					       "conglomerate/conglomerate-logo.png",
					       FALSE,
					       NULL);
	logo_pixbuf = gdk_pixbuf_new_from_file(logo_path, NULL);
	
	g_free(logo_path);
	
	about  = gnome_about_new(_("Conglomerate XML Editor"),
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

static void 
action_callback_help (GtkAction *action,
		      CongPrimaryWindow *primary_window)
{
	GError *error = NULL;

	gnome_help_display("conglomerate.xml", NULL, &error);

	if(error!=NULL)
	{
		g_warning(error->message);
		g_error_free(error);
	}
}

/* Application-level Action Entries with a CongPrimaryWindow pointer as the callback data: */
static GtkActionEntry primary_window_application_action_entries[] = {
 
        /* Menus: */
	{ "FileMenu", NULL, N_("_File") },
	{ "EditMenu", NULL, N_("_Edit") },
#if ENABLE_DEBUG_MENU
	{ "DebugMenu", NULL, "Debug" },
#endif
	{ "ContextMenu", NULL, ("Context Menu") },
	{ "HelpMenu", NULL, N_("_Help") },

	/* Sub-menus: */
	{ "NewSubelement", NULL, N_("New sub-element") },
	{ "NewSibling", NULL, N_("New sibling") },
	{ "RemoveSpanTagSubmenu", NULL, N_("Remove span tag")},

	/* Actions found in File menu: */
	{ "New", GTK_STOCK_NEW, N_("_New..."), "<control>N", NULL, G_CALLBACK (action_callback_file_new) },
	{ "Open", GTK_STOCK_OPEN, N_("_Open..."), "<control>O", NULL, G_CALLBACK (action_callback_file_open) },
	{ "Import", NULL, N_("_Import..."), NULL, NULL, G_CALLBACK (action_callback_file_import) },
	{ "Close", GTK_STOCK_CLOSE, N_("_Close"), "<control>W", NULL, G_CALLBACK (action_callback_file_close) },
	{ "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q", NULL, G_CALLBACK (action_callback_file_quit) },	


	/* Actions found in Edit menu: */
	{ "Preferences", GTK_STOCK_PREFERENCES, N_("Pr_eferences"), NULL, NULL, G_CALLBACK (action_callback_preferences) },
#if ENABLE_DEBUG_MENU
	/* Actions found in Debug menu: */
	{ "ErrorReportSelfTest", NULL, "Begin self-test of error-reporting system...", NULL, NULL, G_CALLBACK (action_callback_debug_error) },
	{ "DebugDialog", NULL, "Dialog", NULL, NULL, G_CALLBACK (action_callback_debug_dialog) },
	{ "DebugProgressChecklist", NULL, "Progress Checklist", NULL, NULL, G_CALLBACK (action_callback_debug_progress_checklist) },
	{ "DebugInformationAlert", NULL, "Information Alert", NULL, NULL, G_CALLBACK (action_callback_debug_information_alert) },
	{ "DebugGladeTest", NULL, "Glade Test", NULL, NULL, G_CALLBACK (action_callback_debug_glade_test) },
	{ "GetPluginInfoAsXML", NULL, "Get Plugin Information as XML", NULL, NULL, G_CALLBACK (action_callback_debug_plugin_info) },
#endif


	/* Actions found in Help menu: */
	{ "Contents", GTK_STOCK_HELP, N_("_Contents"), "F1", NULL, G_CALLBACK (action_callback_help) },	
	{ "Homepage", GTK_STOCK_HOME, N_("_Homepage"), NULL, NULL, G_CALLBACK (action_callback_homepage) },	
	{ "About", GNOME_STOCK_ABOUT, N_("_About"), NULL, NULL, G_CALLBACK (action_callback_about) },	
};

/* Document-level Action Entries with a CongPrimaryWindow pointer as the callback data: */
static GtkActionEntry primary_window_document_action_entries[] = {
 
	{ "ToolsMenu", NULL, N_("_Tools") },


	/* Actions found in File menu: */
	{ "Save", GTK_STOCK_SAVE, N_("_Save"), "<control>S", NULL, G_CALLBACK (action_callback_file_save) },
	{ "SaveAs", GTK_STOCK_SAVE_AS, N_("Save _as..."), "<shift><control>S", NULL, G_CALLBACK (action_callback_file_save_as) },
	{ "SaveACopy", NULL, N_("Sa_ve a Copy..."), NULL, NULL, G_CALLBACK (action_callback_file_save_copy) },
#if ENABLE_PRINTING
	{ "PrintPreview", GTK_STOCK_PRINT_PREVIEW, N_("Print Previe_w..."), "<shift><control>P", NULL, G_CALLBACK (action_callback_file_print_preview) },
	{ "Print", GTK_STOCK_PRINT, N_("_Print"), "<control>P", NULL, G_CALLBACK (action_callback_file_print) },
#endif /* #if ENABLE_PRINTING */
#if ENABLE_UNIMPLEMENTED_MENUS
	{ "Revert", GTK_STOCK_REVERT_TO_SAVED, N_("_Revert"), "F12", NULL, G_CALLBACK (action_callback_file_revert) },
#endif /* #if ENABLE_UNIMPLEMENTED_MENUS */
	{ "Export", NULL, N_("_Export..."), NULL, NULL, G_CALLBACK (action_callback_file_export) },
	{ "Properties", GTK_STOCK_PROPERTIES, N_("Proper_ties"), NULL, NULL, G_CALLBACK (action_callback_file_properties) },


	/* Actions found in Edit menu: */
	{ "Undo", GTK_STOCK_UNDO, N_("_Undo"), "<control>Z", NULL, G_CALLBACK (action_callback_undo) },
	{ "Redo", GTK_STOCK_REDO, N_("_Redo"), "<shift><control>Z", NULL, G_CALLBACK (action_callback_redo) },
	{ "Cut", GTK_STOCK_CUT, N_("Cu_t"), "<control>X", NULL, G_CALLBACK (action_callback_cut) },
	{ "Copy", GTK_STOCK_COPY, N_("_Copy"), "<control>C", NULL, G_CALLBACK (action_callback_copy) },
	{ "Paste", GTK_STOCK_PASTE, N_("_Paste"), "<control>V", NULL, G_CALLBACK (action_callback_paste) },
	{ "Find", GTK_STOCK_FIND, N_("_Find..."), "<control>F", NULL, G_CALLBACK (action_callback_find) },
	{ "FindNext", GTK_STOCK_FIND, N_("Find Ne_xt"), "<control>G", NULL, G_CALLBACK (action_callback_find_next) },
	{ "FindPrev", GTK_STOCK_FIND, N_("Find Pre_vious"), "<shift><control>G", NULL, G_CALLBACK (action_callback_find_prev) },
	{ "Replace", GTK_STOCK_FIND_AND_REPLACE, N_("R_eplace..."), "<control>R", NULL, G_CALLBACK (action_callback_replace) },

	{ "Insert", NULL, N_("_Insert..."), NULL, NULL, G_CALLBACK (action_callback_unimplemented) },
	{ "ViewSource", NULL, N_("View _Source"), "<control>U", NULL, G_CALLBACK (action_callback_view_source) },


#if ENABLE_DEBUG_MENU
	/* Actions found in Debug menu: */
	{ "DebugTransformDocBookToHTML", NULL, "Transform DocBook to HTML", NULL, NULL, G_CALLBACK (action_callback_debug_transform_docbook_to_html) },
	{ "DebugTransformDocBookToXHTML", NULL, "Transform DocBook to XHTML", NULL, NULL, G_CALLBACK (action_callback_debug_transform_docbook_to_xhtml) },
	{ "DebugTransformDocBookToHTMLHelp", NULL, "Transform DocBook to HTML Help", NULL, NULL, G_CALLBACK (action_callback_debug_transform_docbook_to_html_help) },
	{ "DebugTransformDocBookToJavaHelp", NULL, "Transform DocBook to Java Help", NULL, NULL, G_CALLBACK (action_callback_debug_transform_docbook_to_javahelp) },
	{ "DebugTransformDocBookToFO", NULL, "Transform DocBook to FO", NULL, NULL, G_CALLBACK (action_callback_debug_transform_docbook_to_fo) },
#if PRINT_TESTS
	{ "DebugPreviewFO", NULL, "Preview XSL:FO", NULL, NULL, G_CALLBACK (action_callback_debug_preview_fo) },
#endif /* #if PRINT_TESTS */
	{ "DebugDTD", NULL, "DTD", NULL, NULL, G_CALLBACK (action_callback_debug_dtd) },
	{ "DebugMessageLog", NULL, "Document Message Log", NULL, NULL, G_CALLBACK (action_callback_debug_document_message_log) },
	{ "DebugSignalLog", NULL, "Document Signal Log", NULL, NULL, G_CALLBACK (action_callback_debug_document_signal_log) },
	{ "DebugInsertXMLFragment", NULL, "Insert XML Fragment", NULL, NULL, G_CALLBACK (action_callback_debug_insert_xml_fragment) },
	{ "DebugCommandTest", NULL, "Command Test", NULL, NULL, G_CALLBACK (action_callback_debug_command_test) },
	{ "DebugSpell", NULL, "Spell", NULL, NULL, G_CALLBACK (action_callback_debug_gnome_spell) },
#endif


};

/* we name the separators as a workaround for a bug in GTK 2.4.0; see http://mail.gnome.org/archives/gtk-app-devel-list/2004-June/msg00271.html */
static const gchar *ui_description =
"<ui>"
"  <menubar name='MainMenuBar'>"
"    <menu action='FileMenu'>"
"      <menuitem action='New'/>"
"      <menuitem action='Open'/>"
"      <separator name='FileSep1'/>"
"      <menuitem action='Save'/>"
"      <menuitem action='SaveAs'/>"
"      <menuitem action='SaveACopy'/>"
#if ENABLE_PRINTING
"      <separator name='FileSep2'/>"
"      <menuitem action='PrintPreview'/>"
"      <menuitem action='Print'/>"
#endif
#if ENABLE_UNIMPLEMENTED_MENUS
"      <separator name='FileSep3'/>"
"      <menuitem action='Revert'/>"
#endif /* #if ENABLE_UNIMPLEMENTED_MENUS */
"      <separator name='FileSep4'/>"
"      <menuitem action='Import'/>"
"      <menuitem action='Export'/>"
"      <separator name='FileSep5'/>"
"      <menuitem action='Properties'/>"
"      <separator name='FileSep6'/>"
"      <placeholder name='FileRecentsPlaceholder'>"
"        <separator/>"
"      </placeholder>"
"      <separator name='FileSep7'/>"
"      <menuitem action='Close'/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='EditMenu'>"
"      <menuitem action='Undo'/>"
"      <menuitem action='Redo'/>"
"      <separator name='EditSep7'/>"
"      <menuitem action='Cut'/>"
"      <menuitem action='Copy'/>"
"      <menuitem action='Paste'/>"
"      <separator name='EditSep8'/>"
"      <menuitem action='Find'/>"
"      <menuitem action='FindNext'/>"
"      <menuitem action='FindPrev'/>"
"      <menuitem action='Replace'/>"
"      <separator name='EditSep9'/>"
#if ENABLE_UNIMPLEMENTED_MENUS
"      <menuitem action='Insert'/>"
"      <separator name='EditSep10'/>"
#endif /* #if ENABLE_UNIMPLEMENTED_MENUS */
"      <menuitem action='ViewSource'/>"
"      <separator name='EditSep11'/>"
"      <menuitem action='Preferences'/>"
"      <menu action='NewSubelement'/>"
"      <menu action='NewSibling'/>"
"      <menu action='RemoveSpanTagSubmenu'/>"
"    </menu>"
#if ENABLE_DEBUG_MENU
"    <menu action='DebugMenu'>"
"      <menuitem action='ErrorReportSelfTest'/>"
"      <menuitem action='DebugTransformDocBookToHTML'/>"
"      <menuitem action='DebugTransformDocBookToXHTML'/>"
"      <menuitem action='DebugTransformDocBookToHTMLHelp'/>"
"      <menuitem action='DebugTransformDocBookToJavaHelp'/>"
"      <menuitem action='DebugTransformDocBookToFO'/>"
#if PRINT_TESTS
"      <menuitem action='DebugPreviewFO'/>"
#endif /* #if PRINT_TESTS */
"      <menuitem action='DebugDTD'/>"
"      <menuitem action='DebugDialog'/>"
"      <menuitem action='DebugProgressChecklist'/>"
"      <menuitem action='DebugMessageLog'/>"
"      <menuitem action='DebugSignalLog'/>"
"      <menuitem action='DebugInformationAlert'/>"
"      <menuitem action='DebugGladeTest'/>"
"      <menuitem action='DebugInsertXMLFragment'/>"
"      <menuitem action='DebugCommandTest'/>"
"      <menuitem action='DebugSpell'/>"
"    </menu>"
#endif
"    <menu action='ToolsMenu'>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Contents'/>"
"      <menuitem action='Homepage'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"  <popup action='ContextMenu'>"
"      <menuitem action='Cut'/>"
"      <menuitem action='Copy'/>"
"      <separator name='ContextSep1'/>"
"      <menu action='NewSubelement'/>"
"      <menu action='NewSibling'/>"
"      <menu action='RemoveSpanTagSubmenu'/>"
"  </popup>"
"  <toolbar name='MainToolBar'>"
"      <toolitem action='New'/>"
"      <toolitem action='Open'/>"
"      <toolitem action='Save'/>"
"      <separator name='ToolSep1'/>"
"      <toolitem action='Undo'/>"
"      <toolitem action='Redo'/>"
"      <separator name='ToolSep2'/>"
"      <toolitem action='Cut'/>"
"      <toolitem action='Copy'/>"
"      <toolitem action='Paste'/>"
"  </toolbar>"
"</ui>";

struct add_tool_callback_data
{
	CongPrimaryWindow *primary_window;
#if 0
	GtkMenuShell *tools_menu;
	GtkTooltips *menu_tips;
#endif
};

static void 
on_tool_menu_item_activation (GtkAction *action,
			      CongPrimaryWindow *primary_window)
{
	CongServiceDocTool *tool = g_object_get_data (G_OBJECT (action), "cong-tool");
	
	g_assert(primary_window);
	g_assert(tool);

	cong_doc_tool_invoke(tool, primary_window);
}

static void add_tool_callback(CongServiceDocTool *tool, gpointer user_data)
{
	struct add_tool_callback_data *callback_data = user_data;

	if (cong_doc_tool_supports_document(tool, cong_primary_window_get_document(callback_data->primary_window))) {
		gchar *action_name = g_strdup_printf ("Tool-%s", cong_service_get_id (CONG_SERVICE (tool)));
		/* Create a GtkAction for this tool: */
		GtkAction *action = gtk_action_new (action_name,
						    cong_service_tool_get_menu_text (CONG_SERVICE_TOOL (tool)),
						    cong_service_tool_get_tip_text (CONG_SERVICE_TOOL (tool)),
						    NULL /*const gchar *stock_id*/);	
		/* Note that this isn't called: cong_service_tool_get_tip_further_text(CONG_SERVICE_TOOL (tool))); */
	
		g_signal_connect (G_OBJECT (action), 
				  "activate", 
				  G_CALLBACK (on_tool_menu_item_activation),
				  callback_data->primary_window);

		g_object_set_data (G_OBJECT(action), 
				   "cong-tool",
				   tool);

		/* Add the action to the actiongroup and to the UI: */
		gtk_action_group_add_action (cong_primary_window_get_action_group (callback_data->primary_window,
										   CONG_ACTION_GROUP_DOCUMENT_TOOLS),
					     action);

		/* Add to the UI: */
		/* Apparently the UI path must not have a leading slash; see http://mail.gnome.org/archives/gtk-app-devel-list/2004-July/msg00263.html */
		gtk_ui_manager_add_ui (cong_primary_window_get_ui_manager (callback_data->primary_window),
				       gtk_ui_manager_new_merge_id (cong_primary_window_get_ui_manager (callback_data->primary_window)),
				       "ui/MainMenuBar/ToolsMenu",
				       action_name,
				       action_name,
				       GTK_UI_MANAGER_AUTO,				       
				       FALSE);
		g_free (action_name);
	}
}

#if OLD_SKOOL
#define SET_ACTION_LABEL(action_name, label) \
			g_object_set (G_OBJECT (gtk_action_group_get_action (cong_primary_window_get_action_group (primary_window, CONG_ACTION_GROUP_DOCUMENT), action_name)), "label", label, NULL)
#define SET_ACTION_SENSITIVE(action_name, sens) \
			g_object_set (G_OBJECT (gtk_action_group_get_action (cong_primary_window_get_action_group (primary_window, CONG_ACTION_GROUP_DOCUMENT), action_name)), "sensitive", sens, NULL)

#endif

static void 
on_history_changed (CongCommandHistory *history,
		    CongPrimaryWindow *primary_window)
{
	/* Update sensitivity and labels of undo and redo actions: */
	gboolean can_undo = cong_command_history_can_undo (history);
	gboolean can_redo = cong_command_history_can_redo (history);

	cong_primary_window_action_set_sensitive (primary_window, "Undo", can_undo);
	if (can_undo) {
		CongCommand *command = cong_command_history_get_next_undo_command (history);
		gchar *label = g_strdup_printf (_("_Undo: %s"), cong_command_get_description (command));
		cong_primary_window_action_set_label (primary_window, "Undo", label);
		g_free (label);
	} else {
		cong_primary_window_action_set_label (primary_window, "Undo", _("_Undo"));
	}

	cong_primary_window_action_set_sensitive (primary_window, "Redo", cong_command_history_can_redo (history));
	if (can_redo) {
		CongCommand *command = cong_command_history_get_next_redo_command (history);
		gchar *label = g_strdup_printf (_("_Redo: %s"), cong_command_get_description (command));
		cong_primary_window_action_set_label (primary_window, "Redo", label);
		g_free (label);
	} else {
		cong_primary_window_action_set_label (primary_window, "Redo", _("_Redo"));
	}
}

static void 
on_selection_changed (CongDocument *document,
		      CongPrimaryWindow *primary_window)
{
	/* Update sensitivity of cut and copy actions: */
	CongSelection *selection = cong_document_get_selection(document);
	CongRange *range = cong_selection_get_ordered_range(selection);
	cong_primary_window_action_set_sensitive (primary_window, "Cut", cong_range_can_be_cut (range));
	cong_primary_window_action_set_sensitive (primary_window, "Copy", cong_range_can_be_copied (range));
}

void
cong_menus_setup_action_groups (CongPrimaryWindow *primary_window)
{
	primary_window->action_group[CONG_ACTION_GROUP_APPLICATION] = gtk_action_group_new ("ApplicationActions");
	primary_window->action_group[CONG_ACTION_GROUP_DOCUMENT] = gtk_action_group_new ("DocumentActions");
	primary_window->action_group[CONG_ACTION_GROUP_DOCUMENT_TOOLS] = gtk_action_group_new ("DocumentTools");

	/* Allows translated menus */
#ifdef ENABLE_NLS 
	gtk_action_group_set_translation_domain 
		(cong_primary_window_get_action_group (primary_window, 
					CONG_ACTION_GROUP_APPLICATION), GETTEXT_PACKAGE); 
	gtk_action_group_set_translation_domain 
		(cong_primary_window_get_action_group (primary_window, 
					CONG_ACTION_GROUP_DOCUMENT), GETTEXT_PACKAGE); 
	gtk_action_group_set_translation_domain 
		(cong_primary_window_get_action_group (primary_window, 
					CONG_ACTION_GROUP_DOCUMENT_TOOLS), GETTEXT_PACKAGE); 	
#endif /* ENABLE_NLS */ 
	
	gtk_action_group_add_actions (cong_primary_window_get_action_group (primary_window, CONG_ACTION_GROUP_APPLICATION), 
				      primary_window_application_action_entries, 
				      G_N_ELEMENTS (primary_window_application_action_entries), 
				      primary_window);
	/*
	 * FIXME: make the Import menu item insensitive if there are no importers
	 *  (or do we assume there is always an importer available?)
	 */
	gtk_action_group_add_actions (cong_primary_window_get_action_group (primary_window, CONG_ACTION_GROUP_DOCUMENT), 
				      primary_window_document_action_entries, 
				      G_N_ELEMENTS (primary_window_document_action_entries), 
				      primary_window);

	gtk_ui_manager_insert_action_group (cong_primary_window_get_ui_manager (primary_window), 
					    primary_window->action_group[CONG_ACTION_GROUP_APPLICATION],
					    0);
	gtk_ui_manager_insert_action_group (cong_primary_window_get_ui_manager (primary_window), 
					    primary_window->action_group[CONG_ACTION_GROUP_DOCUMENT],
					    0);
}

void
cong_menus_setup_document_action_group (CongPrimaryWindow *primary_window)
{
	CongDocument *doc;
	CongCommandHistory *history;

	g_return_if_fail (primary_window);

	g_assert (NULL!=cong_primary_window_get_document(primary_window));
	gtk_action_group_set_visible (primary_window->action_group[CONG_ACTION_GROUP_DOCUMENT],
				      TRUE);
	
	doc = cong_primary_window_get_document (primary_window);
	history = cong_document_get_command_history (doc);

	g_signal_connect (G_OBJECT(history),
			  "changed",
			  G_CALLBACK(on_history_changed),
			  primary_window);
	g_signal_connect (G_OBJECT(doc),
			  "selection_change",
			  G_CALLBACK(on_selection_changed),
			  primary_window);
	
	cong_primary_window_action_set_sensitive (primary_window, "Undo", FALSE);
	cong_primary_window_action_set_sensitive (primary_window, "Redo", FALSE);
	cong_primary_window_action_set_sensitive (primary_window, "Cut", FALSE);
	cong_primary_window_action_set_sensitive (primary_window, "Copy", FALSE);

	/*
	 * set sensitivity for those menu items that depend on the
	 * document or plugins
	 */
	cong_primary_window_action_set_sensitive (primary_window,
						  "Paste",
						  cong_document_can_paste(doc));
	cong_primary_window_action_set_sensitive (primary_window,
						  "Export",
						  cong_document_can_export(doc));

#if ENABLE_PRINTING
	{
		gboolean can_print = cong_document_can_print(doc);
		cong_primary_window_action_set_sensitive (primary_window,
							  "PrintPreview",
							  can_print);
		cong_primary_window_action_set_sensitive (primary_window,
							  "Print",
							  can_print);
	}
#endif
	
	/* Now add any plugin tools below the "Tools" menu: */
	{
		struct add_tool_callback_data callback_data;
		callback_data.primary_window = primary_window;
#if 0
		callback_data.tools_menu = GTK_MENU_SHELL(tools_menu);
		g_assert(callback_data.tools_menu);
		callback_data.menu_tips = gtk_tooltips_new();
#endif
		
		cong_plugin_manager_for_each_doc_tool (cong_app_get_plugin_manager (cong_app_singleton()), 
						       add_tool_callback,
						       &callback_data);
	}

	gtk_ui_manager_insert_action_group (cong_primary_window_get_ui_manager (primary_window), 
					    primary_window->action_group[CONG_ACTION_GROUP_DOCUMENT_TOOLS],
					    0);
	
	gtk_ui_manager_ensure_update (cong_primary_window_get_ui_manager (primary_window));
}



/**
 * cong_menus_create_items:
 * @primary_window:
 *
 * TODO: Write me
 * Returns:
 */
guint
cong_menus_setup_ui_layout (CongPrimaryWindow *primary_window)
{
	GError *error;
	guint merge_id;

	g_return_val_if_fail (primary_window, 0);

	error = NULL;

	merge_id = gtk_ui_manager_add_ui_from_string (cong_primary_window_get_ui_manager (primary_window), ui_description, -1, &error);
	if (!merge_id) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}		

	if (cong_primary_window_get_document(primary_window)) {
		cong_menus_setup_document_action_group (primary_window);
	} else {
		gtk_action_group_set_visible (primary_window->action_group[CONG_ACTION_GROUP_DOCUMENT],
					      FALSE);
	}

	return merge_id;
}

static void
recent_open_cb (GtkAction *action, CongPrimaryWindow *primary_window)
{
	EggRecentItem *item;
	gchar *uri;

	item = egg_recent_view_uimanager_get_item (primary_window->recent_view,
						   action);
	g_return_if_fail (item != NULL);

	uri = egg_recent_item_get_uri (item);
	open_document_do (uri, GTK_WINDOW (primary_window->window));
	g_free (uri);
}

static gchar *
recent_tooltip_func (EggRecentItem *item, gpointer user_data)
{
	char *tip;
	char *uri_for_display;

	uri_for_display = egg_recent_item_get_uri_for_display (item);
	g_return_val_if_fail (uri_for_display != NULL, NULL);

	tip = g_strdup_printf (_("Open '%s'"), uri_for_display);

	g_free (uri_for_display);

	return tip;
}

/**
 * cong_menus_setup_recent_files:
 * @primary_window: Primary window to use
 *
 * With this function we perform recent files initialization
 */
void
cong_menus_setup_recent_files (CongPrimaryWindow *primary_window)
{
       EggRecentModel *model;
       EggRecentViewUIManager *view;

	model = egg_recent_model_new (EGG_RECENT_MODEL_SORT_MRU);

	egg_recent_model_set_limit (model, 5);
	egg_recent_model_set_filter_groups (model, "Conglomerate", NULL);

	view = egg_recent_view_uimanager_new (primary_window->ui_manager,
					      "/MainMenuBar/FileMenu/FileRecentsPlaceholder",
					      G_CALLBACK (recent_open_cb),
					      primary_window);
	egg_recent_view_uimanager_set_tooltip_func (view,
						    recent_tooltip_func,
						    NULL);
       egg_recent_view_uimanager_show_icons (view, FALSE);
	egg_recent_view_set_model (EGG_RECENT_VIEW (view), model);
	primary_window->recent_view = view;
	primary_window->recent_model = model;

	
	return;
}

