/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-flow-holder-inlines.c
 *
 * Copyright (C) 2003 David Malcolm
 *
 * Conglomerate is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Conglomerate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#include "global.h"
#include "cong-editor-area-flow-holder-inlines.h"
#include <libgnome/gnome-macros.h>

#include "cong-editor-area-composer.h"
#include "cong-editor-area-line.h"
#include "cong-editor-child-policy-inline.h"
#include "cong-editor-line-fragments.h"

#include "cong-document.h"

#define DEBUG_LINE_FLOWS 0


#define PRIVATE(x) ((x)->private)

/* FIXME: this should probabky be done by interrogating the font, or something like that */
#define LINE_SPACING (5)

struct CongEditorAreaFlowHolderInlinesDetails
{
	CongEditorAreaComposer *line_composer;
	/* child areas will all be CongEditorAreaLine */

	CongEditorAreaLine *last_line;

	gint last_width;

	/* We keep a list of all the editor nodes that are "directly" in this inline; we keep it in order corresponding to the document nodes: */
	GList *list_of_editor_nodes;

	gulong signal_handler_id_pending_reflow;
};

/* Method implementation prototypes: */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint);

static void
allocate_child_space (CongEditorArea *area);

static CongEditorArea*
for_all (CongEditorArea *editor_area, 
	 CongEditorAreaCallbackFunc func, 
	 gpointer user_data);

static CongEditorChildPolicy*
insert_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
		       CongEditorNode *editor_node);
static void
remove_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
		       CongEditorNode *editor_node);

static void
on_end_edit (CongDocument *doc,
	     CongEditorAreaFlowHolderInlines *area_flow_holder_inlines);

/* Internal utilties: */
static void
add_line_fragments (CongEditorAreaFlowHolderInlines *inlines,
		    CongEditorLineFragments *line_fragments);

static void
do_line_regeneration (CongEditorAreaFlowHolderInlines *inlines);


/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaFlowHolderInlines, 
			cong_editor_area_flow_holder_inlines,
			CongEditorAreaFlowHolder,
			CONG_EDITOR_AREA_FLOW_HOLDER_TYPE );

static void
cong_editor_area_flow_holder_inlines_class_init (CongEditorAreaFlowHolderInlinesClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);
	CongEditorAreaFlowHolderClass *flow_holder_klass = CONG_EDITOR_AREA_FLOW_HOLDER_CLASS(klass);

	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
	area_klass->for_all = for_all;

	flow_holder_klass->insert_areas_for_node = insert_areas_for_node;
	flow_holder_klass->remove_areas_for_node = remove_areas_for_node;
}

static void
cong_editor_area_flow_holder_inlines_instance_init (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines)
{
	area_flow_holder_inlines->private = g_new0(CongEditorAreaFlowHolderInlinesDetails,1);
}

/* Exported function definitions: */
/**
 * cong_editor_area_flow_holder_inlines_construct:
 * @area_flow_holder_inlines:
 * @editor_widget:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_flow_holder_inlines_construct (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines,
					CongEditorWidget3 *editor_widget)
{
	cong_editor_area_flow_holder_construct (CONG_EDITOR_AREA_FLOW_HOLDER(area_flow_holder_inlines),
						editor_widget);

	PRIVATE(area_flow_holder_inlines)->line_composer = CONG_EDITOR_AREA_COMPOSER( cong_editor_area_composer_new (editor_widget,
														     GTK_ORIENTATION_VERTICAL,
														     LINE_SPACING));
#if 0
	PRIVATE(area_flow_holder_inlines)->hash_of_doc_node_to_area = g_hash_table_new (NULL,
										NULL); 
#endif


	cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (area_flow_holder_inlines),
								   CONG_EDITOR_AREA (PRIVATE(area_flow_holder_inlines)->line_composer));

	cong_editor_area_protected_set_parent (CONG_EDITOR_AREA (PRIVATE(area_flow_holder_inlines)->line_composer),
					       CONG_EDITOR_AREA (area_flow_holder_inlines));

	return CONG_EDITOR_AREA (area_flow_holder_inlines);
}

/**
 * cong_editor_area_flow_holder_inlines_new:
 * @editor_widget:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_flow_holder_inlines_new (CongEditorWidget3 *editor_widget)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_flow_holder_inlines_new");
#endif

	return cong_editor_area_flow_holder_inlines_construct
		(g_object_new (CONG_EDITOR_AREA_FLOW_HOLDER_INLINES_TYPE, NULL),
		 editor_widget);
}

/**
 * cong_editor_area_flow_holder_inlines_get_current_line:
 * @area_flow_holder_inlines:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorAreaLine*
cong_editor_area_flow_holder_inlines_get_current_line (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines)
{
	return PRIVATE(area_flow_holder_inlines)->last_line;
}

/**
 * cong_editor_area_flow_holder_inlines_get_line_width:
 * @area_flow_holder_inlines:
 *
 * TODO: Write me
 * Returns:
 */
gint
cong_editor_area_flow_holder_inlines_get_line_width (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines)
{
	return PRIVATE(area_flow_holder_inlines)->last_width;
}

/**
 * cong_editor_area_flow_holder_inlines_get_current_indent:
 * @area_flow_holder_inlines:
 *
 * TODO: Write me
 * Returns:
 */
gint
cong_editor_area_flow_holder_inlines_get_current_indent (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines)
{
	CongEditorAreaLine* line = cong_editor_area_flow_holder_inlines_get_current_line (area_flow_holder_inlines);

	if (line) {
		return cong_editor_area_line_get_width_used (line);
	} else {
		return 0;
	}
}

/**
 * cong_editor_area_flow_holder_inlines_destroy_lines:
 * @area_flow_holder_inlines:
 *
 * TODO: Write me
 */
void
cong_editor_area_flow_holder_inlines_destroy_lines (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(area_flow_holder_inlines));

	cong_editor_area_remove_all_children ( CONG_EDITOR_AREA_CONTAINER(PRIVATE(area_flow_holder_inlines)->line_composer));
	PRIVATE(area_flow_holder_inlines)->last_line = NULL;
}


/**
 * cong_editor_area_flow_holder_inlines_insert_line:
 * @area_flow_holder_inlines:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorAreaLine*
cong_editor_area_flow_holder_inlines_insert_line (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines)
{
	CongEditorAreaLine* new_line = CONG_EDITOR_AREA_LINE( cong_editor_area_line_new (cong_editor_area_get_widget (CONG_EDITOR_AREA(area_flow_holder_inlines)),
											 PRIVATE(area_flow_holder_inlines)->last_width)
							      );

	cong_editor_area_composer_pack_end (PRIVATE(area_flow_holder_inlines)->line_composer,
					    CONG_EDITOR_AREA(new_line),
					    FALSE,
					    TRUE,
					    0
					    );

	PRIVATE(area_flow_holder_inlines)->last_line = new_line;

	return new_line;
}

/**
 * cong_editor_area_flow_holder_inlines_reflow_required:
 * @area_flow_holder_inlines:
 *
 * TODO: Write me
 */
void
cong_editor_area_flow_holder_inlines_reflow_required (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines)
{
	CongDocument *doc ;
#if 1
	
#if DEBUG_LINE_FLOWS
	g_message ("cong_editor_area_flow_holder_inlines_reflow_required");
#endif

	g_return_if_fail (IS_CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(area_flow_holder_inlines));

	/*
	  Amortisation strategy: separate the reflow_required from the do_the_reflow routine.
	  If a reflow_required happens, check to see if the document is within a begin/end edit; if not, process immediately,
	  otherwise, connect to the document end_edit signal, do the reflow then, disconnecting from it when it happens.

	  This ought to be easy, efficient, and stop almost all redundant caclulations.
	 */
	
	doc = cong_editor_area_get_document (CONG_EDITOR_AREA (area_flow_holder_inlines));
	g_assert (doc);

	if (cong_document_is_within_edit (doc)) {
		if (0==PRIVATE (area_flow_holder_inlines)->signal_handler_id_pending_reflow) {
			PRIVATE (area_flow_holder_inlines)->signal_handler_id_pending_reflow = g_signal_connect (G_OBJECT (doc),
														 "end_edit",
														 G_CALLBACK(on_end_edit),
														 area_flow_holder_inlines);
		} /* otherwise you've already got a pending reflow, and we've optimised away some work */
	} else {
		do_line_regeneration (area_flow_holder_inlines);
	}
#else
	/* Test: add three lines: */
	{
		int i;
		for (i=0;i<3;i++) {
			cong_editor_area_flow_holder_inlines_insert_line (area_flow_holder_inlines);
		}
	}
#endif
}

/**
 * cong_editor_area_flow_holder_inlines_get_first_node:
 * @area_flow_holder_inlines:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_area_flow_holder_inlines_get_first_node (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines)
{
	GList *iter = g_list_first(PRIVATE(area_flow_holder_inlines)->list_of_editor_nodes);

	if (iter) {
		return CONG_EDITOR_NODE(iter->data);
	} else {
		return NULL;
	}
}

/**
 * cong_editor_area_flow_holder_inlines_get_final_node:
 * @area_flow_holder_inlines:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorNode*
cong_editor_area_flow_holder_inlines_get_final_node (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines)
{
	GList *iter = g_list_last(PRIVATE(area_flow_holder_inlines)->list_of_editor_nodes);

	if (iter) {
		return CONG_EDITOR_NODE(iter->data);
	} else {
		return NULL;
	}
}

/* Method implementation definitions: */
static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	CongEditorAreaFlowHolderInlines *area_flow_holder_inlines = CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(area);

#if 1
	if (GTK_ORIENTATION_HORIZONTAL==orientation) {
		return width_hint;
	} else {
		g_assert(PRIVATE(area_flow_holder_inlines)->line_composer);

		/* FIXME: do we have a sane way of calculating the height of the inlines yet? */
		return  cong_editor_area_get_requisition (CONG_EDITOR_AREA(PRIVATE(area_flow_holder_inlines)->line_composer),
							  orientation,
							  width_hint);
	}
#else
	if (PRIVATE(area_flow_holder_inlines)->line_composer) {

		return  cong_editor_area_get_requisition (CONG_EDITOR_AREA(PRIVATE(area_flow_holder_inlines)->line_composer),
							  orientation,
							  width_hint);
	} else {
		return 0;
	}
#endif
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaFlowHolderInlines *area_flow_holder_inlines = CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(area);
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);

	if (PRIVATE(area_flow_holder_inlines)->last_width != rect->width) {
		/* Then we need to rebuild the children of this area i.e. the lines.
		   This will generate "requisition cache flush" notifications.
		 */
		
		/* Immediately update the width; hopefully this should stop reentrancy problems. */
		PRIVATE(area_flow_holder_inlines)->last_width = rect->width;


#if DEBUG_LINE_FLOWS
		g_message("regenerating children of an inline flow_holder");
#endif

		cong_editor_area_flow_holder_inlines_reflow_required(area_flow_holder_inlines);
	}

	if (PRIVATE(area_flow_holder_inlines)->line_composer) {
		const GdkRectangle *rect = cong_editor_area_get_window_coords(area);

		cong_editor_area_set_allocation (CONG_EDITOR_AREA(PRIVATE(area_flow_holder_inlines)->line_composer),
						 rect->x,
						 rect->y,
						 rect->width,
						 rect->height);
	}
}

static CongEditorArea*
for_all (CongEditorArea *editor_area, 
	 CongEditorAreaCallbackFunc func, 
	 gpointer user_data)
{
	CongEditorAreaFlowHolderInlines *area_flow_holder_inlines = CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(editor_area);

	if (PRIVATE(area_flow_holder_inlines)->line_composer) {
		if ((*func)(CONG_EDITOR_AREA(PRIVATE(area_flow_holder_inlines)->line_composer), user_data)) {
			return CONG_EDITOR_AREA(PRIVATE(area_flow_holder_inlines)->line_composer);
		}
	}

	return NULL;
}

static void
on_line_regeneration_required (CongEditorNode *editor_node,
			       gpointer user_data)
{
	CongEditorAreaFlowHolderInlines *area_flow_holder_inlines = CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(user_data);

	cong_editor_area_flow_holder_inlines_reflow_required(area_flow_holder_inlines);
}

static CongEditorChildPolicy*
insert_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
		       CongEditorNode *editor_node)
{
	CongEditorAreaFlowHolderInlines *area_flow_holder_inlines = CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(area_flow_holder);
#if 1
	CongEditorNode *editor_node_next = cong_editor_node_get_next (editor_node);

	if (editor_node_next) {
		/* We keep the list of editor nodes in the "document order": */
		GList *iter = g_list_find (PRIVATE(area_flow_holder_inlines)->list_of_editor_nodes,
					   editor_node_next);
		if (iter) {
			PRIVATE(area_flow_holder_inlines)->list_of_editor_nodes = g_list_insert_before (PRIVATE(area_flow_holder_inlines)->list_of_editor_nodes,
													iter,
													editor_node);
		} else {
			/* Couldn't find the node after this one (if any); insert at end of list (perhaps the next editor node is a structural node): */
			PRIVATE(area_flow_holder_inlines)->list_of_editor_nodes = g_list_append (PRIVATE(area_flow_holder_inlines)->list_of_editor_nodes, 
												 editor_node);
		}
	} else {
		PRIVATE(area_flow_holder_inlines)->list_of_editor_nodes = g_list_append (PRIVATE(area_flow_holder_inlines)->list_of_editor_nodes, 
											 editor_node);
	}
		
	g_signal_connect (G_OBJECT(editor_node),
			  "line_regeneration_required",
			  G_CALLBACK(on_line_regeneration_required),
			  area_flow_holder);

	cong_editor_area_flow_holder_inlines_reflow_required(area_flow_holder_inlines);

	return cong_editor_child_policy_inline_new (editor_node,
						    area_flow_holder_inlines);
#else
	CongEditorArea *new_area;
	CongNodePtr doc_node;
	CongEditorArea *prev_node;

	/* Get the editor node to generate its area: */
	new_area = cong_editor_node_generate_area (editor_node);

	doc_node = cong_editor_node_get_node(editor_node);

	g_hash_table_insert (PRIVATE(area_flow_holder_inlines)->hash_of_doc_node_to_area,
			     doc_node,
			     new_area);

	if (doc_node->prev) {
		CongEditorArea *prev_area = g_hash_table_lookup (PRIVATE(area_flow_holder_inlines)->hash_of_doc_node_to_area,
								 doc_node->prev);
		if (prev_area) {
			
			cong_editor_area_composer_pack_after (PRIVATE(area_flow_holder_inlines)->line_composer,
							      new_area,
							      prev_area,
							      FALSE,
							      FALSE,
							      0);

			return cong_editor_widget_create_child_policy_for_node_with_single_area(editor_node,
												new_area);
		}
	}

	/* Insert this area after any already present in the parent's insertion area: */
	cong_editor_area_composer_pack (PRIVATE(area_flow_holder_inlines)->line_composer,
					new_area,
					FALSE,
					FALSE,
					0);

	return cong_editor_widget_create_child_policy_for_node_with_single_area(editor_node,
										new_area);
#endif
}

static void
remove_areas_for_node (CongEditorAreaFlowHolder *area_flow_holder,
		       CongEditorNode *editor_node)
{
	CongEditorAreaFlowHolderInlines *area_flow_holder_inlines = CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(area_flow_holder);

#if 1
	PRIVATE(area_flow_holder_inlines)->list_of_editor_nodes = g_list_remove (PRIVATE(area_flow_holder_inlines)->list_of_editor_nodes, 
										 editor_node);

	cong_editor_area_flow_holder_inlines_reflow_required(area_flow_holder_inlines);
#else
	CongNodePtr doc_node;
	CongEditorArea *area;
	
	doc_node = cong_editor_node_get_node(editor_node);
	area = g_hash_table_lookup  (PRIVATE(area_flow_holder_inlines)->hash_of_doc_node_to_area,
				     doc_node);
	g_assert (area);

	cong_editor_area_container_remove_child (CONG_EDITOR_AREA_CONTAINER(PRIVATE(area_flow_holder_inlines)->line_composer),
						 area);

	g_hash_table_remove (PRIVATE(area_flow_holder_inlines)->hash_of_doc_node_to_area,
			     doc_node);
#endif
}

static void
on_end_edit (CongDocument *doc,
	     CongEditorAreaFlowHolderInlines *inlines)
{
	g_assert (IS_CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(inlines));

	/* Disconnect from signal: */
	g_assert(PRIVATE(inlines)->signal_handler_id_pending_reflow != 0);

	g_signal_handler_disconnect (doc,
				     PRIVATE(inlines)->signal_handler_id_pending_reflow);

	PRIVATE(inlines)->signal_handler_id_pending_reflow = 0;

	/* Regenerate the lines: */
	do_line_regeneration (inlines);
}

/* Internal utilties: */
static void
add_line_fragments (CongEditorAreaFlowHolderInlines *inlines,
		    CongEditorLineFragments *line_fragments)
{
	GList* iter;

	g_assert (IS_CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(inlines));
	g_assert (IS_CONG_EDITOR_LINE_FRAGMENTS(line_fragments));

	for (iter = cong_editor_line_fragments_get_area_list (line_fragments); iter; iter=iter->next) {
		CongEditorArea *area_fragment = CONG_EDITOR_AREA(iter->data);

		CongEditorAreaLine *line = cong_editor_area_flow_holder_inlines_get_current_line (inlines);

		/* Do we have an existing line? */
		if (line) {
			/* Can it hold the next fragment? If not create a new line: */
			gint width_required = cong_editor_area_get_requisition_width (area_fragment,
										      cong_editor_area_flow_holder_inlines_get_line_width (inlines));

			gint width_free = cong_editor_area_line_get_width_free(line);

#if DEBUG_LINE_FLOWS
			g_message ("got existing line with free = %i; width required = %i", 
				   width_free,
				   width_required);
#endif

			if (width_required > width_free) {
				line = cong_editor_area_flow_holder_inlines_insert_line (inlines);
			}
		} else {
#if DEBUG_LINE_FLOWS
			g_message ("no existing line");
#endif

			line = cong_editor_area_flow_holder_inlines_insert_line (inlines);
		} 

		g_assert(line);

		cong_editor_area_container_add_child (CONG_EDITOR_AREA_CONTAINER(line),
						      area_fragment);
	}
}

static void
do_line_regeneration (CongEditorAreaFlowHolderInlines *area_flow_holder_inlines)
{
	CongEditorNode* editor_node_iter;
	CongEditorNode* editor_node_final;

	g_return_if_fail (IS_CONG_EDITOR_AREA_FLOW_HOLDER_INLINES(area_flow_holder_inlines));

	/* Destroy all child areas (the lines); this ought to destroy all of their children */
	cong_editor_area_flow_holder_inlines_destroy_lines (area_flow_holder_inlines);

	/* Iterate through the nodes of this inline, pouring text: */
	{
		editor_node_iter = cong_editor_area_flow_holder_inlines_get_first_node (area_flow_holder_inlines);
		editor_node_final = cong_editor_area_flow_holder_inlines_get_final_node (area_flow_holder_inlines);
		
		while (editor_node_iter) {
			CongEditorLineFragments *line_fragments;
#if DEBUG_LINE_FLOWS
			g_message("generate_line_areas for %s", G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(editor_node_iter)));
#endif

			g_assert(CONG_FLOW_TYPE_INLINE == cong_editor_node_get_flow_type(editor_node_iter));

			line_fragments = cong_editor_node_generate_line_areas_recursive (editor_node_iter,
											 cong_editor_area_flow_holder_inlines_get_line_width (area_flow_holder_inlines),
											 cong_editor_area_flow_holder_inlines_get_current_indent (area_flow_holder_inlines));

			/* Add the line fragments here: */
			add_line_fragments (area_flow_holder_inlines,
					    line_fragments);
			
			g_object_unref (G_OBJECT(line_fragments));
			
			if (editor_node_iter == editor_node_final) {
				break;
			} else {
				editor_node_iter = cong_editor_node_get_next(editor_node_iter);
			}			
		}
	}


	/* RANDOM THOUGHTS FOLLOW: */
	/* OK - we can figure out which nodes we contain; scan backwards to first node... 
	   (should have an API function for this)
	 */
	/* Add all areas for nodes again: */
	/* Iterate through all the nodes... we know the width of the area, we don't yet know what its height ought to be... */
	/* As you reach each node, you know how far across the current line it is, and hence how much to indent the first line.
	   Text nodes can be easily added.  But what about span tags?  Need to pass some sort of context to the span tag, so that
	   they can add their areas?  But what if the span tag hasnt had its areas created yet?
	   Perhaps a span tag should wait until its children create their areas, and then "annotate" them?  Or provide some kind of nested
	   policy for creating child areas...
	   
	   Perhaps the create_areas/remove_areas hooks should be replaced with more generalised on_node_added/on_node_removed hooks???
	*/
	
	/* I think we need a "recursive_pour_lines" routine for editor_nodes, which adds area into the lines of a flow_holder_inline...
	   Though we have to deal with the case where the editor_nodes havent yet been created for the doc_nodes (esp. for the children
	   of a node).
	   We probably need some sort of caching, so that the inlines only get generated once everything has stabilised (because it's so
	   expensive to deal with child changes)...
	*/
	
	/*
	  But which should do it?  This is an area, not a node.  Perhaps we should have a signal "reflow_required" which the editor_node listens to, 
	  and regenerates the children accordingly?  Hopefully this means we can amortise changes somehow...
	*/
	
	/* standard handler might be for the block holder, because it knows exactly which nodes are "in" the inline...*/
	/* it will empty the existing lines, then iterate through the top-level nodes "in" the inline, recursively asking them to add stuff to the lines.
	   Could have an optimisation in which adding stuff at the end of the inlines doesn't require a reflow, although the extent to which this gets used in practice might be negligible.*/

}
