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
#include "cong-command.h"

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
	/* we really want a black cursor for visibility */	
	/*col_to_gcol(&gcol, 0x00ff8c00); */
	col_to_gcol(&gcol, 0x00000000);
	gdk_colormap_alloc_color(cong_gui_get_a_window()->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(curs->gc, &gcol);
}

void cong_cursor_uninit(CongCursor *curs)
{
	g_assert(curs);

	curs->doc = NULL;
	cong_location_nullify(&curs->location);

	if (curs->timeout_id) {
		gtk_timeout_remove (curs->timeout_id);
		curs->timeout_id = 0;
	}

	/* FIXME: anything else needed? */
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


	cong_document_begin_edit(curs->doc);
	cong_document_private_on_cursor_change(curs->doc);
	cong_document_end_edit(curs->doc);
	
	return(TRUE);
}

int cong_cursor_paragraph_insert(CongCursor *curs)
{
        CongNodePtr t;
        CongNodePtr iter, next;
	CongNodePtr new_element;
	CongDispspecElement *para;
	const gchar *xmlns;
	const gchar *tagname;
	CongDocument *doc;

	g_return_val_if_fail (curs, FALSE);
	g_return_val_if_fail (cong_location_exists(&curs->location), FALSE);
	g_return_val_if_fail (cong_location_node_type(&curs->location) == CONG_NODE_TYPE_TEXT, FALSE);

	doc = curs->doc;

	xmlns = cong_node_xmlns(curs->location.node->parent);
	tagname = cong_node_name(curs->location.node->parent);

	cong_document_begin_edit (doc);

	{
		gchar *desc = g_strdup_printf (_("Split %s tag"), tagname);
		CongCommand *cmd = cong_document_begin_command (doc, desc, NULL);
		CongLocation new_cursor_loc;

		t = cong_command_add_xml_frag_data_nice_split2 (cmd, 
								&curs->location);
		
		cong_location_set_node_and_byte_offset (&new_cursor_loc, 
							t->next, 
							0);

		cong_command_add_cursor_change (cmd,
						&new_cursor_loc);

		/* Assume that we've just split up a text node below a <para> node below some parent into two
		   text nodes below that para.
		   We need to create an new <para> node as a sibling of the other para, and move the second text node
		   and all the rest of the siblings to below it.
		*/
		new_element = cong_node_new_element(xmlns, tagname, doc);

		cong_command_add_node_add_after (cmd, 
						 new_element, 
						 t->parent);

		/* Move the second text node and all successive siblings; this should deal with inline tags further in the para: */
		for (iter = new_cursor_loc.node; iter; iter = next) {
			next = iter->next;
			cong_command_add_node_set_parent (cmd, 
							  iter, 
							  new_element);
		}
		
		cong_document_end_command (doc,
					   cmd);
	}

	cong_document_end_edit (doc);

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


