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

#include <libgnomevfs/gnome-vfs.h>

#include "cong-fake-plugin-hooks.h"

#define CORRECT_CLIPBOARD 1
#define TEST_BIG_FONTS 0

#define PRIVATE(x) ((x)->private)

/* Internal data structure declarations: */
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

#if !CORRECT_CLIPBOARD
	gchar *clipboard; /* can be NULL to signify nothing in clipboard*/
#endif

	const GList *language_list;
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

const gchar*
cong_app_get_clipboard (CongApp *app)
{
#if CORRECT_CLIPBOARD
	GtkClipboard* clipboard;
#endif

	g_return_val_if_fail (app, NULL);

#if CORRECT_CLIPBOARD
	clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);

	/* FIXME: Do as UTF-8 text for now, ultimately should support multiple formats... */
	return gtk_clipboard_wait_for_text (clipboard);

#else
	return PRIVATE(app)->clipboard;
#endif
}

void
cong_app_set_clipboard (CongApp *app, 
			const gchar* text)
{
#if CORRECT_CLIPBOARD
	GtkClipboard* clipboard;
#endif
	g_return_if_fail (app);
	/* text is allowed to be NULL */

#if CORRECT_CLIPBOARD
	clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	
	if (text) {
		/* FIXME: Do as UTF-8 text for now, ultimately should support multiple formats... */
		gtk_clipboard_set_text (clipboard, text, -1);
	} else {
		/* FIXME: should this happen? */
	}
#else
	if (PRIVATE(app)->clipboard) {
		g_free(PRIVATE(app)->clipboard);
	}

	if (text) {
		PRIVATE(app)->clipboard = g_strdup(text);
	} else {
		PRIVATE(app)->clipboard = NULL;
	}
#endif

	g_message("Clipboard set to \"%s\"", text);

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


	return app;	
}

static void
cong_app_free (CongApp *app)
{
	g_return_if_fail (app);

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
	GSList *ds_path_list;
	GSList *path;

	/* Create new empty registry */
	PRIVATE(app)->ds_registry = cong_dispspec_registry_new(NULL, toplevel_window);
	if (PRIVATE(app)->ds_registry==NULL) {
		return FALSE;
	}
	
	/* Dispspec search goes from the start of the list to the end, so
	   we need to load in the most-trusted sources first */

	/* Load gconf-specified directories */

	ds_path_list = gconf_client_get_list(PRIVATE(app)->gconf_client,
					     "/apps/conglomerate/dispspec-paths",
					     GCONF_VALUE_STRING,
					     NULL);
	
	if (ds_path_list == NULL) {
		/* Fallback in the case where there is nothing in GConf.
		   Look in application default directory for dispspecs. */
		gchar* xds_directory
			= gnome_program_locate_file(PRIVATE(app)->gnome_program,
						    GNOME_FILE_DOMAIN_APP_DATADIR,
						    "dispspecs",
						    FALSE,
						    NULL);
		ds_path_list = g_slist_append (ds_path_list, (gpointer)xds_directory);
	}
		
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








