#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <ttree.h>
#include <sock.h>
#include <comm.h>
#include <xml.h>
#include "global.h"

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

};

GtkWidget* cong_gui_get_window(struct cong_gui* gui)
{
  g_assert(gui!=NULL);

  return gui->window;
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
  gui->global_tree_store = gtk_tree_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
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

int test_open_do(const char *doc_name, const char *ds_name)
{
	char *p;
#if 1
	TTREE *ds_temp, *xml_in;
	FILE *xml_f;

	ds_temp = ttree_load((char*)ds_name);
	if (!ds_temp) {
	  g_warning("Problem loading dispspec file \"%s\"\n", ds_name);
	  return(TRUE);  /* Invalid displayspec. */
	}

	/* Use libxml to load the doc: */
#if 0
	{
	  #if 1
	  // Use special DocBook loader for DocBook; should handle SGML better...
	  xmlDocPtr doc = docbParseFile(doc_name,NULL);
	  #else
	  xmlDocPtr doc = xmlParseFile(doc_name);
	  #endif
	  xmlDebugDumpDocument(stdout,doc);
	  xml_in = convert_libxml_to_ttree_doc(doc);
	}
#else
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

	the_globals.ds = cong_dispspec_new_from_ttree(ds_temp);

#if 0
	gtk_box_pack_start(GTK_BOX(cong_gui_get_root(&the_gui)), do_ttree_test(xml_in), FALSE, FALSE, 0);
#else
	the_globals.xv = xmlview_new(xml_in, the_globals.ds);
	gtk_box_pack_start(GTK_BOX(cong_gui_get_root(&the_gui)), the_globals.xv->w, FALSE, FALSE, 0);
#endif

#else
	TTREE *ds_temp, *xml_in;
	FILE *xml_f;

	ds_temp = ttree_load(ds_name);
	if (!ds_temp) {
	  g_warning("Problem loading dispspec file \"%s\"\n", ds_name);
	  return(TRUE);  /* Invalid displayspec. */
	}

	xml_f = fopen(doc_name, "rt");
	if (!xml_f) {
	  g_warning("Problem opening doc file \"%s\"\n", doc_name);
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
	
	the_globals.ds = cong_dispspec_new_from_ttree(ds_temp);

	xml_t_trim(xml_in);
	the_globals.xv = xmlview_new(xml_in, the_globals.ds);
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

	ds_name = get_file_name("Select a matching displayspec");
	if (!ds_name) return(TRUE);

	test_open_do(doc_name, ds_name);
#else
	test_open_do("../examples/readme.xml", "../examples/readme.ds");
#endif
	return(TRUE);
}

void test_open_wrap(GtkWidget *widget, gpointer data) { test_open(widget, 0); }


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
	{ "/Tests",                 "NULL", NULL, 0, "<Branch>" },
	{ "/Tests/Open...",         "NULL", test_open_wrap, 0, NULL }
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

	gui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_usize(GTK_WIDGET(gui->window), 360, 240);
	gtk_window_set_default_size(GTK_WINDOW(gui->window), 480, 480);
	gtk_window_set_title(GTK_WINDOW(gui->window), "Conglomerate Editor 0.1.0");

	gtk_signal_connect(GTK_OBJECT(gui->window), "delete_event",
										 GTK_SIGNAL_FUNC(delete_event), NULL);

	gtk_signal_connect(GTK_OBJECT(gui->window), "destroy",
										 GTK_SIGNAL_FUNC(destroy), NULL);

	gtk_container_set_border_width(GTK_CONTAINER(gui->window), 0);

	gtk_widget_realize(gui->window);

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

	/* --- Menus --- */

	gui->accel = gtk_accel_group_new();
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", gui->accel);
	gtk_item_factory_create_items(item_factory, sizeof(menu_items) / sizeof(menu_items[0]),
																menu_items, NULL);
	gtk_window_add_accel_group(GTK_WINDOW(gui->window), gui->accel);

	gui->menus = gtk_item_factory_get_widget(item_factory, "<main>");
	gtk_box_pack_start(GTK_BOX(w2), gui->menus, TRUE, TRUE, 0);
	gtk_widget_show(gui->menus);
	
	/* --- Toolbar --- */
	
	w3 = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(w3), GTK_SHADOW_OUT);
	gtk_box_pack_start(GTK_BOX(w2), w3, FALSE, TRUE, 0);
	gtk_widget_show(w3);

	gui->toolbar = gtk_toolbar_new();
	gtk_container_add(GTK_CONTAINER(w3), gui->toolbar);
	gtk_widget_show(gui->toolbar);

	/* --- Toolbar icons --- */

	gtk_widget_realize(gui->window);
	style = gtk_widget_get_style(gui->window);

  /* Open file */
	
	p = gdk_pixmap_create_from_xpm_d(gui->window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) icon_openfile);
	w3 = gtk_pixmap_new(p, mask);
	gtk_pixmap_set_build_insensitive(GTK_PIXMAP(w3), 1);
	w2 = gtk_toolbar_append_item(GTK_TOOLBAR(gui->toolbar), "Open", "Open document",
															 "Open document", w3, 0, 0);
	gtk_widget_show(w3);
/*
	gtk_widget_set_sensitive(w2, 0);
 */
	gtk_button_set_relief(GTK_BUTTON(w2), GTK_RELIEF_NONE);

	gtk_signal_connect(GTK_OBJECT(w2), "clicked", GTK_SIGNAL_FUNC(open_document), 0);

	gui->butt_find = w2;
	
	/* Submit */

	p = gdk_pixmap_create_from_xpm_d(gui->window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) icon_submit);
	w3 = gtk_pixmap_new(p, mask);
	gtk_pixmap_set_build_insensitive(GTK_PIXMAP(w3), 1);
	gui->butt_submit = gtk_toolbar_append_item(GTK_TOOLBAR(gui->toolbar), "Save", "Save document",
															 "Save document", w3, 0, 0);
	gtk_widget_show(w3);
/*
	gtk_widget_set_sensitive(gui->butt_submit, 0);
 */
	gtk_button_set_relief(GTK_BUTTON(gui->butt_submit), GTK_RELIEF_NONE);

	gtk_signal_connect(GTK_OBJECT(gui->butt_submit), "clicked", GTK_SIGNAL_FUNC(save_document), 0);


	/* --- Logo --- */

	w2 = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(w2), GTK_SHADOW_NONE);
	gtk_box_pack_start(GTK_BOX(w1), w2, FALSE, TRUE, 0);
	gtk_widget_show(w2);

	w1 = gtk_hbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(w2), w1);
	gtk_widget_show(w1);

	p = gdk_pixmap_create_from_xpm_d(gui->window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) ilogo_xpm);
	logo = gtk_pixmap_new(p, mask);
	gtk_box_pack_start(GTK_BOX(w1), logo, FALSE, TRUE, 0);
	gtk_widget_show(logo);

	/* --- Main window -> hpane --- */

	w1 = gtk_hpaned_new();
	gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
	gtk_widget_show(w1);

	/* --- Tree view --- */

	w2 = gtk_scrolled_window_new(NULL, NULL);
	gtk_paned_add1(GTK_PANED(w1), w2);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(w2), GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(GTK_WIDGET(w2), 100, 0);
	gtk_widget_show(w2);

#if 1
        gui->global_tree_store = gtk_tree_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);

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
							   "text", TITLE_COLUMN,
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
	gtk_init(&argc, &argv);

	fonts_load();
	popup_init();
	
	gui_window_main_make();
	gtk_window_set_default_size(GTK_WINDOW(the_gui.window), 400, 460);
	gtk_widget_show(GTK_WIDGET(the_gui.window));

	curs_init(&the_globals.curs);
	selection_init(&the_globals.selection);
	insert_element_init();

	the_globals.clipboard = 0;

	/* --- Putting it together --- */

	the_gui.status_main_ctx = gtk_statusbar_get_context_id(GTK_STATUSBAR(the_gui.status), "Main");
	gtk_statusbar_push(GTK_STATUSBAR(the_gui.status), the_gui.status_main_ctx,
			   "Welcome to the much-too-early Conglomerate editor.");

	/* --- */
	
	if (argc > 2) open_document_do(argv[1], argv[2]);
	
	gtk_main();

	return(0);
}
