#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <ttree.h>
#include <sock.h>
#include <comm.h>
#include <xml.h>
#include "global.h"

GtkWidget *window, *menus, *toolbar, *status, *tray, *tree, *scroller, *root,
          *auth, *popup = 0, *butt_new, *butt_submit, *butt_find;

GdkGC *insert_element_gc;

guint status_main_ctx;

void get_example(GtkWidget *w, gpointer data);
gint set_vectors(GtkWidget *w, gpointer data);

GdkFont *f, *fm, *ft;
int f_asc, f_desc, fm_asc, fm_desc, ft_asc, ft_desc;

TTREE *ds_global = 0;
TTREE *vect_global = 0;
TTREE *medias_global = 0;
TTREE *class_global = 0;
TTREE *clipboard = 0;

GtkAccelGroup *accel;

struct xed *xed;
struct xview *xv = 0;
struct curs curs;


void xed_cut_wrap(GtkWidget *widget, gpointer data) { xed_cut(widget, 0); }
void xed_copy_wrap(GtkWidget *widget, gpointer data) { xed_copy(widget, 0); }
void xed_paste_wrap(GtkWidget *widget, gpointer data) { xed_paste(widget, 0); }

void open_document_wrap(GtkWidget *widget, gpointer data) { open_document(widget, 0); }
void save_document_wrap(GtkWidget *widget, gpointer data) { save_document(widget, 0); }


static GtkItemFactoryEntry menu_items[] =
{
	{ "/_Document",             NULL, NULL, 0, "<Branch>" },
	{ "/Document/Open...",      NULL, open_document_wrap, 0, NULL },
	{ "/Document/Save As...",   NULL, save_document_wrap, 0, NULL },
	{ "/Document/Quit",         NULL, gtk_main_quit, 0, NULL },
	{ "/_Edit",                 NULL, 0, 0, "<Branch>" },
	{ "/Edit/Cut",              "<control>X", xed_cut_wrap, 0, 0 },
	{ "/Edit/Copy",             "<control>C", xed_copy_wrap, 0, 0 },
	{ "/Edit/Paste",            "<control>V", xed_paste_wrap, 0, 0 }
};




void insert_element_init()
{
	GdkColor gcol;

	insert_element_gc = gdk_gc_new(window->window);
	gdk_gc_copy(insert_element_gc, window->style->white_gc);
	col_to_gcol(&gcol, 0x00e0e0e0);
	gdk_colormap_alloc_color(window->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(insert_element_gc, &gcol);
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
	  f = gdk_font_load("-*-arial-*-r-normal-*-14-*-*-*-*-*-iso8859-1");            
	  ft = gdk_font_load("-*-MS Sans Serif-bold-r-normal-*-12-*-*-*-*-*-iso8859-1");
		  fm = gdk_font_load("-*-arial-*-*-normal-*-12-*-*-*-c-*-iso8859-1");           
#else                                                                           
	  f = gdk_font_load("-*-helvetica-*-r-normal-*-10-*-*-*-*-*-iso8859-1");        
	  ft = gdk_font_load("-*-helvetica-*-r-normal-*-12-*-*-*-*-*-iso8859-1");       
	  fm = gdk_font_load("-*-clean-*-*-normal-*-6-*-*-*-c-*-iso8859-1");            
#endif                                                                          
	                                                                                
	  gdk_string_extents(f, font_chars, 0, 0, 0, &f_asc, &f_desc);                  
	  gdk_string_extents(fm, font_chars, 0, 0, 0, &fm_asc, &fm_desc);               
	  gdk_string_extents(ft, font_chars, 0, 0, 0, &ft_asc, &ft_desc);               
	                                                                                
#ifdef WINDOWS_BUILD                                                            
	  f_asc -= 2;                                                                   
	  ft_asc -= 4;                                                                  
	  fm_asc -= 8;                                                                  
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
	TTREE *x_in, *displayspec;
	int i;

	gdk_rgb_init();

	/* --- Main window --- */

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_usize(GTK_WIDGET(window), 360, 240);
	gtk_window_set_default_size(GTK_WINDOW(window), 480, 480);
	gtk_window_set_title(GTK_WINDOW(window), "Conglomerate Editor 0.1.0");

	gtk_signal_connect(GTK_OBJECT(window), "delete_event",
										 GTK_SIGNAL_FUNC(delete_event), NULL);

	gtk_signal_connect(GTK_OBJECT(window), "destroy",
										 GTK_SIGNAL_FUNC(destroy), NULL);

	gtk_container_set_border_width(GTK_CONTAINER(window), 0);

	gtk_widget_realize(window);

	/* --- Main window -> vbox --- */

	w0 = gtk_vbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(window), w0);
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

	accel = gtk_accel_group_new();
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel);
	gtk_item_factory_create_items(item_factory, sizeof(menu_items) / sizeof(menu_items[0]),
																menu_items, NULL);
	gtk_accel_group_attach(accel, GTK_OBJECT(window));

	menus = gtk_item_factory_get_widget(item_factory, "<main>");
	gtk_box_pack_start(GTK_BOX(w2), menus, TRUE, TRUE, 0);
	gtk_widget_show(menus);
	
	/* --- Toolbar --- */
	
	w3 = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(w3), GTK_SHADOW_OUT);
	gtk_box_pack_start(GTK_BOX(w2), w3, FALSE, TRUE, 0);
	gtk_widget_show(w3);
	
	toolbar = gtk_toolbar_new(GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_space_size(GTK_TOOLBAR(toolbar), 16);
	gtk_toolbar_set_space_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_SPACE_LINE);
	gtk_container_add(GTK_CONTAINER(w3), toolbar);
	gtk_widget_show(toolbar);

	/* --- Toolbar icons --- */

	gtk_widget_realize(window);
	style = gtk_widget_get_style(window);

  /* Open file */
	
	p = gdk_pixmap_create_from_xpm_d(window->window, &mask,
																	 &style->bg[GTK_STATE_NORMAL],
																	 (gchar **) icon_openfile);
	w3 = gtk_pixmap_new(p, mask);
	gtk_pixmap_set_build_insensitive(GTK_PIXMAP(w3), 1);
	w2 = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), "Open", "Open document",
															 "Open document", w3, 0, 0);
	gtk_widget_show(w3);
/*
	gtk_widget_set_sensitive(w2, 0);
 */
	gtk_button_set_relief(GTK_BUTTON(w2), GTK_RELIEF_NONE);

	gtk_signal_connect(GTK_OBJECT(w2), "clicked", GTK_SIGNAL_FUNC(open_document), 0);

  butt_find = w2;
	
	/* Submit */

	p = gdk_pixmap_create_from_xpm_d(window->window, &mask,
																	 &style->bg[GTK_STATE_NORMAL],
																	 (gchar **) icon_submit);
	w3 = gtk_pixmap_new(p, mask);
	gtk_pixmap_set_build_insensitive(GTK_PIXMAP(w3), 1);
	butt_submit = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), "Save", "Save document",
															 "Save document", w3, 0, 0);
	gtk_widget_show(w3);
/*
	gtk_widget_set_sensitive(butt_submit, 0);
 */
	gtk_button_set_relief(GTK_BUTTON(butt_submit), GTK_RELIEF_NONE);

	gtk_signal_connect(GTK_OBJECT(butt_submit), "clicked", GTK_SIGNAL_FUNC(save_document), 0);


	/* --- Logo --- */

	w2 = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(w2), GTK_SHADOW_NONE);
	gtk_box_pack_start(GTK_BOX(w1), w2, FALSE, TRUE, 0);
	gtk_widget_show(w2);

	w1 = gtk_hbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(w2), w1);
	gtk_widget_show(w1);

	p = gdk_pixmap_create_from_xpm_d(window->window, &mask,
																	 &style->bg[GTK_STATE_NORMAL],
																	 (gchar **) ilogo_xpm);
	logo = gtk_pixmap_new(p, mask);
	gtk_box_pack_start(GTK_BOX(w1), logo, FALSE, TRUE, 4);
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

	tree = gtk_tree_new();
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(w2), tree);
	gtk_widget_show(tree);

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

	scroller = gtk_scrolled_window_new(NULL, NULL);
	gtk_paned_add2(GTK_PANED(w1), scroller);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_AUTOMATIC,
																 GTK_POLICY_ALWAYS);

	root = gtk_vbox_new(FALSE, 1);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroller), root);
	
	
	/* TEMPORARY: Set white background */
	
  gcol.blue = 0xffff;
	gcol.green = 0xffff;
	gcol.red = 0xffff;
	
	style = gtk_widget_get_default_style();
	style = gtk_style_copy(style);

	gdk_colormap_alloc_color(window->style->colormap, &gcol, 0, 1);

	for (i = 0; i < 5; i++) style->bg[i] = gcol;
	xv_style_r(scroller, style);

	gtk_widget_show(scroller);
	gtk_widget_show(root);


	/* --- Main window -> vbox -> hbox --- */

	w1 = gtk_hbox_new(FALSE, 1);
	gtk_box_pack_start(GTK_BOX(w0), w1, FALSE, TRUE, 0);
	gtk_widget_show(w1);

	/* --- Statusbar --- */

	status = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(w1), status, TRUE, TRUE, 0);
	gtk_widget_show(status);

	/* --- Statustray --- */

	w0 = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(w0), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(w1), w0, FALSE, TRUE, 0);
	gtk_widget_show(w0);

	tray = gtk_hbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(w0), tray);
	gtk_widget_show(tray);

	/* --- Indicator: Auth --- */

	p = gdk_pixmap_create_from_xpm_d(window->window, &mask,
																	 &style->bg[GTK_STATE_NORMAL],
																	 (gchar **) auth_off_xpm);
	auth = gtk_pixmap_new(p, mask);
	gtk_box_pack_start(GTK_BOX(tray), auth, FALSE, TRUE, 2);
	gtk_widget_show(auth);
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
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 460);
	gtk_widget_show(GTK_WIDGET(window));

	curs_init();
	selection_init();
	insert_element_init();

	clipboard = 0;

	/* --- Putting it together --- */

	status_main_ctx = gtk_statusbar_get_context_id(GTK_STATUSBAR(status), "Main");
	gtk_statusbar_push(GTK_STATUSBAR(status), status_main_ctx,
										 "Welcome to the much-too-early Conglomerate editor.");

	/* --- */
	
	if (argc > 2) open_document_do(argv[1], argv[2]);
	
  gtk_main();

	return(0);
}
