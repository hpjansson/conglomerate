/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "global.h"

#include <libxml/tree.h>

#if 0
#include <libxslt/xsltInternals.h>
#endif



void get_example(GtkWidget *w, gpointer data);
gint set_vectors(GtkWidget *w, gpointer data);

void xed_cut_wrap(GtkWidget *widget, gpointer data) { xed_cut(widget, 0); }
void xed_copy_wrap(GtkWidget *widget, gpointer data) { xed_copy(widget, 0); }
void xed_paste_wrap(GtkWidget *widget, gpointer data) { xed_paste(widget, 0); }

void open_document_wrap(GtkWidget *widget, gpointer data) { open_document(widget, 0); }
void save_document_wrap(GtkWidget *widget, gpointer data) { save_document(widget, 0); }



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

int test_open_do(const char *doc_name, const char *ds_name)
{
	return (TRUE);
}


gint test_open(GtkWidget *w, gpointer data)
{
#if 1
	const char *doc_name, *ds_name;
	
	doc_name = get_file_name("Select an XML document");
	if (!doc_name) return(TRUE);

	ds_name = get_file_name("Select a matching XDS displayspec");
	if (!ds_name) return(TRUE);

	test_open_do(doc_name, ds_name);
#else
	#if 0
	test_open_do("../examples/hacked_gconf.sgml", "../examples/docbook.xds");
	#else
	/* test_open_do("../examples/readme.xml", "../examples/readme.xds"); */
	test_open_do("../examples/test-docbook.xml", "../examples/docbook.xds");
	#endif
#endif
	return(TRUE);
}

void test_open_wrap(GtkWidget *widget, gpointer data) { test_open(widget, 0); }

gint test_error(GtkWidget *w, gpointer data)
{
	cong_error_tests();

	return TRUE;
}

void test_error_wrap(GtkWidget *widget, gpointer data) { test_error(widget, 0); }

enum
{
	DOCTYPELIST_NAME_COLUMN,
	DOCTYPELIST_DESCRIPTION_COLUMN,
	DOCTYPELIST_N_COLUMNS
};

gint test_document_types(GtkWidget *w, gpointer data)
{
	GtkWidget* dialog;
	GtkWidget* list_view;

	GtkListStore *store;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	dialog = gtk_dialog_new();

	gtk_window_set_title(GTK_WINDOW(dialog), "Document Types");

	store = gtk_list_store_new (DOCTYPELIST_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);

	list_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

	/* The view now holds a reference.  We can get rid of our own
	 * reference */
	g_object_unref (G_OBJECT (store));

	/* Populate the store based on the ds-registry: */
	{
		CongDispspecRegistry* registry = the_globals.ds_registry;
		int i;

		for (i=0;i<cong_dispspec_registry_get_num(registry);i++) {
			const CongDispspec* ds = cong_dispspec_registry_get(registry,i);
			
			GtkTreeIter iter;
			gtk_list_store_append (store, &iter);  /* Acquire an iterator */
			
			gtk_list_store_set (store, &iter,
					    DOCTYPELIST_NAME_COLUMN, cong_dispspec_get_name(ds),
					    DOCTYPELIST_DESCRIPTION_COLUMN, cong_dispspec_get_description(ds),
					    -1);
		}
	}

	renderer = gtk_cell_renderer_text_new ();

	column = gtk_tree_view_column_new_with_attributes ("Name", renderer,
							   "text", DOCTYPELIST_NAME_COLUMN,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (list_view), column);

	column = gtk_tree_view_column_new_with_attributes ("Description", renderer,
							   "text", DOCTYPELIST_DESCRIPTION_COLUMN,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (list_view), column);

	gtk_widget_show (GTK_WIDGET(list_view));

	gtk_container_add (GTK_CONTAINER( GTK_DIALOG (dialog)->vbox ), 
			   list_view);

	gtk_dialog_add_button(GTK_DIALOG(dialog),
			      "gtk-ok",
			      GTK_RESPONSE_OK);

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));

	return TRUE;
}

void test_document_types_wrap(GtkWidget *widget, gpointer data) { test_document_types(widget, 0); }

gint test_transform(GtkWidget *w, gpointer data)
{
#if 0
	/* Hackish test of libxslt */
	xmlDocPtr doc;
	xsltStylesheetPtr xsl;
	xmlDocPtr result;

#if 0
	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue(1);
#endif
	
	doc = xmlParseFile("../examples/test-docbook.xml");
	g_assert(doc);

	xsl = xsltParseStylesheetFile("../examples/test-docbook-to-html.xsl");
	g_assert(xsl);

	result = xsltApplyStylesheet(xsl, doc, NULL);
	g_assert(result);

	xsltSaveResultToFile(stdout, result, xsl);
	
	xsltFreeStylesheet(xsl);
	xmlFreeDoc(result);
	xmlFreeDoc(doc);

	/* do we need to clean up the globals? */
#if 0
	xmlSubstituteEntitiesDefault(0);
	xmlLoadExtDtdDefaultValue(0);
#endif

#endif
	return TRUE;
}

void test_transform_wrap(GtkWidget *widget, gpointer data) { test_transform(widget, 0); }






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
#ifdef WINDOWS_BUILD                                                            
	  the_globals.fonts[CONG_FONT_ROLE_BODY_TEXT] = cong_font_load("-*-arial-*-r-normal-*-14-*-*-*-*-*-iso8859-1");            
	  the_globals.fonts[CONG_FONT_ROLE_TITLE_TAG] = cong_font_load("-*-MS Sans Serif-bold-r-normal-*-12-*-*-*-*-*-iso8859-1");
	  the_globals.fonts[CONG_FONT_ROLE_SPAN_TAG] = cong_font_load("-*-arial-*-*-normal-*-12-*-*-*-c-*-iso8859-1");           
#else                                                                           
	  the_globals.fonts[CONG_FONT_ROLE_BODY_TEXT] = cong_font_load("-*-helvetica-*-r-normal-*-10-*-*-*-*-*-iso8859-1");        
	  the_globals.fonts[CONG_FONT_ROLE_TITLE_TEXT] = cong_font_load("-*-helvetica-*-r-normal-*-12-*-*-*-*-*-iso8859-1");       
	  the_globals.fonts[CONG_FONT_ROLE_SPAN_TAG] = cong_font_load("-*-clean-*-*-normal-*-6-*-*-*-c-*-iso8859-1");            
#endif                                                                         

#ifdef WINDOWS_BUILD                                                            
	  the_globals.fonts[CONG_FONT_ROLE_BODY_TEXT]->asc -= 2;                                                                   
	  the_globals.fonts[CONG_FONT_ROLE_TITLE_TEXT]->asc -= 4;                                                                  
	  the_globals.fonts[CONG_FONT_ROLE_SPAN_TAG]asc -= 8;                                                                  
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


int main( int   argc,
	  char *argv[] )
{
#if 1
	the_globals.gnome_program = gnome_program_init("Conglomerate",
						       "0.1.0",
						       LIBGNOMEUI_MODULE,
						       argc,argv,
						       GNOME_PARAM_NONE);
#else
	gtk_init(&argc, &argv);
#endif


	fonts_load();
	popup_init(NULL); /* FIXME */

#if USE_PANGO
	the_globals.pango_context = pango_context_new();
	the_globals.pango_font_description = pango_font_description_new();

	pango_font_description_set_family(the_globals.pango_font_description,
					  "helvetica");
	pango_font_description_set_size(the_globals.pango_font_description,
			       PANGO_SCALE*12);

	pango_context_set_font_description(the_globals.pango_context,
					   the_globals.pango_font_description);


	the_globals.pango_font = pango_context_load_font(the_globals.pango_context,
							 the_globals.pango_font_description);

	g_assert(the_globals.pango_font);

#endif /* #if USE_PANGO */
	
	cong_primary_window_new(NULL);


	{
#if 0
		gchar*      xds_directory = gnome_program_locate_file(the_gui.gnome_program,
								      GNOME_FILE_DOMAIN_APP_DATADIR,
								      "dispspec",
								      FALSE,
								      NULL);
#else
		gchar* current_dir = g_get_current_dir();
		gchar* xds_directory = g_strdup_printf("%s/../examples",current_dir);
		g_free(current_dir);
#endif

		if (xds_directory) {
			g_message("Loading xds files from \"%s\"\n", xds_directory);
			the_globals.ds_registry = cong_dispspec_registry_new(xds_directory);

			g_free(xds_directory);

			if (the_globals.ds_registry==NULL) {
				return(1);
			}

			cong_dispspec_registry_dump(the_globals.ds_registry);
		} else {
			GtkDialog* dialog = cong_error_dialog_new("Conglomerate could not find its registry of document types.",
 								  "You must run the program from the location in which you built it.",
 								  "This is a known problem and will be fixed.");
			cong_error_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(GTK_WIDGET(dialog));
			return(1);
		}
	}

	insert_element_init();

	the_globals.clipboard = NULL;

	/* --- */
#if 0	
	if (argc > 2) open_document_do(argv[1], argv[2]);
#endif
	
	gtk_main();

	return(0);
}
