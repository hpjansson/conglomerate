/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>

#include "global.h"
#include "cong-document.h"
#include "cong-error-dialog.h"

#include "cong-selection.h"
#include "cong-ui-hooks.h"

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
/**
 * cong_selection_new:
 *
 * Returns: a new #CongSelection
 */
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

/**
 * cong_selection_free:
 * @selection:
 *
 * TODO: Write me
 */
void 
cong_selection_free (CongSelection *selection)
{	
	if (selection->gc_valid) {
		g_object_unref (G_OBJECT (selection->gc_valid));
		selection->gc_valid = NULL;
	} 
	if (selection->gc_invalid) {
		g_object_unref (G_OBJECT (selection->gc_invalid));
		selection->gc_invalid = NULL;
	}
}

/**
 * cong_selection_start_from_curs:
 * @selection:
 * @curs:
 *
 * TODO: Write me
 */
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

/**
 * cong_selection_end_from_curs:
 * @selection:
 * @curs:
 *
 * TODO: Write me
 */
void 
cong_selection_end_from_curs (CongSelection *selection, 
			      CongCursor *curs)
{
	g_assert(selection!=NULL);
	g_assert(curs!=NULL);

	cong_location_copy(&selection->logical_range.loc1, &curs->location);

	update_ordered_selection (selection);
}

/**
 * cong_selection_get_logical_range:
 * @selection:
 *
 * TODO: Write me
 * Returns:
 */
CongRange*
cong_selection_get_logical_range (CongSelection *selection)
{
	g_return_val_if_fail (selection, NULL);

	return &selection->logical_range;
}

/**
 * cong_selection_get_ordered_range:
 * @selection:
 *
 * TODO: Write me
 * Returns:
 */
CongRange*
cong_selection_get_ordered_range (CongSelection *selection)
{
	g_return_val_if_fail (selection, NULL);

	/* FIXME: should we call this here: */
	update_ordered_selection (selection);

	return &selection->ordered_range;
}

/**
 * cong_selection_get_logical_start:
 * @selection:
 *
 * TODO: Write me
 * Returns:
 */
CongLocation*
cong_selection_get_logical_start (CongSelection *selection)
{
	g_return_val_if_fail (selection, NULL);

	return &(selection->logical_range.loc0);
}

/**
 * cong_selection_get_logical_end:
 * @selection:
 *
 * TODO: Write me
 * Returns:
 */
CongLocation*
cong_selection_get_logical_end (CongSelection *selection)
{
	g_return_val_if_fail (selection, NULL);

	return &(selection->logical_range.loc1);
}

/**
 * cong_selection_get_ordered_start:
 * @selection:
 *
 * TODO: Write me
 * Returns:
 */
CongLocation*
cong_selection_get_ordered_start (CongSelection *selection)
{
	g_return_val_if_fail (selection, NULL);

	/* FIXME: should we call this here: */
	update_ordered_selection (selection);

	return &(selection->ordered_range.loc0);
}

/**
 * cong_selection_get_ordered_end:
 * @selection:
 *
 * TODO: Write me
 * Returns:
 */
CongLocation*
cong_selection_get_ordered_end (CongSelection *selection)
{
	g_return_val_if_fail (selection, NULL);

	/* FIXME: should we call this here: */
	update_ordered_selection (selection);

	return &(selection->ordered_range.loc1);
}

/**
 * cong_selection_set_logical_start:
 * @selection:
 * @location:
 *
 * TODO: Write me
 */
void
cong_selection_set_logical_start (CongSelection *selection,
				  const CongLocation *location)
{	
	g_assert(selection!=NULL);
	g_assert(location!=NULL);

	cong_location_copy(&selection->logical_range.loc0, location);

	update_ordered_selection (selection);

}

/**
 * cong_selection_set_logical_end:
 * @selection:
 * @location:
 *
 * TODO: Write me
 */
void
cong_selection_set_logical_end (CongSelection *selection,
				const CongLocation *location)
{
	g_assert(selection!=NULL);
	g_assert(location!=NULL);

	cong_location_copy(&selection->logical_range.loc1, location);

	update_ordered_selection (selection);
}

/**
 * cong_selection_set_logical_range:
 * @selection:
 * @start_loc:
 * @end_loc:
 *
 * TODO: Write me
 */
void
cong_selection_set_logical_range (CongSelection *selection,
				  const CongLocation *start_loc,
				  const CongLocation *end_loc)
{
	g_assert (selection!=NULL);
	g_assert (start_loc!=NULL);
	g_assert (end_loc!=NULL);

	cong_location_copy (&selection->logical_range.loc0, start_loc);
	cong_location_copy (&selection->logical_range.loc1, end_loc);

	update_ordered_selection (selection);
}


#if 1

/**
 * cong_selection_import:
 * @selection:
 * @widget:
 *
 * TODO: Write me
 */
void 
cong_selection_import(CongSelection *selection, GtkWidget* widget)
{
	gtk_selection_convert(widget, GDK_SELECTION_PRIMARY,
			      gdk_atom_intern("STRING", FALSE), GDK_CURRENT_TIME);

#ifndef RELEASE	
	printf("In cong_selection_import().\n");
#endif
	
}

/**
 * cong_selection_claim:
 * @selection:
 *
 * TODO: Write me
 */
void 
cong_selection_claim(CongSelection *selection)
{

#if 0	
	gint gtk_selection_owner_set(GtkWidget *widget,
															 GdkAtom    selection,
															 guint32    time);


	
#endif
}

#endif

/**
 * cong_selection_is_node:
 * @selection:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_selection_is_node (CongSelection *selection,
			CongNodePtr node)
{
	g_return_val_if_fail (selection, FALSE);

	return cong_range_is_node (&selection->logical_range,
				   node);
}

/**
 * cong_selection_nullify:
 * @selection:
 *
 * TODO: Write me
 */
void
cong_selection_nullify (CongSelection *selection)
{
	g_return_if_fail (selection);

	cong_range_nullify (&selection->logical_range);
	cong_range_nullify (&selection->ordered_range);
}

/**
 * cong_selection_is_valid:
 * @selection:
 *
 * TODO: Write me
 * Returns:
 */
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

/**
 * cong_selection_legacy_get_gc_valid:
 * @selection:
 *
 * TODO: Write me
 * Returns:
 */
GdkGC*
cong_selection_legacy_get_gc_valid (CongSelection *selection)
{
	return selection->gc_valid;
}

/**
 * cong_selection_legacy_get_gc_invalid:
 * @selection:
 *
 * TODO: Write me
 * Returns:
 */
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
  
/**
 * cong_selection_get_selected_text:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
gchar* 
cong_selection_get_selected_text (CongDocument *doc)
{
   CongSelection *selection;
   CongCursor *curs;

   g_return_val_if_fail(doc, NULL);
 
   selection = cong_document_get_selection(doc);
   curs = cong_document_get_cursor(doc);
	
   if (!cong_location_exists(&curs->location)) return NULL;
	
   if (!(cong_range_exists (cong_selection_get_logical_range (selection)) &&
	      cong_range_is_valid (cong_selection_get_logical_range (selection)))) { 
		return NULL;
   }

   if (cong_range_is_empty (cong_selection_get_logical_range (selection))) {
      return NULL;
   }

   if (!cong_range_can_be_copied (cong_selection_get_ordered_range (selection))) {
      return NULL;
   }

   return cong_range_generate_source (cong_selection_get_ordered_range (selection));
}

