/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include "global.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-app.h"

/* --- Cut/copy/paste --- */

void selection_cursor_unset(CongDocument *doc)
{
	CongCursor *cursor;
	CongSelection *selection;

	g_return_if_fail(doc);

	cursor = cong_document_get_cursor(doc);
	selection = cong_document_get_selection(doc);

#if 0
	cursor->set = 0;
#endif
	cong_location_nullify(&cursor->location);
	cong_location_nullify(&selection->loc0);
	cong_location_nullify(&selection->loc1);
}

void cong_document_cut_selection(CongDocument *doc)
{
	CongNodePtr t;
	CongSelection *selection;
	CongCursor *curs;

	g_return_if_fail(doc);

	selection = cong_document_get_selection(doc);
	curs = cong_document_get_cursor(doc);
	
	if (!cong_location_exists(&curs->location)) return;
	if (!(cong_location_exists(&selection->loc0) && cong_location_exists(&selection->loc1) &&
				cong_location_parent(&selection->loc0) == cong_location_parent(&selection->loc1))) return;

	if (cong_location_equals(&selection->loc0, &selection->loc1)) return;
	
	if (cong_app_singleton()->clipboard) {
		cong_document_node_recursive_delete (NULL, 
						     cong_app_singleton()->clipboard);
	}
	
	t = cong_node_new_element(NULL, "dummy", doc);

	cong_selection_reparent_all(selection, doc, t);
	
	cong_document_node_make_orphan(doc, t);

	cong_app_singleton()->clipboard = t;

	selection_cursor_unset(doc);
}

void cong_document_copy_selection(CongDocument *doc)
{
	CongNodePtr t;
	CongNodePtr t0 = NULL;
	CongNodePtr t_next = NULL;
	int replace_xed = 0;

	CongSelection *selection;
	CongCursor *curs;

	g_return_if_fail(doc);

	selection = cong_document_get_selection(doc);
	curs = cong_document_get_cursor(doc);
	
	if (!cong_location_exists(&curs->location)) return;
	
	if (!(cong_location_exists(&selection->loc0) && cong_location_exists(&selection->loc1) &&
				cong_location_parent(&selection->loc0) == cong_location_parent(&selection->loc1))) return;

	if (cong_location_equals(&selection->loc0, &selection->loc1)) return;

	/* GREP FOR MVC */

	if (cong_app_singleton()->clipboard) {
		cong_document_node_recursive_delete (NULL, 
						     cong_app_singleton()->clipboard);
	}

	t = cong_node_new_element(NULL, "dummy", doc);

	cong_selection_reparent_all(selection, doc, t);
	cong_app_singleton()->clipboard = cong_node_recursive_dup(t);

	/* FIXME: doesn't this approach leave us with extra TEXT nodes abutting each other? */

	for (t0 = cong_node_first_child(t); t0; t0 = t_next) {
		t_next = t0->next;
		cong_document_node_add_before(doc, t0, t);
	}

	cong_document_node_make_orphan(doc,t);
	cong_node_free(t);

	selection_cursor_unset(doc);
}


void cong_document_paste_selection(CongDocument *doc, GtkWidget *widget)
{
	CongNodePtr t;
	CongNodePtr t0 = NULL;
	CongNodePtr t1 = NULL;
	CongNodePtr clip;
	CongNodePtr t_next;

	CongDispspec *ds;
	CongSelection *selection;
	CongCursor *curs;

	g_return_if_fail(doc);

	selection = cong_document_get_selection(doc);
	curs = cong_document_get_cursor(doc);
	
	if (!cong_location_exists(&curs->location)) return;
	ds = cong_document_get_dispspec(doc);

	/* GREP FOR MVC */

	if (!cong_app_singleton()->clipboard)
	{
		cong_selection_import(selection, widget);
		return;
	}

	if (!cong_app_singleton()->clipboard->children) return;
	
	if (cong_dispspec_element_structural(ds, cong_node_xmlns(cong_app_singleton()->clipboard), xml_frag_name_nice(cong_app_singleton()->clipboard))) return;
	
	if (cong_location_node_type(&curs->location) == CONG_NODE_TYPE_TEXT)
	{
		if (!curs->location.byte_offset)
		{
			t0 = cong_location_xml_frag_prev(&curs->location);
			t1 = cong_location_node(&curs->location);
		}
		else if (!cong_location_get_unichar(&curs->location))
		{
			t0 = cong_location_node(&curs->location);
			t1 = cong_location_xml_frag_next(&curs->location);
		}
		else
		{
			/* Split data node */
			cong_location_xml_frag_data_nice_split2(doc, &curs->location);

			curs->location.byte_offset = 0;
			t0 = cong_location_node(&curs->location);
			t1 = cong_location_xml_frag_next(&curs->location);
			if (cong_location_xml_frag_next(&curs->location)) {
				curs->location.node = cong_location_xml_frag_next(&curs->location);
			}
		}
	}
	else t0 = cong_location_node(&curs->location);


	/* FIXME:  does this leak "clip": */
	clip = cong_node_recursive_dup(cong_app_singleton()->clipboard);

	t = cong_node_first_child(clip);

	if (!t) return;
	
	for (; t; t = t_next) {
		t_next = t->next;
		cong_document_node_add_before(doc, t, t1);
	}

	cong_location_nullify(&selection->loc0);
	cong_location_nullify(&selection->loc1);
}

void 
cong_document_paste_text (CongDocument *doc, 
			  CongLocation *insert_loc, 
			  const gchar *source_fragment)
{
	CongNodePtr new_nodes; 
	CongNodePtr node_after;
	CongNodePtr iter, iter_next;

	g_return_if_fail (doc);
	g_return_if_fail (insert_loc);
	g_return_if_fail (source_fragment);
	g_return_if_fail (insert_loc->node);	

	new_nodes = cong_document_make_nodes_from_source_fragment (doc, 
								   source_fragment);
	cong_document_begin_edit (doc);

	/* We will add the children of new_node in place, then delete the placeholder parent, then merge adjacent text nodes if necessary. */

	/* Calculate insertion point, splitting text nodes if necessary: */
	if (cong_location_node_type(insert_loc) == CONG_NODE_TYPE_TEXT) {

		if (0==insert_loc->byte_offset) {
			node_after = insert_loc->node;
		} else if (!cong_location_get_unichar(insert_loc)) {
		        node_after = cong_location_xml_frag_next(insert_loc);
		} else {
			/* Split data node */
			cong_location_xml_frag_data_nice_split2(doc, insert_loc);
			
			node_after = cong_location_xml_frag_next(insert_loc);
		}
	} else {
		g_assert_not_reached();
	}

	/* Add the new nodes: */
	for (iter = new_nodes->children; iter; iter = iter_next) {
		iter_next = iter->next;

		cong_document_node_add_before(doc, iter, node_after);		
	}
	
	/* Delete the placeholder parent: */
	cong_document_node_recursive_delete (doc, 
					     new_nodes);

	/* Merge adjacent text nodes: */
	cong_document_merge_adjacent_text_children_of_node (doc, 
							    node_after->parent);

	cong_document_end_edit (doc);

}

extern char *ilogo_xpm[];

void cong_document_view_source(CongDocument *doc)
{
	GtkWidget *window;
	GtkWidget *source_view;

	g_return_if_fail(doc);

	window = gnome_app_new(PACKAGE_NAME,
			       _("Source View - Conglomerate"));
	source_view = cong_source_view_new(doc);

	gnome_app_set_contents(GNOME_APP(window), source_view);

	/* Set up the window nicely: */
	{	
		GdkPixbuf *icon_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)ilogo_xpm);
		
		gtk_window_set_icon(GTK_WINDOW(window),
				    icon_pixbuf);

		gdk_pixbuf_unref(icon_pixbuf);

	}

	gtk_window_set_default_size(GTK_WINDOW(window),
				    500,
				    400);

	gtk_widget_show(GTK_WIDGET(window));

}










