/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-util.c
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
 * Fragments of code based upon libxslt: numbers.c
 */

#include "global.h"
#include "cong-util.h"
#include "cong-app.h"
#include "cong-document.h"
#include "cong-text-cache.h"

#include "cong-dispspec.h"
#include "cong-dispspec-element.h"

#include "cong-ui-hooks.h"

#include <libxml/globals.h>
#include <libxml/catalog.h>

#if ENABLE_PRINTING
#if 0
/* FIXME: use xmlroff eventually */
#include <libfo/fo-libfo.h>
#else
#include "fo.h"
#endif
#endif

#include "cong-attribute-editor.h"

GladeXML*
cong_util_load_glade_file (const gchar *filename,
			   const gchar *root,
			   CongDocument *doc,
			   CongNodePtr node)
{
	gchar* glade_filename;
	GladeXML *xml;
		
	g_return_val_if_fail (filename, NULL);
	if (doc || node) {
		g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	}

	glade_filename = gnome_program_locate_file (cong_app_get_gnome_program (cong_app_singleton()),
						    GNOME_FILE_DOMAIN_APP_DATADIR,
						    filename,
						    FALSE,
						    NULL);
	global_glade_doc_ptr = doc;
	global_glade_node_ptr = node;
	
	xml = glade_xml_new (glade_filename, 
			     root, 
			     NULL);
	glade_xml_signal_autoconnect(xml);
	
	global_glade_doc_ptr = NULL;
	global_glade_node_ptr = NULL;
	
	g_free(glade_filename);

	return xml;
}

gboolean 
cong_util_is_docbook (CongDocument *doc) 
{
	const CongXMLChar* dtd_public_id;

	g_return_val_if_fail(doc, FALSE);

	dtd_public_id = cong_document_get_dtd_public_identifier(doc);
	
	if (NULL==dtd_public_id) {
		return FALSE;
	}

	/* FIXME: we may want to add more public IDs here */
	if (0==strcmp(dtd_public_id, "-//OASIS//DTD DocBook XML V4.1.2//EN")) {
		return TRUE;
	} 

	/* FIXME:  this has been used by some of the GNOME docs; e.g. the GNOME accessibility guide; is it correct? */
	if (0==strcmp(dtd_public_id, "-//OASIS/DTD DocBookXML V4.1.2//EN")) {
		return TRUE;
	}


	return FALSE;
}

gboolean 
cong_util_is_pure_whitespace (const gchar *utf8_text)
{
	gunichar ch;

	g_return_val_if_fail(utf8_text, FALSE);

	while ( (ch = g_utf8_get_char(utf8_text)) ) {
		if (!g_unichar_isspace(ch)) {
			return FALSE;
		}

		utf8_text = g_utf8_next_char(utf8_text);
	}
	
	return TRUE;
}

gchar* 
cong_util_cleanup_text (const xmlChar *src_text) 
{
#if 0
	gchar *buffer;
	gchar *dst;
	gunichar unichar;
#endif

	g_return_val_if_fail(src_text, NULL);

	g_assert(g_utf8_validate(src_text, -1, NULL));

#if 1
	return g_strescape (src_text, "");
#else

	buffer = g_malloc((strlen(src_text)*6)+1); /* allow 6 bytes per character, plus a terminating byte; this SHOULD be big enough */
	dst = buffer;

	while (	(unichar = g_utf8_get_char(src_text)) ) {
		switch (unichar) {
		default:
			dst += g_unichar_to_utf8(unichar,dst);
			break;
		case '\n':
			*(dst++) = '\\';
			*(dst++) = 'n';
			break;
		case '\t':
			*(dst++) = '\\';
			*(dst++) = 't';
			break;
		}

		src_text = g_utf8_next_char(src_text);
	}

	*(dst++) = '\0';

	return buffer;
#endif
}

gchar* 
cong_util_text_header (const xmlChar *text,
		       guint truncation_length)
{
	CongTextCache* text_cache;
	const gchar* stripped_text;
	gchar *result;

	text_cache = cong_text_cache_new (TRUE,
					  text,
					  NULL);

	stripped_text = cong_text_cache_get_output_text (text_cache);

	if (g_utf8_strlen (stripped_text, -1)>truncation_length) {
		gchar *truncated = g_strndup (stripped_text, truncation_length);

		result = g_strdup_printf ("%s...", truncated);

		g_free (truncated);
	} else {
		result = g_strdup (stripped_text);
	}

	cong_text_cache_free (text_cache);

	return result;
}


gchar*
cong_utils_get_norman_walsh_stylesheet_path(void)
{
       /* This should be changed if another catalog is in use, i guess */
       xmlChar *resolved_path = NULL;

       resolved_path = xmlCatalogResolveURI ("http://docbook.sourceforge.net/release/xsl/current/");

       g_message ("Norman Walsh XSL path: %s", resolved_path);

       return resolved_path;
}

gchar*
cong_utils_get_norman_walsh_stylesheet(const gchar *stylesheet_relative_path)
{
	xmlChar *path;
	gchar *result;

	g_return_val_if_fail(stylesheet_relative_path, NULL);

	path = cong_utils_get_norman_walsh_stylesheet_path();

	result = g_strdup_printf("%s%s", path, stylesheet_relative_path);

	xmlFree (path);

	return result;
}

GdkPixbuf*
cong_util_load_icon (const gchar *icon_basename)
{
	gchar *filename;
	gchar *full_path;
	GdkPixbuf *pixbuf;

	g_return_val_if_fail(icon_basename, NULL);

	filename = g_strdup_printf("%s-16.png", icon_basename);
	full_path = gnome_program_locate_file (cong_app_get_gnome_program (cong_app_singleton()),
					       GNOME_FILE_DOMAIN_APP_PIXMAP,
					       filename,
					       FALSE,
					       NULL);
	pixbuf = gdk_pixbuf_new_from_file(full_path, NULL);
	
	g_free(full_path);
	g_free(filename);

	return pixbuf;
}

void 
cong_util_append (gchar **string, 
		  const gchar *to_add)
{
	gchar *new_string;

	g_return_if_fail(string);
	g_return_if_fail(*string);
	g_return_if_fail(to_add);

	new_string = g_strdup_printf("%s%s", *string, to_add);
	g_free(*string);

	*string = new_string;
}

void 
cong_util_prepend (gchar **string, 
		  const gchar *to_add)
{
	gchar *new_string;

	g_return_if_fail(string);
	g_return_if_fail(*string);
	g_return_if_fail(to_add);

	new_string = g_strdup_printf("%s%s", to_add, *string);
	g_free(*string);

	*string = new_string;
}

#if (ENABLE_PRINTING && ENABLED_LIBFO)
void 
cong_util_print_xslfo (GtkWindow *toplevel_window, 
		       GnomePrintContext *gpc,
		       xmlDocPtr xml_doc)
{
	g_return_if_fail(gpc);
	g_return_if_fail(xml_doc);

#if 0
	{
		/* FIXME: ultimately we probably want to use xmlroff to do this stage */
		FoDoc *fo_doc;
		FoFo *fo_tree;
		FoArea *area_tree;
		GError *error = NULL;

		fo_doc = fo_doc_gp_new ();

		/* FIXME:  need some way to pass our gpc or job to the FoDoc constructor; would a config do it? */

		fo_xml_doc_to_fo_and_area_trees (xml_doc,
						 fo_doc,
						 &fo_tree,
						 &area_tree,
						 0, /* gint debug_level */,
						 &error);

		/* FIXME: error handling! */
	}
#else

	CONG_DO_UNIMPLEMENTED_DIALOG_WITH_BUGZILLA_ID(toplevel_window, _("Printing XSL Formatting Objects"), 108468);

#if 0
	{
		FoPrintContext *fpc;
		FoParserResult *parser_result;
		FoSolverResult *solver_result;
		
		fpc = fo_print_context_new_from_gnome_print(gpc);
		
		parser_result = fo_parser_result_new_from_xmldoc(xml_doc);
		
		if (parser_result) {
			
#if 1
			/* View solver result: */
			solver_result = fo_solver_result_new_from_parser_result(parser_result);
			
			if (solver_result) {
				fo_solver_result_render(solver_result, fpc);
				
				fo_solver_result_delete(solver_result);
			}
#else
			/* View parser result: */
			fo_parser_result_test_render(parser_result, fpc);
#endif
			
			fo_parser_result_delete(parser_result);
			
		}
		
		fo_print_context_delete(fpc);
	}
#else
	/* Some test code: */
	{
		GnomeFont *font;
		font = gnome_font_find_closest ("Helvetica", 12);
		
		gnome_print_beginpage (gpc, "1");
		
		gnome_print_setfont (gpc, font);
		gnome_print_moveto (gpc, 100, 400);
		gnome_print_show (gpc, _("This will eventually be the result from cong_util_print_xslfo"));
		
		gnome_print_moveto (gpc, 100, 200);
		gnome_print_lineto (gpc, 200, 200);
		gnome_print_stroke (gpc);
		
		gnome_print_showpage (gpc);
	}
#endif
#endif


}
#endif

/**
   Make DTD declaration, and assigns it to the given document.  Doesn't add it to doc tree.
 */
xmlDtdPtr
cong_util_make_dtd (xmlDocPtr xml_doc,
		    const xmlChar *root_element,
		    const xmlChar *ExternalID, 
		    const xmlChar *SystemID)
{
#if 1
	xmlDtdPtr  dtd_ptr;

	g_message ("Trying to load DTD with PUBLIC \"%s\" SYSTEM \"%s\"", ExternalID, SystemID);

	dtd_ptr = xmlParseDTD (ExternalID, 
			       SystemID);
	
	if (dtd_ptr) {
		g_message ("Succeeded");

		/* Then set the document and the root_element: */
		cong_node_recursive_set_doc ((CongNodePtr)dtd_ptr, 
					     xml_doc);

		if (dtd_ptr->name) {
			xmlFree ((char*)dtd_ptr->name);
		}
		dtd_ptr->name = xmlStrdup(root_element);

		/* Set up ptr within the xml_doc: */
		xml_doc->extSubset = dtd_ptr;

		/* Nullify the tree descendants; we have to do this otherwise the entire DTD is in the tree and appears in the editor view etc etc (and is very slow for large DTDs): */
		dtd_ptr->children = NULL;
		dtd_ptr->last = NULL;

		return dtd_ptr;
	} else {
		g_message ("Failed");

		return NULL;
	}
#else
	/* But this does not actually load the DTDs... */
	return xmlNewDtd(xml_doc,
			 root_element,
			 ExternalID,
			 SystemID);
#endif
}	

/**
   Make DTD declaration, assigns it to the given document, and add it to the tree.
*/
xmlDtdPtr 
cong_util_add_external_dtd (xmlDocPtr xml_doc, 
			    const xmlChar *root_element,
			    const xmlChar *ExternalID, 
			    const xmlChar *SystemID)
{
	xmlDtdPtr xml_dtd;

	g_return_val_if_fail(xml_doc, NULL);
	g_return_val_if_fail(root_element, NULL);

	xml_dtd = cong_util_make_dtd (xml_doc,
				      root_element,
				      ExternalID, 
				      SystemID);

	if (xml_dtd) {
		if (xml_doc->children) {
			/* FIXME: what if there already is a DTD? */
			xmlAddPrevSibling((xmlNodePtr)xml_doc->children,
					  (xmlNodePtr)xml_dtd);
		} else {
			xmlAddChild((xmlNodePtr)xml_doc,
				    (xmlNodePtr)xml_dtd);
		}

		/* Ensure the DTD ptr is still set up within the xml_doc; the tree manipulation seems to make it lose the extSubset pointer: */
		xml_doc->extSubset = xml_dtd;
	}

	return xml_dtd;
}

/* Dodgy blend func: */
static void blend_col(GdkColor *dst, const GdkColor *src0, const GdkColor *src1, float proportion)
{
	float one_minus = 1.0f - proportion;

	dst->red = ((float)src1->red*proportion) + ((float)src0->red*one_minus);
	dst->green = ((float)src1->green*proportion) + ((float)src0->green*one_minus);
	dst->blue = ((float)src1->blue*proportion) + ((float)src0->blue*one_minus);
}

static GdkColor white = {0, 0xffff, 0xffff, 0xffff};

/* Dodgy hack to do lines that blend to white: */
void 
cong_util_draw_blended_line (GtkWidget *w,
			     const GdkColor *col,
			     int x0, int y0,
			     int x1)
{
	/* inefficient: claim a gc to do this! (ideally we'd just invoke the hardware... grrr... arrg...) */
	GdkGC *gc = gdk_gc_new(w->window);
	int x;
	float proportion;
	GdkColor blended_col;

	g_assert(x0!=x1);

	for (x=x0; x<x1; x++) {
		proportion = (float)(x-x0)/(float)(x1-x0);
		proportion = (proportion>0.5f)?((proportion-0.5f)*2.0f):0.0f;

		blend_col(&blended_col, col, &white, proportion);

		gdk_colormap_alloc_color(cong_gui_get_a_window()->style->colormap, &blended_col, FALSE, TRUE);

		gdk_gc_set_foreground(gc,&blended_col);

		gdk_draw_point(w->window, gc, x, y0);
	}

	gdk_gc_unref(gc);
}

unsigned int
cong_util_get_int_from_rgb_hex (const gchar *string)
{
	unsigned int col;
	
	col = 0;
	while (*string) {
		gchar ch = *(string++);

		col <<= 4;

		if (g_ascii_isalpha (ch)) {
			col |= (g_ascii_tolower (ch) - 'a' + 10);
		} else if (g_ascii_isdigit (ch)) {
			col |= (ch - '0');
		}
	}
	
	return(col);
}


gboolean
cong_util_ns_equality (const xmlNs *xml_ns1,
		       const xmlNs *xml_ns2)
{
	if (xml_ns1 == NULL && xml_ns2 == NULL)
		return TRUE;

	if (xml_ns1 == NULL || xml_ns2 == NULL)
		return FALSE;

	return cong_util_ns_uri_equality (xml_ns1->href, 
					  xml_ns2->href);
}

gboolean
cong_util_ns_uri_equality (const gchar* uri0, 
			   const gchar* uri1)
{
	return 0==cong_util_ns_uri_sort_order (uri0, 
					       uri1);

}

gint
cong_util_ns_uri_sort_order (const gchar* uri0, 
			     const gchar* uri1)
{
	/* Names are the same; continue searching by namespace; order the NULL namespace before all others: */
	if (NULL == uri0) {
		if (NULL == uri1) {
			return 0;
		} else {
			return -1; /* a is less than b */
		}
	} else {
		/* "a" has non-NULL namespace: */
		if (NULL == uri1) {
			return 1; /* a is greater than b */
		} else {
			/* Both have non-NULL namespaces; order based on them: */
			/* FIXME: do we need a less strict comparison? */
			return strcmp(uri0, uri1);
		}
	}

}

gboolean
cong_util_attribute_value_equality (const gchar *value0,
				    const gchar *value1)
{
	if (value0) {
		if (value1) {
			return 0==strcmp (value0,value1);
		} else {
			return FALSE;
		}
	} else {
		if (value1) {
			return FALSE;
		} else {
			return TRUE;
		}
	}
}


CongElementDescription*
cong_element_description_new (const gchar *ns_uri,
			      const gchar *local_name)
{
	CongElementDescription *element_desc;

	g_return_val_if_fail (local_name, NULL);

	element_desc = g_new0 (CongElementDescription, 1);

	if (element_desc->ns_uri) {
		element_desc->ns_uri = g_strdup (ns_uri);
	}
	element_desc->local_name = g_strdup (local_name);

	return element_desc;
}

CongElementDescription*
cong_element_description_clone (const CongElementDescription *element_desc)
{
	CongElementDescription *new_element_desc;

	g_return_val_if_fail (element_desc, NULL);

	new_element_desc = g_new0 (CongElementDescription, 1);

	if (element_desc->ns_uri) {
		new_element_desc->ns_uri = g_strdup (element_desc->ns_uri);
	}
	new_element_desc->local_name = g_strdup (element_desc->local_name);

	return new_element_desc;
}

void
cong_element_description_free (CongElementDescription *element_desc)
{
	g_return_if_fail (element_desc);

	if (element_desc->ns_uri) {
		g_free (element_desc->ns_uri);
	}
	g_free (element_desc->local_name);
}

CongNodePtr
cong_element_description_make_node (const CongElementDescription *element_desc,
				    CongDocument *doc,
				    CongNodePtr ns_search_node)
{
	xmlNsPtr xml_ns;
	CongNodePtr new_node;
	
	g_return_val_if_fail (element_desc, NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	g_return_val_if_fail (ns_search_node, NULL);

	/* FIXME:  what if the namespace doesn't exist in the document yet? */
	xml_ns = cong_node_get_ns_for_uri (ns_search_node,
					   element_desc->ns_uri);
	new_node = cong_node_new_element (xml_ns,
					  element_desc->local_name,
					  doc);
	return new_node;	
}

CongDispspecElement*
cong_element_description_get_dispspec_element_for_doc (const CongElementDescription *element_desc,
						       CongDocument *doc)
{
	CongDispspec *ds;

	g_return_val_if_fail (element_desc, NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	
	ds = cong_document_get_dispspec (doc);
	g_assert (ds);

	return cong_element_description_get_dispspec_element_for_dispspec (element_desc,
									   ds);
}

CongDispspecElement*
cong_element_description_get_dispspec_element_for_dispspec (const CongElementDescription *element_desc,
							    CongDispspec *ds)
{
	g_return_val_if_fail (element_desc, NULL);
	g_return_val_if_fail (ds, NULL);

	return cong_dispspec_lookup_element (ds, 
					     element_desc->ns_uri, 
					     element_desc->local_name);
}

#if 0
gchar*
cong_element_description_get_qualified_name (const CongElementDescription *element_desc)
{
	g_return_val_if_fail (element_desc, NULL);

	return ;
}
#endif


void
cong_element_description_list_free (GList *list_of_element_desc)
{
	GList *iter;

	for (iter=list_of_element_desc; iter; iter=iter->next) {
		cong_element_description_free ((CongElementDescription*)iter->data);
	}
	g_list_free (list_of_element_desc);
}

enum {
	FIELD_ELEMENT_DESC_PTR,
	FIELD_DS_PIXBUF,
	FIELD_DS_USER_VISIBLE_NAME,
	FIELD_ELEMENT_LOCAL_NAME,

	NUM_FIELDS
};

gint
sort_func_ds_user_visible_name (GtkTreeModel *model,
				GtkTreeIter *a,
				GtkTreeIter *b,
				gpointer user_data)
{
	gint result;

	gchar *val_a = NULL;
	gchar *val_b = NULL;

	gtk_tree_model_get (model,
			    a,
			    FIELD_DS_USER_VISIBLE_NAME, &val_a, 
			    -1);
	gtk_tree_model_get (model,
			    b,
			    FIELD_DS_USER_VISIBLE_NAME, &val_b, 
			    -1);

	/* Sort all empty strings after non-empty strings: */
	if (0==strcmp (val_a, "")) {
		if (0==strcmp (val_b, "")) {
			result = 0;
		} else {
			result = 1;
		}
	} else {
		if (0==strcmp (val_b, "")) {
			result = -1;
		} else {
			result = strcmp (val_a, val_b);
		}
	}

	g_free (val_a);
	g_free (val_b);

	return result;
}

gint
sort_func_element_local_name (GtkTreeModel *model,
			      GtkTreeIter *a,
			      GtkTreeIter *b,
			      gpointer user_data)
{
	gint result;

	gchar *val_a = NULL;
	gchar *val_b = NULL;

	gtk_tree_model_get (model,
			    a,
			    FIELD_ELEMENT_LOCAL_NAME, &val_a, 
			    -1);
	gtk_tree_model_get (model,
			    b,
			    FIELD_ELEMENT_LOCAL_NAME, &val_b, 
			    -1);

	result = strcmp (val_a, val_b);

	g_free (val_a);
	g_free (val_b);

	return result;
}

static void
selection_changed_cb (GtkTreeSelection *selection,
		      gpointer user_data)
{
	GladeXML *xml = GLADE_XML (user_data);
	
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "okbutton"),
				  gtk_tree_selection_get_selected (selection, NULL, NULL));
}


CongElementDescription*
cong_util_modal_element_selection_dialog (const gchar *title, 
					  const gchar *description,
					  CongDocument *doc,
					  GList *elements)
{
	GladeXML *xml;
	GtkWidget *dialog, *label, *tree_view;
	GtkListStore *list_store;
	GtkTreeSelection *selection;
	CongElementDescription *result;

	g_return_val_if_fail (title, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);
	g_return_val_if_fail (elements, NULL);

	xml = cong_util_load_glade_file ("glade/string_selection_dialog.glade",
					 NULL,
					 doc,
					 NULL);
	dialog = glade_xml_get_widget (xml, "string_selection_dialog");
	label = glade_xml_get_widget (xml, "label");
	tree_view = glade_xml_get_widget (xml, "treeview1");
	
        gtk_window_set_title (GTK_WINDOW (dialog), 
			      title);
	gtk_label_set_text (GTK_LABEL (label), 
			    description);

	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "okbutton"),
				  FALSE);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (selection), "changed",
			  G_CALLBACK (selection_changed_cb),
			  xml);

	/* Set up list view columns */
	{
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *column;

		/* First column: */
		{
			column = gtk_tree_view_column_new ();
			renderer = gtk_cell_renderer_pixbuf_new ();
			gtk_tree_view_column_pack_start (column, renderer, FALSE);
			gtk_tree_view_column_set_attributes (column,
							     renderer,
							     "pixbuf", FIELD_DS_PIXBUF,
							     NULL);
			
			renderer = gtk_cell_renderer_text_new ();
			gtk_tree_view_column_pack_start (column, renderer, FALSE);
			gtk_tree_view_column_set_attributes (column,
							     renderer,
							     "text", FIELD_DS_USER_VISIBLE_NAME,
							     NULL);			

			gtk_tree_view_column_set_sort_column_id (column,
								 FIELD_DS_USER_VISIBLE_NAME);
			
			gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
						     column);
		}

		/* Second column: */
		{
			column = gtk_tree_view_column_new ();
			renderer = gtk_cell_renderer_text_new ();
			gtk_tree_view_column_pack_start (column, renderer, FALSE);
			gtk_tree_view_column_set_attributes (column,
							     renderer,
							     "text", FIELD_ELEMENT_LOCAL_NAME,
							     NULL);

			gtk_tree_view_column_set_sort_column_id (column,
								 FIELD_ELEMENT_LOCAL_NAME);
			
			gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
						     column);
		}
	}
		
	/* Set up and populate list model: */
	{
		GList *iter;

		list_store = gtk_list_store_new (NUM_FIELDS,
						 G_TYPE_POINTER,
						 GDK_TYPE_PIXBUF,
						 G_TYPE_STRING,
						 G_TYPE_STRING);

		gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (list_store),
						 FIELD_DS_USER_VISIBLE_NAME,
						 sort_func_ds_user_visible_name,
						 NULL,
						 NULL);
		gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (list_store),
						 FIELD_ELEMENT_LOCAL_NAME,
						 sort_func_element_local_name,
						 NULL,
						 NULL);

		for (iter = elements; iter; iter=iter->next) {
			CongElementDescription *element_desc = (CongElementDescription *)iter->data;
			GtkTreeIter iter;
			gchar *tagged_name;
			CongDispspecElement *ds_element = cong_element_description_get_dispspec_element_for_doc (element_desc,
														 doc);
			gtk_list_store_append (list_store, &iter);

			tagged_name = g_strdup_printf ("<%s>", element_desc->local_name);

			gtk_list_store_set (list_store, &iter,
					    FIELD_ELEMENT_DESC_PTR, element_desc,
					    FIELD_ELEMENT_LOCAL_NAME, tagged_name,
					    -1);
			g_free (tagged_name);

			if (ds_element) {
				GdkPixbuf* icon = cong_dispspec_element_get_icon (ds_element);

				gtk_list_store_set (list_store, &iter,
						    FIELD_DS_USER_VISIBLE_NAME, cong_dispspec_element_username(ds_element),
						    -1);

				if (icon) {
					gtk_list_store_set (list_store, &iter,
							    FIELD_DS_PIXBUF, icon,
							    -1);
					g_object_unref (G_OBJECT (icon));
				}
			} else {
				gtk_list_store_set (list_store, &iter,
						    FIELD_DS_USER_VISIBLE_NAME, "",
						    -1);
			}
		}

		gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view),
					 GTK_TREE_MODEL (list_store));
		g_object_unref (G_OBJECT (list_store));
	}

	gtk_dialog_run (GTK_DIALOG (dialog));

	/* Get selection: */
	{
		CongElementDescription *element_desc;
		GtkTreeIter iter;

		if (gtk_tree_selection_get_selected (selection, 
						     NULL, 
						     &iter)) {
			gtk_tree_model_get (GTK_TREE_MODEL (list_store), 
					    &iter, 
					    FIELD_ELEMENT_DESC_PTR, &element_desc, 
					    -1);

			/* need to clone: */
			result = cong_element_description_clone (element_desc);;

		} else {
			result = NULL;
		}
	}
				
	gtk_widget_destroy(GTK_WIDGET(dialog));
        g_object_unref(G_OBJECT(xml));

	return result;
}


GtkMenuItem* 
cong_util_make_menu_item (const gchar *label,
			  const gchar *tip,
			  GdkPixbuf *pixbuf)
{
	GtkWidget *item;

	g_return_val_if_fail(label, NULL);

	item = gtk_image_menu_item_new_with_label (label);

	if (pixbuf) {
		GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
		gtk_widget_show(image);
	}

	if (tip) {
		gtk_tooltips_set_tip (cong_app_get_tooltips (cong_app_singleton()),
				      GTK_WIDGET(item),
				      tip,
				      tip);
	}

	return GTK_MENU_ITEM(item);
}

GtkMenuItem* 
cong_util_make_stock_menu_item (const gchar *stock_id)
{
	g_return_val_if_fail (stock_id, NULL);

	return GTK_MENU_ITEM (gtk_image_menu_item_new_from_stock(stock_id,
								 NULL));
}

GtkMenuItem* 
cong_util_make_menu_item_for_dispspec_element (CongDispspecElement *element)
{
	GtkMenuItem *item;
	GdkPixbuf *pixbuf;
	const gchar *tip;

	g_return_val_if_fail (element, NULL);

	pixbuf = cong_dispspec_element_get_icon (element);
	tip = cong_dispspec_element_get_description (element);

	if (NULL==tip) {
		tip = _("(no description available)");
	}
	
	item = cong_util_make_menu_item (cong_dispspec_element_username (element),
					 tip,
					 pixbuf);
	if (pixbuf) {
		g_object_unref (G_OBJECT (pixbuf));
	}

	return item;
}

GtkMenuItem* 
cong_util_make_menu_item_for_element_desc (const CongElementDescription *element_desc,
					   CongDocument *doc)
{
	CongDispspecElement* ds_element;

	g_assert (element_desc);
	g_assert (IS_CONG_DOCUMENT (doc));

	ds_element = cong_element_description_get_dispspec_element_for_doc (element_desc,
									    doc);
	if (ds_element) {
		return cong_util_make_menu_item_for_dispspec_element (ds_element);
	} else {
		return cong_util_make_menu_item (element_desc->local_name,
						 NULL,
						 NULL);
	}
}

GtkWidget*
cong_util_add_menu_separator (GtkMenu *menu)
{
	GtkWidget *item = gtk_menu_item_new ();
	GtkWidget *w0 = gtk_hseparator_new ();
	gtk_container_add (GTK_CONTAINER (item), 
			   w0);
	gtk_menu_append (menu, 
			 item);
	gtk_widget_set_sensitive (item, 
				  FALSE);
	gtk_widget_show (w0);
	gtk_widget_show (item);

	return item;
}

char *
cong_util_get_qualified_attribute_name(const xmlNs *namespace,
				       const xmlChar *local_attribute_name)
{
	g_return_val_if_fail(local_attribute_name != NULL, NULL);

	if(namespace == NULL ||
	   namespace->prefix == NULL ||
	   namespace->prefix[0] == '\0')
		return g_strdup(local_attribute_name);

	return g_strdup_printf("%s:%s",
			       namespace->prefix,
			       local_attribute_name);
}
