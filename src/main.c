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
#include "cong-app.h"
#include "cong-fake-plugin-hooks.h"

#define TEST_BIG_FONTS 0

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
		GnomeVFSFileInfo *info;
		*buffer=NULL;
		
		info = gnome_vfs_file_info_new ();

		/* Get the size of the file: */
		vfs_result = gnome_vfs_get_file_info_from_handle(vfs_handle,
								 info,
								 GNOME_VFS_FILE_INFO_DEFAULT);
		if (GNOME_VFS_OK!=vfs_result) {
			gnome_vfs_close(vfs_handle);
			gnome_vfs_file_info_unref (info);
			
			return vfs_result;
		}

		if (!(info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE)) {
			gnome_vfs_close(vfs_handle);
			gnome_vfs_file_info_unref (info);
			
			return GNOME_VFS_ERROR_IO; /* FIXME: is this appropriate? */
		}

		
		/* Allocate the buffer: */
		*buffer = g_malloc(info->size);
		
		/* Read the file into the buffer: */
		vfs_result = cong_vfs_read_bytes(vfs_handle, *buffer, info->size);
		
		if (GNOME_VFS_OK!=vfs_result) {
			
			g_free(*buffer);
			gnome_vfs_close(vfs_handle);
			gnome_vfs_file_info_unref (info);

			*buffer=NULL;
			
			return vfs_result;
		}
		
		gnome_vfs_close(vfs_handle);
		
		*size = info->size;
		
		gnome_vfs_file_info_unref (info);

		return GNOME_VFS_OK;
	}
}



void insert_element_init()
{
	GdkColor gcol;

	cong_app_singleton()->insert_element_gc = gdk_gc_new(cong_gui_get_a_window()->window);
	gdk_gc_copy(cong_app_singleton()->insert_element_gc, cong_gui_get_a_window()->style->white_gc);
	col_to_gcol(&gcol, 0x00e0e0e0);
	gdk_colormap_alloc_color(cong_gui_get_a_window()->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(cong_app_singleton()->insert_element_gc, &gcol);
}







void fonts_load()
{
#if TEST_BIG_FONTS
	  cong_app_singleton()->fonts[CONG_FONT_ROLE_BODY_TEXT] = cong_font_load("sans 20");
	  cong_app_singleton()->fonts[CONG_FONT_ROLE_TITLE_TEXT] = cong_font_load("sans 24");
	  cong_app_singleton()->fonts[CONG_FONT_ROLE_SPAN_TAG] = cong_font_load("sans 16");
#else
	  cong_app_singleton()->fonts[CONG_FONT_ROLE_BODY_TEXT] = cong_font_load("sans 10");
	  cong_app_singleton()->fonts[CONG_FONT_ROLE_TITLE_TEXT] = cong_font_load("sans 12");
	  cong_app_singleton()->fonts[CONG_FONT_ROLE_SPAN_TAG] = cong_font_load("sans 8");
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
	gchar*      xds_directory = gnome_program_locate_file(cong_app_singleton()->gnome_program,
							      GNOME_FILE_DOMAIN_APP_DATADIR,
							      "conglomerate/dispspecs",
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
		cong_app_singleton()->ds_registry = cong_dispspec_registry_new(xds_directory, toplevel_window);

		/* If no xds files were found, perhaps the program hasn't been installed yet (merely built): */
		if (cong_dispspec_registry_get_num(cong_app_singleton()->ds_registry)==0) {

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
		
		if (cong_app_singleton()->ds_registry==NULL) {
			return FALSE;
		}
		
		cong_dispspec_registry_dump(cong_app_singleton()->ds_registry);
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

	g_assert(cong_app_singleton()->plugin_manager);

	cong_plugin_manager_register(cong_app_singleton()->plugin_manager,
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

	register_plugin("empty",
			plugin_empty_plugin_register,
			plugin_empty_plugin_configure);

	register_plugin("fo",
			plugin_fo_plugin_register,
			plugin_fo_plugin_configure);

	register_plugin("lists",
			plugin_lists_plugin_register,
			plugin_lists_plugin_configure);

	register_plugin("sgml",
			plugin_sgml_plugin_register,
			plugin_sgml_plugin_configure);

	register_plugin("tests",
			plugin_tests_plugin_register,
			plugin_tests_plugin_configure);

	register_plugin("validate",
			plugin_validate_plugin_register,
			plugin_validate_plugin_configure);

	register_plugin("website",
			plugin_website_plugin_register,
			plugin_website_plugin_configure);

	register_plugin("xsl",
			plugin_xsl_plugin_register,
			plugin_xsl_plugin_configure);

	register_plugin("convert-case",
			plugin_convert_case_plugin_register,
			plugin_convert_case_plugin_configure);

	register_plugin("cleanup-source",
			plugin_cleanup_source_plugin_register,
			plugin_cleanup_source_plugin_configure);

	register_plugin("dtd",
			plugin_dtd_plugin_register,
			plugin_dtd_plugin_configure);

	register_plugin("paragraph",
			plugin_paragraph_plugin_register,
			plugin_paragraph_plugin_configure);

	register_plugin("save-dispspec",
			plugin_save_dispspec_plugin_register,
			plugin_save_dispspec_plugin_configure);

	register_plugin("doc-from-xds",
			plugin_doc_from_xds_plugin_register,
			plugin_doc_from_xds_plugin_configure);
}

int main( int   argc,
	  char *argv[] )
{
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

#if 0
	g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL);
#endif

	cong_app_construct_singleton();

	/* Set up the GnomeProgram: */
	cong_app_singleton()->gnome_program = gnome_program_init (PACKAGE_NAME, PACKAGE_VERSION,
								  LIBGNOMEUI_MODULE,
								  argc,argv,
								  GNOME_PARAM_HUMAN_READABLE_NAME,
								  _("XML Editor"),
								  GNOME_PARAM_APP_DATADIR, DATADIR,
								  NULL);

	fonts_load();
	editor_popup_init(NULL); /* FIXME */

	cong_app_singleton()->tooltips = gtk_tooltips_new();

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
	cong_app_singleton()->plugin_manager = cong_plugin_manager_new();
	main_load_plugins();

	insert_element_init();

#if 0
	cong_app_singleton()->clipboard = NULL;
#endif

#if 1
	/* 
	   If we have any arguments, interpret the final one as the name of a file to be opened: 

	   FIXME: should we use popt?
	*/
	if (argc > 1) {
		open_document_do(argv[argc-1], NULL);
	}
#endif


	/* The main loop: */
	gtk_main();

	/* Cleanup: */
	cong_app_destroy_singleton();

	return(0);
}
