/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>

#include <ctype.h>

#include <ttree.h>
#include <xml.h>
#include "global.h"


#define WIDTH_WRAP(disp_w, x, word_w) (((x) + (word_w)) > (disp_w - 1))

void pos_pl_data(struct xed *xed, struct pos *pos)
{
	char *data;
	UNUSED_VAR(int word_x_offset = 0)
	UNUSED_VAR(int i_last_space = 0)
	int i;

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
				if (WIDTH_WRAP(xed->w->allocation.width, pos->x, pos->word_width))
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
			pos->x += gdk_char_width(xed->f, ' ');
			pos->space = 0;
			continue;
		}
		else
		{
#ifndef RELEASE			
			fputc(data[i], stdout);
#endif			
			pos->word_width += gdk_char_width(xed->f, data[i]);
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


void pos_pl(struct xed *xed, struct pos *pos)
{
	TTREE *node_prev;
	TTREE *node_prev2;
	TTREE *node_first;

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

			if (xml_frag_type(pos->node) == XML_TAG_EMPTY &&
					!strcasecmp("p", xml_frag_name_nice(pos->node)))
			{
				pos->node = node_prev2;
				if (xml_frag_type(pos->node) == XML_DATA) 
					pos->c = strlen(xml_frag_data_nice(pos->node));
				else pos->c = 0;
				return;
			}

			else if (xml_frag_type(pos->node) == XML_TAG_SPAN &&
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

			else if (xml_frag_type(pos->node) == XML_TAG_SPAN)
			{
				if (cong_dispspec_element_structural(xed->displayspec, xml_frag_name_nice(pos->node)))
				{
					pos->node = node_prev2;
					if (xml_frag_type(pos->node) == XML_DATA) 
						pos->c = strlen(xml_frag_data_nice(pos->node));
					else pos->c = 0;
					return;
				}
				else if (xml_frag_enter(pos->node))
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
					pos->node = xml_frag_enter(pos->node);
					continue;
				}
			}

			/* If data fragment... */

			if (xml_frag_type(pos->node) == XML_DATA)
			{
				/* Do data */

				pos_pl_data(xed, pos);
				if (pos->mode == 1 || pos->node == pos->node_last) return;
				pos->c_given = 0;
			}

			/* Go forward */

			node_prev2 = pos->node;
			pos->node = xml_frag_next(pos->node);
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
			else if (!xml_frag_next(pos->node)) pos->node = xml_frag_exit(pos->node);
			else break;
		}
		
		if (pos->node) pos->node = xml_frag_next(pos->node);
	}
	while (pos->node);

  pos->node = node_prev;
}


struct pos *pos_physical_to_logical(struct xed *xed, int x, int y)
{
	struct pos *pos;
	TTREE *l;
	int i;

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

	/* Find line by y coord */

	g_assert(xed->lines);
	g_assert(xed->lines->child);

	for (i = 0, l = xed->lines->child; l->next; i++, l = l->next)
	{
		if ((int) *((int *) l->child->next->next->data) >= y)
		{
			/* Found the right line */
			break;
		}
	}

	pos->node = (TTREE *) *((TTREE **) l->child->data);
	pos->node_last = (TTREE *) *((TTREE **) l->child->next->next->next->data);
	pos->c_given = (int) *((int *) l->child->next->data);

	/* Traverse it */
	
	pos_pl(xed, pos);

#ifndef RELEASE
	printf("\n\n");
#endif
	return(pos);
}


/* -------- */


void pos_lp_data(struct xed *xed, struct pos *pos)
{
	char *data;
	int word_x_offset = 0;
	int i;

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
			if (pos->x > 1) pos->x += gdk_char_width(xed->f, ' ');  /* pos->word_width? */
			pos->space = 0;
			if (pos->mode == 1) return;
			continue;
		}
		else
		{
#ifndef RELEASE
			fputc(data[i], stdout);
#endif			
			pos->word_width += gdk_char_width(xed->f, data[i]);
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


void pos_lp(struct xed *xed, struct pos *pos)
{
	TTREE *node_prev;
	TTREE *node_first;

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

			if (xml_frag_type(pos->node) == XML_TAG_EMPTY &&
					!strcasecmp("p", xml_frag_name_nice(pos->node)))
			{
				pos->line++;
				pos->x = 0;
				pos->word_width = 0;
			}
			
			else if (xml_frag_type(pos->node) == XML_TAG_SPAN &&
					     !strcasecmp("table", xml_frag_name_nice(pos->node)))
			{
				pos->line += 2;
				pos->x = 0;
				pos->word_width = 0;
			}
			
			/* If spanning tag... */

			else if (xml_frag_type(pos->node) == XML_TAG_SPAN &&
					     xml_frag_enter(pos->node))
			{
				/* Go down */
#ifndef RELEASE
				fputc('>', stdout);
#endif				
				pos->node = xml_frag_enter(pos->node);
				continue;
			}

			/* If data fragment... */

			if (xml_frag_type(pos->node) == XML_DATA)
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
			
			pos->node = xml_frag_next(pos->node);
		}

		if (!pos->node) pos->node = node_prev;

		/* Go up */
		/* Go forward */

#ifndef RELEASE
		fputc('<', stdout);
		fputc('.', stdout);
#endif		

		while (pos->node && !xml_frag_next(pos->node))
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
			pos->node = xml_frag_exit(pos->node);
		}
		
		if (pos->node) pos->node = xml_frag_next(pos->node);
	}
	while (pos->node);

	pos->x += pos->word_width;
  pos->node = node_prev;
	
#ifndef RELEASE
	printf("[Q]");
#endif	
}


struct pos *pos_logical_to_physical(struct xed *xed, CongNodePtr node, int c)
{
	struct pos *pos;
	TTREE *l;
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

	g_assert(xed->lines);
	g_assert(xed->lines->child);

#ifndef RELEASE
	printf("\nGot line: %d.\n", pos->line);
#endif
	for (i = pos->line, l = xed->lines->child; i && l->next; i--) l = l->next;

	g_assert(l);
	
	if (l->prev) pos->y = (int) *((int *) l->prev->child->next->next->data);
	else pos->y = 0;

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

struct pos *pos_logical_to_physical_new(struct xed *xed, CongLocation *loc)
{
	return pos_logical_to_physical(xed, loc->tt_loc, loc->char_loc);
}
