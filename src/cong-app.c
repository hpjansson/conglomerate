/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-app.c
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

#include "global.h"
#include "cong-app.h"
#include "cong-dialog.h"
#include "cong-error-dialog.h"
#include "cong-dispspec.h"
#include "cong-dispspec-registry.h"
#include "cong-document.h"
#include "cong-font.h"
#include "cong-plugin-manager.h"

#include <libgnomevfs/gnome-vfs.h>

#include "cong-fake-plugin-hooks.h"

#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>

#define TEST_BIG_FONTS 0

#define LOG_SELECTIONS 0

#if LOG_SELECTIONS
#define LOG_SELECTIONS1(x) g_message(x)
#define LOG_SELECTIONS2(x, a) g_message((x), (a))
#else
#define LOG_SELECTIONS1(x) ((void)0)
#define LOG_SELECTIONS2(x, a) ((void)0)
#endif


#define PRIVATE(x) ((x)->private)

/* Internal data structure declarations: */
typedef struct CongSelectionData CongSelectionData;

struct CongSelectionData
{
	gchar *utf8_xml_buffer;
	gchar *utf8_text;
};

struct CongAppPrivate
{
	GnomeProgram *gnome_program;
	CongPluginManager *plugin_manager;
#if 0
	GdkGC *insert_element_gc;
#endif
	CongDispspecRegistry* ds_registry;
	GConfClient* gconf_client;
	GtkTooltips *tooltips;
	CongFont *fonts[CONG_FONT_ROLE_NUM];

	const GList *language_list;

	CongSelectionData clipboard_selection;
	CongSelectionData primary_selection;

	xsltStylesheetPtr xsl_selection_to_text;

};



CongApp *the_singleton_app = NULL;

/* Internal function declarations: */
static CongApp*
cong_app_new(int   argc,
	     char *argv[]);

static void
cong_app_free(CongApp *app);

static void 
cong_app_private_load_fonts (CongApp *app);

static gboolean 
cong_app_private_load_displayspecs (CongApp *app,
				    GtkWindow *toplevel_window);

static void 
register_plugin (CongApp *app,
		 const gchar *id,
		 CongPluginCallbackRegister register_callback,
		 CongPluginCallbackConfigure configure_callback);

static void 
cong_app_private_load_plugins (CongApp *app);

static void 
cong_app_private_insert_element_init (CongApp *app);

/* Exported function definitions: */
CongApp*
cong_app_singleton (void)
{
	g_assert(the_singleton_app);

	return the_singleton_app;
}

void
cong_app_construct_singleton (int   argc,
			      char *argv[])
{
	g_assert(NULL == the_singleton_app);

	the_singleton_app = cong_app_new(argc,
					 argv);
}

void
cong_app_destroy_singleton(void)
{
	g_assert(the_singleton_app);

	cong_app_free(the_singleton_app);

	the_singleton_app = NULL;
}

int
cong_app_post_init_hack (CongApp *app)
{
	
	/* Load all the displayspec (xds) files: */
	/* 
	   FIXME: currently this function requires a primary window to exist, so it can manipulate graphics contexts... 
	   Ideally we would only create a "document-less" window if no file was specified on the command line.
	*/
	if (!cong_app_private_load_displayspecs (app, NULL)) {
		return 1;
	}

	/* 
	   Load all the plugins.  We do this after loading the xds files in case some of the plugins want to operate on the registry
	   of displayspecs
	 */
	PRIVATE(app)->plugin_manager = cong_plugin_manager_new();
	cong_app_private_load_plugins (app);

	cong_app_private_insert_element_init (app);

	editor_popup_init(NULL); /* FIXME */

	return 0;
}

typedef gchar*
(CongSelectionDataToXMLSourceConversionFn) (guchar *data, 
					    gint length);


static void
debug_selection_data (GtkSelectionData *selection_data, 
		      const gchar *type,
		      CongSelectionDataToXMLSourceConversionFn conversion_fn)
{
	if (selection_data) {
		gchar *selection_name = gdk_atom_name (selection_data->selection);
		gchar *target_name = gdk_atom_name (selection_data->target);
		gchar *type_name = gdk_atom_name (selection_data->type);
		gchar *source_string;

		if (conversion_fn) {
			source_string = conversion_fn (selection_data->data, selection_data->length);
		} else {
			source_string = g_strdup ("conversion_fn unavailable");
		}

		g_message ("Available: selection:\"%s\" target:\"%s\" type:\"%s\" format: %i, length: %i, source_string:\"%s\":", 
			   selection_name,
			   target_name,
			   type_name,
			   selection_data->format,
			   selection_data->length,
			   source_string);

		g_free (selection_name);
		g_free (target_name);
		g_free (type_name);
		g_free (source_string);

#if 0
		if (0!=strcmp(type, "UTF8_STRING")) {
			G_BREAKPOINT();
		}
		g_message (gtk_selection_data_get_text (selection_data));
		g_message (selection_data->data);
#endif
	} else {
		g_message ("Unavailable: type \"%s\"", type);
	}
}

static void
debug_try_selection_type (GtkClipboard* clipboard,
			  const gchar *type,
			  CongSelectionDataToXMLSourceConversionFn conversion_fn)
{
	GtkSelectionData* selection_data;

	g_return_if_fail (clipboard);
	g_return_if_fail (type);

	selection_data = gtk_clipboard_wait_for_contents (clipboard,
							  gdk_atom_intern (type,
									   TRUE));
	debug_selection_data (selection_data, 
			      type, 
			      conversion_fn);

	if (selection_data) {
		gtk_selection_data_free (selection_data);
	}
}

static void
debug_target_list (GtkClipboard *clipboard,
		   GdkAtom *targets,
		   gint n_targets)
{
	gint i;
	
	for (i=0;i<n_targets;i++) {
		GtkSelectionData *selection_data;
		gchar *atom_name = gdk_atom_name (targets[i]);

		g_message ("target [%i]: \"%s\"", i, atom_name);

#if 0
		/* It's dying inside here with a bad atom error: */
		selection_data = gtk_clipboard_wait_for_contents (clipboard,
								  targets[i]);
		debug_selection_data (selection_data, 
				      atom_name, 
				      NULL);

		if (selection_data) {
			gtk_selection_data_free (selection_data);
		}
#endif

		g_free (atom_name);
	}
}

gchar*
convert_ucs2_to_utf8 (guchar *data,
		      gint length)
{
	return g_utf16_to_utf8 ((const gunichar2 *)data,
				(glong)length,
				NULL,
				NULL,
				NULL);
}

gchar*
convert_utf8_string (guchar *data, 
		     gint length)
{
	g_return_val_if_fail (data, NULL);

	/* It should be a valid UTF-8 string; escape as necessary: */
	return g_markup_escape_text ((const gchar *)data,
				     (gssize)length);
}

gchar*
convert_text_xml (guchar *data, 
		  gint length)
{
	g_return_val_if_fail (data, NULL);

	return NULL;
}

gchar*
convert_text_html (guchar *data, 
		   gint length)
{
	g_return_val_if_fail (data, NULL);

#if 0
	/* Assume that it's UCS-2 HTML: */
	return convert_ucs2_to_utf8 (data, length);
#else
	/* Assume that it's valid UTF-8 HTML: */
	return g_strndup (data, length);
#endif
}

gchar*
convert_text_plain (guchar *data, 
		    gint length)
{
	g_return_val_if_fail (data, NULL);

	return NULL;
}

gchar*
convert_application_xhtml_plus_xml (guchar *data, 
				    gint length)
{
	g_return_val_if_fail (data, NULL);

	return NULL;
}

gchar*
convert_application_rtf (guchar *data, 
			 gint length)
{
	g_return_val_if_fail (data, NULL);

	return NULL;
}

static void
debug_well_known_targets (GtkClipboard *clipboard)
{
	debug_try_selection_type (clipboard, "UTF8_STRING", convert_utf8_string);
	debug_try_selection_type (clipboard, "text/xml", convert_text_xml);
	debug_try_selection_type (clipboard, "text/html", convert_text_html);
	debug_try_selection_type (clipboard, "text/plain", convert_text_plain);
	debug_try_selection_type (clipboard, "application/xhtml+xml", convert_application_xhtml_plus_xml);
	debug_try_selection_type (clipboard, "application/rtf", convert_application_rtf);

	/*
	  Of the above
	  Evolution offers 9 atoms in TARGETS; of the above:
	  - "UTF8_STRING" as UTF-8, format 8
	  - "text/html", appears to be UCS-2, format 16, though I got some trailing junk characters.  Also appears to have capitalised the element names.

	  Emacs doesn't offer any (20 TARGETS reported, though)

	  Mozilla offers 107 atoms in TARGETS; of the above:
	  - "UTF8_STRING" as UTF-8, format 8
	  - "text/html", appears to be UCS-2, but format=8, and be a genuine fragment of the document source.

	  OpenOffice.org Writer offers 10 atoms in TARGETS, of the above:
	  - "UTF8_STRING" as UTF-8, format=8
	  - "text/html" as a UTF-8 document, format=8 consisting of a <!DOCTYPE declaration> with <HTML> element, <HEAD>, and a <BODY> containing the highlighted text!
	  - "text/plain"; probably as UCS-2, though format=8
	  
	  AbiWord offers 6 atoms in TARGETS, though none of the above

	  So how do we distinguish between UTF-8 and UCS-2???
	 */
}

/* This is a simple copy-and-paste of gtk_clipboard_wait_for_targets, which was added to GTK in version 2.4: */
gboolean
cong_eel_gtk_clipboard_wait_for_targets (GtkClipboard  *clipboard, 
					 GdkAtom      **targets,
					 gint          *n_targets)
{
  GtkSelectionData *data;
  gboolean result = FALSE;
  
  g_return_val_if_fail (clipboard != NULL, FALSE);

  /* TODO: see http://bugzilla.gnome.org/show_bug.cgi?id=101774 with regard to XFIXES */

  if (n_targets)
    *n_targets = 0;
      
  targets = NULL;      

  data = gtk_clipboard_wait_for_contents (clipboard, gdk_atom_intern ("TARGETS", FALSE));

  if (data)
    {
      result = gtk_selection_data_get_targets (data, targets, n_targets);
      gtk_selection_data_free (data);
    }

  return result;
}

static gchar*
generate_source_fragment_from_selection_doc (xmlDocPtr xml_doc,
					     CongDocument *target_doc)
{
	xmlNodePtr iter;

	g_assert (xml_doc);
	g_assert (IS_CONG_DOCUMENT (target_doc));

	/* FIXME:  Bugzilla #130445
	   Eventually we may want to do transformations/conversions based on the DOCTYPE of the input xml_doc and the type of the target CongDocument.

	   For now we just strip off the headers and get at the content inside the top-level <xml-fragment> element:
	*/
	for (iter=xml_doc->children;iter;iter=iter->next) {
		if (cong_node_type (iter)==CONG_NODE_TYPE_ELEMENT) {
			/* This is the top-level element; we expect it to be an <xml-fragment>: */
			return cong_node_generate_child_source (iter);
		}
	}

	return NULL;
}

gchar*
cong_app_get_clipboard_xml_source (CongApp *app,
				   GdkAtom selection,
				   CongDocument *target_doc)
{
	GtkClipboard* clipboard;
	GdkAtom *targets;
	gint n_targets;

	g_return_val_if_fail (app, NULL);
	g_return_val_if_fail ((selection == GDK_SELECTION_CLIPBOARD)||(selection == GDK_SELECTION_PRIMARY), NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT (target_doc), NULL);

	clipboard = gtk_clipboard_get (selection);

	/* Try XML, then text: */
	{
		/* Try XML: */
		{
			GtkSelectionData *selection_data = gtk_clipboard_wait_for_contents (clipboard,
											    gdk_atom_intern ("text/xml", FALSE));
			
			if (selection_data) {
				/* Parse, potentially convert to another XML format, then output in the desired form: */
				xmlDocPtr xml_doc = xmlParseMemory (selection_data->data,
								    selection_data->length);

				gtk_selection_data_free (selection_data);

				if (xml_doc) {
					gchar *source_fragment = generate_source_fragment_from_selection_doc (xml_doc,
													      target_doc);
					xmlFreeDoc (xml_doc);

					if (source_fragment) {
						LOG_SELECTIONS2 ("Got clipboard data as XML: \"%s\"", source_fragment);
						return source_fragment;
					} else {
						LOG_SELECTIONS1 ("Failed to generate source fragment from selected XML");
					}
				} else {
					LOG_SELECTIONS1 ("Parsing of XML from selection failed");
				}	
			}
		}
		
		/* FIXME: Bugzilla #130447 : eventually try HTML, and perhaps arbitrary plugins: */
		
		/* Try text: */
		{
			gchar *raw_text = gtk_clipboard_wait_for_text (clipboard);

			if (raw_text) {			
				/* Need to escape any text: */
				gchar *escaped_text = g_markup_escape_text (raw_text, strlen(raw_text));
				
				g_free (raw_text);

				LOG_SELECTIONS2 ("Got clipboard data as text \"%s\"", escaped_text);
				return escaped_text;
			}			
		}
	}

	return NULL;

#if 0
	if (cong_eel_gtk_clipboard_wait_for_targets (clipboard,
						     &targets,
						     &n_targets)) {
		#if 0
		debug_target_list (clipboard,
				   targets,
				   n_targets);
		g_free (targets);
		#endif

		debug_well_known_targets (clipboard);

		return gtk_clipboard_wait_for_text (clipboard);
	} else {
		return NULL;
	}
#endif

#if 0

	/* FIXME: Do as UTF-8 text for now, ultimately should support multiple formats... */
	return gtk_clipboard_wait_for_text (clipboard);
#endif
}

/* FIXME: Bugzilla bug #130440:  Offer HTML as a selection format as well */
enum CongTargetTypes {
	CONG_TARGET_TYPE_TEXT,
	CONG_TARGET_TYPE_XML
};

static const GtkTargetEntry selection_targets[] = 
{
	/* Offer various kinds of well-known textual atoms; we use gtk_selection_data_set_text for each of these, which should handle any necessary encoding conversions: */
	{ "STRING", 0, CONG_TARGET_TYPE_TEXT},
	{ "TEXT",   0, CONG_TARGET_TYPE_TEXT}, 
	{ "COMPOUND_TEXT", 0, CONG_TARGET_TYPE_TEXT},
	{ "UTF8_STRING", 0, CONG_TARGET_TYPE_TEXT},

	/* Offer "text/xml": */
	{ "text/xml", 0, CONG_TARGET_TYPE_XML}
};

static void 
clipboard_get_cb (GtkClipboard *clipboard,
		  GtkSelectionData *selection_data,
		  guint info,
		  gpointer user_data_or_owner)
{
	CongSelectionData *cong_selection = (CongSelectionData*)user_data_or_owner;

	switch (info) {
	default: g_assert_not_reached();
	case CONG_TARGET_TYPE_TEXT:
		/* We offer various textual atoms, and trust gtk_selection_data_set_text to do any necessary conversions: */
		LOG_SELECTIONS2 ("Providing one of the text formats for selection: \"%s\"", cong_selection->utf8_text);
		gtk_selection_data_set_text (selection_data,
					     cong_selection->utf8_text,
					     -1);
		break;
	case CONG_TARGET_TYPE_XML:
		LOG_SELECTIONS2 ("Providing \"text/xml\" for selection: \"%s\"", cong_selection->utf8_xml_buffer);
		gtk_selection_data_set (selection_data,
					gdk_atom_intern ("text/xml", FALSE),
					8,
					cong_selection->utf8_xml_buffer,
					strlen (cong_selection->utf8_xml_buffer));
		break;
	}
}

static void
clipboard_clear_cb (GtkClipboard *clipboard,
		    gpointer user_data_or_owner)
{
}

static gchar*
generate_xml_for_selection (const gchar* xml_fragment,
			    CongDocument *source_doc)
{
	const CongXMLChar* dtd_public_identifier;

	g_assert (xml_fragment);
	g_assert (IS_CONG_DOCUMENT (source_doc));


	dtd_public_identifier = cong_document_get_dtd_public_identifier(source_doc);
	
	if (dtd_public_identifier) {
		/* FIXME: Bugzilla bug #130443 add a DOCTYPE declaration if available */
		return g_strdup_printf ("<?xml version=\"1.0\" ?>\n"
					"<xml-fragment>%s</xml-fragment>", xml_fragment);
	} else {
		return g_strdup_printf ("<?xml version=\"1.0\" ?>\n"
					"<xml-fragment>%s</xml-fragment>", xml_fragment);
	}
}

static gchar*
generate_text_for_selection (CongApp *app,
			     const gchar* xml_fragment,
			     CongDocument *source_doc)
{
	gchar *doc_source;
	xmlDocPtr xml_doc_input;
	xmlDocPtr xml_doc_result;

	g_assert (app);
	g_assert (xml_fragment);
	g_assert (IS_CONG_DOCUMENT (source_doc));

	doc_source = g_strdup_printf ("<?xml version=\"1.0\"?>\n<placeholder>%s</placeholder>", xml_fragment);
	
	xml_doc_input = xmlParseMemory (doc_source, 
					strlen(doc_source));
	g_free (doc_source);
	
	if (xml_doc_input) {

		/* FIXME:  Bugzilla bug #130442: perhaps we should choose a stylesheet based on source doc type */
		xml_doc_result = xsltApplyStylesheet (PRIVATE(app)->xsl_selection_to_text, xml_doc_input, NULL);
		xmlFreeDoc (xml_doc_input);

		if (xml_doc_result) {
			gchar *result;
			/* The stylesheet outputs as text, so we should have a single TEXT node directly below the document containing the text: */
			if (xml_doc_result->children) {
				g_assert (xml_doc_result->children == xml_doc_result->last);
				g_assert (xml_doc_result->children->type == XML_TEXT_NODE);

				result = g_strdup (xml_doc_result->children->content);		
			} else {
				result = g_strdup ("");
			}

			xmlFreeDoc (xml_doc_result);

			return result;
		} else {
			g_warning ("Couldn't apply stylesheet");
		}
	}

	return NULL;
}


static void
regenerate_selection (CongApp *app,
		      CongSelectionData *cong_selection,
		      const gchar* xml_fragment,
		      CongDocument *source_doc)
{
	g_assert (cong_selection);
	g_assert (xml_fragment);
	g_assert (IS_CONG_DOCUMENT (source_doc));

	/* Regenerate XML: */
	{
		if (cong_selection->utf8_xml_buffer) {
			g_free (cong_selection->utf8_xml_buffer);
		}

		cong_selection->utf8_xml_buffer = generate_xml_for_selection (xml_fragment, source_doc);
	}

	/* Regenerate textual version: */
	{
		if (cong_selection->utf8_text) {
			g_free (cong_selection->utf8_text);
		}

		cong_selection->utf8_text = generate_text_for_selection (app,
									 xml_fragment, 
									 source_doc);

		/* FIXME: reduce debug spew as well! */
	}
}



void
cong_app_set_clipboard_from_xml_fragment (CongApp *app,
					  GdkAtom selection,
					  const gchar* xml_fragment,
					  CongDocument *source_doc)
{
	GtkClipboard *clipboard;
	CongSelectionData *cong_selection;

	g_return_if_fail (app);
	g_return_if_fail ((selection == GDK_SELECTION_CLIPBOARD)||(selection == GDK_SELECTION_PRIMARY));
	g_return_if_fail (xml_fragment);
	g_return_if_fail (IS_CONG_DOCUMENT (source_doc));

	clipboard = gtk_clipboard_get (selection);

	if (selection==GDK_SELECTION_CLIPBOARD) {
		cong_selection = &(PRIVATE(app)->clipboard_selection);
	} else {
		cong_selection = &(PRIVATE(app)->primary_selection);
	}

	/* For now we build all possible types in advance and store, ready to be supply whatever is requested.  Eventually we might try to be smarter about this: */
	regenerate_selection (app,
			      cong_selection,
			      xml_fragment,
			      source_doc);

	gtk_clipboard_set_with_data (clipboard,
				     selection_targets,
				     G_N_ELEMENTS (selection_targets),
				     clipboard_get_cb,
				     clipboard_clear_cb,
				     cong_selection);

	LOG_SELECTIONS2("Clipboard set to \"%s\"", xml_fragment);

	/* emit signals? */
}

GnomeProgram*
cong_app_get_gnome_program (CongApp *app)
{
	g_return_val_if_fail (app, NULL);

	return PRIVATE(app)->gnome_program;
}

GtkTooltips*
cong_app_get_tooltips (CongApp *app)
{
	g_return_val_if_fail (app, NULL);

	return PRIVATE(app)->tooltips;
}

CongFont*
cong_app_get_font (CongApp *app,
		   enum CongFontRole role)
{
	g_return_val_if_fail (app, NULL);
	g_return_val_if_fail (role<CONG_FONT_ROLE_NUM, NULL);

	return PRIVATE(app)->fonts[role];
}

CongPluginManager*
cong_app_get_plugin_manager (CongApp *app)
{
	g_return_val_if_fail (app, NULL);

	return PRIVATE(app)->plugin_manager;
}

CongDispspecRegistry*
cong_app_get_dispspec_registry (CongApp *app)
{
	g_return_val_if_fail (app, NULL);

	return PRIVATE(app)->ds_registry;
}

GConfClient*
cong_app_get_gconf_client (CongApp *app)
{
	g_return_val_if_fail (app, NULL);

	return PRIVATE(app)->gconf_client;
}

const GList*
cong_app_get_language_list (CongApp *app)
{
	g_return_val_if_fail (app, NULL);

	return PRIVATE(app)->language_list;
}



/* Internal function definitions: */
static CongApp*
cong_app_new (int   argc,
	      char *argv[])
{
	CongApp *app;

	app = g_new0(CongApp,1);
	app->private = g_new0(CongAppPrivate,1);

	/* Set up the GnomeProgram: */
	PRIVATE(app)->gnome_program = gnome_program_init (PACKAGE_NAME, PACKAGE_VERSION,
							  LIBGNOMEUI_MODULE,
							  argc,argv,
							  GNOME_PARAM_HUMAN_READABLE_NAME,
							  _("XML Editor"),
							  GNOME_PARAM_APP_DATADIR, PKGDATADIR,
							  /* GSt: bugzilla # 128544 reminder */
							  NULL);

	/* Set up usage of GConf: */
	PRIVATE(app)->gconf_client = gconf_client_get_default();
	gconf_client_add_dir (PRIVATE(app)->gconf_client,
			      "/apps/conglomerate",
			      GCONF_CLIENT_PRELOAD_NONE,
			      NULL);

	cong_app_private_load_fonts (app);

	PRIVATE(app)->tooltips = gtk_tooltips_new();

	PRIVATE(app)->language_list = gnome_i18n_get_language_list (NULL);

	{
		const GList *iter;

		for (iter = PRIVATE(app)->language_list; iter; iter=iter->next) {
			g_message ("\"%s\"", (gchar*)iter->data);
		}
	}

	/* Load stylesheets: */
	{
		gchar *stylesheet_file = gnome_program_locate_file (PRIVATE(app)->gnome_program,
								    GNOME_FILE_DOMAIN_APP_DATADIR,
								    "stylesheets/selection-to-text.xsl",
								    FALSE,
								    NULL);

		PRIVATE (app)->xsl_selection_to_text = xsltParseStylesheetFile (stylesheet_file);
		if (PRIVATE (app)->xsl_selection_to_text==NULL) {
			g_warning ("Couldn't parse stylesheet %s", stylesheet_file);
		}
		g_free (stylesheet_file);
	}

	return app;	
}

static void
cong_app_free (CongApp *app)
{
	g_return_if_fail (app);

	xsltFreeStylesheet (PRIVATE (app)->xsl_selection_to_text);

	g_free(app->private);
	g_free(app);
}

static void 
cong_app_private_load_fonts (CongApp *app)
{
#if TEST_BIG_FONTS
	  PRIVATE(app)->fonts[CONG_FONT_ROLE_BODY_TEXT] = cong_font_load("sans 20");
	  PRIVATE(app)->fonts[CONG_FONT_ROLE_TITLE_TEXT] = cong_font_load("sans 24");
	  PRIVATE(app)->fonts[CONG_FONT_ROLE_SPAN_TAG] = cong_font_load("sans 16");
#else
	  PRIVATE(app)->fonts[CONG_FONT_ROLE_BODY_TEXT] = cong_font_load("sans 10");
	  PRIVATE(app)->fonts[CONG_FONT_ROLE_TITLE_TEXT] = cong_font_load("sans 12");
	  PRIVATE(app)->fonts[CONG_FONT_ROLE_SPAN_TAG] = cong_font_load("sans 8");
#endif
}

static gboolean 
cong_app_private_load_displayspecs (CongApp *app,
				    GtkWindow *toplevel_window)
{
	GSList *ds_path_list = NULL;
	GSList *path;

	/* Create new empty registry */
	PRIVATE(app)->ds_registry = cong_dispspec_registry_new(NULL, toplevel_window);
	if (PRIVATE(app)->ds_registry==NULL) {
		return FALSE;
	}
	
	/* Dispspec search goes from the start of the list to the end, so
	   we need to load in the most-trusted sources first.

	   Then load from the standard installation path.
	*/

	/* Load gconf-specified custom directories */
	ds_path_list = gconf_client_get_list(PRIVATE(app)->gconf_client,
					     "/apps/conglomerate/custom-dispspec-paths",
					     GCONF_VALUE_STRING,
					     NULL);
	/* Now run through the path list in order, adding dispspecs to
	   the registry from each dir. */
	path = ds_path_list;
	while (path != NULL) {
		if (path->data != NULL) {
			gchar *realpath;
			realpath = gnome_vfs_expand_initial_tilde((char *)(path->data));
			g_message("Loading xds files from \"%s\"\n", realpath);
			cong_dispspec_registry_add_dir(PRIVATE(app)->ds_registry, realpath, toplevel_window, 0);
			g_free (realpath);
		}
		path = g_slist_next(path);
	}
	g_slist_free (ds_path_list);

	/* Finally, try the standard installation path.  This used to be listed in the GConf path, see Bugzilla #129776 */
	{
		gchar* xds_directory = gnome_program_locate_file (PRIVATE(app)->gnome_program,
								  GNOME_FILE_DOMAIN_APP_DATADIR,
								  "dispspecs",
								  FALSE,
								  NULL);

		
		cong_dispspec_registry_add_dir (PRIVATE(app)->ds_registry, xds_directory, toplevel_window, 0);
		
		g_free (xds_directory);
	}
		
	
	/* If no xds files were found anywhere, perhaps the program
	   hasn't been installed yet (merely built): */
	if (cong_dispspec_registry_get_num(PRIVATE(app)->ds_registry)==0) {
		
		gchar *why_failed = g_strdup_printf(_("Conglomerate could not load any xds files"));
		GtkDialog* dialog = cong_error_dialog_new(toplevel_window,
							  _("Conglomerate could not find any descriptions of document types."),
							  why_failed,
							  _("If you see this error, it is likely that you built Conglomerate, but did not install it.  Try installing it.  If you have changed the default setting for the display spec path, please double check that as well."));
		g_free(why_failed);
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		return FALSE;
	}
		
	cong_dispspec_registry_dump(PRIVATE(app)->ds_registry);
	
	return TRUE;
}

static void 
register_plugin (CongApp *app,
		 const gchar *id,
		 CongPluginCallbackRegister register_callback,
		 CongPluginCallbackConfigure configure_callback)
{
	g_return_if_fail(id);
	g_return_if_fail(register_callback);

	g_assert(PRIVATE(app)->plugin_manager);

	cong_plugin_manager_register(PRIVATE(app)->plugin_manager,
				     id,
				     register_callback, 
				     configure_callback);
}



static void 
cong_app_private_load_plugins (CongApp *app)
{
	/* For the moment, there aren't any actual plugins; instead we fake it. */
	register_plugin(app,"admonition",
			plugin_admonition_plugin_register,
			plugin_admonition_plugin_configure);

	register_plugin(app,"docbook",
			plugin_docbook_plugin_register,
			plugin_docbook_plugin_configure);

	register_plugin(app,"empty",
			plugin_empty_plugin_register,
			plugin_empty_plugin_configure);

	register_plugin(app,"fo",
			plugin_fo_plugin_register,
			plugin_fo_plugin_configure);

	register_plugin(app,"lists",
			plugin_lists_plugin_register,
			plugin_lists_plugin_configure);

	register_plugin(app,"sgml",
			plugin_sgml_plugin_register,
			plugin_sgml_plugin_configure);

	register_plugin(app,"tests",
			plugin_tests_plugin_register,
			plugin_tests_plugin_configure);

	register_plugin(app,"validate",
			plugin_validate_plugin_register,
			plugin_validate_plugin_configure);

	register_plugin(app,"website",
			plugin_website_plugin_register,
			plugin_website_plugin_configure);

	register_plugin(app,"xsl",
			plugin_xsl_plugin_register,
			plugin_xsl_plugin_configure);

	register_plugin(app,"convert-case",
			plugin_convert_case_plugin_register,
			plugin_convert_case_plugin_configure);

	register_plugin(app,"cleanup-source",
			plugin_cleanup_source_plugin_register,
			plugin_cleanup_source_plugin_configure);

	register_plugin(app,"dtd",
			plugin_dtd_plugin_register,
			plugin_dtd_plugin_configure);

	register_plugin(app,"paragraph",
			plugin_paragraph_plugin_register,
			plugin_paragraph_plugin_configure);

	register_plugin(app,"save-dispspec",
			plugin_save_dispspec_plugin_register,
			plugin_save_dispspec_plugin_configure);

	register_plugin(app,"templates",
			plugin_templates_plugin_register,
			plugin_templates_plugin_configure);
}

static void 
cong_app_private_insert_element_init (CongApp *app)
{
#if 0
	GdkColor gcol;

	app->insert_element_gc = gdk_gc_new(cong_gui_get_a_window()->window);
	gdk_gc_copy(app->insert_element_gc, cong_gui_get_a_window()->style->white_gc);
	col_to_gcol(&gcol, 0x00e0e0e0);
	gdk_colormap_alloc_color(cong_gui_get_a_window()->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(app->insert_element_gc, &gcol);
#endif
}








