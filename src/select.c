/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>

#include "global.h"

void cong_selection_start_from_curs(CongSelection *selection, CongCursor *curs)
{
	g_assert(selection!=NULL);
	g_assert(curs!=NULL);

	selection->xed = curs->xed;
	selection->x0 = selection->x1 = curs->x;
	selection->y0 = selection->y1 = curs->y;

	cong_location_copy(&selection->loc0, &curs->location);
	cong_location_copy(&selection->loc1, &curs->location);
}


void cong_selection_end_from_curs(CongSelection *selection, CongCursor *curs)
{
	g_assert(selection!=NULL);
	g_assert(curs!=NULL);

	selection->x1 = curs->x;
	selection->y1 = curs->y;
	cong_location_copy(&selection->loc1, &curs->location);
}


void cong_selection_draw(CongSelection *selection, CongCursor *curs)
{
	int x0, y0, x1, y1;
	GdkGC *gc;
	CongLayoutLine *l;

	g_assert(selection!=NULL);
	g_assert(curs!=NULL);

	if (!curs->xed) return;
	
	/* Determine selection's usefulness code */
	if (cong_location_exists(&selection->loc0) && cong_location_exists(&selection->loc1) &&
	    cong_location_parent(&selection->loc0) == cong_location_parent(&selection->loc1))
		gc = selection->gc_0;
	else
		gc = selection->gc_3;

	/* Find start/end x, y */

	if (selection->y0 == selection->y1)
	{
		y0 = selection->y0;
		y1 = selection->y1;
/*		y1 += f_asc + f_desc; */

		if (selection->x0 <= selection->x1)
		{
			x0 = selection->x0;
			x1 = selection->x1;
		}
		else
		{
			x0 = selection->x1;
			x1 = selection->x0;
		}
	}
	else if (selection->y0 < selection->y1)
	{
		y0 = selection->y0;
		y1 = selection->y1;
		x0 = selection->x0;
		x1 = selection->x1;
	}
	else
	{
		y0 = selection->y1;
		y1 = selection->y0;
		x0 = selection->x1;
		x1 = selection->x0;
	}

	/* Find first line */
#if 1
	l = cong_layout_cache_get_line_by_y_coord(&curs->xed->layout_cache, y0+1);
	/* FIXME: check the boundary conditions are exactly correct! */
#else
	for (l = curs->xed->lines->child; l && l->next; l = l->next)
	{
#ifndef RELEASE
		printf("[%d : %d]\n", y0, (int) *((int *) l->child->next->next->data));
#endif
		if ((int) *((int *) l->child->next->next->data) > y0)
		{
			/* Found the right line */
			break;
		}
	}
#endif

	/* Draw all lines but last */

	for (; y0 != y1 && l; )
	{
#ifndef RELEASE
		printf("D");
#endif
		y0 = cong_layout_line_get_prev(l) ? cong_layout_line_get_second_y(cong_layout_line_get_prev(l)) : 0;
		if (y0 == y1) break;
		gdk_draw_rectangle(curs->xed->p, gc, 1,
				   x0, y0, curs->xed->w->allocation.width - x0,
				   l ? cong_layout_line_get_second_y(l) - y0 :
				   curs->xed->w->allocation.height - y0);

		l = cong_layout_line_get_next(l);
		x0 = 0;
	}

	/* Draw last line */

#ifndef RELEASE
	printf("X");
#endif

#if 1
	gdk_draw_rectangle(curs->xed->p, gc, 1,
			   x0, y0, x1 - x0, 
			   l ? cong_layout_line_get_second_y(l) - y0 /* == y1 */ - (cong_layout_line_get_next(l) ? 4 : 0) :
			   curs->xed->w->allocation.height - y0);
#else	
	gdk_draw_rectangle(curs->xed->p, gc, 1,
			   x0, y0, x1 - x0, 
			   l ? (int) *((int *) l->child->next->next->data) - y0 /* == y1 */ - (l->next ? 4 : 0) :
			   curs->xed->w->allocation.height - y0);
#endif

#ifndef RELEASE
	printf("\n");
#endif
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
CongNodePtr cong_selection_reparent_all(CongSelection *selection, CongNodePtr p)
{
	CongLocation loc0, loc1;
	CongNodePtr n0, n1, n2;
	CongDocument *doc;

	g_return_val_if_fail(selection,NULL);
	g_return_val_if_fail(p,NULL);

	/* Validate selection */
	g_return_val_if_fail( cong_location_exists(&selection->loc0), NULL );
	g_return_val_if_fail( cong_location_exists(&selection->loc1), NULL );
	g_return_val_if_fail( cong_location_parent(&selection->loc0) == cong_location_parent(&selection->loc1), NULL);
	/* both must be children of the same parent to maintain proper nesting */

	g_assert(selection->xed);
	doc = selection->xed->doc;

	CONG_NODE_SELF_TEST(p);

	/* --- Processing for multiple nodes --- */

	if (selection->loc0.tt_loc != selection->loc1.tt_loc)
	{
		CongNodePtr prev_node;
	
		/* Selection is valid, now order first/last nodes */
		
		for (n0 = selection->loc0.tt_loc; n0 && n0 != selection->loc1.tt_loc; n0 = n0->next) ;
		
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

		if (loc0.char_loc && cong_node_type(loc0.tt_loc) == CONG_NODE_TYPE_TEXT)
		{
			prev_node = cong_location_xml_frag_data_nice_split2(doc, &loc0);
			g_assert(prev_node);

			loc0.tt_loc = selection->loc0.tt_loc = prev_node->next;
		} else {
			prev_node = loc0.tt_loc;
		}
		
		selection->loc0.char_loc = 0;

		/* prev_node holds the previous node */

		/* Position p within the tree: */
		if (prev_node) {
			cong_document_node_add_after(doc, p, prev_node);
			CONG_NODE_SELF_TEST(prev_node);
		} else {
			cong_document_node_set_parent(doc, p, loc0.tt_loc->parent);
		}

		/* Reparent, first & middle */
		for (n0 = loc0.tt_loc; n0 != loc1.tt_loc; n0 = n2) {
			n2 = n0->next;

			CONG_NODE_SELF_TEST(n0);
			CONG_NODE_SELF_TEST(p);

			cong_document_node_set_parent(doc, n0, p);			

			CONG_NODE_SELF_TEST(n0);
			CONG_NODE_SELF_TEST(p);
		}

		/* Split, last */

		if (loc1.char_loc && cong_node_type(loc1.tt_loc) == CONG_NODE_TYPE_TEXT)
		{
			loc1.tt_loc = cong_location_xml_frag_data_nice_split2(doc, &loc1);
			selection->loc1.tt_loc = loc1.tt_loc->next;
		}

		selection->loc1.char_loc = 0;

		/* Reparent, last */
		cong_document_node_set_parent(doc, loc1.tt_loc, p);
		
		return(prev_node);
	}

	/* --- Processing for single node (loc0.tt_loc == loc1.tt_loc) --- */

	else
	{
		/* Sort out the ordering: */
		if (selection->loc0.char_loc < selection->loc1.char_loc)
		{
			cong_location_copy(&loc0,&selection->loc0);
			cong_location_copy(&loc1,&selection->loc1);
		}
		else
		{
			cong_location_copy(&loc0,&selection->loc1);
			cong_location_copy(&loc1,&selection->loc0);
		}

		if (cong_node_type(loc0.tt_loc) == CONG_NODE_TYPE_TEXT)
		{
			if (loc0.char_loc == loc1.char_loc) return(0); /* The end is the beginning is the end */
			
			loc0.tt_loc = loc1.tt_loc = xml_frag_data_nice_split3(doc, loc0.tt_loc, loc0.char_loc, loc1.char_loc);
			selection->loc0.tt_loc = loc0.tt_loc;
			selection->loc1.tt_loc = loc0.tt_loc->next;
		}

		selection->loc0.char_loc = 0;
		selection->loc1.char_loc = 0;
		
		/* Position p where the selection was: */
		if (loc0.tt_loc->prev) {
			cong_document_node_add_after(doc, p, loc0.tt_loc->prev);
		} else {
			cong_document_node_set_parent(doc, p, loc0.tt_loc->parent);
		}
		/* Move the selection below p: */
		cong_document_node_set_parent(doc, selection->loc0.tt_loc, p);

		/* Return node before p's new position (I think): */
		return p->prev;
	}
}

void cong_selection_init(CongSelection *selection)
{
	GdkColor gcol;
	GtkWidget* window;

	g_assert(selection!=NULL);

	window = cong_gui_get_a_window();
	
	selection->gc_0 = gdk_gc_new(window->window);
	gdk_gc_copy(selection->gc_0, window->style->white_gc);
	col_to_gcol(&gcol, 0x00ffffd0);
	gdk_colormap_alloc_color(window->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(selection->gc_0, &gcol);

	selection->gc_1 = gdk_gc_new(window->window);
	gdk_gc_copy(selection->gc_1, window->style->white_gc);
	col_to_gcol(&gcol, 0x00ffffb0);
	gdk_colormap_alloc_color(window->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(selection->gc_1, &gcol);
	
	selection->gc_2 = gdk_gc_new(window->window);
	gdk_gc_copy(selection->gc_2, window->style->white_gc);
	col_to_gcol(&gcol, 0x00ffff90);
	gdk_colormap_alloc_color(window->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(selection->gc_2, &gcol);
	
	selection->gc_3 = gdk_gc_new(window->window);
	gdk_gc_copy(selection->gc_3, window->style->white_gc);
	col_to_gcol(&gcol, 0x00ffd0d0);
	gdk_colormap_alloc_color(window->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(selection->gc_3, &gcol);

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
