/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>

#include "global.h"
#include <string.h>
#include "cong-dispspec.h"
#include "cong-document.h"
#include "cong-error-dialog.h"

void cong_cursor_on(CongCursor *curs)
{
	CongFont *font;

	g_return_if_fail(curs!=NULL);

	font = the_globals.fonts[CONG_FONT_ROLE_BODY_TEXT];
	g_assert(font);

	if (curs->w) {
		gdk_draw_line(curs->w->window, curs->gc, curs->x - 1 + 1, curs->y - 4,
			      curs->x - 1 + 1, curs->y + font->asc + font->desc + 4);
	}
}


void cong_cursor_off(CongCursor *curs)
{
	GdkRectangle r;
	CongFont *font;

	g_return_if_fail(curs!=NULL);

	font = the_globals.fonts[CONG_FONT_ROLE_BODY_TEXT];
	g_assert(font);

	r.x = curs->x - 1 + 1;
	r.y = curs->y - 4;
	r.height = font->asc + font->desc + 9;
	r.width = 1;
	if (curs->w) gtk_widget_draw(curs->w, &r);
}


#if 0
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
#endif

#if !USE_CONG_EDITOR_WIDGET
void cong_cursor_place_in_xed(CongCursor *curs, CongSpanEditor *xed, int x, int y)
{
	struct pos *pos0, *pos1;
	UNUSED_VAR(TTREE *l);
	UNUSED_VAR(int i);

	g_assert(curs!=NULL);

	curs->xed = xed;

	/* Remove from previous location */

	cong_cursor_off(curs);

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

	cong_cursor_on(curs);
}
#endif /* #if !USE_CONG_EDITOR_WIDGET */

void cong_cursor_init(CongCursor *curs, CongDocument *doc)
{
	GdkColor gcol;

	g_assert(curs!=NULL);
	g_assert(doc!=NULL);

	curs->doc = doc;

	curs->w = 0;
	curs->x = curs->y = 0;
#if 1
	cong_location_set(&curs->location, NULL, 0);
#else
	curs->t = 0;
	curs->c = 0;
#endif

	curs->timeout_id = gtk_timeout_add(500, cong_cursor_blink, curs);
	curs->gc = gdk_gc_new(cong_gui_get_a_window()->window);
	gdk_gc_copy(curs->gc, cong_gui_get_a_window()->style->black_gc);
	col_to_gcol(&gcol, 0x00ff8c00);
	gdk_colormap_alloc_color(cong_gui_get_a_window()->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(curs->gc, &gcol);
}

void cong_cursor_uninit(CongCursor *curs)
{
	g_assert(curs);

	gtk_timeout_remove(curs->timeout_id);
}


gint cong_cursor_blink(gpointer data)
{
	CongCursor *curs = data;

	g_message("cong_cursor_blink");

#if !USE_CONG_EDITOR_WIDGET
	if (!curs->w) return(TRUE);
#endif

	if (curs->on)
	{
		cong_cursor_off(curs);
		curs->on = 0;
	}
	else
	{
		cong_cursor_on(curs);
		curs->on = 1;
	}

	cong_document_on_cursor_change(curs->doc);
	
	return(TRUE);
}

gint cong_cursor_data_insert(CongCursor *curs, char *s)
{
	CongDocument *doc;
	int len;

	g_return_val_if_fail(curs, 0);
	g_return_val_if_fail(s, 0);

	doc = curs->doc;

	len = strlen(s);

	if (!cong_location_exists(&curs->location)) return(0);
	if (cong_location_node_type(&curs->location) != CONG_NODE_TYPE_TEXT) return(0);

#if 1
	cong_location_insert_chars(doc, &curs->location, s);
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


int cong_cursor_paragraph_insert(CongCursor *curs)
{
        CongNodePtr t;
	CongNodePtr new_element;
	CongDispspec *ds;
	CongDispspecElement *para;
	const char *tagname;
	CongDocument *doc;

	g_assert(curs!=NULL);

#if !USE_CONG_EDITOR_WIDGET	
	if (!curs->xed) return(0);
#endif

	if (!cong_location_exists(&curs->location)) return(0);
	if (cong_location_node_type(&curs->location) != CONG_NODE_TYPE_TEXT) return(0);

	doc = curs->doc;
	ds = cong_document_get_dispspec(doc);

#if 0
	para = cong_dispspec_get_paragraph(ds);

	if (NULL==para) {
		/* The dispspec does not have a "paragraph" tag */
		return 0;
	}

	tagname = cong_dispspec_element_tagname(para);
#else
	/* Dodgy hack for now: */
#if 1
	tagname = cong_node_name(curs->location.node->parent);
#else
	tagname = "para";
#endif
#endif

	/* GREP FOR MVC */

#if 1
	t = cong_location_xml_frag_data_nice_split2(doc, &curs->location);
	cong_location_set(&curs->location,t->next,0);
#else	
	t = xml_frag_data_nice_split2(curs->t, curs->c);
	curs->t = t->next;
	curs->c = 0;
#endif

	/* New approach, aimed at DocBook support: 
	   Assume that we've just split up a text node below a <para> node below some parent into two
	   text nodes below that para.
	   We need to create an new <para> node as a sibling of the other para, and move the second text node
	   to below it.
	*/
	new_element = cong_node_new_element(tagname);

	cong_document_node_add_after(doc, new_element, t->parent);
	cong_document_node_set_parent(doc, curs->location.node, new_element);

	/* FIXME:  
	   Stepping through the code, it appears to work.  However the second para won't appear, as we need this to happen via the MVC framework.
	*/

	return(1);
}


gboolean cong_location_calc_prev_char(const CongLocation *input_loc, 
				      CongDispspec *dispspec,
				      CongLocation *output_loc)
{
	CongNodePtr n;
	CongNodePtr n0;

#ifndef RELEASE	
	printf("<- [curs]\n");
#endif

	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);
	
	n = input_loc->node;
	if (cong_location_node_type(input_loc) == CONG_NODE_TYPE_TEXT && input_loc->byte_offset) { 

		gchar *this_char;
		gchar *prev_char;

		g_assert(input_loc->node);
		g_assert(input_loc->node->content);
		g_assert(g_utf8_validate(input_loc->node->content,-1,NULL));

		this_char = input_loc->node->content+input_loc->byte_offset;
		prev_char = g_utf8_find_prev_char(input_loc->node->content, this_char);
		g_assert(prev_char);

		cong_location_set(output_loc,input_loc->node, prev_char - (gchar*)input_loc->node->content);

		return TRUE;
	}

	do
	{
		n0 = n;
		if (n) n = cong_node_prev(n);
		
		for ( ; n; )
		{
			if (cong_node_type(n) == CONG_NODE_TYPE_TEXT) break;
			else if (cong_node_type(n) == CONG_NODE_TYPE_ELEMENT)
			{
				if (!strcmp(cong_node_name(n), "table")) break;
				if (cong_dispspec_element_structural(dispspec, xml_frag_name_nice(n)))
				{
					n = n0 = 0;
					break;
				}
				else if (cong_node_first_child(n))
				{
#ifndef RELEASE					
					printf("Entering tag: %s.\n", xml_frag_name_nice(n));
#endif					
					n = cong_node_first_child(n);
					continue;
				}
			}
			
			n0 = n;
			n = cong_node_prev(n);
		}

		if (!n) n = n0;
		else if (cong_node_type(n) == CONG_NODE_TYPE_TEXT) break;

		while (n)
		{
			if (cong_dispspec_element_structural(dispspec, xml_frag_name_nice(n))) { n = 0; break; }
			if (!cong_node_prev(n)) n = n0 = cong_node_parent(n);
			else break;
		}
	}
	while (n);

	if (n) {
		cong_location_set(output_loc,n, strlen(xml_frag_data_nice(n)));
		return TRUE;
	} else {
		return FALSE;
	}
}


gboolean cong_location_calc_next_char(const CongLocation *input_loc,
				      CongDispspec *dispspec,
				      CongLocation *output_loc)
{
	CongNodePtr n;
	CongNodePtr n0;

#ifndef RELEASE	
	printf("[curs] ->\n");
#endif

	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);
	

	n = input_loc->node;
	if (cong_location_node_type(input_loc) == CONG_NODE_TYPE_TEXT && cong_location_get_unichar(input_loc))
	{ 
		gchar *this_char;
		gchar *next_char;

		g_assert(input_loc->node);
		g_assert(input_loc->node->content);
		g_assert(g_utf8_validate(input_loc->node->content,-1,NULL));

		this_char = input_loc->node->content+input_loc->byte_offset;
		next_char = g_utf8_find_next_char(this_char, NULL);
		g_assert(next_char);

		cong_location_set(output_loc,input_loc->node, next_char - (gchar*)input_loc->node->content);

		return TRUE; 
	}

	do
	{
		n0 = n;
		if (n) n = cong_node_next(n);

		for ( ; n; )
		{
			if (cong_node_type(n) == CONG_NODE_TYPE_TEXT) break;
			else if (cong_node_type(n) == CONG_NODE_TYPE_ELEMENT)
			{				 
				if (!strcmp(cong_node_name(n), "table")) break;
				if (cong_dispspec_element_structural(dispspec, xml_frag_name_nice(n)))
				{
					n = n0 = 0;
					break;
				}
				else if (cong_node_first_child(n))
				{
#ifndef RELEASE					
					printf("Entering tag: %s.\n", xml_frag_name_nice(n));
#endif
					n = cong_node_first_child(n);
					continue;
				}
			}
			
			n0 = n;
			n = cong_node_next(n);
		}

		if (!n) n = n0;
		else if (cong_node_type(n) == CONG_NODE_TYPE_TEXT) break;

		while (n)
		{
			if (cong_dispspec_element_structural(dispspec, xml_frag_name_nice(n))) { n = 0; break; }
			if (!cong_node_next(n)) n = n0 = cong_node_parent(n);
			else break;
		}
	}
	while (n);

	if (n) {
		cong_location_set(output_loc, n, 0);
		return TRUE;
	} else {
		return FALSE;
	}
}

gboolean cong_location_calc_prev_word(const CongLocation *input_loc, 
				      CongDispspec *dispspec,
				      CongLocation *output_loc)
{
	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	CONG_DO_UNIMPLEMENTED_DIALOG("Calculating previous word");

	return FALSE;
}

gboolean cong_location_calc_next_word(const CongLocation *input_loc, 
				      CongDispspec *dispspec,
				      CongLocation *output_loc)
{
	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	CONG_DO_UNIMPLEMENTED_DIALOG("Calculating next word");

	return FALSE;
}

gboolean cong_location_calc_document_start(const CongLocation *input_loc, 
					   CongDispspec *dispspec,
					   CongLocation *output_loc)
{
	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	CONG_DO_UNIMPLEMENTED_DIALOG("Calculating document start");

	return FALSE;
}
gboolean cong_location_calc_line_start(const CongLocation *input_loc, 
				      CongDispspec *dispspec,
				      CongLocation *output_loc)
{
	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	CONG_DO_UNIMPLEMENTED_DIALOG("Calculating line start");

	return FALSE;
}
gboolean cong_location_calc_document_end(const CongLocation *input_loc, 
					 CongDispspec *dispspec,
					 CongLocation *output_loc)
{
	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	CONG_DO_UNIMPLEMENTED_DIALOG("Calculating document end");

	return FALSE;
}
gboolean cong_location_calc_line_end(const CongLocation *input_loc, 
				     CongDispspec *dispspec,
				     CongLocation *output_loc)
{
	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	CONG_DO_UNIMPLEMENTED_DIALOG("Calculating line end");

	return FALSE;
}
gboolean cong_location_calc_prev_page(const CongLocation *input_loc, 
				      CongDispspec *dispspec,
				      CongLocation *output_loc)
{
	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	CONG_DO_UNIMPLEMENTED_DIALOG("Calculating previous page");

	return FALSE;
}
gboolean cong_location_calc_next_page(const CongLocation *input_loc, 
				      CongDispspec *dispspec,
				      CongLocation *output_loc)
{
	g_return_val_if_fail(input_loc, FALSE);
	g_return_val_if_fail(dispspec, FALSE);
	g_return_val_if_fail(output_loc, FALSE);

	CONG_DO_UNIMPLEMENTED_DIALOG("Calculating next page");

	return FALSE;
}

#if !USE_CONG_EDITOR_WIDGET
void cong_cursor_prev_line(CongCursor *curs, CongSpanEditor *xed)
{
	struct pos *pos;
	CongLayoutLine *l;
	
	g_assert(curs!=NULL);
	
	if (curs->line == 0) return;
	
	curs->line--;

	l = cong_layout_cache_get_line_by_index(&xed->layout_cache, curs->line);

	if (!l) return;

	curs->y = cong_layout_line_get_second_y(l);

	pos = pos_physical_to_logical(xed, curs->x, curs->y);

	cong_location_set(&curs->location, pos->node, pos->c);

	free(pos);
}


void cong_cursor_next_line(CongCursor *curs, CongSpanEditor *xed)
{
	struct pos *pos;
	CongLayoutLine *l;

	g_assert(curs!=NULL);

	l = cong_layout_cache_get_line_by_index(&xed->layout_cache, curs->line);

	if (!l) return;
	l = cong_layout_line_get_next(l);
	if (!l) return;

	curs->line++;

	curs->y = cong_layout_line_get_second_y(l) - 1;
	pos = pos_physical_to_logical(xed, curs->x, curs->y);

#if 1
	cong_location_set(&curs->location, pos->node, pos->c);
#else
	curs->t = pos->node;
	curs->c = pos->c;
#endif
	free(pos);
}
#endif /* #if !USE_CONG_EDITOR_WIDGET */

void cong_cursor_del_prev_char(CongCursor *curs, CongDocument *doc)
{
	CongLocation prev_char;

	g_return_if_fail(curs);
	g_return_if_fail(doc);

	if (!cong_location_exists(&curs->location)) {
		return;
	}
	
 	cong_location_calc_prev_char(&curs->location, cong_document_get_dispspec(doc), &prev_char);
	cong_location_copy(&curs->location, &prev_char);

	cong_location_del_next_char(doc, &curs->location);
}


void cong_cursor_del_next_char(CongCursor *curs, CongDocument *doc)
{
	g_return_if_fail(curs);
	g_return_if_fail(doc);

	if (!cong_location_exists(&curs->location)) return;

	cong_location_del_next_char(doc, &curs->location);
}


