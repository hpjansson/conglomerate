/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-overview-view.c
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

#include "global.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-dispspec-element.h"
#include "cong-tree-view.h"

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

GdkPixbuf* pixbuf_callback (CongTreeView *cong_tree_view,
			    CongNodePtr node,
			    gpointer user_data);

static void
on_doc_set_url (CongDocument *doc,
		const gchar *new_url,
		gpointer user_data);

static void
on_widget_destroy (GtkWidget *widget,
		   gpointer user_data);


typedef struct CongOverviewDetails CongOverviewDetails;

struct CongOverviewDetails
{
	CongTreeView *tree_view;
	CongDocument *doc;
	gulong sigid_set_url;
};

/* Exported function implementations: */
CongTreeView*
cong_overview_view_new (CongDocument *doc)
{
	CongOverviewDetails *overview_details;

	g_return_val_if_fail (doc, NULL);

	overview_details = g_new0 (CongOverviewDetails,1);
	
	overview_details->tree_view = cong_tree_view_new (doc,
							  FALSE,
							  node_filter,
							  node_creation_callback,
							  pixbuf_callback,
							  NULL);

	overview_details->doc = doc;
	g_object_ref (G_OBJECT (doc));
	overview_details->sigid_set_url = g_signal_connect_after (G_OBJECT (doc),
								  "set_url",
								  G_CALLBACK (on_doc_set_url),
								  overview_details);
	g_signal_connect_after (G_OBJECT (cong_tree_view_get_widget (overview_details->tree_view)),
				"destroy",
				G_CALLBACK (on_widget_destroy),
				overview_details);
	return overview_details->tree_view;
}

/* Internal function implementations: */
static gboolean 
node_filter (CongTreeView *cong_tree_view,
	     CongNodePtr node,
	     gpointer user_data)
{
	enum CongNodeType node_type;

	g_return_val_if_fail(cong_tree_view, FALSE);
	g_return_val_if_fail(node, FALSE);

	node_type = cong_node_type(node);

	switch (node_type) {
	default: g_assert_not_reached();

	case CONG_NODE_TYPE_UNKNOWN:
		return TRUE;

	case CONG_NODE_TYPE_ELEMENT:
		{
			CongDispspecElement* element = cong_dispspec_lookup_node(cong_view_get_dispspec(CONG_VIEW(cong_tree_view)), node);
			
			if (element) {
				switch (cong_dispspec_element_type(element)) {
				default: g_assert_not_reached();
				case CONG_ELEMENT_TYPE_STRUCTURAL: 
					return TRUE;

				case CONG_ELEMENT_TYPE_SPAN:
				case CONG_ELEMENT_TYPE_INSERT:						
				case CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE:
					return FALSE;
						
				case CONG_ELEMENT_TYPE_PLUGIN:
					return FALSE;
						
				case CONG_ELEMENT_TYPE_UNKNOWN:
					return TRUE;

				}
			} else {
				/* Unknown elements should be added: */
				return TRUE;
			}
		}
	case CONG_NODE_TYPE_ATTRIBUTE:
	case CONG_NODE_TYPE_TEXT:
	case CONG_NODE_TYPE_CDATA_SECTION:
	case CONG_NODE_TYPE_ENTITY_REF:
	case CONG_NODE_TYPE_ENTITY_NODE:
	case CONG_NODE_TYPE_PI:
	case CONG_NODE_TYPE_COMMENT:
		return FALSE;

	case CONG_NODE_TYPE_DOCUMENT:
		return TRUE;

	case CONG_NODE_TYPE_DOCUMENT_TYPE:
	case CONG_NODE_TYPE_DOCUMENT_FRAG:
	case CONG_NODE_TYPE_NOTATION:
	case CONG_NODE_TYPE_HTML_DOCUMENT:
	case CONG_NODE_TYPE_DTD:
	case CONG_NODE_TYPE_ELEMENT_DECL:
	case CONG_NODE_TYPE_ATRRIBUTE_DECL:
	case CONG_NODE_TYPE_ENTITY_DECL:
	case CONG_NODE_TYPE_NAMESPACE_DECL:
	case CONG_NODE_TYPE_XINCLUDE_START:
	case CONG_NODE_TYPE_XINCLUDE_END:
		return FALSE;
	}
}

static void 
node_creation_callback (CongTreeView *cong_tree_view,
			GtkTreeIter* tree_iter,
			CongNodePtr node,
			gpointer user_data)
{
	GtkTreeStore *gtk_tree_store;
	enum CongNodeType node_type;
	CongDocument *doc;
	CongDispspec *ds;

	g_return_if_fail (cong_tree_view);
	g_return_if_fail (tree_iter);
	g_return_if_fail (node);

	gtk_tree_store= cong_tree_view_protected_get_tree_store (cong_tree_view);

	node_type = cong_node_type(node);

	doc = cong_view_get_document (CONG_VIEW(cong_tree_view));
	ds = cong_view_get_dispspec (CONG_VIEW(cong_tree_view));

	switch (node_type) {
	default: g_assert_not_reached();
	case CONG_NODE_TYPE_UNKNOWN:
		{
			gtk_tree_store_set (gtk_tree_store, 
					    tree_iter,
					    CONG_TREE_VIEW_TREE_MODEL_TITLE_COLUMN, _("Unknown data"),
					    -1);
		}
		break;

	case CONG_NODE_TYPE_ELEMENT:
		{
			CongDispspecElement *element;
			gchar *text;

			element = cong_dispspec_lookup_node(ds, node);

			if (element) {
				const GdkColor *col;
				gchar *col_string;

				text = cong_dispspec_element_get_section_header_text(element, node);
      	
				gtk_tree_store_set (gtk_tree_store, 
						    tree_iter,
						    CONG_TREE_VIEW_TREE_MODEL_TITLE_COLUMN, text,
						    -1);
				g_free(text);
				/* FIXME:  this will fail to update when the text is edited */
#if NEW_LOOK
				col = cong_dispspec_element_col(element, CONG_DISPSPEC_GC_USAGE_TEXT);
				/* We hope this will contrast well against white */
#else
				col = cong_dispspec_element_col(element);
#endif

				col_string = get_col_string(col);
				
				gtk_tree_store_set (gtk_tree_store, 
						    tree_iter,
						    CONG_TREE_VIEW_TREE_MODEL_FOREGROUND_COLOR_COLUMN, col_string,
						    -1);
				
				g_free(col_string);
				
			} else {
				/* Use red for "tag not found" errors: */ 
				gchar *text;

				if (cong_node_get_ns_prefix (node)) {
					text= g_strdup_printf("<%s:%s>", cong_node_get_ns_prefix (node), node->name);
				} else {
					text= g_strdup_printf("<%s>", node->name);
				}

				gtk_tree_store_set (gtk_tree_store, 
						    tree_iter,
						    CONG_TREE_VIEW_TREE_MODEL_TITLE_COLUMN, text,
						    CONG_TREE_VIEW_TREE_MODEL_FOREGROUND_COLOR_COLUMN, "#ff0000", 
						    -1);
				g_free(text);
			}
		}
		break;

	case CONG_NODE_TYPE_DOCUMENT:
		{
			gchar *filename = cong_document_get_filename(doc);

			gtk_tree_store_set (gtk_tree_store, tree_iter,
					    CONG_TREE_VIEW_TREE_MODEL_TITLE_COLUMN, filename,
					    /* CONG_TREE_VIEW_TREE_MODEL_COLOR_COLUMN, g_strdup_printf("#305050"), */
					    -1);
			/* FIXME: What colour should the Document node be? */
			
			g_free(filename);
		}
		break;
	}
}

GdkPixbuf* pixbuf_callback (CongTreeView *cong_tree_view,
			    CongNodePtr node,
			    gpointer user_data)
{
	enum CongNodeType node_type;

	g_return_val_if_fail (cong_tree_view, NULL);
	g_return_val_if_fail (node, NULL);
	
	node_type = cong_node_type (node);

	switch (node_type) {
	default: g_assert_not_reached();
	case CONG_NODE_TYPE_UNKNOWN:
		{
			/* Empty for now */
		}
		break;

	case CONG_NODE_TYPE_ELEMENT:
		{
			CongDispspecElement *element;

			element = cong_document_get_dispspec_element_for_node (cong_view_get_document (CONG_VIEW (cong_tree_view)), node);

			if (element) {
				return cong_dispspec_element_get_icon (element);
			}
		}
		break;

	case CONG_NODE_TYPE_DOCUMENT:
		{
			/* Empty for now; FIXME: should get icon from mime-type */
		}
		break;
	}

	return NULL;
}

static void
on_doc_set_url (CongDocument *doc,
		const gchar *new_url,
		gpointer user_data)
{
	CongOverviewDetails *overview_details = (CongOverviewDetails*)user_data;
	g_assert (IS_CONG_DOCUMENT (doc));
	g_assert (new_url);

	/* Update root: */
	cong_tree_view_protected_force_node_update (overview_details->tree_view,
						    (CongNodePtr)cong_document_get_xml (doc));
}

static void
on_widget_destroy (GtkWidget *widget,
		   gpointer user_data)
{
	CongOverviewDetails *overview_details = (CongOverviewDetails*)user_data;

	g_signal_handler_disconnect (G_OBJECT (overview_details->doc),
				     overview_details->sigid_set_url);
	g_object_unref (G_OBJECT (overview_details->doc));

	g_free (overview_details);
}
