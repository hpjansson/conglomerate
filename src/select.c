/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>

#include "global.h"
#include "cong-document.h"
#include "cong-error-dialog.h"

#include "cong-selection.h"

struct CongSelection
{
	/* Range, respecting start/end: */
	CongRange logical_range;

	/* As above, but potentially reordered so that start/end are in "document order" */
	CongRange ordered_range;	

	GdkGC *gc_valid; /* corresponds to value gc_0 in old implementation */
	GdkGC *gc_invalid;   /* corresponds to value gc_3 in old implementation */
};

/* Internal function declarations: */
static void
update_ordered_selection (CongSelection *selection);

/* Exported function definitions: */
CongSelection*
cong_selection_new (void)
{
	CongSelection* selection; 
	GdkColor gcol;
	GtkWidget* window;

	selection = g_new0(CongSelection, 1);

	cong_range_init (&selection->logical_range);
	cong_range_init (&selection->ordered_range);

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

	return selection;
}

void 
cong_selection_free (CongSelection *selection)
{
	g_assert_not_reached(); /* UNWRITTEN */
}

void 
cong_selection_start_from_curs (CongSelection *selection, 
				CongCursor *curs)
{
	g_assert(selection!=NULL);
	g_assert(curs!=NULL);

	cong_location_copy(&selection->logical_range.loc0, &curs->location);
	cong_location_copy(&selection->logical_range.loc1, &curs->location);

	update_ordered_selection (selection);
}


void 
cong_selection_end_from_curs (CongSelection *selection, 
			      CongCursor *curs)
{
	g_assert(selection!=NULL);
	g_assert(curs!=NULL);

	cong_location_copy(&selection->logical_range.loc1, &curs->location);

	update_ordered_selection (selection);
}

CongRange*
cong_selection_get_logical_range (CongSelection *selection)
{
	g_return_val_if_fail (selection, NULL);

	return &selection->logical_range;
}

CongRange*
cong_selection_get_ordered_range (CongSelection *selection)
{
	g_return_val_if_fail (selection, NULL);

	/* FIXME: should we call this here: */
	update_ordered_selection (selection);

	return &selection->ordered_range;
}

CongLocation*
cong_selection_get_logical_start (CongSelection *selection)
{
	g_return_val_if_fail (selection, NULL);

	return &(selection->logical_range.loc0);
}

CongLocation*
cong_selection_get_logical_end (CongSelection *selection)
{
	g_return_val_if_fail (selection, NULL);

	return &(selection->logical_range.loc1);
}

CongLocation*
cong_selection_get_ordered_start (CongSelection *selection)
{
	g_return_val_if_fail (selection, NULL);

	/* FIXME: should we call this here: */
	update_ordered_selection (selection);

	return &(selection->ordered_range.loc0);
}

CongLocation*
cong_selection_get_ordered_end (CongSelection *selection)
{
	g_return_val_if_fail (selection, NULL);

	/* FIXME: should we call this here: */
	update_ordered_selection (selection);

	return &(selection->ordered_range.loc1);
}

void
cong_selection_set_logical_start (CongSelection *selection,
				  CongLocation *location)
{	
	g_assert(selection!=NULL);
	g_assert(location!=NULL);

	cong_location_copy(&selection->logical_range.loc0, location);

	update_ordered_selection (selection);

}

void
cong_selection_set_logical_end (CongSelection *selection,
				CongLocation *location)
{
	g_assert(selection!=NULL);
	g_assert(location!=NULL);

	cong_location_copy(&selection->logical_range.loc1, location);

	update_ordered_selection (selection);
}

#if 1
struct split3_userdata
{
	CongNodePtr s;
	int c0;
	int c1;

	CongNodePtr d1;
	CongNodePtr d2;
	CongNodePtr d3;
};

static gboolean
split3_location_callback (CongDocument *doc,
			  CongLocation *location, 
			  gpointer user_data)
{
	struct split3_userdata* split3_data = user_data;
	
	if (location->node == split3_data->s) {
		if (location->byte_offset<split3_data->c0) {
			location->node = split3_data->d1;
		} else {
			if (location->byte_offset<split3_data->c1) {
				location->node = split3_data->d2;
				location->byte_offset -= split3_data->c0;
			} else {
				location->node = split3_data->d3;
				location->byte_offset -= split3_data->c1;
			}
		}

		return TRUE;
	}

	return FALSE;
}

/* Splits a data node in 3 and returns pointer to the middle one */
CongNodePtr
cong_document_node_split3 (CongDocument *doc, 
			   CongNodePtr s, 
			   int c0, 
			   int c1)
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
	d1 = cong_node_new_text_len(xml_frag_data_nice(s), len1, doc); /* FIXME:  audit the char types here, and the char pointer arithmetic. UTF8? */
	d2 = cong_node_new_text_len(xml_frag_data_nice(s) + len1, len2, doc);
	d3 = cong_node_new_text_len(xml_frag_data_nice(s) + len1 + len2, len3, doc);

	cong_document_begin_edit(doc);

	/* Link it in */
	cong_document_node_add_after(doc, d1, s);
	cong_document_node_add_after(doc, d2, d1);
	cong_document_node_add_after(doc, d3, d2);
	cong_document_node_make_orphan(doc, s);

	/* Update the cursor and selection as necessary: */
	{
		struct split3_userdata user_data;
		
		user_data.s = s;
		user_data.c0 = c0;
		user_data.c1 = c1;
		user_data.d1 = d1;
		user_data.d2 = d2;
		user_data.d3 = d3;
		
		cong_document_for_each_location (doc, 
						 split3_location_callback,
						 &user_data);
	}

	/* Unlink old node */
	cong_document_node_recursive_delete (doc, s);

	cong_document_end_edit(doc);

	CONG_NODE_SELF_TEST(d2);

	return(d2);
}


/* Splits a data node in 2 and returns pointer to first one */
CongNodePtr
cong_document_node_split2 (CongDocument *doc, 
			   CongNodePtr s, 
			   int c)
{
	CongNodePtr d = NULL;
	int len1, len2;

	g_return_val_if_fail(cong_node_type(s) == CONG_NODE_TYPE_TEXT, NULL);

	CONG_NODE_SELF_TEST(s);

	/* Calculate segments */

	len1 = c;
	len2 = cong_node_get_length(s) - len1;

	if (!len1 && !len2) {
		d = cong_node_new_text("", doc);
	} else if (!len1) {
		d = cong_node_new_text("", doc);

		/* Link it in */
		cong_document_node_add_before(doc, d,s);
		return(d);
	} else if (!len2) {
		d = cong_node_new_text("", doc);
	} else {
		xmlChar* new_text = g_strndup(s->content, len1); /* FIXME:  char type conversion? */

		/* Make split representation */
		d = cong_node_new_text_len(xml_frag_data_nice(s) + len1, len2, doc); /* FIXME: check char ptr arithmetic; UTF8? */

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
	g_return_val_if_fail( cong_location_exists(&selection->logical_range.loc0), NULL );
	g_return_val_if_fail( cong_location_exists(&selection->logical_range.loc1), NULL );
	g_return_val_if_fail( cong_location_parent(&selection->logical_range.loc0) == cong_location_parent(&selection->logical_range.loc1), NULL);
	/* both must be children of the same parent to maintain proper nesting */

	CONG_NODE_SELF_TEST(p);

	/* --- Processing for multiple nodes --- */

	if (selection->logical_range.loc0.node != selection->logical_range.loc1.node)
	{
		CongNodePtr prev_node;

		cong_document_begin_edit(doc);
	
		/* Selection is valid, now order first/last nodes */
		
		for (n0 = selection->logical_range.loc0.node; n0 && n0 != selection->logical_range.loc1.node; n0 = n0->next) ;
		
		if (!n0)
		{
			cong_location_copy(&loc0, &selection->logical_range.loc1);
			cong_location_copy(&loc1, &selection->logical_range.loc0);
		}
		else
		{
			cong_location_copy(&loc0, &selection->logical_range.loc0);
			cong_location_copy(&loc1, &selection->logical_range.loc1);
		}

		/* Split, first */

		if (loc0.byte_offset && cong_node_type(loc0.node) == CONG_NODE_TYPE_TEXT)
		{
			prev_node = cong_location_xml_frag_data_nice_split2(doc, &loc0);
			g_assert(prev_node);

			loc0.node = selection->logical_range.loc0.node = prev_node->next;
		} else {
			prev_node = loc0.node;
		}
		
		selection->logical_range.loc0.byte_offset = 0;

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
			selection->logical_range.loc1.node = loc1.node->next;
		}

		selection->logical_range.loc1.byte_offset = 0;

		/* Reparent, last */
		cong_document_node_set_parent(doc, loc1.node, p);

		cong_document_end_edit(doc);
		
		return(prev_node);
	}

	/* --- Processing for single node (loc0.node == loc1.node) --- */

	else
	{
		cong_document_begin_edit(doc);

		/* Sort out the ordering: */
		if (selection->logical_range.loc0.byte_offset < selection->logical_range.loc1.byte_offset)
		{
			cong_location_copy(&loc0,&selection->logical_range.loc0);
			cong_location_copy(&loc1,&selection->logical_range.loc1);
		}
		else
		{
			cong_location_copy(&loc0,&selection->logical_range.loc1);
			cong_location_copy(&loc1,&selection->logical_range.loc0);
		}

		if (cong_node_type(loc0.node) == CONG_NODE_TYPE_TEXT)
		{
			if (loc0.byte_offset == loc1.byte_offset) return(0); /* The end is the beginning is the end */
			
			loc0.node = loc1.node = cong_document_node_split3(doc, loc0.node, loc0.byte_offset, loc1.byte_offset);
		}

		selection->logical_range.loc0.byte_offset = 0;
		selection->logical_range.loc1.byte_offset = 0;
		
		/* Position p where the selection was: */
		if (loc0.node->prev) {
			cong_document_node_add_after(doc, p, loc0.node->prev);
		} else {
			cong_document_node_set_parent(doc, p, loc0.node->parent);
		}
		/* Move the selection below p: */
		cong_document_node_set_parent(doc, selection->logical_range.loc0.node, p);

		cong_document_end_edit(doc);

		/* Return node before p's new position (I think): */
		return p->prev;
	}
}

void cong_selection_delete(CongSelection *selection, CongDocument *doc)
{
	cong_document_delete_range (doc, 
				    cong_selection_get_ordered_range (selection));

	cong_selection_nullify (selection);
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

#endif

void
cong_selection_nullify (CongSelection *selection)
{
	g_return_if_fail (selection);

	cong_range_nullify (&selection->logical_range);
	cong_range_nullify (&selection->ordered_range);
}

gboolean
cong_selection_is_valid (CongSelection *selection)
{
	g_return_val_if_fail (selection, FALSE);

	if (selection->logical_range.loc0.node) {
		if (selection->logical_range.loc1.node) {
			return (selection->logical_range.loc0.node->parent == selection->logical_range.loc1.node->parent);
		}
	}
	
	return FALSE;
}

GdkGC*
cong_selection_legacy_get_gc_valid (CongSelection *selection)
{
	return selection->gc_valid;
}

GdkGC*
cong_selection_legacy_get_gc_invalid (CongSelection *selection)
{
	return selection->gc_invalid;
}

/* Internal function definitions: */
static void
update_ordered_selection (CongSelection *selection)
{
	cong_range_copy (&selection->ordered_range, &selection->logical_range);
	cong_range_make_ordered (&selection->ordered_range);

	/* probabky should have some caching of the result */
}

