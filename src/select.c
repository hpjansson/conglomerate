/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>

#include <ttree.h>
#include <xml.h>
#include "global.h"

void selection_start_from_curs(struct selection* selection, struct curs* curs)
{
	g_assert(selection!=NULL);
	g_assert(curs!=NULL);

	selection->xed = curs->xed;
	selection->x0 = selection->x1 = curs->x;
	selection->y0 = selection->y1 = curs->y;

	cong_location_copy(&selection->loc0, &curs->location);
	cong_location_copy(&selection->loc1, &curs->location);
}


void selection_end_from_curs(struct selection* selection, struct curs* curs)
{
	g_assert(selection!=NULL);
	g_assert(curs!=NULL);

	selection->x1 = curs->x;
	selection->y1 = curs->y;
	cong_location_copy(&selection->loc1, &curs->location);
}


void selection_draw(struct selection* selection, struct curs* curs)
{
	int x0, y0, x1, y1;
	GdkGC *gc;
	TTREE *l;

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

	/* Draw all lines but last */

	for (; y0 != y1 && l; )
	{
#ifndef RELEASE
		printf("D");
#endif
		y0 = l->prev ? (int) *((int *) l->prev->child->next->next->data) : 0;
		if (y0 == y1) break;
		gdk_draw_rectangle(curs->xed->p, gc, 1,
											 x0, y0, curs->xed->w->allocation.width - x0,
											 l ? (int) *((int *) l->child->next->next->data) - y0 :
											    curs->xed->w->allocation.height - y0);
		l = l->next;
		x0 = 0;
	}

	/* Draw last line */

#ifndef RELEASE
	printf("X");
#endif
	
	gdk_draw_rectangle(curs->xed->p, gc, 1,
										 x0, y0, x1 - x0, 
										 l ? (int) *((int *) l->child->next->next->data) - y0 /* == y1 */ - (l->next ? 4 : 0) :
										     curs->xed->w->allocation.height - y0);

#ifndef RELEASE
	printf("\n");
#endif
}


/* Splits a data node in 3 and returns pointer to the middle one */
TTREE *xml_frag_data_nice_split3(TTREE *s, int c0, int c1)
{
	TTREE *dummy, *d1, *d2, *d3;
	UNUSED_VAR(TTREE *n0)
	UNUSED_VAR(TTREE *n1)
	int len1, len2, len3;

	if (xml_frag_type(s) != XML_DATA)
	{
#ifndef RELEASE
		printf("--- xml_frag_data_nice_split3() got a non-data node!\n");
#endif
		return(s);
	}
	
	/* Calculate segments */
	
	if (s->child->size < c1) c1 = s->child->size;
	if (c1 < c0) c1 = c0;
	
	len1 = c0;
	len2 = c1 - c0;
	len3 = s->child->size - c1;

	/* Make split representation */
	
	dummy = ttree_node_add(0, "d", 1);

	d1 = ttree_node_add(dummy, "data", 4);
	ttree_node_add(d1, xml_frag_data_nice(s), len1);
	d2 = ttree_node_add(dummy, "data", 4);
	ttree_node_add(d2, xml_frag_data_nice(s) + len1, len2);
	d3 = ttree_node_add(dummy, "data", 4);
	ttree_node_add(d3, xml_frag_data_nice(s) + len1 + len2, len3);

	dummy->child = 0;
	ttree_branch_remove(dummy);

	/* Link it in */

	if (s->prev) s->prev->next = d1;
	d1->prev = s->prev;
	d1->next = d2;
	d2->prev = d1;
	d2->next = d3;
	d3->prev = d2;
	d3->next = s->next;
	if (s->next) s->next->prev = d3;

	d1->parent = s->parent;
	d2->parent = s->parent;
	d3->parent = s->parent;

	if (!d1->prev && d1->parent) d1->parent->child = d1;

	/* Unlink old node */
	
	if (s->child)
	{
		if (s->child->data) free(s->child->data);
		free(s->child);
	}

	if (s->data) free(s->data);
	free(s);

	return(d2);
}


/* Splits a data node in 2 and returns pointer to first one */

TTREE *xml_frag_data_nice_split2(TTREE *s, int c)
{
	TTREE *dummy, *d;
	int len1, len2;

	/* Calculate segments */

	len1 = c;
	len2 = s->child->size - len1;

	if (!len1 && !len2)
	{
	  dummy = ttree_node_add(0, "d", 1);

	  d = ttree_node_add(dummy, "data", 4);
	  ttree_node_add(d, 0, 0);
		
		if (d->child->data) free(d->child->data);
		d->child->size = 0;
		d->child->data = malloc(1);
		*(d->child->data) = 0;
	
	  dummy->child = 0;
	  ttree_branch_remove(dummy);
	}
	else if (!len1)
	{
	  dummy = ttree_node_add(0, "d", 1);

	  d = ttree_node_add(dummy, "data", 4);
	  ttree_node_add(d, 0, 0);
		
		if (d->child->data) free(d->child->data);
		d->child->size = 0;
		d->child->data = malloc(1);
		*(d->child->data) = 0;
	
	  dummy->child = 0;
	  ttree_branch_remove(dummy);

		/* Link it in */
	
	  if (s->prev) s->prev->next = d;
	  d->next = s;
	  d->prev = s->prev;
	  s->prev = d;

	  d->parent = s->parent;
		return(d);
	}
	else if (!len2)
	{
	  dummy = ttree_node_add(0, "d", 1);

	  d = ttree_node_add(dummy, "data", 4);
	  ttree_node_add(d, 0, 0);
		
		if (d->child->data) free(d->child->data);
		d->child->size = 0;
		d->child->data = malloc(1);
		*(d->child->data) = 0;
	
	  dummy->child = 0;
	  ttree_branch_remove(dummy);
	}
	else
	{
	  /* Make split representation */

	  dummy = ttree_node_add(0, "d", 1);

	  d = ttree_node_add(dummy, "data", 4);
	  ttree_node_add(d, xml_frag_data_nice(s) + len1, len2);
	
	  dummy->child = 0;
	  ttree_branch_remove(dummy);

	  /* Shrink original node */
	
	  s->child->data = realloc(s->child->data, len1 + 1);
	  *(s->child->data + len1) = 0;
	  s->child->size = len1;
	}

	/* Link it in */
	
	if (s->next) s->next->prev = d;
	d->prev = s;
	d->next = s->next;
	s->next = d;

	d->parent = s->parent;

	return(s);
}


TTREE *selection_reparent_all(struct selection* selection, TTREE *p)
{
	cong_location loc0, loc1;
	TTREE *n0, *n1, *n2;
	UNUSED_VAR(int len)
	UNUSED_VAR(char *p_data)
	TTREE *p_node;

	g_assert(selection!=NULL);

	/* Validate selection */
	if (!(cong_location_exists(&selection->loc0) && cong_location_exists(&selection->loc1) &&
	      cong_location_parent(&selection->loc0) == cong_location_parent(&selection->loc1)))
		return(0);

	/* --- Processing for multiple nodes --- */

	if (selection->loc0.tt_loc != selection->loc1.tt_loc)
	{
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
	
		if (loc0.char_loc && xml_frag_type(loc0.tt_loc) == XML_DATA)
		{
			p_node = cong_location_xml_frag_data_nice_split2(&loc0);
			loc0.tt_loc = selection->loc0.tt_loc = p_node->next;
		} else {
			p_node = loc0.tt_loc;
		}
		
		selection->loc0.char_loc = 0;

		if (loc0.tt_loc->prev) loc0.tt_loc->prev->next = p;
		else if (loc0.tt_loc->parent) loc0.tt_loc->parent->child = p;

		p->prev = loc0.tt_loc->prev;
		p->parent = loc0.tt_loc->parent;

		/* Reparent, first & middle */

		for (n0 = loc0.tt_loc, n1 = 0; n0 != loc1.tt_loc; n0 = n2)
		{
			n2 = n0->next;
			
			n0->parent = p->child;
			n0->prev = n1;
			n0->next = 0;
			
			if (!p->child->child) p->child->child = n0;
			if (n0->prev) n0->prev->next = n0;
					
			n1 = n0;
		}

		/* Split, last */

		if (loc1.char_loc && xml_frag_type(loc1.tt_loc) == XML_DATA)
		{
			loc1.tt_loc = cong_location_xml_frag_data_nice_split2(&loc1);
			selection->loc1.tt_loc = loc1.tt_loc->next;
		}

		selection->loc1.char_loc = 0;

		/* Reparent, last */

		loc1.tt_loc->parent = p->child;
		loc1.tt_loc->prev = n1;
		n1->next = loc1.tt_loc;
		if (loc1.tt_loc->next) loc1.tt_loc->next->prev = p;
		p->next = loc1.tt_loc->next;
		loc1.tt_loc->next = 0;
		if (!p->child->child) p->child->child = loc1.tt_loc;

#ifndef RELEASE		
		if (p_node->parent->parent)
		{
			ttree_fsave(p_node->parent->parent, stdout);
		}
#endif
		
		return(p_node);
	}

	/* --- Processing for single node (loc0.tt_loc == loc1.tt_loc) --- */

	else
	{
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

		if (xml_frag_type(loc0.tt_loc) == XML_DATA)
		{
			if (loc0.char_loc == loc1.char_loc) return(0); /* The end is the beginning is the end */
			
			loc0.tt_loc = loc1.tt_loc = xml_frag_data_nice_split3(loc0.tt_loc, loc0.char_loc, loc1.char_loc);
			selection->loc0.tt_loc = loc0.tt_loc;
			selection->loc1.tt_loc = loc0.tt_loc->next;
		}

		selection->loc0.char_loc = 0;
		selection->loc1.char_loc = 0;
		
		if (loc0.tt_loc->prev) loc0.tt_loc->prev->next = p;
		else if (loc0.tt_loc->parent) loc0.tt_loc->parent->child = p;

		if (loc0.tt_loc->next) loc0.tt_loc->next->prev = p;
		p->parent = loc0.tt_loc->parent;
		p->next = loc0.tt_loc->next;
		p->prev = loc0.tt_loc->prev;
		p->child->child = loc0.tt_loc;
		loc0.tt_loc->parent = p->child;
		loc0.tt_loc->next = 0;
		loc0.tt_loc->prev = 0;

#ifndef RELEASE		
		if (loc0.tt_loc->parent->parent->parent->parent)
		{
			ttree_fsave(loc0.tt_loc->parent->parent->parent, stdout);
		}
#endif
		
		return(loc0.tt_loc->parent->parent->prev);
	}
}

void selection_init(struct selection* selection)
{
	GdkColor gcol;
	GtkWidget* window;

	g_assert(selection!=NULL);

	window = cong_gui_get_window(&the_gui);
	
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


void selection_import(struct selection* selection)
{

	gtk_selection_convert(the_globals.curs.xed->w, GDK_SELECTION_PRIMARY,
												gdk_atom_intern("STRING", FALSE), GDK_CURRENT_TIME);

#ifndef RELEASE	
	printf("In selection_import().\n");
#endif
	
#if 0	
	gint gtk_selection_convert(GtkWidget *widget,
														 GdkAtom    selection,
														 GdkAtom    target,
														 guint32    time);
#endif
}


void selection_claim(struct selection* selection)
{

#if 0	
	gint gtk_selection_owner_set(GtkWidget *widget,
															 GdkAtom    selection,
															 guint32    time);


	
#endif
}
