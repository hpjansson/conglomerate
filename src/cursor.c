/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>

#include <ttree.h>
#include <xml.h>
#include "global.h"
#include <string.h>

void curs_on(struct curs* curs)
{
	g_assert(curs!=NULL);

	if (curs->w)
	  gdk_draw_line(curs->w->window, curs->gc, curs->x - 1 + 1, curs->y - 4,
								  curs->x - 1 + 1, curs->y + the_globals.f_asc + the_globals.f_desc + 4);
}


void curs_off(struct curs* curs)
{
	GdkRectangle r;

	g_assert(curs!=NULL);

	r.x = curs->x - 1 + 1;
	r.y = curs->y - 4;
	r.height = the_globals.f_asc + the_globals.f_desc + 9;
	r.width = 1;
	if (curs->w) gtk_widget_draw(curs->w, &r);
}


void print_lines(TTREE *l)
{
	int i = 0;
	
	l = l->child;
	
	for ( ; l; l = l->next, i++)
	{
#ifndef RELEASE
		printf("Line %d: y = %d, c = %d. Data: %s\n", i,
					 (int) *((int *) l->child->next->next->data),
					 (int) *((int *) l->child->next->data),
					 (char *) xml_frag_data_nice((TTREE *) *((TTREE **) l->child->data)) +
					 (int) *((int *) l->child->next->data));
#endif		
	}
}


void curs_place_in_xed(struct curs* curs, CongXMLEditor *xed, int x, int y)
{
	struct pos *pos0, *pos1;
	UNUSED_VAR(TTREE *l);
	UNUSED_VAR(int i);

	g_assert(curs!=NULL);

	curs->xed = xed;

	/* Remove from previous location */

	curs_off(curs);

	pos0 = pos_physical_to_logical(xed, x, y);
	pos1 = pos_logical_to_physical(xed, pos0->node, pos0->c);

#if 1
	cong_location_set(&curs->location, pos0->node, pos0->c);
#else
	curs->t = pos0->node;
	curs->c = pos0->c;
#endif
	free(pos0);

	curs->x = pos1->x;
	curs->y = pos1->y;
	curs->line = pos1->line;
	free(pos1);
	
	/* Draw immediately */

	curs_on(curs);
}


void curs_init(struct curs* curs)
{
	GdkColor gcol;

	g_assert(curs!=NULL);

	curs->w = 0;
	curs->x = curs->y = 0;
#if 1
	cong_location_set(&curs->location, NULL, 0);
#else
	curs->t = 0;
	curs->c = 0;
#endif

	gtk_timeout_add(500, curs_blink, 0);
	curs->gc = gdk_gc_new(cong_gui_get_window(&the_gui)->window);
	gdk_gc_copy(curs->gc, cong_gui_get_window(&the_gui)->style->black_gc);
	col_to_gcol(&gcol, 0x00ff8c00);
	gdk_colormap_alloc_color(cong_gui_get_window(&the_gui)->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(curs->gc, &gcol);
}


gint curs_blink(gpointer data)
{
	struct curs* curs = &the_globals.curs;

	if (!curs->w) return(TRUE);

	if (curs->on)
	{
		curs_off(curs);
		curs->on = 0;
	}
	else
	{
		curs_on(curs);
		curs->on = 1;
	}
	
	return(TRUE);
}

gint curs_data_insert(struct curs* curs, char *s)
{
	TTREE *n;
	int len;

	g_assert(curs!=NULL);

	len = strlen(s);

#if 1
	if (!cong_location_exists(&curs->location)) return(0);
	if (cong_location_frag_type(&curs->location) != XML_DATA) return(0);
#else
	if (!curs->t) return(0);
	if (xml_frag_type(curs->t) != XML_DATA) return(0);
#endif

#if 1
	cong_location_insert_chars(&curs->location, s);
#else
	n = curs->t->child;
	n->data = realloc(n->data, (n->size + 1) + len);

	memmove(n->data + curs->c + len, n->data + curs->c, (n->size + 1) - curs->c);
	memcpy(n->data + curs->c, s, len);
	n->size += len;
	curs->c += len;
#endif
	return(1);
}


int curs_paragraph_insert(struct curs* curs)
{
	TTREE *t, *dummy;
	CongDispspec *ds;
	CongDispspecElement *para;
	const char *tagname;

	g_assert(curs!=NULL);
	
	if (!curs->xed) return(0);

#if 1
	if (!cong_location_exists(&curs->location)) return(0);
	if (cong_location_frag_type(&curs->location) != XML_DATA) return(0);
#else	
	if (!curs->t) return(0);
	if (xml_frag_type(curs->t) != XML_DATA) return(0);
#endif

	g_assert(curs->xed);
	ds = curs->xed->displayspec;

	para = cong_dispspec_get_paragraph(ds);

	if (NULL==para) {
		/* The dispspec does not have a "paragraph" tag */
		return 0;
	}

	tagname = cong_dispspec_element_tagname(para);

#if 1
	t = cong_location_xml_frag_data_nice_split2(&curs->location);
	cong_location_set(&curs->location,t->next,0);
#else	
	t = xml_frag_data_nice_split2(curs->t, curs->c);
	curs->t = t->next;
	curs->c = 0;
#endif


	dummy = ttree_node_add(0, "dummy", 5);
	
	ttree_node_add(dummy, "tag_empty", 9);
	ttree_node_add(dummy->child, tagname, strlen(tagname));

	printf("inserting paragraph using tag <%s>\n",tagname);

	t->next->prev = dummy->child;
	dummy->child->next = t->next;
	dummy->child->prev = t;
	t->next = dummy->child;
	dummy->child->parent = t->parent;
	dummy->child = 0;
	ttree_branch_remove(dummy);
	return(1);
}


void curs_prev_char(struct curs* curs, CongXMLEditor *xed)
{
	TTREE *n, *n0;
	UNUSED_VAR(int c);

#ifndef RELEASE	
	printf("<- [curs]\n");
#endif

	g_assert(curs!=NULL);
	
	if (!curs->xed) return;

#if 1
	n = curs->location.tt_loc;
	if (cong_location_frag_type(&curs->location) == XML_DATA && curs->location.char_loc) { curs->location.char_loc--; return; }
#else
	n = curs->t;
	if (xml_frag_type(n) == XML_DATA && curs->c) { curs->c--; return; }
#endif

	do
	{
		n0 = n;
		if (n) n = xml_frag_prev(n);
		
		for ( ; n; )
		{
			if (xml_frag_type(n) == XML_DATA) break;
			else if (xml_frag_type(n) == XML_TAG_SPAN)
			{
				if (!strcmp(xml_frag_name(n), "table")) break;
				if (cong_dispspec_element_structural(xed->displayspec, xml_frag_name_nice(n)))
				{
					n = n0 = 0;
					break;
				}
				else if (xml_frag_enter(n))
				{
#ifndef RELEASE					
					printf("Entering tag: %s.\n", xml_frag_name_nice(n));
#endif					
					n = xml_frag_enter(n);
					continue;
				}
			}
			
			n0 = n;
			n = xml_frag_prev(n);
		}

		if (!n) n = n0;
		else if (xml_frag_type(n) == XML_DATA) break;

		while (n)
		{
			if (cong_dispspec_element_structural(xed->displayspec, xml_frag_name_nice(n))) { n = 0; break; }
			if (!xml_frag_prev(n)) n = n0 = xml_frag_exit(n);
			else break;
		}
	}
	while (n);
	
	
	if (n) {
	  cong_location_set(&curs->location,n, strlen(xml_frag_data_nice(n)));
	}
}


void curs_next_char(struct curs* curs, CongXMLEditor *xed)
{
	TTREE *n, *n0;
	UNUSED_VAR(int c);

#ifndef RELEASE	
	printf("[curs] ->\n");
#endif

	g_assert(curs!=NULL);
	
	if (!curs->xed) return;

#if 1
	n = curs->location.tt_loc;
	if (cong_location_frag_type(&curs->location) == XML_DATA && cong_location_get_char(&curs->location)!='\0')
	{ 
		curs->location.char_loc++; 
		return; 
	}
#else
	n = curs->t;
	if (xml_frag_type(n) == XML_DATA && *(xml_frag_data_nice(n) + curs->c))
	{ 
		curs->c++; 
		return; 
	}
#endif

	do
	{
		n0 = n;
		if (n) n = xml_frag_next(n);

		for ( ; n; )
		{
			if (xml_frag_type(n) == XML_DATA) break;
			else if (xml_frag_type(n) == XML_TAG_SPAN)
			{				 
				if (!strcmp(xml_frag_name(n), "table")) break;
				if (cong_dispspec_element_structural(xed->displayspec, xml_frag_name_nice(n)))
				{
					n = n0 = 0;
					break;
				}
				else if (xml_frag_enter(n))
				{
#ifndef RELEASE					
					printf("Entering tag: %s.\n", xml_frag_name_nice(n));
#endif
					n = xml_frag_enter(n);
					continue;
				}
			}
			
			n0 = n;
			n = xml_frag_next(n);
		}

		if (!n) n = n0;
		else if (xml_frag_type(n) == XML_DATA) break;

		while (n)
		{
			if (cong_dispspec_element_structural(xed->displayspec, xml_frag_name_nice(n))) { n = 0; break; }
			if (!xml_frag_next(n)) n = n0 = xml_frag_exit(n);
			else break;
		}
	}
	while (n);

	if (n)
	{
	  cong_location_set(&curs->location, n, 0);
	}
}


void curs_prev_line(struct curs* curs, CongXMLEditor *xed)
{
	struct pos *pos;
	TTREE *l;
	
	g_assert(curs!=NULL);
	
	if (curs->line == 0) return;
	
	curs->line--;

#if 1
	l = cong_layout_cache_get_line_by_index(&xed->layout_cache, curs->line);
#else
	for (i = 0, l = xed->lines->child; l && i < curs->line; i++, l = l->next) {
		/* empty */
	}
#endif

	if (!l) return;

	curs->y = (int) *((int *) l->child->next->next->data);
	pos = pos_physical_to_logical(xed, curs->x, curs->y);

#if 1
	cong_location_set(&curs->location, pos->node, pos->c);
#else
	curs->t = pos->node;
	curs->c = pos->c;
#endif
	free(pos);
}


void curs_next_line(struct curs* curs, CongXMLEditor *xed)
{
	struct pos *pos;
	TTREE *l;

	g_assert(curs!=NULL);

#if 1
	l = cong_layout_cache_get_line_by_index(&xed->layout_cache, curs->line);
#else
	for (i = 0, l = xed->lines->child; l && i < curs->line; i++, l = l->next) {
		/* empty */
	}
#endif

	if (!l) return;
	l = l->next;
	if (!l) return;

	curs->line++;

	curs->y = (int) *((int *) l->child->next->next->data) - 1;
	pos = pos_physical_to_logical(xed, curs->x, curs->y);

#if 1
	cong_location_set(&curs->location, pos->node, pos->c);
#else
	curs->t = pos->node;
	curs->c = pos->c;
#endif
	free(pos);
}


void curs_del_prev_char(struct curs* curs, CongXMLEditor *xed)
{
	g_assert(curs!=NULL);

#if 1
	if (!cong_location_exists(&curs->location)) return;
#else
	if (!curs->t) return;
#endif
	
	curs_prev_char(curs,xed);

#if 1
	cong_location_del_next_char(&curs->location);
#else	
	if (*(xml_frag_data_nice(curs->t) + curs->c))
	{
		memmove(xml_frag_data_nice(curs->t) + curs->c, xml_frag_data(curs->t) + curs->c + 1,
						strlen(xml_frag_data_nice(curs->t) + curs->c));
		curs->t->child->size--;
	}
	else if (curs->t->next && xml_frag_type(curs->t->next) == XML_TAG_EMPTY)
		ttree_branch_remove(curs->t->next);
#endif
}


void curs_del_next_char(struct curs* curs, CongXMLEditor *xed)
{
	g_assert(curs!=NULL);

#if 1
	if (!cong_location_exists(&curs->location)) return;

	cong_location_del_next_char(&curs->location);
#else
	if (!curs->t) return;
	
	if (*(xml_frag_data_nice(curs->t) + curs->c))
	{
		memmove(xml_frag_data_nice(curs->t) + curs->c, xml_frag_data(curs->t) + curs->c + 1,
						strlen(xml_frag_data_nice(curs->t) + curs->c));
		curs->t->child->size--;
	}
	else if (curs->t->next && xml_frag_type(curs->t->next) == XML_TAG_EMPTY)
		ttree_branch_remove(curs->t->next);
#endif
}


