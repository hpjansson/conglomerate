/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-primary-window.c
 *
 * Copyright (C) 2002 David Malcolm
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

#if 1
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>
#endif

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

struct CongPrimaryWindow
{
	CongDocument *doc;

/*  	GtkWidget *w; */

	CongTreeView *cong_tree_view;
	CongEditorView *cong_editor_view;

	GtkWidget *window, *menus, *toolbar, *status, *tray, *scroller,
		*auth, *butt_submit, *butt_find;

	guint status_main_ctx;

	GtkAccelGroup *accel;



};

#if 1
GtkWidget* cong_gui_get_a_window()
{
	CongPrimaryWindow *primary_window;

	/* A window _must_ exist for this to work: */
	g_assert(the_globals.primary_windows);

	primary_window = the_globals.primary_windows->data;

	g_assert(primary_window->window);

	return GTK_WIDGET(primary_window->window);
}
#else
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
	gui->global_tree_store = gtk_tree_store_new (TREEVIEW_N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(gui->global_tree_view, GTK_TREE_MODEL(gui->global_tree_store));
 
#if 0
	gtk_widget_destroy(GTK_WIDGET(gui->global_tree_view));
	
	gui->global_tree_view=NULL;
#endif
}
#endif
 
struct CongGlobals the_globals;

extern char *ilogo_xpm[];
extern char *auth_off_xpm[];
extern char *icon_connect[];
extern char *icon_new[];
extern char *icon_assemble[];
extern char *icon_openfile[];
extern char *icon_submit[];

GtkPixmap* cong_primary_window_create_pixmap(CongPrimaryWindow *primary_window, char** xpm)
{
	GdkPixmap *p;
	GdkBitmap *mask;
	GtkStyle *style;

	gtk_widget_realize(GTK_WIDGET(primary_window->window));

	style = gtk_widget_get_style(GTK_WIDGET(primary_window->window));

	p = gdk_pixmap_create_from_xpm_d(primary_window->window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm);
	return GTK_PIXMAP(gtk_pixmap_new(p, mask));
}

GtkWidget* cong_primary_window_toolbar_create_item(CongPrimaryWindow *primary_window,
						   GtkToolbar* toolbar,
						   const char* text, 
						   const char* tooltip_text, 
						   const char* tooltip_private_text, 
						   char** xpm,
						   gint (*handler)(GtkWidget *w, gpointer data),
						   gpointer user_data)
{
	GtkWidget* widget;
	GtkWidget* icon;

	icon = GTK_WIDGET(cong_primary_window_create_pixmap(primary_window, xpm));

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

	gtk_signal_connect(GTK_OBJECT(widget), "clicked", GTK_SIGNAL_FUNC(handler), user_data);

	return widget;
}

void cong_primary_window_toolbar_populate(CongPrimaryWindow *primary_window)
{
	/* Open */
	primary_window->butt_find = cong_primary_window_toolbar_create_item(primary_window,
									    GTK_TOOLBAR(primary_window->toolbar),
									    "Open", 
									    "Open document",
									    "Open document",
									    icon_openfile, 
									    toolbar_callback_open,
									    primary_window);
	
	/* Submit */
	primary_window->butt_submit = cong_primary_window_toolbar_create_item(primary_window,
									      GTK_TOOLBAR(primary_window->toolbar),
									      "Save as...", 
									      "Save document as...",
									      "Save document as...", 
									      icon_submit,
									      toolbar_callback_save_as,
									      primary_window);
}

/* Handy routines for implementing menu callbacks: */
void unimplemented_menu_item(gpointer callback_data,
			     guint callback_action,
			     GtkWidget *widget)
{
	CONG_DO_UNIMPLEMENTED_DIALOG("The selected menu item has not yet been implemented.");
}

static void dispatch_span_editor_command(void (*span_editor_command)(CongSpanEditor *span_editor), gpointer callback_data)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongCursor *cursor;

	g_assert(span_editor_command);
	g_assert(primary_window);

	g_return_if_fail(primary_window->doc);

	cursor = cong_document_get_cursor(primary_window->doc);

	g_assert(cursor);

	(*span_editor_command)(cursor->xed);	
}

static void dispatch_span_editor_command2(void (*span_editor_command)(CongSpanEditor *span_editor, GtkWidget *widget), gpointer callback_data, GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongCursor *cursor;

	g_assert(span_editor_command);
	g_assert(primary_window);

	g_return_if_fail(primary_window->doc);

	cursor = cong_document_get_cursor(primary_window->doc);

	g_assert(cursor);

	(*span_editor_command)(cursor->xed, widget);	
}

/* Callbacks for "File" menu: */
static void menu_callback_file_new(gpointer callback_data,
				   guint callback_action,
				   GtkWidget *widget)
{
	CONG_DO_UNIMPLEMENTED_DIALOG("The selected menu item has not yet been implemented.");
}

static void menu_callback_file_open(gpointer callback_data,
				    guint callback_action,
				    GtkWidget *widget)
{
	open_document();
}

static void menu_callback_file_save(gpointer callback_data,
				    guint callback_action,
				    GtkWidget *widget)
{
	CONG_DO_UNIMPLEMENTED_DIALOG("The selected menu item has not yet been implemented.");
}

static void menu_callback_file_save_as(gpointer callback_data,
				       guint callback_action,
				       GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = primary_window->doc;
	const char *doc_name;

	g_return_if_fail(doc);

	doc_name = get_file_name("Save XML as...");
	if (!doc_name) {
		return;
	}
	
	cong_document_save(doc, doc_name);
}

static void menu_callback_file_save_copy(gpointer callback_data,
					 guint callback_action,
					 GtkWidget *widget)
{
	CONG_DO_UNIMPLEMENTED_DIALOG("The selected menu item has not yet been implemented.");
}

static void menu_callback_file_revert(gpointer callback_data,
				      guint callback_action,
				      GtkWidget *widget)
{
	CONG_DO_UNIMPLEMENTED_DIALOG("The selected menu item has not yet been implemented.");
}

static void menu_callback_file_properties(gpointer callback_data,
					  guint callback_action,
					  GtkWidget *widget)
{
	CONG_DO_UNIMPLEMENTED_DIALOG("The selected menu item has not yet been implemented.");
}

static void menu_callback_file_close(gpointer data, guint
				     callback_action, GtkWidget
				     *widget) {
	/* Need to prompt to save any unsaved data in the current
	   document */
	gtk_widget_destroy(((struct CongPrimaryWindow*)data)->window);
}

/* Callbacks for "Edit" menu: */
static void menu_callback_cut(gpointer callback_data,
			      guint callback_action,
			      GtkWidget *widget)
{
	dispatch_span_editor_command(cong_span_editor_cut, callback_data);
}

static void menu_callback_copy(gpointer callback_data,
			       guint callback_action,
			       GtkWidget *widget)
{
	dispatch_span_editor_command(cong_span_editor_copy, callback_data);
}

static void menu_callback_paste(gpointer callback_data,
				guint callback_action,
				GtkWidget *widget)
{
	dispatch_span_editor_command2(cong_span_editor_paste, callback_data, widget);
}

/* Callbacks for "Help" menu: */
static void menu_callback_about(gpointer callback_data,
				guint callback_action,
				GtkWidget *widget)
{
	GdkPixbuf *logo_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)ilogo_xpm);
	const gchar* authors[] = {"Hans Petter Jansson", "David Malcolm", NULL};

	GtkWidget *about = gnome_about_new("Conglomerate",
					   NULL, /* const gchar *version, */
					   "(C) 1999 Hans Petter Jansson\n(C) 2002 David Malcolm",
					   "Conglomerate will be a user-friendly XML editor for GNOME",
					   authors,
					   NULL, /*  const gchar **documenters, */
					   NULL, /*  const gchar *translator_credits, */
					   logo_pixbuf);
	gdk_pixbuf_unref(logo_pixbuf);

	gtk_dialog_run(GTK_DIALOG(about));

}


/* The menus: */
static GtkItemFactoryEntry menu_items[] =
{
	{ "/_File",             NULL, NULL, 0, "<Branch>" },
	{ "/File/_New...",       NULL, menu_callback_file_new, 0, "<StockItem>", GTK_STOCK_NEW },
	{ "/File/_Open...",      NULL, menu_callback_file_open, 0, "<StockItem>", GTK_STOCK_OPEN },
	{ "/File/", NULL, NULL, 0, "<Separator>" },
	{ "/File/_Save",           "<control>S", menu_callback_file_save, 0, "<StockItem>", GTK_STOCK_SAVE },
	{ "/File/Save _As...",     NULL, menu_callback_file_save_as, 0, "<StockItem>", GTK_STOCK_SAVE_AS },
	{ "/File/Sa_ve a Copy...", "<shift><control>S", menu_callback_file_save_copy, 0, "<Item>" },
	{ "/File/_Revert",         NULL, menu_callback_file_revert, 0, "<StockItem>", GTK_STOCK_REVERT_TO_SAVED },
	{ "/File/", NULL, NULL, 0, "<Separator>" },
	{ "/File/Proper_ties",     NULL, menu_callback_file_properties, 0, "<StockItem>", GTK_STOCK_PROPERTIES },
	{ "/File/", NULL, NULL, 0, "<Separator>" },
	{ "/File/_Close",         "<control>W", menu_callback_file_close, 0, "<StockItem>", GTK_STOCK_CLOSE },
	{ "/File/_Quit",         "<control>Q", gtk_main_quit, 0, "<StockItem>", GTK_STOCK_QUIT },

	{ "/_Edit",                 NULL, 0, 0, "<Branch>" },
	{ "/Edit/_Undo",              "<control>Z", unimplemented_menu_item, 0, "<StockItem>", GTK_STOCK_UNDO },
	{ "/Edit/_Redo",              "<shift><control>Z", unimplemented_menu_item, 0, "<StockItem>", GTK_STOCK_REDO },
	{ "/Edit/", NULL, NULL, 0, "<Separator>" },
	{ "/Edit/Cu_t",              "<control>X", menu_callback_cut, 0, "<StockItem>", GTK_STOCK_CUT },
	{ "/Edit/_Copy",             "<control>C", menu_callback_copy, 0, "<StockItem>", GTK_STOCK_COPY },
	{ "/Edit/_Paste",            "<control>V", menu_callback_paste, 0, "<StockItem>", GTK_STOCK_PASTE },
	{ "/Edit/", NULL, NULL, 0, "<Separator>" },
	{ "/Edit/_Find...",         "<control>F", unimplemented_menu_item, 0, "<StockItem>", GTK_STOCK_FIND },
	{ "/Edit/Find Ne_xt",       "<control>G", unimplemented_menu_item, 0, "<Item>" },
	{ "/Edit/Find Pre_vious",   "<shift><control>G", unimplemented_menu_item, 0, "<Item>" },
	{ "/Edit/R_eplace...",      "<control>R", unimplemented_menu_item, 0, "<StockItem>", GTK_STOCK_FIND_AND_REPLACE },
	{ "/Edit/", NULL, NULL, 0, "<Separator>" },
	{ "/Edit/_Insert...",       NULL, unimplemented_menu_item, 0, "<Item>" },

	{ "/Tests",                 NULL, NULL, 0, "<Branch>" },
	{ "/Tests/Open...",         NULL, test_open_wrap, 0, NULL },
	{ "/Tests/Error",           NULL, test_error_wrap, 0, NULL },
	{ "/Tests/Document Types",  NULL, test_document_types_wrap, 0, NULL },
	{ "/Tests/Transform DocBook to HTML",       NULL, menu_callback_test_transform, 0, NULL },

	{ "/_Help",        NULL, NULL, 0, "<Branch>" },
	{ "/Help/_Contents", "F1", unimplemented_menu_item, 0, "<StockItem>",GTK_STOCK_HELP },
	{ "/Help/_About",    NULL, menu_callback_about, 0, "<Item>" }

};

gint delete_event( GtkWidget *widget,
		   GdkEvent  *event,
		   gpointer   data )
{
	/* This path to window deletion is accessed by a window
	   manager delete_event. This is where we should prompt the user to save
	   any unsaved information in the document */
	g_print ("delete event occurred\n");
	return(FALSE);
}


void destroy( GtkWidget *widget,
	      gpointer   data )
{
	g_print("destroy event occurred\n");
	cong_primary_window_free((CongPrimaryWindow *)data);
	if (g_list_length(the_globals.primary_windows) == 0) {		
		gtk_main_quit();
	}
}

void cong_primary_window_make_gui(CongPrimaryWindow *primary_window)
{
	GtkWidget *w0, *w1, *w2, *w3, *logo;
	GdkPixmap *p;
	GdkBitmap *mask;
	GdkColor gcol;
	GtkStyle *style;
	GtkItemFactory *item_factory;
	gchar *title, *filename;
	int i;

	g_assert(primary_window);

	gdk_rgb_init();

	if (primary_window->doc) {
		filename = cong_document_get_filename(primary_window->doc);
		title = g_strdup_printf("%s - %s", filename, "Conglomerate Editor");
	} else {
		title = g_strdup("Conglomerate Editor");
	}

	/* --- Main window --- */
	primary_window->window = gnome_app_new("Conglomerate",
					       title);

	{	
		GdkPixbuf *icon_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)ilogo_xpm);

		gtk_window_set_icon(GTK_WINDOW(primary_window->window),
				    icon_pixbuf);

		gdk_pixbuf_unref(icon_pixbuf);
	}

	gtk_signal_connect(GTK_OBJECT(primary_window->window), "delete_event",
			   GTK_SIGNAL_FUNC(delete_event), primary_window);

	gtk_signal_connect(GTK_OBJECT(primary_window->window), "destroy",
			   GTK_SIGNAL_FUNC(destroy), primary_window);

	gtk_container_set_border_width(GTK_CONTAINER(primary_window->window), 0);

	gtk_widget_realize(GTK_WIDGET(primary_window->window));

	/* --- Menus --- */

	primary_window->accel = gtk_accel_group_new();
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", primary_window->accel);
	gtk_item_factory_create_items(item_factory, sizeof(menu_items) / sizeof(menu_items[0]),
				      menu_items, primary_window);
	gtk_window_add_accel_group(GTK_WINDOW(primary_window->window), primary_window->accel);

	primary_window->menus = gtk_item_factory_get_widget(item_factory, "<main>");
	gnome_app_set_menus(GNOME_APP(primary_window->window), GTK_MENU_BAR(primary_window->menus));
	gtk_widget_show(primary_window->menus);
	
	/* --- Toolbar --- */
	primary_window->toolbar = gtk_toolbar_new();
	gnome_app_set_toolbar(GNOME_APP(primary_window->window), GTK_TOOLBAR(primary_window->toolbar));
	gtk_widget_show(primary_window->toolbar);

	/* --- Toolbar icons --- */

	cong_primary_window_toolbar_populate(primary_window);

	/* --- Main window -> hpane --- */

	w1 = gtk_hpaned_new();
	gnome_app_set_contents(GNOME_APP(primary_window->window),w1);
	gtk_widget_show(w1);

	g_return_if_fail(primary_window->cong_tree_view);
	g_return_if_fail(primary_window->cong_editor_view);

	/* --- Tree view --- */

	w2 = gtk_scrolled_window_new(NULL, NULL);
	gtk_paned_add1(GTK_PANED(w1), w2);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(w2), GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(GTK_WIDGET(w2), 100, 0);
	gtk_widget_show(w2);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(w2), 
					      cong_tree_view_get_widget(primary_window->cong_tree_view));


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

	primary_window->scroller = gtk_scrolled_window_new(NULL, NULL);
	gtk_paned_add2(GTK_PANED(w1), primary_window->scroller);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(primary_window->scroller), GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_ALWAYS);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(primary_window->scroller), 
					      cong_editor_view_get_widget(primary_window->cong_editor_view));
	
	
	/* TEMPORARY: Set white background */
	
	gcol.blue = 0xffff;
	gcol.green = 0xffff;
	gcol.red = 0xffff;
	
	style = gtk_widget_get_default_style();
	style = gtk_style_copy(style);

	gdk_colormap_alloc_color(primary_window->window->style->colormap, &gcol, 0, 1);

	for (i = 0; i < 5; i++) style->bg[i] = gcol;
	xv_style_r(primary_window->scroller, style);

	gtk_widget_show(primary_window->scroller);


	/* --- Main window -> vbox -> hbox --- */
	primary_window->status = gtk_statusbar_new();
	gnome_app_set_statusbar(GNOME_APP(primary_window->window), primary_window->status);

	/* --- Putting it together --- */

	primary_window->status_main_ctx = gtk_statusbar_get_context_id(GTK_STATUSBAR(primary_window->status), 
								       "Main");
	gtk_statusbar_push(GTK_STATUSBAR(primary_window->status), 
			   primary_window->status_main_ctx,
			   "Welcome to the much-too-early Conglomerate editor.");

}


CongPrimaryWindow *cong_primary_window_new(CongDocument *doc)
{
	/* is it valid to have a NULL document? */

	CongPrimaryWindow *primary_window = g_new0(CongPrimaryWindow, 1);

	primary_window->doc = doc;
	primary_window->cong_tree_view = cong_tree_view_new(doc);
	primary_window->cong_editor_view = cong_editor_view_new(doc);

	cong_primary_window_make_gui(primary_window);
	gtk_window_set_default_size(GTK_WINDOW(primary_window->window), 400, 460);
	gtk_widget_show(GTK_WIDGET(primary_window->window));

#if 0
	primary_window->xv = xmlview_new(doc);
#endif


	the_globals.primary_windows = g_list_prepend(the_globals.primary_windows, primary_window);

	cong_document_set_primary_window(doc, primary_window);

	return primary_window;
	
}

void cong_primary_window_free(CongPrimaryWindow *primary_window)
{
	g_return_if_fail(primary_window);

	cong_tree_view_free(primary_window->cong_tree_view);
	cong_editor_view_free(primary_window->cong_editor_view);
	cong_document_delete(primary_window->doc);

	the_globals.primary_windows = g_list_remove(the_globals.primary_windows, primary_window);	

	g_free(primary_window);
}

CongDocument *cong_primary_window_get_document(CongPrimaryWindow *primary_window)
{
	g_return_val_if_fail(primary_window, NULL);

	return primary_window->doc;
}

void cong_primary_window_update_title(CongPrimaryWindow *primary_window)
{
	gchar *title;
	gchar *filename;

	g_return_if_fail(primary_window);

	if (primary_window->doc) {

		filename = cong_document_get_filename(primary_window->doc);

		if (cong_document_is_modified( primary_window->doc ) ) {
			title = g_strdup_printf("%s (modified) - %s", filename, "Conglomerate Editor");
		} else {
			title = g_strdup_printf("%s - %s", filename, "Conglomerate Editor");
		}

		g_free(filename);

	} else {
		title = g_strdup("Conglomerate Editor");
	}

	gtk_window_set_title( GTK_WINDOW(primary_window->window),
			      title);

	g_free(title);
}
