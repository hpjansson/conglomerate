/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>

#include "global.h"
#include "cong-document.h"
#include "cong-error-dialog.h"

void cong_selection_start_from_curs(CongSelection *selection, CongCursor *curs)
{
	g_assert(selection!=NULL);
	g_assert(curs!=NULL);

	cong_location_copy(&selection->loc0, &curs->location);
	cong_location_copy(&selection->loc1, &curs->location);
}


void cong_selection_end_from_curs(CongSelection *selection, CongCursor *curs)
{
	g_assert(selection!=NULL);
	g_assert(curs!=NULL);

	cong_location_copy(&selection->loc1, &curs->location);
}

/* Splits a data node in 3 and returns pointer to the middle one */
CongNodePtr xml_frag_data_nice_split3(CongDocument *doc, CongNodePtr s, int c0, int c1)
{
	CongNodePtr d1, d2, d3;
	int len1, len2, len3;

	g_return_val_if_fail(cong_node_type(s) == CONG_NODE_TYPE_TEXT, NULL);

	CONG_NODE_SELF_TEST(s);
	
	/* Calculate segments */
	if (cong_node_get_length(s) < c1) c1 = cong_node_get_length(s);
	if (c1 < c0) c1 = c0;
	
	len1 = c0;
	len2 = c1 - c0;
	len3 = cong_node_get_length(s) - c1;

	/* Make split representation */
	d1 = cong_node_new_text_len(xml_frag_data_nice(s), len1); /* FIXME:  audit the char types here, and the char pointer arithmetic. UTF8? */
	d2 = cong_node_new_text_len(xml_frag_data_nice(s) + len1, len2);
	d3 = cong_node_new_text_len(xml_frag_data_nice(s) + len1 + len2, len3);

	/* Link it in */
	cong_document_node_add_after(doc, d1, s);
	cong_document_node_add_after(doc, d2, d1);
	cong_document_node_add_after(doc, d3, d2);
	cong_document_node_make_orphan(doc, s);

	/* Unlink old node */
	cong_node_free(s);

	CONG_NODE_SELF_TEST(d2);

	return(d2);
}


/* Splits a data node in 2 and returns pointer to first one */
CongNodePtr xml_frag_data_nice_split2(CongDocument *doc, CongNodePtr s, int c)
{
	CongNodePtr d = NULL;
	int len1, len2;

	CONG_NODE_SELF_TEST(s);

	/* Calculate segments */

	len1 = c;
	len2 = cong_node_get_length(s) - len1;

	if (!len1 && !len2) {
		d = cong_node_new_text("");
	} else if (!len1) {
		d = cong_node_new_text("");

		/* Link it in */
		cong_document_node_add_before(doc, d,s);
		return(d);
	} else if (!len2) {
		d = cong_node_new_text("");
	} else {
		xmlChar* new_text = g_strndup(s->content, len1); /* FIXME:  char type conversion? */

		/* Make split representation */
		d = cong_node_new_text_len(xml_frag_data_nice(s) + len1, len2); /* FIXME: check char ptr arithmetic; UTF8? */

		/* Shrink original node */
		cong_document_node_set_text(doc, s, new_text);

		g_free(new_text);
	}

	g_assert(d);

	/* Link it in */
	cong_document_node_add_after(doc, d, s);

	CONG_NODE_SELF_TEST(s);

	return(s);
}


/*
  DHM 22/10/2002:  This routine is used when applying a span to a selection.
  
  The selection is extracted (splitting text nodes at the front and rear if necessary), and then reparented below the second
  node, which is inserted into the position formerly occupied by the selection.
 */
CongNodePtr cong_selection_reparent_all(CongSelection *selection, CongDocument *doc, CongNodePtr p)
{
	CongLocation loc0, loc1;
	CongNodePtr n0, n1, n2;

	g_return_val_if_fail(selection,NULL);
	g_return_val_if_fail(doc,NULL);
	g_return_val_if_fail(p,NULL);

	/* Validate selection */
	g_return_val_if_fail( cong_location_exists(&selection->loc0), NULL );
	g_return_val_if_fail( cong_location_exists(&selection->loc1), NULL );
	g_return_val_if_fail( cong_location_parent(&selection->loc0) == cong_location_parent(&selection->loc1), NULL);
	/* both must be children of the same parent to maintain proper nesting */

	CONG_NODE_SELF_TEST(p);

	/* --- Processing for multiple nodes --- */

	if (selection->loc0.node != selection->loc1.node)
	{
		CongNodePtr prev_node;
	
		/* Selection is valid, now order first/last nodes */
		
		for (n0 = selection->loc0.node; n0 && n0 != selection->loc1.node; n0 = n0->next) ;
		
		if (!n0)
		{
			cong_location_copy(&loc0, &selection->loc1);
			cong_location_copy(&loc1, &selection->loc0);
		}
		else
		{
			cong_location_copy(&loc0, &selection->loc0);
			cong_location_copy(&loc1, &selection->loc1);
		}

		/* Split, first */

		if (loc0.byte_offset && cong_node_type(loc0.node) == CONG_NODE_TYPE_TEXT)
		{
			prev_node = cong_location_xml_frag_data_nice_split2(doc, &loc0);
			g_assert(prev_node);

			loc0.node = selection->loc0.node = prev_node->next;
		} else {
			prev_node = loc0.node;
		}
		
		selection->loc0.byte_offset = 0;

		/* prev_node holds the previous node */

		/* Position p within the tree: */
		if (prev_node) {
			cong_document_node_add_after(doc, p, prev_node);
			CONG_NODE_SELF_TEST(prev_node);
		} else {
			cong_document_node_set_parent(doc, p, loc0.node->parent);
		}

		/* Reparent, first & middle */
		for (n0 = loc0.node; n0 != loc1.node; n0 = n2) {
			n2 = n0->next;

			CONG_NODE_SELF_TEST(n0);
			CONG_NODE_SELF_TEST(p);

			cong_document_node_set_parent(doc, n0, p);			

			CONG_NODE_SELF_TEST(n0);
			CONG_NODE_SELF_TEST(p);
		}

		/* Split, last */

		if (loc1.byte_offset && cong_node_type(loc1.node) == CONG_NODE_TYPE_TEXT)
		{
			loc1.node = cong_location_xml_frag_data_nice_split2(doc, &loc1);
			selection->loc1.node = loc1.node->next;
		}

		selection->loc1.byte_offset = 0;

		/* Reparent, last */
		cong_document_node_set_parent(doc, loc1.node, p);
		
		return(prev_node);
	}

	/* --- Processing for single node (loc0.node == loc1.node) --- */

	else
	{
		/* Sort out the ordering: */
		if (selection->loc0.byte_offset < selection->loc1.byte_offset)
		{
			cong_location_copy(&loc0,&selection->loc0);
			cong_location_copy(&loc1,&selection->loc1);
		}
		else
		{
			cong_location_copy(&loc0,&selection->loc1);
			cong_location_copy(&loc1,&selection->loc0);
		}

		if (cong_node_type(loc0.node) == CONG_NODE_TYPE_TEXT)
		{
			if (loc0.byte_offset == loc1.byte_offset) return(0); /* The end is the beginning is the end */
			
			loc0.node = loc1.node = xml_frag_data_nice_split3(doc, loc0.node, loc0.byte_offset, loc1.byte_offset);
			selection->loc0.node = loc0.node;
			selection->loc1.node = loc0.node->next;
		}

		selection->loc0.byte_offset = 0;
		selection->loc1.byte_offset = 0;
		
		/* Position p where the selection was: */
		if (loc0.node->prev) {
			cong_document_node_add_after(doc, p, loc0.node->prev);
		} else {
			cong_document_node_set_parent(doc, p, loc0.node->parent);
		}
		/* Move the selection below p: */
		cong_document_node_set_parent(doc, selection->loc0.node, p);

		/* Return node before p's new position (I think): */
		return p->prev;
	}
}

void cong_selection_delete(CongSelection *selection, CongDocument *doc)
{
	/* FIXME: this code is fairly unstable */

	CongLocation loc0, loc1;
	CongNodePtr n0, n1, n2;

	g_return_if_fail(selection);
	g_return_if_fail(doc);

	/* Validate selection */
	g_return_if_fail( cong_location_exists(&selection->loc0) );
	g_return_if_fail( cong_location_exists(&selection->loc1) );
	g_return_if_fail( cong_location_parent(&selection->loc0) == cong_location_parent(&selection->loc1) );
	/* both must be children of the same parent to maintain proper nesting */

	/* --- Processing for multiple nodes --- */

	if (selection->loc0.node != selection->loc1.node)
	{
		CongNodePtr prev_node;
	
		/* Selection is valid, now order first/last nodes */
		
		for (n0 = selection->loc0.node; n0 && n0 != selection->loc1.node; n0 = n0->next) ;
		
		if (!n0)
		{
			cong_location_copy(&loc0, &selection->loc1);
			cong_location_copy(&loc1, &selection->loc0);
		}
		else
		{
			cong_location_copy(&loc0, &selection->loc0);
			cong_location_copy(&loc1, &selection->loc1);
		}

		/* Split, first */

		if (loc0.byte_offset && cong_node_type(loc0.node) == CONG_NODE_TYPE_TEXT)
		{
			prev_node = cong_location_xml_frag_data_nice_split2(doc, &loc0);
			g_assert(prev_node);

			loc0.node = selection->loc0.node = prev_node->next;
		} else {
			prev_node = loc0.node;
		}
		
		/* prev_node holds the previous node */

		cong_location_nullify(&selection->loc0);
		cong_location_nullify(&selection->loc1);

		/* Reparent, first & middle */
		for (n0 = loc0.node; n0 != loc1.node; n0 = n2) {
			n2 = n0->next;

			CONG_NODE_SELF_TEST(n0);

			xml_tag_remove(doc, n0);
		}

		/* Split, last */

		if (loc1.byte_offset && cong_node_type(loc1.node) == CONG_NODE_TYPE_TEXT)
		{
			loc1.node = cong_location_xml_frag_data_nice_split2(doc, &loc1);
		}

		/* Delete last */
		xml_tag_remove(doc, loc1.node);
	}

	/* --- Processing for single node (loc0.node == loc1.node) --- */

	else
	{
		/* Sort out the ordering: */
		if (selection->loc0.byte_offset < selection->loc1.byte_offset)
		{
			cong_location_copy(&loc0,&selection->loc0);
			cong_location_copy(&loc1,&selection->loc1);
		}
		else
		{
			cong_location_copy(&loc0,&selection->loc1);
			cong_location_copy(&loc1,&selection->loc0);
		}

		cong_location_nullify(&selection->loc0);
		cong_location_nullify(&selection->loc1);

		if (cong_node_type(loc0.node) == CONG_NODE_TYPE_TEXT)
		{
			if (loc0.byte_offset == loc1.byte_offset) return; /* The end is the beginning is the end */
			
			/* Split up textual content of node: */
#if 1
			CONG_DO_UNIMPLEMENTED_DIALOG(NULL, "Deletion of text within a single node");
#else
			/* what should happen to cursor? */
#endif
		} else {
			/* Delete entire node: */
#if 1
			CONG_DO_UNIMPLEMENTED_DIALOG(NULL, "Deletion of a single non-textual node");
#else
			/* what should happen to cursor? */
			xml_tag_remove(doc, loc0.node);
#endif
		}
	}


}

void cong_selection_init(CongSelection *selection)
{
	GdkColor gcol;
	GtkWidget* window;

	g_assert(selection!=NULL);

	window = cong_gui_get_a_window();
	
	selection->gc_valid = gdk_gc_new(window->window);
	gdk_gc_copy(selection->gc_valid, window->style->white_gc);
	col_to_gcol(&gcol, 0x00ffffd0);
	gdk_colormap_alloc_color(window->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(selection->gc_valid, &gcol);

	selection->gc_invalid = gdk_gc_new(window->window);
	gdk_gc_copy(selection->gc_invalid, window->style->white_gc);
	col_to_gcol(&gcol, 0x00ffd0d0);
	gdk_colormap_alloc_color(window->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(selection->gc_invalid, &gcol);

#if 0	
void gtk_selection_add_handler(GtkWidget            *widget, 
															 GdkAtom               selection,
															 GdkAtom               target,
															 GtkSelectionFunction  function,
															 GtkRemoveFunction     remove_func,
															 gpointer              data );
#endif
}


void cong_selection_import(CongSelection *selection, GtkWidget* widget)
{
	gtk_selection_convert(widget, GDK_SELECTION_PRIMARY,
			      gdk_atom_intern("STRING", FALSE), GDK_CURRENT_TIME);

#ifndef RELEASE	
	printf("In cong_selection_import().\n");
#endif
	
}


void cong_selection_claim(CongSelection *selection)
{

#if 0	
	gint gtk_selection_owner_set(GtkWidget *widget,
															 GdkAtom    selection,
															 guint32    time);


	
#endif
}
