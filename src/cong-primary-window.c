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
#include "../config.h"
#include "cong-dispspec.h"
#include "cong-document.h"
#include "cong-error-dialog.h"
#include "cong-dialog.h"
#include "cong-plugin.h"
#include "cong-app.h"
#include "cong-overview-view.h"
#include "cong-primary-window.h"
#include "cong-editor-widget.h"
#include "cong-command-history.h"
#include "cong-selection.h"
#include "cong-range.h"
#include "cong-ui-hooks.h"

#if 0
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>
#endif

#define ENABLE_MAIN_WIDGET 1
#define LOG_PRIMARY_WINDOW_CREATION 0

#if LOG_PRIMARY_WINDOW_CREATION
#define LOG_PRIMARY_WINDOW_CREATION1(msg) g_message(msg)
#else
#define LOG_PRIMARY_WINDOW_CREATION1(msg) ((void)0)
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
 * | AppBar                                 |
 * `----------------------------------------'
 * 
 */

struct CongPrimaryWindow
{
	CongDocument *doc;

/*  	GtkWidget *w; */

	GtkWidget *cong_editor_widget3;
	GtkWidget *scroller3;

	GtkWidget *window, *menus;
	GtkToolbar *toolbar;
#if 1
	GtkWidget *app_bar;
#else
	GtkWidget *status;
#endif
	GtkWidget *auth, *butt_submit, *butt_find;
	
#if 0
	guint status_main_ctx;
#endif

	GtkAccelGroup *accel;



};

#if 1
/**
 * cong_gui_get_a_window:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget* 
cong_gui_get_a_window(void)
{
	CongPrimaryWindow *primary_window;

	/* A window _must_ exist for this to work: */
	if (cong_app_singleton()->primary_windows) {

		primary_window = cong_app_singleton()->primary_windows->data;

		g_assert(primary_window->window);

		return GTK_WIDGET(primary_window->window);
	} else {
		/* No primary windows exist: */
		return NULL;
	}
}
#else
/**
 * cong_gui_get_window:
 * @gui:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget* 
cong_gui_get_window(struct cong_gui* gui)
{
	g_assert(gui!=NULL);

	return GTK_WIDGET(gui->window);
}

/**
 * cong_gui_get_popup:
 * @gui:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget* 
cong_gui_get_popup(struct cong_gui* gui)
{
	g_assert(gui!=NULL);

	return gui->popup;
}

/**
 * cong_gui_set_popup:
 * @gui:
 * @popup:
 *
 * TODO: Write me
 */
void 
cong_gui_set_popup(struct cong_gui* gui, GtkWidget* popup)
{
	g_assert(gui!=NULL);

	gui->popup=popup;
}

/**
 * cong_gui_get_button_submit:
 * @gui:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget* 
cong_gui_get_button_submit(struct cong_gui* gui)
{
	g_assert(gui!=NULL);

	return gui->butt_submit;
}

/**
 * cong_gui_get_tree_store:
 * @gui:
 *
 * TODO: Write me
 * Returns:
 */
GtkTreeStore* 
cong_gui_get_tree_store(struct cong_gui* gui)
{
	g_assert(gui!=NULL);

	return gui->global_tree_store;
}

/**
 * cong_gui_get_root:
 * @gui:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget* 
cong_gui_get_root(struct cong_gui* gui)
{
	g_assert(gui!=NULL);

	return gui->root;
}

/**
 * cong_gui_destroy_tree_store:
 * @gui:
 *
 * TODO: Write me
 */
void 
cong_gui_destroy_tree_store(struct cong_gui* gui)
{
	gui->global_tree_store = gtk_tree_store_new (TREEVIEW_N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(gui->global_tree_view, GTK_TREE_MODEL(gui->global_tree_store));
 
#if 0
	gtk_widget_destroy(GTK_WIDGET(gui->global_tree_view));
	
	gui->global_tree_view=NULL;
#endif
}
#endif
 
struct CongApp the_app;

extern char *ilogo_xpm[];
extern char *auth_off_xpm[];
extern char *icon_connect[];
extern char *icon_new[];
extern char *icon_assemble[];
extern char *icon_openfile[];
extern char *icon_submit[];

/**
 * cong_primary_window_create_pixmap:
 * @primary_window:
 * @xpm:
 *
 * TODO: Write me
 * Returns:
 */
GtkPixmap* 
cong_primary_window_create_pixmap(CongPrimaryWindow *primary_window, char** xpm)
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

/**
 * toolbar_callback_undo:
 * @w:
 * @data:
 *
 * TODO: Write me
 * Returns:
 */
gint 
toolbar_callback_undo(GtkWidget *w, gpointer data)
{
	CongPrimaryWindow *primary_window = data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	cong_document_undo(doc);

	return TRUE;
}

/**
 * toolbar_callback_redo:
 * @w:
 * @data:
 *
 * TODO: Write me
 * Returns:
 */
gint 
toolbar_callback_redo(GtkWidget *w, gpointer data)
{
	CongPrimaryWindow *primary_window = data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	cong_document_redo(doc);

	return TRUE;
}

/**
 * toolbar_callback_cut:
 * @w:
 * @data:
 *
 * TODO: Write me
 * Returns:
 */
gint 
toolbar_callback_cut(GtkWidget *w, gpointer data)
{
	CongPrimaryWindow *primary_window = data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	cong_document_cut_selection(doc);

	return TRUE;
}

/**
 * toolbar_callback_copy:
 * @w:
 * @data:
 *
 * TODO: Write me
 * Returns:
 */
gint 
toolbar_callback_copy(GtkWidget *w, gpointer data)
{
	CongPrimaryWindow *primary_window = data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	cong_document_copy_selection(doc);

	return TRUE;
}

/**
 * toolbar_callback_paste:
 * @w:
 * @data:
 *
 * TODO: Write me
 * Returns:
 */
gint 
toolbar_callback_paste(GtkWidget *w, gpointer data)
{
	CongPrimaryWindow *primary_window = data;
	CongDocument *doc = cong_primary_window_get_document(primary_window);

	cong_document_paste_clipboard_or_selection(doc, w);

	return TRUE;
}


static void 
on_history_changed_undo (CongCommandHistory *history,
			 gpointer user_data)
{
	GtkWidget *undo = GTK_WIDGET (user_data);
	
	gtk_widget_set_sensitive (undo,
				  cong_command_history_can_undo (history));
}

static void 
on_history_changed_redo (CongCommandHistory *history,
			 gpointer user_data)
{
	GtkWidget *redo = GTK_WIDGET (user_data);
	
	gtk_widget_set_sensitive (redo,
				  cong_command_history_can_redo (history));	
}

static void 
on_selection_changed_copy (CongDocument *document,
			 gpointer user_data)
{
	GtkWidget *copy = GTK_WIDGET (user_data);
	
        CongSelection *selection = cong_document_get_selection(document);
        CongRange *range = cong_selection_get_ordered_range(selection);
        gtk_widget_set_sensitive (copy,
                                  cong_range_can_be_copied (range));
}

static void 
on_selection_changed_cut (CongDocument *document,
			 gpointer user_data)
{
	GtkWidget *cut = GTK_WIDGET (user_data);
	
        CongSelection *selection = cong_document_get_selection(document);
        CongRange *range = cong_selection_get_ordered_range(selection);
        gtk_widget_set_sensitive (cut,
                                  cong_range_can_be_cut (range));
}

/**
 * cong_primary_window_toolbar_populate:
 * @primary_window:
 *
 * TODO: Write me
 */
void 
cong_primary_window_toolbar_populate(CongPrimaryWindow *primary_window)
{
	if ( !(primary_window->doc) ) {
		gtk_toolbar_insert_stock(primary_window->toolbar, 
				 GTK_STOCK_OPEN,
				 _("Open document"),
				 _("Open document"),
				 GTK_SIGNAL_FUNC(toolbar_callback_open),
				 primary_window,
				 -1);
		
		gtk_toolbar_insert_stock(primary_window->toolbar,
				GTK_STOCK_NEW,
				_("New document"),
				_("New document"),
				GTK_SIGNAL_FUNC(toolbar_callback_new),
				primary_window,
				-1);

	}

	if (primary_window->doc) {
		CongDocument *doc = cong_primary_window_get_document (primary_window);
		CongCommandHistory *history = cong_document_get_command_history (doc);
		GtkWidget *undo;
		GtkWidget *redo;
		GtkWidget *copy;
		GtkWidget *cut;
		GtkWidget *paste;

		gtk_toolbar_insert_stock(primary_window->toolbar, 
					 GTK_STOCK_SAVE,
					 _("Save document"),
					 _("Save document"), 
					 GTK_SIGNAL_FUNC(toolbar_callback_save),
					 primary_window,
					 -1);
		gtk_toolbar_append_space(primary_window->toolbar);
		undo = gtk_toolbar_insert_stock(primary_window->toolbar, 
						GTK_STOCK_UNDO,
						_("Undo"),
						_("Undo"), 
						GTK_SIGNAL_FUNC(toolbar_callback_undo),
						primary_window,
						-1);
		redo = gtk_toolbar_insert_stock(primary_window->toolbar, 
						GTK_STOCK_REDO,
						_("Redo"),
						_("Redo"), 
						GTK_SIGNAL_FUNC(toolbar_callback_redo),
						primary_window,
						-1);
		g_signal_connect (G_OBJECT(history),
				  "changed",
				  G_CALLBACK(on_history_changed_undo),
				  undo);
		g_signal_connect (G_OBJECT(history),
				  "changed",
				  G_CALLBACK(on_history_changed_redo),
				  redo);		
		gtk_widget_set_sensitive (undo, FALSE);
		gtk_widget_set_sensitive (redo, FALSE);

		gtk_toolbar_append_space(primary_window->toolbar);
		cut = gtk_toolbar_insert_stock(primary_window->toolbar, 
					 GTK_STOCK_CUT,
					 _("Cut"),
					 _("Cut"), 
					 GTK_SIGNAL_FUNC(toolbar_callback_cut),
					 primary_window,
					 -1);
		copy = gtk_toolbar_insert_stock(primary_window->toolbar, 
					 GTK_STOCK_COPY,
					 _("Copy"),
					 _("Copy"), 
					 GTK_SIGNAL_FUNC(toolbar_callback_copy),
					 primary_window,
					 -1);
		paste = gtk_toolbar_insert_stock(primary_window->toolbar, 
					 GTK_STOCK_PASTE,
					 _("Paste"),
					 _("Paste"), 
					 GTK_SIGNAL_FUNC(toolbar_callback_paste),
					 primary_window,
					 -1);
		g_signal_connect (G_OBJECT(doc),
				  "selection_change",
				  G_CALLBACK(on_selection_changed_copy),
				  copy);
		g_signal_connect (G_OBJECT(doc),
				  "selection_change",
				  G_CALLBACK(on_selection_changed_cut),
				  cut);		
		/* FIXME: What signal to connect paste to in order to update sensitivity? */
		gtk_widget_set_sensitive (copy, FALSE);
		gtk_widget_set_sensitive (cut, FALSE);
		gtk_widget_set_sensitive (paste, cong_document_can_paste(doc));
	}
}

/**
 * cong_primary_window_can_close:
 * @primary_window:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
cong_primary_window_can_close(CongPrimaryWindow *primary_window)
{
	g_assert(primary_window);

	/* If there's no document, you're free to close */
	if (!primary_window->doc) {
		return TRUE;
	}

	/* See if document was modified */
	if (cong_document_is_modified(primary_window->doc)) {
		gint result; 
		GtkWidget *dialog;

		dialog = GTK_WIDGET( cong_dialog_save_confirmation_alert_new(GTK_WINDOW(primary_window->window),
									     cong_document_get_filename(primary_window->doc),
									     cong_document_get_seconds_since_last_save_or_load(primary_window->doc)));

		/* Bring the modified buffer to the top */
		gtk_window_present(GTK_WINDOW(primary_window->window));

		/* Put the dialog back on top of the window and make
		   sure it has focus */
		gtk_window_present(GTK_WINDOW(dialog));

		result = gtk_dialog_run (GTK_DIALOG (dialog));

		switch (result) {
		default: g_assert_not_reached();
		case CONG_SAVE_CONFIRMATION_RESULT_SAVE_AND_CLOSE:
			save_document(primary_window->doc, cong_primary_window_get_toplevel(primary_window));
			break;

		case CONG_SAVE_CONFIRMATION_RESULT_CLOSE_WITHOUT_SAVING:
			break;
			
		case GTK_RESPONSE_DELETE_EVENT:
			/* (the dialog was deleted, probably by the user clicking on a window manager button) */
		case CONG_SAVE_CONFIRMATION_RESULT_CANCEL:
			gtk_widget_destroy (dialog);
			return FALSE;
		}

		gtk_widget_destroy (dialog);
	}
	
	return TRUE;
}

/**
 * delete_event:
 * @widget:
 * @event:
 * @data:
 *
 * TODO: Write me
 * Returns:
 */
gint 
delete_event( GtkWidget *widget,
	      GdkEvent  *event,
	      gpointer   data )
{
	/* If the window can't close, return TRUE so the callback chain
	   ends here */
	return ! cong_primary_window_can_close((CongPrimaryWindow *)data);
}

/**
 * destroy:
 * @widget:
 * @data:
 *
 * TODO: Write me
 */
void 
destroy( GtkWidget *widget,
	 gpointer   data )
{
	cong_primary_window_free((CongPrimaryWindow *)data);
	if (g_list_length(cong_app_singleton()->primary_windows) == 0) {		
		gtk_main_quit();
	}
}

static void
add_standard_layout_for_doc (CongPrimaryWindow *primary_window, 
			     CongDocument *doc)
{
	GtkWidget *w1 = NULL, *w2 = NULL;
	GtkWidget *sidebar_notebook = NULL;

	/* --- Main window -> hpane --- */
	w1 = gtk_hpaned_new();
	gnome_app_set_contents(GNOME_APP(primary_window->window),w1);
	gtk_widget_show(w1);
	
	/* --- Notebook to appear in the sidebar: --- */
	sidebar_notebook = gtk_notebook_new();
	gtk_widget_show(sidebar_notebook);
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(sidebar_notebook), GTK_POS_BOTTOM);
	
	gtk_paned_add1(GTK_PANED(w1), sidebar_notebook);
	
	/* --- Tree view --- */
	LOG_PRIMARY_WINDOW_CREATION1 ("Creating overview");
	
	w2 = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(w2), GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(GTK_WIDGET(w2), 100, 0);
	gtk_widget_show(w2);
	
	gtk_notebook_append_page(GTK_NOTEBOOK(sidebar_notebook),
				 w2,
				 gtk_label_new(_("Overview"))
				 );
	
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(w2), 
					      cong_tree_view_get_widget (cong_overview_view_new (primary_window->doc)));
	
	/* --- Bookmark view --- */
#if 0
	gtk_notebook_append_page(GTK_NOTEBOOK(sidebar_notebook),
				 cong_bookmark_view_new(primary_window->doc),
				 gtk_label_new(_("Bookmarks"))
				 );
	
#endif
	
	/* --- Raw XML view --- */
	LOG_PRIMARY_WINDOW_CREATION1 ("Creating raw XML view");
	gtk_notebook_append_page(GTK_NOTEBOOK(sidebar_notebook),
				 cong_dom_view_new(primary_window->doc),
				 gtk_label_new(_("Raw XML"))
				 );
	
		/* Set up the editor_widget v3: */
#if ENABLE_MAIN_WIDGET
	{
		LOG_PRIMARY_WINDOW_CREATION1 ("Creating v3 widget");
		primary_window->cong_editor_widget3 = cong_editor_widget3_new(doc);
		
		/* --- Scrolling area for editor widget 3--- */
		primary_window->scroller3 = gtk_scrolled_window_new(NULL, NULL);
		
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(primary_window->scroller3), 
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_ALWAYS);
		
		gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(primary_window->scroller3), 
						       GTK_WIDGET(primary_window->cong_editor_widget3));
		gtk_widget_show (primary_window->cong_editor_widget3);
		gtk_widget_show(primary_window->scroller3);
		
		gtk_paned_add2 (GTK_PANED(w1), 
				primary_window->scroller3);
	}
#endif
	
}

static void
add_tree_layout_for_doc (CongPrimaryWindow *primary_window, 
			 CongDocument *doc)
{
	GtkWidget *widget = cong_dom_view_new(primary_window->doc);
	gnome_app_set_contents(GNOME_APP(primary_window->window),widget);
}

/**
 * cong_primary_window_add_doc:
 * @primary_window:
 * @doc:
 * 
 * Adds the #CongDocument @doc to a given window. This function is called from 
 * cong_primary_window_new().
 *
 */
void
cong_primary_window_add_doc (CongPrimaryWindow *primary_window, CongDocument *doc)
{
	GdkColor gcol;
	GtkStyle *style;
	int i;
	GtkItemFactory *item_factory = NULL;
	
	g_assert (primary_window);

	if (doc) {
		gchar *status_text;
		primary_window->doc = doc;
		g_object_ref(G_OBJECT(doc));

		if (cong_document_get_dispspec (doc)) {
			add_standard_layout_for_doc (primary_window, 
						     doc);
		} else {
			add_tree_layout_for_doc (primary_window,
						 doc);
		}

		/* TEMPORARY: Set white background */
		
		gcol.blue = 0xffff;
		gcol.green = 0xffff;
		gcol.red = 0xffff;
		
		style = gtk_widget_get_default_style();
		style = gtk_style_copy(style);
		
		gdk_colormap_alloc_color(primary_window->window->style->colormap, &gcol, 0, 1);
	
		for (i = 0; i < 5; i++) {
			style->bg[i] = gcol;
		}
		
		/* update the statusbar */
		
		status_text = g_strdup(cong_dispspec_get_name( cong_document_get_dispspec(primary_window->doc) ));
#if 1
		gnome_appbar_set_status (GNOME_APPBAR(primary_window->app_bar), 
			 status_text);
#else
		gtk_statusbar_push(GTK_STATUSBAR(primary_window->status), 
				   primary_window->status_main_ctx,
				   status_text);
#endif
		g_free(status_text);
		cong_document_set_primary_window(doc, primary_window);
	} /* if (doc) */
	
	/* toolbar update */
	cong_primary_window_toolbar_populate (primary_window);
	/* title update */
	cong_primary_window_update_title (primary_window);
	/* update the menus */
	item_factory = gtk_item_factory_from_widget (primary_window->menus);
	cong_menus_create_items(item_factory, primary_window);
}

/**
 * cong_primary_window_make_gui:
 * @primary_window:
 *
 * TODO: Write me
 */
void 
cong_primary_window_make_gui(CongPrimaryWindow *primary_window)
{
	GtkItemFactory *item_factory;
	gchar *title;

	g_assert(primary_window);

	gdk_rgb_init();

	title = g_strdup(_("Conglomerate XML Editor"));
	
	/* --- Main window --- */
	primary_window->window = gnome_app_new(PACKAGE_NAME,
					       title);

	gtk_window_set_default_icon_from_file(
		gnome_program_locate_file (cong_app_get_gnome_program (cong_app_singleton()),
					   GNOME_FILE_DOMAIN_APP_PIXMAP,
					   "conglomerate/conglomerate-icon-32.png", FALSE, 
					   NULL), 
		NULL);

	gtk_signal_connect(GTK_OBJECT(primary_window->window), "delete_event",
			   GTK_SIGNAL_FUNC(delete_event), primary_window);

	gtk_signal_connect(GTK_OBJECT(primary_window->window), "destroy",
			   GTK_SIGNAL_FUNC(destroy), primary_window);

	gtk_container_set_border_width(GTK_CONTAINER(primary_window->window), 0);

	gtk_widget_realize(GTK_WIDGET(primary_window->window));

	/* --- Menus --- */

	primary_window->accel = gtk_accel_group_new();
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", primary_window->accel);
	gtk_item_factory_set_translate_func(item_factory, (GtkTranslateFunc) gettext, NULL, NULL);

	cong_menus_create_items(item_factory, primary_window);

	gtk_window_add_accel_group(GTK_WINDOW(primary_window->window), primary_window->accel);

	primary_window->menus = gtk_item_factory_get_widget(item_factory, "<main>");
	gnome_app_set_menus(GNOME_APP(primary_window->window), GTK_MENU_BAR(primary_window->menus));
	gtk_widget_show(primary_window->menus);
	
	/* --- Toolbar --- */
	primary_window->toolbar = GTK_TOOLBAR(gtk_toolbar_new());
	gnome_app_set_toolbar(GNOME_APP(primary_window->window), primary_window->toolbar);
	gtk_widget_show(GTK_WIDGET(primary_window->toolbar));

	/* --- Toolbar icons --- */

	cong_primary_window_toolbar_populate(primary_window);

	primary_window->doc = NULL;

	/* --- Main window -> status area --- */
#if 1
	primary_window->app_bar = gnome_appbar_new (TRUE,
						    TRUE,
						    GNOME_PREFERENCES_NEVER);

	gnome_app_set_statusbar(GNOME_APP(primary_window->window), primary_window->app_bar);

	{
		gchar *status_text = g_strdup(_("Welcome to the much-delayed Conglomerate editor."));	

		gnome_appbar_set_status (GNOME_APPBAR(primary_window->app_bar), 
					 status_text);

		g_free(status_text);
	}
	
#else
	primary_window->status = gtk_statusbar_new();

	gnome_app_set_statusbar(GNOME_APP(primary_window->window), primary_window->status);

	/* --- Putting it together --- */

	primary_window->status_main_ctx = gtk_statusbar_get_context_id(GTK_STATUSBAR(primary_window->status), 
								       "Main");

	{
		gchar *status_text = g_strdup(_("Welcome to the much-delayed Conglomerate editor."));	

		gtk_statusbar_push(GTK_STATUSBAR(primary_window->status), 
				   primary_window->status_main_ctx,
				   status_text);

		g_free(status_text);
	}
#endif
}

/**
 * cong_primary_window_new:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
CongPrimaryWindow *
cong_primary_window_new(CongDocument *doc)
{

	CongPrimaryWindow *prwin = (CongPrimaryWindow *)g_list_nth_data(cong_app_singleton()->primary_windows, 0);
	CongPrimaryWindow *primary_window = NULL;
	
	if (doc && prwin && !(prwin->doc))	// we have an empty window
	{
		cong_primary_window_add_doc (prwin, doc);
		LOG_PRIMARY_WINDOW_CREATION1 ("Finished adding document to empty primary window");
		return g_list_nth_data (cong_app_singleton()->primary_windows, 0);
	};
	
	primary_window = g_new0(CongPrimaryWindow, 1);

	cong_primary_window_make_gui(primary_window);
	gtk_window_set_default_size(GTK_WINDOW(primary_window->window), 400, 460);


	LOG_PRIMARY_WINDOW_CREATION1 ("Showing the primary window");
	gtk_widget_show_all(GTK_WIDGET(primary_window->window));

	cong_app_singleton()->primary_windows = g_list_prepend(cong_app_singleton()->primary_windows, primary_window);

	if (doc) {
		cong_primary_window_add_doc (primary_window, doc);
	};
	
	LOG_PRIMARY_WINDOW_CREATION1 ("Finished creating primary window");

	return primary_window;
	
}

/**
 * cong_primary_window_free:
 * @primary_window:
 *
 * TODO: Write me
 */
void 
cong_primary_window_free(CongPrimaryWindow *primary_window)
{
	g_return_if_fail(primary_window);

	g_message ("cong_primary_window_free");

	if (primary_window->doc) {
		g_object_unref(G_OBJECT(primary_window->doc));
	}

	cong_app_singleton()->primary_windows = g_list_remove(cong_app_singleton()->primary_windows, primary_window);	

	g_free(primary_window);
}

/**
 * cong_primary_window_get_document:
 * @primary_window:
 *
 * TODO: Write me
 * Returns:
 */
CongDocument *
cong_primary_window_get_document(CongPrimaryWindow *primary_window)
{
	g_return_val_if_fail(primary_window, NULL);

	return primary_window->doc;
}

/**
 * cong_primary_window_update_title:
 * @primary_window:
 *
 * TODO: Write me
 */
void 
cong_primary_window_update_title(CongPrimaryWindow *primary_window)
{
	gchar *title;
	gchar *filename;

	g_return_if_fail(primary_window);

	if (primary_window->doc) {

		filename = cong_document_get_filename(primary_window->doc);

		if (cong_document_is_modified( primary_window->doc ) ) {
			title = g_strdup_printf(_("%s (modified) - %s"), filename, _("Conglomerate XML Editor"));
		} else {
			title = g_strdup_printf("%s - %s", filename, _("Conglomerate XML Editor"));
		}

		g_free(filename);

	} else {
		title = g_strdup(_("Conglomerate XML Editor"));
	}

	gtk_window_set_title( GTK_WINDOW(primary_window->window),
			      title);

	g_free(title);
}

/**
 * cong_primary_window_get_toplevel:
 * @primary_window:
 *
 * TODO: Write me
 * Returns:
 */
GtkWindow *
cong_primary_window_get_toplevel(CongPrimaryWindow *primary_window)
{
	g_return_val_if_fail(primary_window, NULL);

	return GTK_WINDOW(primary_window->window);
}
