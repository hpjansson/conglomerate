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

typedef void 
(*CongUICallback_Document_ElementDescription_Node) (CongDocument *doc,
						    CongElementDescription *element_desc,
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
							   CongNodePtr node,
							   GtkWindow *parent_window);

GtkMenuItem* 
cong_menu_item_attach_callback_Document_DispspecElement_Node (GtkMenuItem *item, 
							      CongUICallback_Document_DispspecElement_Node callback,
							      CongDocument *doc,
							      CongDispspecElement *ds_element,
							      CongNodePtr node);

GtkMenuItem* 
cong_menu_item_attach_callback_Document_ElementDescription_Node (GtkMenuItem *item, 
								 CongUICallback_Document_ElementDescription_Node callback,
								 CongDocument *doc,
								 CongElementDescription *element_desc,
								 CongNodePtr node);
/*
 * Simple utility function.
 * Adds the item to the menu with appropriate sensitivity, and ensures that the item is "shown"
 */
GtkMenuItem* 
cong_menu_add_item (GtkMenu *menu,
		    GtkMenuItem *item,
		    gboolean is_sensitive);

/*
 *
 * UI routines for invocation by menus/toolbars: 
 *
 */

/** UI Hooks with clean marshalling: **/

/* File menu hooks: */
void
cong_ui_hook_file_import (GtkWindow *toplevel_window);

void
cong_ui_hook_file_export (CongDocument *doc,
			  GtkWindow *toplevel_window);

#if ENABLE_PRINTING
void
cong_ui_hook_file_print_preview (CongDocument *doc,
				 GtkWindow *toplevel_window);
void
cong_ui_hook_file_print (CongDocument *doc,
			 GtkWindow *toplevel_window);
#endif

/* Edit menu hooks: */
void
cong_ui_hook_edit_preferences (GtkWindow *toplevel_window);


/* Tree editing hooks: */
void 
cong_ui_hook_tree_new_sibling (CongDocument *doc,
			       CongElementDescription *element_desc,
			       CongNodePtr node);
void
cong_ui_hook_tree_new_sub_element (CongDocument *doc,
				   CongElementDescription *element_desc,
				   CongNodePtr node);
void
cong_ui_hook_tree_properties (CongDocument *doc,
			      CongNodePtr node,
			      GtkWindow *parent_window);
void 
cong_ui_hook_tree_cut (CongDocument *doc,
		       CongNodePtr node,
		       GtkWindow *parent_window);
void
cong_ui_hook_tree_copy (CongDocument *doc,
			CongNodePtr node,
			GtkWindow *parent_window);
void
cong_ui_hook_tree_paste_under (CongDocument *doc,
			       CongNodePtr node,
			       GtkWindow *parent_window);
void
cong_ui_hook_tree_paste_before (CongDocument *doc,
				CongNodePtr node,
				GtkWindow *parent_window);
void
cong_ui_hook_tree_paste_after (CongDocument *doc,
			       CongNodePtr node,
			       GtkWindow *parent_window);
void
cong_ui_hook_tree_convert_to_comment (CongDocument *doc,
				      CongNodePtr node,
				      GtkWindow *parent_window);
void
cong_ui_hook_tree_convert_from_comment (CongDocument *doc,
					CongNodePtr node,
					GtkWindow *parent_window);

/** Legacy UI Hooks without clean marshalling (to be cleanup up eventually): **/

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

GtkWidget*
cong_file_properties_dialog_new (CongDocument *doc, 
				 GtkWindow *parent_window);

G_END_DECLS

#endif
