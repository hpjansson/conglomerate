/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-ui-hooks.h
 *
 * Copyright (C) 2004 David Malcolm
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
 */

#ifndef __CONG_UI_HOOKS_H__
#define __CONG_UI_HOOKS_H__

G_BEGIN_DECLS

/* Various callback types for UI actions: */
typedef void
(*CongUICallback_Document) (CongDocument *doc);

typedef void
(*CongUICallback_Document_Node_ParentWindow) (CongDocument *doc,
					      CongNodePtr node,
					      GtkWindow *parent_window);

typedef void 
(*CongUICallback_Document_DispspecElement_Node) (CongDocument *doc,
						 CongDispspecElement *ds_element,
						 CongNodePtr node);

/* Functions to hook up the callback types to a GtkMenuItem */
GtkMenuItem* 
cong_menu_item_attach_callback_Document (GtkMenuItem *item,
					 CongUICallback_Document callback,
					 CongDocument *doc);

GtkMenuItem* 
cong_menu_item_attach_callback_Document_Node_ParentWindow (GtkMenuItem *item,
							   CongUICallback_Document_Node_ParentWindow callback,
							   CongDocument *doc,
							   CongNodePtr callback_data,
							   GtkWindow *parent_window);

GtkMenuItem* 
cong_menu_item_attach_callback_Document_DispspecElement_Node (GtkMenuItem *item, 
							      CongUICallback_Document_DispspecElement_Node callback,
							      CongDocument *doc,
							      CongDispspecElement *ds_element,
							      CongNodePtr callback_data);
/*
 * Simple utility function.
 * Adds the item to the menu with appropriate sensitivity, and ensures that the item is "shown"
 */
GtkMenuItem* 
cong_menu_add_item (GtkMenu *menu,
		    GtkMenuItem *item,
		    gboolean is_sensitive);

/* UI Hooks: */
void 
tree_new_sibling (CongDocument *doc,
		  CongDispspecElement *ds_element,
		  CongNodePtr node);
void
tree_new_sub_element (CongDocument *doc,
		      CongDispspecElement *ds_element,
		      CongNodePtr node);

#if 0
void
tree_properties (CongDocument *doc,
		 CongNodePtr node,
		 GtkWindow *parent_window);
#else
gint tree_properties(GtkWidget *widget, CongNodePtr tag);
#endif
gint tree_cut(GtkWidget *widget, CongNodePtr tag);
gint tree_copy(GtkWidget *widget, CongNodePtr tag);
gint tree_paste_under(GtkWidget *widget, CongNodePtr tag);
gint tree_paste_before(GtkWidget *widget, CongNodePtr tag);
gint tree_paste_after(GtkWidget *widget, CongNodePtr tag);

/* Toolbar hooks: */
gint toolbar_callback_open(GtkWidget *widget, gpointer data);
gint toolbar_callback_new(GtkWidget *w, gpointer data);
gint toolbar_callback_save(GtkWidget *w, gpointer data);
gint toolbar_callback_cut(GtkWidget *w, gpointer data);
gint toolbar_callback_copy(GtkWidget *w, gpointer data);
gint toolbar_callback_paste(GtkWidget *w, gpointer data);

/* Menu hooks: */
void menu_callback_debug_error(gpointer callback_data,
			       guint callback_action,
			       GtkWidget *widget);
void menu_callback_debug_document_types(gpointer callback_data,
					guint callback_action,
					GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_html(gpointer callback_data,
						   guint callback_action,
						   GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_xhtml(gpointer callback_data,
						    guint callback_action,
						    GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_html_help(gpointer callback_data,
							guint callback_action,
							GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_javahelp(gpointer callback_data,
						       guint callback_action,
						       GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_fo(gpointer callback_data,
						 guint callback_action,
						 GtkWidget *widget);
void menu_callback_debug_preview_fo(gpointer callback_data,
				    guint callback_action,
				    GtkWidget *widget);

#if PRINT_TESTS
void cong_gnome_print_render_xslfo(xmlDocPtr xml_doc, GnomePrintMaster *gpm);
#endif
void menu_callback_debug_dtd(gpointer callback_data,
			    guint callback_action,
			    GtkWidget *widget);

void menu_callback_debug_dialog(gpointer callback_data,
				guint callback_action,
				GtkWidget *widget);

void cong_menus_create_items(GtkItemFactory *item_factory, 
			     CongPrimaryWindow *primary_window);

/* UI routines for invocation by menus/toolbars: */
void
cong_ui_file_import (GtkWindow *toplevel_window);

void
cong_ui_file_export (CongDocument *doc,
		     GtkWindow *toplevel_window);

#if ENABLE_PRINTING
void
cong_ui_file_print_preview (CongDocument *doc,
			    GtkWindow *toplevel_window);
void
cong_ui_file_print (CongDocument *doc,
		    GtkWindow *toplevel_window);
#endif


GtkWidget*
cong_file_properties_dialog_new (CongDocument *doc, 
				 GtkWindow *parent_window);


/* Popup (context) menus for editor view: */
void editor_popup_show(GtkWidget *widget, GdkEventButton *bevent);
void editor_popup_init();

/* Popup (context) menus for tree view and for section headings: */
GtkWidget* cong_ui_popup_init(CongDocument *doc, CongNodePtr node, GtkWindow *parent_window);


char *tag_new_pick();

void open_document(GtkWindow *parent_window);
gint save_document(CongDocument *doc, GtkWindow *parent_window);
gint save_document_as(CongDocument *doc, GtkWindow *parent_window);

char *pick_structural_tag(CongDispspec *ds);

void open_document_do(const gchar *doc_name, GtkWindow *parent_window);

void new_document(GtkWindow *parent_window);
int gui_window_new_document_make();

GtkWidget* cong_gui_get_a_window(void);

G_END_DECLS

#endif
