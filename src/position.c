/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>

#include <ctype.h>

#include "global.h"

void
cong_layout_line_free(CongLayoutLine *line)
{
	g_return_if_fail(line);

	g_free(line);
}

void
cong_layout_cache_init(CongLayoutCache *layout_cache)
{
	g_return_if_fail(layout_cache);

	while (layout_cache->first_line) {
		CongLayoutLine *next = layout_cache->first_line->next;
		
		cong_layout_line_free(layout_cache->first_line);

		layout_cache->first_line=next;		
	}
	layout_cache->last_line=NULL;
}

void
cong_layout_cache_clear(CongLayoutCache *layout_cache)
{
	g_return_if_fail(layout_cache);

	while (layout_cache->first_line) {
		CongLayoutLine *next = layout_cache->first_line->next;
		
		cong_layout_line_free(layout_cache->first_line);

		layout_cache->first_line=next;		
	}
	layout_cache->last_line=NULL;
}

CongLayoutLine*
cong_layout_cache_get_line_by_y_coord(CongLayoutCache *layout_cache, int y)
{
	CongLayoutLine *l;

	/* Find line by y coord */
	g_return_val_if_fail(layout_cache, NULL);

	for (l = layout_cache->first_line; l->next; l = l->next)
	{
		if (cong_layout_line_get_second_y(l) >= y)
		{
			/* Found the right line */
			break;
		}
	}

	return l;
}

CongLayoutLine*
cong_layout_cache_get_line_by_index(CongLayoutCache *layout_cache, int i)
{
	CongLayoutLine *l;

	g_return_val_if_fail(layout_cache, NULL);
	g_return_val_if_fail(i>=0, NULL);

	for (l = layout_cache->first_line; i && l->next; i--) {
		l = l->next;
	}

	return l;
}

CongLayoutLine*
cong_layout_cache_get_last_line(CongLayoutCache *layout_cache)
{	
	CongLayoutLine *t;

	g_return_val_if_fail(layout_cache, NULL);

	t = layout_cache->last_line;

	return(t);
}

#define WIDTH_WRAP(disp_w, x, word_w) (((x) + (word_w)) > (disp_w - 1))

void pos_pl_data(CongSpanEditor *xed, struct pos *pos)
{
	const char *data;
	UNUSED_VAR(int word_x_offset = 0)
	UNUSED_VAR(int i_last_space = 0)
	int i;

	CongFont *font = cong_span_editor_get_font(xed, CONG_FONT_ROLE_BODY_TEXT);
	g_assert(font);

	for (i = pos->c_given, data = xml_frag_data_nice(pos->node); data[i]; )
	{
		if (pos->x + pos->word_width >= pos->x_find)
		{
#ifndef RELEASE
			fputc('*', stdout);
#endif
			pos->x += pos->word_width;
			pos->c = i;
			pos->mode = 1;
			return;
		}

		if ((isspace(data[i]) || isblank(data[i])))
		{
			if (!pos->space)
			{
				/* If cumulated word width makes for a wrap, we're done */
				if (WIDTH_WRAP(cong_span_editor_get_widget(xed)->allocation.width, pos->x, pos->word_width))
				{
#ifndef RELEASE					
					fputc('\\', stdout);
#endif					
					pos->c = i;
					pos->mode = 1;  /* Don't include its width */
					return;
				}
				pos->space = 1;
			}
		}
		else if (pos->space)
		{
#ifndef RELEASE
			fputc(' ', stdout);
#endif
			pos->x += gdk_char_width(font->gdk_font, ' ');
			pos->space = 0;
			continue;
		}
		else
		{
#ifndef RELEASE			
			fputc(data[i], stdout);
#endif			
			pos->word_width += gdk_char_width(font->gdk_font, data[i]);
		}

		i++;
	}

	if (pos->node == pos->node_last) 
	{
		pos->c = i;
		pos->mode = 1;
#ifndef RELEASE
		printf("!");
#endif
	}
}


void pos_pl(CongSpanEditor *xed, struct pos *pos)
{
	CongNodePtr node_prev;
	CongNodePtr node_prev2;
	CongNodePtr node_first;

	g_assert(pos->node);
	
	node_first = pos->node;

#ifndef RELEASE
	printf("*** Physical to logical conversion: (%d,%d)\n", pos->x_find, pos->y);
#endif

	do
	{
		for (; pos->node; )
		{
			node_prev = pos->node;  /* Backup for later */

			/* XML_TAG_EMPTY isn't supported by my parser */
#if 0
			if (xml_frag_type(pos->node) == XML_TAG_EMPTY &&
					!strcasecmp("p", xml_frag_name_nice(pos->node)))
			{
				pos->node = node_prev2;
				if (xml_frag_type(pos->node) == XML_DATA) 
					pos->c = strlen(xml_frag_data_nice(pos->node));
				else pos->c = 0;
				return;
			}
			else 
#endif

			if (cong_node_type(pos->node) == CONG_NODE_TYPE_ELEMENT &&
			    !strcasecmp("table", xml_frag_name_nice(pos->node)))
			{
/*				
				pos->node = node_prev2;

				if (xml_frag_type(pos->node) == XML_DATA)
					pos->c = strlen(xml_frag_data_nice(pos->node));
				else pos->c = 0;
*/
				return;
			}

			/* If spanning tag... */

			else if (cong_node_type(pos->node) == CONG_NODE_TYPE_ELEMENT)
			{
				if (cong_dispspec_element_structural(xed->displayspec, xml_frag_name_nice(pos->node)))
				{
					pos->node = node_prev2;
					if (cong_node_type(pos->node) == CONG_NODE_TYPE_TEXT) {
						pos->c = strlen(xml_frag_data_nice(pos->node));
					} else {
						pos->c = 0;
					}
					return;
				}
				else if (cong_node_first_child(pos->node))
				{
					if (pos->node != node_first)
					{
						if (pos->node == pos->node_last) return;
					}
					else node_first = 0;
					
					pos->space = 0;
					
					/* Go down */
#ifndef RELEASE					
					fputc('>', stdout);
#endif					
					pos->node = cong_node_first_child(pos->node);
					continue;
				}
			}

			/* If data fragment... */

			if (cong_node_type(pos->node) == CONG_NODE_TYPE_TEXT)
			{
				/* Do data */

				pos_pl_data(xed, pos);
				if (pos->mode == 1 || pos->node == pos->node_last) return;
				pos->c_given = 0;
			}

			/* Go forward */

			node_prev2 = pos->node;
			pos->node = cong_node_next(pos->node);
#ifndef RELEASE
			fputc('.', stdout);
#endif			
		}

		if (!pos->node) pos->node = node_prev;

		/* Go up */
		/* Go forward */

#ifndef RELEASE		
		fputc('<', stdout);
		fputc('.', stdout);
#endif
		
		while (pos->node)
		{
			if (cong_dispspec_element_structural(xed->displayspec, xml_frag_name_nice(pos->node)))
			{ pos->node = 0; break; }
				
			node_prev = pos->node;
			if (pos->node == pos->node_last) return;
			else if (!cong_node_next(pos->node)) pos->node = cong_node_parent(pos->node);
			else break;
		}
		
		if (pos->node) pos->node = cong_node_next(pos->node);
	}
	while (pos->node);

	pos->node = node_prev;
}


struct pos *pos_physical_to_logical(CongSpanEditor *xed, int x, int y)
{
	struct pos *pos;
	CongLayoutLine *l;

	/* Basic init */

	pos = malloc(sizeof(*pos));
	pos->x_find = x;
	pos->x = 0;
	pos->y = y;
	pos->c = 0;
	pos->word_width = 0;
	pos->space = 0;
	pos->mode = 0;

	/* xed_word() workarounds */

	xed->draw_char = 0;
	xed->draw_pos_x = 0;

	l = cong_layout_cache_get_line_by_y_coord(&xed->layout_cache, y);

#if 1
	pos->node = cong_layout_line_get_node(l);
	pos->node_last = cong_layout_line_get_node_last(l);
	pos->c_given = cong_layout_line_get_c_given(l);
#else
	pos->node = (TTREE *) *((TTREE **) l->child->data);
	pos->node_last = (TTREE *) *((TTREE **) l->child->next->next->next->data);
	pos->c_given = (int) *((int *) l->child->next->data);
#endif

	/* Traverse it */
	
	pos_pl(xed, pos);

#ifndef RELEASE
	printf("\n\n");
#endif
	return(pos);
}


/* -------- */


void pos_lp_data(CongSpanEditor *xed, struct pos *pos)
{
	const char *data;
	int word_x_offset = 0;
	int i;

	CongFont *font = cong_span_editor_get_font(xed, CONG_FONT_ROLE_BODY_TEXT);
	g_assert(font);


	if (pos->node == pos->node_find && !pos->c) 
	{
		pos->x += pos->word_width;
		pos->mode = 1;
		return;
	}

	for (i = 0, data = xml_frag_data_nice(pos->node); data[i]; )
	{
		if ((isspace(data[i]) || isblank(data[i])))
		{
			if (!pos->space)
			{
				/* If cumulated word width makes for a wrap, do it now */
				if (WIDTH_WRAP(xed->w->allocation.width, pos->x, pos->word_width))
				{
					pos->line++;
					if (word_x_offset)
					{
#ifndef RELEASE
						fputc('\\', stdout);
#endif						
						pos->x = word_x_offset;
						return;
					}
					
					pos->x = pos->word_width;
					pos->word_width = 0;
				}
				else if (word_x_offset)
				{
#ifndef RELEASE
					fputc('/', stdout);
#endif					
					pos->x += word_x_offset;
					return;
				}
				else
				{
					pos->x += pos->word_width;
					pos->word_width = 0;
				}
				pos->space = 1;
			}
		}
		else if (pos->space)
		{
#ifndef RELEASE
			fputc(' ', stdout);
#endif			
			if (pos->x > 1) pos->x += gdk_char_width(font->gdk_font, ' ');  /* pos->word_width? */
			pos->space = 0;
			if (pos->mode == 1) return;
			continue;
		}
		else
		{
#ifndef RELEASE
			fputc(data[i], stdout);
#endif			
			pos->word_width += gdk_char_width(font->gdk_font, data[i]);
		}

		i++;

		if (pos->node == pos->node_find && pos->c == i)
		{
#ifndef RELEASE
			fputc('%', stdout);
#endif
			word_x_offset = pos->word_width;
			pos->mode = 1;
		}
	}
#ifndef RELEASE
	fputc('|', stdout);
#endif	
	if (WIDTH_WRAP(xed->w->allocation.width, pos->x, pos->word_width))
	{
		pos->line++;
		pos->x = 0;
	}
	
	pos->x += word_x_offset;
}


void pos_lp(CongSpanEditor *xed, struct pos *pos)
{
	CongNodePtr node_prev;
	CongNodePtr node_first;

	g_assert(pos->node);
	
	node_first = pos->node;

#ifndef RELEASE
	printf("*** Logical to physical conversion: (c=%d)\n", pos->c);
#endif

	do
	{
		for (; pos->node; )
		{
			node_prev = pos->node;  /* Backup for later */

			/* XML_TAG_EMPTY is never output by my parser */
#if 0
			if (xml_frag_type(pos->node) == XML_TAG_EMPTY &&
					!strcasecmp("p", xml_frag_name_nice(pos->node)))
			{
				pos->line++;
				pos->x = 0;
				pos->word_width = 0;
			}
			else 
#endif
			if (cong_node_type(pos->node) == CONG_NODE_TYPE_ELEMENT &&
					     !strcasecmp("table", xml_frag_name_nice(pos->node)))
			{
				pos->line += 2;
				pos->x = 0;
				pos->word_width = 0;
			}
			
			/* If spanning tag... */

			else if (cong_node_type(pos->node) == CONG_NODE_TYPE_ELEMENT &&
				 cong_node_first_child(pos->node))
			{
				/* Go down */
#ifndef RELEASE
				fputc('>', stdout);
#endif				
				pos->node = cong_node_first_child(pos->node);
				continue;
			}

			/* If data fragment... */

			if (cong_node_type(pos->node) == CONG_NODE_TYPE_TEXT)
			{
				/* Do data */

				pos_lp_data(xed, pos);
				if (pos->mode == 1)
				{
#ifndef RELEASE
					printf("[D]");
#endif					
					return;
				}
			}

			/* Go forward */
			
			pos->node = cong_node_next(pos->node);
		}

		if (!pos->node) pos->node = node_prev;

		/* Go up */
		/* Go forward */

#ifndef RELEASE
		fputc('<', stdout);
		fputc('.', stdout);
#endif		

		while (pos->node && !cong_node_next(pos->node))
		{
			if (pos->node == pos->node_find)
			{
				pos->x += pos->word_width;
#ifndef RELEASE
				printf("[N]");
#endif				
				return;
			}

			node_prev = pos->node;
			pos->node = cong_node_parent(pos->node);
		}
		
		if (pos->node) pos->node = cong_node_next(pos->node);
	}
	while (pos->node);

	pos->x += pos->word_width;
	pos->node = node_prev;
	
#ifndef RELEASE
	printf("[Q]");
#endif	

}


struct pos *pos_logical_to_physical(CongSpanEditor *xed, CongNodePtr node, int c)
{
	struct pos *pos;
	CongLayoutLine *l;
	int i;

	/* Basic init */

	pos = malloc(sizeof(*pos));
	pos->node_find = node;
	pos->node = xed->x;
	pos->c = c;
	pos->x = 0;
	pos->y = 0;
	pos->line = 0;
	pos->word_width = 0;
	pos->space = 0;
	pos->mode = 0;
	xed->draw_char = 0;  /* xed_word() workaround */

	pos_lp(xed, pos);

#ifndef RELEASE
	printf("\nGot line: %d.\n", pos->line);
#endif

	l = cong_layout_cache_get_line_by_index(&xed->layout_cache, pos->line);

	g_assert(l);
	
	if (cong_layout_line_get_prev(l)) {
#if 1
		pos->y = cong_layout_line_get_second_y(cong_layout_line_get_prev(l));
#else
		g_assert(l->prev->child);
		g_assert(l->prev->child->next);
		g_assert(l->prev->child->next->next);
		pos->y = (int) *((int *) l->prev->child->next->next->data);
#endif
	} else {
		pos->y = 0;
	}

#if 0	
	for (i = 0, l = xed->lines->child; l->next; i++, l = l->next)
	{
		if ((int) *((int *) l->child->next->next->data) >= y)
		{
			/* Found the right line */
			break;
		}
	}
#endif
	
	/* x and y are now set */

#ifndef RELEASE
	printf("\n\n");
#endif
	return(pos);
}

struct pos *pos_logical_to_physical_new(CongSpanEditor *xed, CongLocation *loc)
{
	return pos_logical_to_physical(xed, loc->tt_loc, loc->char_loc);
}
