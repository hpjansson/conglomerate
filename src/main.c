/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <ttree.h>
#include <sock.h>
#include <comm.h>
#include <xml.h>
#include "global.h"

#include <libxml/tree.h>

#if 1
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>
#endif

#if 1
struct cong_gui
{
  GtkWidget *window, *menus, *toolbar, *status, *tray, *scroller, *root,
    *auth, *butt_submit, *butt_find;

  guint status_main_ctx;

  GtkAccelGroup *accel;

  GtkWidget *popup;

  GtkTreeView *global_tree_view;
  GtkTreeStore *global_tree_store;

	GnomeProgram *gnome_program;
};

GtkWidget* cong_gui_get_window(struct cong_gui* gui)
{
  g_assert(gui!=NULL);

  return GTK_WIDGET(gui->window);
}

GtkWidget* cong_gui_get_popup(struct cong_gui* gui)
{
  g_assert(gui!=NULL);

  return gui->popup;
}

void cong_gui_set_popup(struct cong_gui* gui, GtkWidget* popup)
{
  g_assert(gui!=NULL);

  gui->popup=popup;
}

GtkWidget* cong_gui_get_button_submit(struct cong_gui* gui)
{
  g_assert(gui!=NULL);

  return gui->butt_submit;
}

GtkTreeStore* cong_gui_get_tree_store(struct cong_gui* gui)
{
  g_assert(gui!=NULL);

  return gui->global_tree_store;
}

GtkWidget* cong_gui_get_root(struct cong_gui* gui)
{
  g_assert(gui!=NULL);

  return gui->root;
}

void cong_gui_destroy_tree_store(struct cong_gui* gui)
{
  gui->global_tree_store = gtk_tree_store_new (TREEVIEW_N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
  gtk_tree_view_set_model(gui->global_tree_view, GTK_TREE_MODEL(gui->global_tree_store));
 
#if 0
  gtk_widget_destroy(GTK_WIDGET(gui->global_tree_view));
   
  gui->global_tree_view=NULL;
#endif
}
 
struct cong_globals the_globals;
struct cong_gui the_gui;

#else
GtkWidget *window, *menus, *toolbar, *status, *tray, /* *tree, */ *scroller, *root,
          *auth, *popup = 0, *butt_new, *butt_submit, *butt_find;

GtkTreeView *global_tree_view;
GtkTreeStore *global_tree_store;

GdkGC *insert_element_gc;

guint status_main_ctx;
#endif

void get_example(GtkWidget *w, gpointer data);
gint set_vectors(GtkWidget *w, gpointer data);

#if 0
GdkFont *f, *fm, *ft;
int f_asc, f_desc, fm_asc, fm_desc, ft_asc, ft_desc;

TTREE *ds_global = 0;
TTREE *vect_global = 0;
TTREE *medias_global = 0;
TTREE *class_global = 0;
TTREE *clipboard = 0;


struct xed *xed;
struct xview *xv = 0;
struct curs curs;
#endif


void xed_cut_wrap(GtkWidget *widget, gpointer data) { xed_cut(widget, 0); }
void xed_copy_wrap(GtkWidget *widget, gpointer data) { xed_copy(widget, 0); }
void xed_paste_wrap(GtkWidget *widget, gpointer data) { xed_paste(widget, 0); }

void open_document_wrap(GtkWidget *widget, gpointer data) { open_document(widget, 0); }
void save_document_wrap(GtkWidget *widget, gpointer data) { save_document(widget, 0); }



void add_node_recursive(TTREE *tt, GtkTreeStore *store, GtkTreeIter *parent_iter)
{
  GtkTreeIter child_iter;  /* Child iter  */

  gtk_tree_store_append (store, &child_iter, parent_iter);  /* Acquire a child iterator */

  gtk_tree_store_set (store, &child_iter,
		      0, tt->data,
		      -1); 
  
  /* Recurse over children: */
  {
    TTREE *child_tt = tt->child;
    while (child_tt != NULL) {
      add_node_recursive(child_tt, store, &child_iter);
      child_tt=child_tt->next;
    }
  }
 
}

GtkWidget* do_ttree_test(TTREE* tt)
{
  GtkTreeStore *store = gtk_tree_store_new (1, G_TYPE_STRING);
  GtkWidget *tree;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  g_return_val_if_fail(tt, NULL);
 
  add_node_recursive(tt,store,NULL);

  tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Author",
						     renderer,
						     "text", 0,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  gtk_widget_show(tree);

  return tree;
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

int test_open_do(const char *doc_name, const char *ds_name)
{
	char *p;
	TTREE *xml_in;
	FILE *xml_f;
	CongDispspec *ds;

#ifndef AUTOGENERATE_DS
	GnomeVFSURI *uri = gnome_vfs_uri_new(ds_name);
	GnomeVFSResult vfs_result = cong_dispspec_new_from_xds_file(uri, &ds);
	if (vfs_result!=GNOME_VFS_OK) {
		GtkDialog* dialog = cong_error_dialog_new_file_open_failed_from_vfs_result(uri, vfs_result);
			
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));

		gnome_vfs_uri_unref(uri);
		
		return(TRUE);  /* Invalid displayspec. */
	}

	gnome_vfs_uri_unref(uri);

#endif

#if 1
	/* Use libxml to load the doc: */
	{
		xmlDocPtr doc=NULL;
#if 1
		/* Load using GnomeVFS: */
		{
			char* buffer;
			GnomeVFSFileSize size;
			GnomeVFSURI* file_uri = gnome_vfs_uri_new(doc_name);
			GnomeVFSResult vfs_result = cong_vfs_new_buffer_from_file(doc_name, &buffer, &size);

			if (vfs_result!=GNOME_VFS_OK) {
				GtkDialog* dialog = cong_error_dialog_new_file_open_failed_from_vfs_result(file_uri, vfs_result);
			
				cong_error_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(GTK_WIDGET(dialog));

				gnome_vfs_uri_unref(file_uri);

				return TRUE;
			}

			g_assert(buffer);

			/* Parse the file from the buffer: */
#if 0
			// Can't use DocBook loader as it only supports loading from a file, not from memory
#else
			doc = xmlParseMemory(buffer, size);
#endif

			gnome_vfs_uri_unref(file_uri);

			g_free(buffer);
		}
#else
		/* Load using standard filesystem: */
		{
#if 0
			// Use special DocBook loader for DocBook; should handle SGML better...
			doc = docbParseFile(doc_name,NULL);
#else
			doc = xmlParseFile(doc_name);
#endif
		}
#endif

#ifdef AUTOGENERATE_DS
		/* Autogenerate the ds: */
		ds = cong_dispspec_new_from_xml_file(doc);
#endif

		xml_in = convert_libxml_to_ttree_doc(doc);
	}
#else
	/* Use the flux loaders to load the doc: */
	xml_f = fopen(doc_name, "rt");
	if (!xml_f) {
	  g_warning("Problem opening doc file \"%s\"\n", doc_name);
	  return(TRUE);
	}

	xml_in = xml_f_to_ttree(xml_f, 0);
	if (!xml_in) {
	  g_warning("Problem parsing doc file \"%s\"\n", doc_name);
	  return(TRUE);  /* Invalid XML document. */
	}

	fclose(xml_f);

	xml_t_trim(xml_in);
#endif


#if 0
	gtk_box_pack_start(GTK_BOX(cong_gui_get_root(&the_gui)), do_ttree_test(xml_in), FALSE, FALSE, 0);
#else
	the_globals.xv = xmlview_new(cong_document_new_from_ttree(xml_in, ds));
	gtk_box_pack_start(GTK_BOX(cong_gui_get_root(&the_gui)), the_globals.xv->w, FALSE, FALSE, 0);
#endif

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


static GtkItemFactoryEntry menu_items[] =
{
	{ "/_Document",             NULL, NULL, 0, "<Branch>" },
	{ "/Document/Open...",      NULL, open_document_wrap, 0, NULL },
	{ "/Document/Save As...",   NULL, save_document_wrap, 0, NULL },
	{ "/Document/Quit",         NULL, gtk_main_quit, 0, NULL },
	{ "/_Edit",                 NULL, 0, 0, "<Branch>" },
	{ "/Edit/Cut",              "<control>X", xed_cut_wrap, 0, 0 },
	{ "/Edit/Copy",             "<control>C", xed_copy_wrap, 0, 0 },
	{ "/Edit/Paste",            "<control>V", xed_paste_wrap, 0, 0 },
	{ "/Tests",                 NULL, NULL, 0, "<Branch>" },
	{ "/Tests/Open...",         NULL, test_open_wrap, 0, NULL },
	{ "/Tests/Error",           NULL, test_error_wrap, 0, NULL },
	{ "/Tests/Document Types",  NULL, test_document_types_wrap, 0, NULL }
};




void insert_element_init()
{
	GdkColor gcol;

	the_globals.insert_element_gc = gdk_gc_new(the_gui.window->window);
	gdk_gc_copy(the_globals.insert_element_gc, the_gui.window->style->white_gc);
	col_to_gcol(&gcol, 0x00e0e0e0);
	gdk_colormap_alloc_color(the_gui.window->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(the_globals.insert_element_gc, &gcol);
}



gint delete_event( GtkWidget *widget,
									GdkEvent  *event,
									gpointer   data )
{
#ifndef RELEASE
	g_print ("delete event occurred\n");
#endif	
	return(FALSE);
}


void destroy( GtkWidget *widget,
						 gpointer   data )
{
	gtk_main_quit();
}


char font_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "abcdefghijklmnopqrstuvwxyz"
                    "0123456789!@\"'/()[]{}*&#~";

void fonts_load()
{
#ifdef WINDOWS_BUILD                                                            
	  the_globals.f = gdk_font_load("-*-arial-*-r-normal-*-14-*-*-*-*-*-iso8859-1");            
	  the_globals.ft = gdk_font_load("-*-MS Sans Serif-bold-r-normal-*-12-*-*-*-*-*-iso8859-1");
	  the_globals.fm = gdk_font_load("-*-arial-*-*-normal-*-12-*-*-*-c-*-iso8859-1");           
#else                                                                           
	  the_globals.f = gdk_font_load("-*-helvetica-*-r-normal-*-10-*-*-*-*-*-iso8859-1");        
	  the_globals.ft = gdk_font_load("-*-helvetica-*-r-normal-*-12-*-*-*-*-*-iso8859-1");       
	  the_globals.fm = gdk_font_load("-*-clean-*-*-normal-*-6-*-*-*-c-*-iso8859-1");            
#endif                                                                          
	                                                                                
	  gdk_string_extents(the_globals.f, font_chars, 0, 0, 0, &the_globals.f_asc, &the_globals.f_desc);                  
	  gdk_string_extents(the_globals.fm, font_chars, 0, 0, 0, &the_globals.fm_asc, &the_globals.fm_desc);               
	  gdk_string_extents(the_globals.ft, font_chars, 0, 0, 0, &the_globals.ft_asc, &the_globals.ft_desc);               
	                                                                                
#ifdef WINDOWS_BUILD                                                            
	  the_globals.f_asc -= 2;                                                                   
	  the_globals.ft_asc -= 4;                                                                  
	  the_globals.fm_asc -= 8;                                                                  
#endif     
}


/* Main window layout:
 * 
 * .----------------------------------------.
 * | Menus                           |      |
 * |---------------------------------| LOGO |
 * | Toolbar                         |      |
 * |---------------------------------`------|
 * |       |                                |
 * | Tree  | Document view                  |
 * | View  |                                |
 * |       |                                |
 * |       |                                |
 * |       |                                |
 * |       |                                |
 * |       |                                |
 * |       |                                |
 * |       |                                |
 * |       |                                |
 * |       |                                |
 * |       O <- split pane                  |
 * |       |                                |
 * |----------------------------------------|
 * | Statusbar                       | Tray |
 * `----------------------------------------'
 * 
 */


extern char *ilogo_xpm[];
extern char *auth_off_xpm[];
extern char *icon_connect[];
extern char *icon_new[];
extern char *icon_assemble[];
extern char *icon_openfile[];
extern char *icon_submit[];

GtkPixmap* gui_create_pixmap(struct cong_gui* gui, char** xpm)
{
	GdkPixmap *p;
	GdkBitmap *mask;
	GtkStyle *style;

	gtk_widget_realize(GTK_WIDGET(gui->window));

	style = gtk_widget_get_style(GTK_WIDGET(gui->window));

	p = gdk_pixmap_create_from_xpm_d(gui->window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm);
	return GTK_PIXMAP(gtk_pixmap_new(p, mask));
}

GtkWidget* gui_toolbar_create_item(struct cong_gui* gui,
				   GtkToolbar* toolbar,
				   const char* text, 
				   const char* tooltip_text, 
				   const char* tooltip_private_text, 
				   char** xpm,
				   gint (*handler)(GtkWidget *w, gpointer data))
{
	GtkWidget* widget;
	GtkWidget* icon;

	icon = GTK_WIDGET(gui_create_pixmap(gui, xpm));

	gtk_pixmap_set_build_insensitive(GTK_PIXMAP(icon), 1);

	widget = gtk_toolbar_append_item(toolbar, 
					 text, 
					 tooltip_text, 
					 tooltip_private_text, 
					 icon, 0, 0);

	gtk_widget_show(icon);

/*
	gtk_widget_set_sensitive(icon, 0);
 */

	gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);

	gtk_signal_connect(GTK_OBJECT(widget), "clicked", GTK_SIGNAL_FUNC(handler), 0);

	return widget;
}

void gui_toolbar_populate(struct cong_gui* gui)
{
	/* Open */
	gui->butt_find = gui_toolbar_create_item(gui,
						 GTK_TOOLBAR(gui->toolbar),
						 "Open", 
						 "Open document",
						 "Open document",
						 icon_openfile, 
						 open_document);
	
	/* Submit */
	gui->butt_submit = gui_toolbar_create_item(gui,
						   GTK_TOOLBAR(gui->toolbar),
						   "Save", 
						   "Save document",
						   "Save document", 
						   icon_submit,
						   save_document);
}

#define USE_GNOME_APP

void gui_window_main_make()
{
	GtkWidget *w0, *w1, *w2, *w3, *logo;
	GdkPixmap *p;
	GdkBitmap *mask;
	GdkColor gcol;
	GtkStyle *style;
	GtkItemFactory *item_factory;
	UNUSED_VAR(TTREE *x_in)
	UNUSED_VAR(TTREE *displayspec)
	int i;

 	GtkTreeViewColumn *column;
 	GtkCellRenderer *renderer;
 
 	struct cong_gui* gui = &the_gui;
 
	gdk_rgb_init();

	/* --- Main window --- */
#ifdef USE_GNOME_APP
	gui->window = gnome_app_new("Conglomerate","Conglomerate Editor");
#else
	gui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_usize(GTK_WIDGET(gui->window), 360, 240);
	gtk_window_set_default_size(GTK_WINDOW(gui->window), 480, 480);
	gtk_window_set_title(GTK_WINDOW(gui->window), "Conglomerate Editor 0.1.0");
#endif

	gtk_signal_connect(GTK_OBJECT(gui->window), "delete_event",
										 GTK_SIGNAL_FUNC(delete_event), NULL);

	gtk_signal_connect(GTK_OBJECT(gui->window), "destroy",
										 GTK_SIGNAL_FUNC(destroy), NULL);

	gtk_container_set_border_width(GTK_CONTAINER(gui->window), 0);

	gtk_widget_realize(GTK_WIDGET(gui->window));

#ifndef USE_GNOME_APP
	/* --- Main window -> vbox --- */

	w0 = gtk_vbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(gui->window), w0);
	gtk_widget_show(w0);

	/* --- Main window -> vbox -> hbox --- */

	w1 = gtk_hbox_new(FALSE, 1);
	gtk_box_pack_start(GTK_BOX(w0), w1, FALSE, TRUE, 0);
	gtk_widget_show(w1);

	/* --- Main window -> vbox -> hbox -> vbox (menu & toolbar) --- */

	w2 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(w1), w2, TRUE, TRUE, 0);
	gtk_widget_show(w2);
#endif

	/* --- Menus --- */

	gui->accel = gtk_accel_group_new();
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", gui->accel);
	gtk_item_factory_create_items(item_factory, sizeof(menu_items) / sizeof(menu_items[0]),
																menu_items, NULL);
	gtk_window_add_accel_group(GTK_WINDOW(gui->window), gui->accel);

	gui->menus = gtk_item_factory_get_widget(item_factory, "<main>");
#ifdef USE_GNOME_APP
	gnome_app_set_menus(GNOME_APP(gui->window), GTK_MENU_BAR(gui->menus));
#else
	gtk_box_pack_start(GTK_BOX(w2), gui->menus, TRUE, TRUE, 0);
#endif
	gtk_widget_show(gui->menus);
	
	/* --- Toolbar --- */
#ifndef USE_GNOME_APP
	w3 = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(w3), GTK_SHADOW_OUT);
	gtk_box_pack_start(GTK_BOX(w2), w3, FALSE, TRUE, 0);
	gtk_widget_show(w3);
#endif

	gui->toolbar = gtk_toolbar_new();
#ifdef USE_GNOME_APP
	gnome_app_set_toolbar(GNOME_APP(gui->window), GTK_TOOLBAR(gui->toolbar));
#else
	gtk_container_add(GTK_CONTAINER(w3), gui->toolbar);
#endif
	gtk_widget_show(gui->toolbar);

	/* --- Toolbar icons --- */

	gui_toolbar_populate(gui);

	/* --- Logo --- */
#ifdef USE_GNOME_APP
	/* can't reimplement this way */
#else
	w2 = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(w2), GTK_SHADOW_NONE);
	gtk_box_pack_start(GTK_BOX(w1), w2, FALSE, TRUE, 0);
	gtk_widget_show(w2);

	w1 = gtk_hbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(w2), w1);
	gtk_widget_show(w1);

	logo = GTK_WIDGET(gui_create_pixmap(gui, ilogo_xpm));
	gtk_box_pack_start(GTK_BOX(w1), logo, FALSE, TRUE, 0);
	gtk_widget_show(logo);
#endif
	/* --- Main window -> hpane --- */

	w1 = gtk_hpaned_new();
#ifdef USE_GNOME_APP
	gnome_app_set_contents(GNOME_APP(gui->window),w1);
#else
	gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
#endif
	gtk_widget_show(w1);

	/* --- Tree view --- */

	w2 = gtk_scrolled_window_new(NULL, NULL);
	gtk_paned_add1(GTK_PANED(w1), w2);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(w2), GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(GTK_WIDGET(w2), 100, 0);
	gtk_widget_show(w2);

#if 1
        gui->global_tree_store = gtk_tree_store_new (TREEVIEW_N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_POINTER);

	gui->global_tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model (GTK_TREE_MODEL(gui->global_tree_store)));

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(w2), GTK_WIDGET(gui->global_tree_view));
#if 0
	column = gtk_tree_view_column_new ();
	cell = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, cell, FALSE);
	gtk_tree_view_column_set_attributes (column, cell,
					     "pixbuf", GCONF_TREE_MODEL_CLOSED_ICON_COLUMN,
					     "pixbuf_expander_closed", GCONF_TREE_MODEL_CLOSED_ICON_COLUMN,
					     "pixbuf_expander_open", GCONF_TREE_MODEL_OPEN_ICON_COLUMN,
					     NULL);
	cell = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, cell, TRUE);
	gtk_tree_view_column_set_attributes (column, cell,
					     "text", GCONF_TREE_MODEL_NAME_COLUMN,
					     NULL);
#endif

#if 1
	renderer = gtk_cell_renderer_text_new ();

	/* Create a column, associating the "text" attribute of the
	 * cell_renderer to the first column of the model */
	column = gtk_tree_view_column_new_with_attributes ("Element", renderer,
							   "text", TREEVIEW_TITLE_COLUMN,
							   NULL);

#endif

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (gui->global_tree_view), column);

 	/* Wire up the context-menu callback */
 	gtk_signal_connect_object(GTK_OBJECT(gui->global_tree_view), "event",
 				  (GtkSignalFunc) tpopup_show, gui->global_tree_view);

	gtk_widget_show(GTK_WIDGET(gui->global_tree_view));
#else
	tree = gtk_tree_new();
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(w2), tree);
	gtk_widget_show(tree);
#endif

#if 0
	w2 = gtk_tree_item_new_with_label("Document");
	gtk_tree_append(GTK_TREE(tree), w2);
	gtk_widget_show(w2);
#endif

#if 0	
	/* --- TEMPORARY: Custom style --- */

	style = gtk_widget_get_default_style();
	style = gtk_style_copy(style);
	for (i = 0; i < 5; i++)
	{
		gcol.red = 0xffff;
		gcol.blue = 0xffff;
		gcol.green = 0x0000;
		style->bg[i] = gcol;
	}
#endif

	/* --- Scrolling area --- */

	gui->scroller = gtk_scrolled_window_new(NULL, NULL);
	gtk_paned_add2(GTK_PANED(w1), gui->scroller);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(gui->scroller), GTK_POLICY_AUTOMATIC,
																 GTK_POLICY_ALWAYS);

	gui->root = gtk_vbox_new(FALSE, 1);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(gui->scroller), gui->root);
	
	
	/* TEMPORARY: Set white background */
	
	gcol.blue = 0xffff;
	gcol.green = 0xffff;
	gcol.red = 0xffff;
	
	style = gtk_widget_get_default_style();
	style = gtk_style_copy(style);

	gdk_colormap_alloc_color(gui->window->style->colormap, &gcol, 0, 1);

	for (i = 0; i < 5; i++) style->bg[i] = gcol;
	xv_style_r(gui->scroller, style);

	gtk_widget_show(gui->scroller);
	gtk_widget_show(gui->root);


	/* --- Main window -> vbox -> hbox --- */
#ifdef USE_GNOME_APP
	gui->status = gtk_statusbar_new();
	gnome_app_set_statusbar(GNOME_APP(gui->window), gui->status);
#else
	w1 = gtk_hbox_new(FALSE, 1);
	gtk_box_pack_start(GTK_BOX(w0), w1, FALSE, TRUE, 0);
	gtk_widget_show(w1);

	/* --- Statusbar --- */

	gui->status = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(w1), gui->status, TRUE, TRUE, 0);
	gtk_widget_show(gui->status);

	/* --- Statustray --- */

	w0 = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(w0), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(w1), w0, FALSE, TRUE, 0);
	gtk_widget_show(w0);

	gui->tray = gtk_hbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(w0), gui->tray);
	gtk_widget_show(gui->tray);

	/* --- Indicator: Auth --- */

	p = gdk_pixmap_create_from_xpm_d(gui->window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) auth_off_xpm);
	gui->auth = gtk_pixmap_new(p, mask);
	gtk_box_pack_start(GTK_BOX(gui->tray), gui->auth, FALSE, TRUE, 2);
	gtk_widget_show(gui->auth);
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
	the_gui.gnome_program = gnome_program_init("Conglomerate",
						   "0.1.0",
						   LIBGNOMEUI_MODULE,
						   argc,argv,
						   GNOME_PARAM_NONE);
#else
	gtk_init(&argc, &argv);
#endif


	fonts_load();
	popup_init();
	
	gui_window_main_make();
	gtk_window_set_default_size(GTK_WINDOW(the_gui.window), 400, 460);
	gtk_widget_show(GTK_WIDGET(the_gui.window));


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

	curs_init(&the_globals.curs);
	selection_init(&the_globals.selection);
	insert_element_init();

	the_globals.clipboard = 0;

	/* --- Putting it together --- */

	the_gui.status_main_ctx = gtk_statusbar_get_context_id(GTK_STATUSBAR(the_gui.status), "Main");
	gtk_statusbar_push(GTK_STATUSBAR(the_gui.status), the_gui.status_main_ctx,
			   "Welcome to the much-too-early Conglomerate editor.");

	/* --- */
#if 0	
	if (argc > 2) open_document_do(argv[1], argv[2]);
#endif
	
	gtk_main();

	return(0);
}
