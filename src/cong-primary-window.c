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
#include "cong-plugin-manager.h"

#if 0
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>
#endif

#define ENABLE_MAIN_WIDGET 1
#define ENABLE_PROPERTY_PAGES 0
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
 * |         |                              |
 * | Tree    | Document view                |
 * | View    |                              |
 * |         |                              |
 * |         |                              |
 * |         |                              |
 * |         |                              |
 * |         |                              |
 * |---------|                              |
 * |Property |                              |
 * |  pages  |                              |
 * |         |                              |
 * |         O <- split pane                |
 * |         |                              |
 * |----------------------------------------|
 * | Status bar                             |
 * `----------------------------------------'
 * 
 */

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
 
CongApp the_app;

/**
 * cong_primary_window_create_pixmap:
 * @primary_window:
 * @xpm:
 *
 * TODO: Write me
 * Returns:
 */
GtkImage*
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
	return GTK_IMAGE(gtk_image_new_from_pixmap(p, mask));
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

#if ENABLE_PROPERTY_PAGES
#if 0
static void
property_page_selection_change_cb (CongDocument *doc, gpointer user_data)
{
	GtkWidget *w = GTK_WIDGET (user_data);

#error move these into an inheritance hierarchy below GtkWidget
	/* Do something sane... */
	gtk_widget_hide (w);
}
#endif

static void
create_property_page_cb (CongService *service,
			 gpointer user_data)
{
	CongServiceNodePropertyPage *custom_property_page = CONG_SERVICE_NODE_PROPERTY_PAGE (service);
	CongPrimaryWindow *primary_window = (CongPrimaryWindow*)user_data;
	/* CongDocument *doc = cong_primary_window_get_document (primary_window); */
	GtkWidget *w = cong_custom_property_page_make (custom_property_page,
						       cong_primary_window_get_document (primary_window));
	gtk_widget_show (w);
	gtk_notebook_append_page (primary_window->property_notebook,
				  w,
				  gtk_label_new (cong_service_get_name (CONG_SERVICE (custom_property_page)))
				  );
#if 0
	g_signal_connect (G_OBJECT (doc), "selection_change", G_CALLBACK (property_page_selection_change_cb), w);
	/* FIXME: disconnect from this signal? */
#endif
}
#endif
	
static void
add_standard_layout_for_doc (CongPrimaryWindow *primary_window, 
			     CongDocument *doc)
{
	GtkWidget *w1 = NULL, *w2 = NULL;
	GtkWidget *sidebar_notebook = NULL;
	GtkWidget *sidebar_vpane;

	/* --- Main window -> hpane --- */
	w1 = gtk_hpaned_new();
	gtk_box_pack_start(GTK_BOX(primary_window->vbox), w1, TRUE, TRUE, 0);
	gtk_widget_show(w1);

	/* Sidebar vpane */
	sidebar_vpane = gtk_vpaned_new();
	gtk_widget_show (sidebar_vpane);
	gtk_paned_add1(GTK_PANED(w1), sidebar_vpane);
	
	/* --- Notebook to appear in the sidebar: --- */
	sidebar_notebook = gtk_notebook_new();
	gtk_widget_show(sidebar_notebook);
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(sidebar_notebook), GTK_POS_BOTTOM);
	gtk_paned_add1(GTK_PANED(sidebar_vpane), sidebar_notebook);
	
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
					      cong_tree_view_get_widget (cong_overview_view_new (primary_window->doc,
												 primary_window)));
	
	/* --- Bookmark view --- */
#if 0
	gtk_notebook_append_page(GTK_NOTEBOOK(sidebar_notebook),
				 cong_bookmark_view_new(primary_window->doc),
				 gtk_label_new(_("Bookmarks"))
				 );
	
#endif
	
	/* --- Raw XML view --- */
	LOG_PRIMARY_WINDOW_CREATION1 ("Creating raw XML view");
	gtk_notebook_append_page (GTK_NOTEBOOK(sidebar_notebook),
				  cong_dom_view_new(primary_window->doc,
						    primary_window),
				  gtk_label_new(_("Raw XML"))
				  );

#if ENABLE_PROPERTY_PAGES
	/* Property pages for sidebar pane */
	{
		primary_window->property_notebook = GTK_NOTEBOOK (gtk_notebook_new());
		gtk_widget_show (GTK_WIDGET (primary_window->property_notebook));
		gtk_notebook_set_tab_pos (primary_window->property_notebook, GTK_POS_BOTTOM);
		gtk_paned_add2 (GTK_PANED(sidebar_vpane), GTK_WIDGET (primary_window->property_notebook));

		cong_plugin_manager_for_each_service_of_type (cong_app_get_plugin_manager (cong_app_singleton ()),
							      CONG_SERVICE_NODE_PROPERTY_PAGE_TYPE,
							      create_property_page_cb,
							      primary_window);
	}
#endif
	
	/* Set up the editor_widget v3: */
#if ENABLE_MAIN_WIDGET
	{
		LOG_PRIMARY_WINDOW_CREATION1 ("Creating v3 widget");
		primary_window->cong_editor_widget3 = cong_editor_widget3_new(doc,
									      primary_window);
		
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

		gtk_widget_grab_focus (GTK_WIDGET(primary_window->cong_editor_widget3));
	}
#endif
	
}

static void
add_tree_layout_for_doc (CongPrimaryWindow *primary_window, 
			 CongDocument *doc)
{
	GtkWidget *widget = cong_dom_view_new (primary_window->doc,
					       primary_window);
	gtk_box_pack_start(GTK_BOX(primary_window->vbox), widget, TRUE, TRUE, 0);
}

static void
refresh_statusbar (CongPrimaryWindow *primary_window)
{
	gchar *status_text = NULL;

	g_assert (primary_window);

	gtk_statusbar_pop(GTK_STATUSBAR(primary_window->status),
	                  primary_window->status_xpath_ctx);
	
	/* Set status bar to XPath of selection (if any) */
	if (primary_window->doc) {
		CongNodePtr selected_node = cong_document_get_selected_node (primary_window->doc);

		if (selected_node) {
			status_text = cong_node_get_path (selected_node);
			gtk_statusbar_push (GTK_STATUSBAR(primary_window->status),
			                    primary_window->status_xpath_ctx,
			                    status_text);
			g_free(status_text);
		}
	}
}

static void
end_edit_cb (CongDocument *doc,
	     CongPrimaryWindow *primary_window)
{
	refresh_statusbar (primary_window);
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
	
	g_assert (primary_window);

	if (doc) {
		primary_window->doc = doc;
		g_object_ref(G_OBJECT(doc));

		if (1 /* cong_document_get_default_dispspec (doc) */) {
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
		
		/* Set up status bar: */
		refresh_statusbar (primary_window);
		g_signal_connect_after (G_OBJECT (doc),
					"end_edit",
					G_CALLBACK (end_edit_cb),
					primary_window);

		cong_document_set_primary_window(doc, primary_window);
	} /* if (doc) */
	
	/* toolbar update */
#if 0
	cong_primary_window_toolbar_populate (primary_window);
#endif
	/* title update */
	cong_primary_window_update_title (primary_window);

	/* update the menus */
	cong_menus_setup_document_action_group (primary_window);
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
	g_assert(primary_window);
	
	/* --- Main window --- */
	primary_window->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(primary_window->window),
	                     _("Conglomerate XML Editor"));

	gtk_window_set_default_icon_name("conglomerate");

	g_signal_connect(primary_window->window, "delete_event",
			 G_CALLBACK(delete_event), primary_window);

	g_signal_connect(primary_window->window, "destroy",
			 G_CALLBACK(destroy), primary_window);

	gtk_container_set_border_width(GTK_CONTAINER(primary_window->window), 0);

	gtk_widget_realize(GTK_WIDGET(primary_window->window));

	/* --- Application vbox --- */

	primary_window->vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(primary_window->window), primary_window->vbox);

	/* --- Menus --- */

	primary_window->merge_id = cong_menus_setup_ui_layout (primary_window);
	primary_window->menus = gtk_ui_manager_get_widget (cong_primary_window_get_ui_manager (primary_window), "/MainMenuBar");
	gtk_box_pack_start(GTK_BOX(primary_window->vbox), primary_window->menus, FALSE, FALSE, 0);

	{
		GtkAccelGroup *accel_group;
		
		accel_group = gtk_ui_manager_get_accel_group (primary_window->ui_manager);
		gtk_window_add_accel_group (GTK_WINDOW (primary_window->window), accel_group);	
	}

	primary_window->recent_manager = gtk_recent_manager_get_default();
	primary_window->recent_action_group = NULL;
	primary_window->recent_ui_id = 0;
	g_signal_connect_swapped (primary_window->recent_manager,
				  "changed",
				  G_CALLBACK (cong_menus_setup_recent_files),
				  primary_window);
	cong_menus_setup_recent_files (primary_window);

	/* --- Toolbar --- */
	primary_window->toolbar = GTK_TOOLBAR (gtk_ui_manager_get_widget (cong_primary_window_get_ui_manager (primary_window), "/MainToolBar"));
	gtk_box_pack_start(GTK_BOX(primary_window->vbox), GTK_WIDGET(primary_window->toolbar), FALSE, FALSE, 0);

	primary_window->doc = NULL;

	/* --- Main window -> status area --- */

	primary_window->status = gtk_statusbar_new();

	gtk_box_pack_end(GTK_BOX(primary_window->vbox), primary_window->status, FALSE, FALSE, 0);

	/* --- Putting it together --- */

	primary_window->status_main_ctx = gtk_statusbar_get_context_id(GTK_STATUSBAR(primary_window->status), 
								       "Main");
	primary_window->status_xpath_ctx = gtk_statusbar_get_context_id(GTK_STATUSBAR(primary_window->status),
	                                                                "xpath");
	gtk_statusbar_push(GTK_STATUSBAR(primary_window->status),
			   primary_window->status_main_ctx,
			   _("Welcome to the much-delayed Conglomerate editor."));

	gtk_widget_show_all(primary_window->vbox);
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

	primary_window->ui_manager = gtk_ui_manager_new ();
	cong_menus_setup_action_groups (primary_window);

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

	/* g_message ("cong_primary_window_free"); */

	if (primary_window->doc) {
		g_signal_handlers_disconnect_by_func (G_OBJECT(primary_window->doc),end_edit_cb, primary_window);
		g_object_unref(G_OBJECT(primary_window->doc));
	}

	
	if (primary_window->recent_action_group) {
		g_object_unref (primary_window->recent_action_group);
		primary_window->recent_action_group = NULL;
	}

	if (primary_window->recent_manager) {
		g_signal_handlers_disconnect_by_func (primary_window->recent_manager,
						      cong_menus_setup_recent_files,
						      primary_window);
		primary_window->recent_manager = NULL;
	}

	primary_window->recent_ui_id = 0;

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

/**
 * cong_primary_window_get_ui_manager:
 * @primary_window:
 *
 * TODO: Write me
 * Returns:
 */
GtkUIManager*
cong_primary_window_get_ui_manager (CongPrimaryWindow *primary_window)
{
	g_return_val_if_fail (primary_window, NULL);

	return primary_window->ui_manager;
}

GtkActionGroup*
cong_primary_window_get_action_group (CongPrimaryWindow *primary_window,
				      enum CongActionGroup action_group)
{
	g_return_val_if_fail (primary_window, NULL);
	g_return_val_if_fail (action_group<NUM_CONG_ACTION_GROUPS, NULL);

	return primary_window->action_group[action_group];
}

/**
 * cong_primary_window_action_set_sensitive:
 * @action_name: name of Action item
 * @state: TRUE for sensitive, FALSE for insensitive
 * @primary_window: the primary window containing the action
 *
 * Makes the given Action (@action_name) sensitive (@state=TRUE)
 * or insensitive (@state=FALSE) for the primary window
 * @primary_window.
 */
void
cong_primary_window_action_set_sensitive (CongPrimaryWindow *primary_window,
					  gchar *action_name,
					  gboolean state) {
	GtkActionGroup *action_group;
	GtkAction *action;

	action_group = cong_primary_window_get_action_group (primary_window, CONG_ACTION_GROUP_DOCUMENT);
	g_assert (action_group);

	action = gtk_action_group_get_action (action_group, action_name);
	g_assert (action);

	g_object_set (G_OBJECT (action), "sensitive", state, NULL);
}

/**
 * cong_primary_window_action_set_label:
 * @action_name: name of Action item
 * @label: label for the Action
 * @primary_window: the primary window containing the action
 *
 * Sets the label for the Action (@action_name) whose
 * primary window is @primary_window.
 *
 * NOTE: should we make a copy of label?
 */
void
cong_primary_window_action_set_label (CongPrimaryWindow *primary_window,
				      gchar *action_name,
				      gchar *label) {
	GtkActionGroup *action_group;
	GtkAction *action;

	action_group = cong_primary_window_get_action_group (primary_window, CONG_ACTION_GROUP_DOCUMENT);
	g_assert (action_group);

	action = gtk_action_group_get_action (action_group, action_name);
	g_assert (action);

	g_object_set (G_OBJECT (action), "label", (gpointer)label, NULL);
}

