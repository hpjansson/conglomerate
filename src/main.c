/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "global.h"
#include "cong-dialog.h"
#include "cong-error-dialog.h"
#include "cong-dispspec.h"
#include "cong-dispspec-registry.h"
#include "cong-document.h"
#include "cong-plugin.h"
#include "cong-font.h"

#if 0
#include <libgtkhtml/gtkhtml.h>
#endif

gchar* cong_util_cleanup_text(const xmlChar *text) {
	gchar *buffer = g_malloc((strlen(text)*3)+1); /* for safety's sake */
	gchar *dst = buffer;

	/* FIXME: this is broken; we need to have real UTF8 support here... */

	while (*text) {
		switch (*text) {
		default:
			*(dst++) = *text;
			break;
		case '\n':
			*(dst++) = '\\';
			*(dst++) = 'n';
			break;
		case '\t':
			*(dst++) = '\\';
			*(dst++) = 't';
			break;
		}

		text++;
	}

	*(dst++) = '\0';

	return buffer;
}

const gchar*
cong_utils_get_norman_walsh_stylesheet_path(void)
{
	/* FIXME:  This is currently a nasty hack; how should we set this up so it "just works"? */

#if 0
	/* Value that worked for Dave Malcolm on SuSE 7.1: */
	return "/usr/share/sgml/docbkxsl/";
#endif

#if 1
	/* Value that works for Dave Malcolm on Red Hat 8: */
	return "/usr/share/sgml/docbook/xsl-stylesheets-1.50.0-3/";
#endif
}

gchar*
cong_utils_get_norman_walsh_stylesheet(const gchar *stylesheet_relative_path)
{
	g_return_val_if_fail(stylesheet_relative_path, NULL);

	return g_strdup_printf("%s%s",cong_utils_get_norman_walsh_stylesheet_path(), stylesheet_relative_path);
}


/*
#define AUTOGENERATE_DS
*/

/*
  A routine that tries to load all the bytes requested from the handle into the buffer and bails out on any failure
 */
GnomeVFSResult
cong_vfs_read_bytes(GnomeVFSHandle* vfs_handle, char* buffer, GnomeVFSFileSize bytes)
{
	GnomeVFSFileSize bytes_read;
	GnomeVFSResult vfs_result = gnome_vfs_read(vfs_handle,buffer,bytes,&bytes_read);

	g_assert(bytes==bytes_read); /* for now */

	return vfs_result;
}

/* 
   A routine that tries to syncronously load a file into a buffer in memory (surely this exists already somewhere?)
*/
GnomeVFSResult
cong_vfs_new_buffer_from_file(const char* filename, char** buffer, GnomeVFSFileSize* size)
{
	GnomeVFSResult vfs_result;
	GnomeVFSURI* uri;

	g_return_val_if_fail(filename,GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(buffer,GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(size,GNOME_VFS_ERROR_BAD_PARAMETERS);

	uri = gnome_vfs_uri_new(filename);

	vfs_result = cong_vfs_new_buffer_from_uri(uri, buffer, size);

	gnome_vfs_uri_unref(uri);

	return vfs_result;
}

GnomeVFSResult
cong_vfs_new_buffer_from_uri(GnomeVFSURI* uri, char** buffer, GnomeVFSFileSize* size)
{
	GnomeVFSResult vfs_result;
	GnomeVFSHandle *vfs_handle;

	g_return_val_if_fail(uri,GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(buffer,GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(size,GNOME_VFS_ERROR_BAD_PARAMETERS);

	vfs_result = gnome_vfs_open_uri(&vfs_handle,
					uri,
					GNOME_VFS_OPEN_READ);

	if (GNOME_VFS_OK!=vfs_result) {
		return vfs_result;
	} else {
		GnomeVFSFileInfo info;
		*buffer=NULL;
		
		/* Get the size of the file: */
		vfs_result = gnome_vfs_get_file_info_from_handle(vfs_handle,
								 &info,
								 GNOME_VFS_FILE_INFO_DEFAULT);
		if (GNOME_VFS_OK!=vfs_result) {
			gnome_vfs_close(vfs_handle);
			
			return vfs_result;
		}

		if (!(info.valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE)) {
			gnome_vfs_close(vfs_handle);
			
			return GNOME_VFS_ERROR_IO; /* FIXME: is this appropriate? */
		}

		
		/* Allocate the buffer: */
		*buffer = g_malloc(info.size);
		
		/* Read the file into the buffer: */
		vfs_result = cong_vfs_read_bytes(vfs_handle, *buffer, info.size);
		
		if (GNOME_VFS_OK!=vfs_result) {
			
			g_free(*buffer);
			gnome_vfs_close(vfs_handle);

			*buffer=NULL;
			
			return vfs_result;
		}
		
		gnome_vfs_close(vfs_handle);
		*size = info.size;

		return GNOME_VFS_OK;
	}
}



void insert_element_init()
{
	GdkColor gcol;

	the_globals.insert_element_gc = gdk_gc_new(cong_gui_get_a_window()->window);
	gdk_gc_copy(the_globals.insert_element_gc, cong_gui_get_a_window()->style->white_gc);
	col_to_gcol(&gcol, 0x00e0e0e0);
	gdk_colormap_alloc_color(cong_gui_get_a_window()->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(the_globals.insert_element_gc, &gcol);
}







void fonts_load()
{
#if 1
	  the_globals.fonts[CONG_FONT_ROLE_BODY_TEXT] = cong_font_load("sans 10");
	  the_globals.fonts[CONG_FONT_ROLE_TITLE_TEXT] = cong_font_load("sans 12");
	  the_globals.fonts[CONG_FONT_ROLE_SPAN_TAG] = cong_font_load("sans 8");
#else
	  the_globals.fonts[CONG_FONT_ROLE_BODY_TEXT] = cong_font_load("sans 10");
	  the_globals.fonts[CONG_FONT_ROLE_TITLE_TEXT] = cong_font_load("sans 12");
	  the_globals.fonts[CONG_FONT_ROLE_SPAN_TAG] = cong_font_load("sans 6");
#endif
}




void status_update()
{
  while (g_main_iteration(FALSE));
}


static gint popup_deactivate(GtkWidget *widget, GdkEvent *event)
{
	return(TRUE);
}

gboolean main_load_displayspecs(GtkWindow *toplevel_window)
{
#if 1
	gchar*      xds_directory = gnome_program_locate_file(the_globals.gnome_program,
							      GNOME_FILE_DOMAIN_APP_DATADIR,
							      "conge/dispspecs",
							      FALSE,
							      NULL);
#else
	gchar* current_dir = g_get_current_dir();
	gchar* xds_directory = g_strdup_printf(DATADIR"/conge/dispspecs",current_dir);
	g_free(current_dir);
#endif

	g_message(DATADIR);

	if (xds_directory) {
		g_message("Loading xds files from \"%s\"\n", xds_directory);
		the_globals.ds_registry = cong_dispspec_registry_new(xds_directory, toplevel_window);

		/* If no xds files were found, perhaps the program hasn't been installed yet (merely built): */
		if (cong_dispspec_registry_get_num(the_globals.ds_registry)==0) {

			gchar *why_failed = g_strdup_printf(_("Conglomerate could not load any xds files from the directory \"%s\""), xds_directory);
			GtkDialog* dialog = cong_error_dialog_new(toplevel_window,
								  _("Conglomerate could not find any descriptions of document types."),
								  why_failed,
								  _("If you see this error, it is likely that you built Conglomerate, but did not install it.  Try installing it."));
			g_free(why_failed);
			cong_error_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(GTK_WIDGET(dialog));
			return FALSE;
		}
		
		g_free(xds_directory);
		
		if (the_globals.ds_registry==NULL) {
			return FALSE;
		}
		
		cong_dispspec_registry_dump(the_globals.ds_registry);
	} else {
		GtkDialog* dialog = cong_error_dialog_new(toplevel_window,
							  "Conglomerate could not find its registry of document types.",
							  "You must run the program from the \"src\" directory used to build it.",
							  "This is a known problem and will be fixed.");
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		return FALSE;
	}

	return TRUE;
}

void register_plugin(const gchar *id,
		     CongPluginCallbackRegister register_callback,
		     CongPluginCallbackConfigure configure_callback)
{
	g_return_if_fail(id);
	g_return_if_fail(register_callback);

	g_assert(the_globals.plugin_manager);

	cong_plugin_manager_register(the_globals.plugin_manager,
				     id,
				     register_callback, 
				     configure_callback);
}



void main_load_plugins(void)
{
	/* For the moment, there aren't any actual plugins; instead we fake it. */

	register_plugin("docbook",
			plugin_docbook_plugin_register,
			plugin_docbook_plugin_configure);

	register_plugin("lists",
			plugin_lists_plugin_register,
			plugin_lists_plugin_configure);

	register_plugin("tests",
			plugin_tests_plugin_register,
			plugin_tests_plugin_configure);

	register_plugin("xsl",
			plugin_xsl_plugin_register,
			plugin_xsl_plugin_configure);
}

int main( int   argc,
	  char *argv[] )
{
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	the_globals.gnome_program = gnome_program_init(PACKAGE_NAME, PACKAGE_VERSION,
						       LIBGNOMEUI_MODULE,
						       argc,argv,
						       GNOME_PARAM_HUMAN_READABLE_NAME,
						       _("XML Editor"),
						       GNOME_PARAM_APP_DATADIR, DATADIR,
						       NULL);

	/* Set up usage of GConf: */
	the_globals.gconf_client = gconf_client_get_default();
	gconf_client_add_dir(the_globals.gconf_client,
			     "/apps/conglomerate",
			     GCONF_CLIENT_PRELOAD_NONE,
			     NULL);

	fonts_load();
	editor_popup_init(NULL); /* FIXME */

	cong_primary_window_new(NULL);
	
	/* Load all the displayspec (xds) files: */
	/* 
	   FIXME: currently this function requires a primary window to exist, so it can manipulate graphics contexts... 
	   Ideally we would only create a "document-less" window if no file was specified on the command line.
	*/
	if (!main_load_displayspecs(NULL)) {
		return 1;
	}

	/* 
	   Load all the plugins.  We do this after loading the xds files in case some of the plugins want to operate on the registry
	   of displayspecs
	 */
	the_globals.plugin_manager = cong_plugin_manager_new();
	main_load_plugins();

	insert_element_init();

	the_globals.clipboard = NULL;

#if 1
	/* 
	   If we have any arguments, interpret the final one as the name of a file to be opened: 

	   FIXME: should we use popt?
	*/
	if (argc > 1) {
		open_document_do(argv[argc-1], NULL);
	}
#endif

	gtk_main();

	return(0);
}
