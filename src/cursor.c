/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>

#include "global.h"
#include <string.h>
#include "cong-dispspec.h"
#include "cong-document.h"
#include "cong-error-dialog.h"
#include "cong-font.h"

void cong_cursor_init(CongCursor *curs, CongDocument *doc)
{
	GdkColor gcol;

	g_assert(curs!=NULL);
	g_assert(doc!=NULL);

	curs->doc = doc;

	cong_location_nullify(&curs->location);

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

#if 0
	g_message("cong_cursor_blink");
#endif

	if (curs->on)
	{
		curs->on = 0;
	}
	else
	{
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
	cong_location_set_node_and_byte_offset(&curs->location,t->next,0);
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
	new_element = cong_node_new_element(tagname, doc);

	cong_document_node_add_after(doc, new_element, t->parent);
	cong_document_node_set_parent(doc, curs->location.node, new_element);

	/* FIXME:  
	   Stepping through the code, it appears to work.  However the second para won't appear, as we need this to happen via the MVC framework.
	*/

	return(1);
}

void cong_cursor_del_prev_char(CongCursor *curs, CongDocument *doc)
{
	CongLocation prev_char;

	g_return_if_fail(curs);
	g_return_if_fail(doc);

	if (!cong_location_exists(&curs->location)) {
		return;
	}
	
 	if (cong_location_calc_prev_char(&curs->location, cong_document_get_dispspec(doc), &prev_char)) {
		cong_location_copy(&curs->location, &prev_char);
		
		cong_location_del_next_char(doc, &curs->location);
	}
}


void cong_cursor_del_next_char(CongCursor *curs, CongDocument *doc)
{
	g_return_if_fail(curs);
	g_return_if_fail(doc);

	if (!cong_location_exists(&curs->location)) return;

	cong_location_del_next_char(doc, &curs->location);
}


