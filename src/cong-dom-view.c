/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-dom-view.c
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

#include <gtk/gtk.h>
#include "global.h"
#include "cong-document.h"
#include "cong-tree-view.h"
#include "cong-util.h"

/* Internal function prototypes: */
static gboolean 
node_filter (CongTreeView *cong_tree_view,
	     CongNodePtr node,
	     gpointer user_data);

static void 
node_creation_callback (CongTreeView *cong_tree_view,
			GtkTreeIter* tree_iter,
			CongNodePtr node,
			gpointer user_data);

static gchar*
clean_text(const gchar* txt);

const gchar*
cong_ui_get_colour_string(CongNodeType type);

static gchar*
get_text_for_node(CongNodePtr node);

/* Exported function implementations: */
/**
 * cong_dom_view_new:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget *
cong_dom_view_new (CongDocument *doc)
{
	CongTreeView *cong_tree_view;
	GtkScrolledWindow *scrolled_window;

	g_return_val_if_fail(doc, NULL);

	cong_tree_view = cong_tree_view_new (doc,
					     TRUE,
					     node_filter,
					     node_creation_callback,
					     NULL,
					     NULL				   
					     );

	
	scrolled_window = GTK_SCROLLED_WINDOW( gtk_scrolled_window_new(NULL, NULL) );
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), 
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(GTK_WIDGET(scrolled_window), 100, 50);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), 
					      GTK_WIDGET(cong_tree_view_get_widget(cong_tree_view)));

	gtk_widget_show_all(GTK_WIDGET(scrolled_window));

	return GTK_WIDGET(scrolled_window);	
}

/* Internal function implementations: */
static gboolean 
node_filter (CongTreeView *cong_tree_view,
	     CongNodePtr node,
	     gpointer user_data)
{
	g_return_val_if_fail(cong_tree_view, FALSE);
	g_return_val_if_fail(node, FALSE);

	return cong_node_should_recurse(node);
}

static void 
node_creation_callback (CongTreeView *cong_tree_view,
			GtkTreeIter* tree_iter,
			CongNodePtr node,
			gpointer user_data)
{
	GtkTreeStore *gtk_tree_store;
	gchar *text;

	g_return_if_fail (cong_tree_view);
	g_return_if_fail (tree_iter);
	g_return_if_fail (node);

	gtk_tree_store = cong_tree_view_protected_get_tree_store (cong_tree_view);

	text = get_text_for_node (node);
	g_assert(text);

	gtk_tree_store_set (gtk_tree_store, 
			    tree_iter,
			    CONG_TREE_VIEW_TREE_MODEL_TITLE_COLUMN, text,
			    -1);
	g_free (text);
}

static gchar*
clean_text(const gchar* txt) 
{
	gchar *cleaned;
	gchar *result;

	g_return_val_if_fail(txt, NULL);

	cleaned = cong_util_cleanup_text(txt);

	result = g_markup_escape_text(cleaned,
				      strlen(cleaned));

	g_free(cleaned);

	return result;
}

/**
 * cong_ui_get_colour_string:
 * @type:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_ui_get_colour_string(CongNodeType type)
{
	/* FIXME: this should be linked to the theme and/or the GtkSourceView settings */

	switch (type) {
	default: g_assert_not_reached();
	case CONG_NODE_TYPE_UNKNOWN:
		return "#000000";
	case CONG_NODE_TYPE_ELEMENT:
		return "#0080ff";
	case CONG_NODE_TYPE_ATTRIBUTE:
		return "#000000";
	case CONG_NODE_TYPE_TEXT:
		return "#ff0000";
	case CONG_NODE_TYPE_CDATA_SECTION:
		return "#000000";
	case CONG_NODE_TYPE_ENTITY_REF:
		return "#000000";
	case CONG_NODE_TYPE_ENTITY_NODE:
		return "#000000";
	case CONG_NODE_TYPE_PI:
		return "#000000";
	case CONG_NODE_TYPE_COMMENT:
		return "#0000FF";
	case CONG_NODE_TYPE_DOCUMENT:
		return "#0080ff";
	case CONG_NODE_TYPE_DOCUMENT_TYPE:
		return "#000000";
	case CONG_NODE_TYPE_DOCUMENT_FRAG:
		return "#000000";
	case CONG_NODE_TYPE_NOTATION:
		return "#000000";
	case CONG_NODE_TYPE_HTML_DOCUMENT:
		return "#000000";
	case CONG_NODE_TYPE_DTD:
		return "#0000FF";
	case CONG_NODE_TYPE_ELEMENT_DECL:
		return "#000000";
	case CONG_NODE_TYPE_ATRRIBUTE_DECL:
		return "#000000";
	case CONG_NODE_TYPE_ENTITY_DECL:
		return "#000000";
	case CONG_NODE_TYPE_NAMESPACE_DECL:
		return "#000000";
	case CONG_NODE_TYPE_XINCLUDE_START:
		return "#000000";
	case CONG_NODE_TYPE_XINCLUDE_END:
		return "#000000";
	}
}

static gchar*
get_text_for_node(CongNodePtr node)
{
	CongNodeType node_type;
	const gchar *colour_string;
	const gchar *string_colour_string;
	gchar *text = NULL;
	gchar *cleaned_text = NULL;


	node_type = cong_node_type(node);
	
	colour_string = cong_ui_get_colour_string(node_type);
	string_colour_string = "#00FF00"; /* FIXME: get this from the theme and/or GtkSourceView settings */

	switch (node_type) {
	default: g_assert_not_reached();
	case CONG_NODE_TYPE_UNKNOWN: 
		text = g_strdup(_("UNKNOWN"));
		break;
	case CONG_NODE_TYPE_ELEMENT: 
		{
			xmlAttrPtr attr_iter;
			const gchar* attname_col = cong_ui_get_colour_string(CONG_NODE_TYPE_ATTRIBUTE);
			const gchar* attval_col = string_colour_string;

			text = g_strdup_printf ("<span foreground=\"%s\">&lt;%s", 
						colour_string, 
						cong_node_get_qualified_name (node));
			
			/* Add attributes (if any): */
			for (attr_iter = node->properties; attr_iter; attr_iter=attr_iter->next) {
				gchar *attribute = g_strdup_printf(" <span foreground=\"%s\">%s=<span foreground=\"%s\">\"%s\"</span></span>",attname_col, attr_iter->name, attval_col, attr_iter->children->content);
				cong_util_append(&text, attribute);
				g_free(attribute);
			}
			cong_util_append(&text, "&gt;</span>");
		}
		break;
	case CONG_NODE_TYPE_ATTRIBUTE:
		text = g_strdup_printf("ATTRIBUTE");
		break;		
	case CONG_NODE_TYPE_TEXT:
		cleaned_text = clean_text(node->content);
		text = g_strdup_printf("%s \"<span foreground=\"%s\">%s</span>\"", 
				       _("Text:"), colour_string, cleaned_text);
		g_free(cleaned_text);
		break;
	case CONG_NODE_TYPE_CDATA_SECTION:
		text = g_strdup_printf("CDATA SECTION");
		break;
	case CONG_NODE_TYPE_ENTITY_REF:
		text = g_strdup_printf("ENTITY REF: %s", node->name);
		break;
	case CONG_NODE_TYPE_ENTITY_NODE:
		text = g_strdup_printf("ENTITY");
		break;
	case CONG_NODE_TYPE_PI:
		text = g_strdup_printf("PI");
		break;
	case CONG_NODE_TYPE_COMMENT:
		cleaned_text = clean_text(node->content);
		text = g_strdup_printf("<span foreground=\"%s\">&lt;!-- %s --&gt;</span>", colour_string, cleaned_text);
		g_free(cleaned_text);
		break;
	case CONG_NODE_TYPE_DOCUMENT:
		{
			xmlDocPtr doc = (xmlDocPtr)node;
			text = g_strdup_printf("<span foreground=\"%s\">&lt;?xml version="
					       "<span foreground=\"%s\">\"%s\"</span>"
					       " encoding="
					       "<span foreground=\"%s\">\"%s\"</span>"
					       " standalone="
					       "<span foreground=\"%s\">\"%s\"</span>"
					       "&gt;</span>",
					       
					       colour_string,

					       string_colour_string,
					       doc->version,

					       string_colour_string,
					       doc->encoding,

					       string_colour_string,
					       (doc->standalone?"yes":"no"));
		}
		break;
	case CONG_NODE_TYPE_DOCUMENT_TYPE:
		text = g_strdup_printf("DOCUMENT TYPE");
		break;
	case CONG_NODE_TYPE_DOCUMENT_FRAG:
		text = g_strdup_printf("DOCUMENT FRAG");
		break;
	case CONG_NODE_TYPE_NOTATION:
		text = g_strdup_printf("NOTATION");
		break;
	case CONG_NODE_TYPE_HTML_DOCUMENT:
		text = g_strdup_printf("HTML DOCUMENT");
		break;
	case CONG_NODE_TYPE_DTD:
		{
			text = g_strdup_printf("&lt;!DOCTYPE %s&gt;", node->name);
			/* FIXME: show any SYSTEM/PUBLIC DTD stuff */
		}
		break;
	case CONG_NODE_TYPE_ELEMENT_DECL:
		text = g_strdup_printf("ELEMENT DECL");
		break;
	case CONG_NODE_TYPE_ATRRIBUTE_DECL:
		text = g_strdup_printf("ATTRIBUTE DECL");
		break;
	case CONG_NODE_TYPE_ENTITY_DECL:
		text = g_strdup_printf("<span>&lt;!ENTITY %s ...&gt;</span>", node->name);
		break;
	case CONG_NODE_TYPE_NAMESPACE_DECL:
		text = g_strdup_printf("NAMESPACE DECL");
		break;
	case CONG_NODE_TYPE_XINCLUDE_START:
		text = g_strdup_printf("XINCLUDE START");
		break;
	case CONG_NODE_TYPE_XINCLUDE_END:
		text = g_strdup_printf("XINCLUDE END");
		break;		
	}

	return (text);
	
}
