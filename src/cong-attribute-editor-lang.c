/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-attribute-editor-lang.c
 *
 * Copyright (C) 2003 David Malcolm
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


#include <string.h>

#include "global.h"
#include "cong-attribute-editor-lang.h"
#include "cong-command.h"

#define PRIVATE(x) ((x)->private)

enum {
	COLUMN_NAME,
	COLUMN_CODE,
	NUM_COLUMNS
};

typedef struct _CongLanguage {
   char *code;
   char *name;
} CongLanguage;

#define KNOWN_LANGUAGES 23
static CongLanguage known_languages [KNOWN_LANGUAGES] = 
{
	{"br", N_("Breton")},
	{"ca", N_("Catalan")},
	{"cs", N_("Czech")},
	{"da", N_("Danish")},
	{"de_de", N_("German (Germany)")},
	{"de_ch", N_("German (Swiss)")},
	{"en_us", N_("English (American)")},
	{"en_gb", N_("English (British)")},
	{"en_ca", N_("English (Canadian)")},
	{"eo", N_("Esperanto")},
	{"es", N_("Spanish")},
	{"fo", N_("Faroese")},
	{"fr_fr", N_("French (France)")},
	{"fr_ch", N_("French (Swiss)")},
	{"it", N_("Italian")},
	{"nl", N_("Dutch")},
	{"no", N_("Norwegian")},
	{"pl", N_("Polish")},
	{"pt_pt", N_("Portuguese (Portugal)")},
	{"pt_br", N_("Portuguese (Brazilian)")},
	{"ru", N_("Russian")},
	{"sv", N_("Swedish")},
	{"uk", N_("Ukrainian")},
};

struct _CongAttributeEditorLangDetails
{
	GtkWidget *scrolled_window;
	GtkWidget *tree_view;
	GtkTreeModel *model;
	
	gulong handler_id_changed;
};

static void
cong_attribute_editor_lang_set_attribute_handler (CongAttributeEditor *attribute_editor);
static void
cong_attribute_editor_lang_remove_attribute_handler (CongAttributeEditor *attribute_editor);
static void
cong_attribute_editor_lang_do_refresh (CongAttributeEditorLang *attribute_editor_lang);
static void
on_selection_changed (GtkTreeSelection *selected, gpointer user_data);


/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongAttributeEditorLang, 
			cong_attribute_editor_lang,
			CongAttributeEditor,
			CONG_ATTRIBUTE_EDITOR_TYPE);

static void
cong_attribute_editor_lang_class_init (CongAttributeEditorLangClass *klass)
{
	CongAttributeEditorClass *editor_klass = CONG_ATTRIBUTE_EDITOR_CLASS (klass);

	editor_klass->set_attribute_handler = cong_attribute_editor_lang_set_attribute_handler;
	editor_klass->remove_attribute_handler = cong_attribute_editor_lang_remove_attribute_handler;
}

static void
cong_attribute_editor_lang_instance_init (CongAttributeEditorLang *attribute_editor)
{
	attribute_editor->private = g_new0(CongAttributeEditorLangDetails,1);
}

/**
 * cong_attribute_editor_lang_construct:
 * @attribute_editor_lang:
 * @doc:
 * @node:
 * @ns_ptr:
 * @attr:
 *
 * TODO: Write me
 * Returns:
 */
CongAttributeEditor*
cong_attribute_editor_lang_construct (CongAttributeEditorLang *attribute_editor_lang,
				       CongDocument *doc,
				       CongNodePtr node,
				       xmlNs *ns_ptr,
				       xmlAttributePtr attr)
{
	CongAttributeEditorLangDetails *details;

	int i;

	GtkTreeIter iter;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeSelection *selection;

	
	g_return_val_if_fail (IS_CONG_ATTRIBUTE_EDITOR_LANG(attribute_editor_lang), NULL);

        details = PRIVATE(attribute_editor_lang);
	
	cong_attribute_editor_construct (CONG_ATTRIBUTE_EDITOR(attribute_editor_lang),
					 doc,
					 node,
					 ns_ptr,
					 "lang",
					 attr);
	
	details->model = GTK_TREE_MODEL(gtk_list_store_new (NUM_COLUMNS,
							    G_TYPE_STRING,
							    G_TYPE_STRING));
    	

        gtk_list_store_append (GTK_LIST_STORE(details->model), &iter);
        gtk_list_store_set (GTK_LIST_STORE(details->model), &iter,
   		            COLUMN_NAME, _("No Language"), 
			    -1);
	for (i = 0; i < KNOWN_LANGUAGES; i++) {
	   gtk_list_store_append (GTK_LIST_STORE(details->model), &iter);
	   gtk_list_store_set (GTK_LIST_STORE(details->model), &iter,
			       COLUMN_NAME, known_languages[i].name, 
			       COLUMN_CODE, known_languages[i].code,
			       -1);
	}
							    
	details->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (details->scrolled_window), 
					GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	details->tree_view = gtk_tree_view_new_with_model (details->model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(details->tree_view), TRUE);
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(details->tree_view));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	details->handler_id_changed = g_signal_connect (G_OBJECT(selection), "changed", 
		          G_CALLBACK (on_selection_changed), attribute_editor_lang);

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes ("Language",
                                                     renderer,
                                                     "text",
                                                     COLUMN_NAME, NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (details->tree_view), column);

	gtk_container_add (GTK_CONTAINER(details->scrolled_window), details->tree_view);
	gtk_container_add (GTK_CONTAINER(attribute_editor_lang), details->scrolled_window);
	
        cong_attribute_editor_lang_do_refresh (attribute_editor_lang);
	
	return CONG_ATTRIBUTE_EDITOR (attribute_editor_lang);
}

/**
 * cong_attribute_editor_lang_new:
 * @doc: Valid document
 * @node: Cong Node of attribute
 * @ns_ptr: Pointer to xml namespace
 * @attr: Pointer to attribute
 *
 * TODO: Creates editor of language-based attribute
 * Returns:
 */
GtkWidget*
cong_attribute_editor_lang_new (CongDocument *doc,
				 CongNodePtr node,
				 xmlNs *ns_ptr,
				 xmlAttributePtr attr)
{
	return GTK_WIDGET( cong_attribute_editor_lang_construct
			   (g_object_new (CONG_ATTRIBUTE_EDITOR_LANG_TYPE, NULL),
			    doc,
			    node,
			    ns_ptr,
			    attr));			   
}

/* Internal function definitions: */
static void
cong_attribute_editor_lang_set_attribute_handler (CongAttributeEditor *attribute_editor)
{
	cong_attribute_editor_lang_do_refresh (CONG_ATTRIBUTE_EDITOR_LANG(attribute_editor));
}

static void
cong_attribute_editor_lang_remove_attribute_handler (CongAttributeEditor *attribute_editor)
{
	cong_attribute_editor_lang_do_refresh (CONG_ATTRIBUTE_EDITOR_LANG(attribute_editor));
}

static void
on_selection_changed (GtkTreeSelection *selection, gpointer user_data)
{
	gchar *value;
	GtkTreeModel *model;
	GtkTreeIter iter;
	
        if (gtk_tree_selection_get_selected (selection, &model, &iter)) {	

		gtk_tree_model_get (model, &iter, COLUMN_CODE, &value, -1);
		cong_attribute_editor_try_set_value (CONG_ATTRIBUTE_EDITOR(user_data), value);
		g_free (value);
        }
}


static void
cong_attribute_editor_lang_do_refresh (CongAttributeEditorLang *attribute_editor_lang)
{
	gchar *attr_value = cong_attribute_editor_get_attribute_value (CONG_ATTRIBUTE_EDITOR(attribute_editor_lang));
	gchar *value;

	GtkTreeSelection *selection;
        GtkTreeIter iter;
        GtkTreeModel *model;
        gboolean valid;
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(PRIVATE(attribute_editor_lang)->tree_view));

	gtk_tree_selection_unselect_all (selection);

        model = PRIVATE(attribute_editor_lang)->model;
		
        valid = gtk_tree_model_get_iter_first (model, &iter);
 
 	if (attr_value) {
	              
	       
	       /* Skip first row - no language */
	       
	       valid = gtk_tree_model_iter_next (model, &iter);

	       while (valid) {
		    gtk_tree_model_get (model, &iter, COLUMN_CODE, &value, -1);
		    
		    if (strcmp (value, attr_value) == 0) 
		        {
	  		  g_signal_handler_block ( G_OBJECT(selection),
					 PRIVATE(attribute_editor_lang)->handler_id_changed);
			  gtk_tree_selection_select_iter (selection, &iter);

	  		  g_signal_handler_unblock ( G_OBJECT(selection),
					 PRIVATE(attribute_editor_lang)->handler_id_changed);
			  g_free(value);
			  break;
			}
		    
		    g_free (value);
	    	    valid = gtk_tree_model_iter_next (model, &iter);
	       }
	} else {
   	       gtk_tree_selection_select_iter (selection, &iter);
	}

    return;
}

