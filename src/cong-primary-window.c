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

#if 1
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>
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
#if USE_CONG_EDITOR_WIDGET
	GtkWidget *cong_editor_widget;
#else
	CongEditorView *cong_editor_view;
#endif

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
									    _("Open"), 
									    _("Open document"),
									    _("Open document"),
									    icon_openfile, 
									    toolbar_callback_open,
									    primary_window);
	
	/* Submit */
	primary_window->butt_submit = cong_primary_window_toolbar_create_item(primary_window,
									      GTK_TOOLBAR(primary_window->toolbar),
									      _("Save as..."), 
									      _("Save document as..."),
									      _("Save document as..."), 
									      icon_submit,
									      toolbar_callback_save_as,
									      primary_window);
}


gboolean cong_primary_window_can_close(CongPrimaryWindow *primary_window)
{
	g_assert(primary_window);

	/* If there's no document, you're free to close */
	if (!primary_window->doc)
		return TRUE;

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
			save_document(primary_window->doc);
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


/* Handy routines for implementing menu callbacks: */
void unimplemented_menu_item(gpointer callback_data,
			     guint callback_action,
			     GtkWidget *widget)
{
	CONG_DO_UNIMPLEMENTED_DIALOG(_("The selected menu item has not yet been implemented."));
}

static void dispatch_document_command(void (*document_command)(CongDocument *doc), gpointer callback_data)
{
	CongPrimaryWindow *primary_window = callback_data;

	g_assert(document_command);
	g_assert(primary_window);

	g_return_if_fail(primary_window->doc);

	(*document_command)(primary_window->doc);	
}

static void dispatch_document_command2(void (*document_command)(CongDocument *doc, GtkWidget *widget), gpointer callback_data, GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;

	g_assert(document_command);
	g_assert(primary_window);

	g_return_if_fail(primary_window->doc);

	(*document_command)(primary_window->doc, widget);	
}

static GtkWidget* make_uneditable_text(const gchar* text)
{
	GtkEntry *entry = GTK_ENTRY(gtk_entry_new());

	gtk_entry_set_text(entry, text);
	gtk_entry_set_editable(entry, FALSE);

	return GTK_WIDGET(entry);
}


/* Callbacks for "File" menu: */
static void menu_callback_file_new(gpointer callback_data,
				   guint callback_action,
				   GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	new_document(GTK_WINDOW(primary_window->window));
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
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = primary_window->doc;

	save_document(doc);
}

static void menu_callback_file_save_as(gpointer callback_data,
				       guint callback_action,
				       GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = primary_window->doc;

	save_document_as(doc);
}

static void menu_callback_file_save_copy(gpointer callback_data,
					 guint callback_action,
					 GtkWidget *widget)
{
	CONG_DO_UNIMPLEMENTED_DIALOG(_("The selected menu item has not yet been implemented."));
}

static void menu_callback_file_revert(gpointer callback_data,
				      guint callback_action,
				      GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = primary_window->doc;

	g_return_if_fail(doc);

	if (cong_document_is_modified(doc)) {
		gchar* filename;
		GtkDialog *dialog;
		gint result;

		filename = cong_document_get_filename(doc);

		dialog = cong_dialog_revert_confirmation_alert_new(NULL, /* FIXME:  use correct window */
								   filename,
								   cong_document_get_seconds_since_last_save_or_load(doc));

		g_free(filename);

		result = gtk_dialog_run(dialog);

		gtk_widget_destroy(GTK_WIDGET(dialog));

		if (result != CONG_REVERT_CONFIRMATION_RESULT_REVERT) {
			return;
		}

		CONG_DO_UNIMPLEMENTED_DIALOG(_("The selected menu item has not yet been implemented."));
	} 
}

struct add_importer_to_list_data
{
	const gchar *mime_type;
	GList **list_head;
};

static void add_importer_to_list(CongImporter *importer, gpointer user_data)
{
	struct add_importer_to_list_data *data = user_data;
	if (cong_importer_supports_mime_type(importer, data->mime_type) ) {
		*data->list_head = g_list_append(*data->list_head, importer);
	}
}

static void menu_callback_file_import(gpointer callback_data,
				      guint callback_action,
				      GtkWidget *widget)
{
	const char *filename;

	/* FIXME: this option should be disabled if there are no importers installed */

	filename = get_file_name(_("Import file..."));

	if (filename) {
		const char* mime_type = gnome_vfs_mime_type_from_name(filename);
		GList *list_of_valid = NULL; 
		struct add_importer_to_list_data data;
		CongImporter *importer = NULL;

		g_message("Got mimetype: \"%s\"",mime_type);

		data.mime_type = mime_type;
		data.list_head = &list_of_valid;


		/* Construct a list of importers that can handle this mimetype: */
		cong_plugin_manager_for_each_importer(the_globals.plugin_manager, add_importer_to_list, &data);

		/* OK:  there are three cases:
		   (i) if no importers can handle this mimetype; then tell the user and give them the option of cancelling or forcing the use of a plugin (with a dialog to choose)
		   (ii) if exactly one importer can handle the mimetype, then use it
		   (iii) if more than one importer can handle it, then present the user with a choice dialog.
		*/
		if (NULL==list_of_valid) {
			GtkDialog* dialog;
			gchar *what_failed;
			gchar *why_failed;
			gchar *suggestions;

			/* FIXME:  eventually provide a convenience dialog to force things */

			what_failed = g_strdup_printf(_("Could not import the file."));
			why_failed = g_strdup_printf(_("None of Conglomerate's plugins know how to load files of type \"%s\""), gnome_vfs_mime_get_description(mime_type));
			suggestions = g_strdup_printf(_("FIXME"));
			dialog = cong_error_dialog_new(what_failed,
						       why_failed,
						       suggestions);
			cong_error_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(GTK_WIDGET(dialog));
			
			g_list_free(list_of_valid);
			
			return;
		}

		g_assert(list_of_valid);

		if (list_of_valid->next) {
			/* There's more than one valid importer... */
			CONG_DO_UNIMPLEMENTED_DIALOG(_("More than one importer can handle that file type; the selection dialog has yet to be implemented.  You will have to use the first one that the plugin manager found."));

			importer = list_of_valid->data;
		} else {
			importer = list_of_valid->data;
		}

		g_list_free(list_of_valid);

		g_assert(importer);
		
		cong_importer_invoke(importer, filename, mime_type);
	}

}

static void add_exporter_to_menu(CongExporter *exporter, gpointer user_data)
{
	/* FIXME: should check for an appropriate FPI */
	GtkWidget *menu = user_data;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),
			      gtk_menu_item_new_with_label( cong_functionality_get_name(CONG_FUNCTIONALITY(exporter))));

}


static void on_exporter_selection_changed(GtkOptionMenu *optionmenu,
					  gpointer user_data)
{
	GtkWidget* menu = gtk_option_menu_get_menu(optionmenu);

	g_message("on_exporter_selection_changed");
}

static GtkWidget *cong_document_export_dialog_new(CongDocument *doc)
{
	xmlDocPtr xml_doc;
	CongDispspec* ds;
	GtkWidget *dialog;
	CongDialogContent *content;
	CongDialogCategory *general_category;
	CongDialogCategory *exporter_category;
	gchar *filename, *title;
	GtkWidget *select_exporter_option_menu;
	GtkWidget *select_exporter_menu;

	g_return_val_if_fail(doc, NULL);

	xml_doc = cong_document_get_xml(doc);
	ds = cong_document_get_dispspec(doc);

	filename = cong_document_get_filename(doc);

	title = g_strdup_printf(_("Export \"%s\""), filename);

	dialog = gtk_dialog_new_with_buttons(_("Export"),
					     NULL,
					     0,
					     GTK_STOCK_CANCEL,
					     GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	gtk_container_set_border_width(GTK_CONTAINER(dialog), 12);

	content = cong_dialog_content_new(FALSE);
	general_category = cong_dialog_content_add_category(content, _("General"));

	select_exporter_option_menu = gtk_option_menu_new();
	select_exporter_menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(select_exporter_option_menu),
				 select_exporter_menu);

	cong_plugin_manager_for_each_exporter(the_globals.plugin_manager, add_exporter_to_menu, select_exporter_menu);
	gtk_option_menu_set_history(GTK_OPTION_MENU(select_exporter_option_menu),0);

	cong_dialog_category_add_field(general_category, _("File:"), make_uneditable_text("filename"));
	cong_dialog_category_add_field(general_category, _("Exporter:"), select_exporter_option_menu);

	exporter_category = cong_dialog_content_add_category(content, _("Export Options"));

#if 0
	g_signal_connect(select_exporter_option_menu,
			 "changed"
			 on_exporter_selection_changed,
			 NULL);
#endif

#if 0
	cong_dialog_category_add_field(exporter_category, _("Name"), make_uneditable_text(cong_dispspec_get_name(ds)));
	cong_dialog_category_add_field(exporter_category, _("Description"), make_uneditable_text(cong_dispspec_get_description(ds)));
#endif

	g_free(title);
	g_free(filename);

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   cong_dialog_content_get_widget(content));

	gtk_widget_show_all(dialog);

	return dialog;
}

static void menu_callback_file_export(gpointer callback_data,
				      guint callback_action,
				      GtkWidget *widget)
{
	GtkWidget *dialog;

	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = primary_window->doc;

	/* FIXME: this option should be disabled if there are no importers installed */

	g_return_if_fail(doc);

	dialog = cong_document_export_dialog_new(doc);

	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);

	/* FIXME: memory leaks */

}

static GtkWidget *cong_document_properties_dialog_new(CongDocument *doc)
{
	xmlDocPtr xml_doc;
	CongDispspec* ds;
	GtkWidget *dialog;
	CongDialogContent *content;
	CongDialogCategory *general_category;
	CongDialogCategory *doctype_category;
	gchar *filename, *path;

	g_return_val_if_fail(doc, NULL);

	xml_doc = cong_document_get_xml(doc);
	ds = cong_document_get_dispspec(doc);

	dialog = gtk_dialog_new_with_buttons(_("Properties"),
					     NULL,
					     0,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_ACCEPT,
					     NULL);

	gtk_container_set_border_width(GTK_CONTAINER(dialog), 12);

	content = cong_dialog_content_new(FALSE);
	general_category = cong_dialog_content_add_category(content, _("General"));
	doctype_category = cong_dialog_content_add_category(content, _("Type"));

	filename = cong_document_get_filename(doc);
	path = cong_document_get_parent_uri(doc);

	cong_dialog_category_add_field(general_category, _("Name"), make_uneditable_text(filename));
	cong_dialog_category_add_field(general_category, _("Location"), make_uneditable_text(path));
	cong_dialog_category_add_field(general_category, _("Modified"), make_uneditable_text(cong_document_is_modified(doc)?"Yes":"No"));

	cong_dialog_category_add_field(doctype_category, _("Name"), make_uneditable_text(cong_dispspec_get_name(ds)));
	cong_dialog_category_add_field(doctype_category, _("Description"), make_uneditable_text(cong_dispspec_get_description(ds)));

	g_free(filename);
	g_free(path);

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   cong_dialog_content_get_widget(content));

	gtk_widget_show_all(dialog);

	return dialog;
}

static void menu_callback_file_properties(gpointer callback_data,
					  guint callback_action,
					  GtkWidget *widget)
{
	GtkWidget *dialog;

	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc = primary_window->doc;

	g_return_if_fail(doc);

	dialog = cong_document_properties_dialog_new(doc);

	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);

	/* FIXME: memory leaks */
}

static void menu_callback_file_close(gpointer data, guint
				     callback_action, GtkWidget
				     *widget) {
	if (cong_primary_window_can_close((struct CongPrimaryWindow*)data)) {
		gtk_widget_destroy(((struct CongPrimaryWindow*)data)->window);
	}
}

static void menu_callback_file_quit(gpointer data, guint
				    callback_action, GtkWidget
				    *widget) {

	GList *current;
	gboolean canceled = FALSE;

	current = g_list_first(the_globals.primary_windows);

	while (current) {
		if (!cong_primary_window_can_close((struct CongPrimaryWindow*)(current->data))) {
			canceled = TRUE;
			break;
		}
		current = g_list_next(current);
	}
	
	if (!canceled) {
		/* FIXME: This is probably leaking memory by not
		   freeing the primary windows...*/
		gtk_main_quit();
	}
}



/* Callbacks for "Edit" menu: */
static void menu_callback_cut(gpointer callback_data,
			      guint callback_action,
			      GtkWidget *widget)
{
	dispatch_document_command(cong_document_cut, callback_data);
}

static void menu_callback_copy(gpointer callback_data,
			       guint callback_action,
			       GtkWidget *widget)
{
	dispatch_document_command(cong_document_copy, callback_data);
}

static void menu_callback_paste(gpointer callback_data,
				guint callback_action,
				GtkWidget *widget)
{
	dispatch_document_command2(cong_document_paste, callback_data, widget);
}

static void menu_callback_view_source(gpointer callback_data,
				guint callback_action,
				GtkWidget *widget)
{
	dispatch_document_command(cong_document_view_source, callback_data);
}

/* Callbacks for "Help" menu: */
static void menu_callback_about(gpointer callback_data,
				guint callback_action,
				GtkWidget *widget)
{
	GdkPixbuf *logo_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)ilogo_xpm);
	gchar* authors[] = {"Hans Petter Jansson", "David Malcolm", NULL};

	gchar* documenters[] = { NULL };

 	gchar* translator_credits = _("translator_credits");
  
 	GtkWidget *about = gnome_about_new(_("Conglomerate XML Editor"),
 					   PACKAGE_VERSION,
 					   _("(C) 1999 Hans Petter Jansson\n(C) 2002 David Malcolm"),
 					   _("Conglomerate will be a user-friendly XML editor for GNOME"),
 					   (const char **)authors,
 					   (const char **)documenters,
 					   strcmp(translator_credits, "translator_credits") != 0 ?
 						    translator_credits : NULL,
  					   logo_pixbuf);
	gdk_pixbuf_unref(logo_pixbuf);

	gtk_dialog_run(GTK_DIALOG(about));

}


/* The menus: */
static GtkItemFactoryEntry menu_items[] =
{
	{ N_("/_File"),             NULL, NULL, 0, "<Branch>" },
	{ N_("/File/_New..."),       NULL, menu_callback_file_new, 0, "<StockItem>", GTK_STOCK_NEW },
	{ N_("/File/_Open..."),      NULL, menu_callback_file_open, 0, "<StockItem>", GTK_STOCK_OPEN },
	{ N_("/File/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/File/_Save"),           "<control>S", menu_callback_file_save, 0, "<StockItem>", GTK_STOCK_SAVE },
	{ N_("/File/Save _As..."),     NULL, menu_callback_file_save_as, 0, "<StockItem>", GTK_STOCK_SAVE_AS },
	{ N_("/File/Sa_ve a Copy..."), "<shift><control>S", menu_callback_file_save_copy, 0, "<Item>" },
	{ N_("/File/_Revert"),         NULL, menu_callback_file_revert, 0, "<StockItem>", GTK_STOCK_REVERT_TO_SAVED },
	{ N_("/File/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/File/_Import..."),           NULL, menu_callback_file_import, 0, "<Item>" },
	{ N_("/File/_Export..."),           NULL, menu_callback_file_export, 0, "<Item>" },
	{ N_("/File/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/File/Proper_ties"),     NULL, menu_callback_file_properties, 0, "<StockItem>", GTK_STOCK_PROPERTIES },
	{ N_("/File/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/File/_Close"),         "<control>W", menu_callback_file_close, 0, "<StockItem>", GTK_STOCK_CLOSE },
	{ N_("/File/_Quit"),         "<control>Q", menu_callback_file_quit, 0, "<StockItem>", GTK_STOCK_QUIT },

	{ N_("/_Edit"),                 NULL, 0, 0, "<Branch>" },
	{ N_("/Edit/_Undo"),              "<control>Z", unimplemented_menu_item, 0, "<StockItem>", GTK_STOCK_UNDO },
	{ N_("/Edit/_Redo"),              "<shift><control>Z", unimplemented_menu_item, 0, "<StockItem>", GTK_STOCK_REDO },
	{ N_("/Edit/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/Edit/Cu_t"),              "<control>X", menu_callback_cut, 0, "<StockItem>", GTK_STOCK_CUT },
	{ N_("/Edit/_Copy"),             "<control>C", menu_callback_copy, 0, "<StockItem>", GTK_STOCK_COPY },
	{ N_("/Edit/_Paste"),            "<control>V", menu_callback_paste, 0, "<StockItem>", GTK_STOCK_PASTE },
	{ N_("/Edit/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/Edit/_Find..."),         "<control>F", unimplemented_menu_item, 0, "<StockItem>", GTK_STOCK_FIND },
	{ N_("/Edit/Find Ne_xt"),       "<control>G", unimplemented_menu_item, 0, "<Item>" },
	{ N_("/Edit/Find Pre_vious"),   "<shift><control>G", unimplemented_menu_item, 0, "<Item>" },
	{ N_("/Edit/R_eplace..."),      "<control>R", unimplemented_menu_item, 0, "<StockItem>", GTK_STOCK_FIND_AND_REPLACE },
	{ N_("/Edit/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/Edit/_Insert..."),       NULL, unimplemented_menu_item, 0, "<Item>" },
	{ N_("/Edit/"), NULL, NULL, 0, "<Separator>" },
	{ N_("/Edit/View _Source"),     NULL, menu_callback_view_source, 0, NULL },

	{ N_("/Tests"),                 NULL, NULL, 0, "<Branch>" },
	{ N_("/Tests/Open..."),         NULL, test_open_wrap, 0, NULL },
	{ N_("/Tests/Error"),           NULL, test_error_wrap, 0, NULL },
	{ N_("/Tests/Document Types"),  NULL, test_document_types_wrap, 0, NULL },
	{ N_("/Tests/Transform DocBook to HTML"),       NULL, menu_callback_test_transform_docbook_to_html, 0, NULL },
	{ N_("/Tests/Transform DocBook to XHTML"),       NULL, menu_callback_test_transform_docbook_to_xhtml, 0, NULL },
	{ N_("/Tests/Transform DocBook to HTML Help"),       NULL, menu_callback_test_transform_docbook_to_html_help, 0, NULL },
	{ N_("/Tests/Transform DocBook to Java Help"),       NULL, menu_callback_test_transform_docbook_to_javahelp, 0, NULL },
	{ N_("/Tests/Transform DocBook to FO"),       NULL, menu_callback_test_transform_docbook_to_fo, 0, NULL },
#if PRINT_TESTS
	{ N_("/Tests/Preview XSL:FO"),       NULL, menu_callback_test_preview_fo, 0, NULL },
#endif /* #if PRINT_TESTS */
	{ N_("/Tests/DTD"),             NULL, menu_callback_test_dtd, 0, NULL },
	{ N_("/Tests/Dialog"),             NULL, menu_callback_test_dialog, 0, NULL },
	{ N_("/_Help"),        NULL, NULL, 0, "<Branch>" },
	{ N_("/Help/_Contents"), "F1", unimplemented_menu_item, 0, "<StockItem>",GTK_STOCK_HELP },
	{ N_("/Help/_About"),    NULL, menu_callback_about, 0, "<StockItem>", GNOME_STOCK_ABOUT }

};

gint delete_event( GtkWidget *widget,
		   GdkEvent  *event,
		   gpointer   data )
{
	/* If the window can't close, return TRUE so the callback chain
	   ends here */
	return ! cong_primary_window_can_close((CongPrimaryWindow *)data);
}


void destroy( GtkWidget *widget,
	      gpointer   data )
{
	cong_primary_window_free((CongPrimaryWindow *)data);
	if (g_list_length(the_globals.primary_windows) == 0) {		
		gtk_main_quit();
	}
}

void cong_primary_window_make_gui(CongPrimaryWindow *primary_window)
{
	GtkWidget *w1, *w2;
	GdkColor gcol;
	GtkStyle *style;
	GtkItemFactory *item_factory;
	gchar *title, *filename;
	int i;

	g_assert(primary_window);

	gdk_rgb_init();

	if (primary_window->doc) {
		filename = cong_document_get_filename(primary_window->doc);
		title = g_strdup_printf("%s - %s", filename, _("Conglomerate XML Editor"));
	} else {
		title = g_strdup(_("Conglomerate XML Editor"));
	}

	/* --- Main window --- */
	primary_window->window = gnome_app_new(PACKAGE_NAME,
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
	gtk_item_factory_set_translate_func(item_factory, (GtkTranslateFunc) gettext, NULL, NULL);
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
#if USE_CONG_EDITOR_WIDGET
	g_return_if_fail(primary_window->cong_editor_widget);
#else
	g_return_if_fail(primary_window->cong_editor_view);
#endif

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

#if USE_CONG_EDITOR_WIDGET
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(primary_window->scroller), 
					      GTK_WIDGET(primary_window->cong_editor_widget));
	gtk_widget_show(primary_window->cong_editor_widget);
#else
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(primary_window->scroller), 
					      cong_editor_view_get_widget(primary_window->cong_editor_view));
#endif
	
	
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

#if !USE_CONG_EDITOR_WIDGET
	xv_style_r(primary_window->scroller, style);
#endif

	gtk_widget_show(primary_window->scroller);


	/* --- Main window -> vbox -> hbox --- */
	primary_window->status = gtk_statusbar_new();
	gnome_app_set_statusbar(GNOME_APP(primary_window->window), primary_window->status);

	/* --- Putting it together --- */

	primary_window->status_main_ctx = gtk_statusbar_get_context_id(GTK_STATUSBAR(primary_window->status), 
								       "Main");

	{
		gchar *status_text;

		if (primary_window->doc) {
			status_text = g_strdup(cong_dispspec_get_name( cong_document_get_dispspec(primary_window->doc) ));

			

		} else {

			status_text = g_strdup(_("Welcome to the much-too-early Conglomerate editor."));	

		}

		gtk_statusbar_push(GTK_STATUSBAR(primary_window->status), 
				   primary_window->status_main_ctx,
				   status_text);

		g_free(status_text);
	}

}


CongPrimaryWindow *cong_primary_window_new(CongDocument *doc)
{
	/* is it valid to have a NULL document? */

	CongPrimaryWindow *primary_window = g_new0(CongPrimaryWindow, 1);

	primary_window->doc = doc;
	cong_document_ref(doc);

	primary_window->cong_tree_view = cong_tree_view_new(doc);
#if USE_CONG_EDITOR_WIDGET
#if 0
	primary_window->cong_editor_widget = gtk_calendar_new();
#else
	primary_window->cong_editor_widget = cong_editor_widget_new(doc);
#endif
#else
	primary_window->cong_editor_view = cong_editor_view_new(doc);
#endif

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
#if !USE_CONG_EDITOR_WIDGET
	cong_editor_view_free(primary_window->cong_editor_view);
#endif
	cong_document_unref(primary_window->doc);

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
