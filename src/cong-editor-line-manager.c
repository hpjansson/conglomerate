/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-line-manager.c
 *
 * Copyright (C) 2004 David Malcolm
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
#include "cong-editor-line-manager.h"
#include "cong-editor-line-iter.h"
#include "cong-editor-area.h"
#include "cong-editor-node.h"
#include "cong-editor-creation-record.h"
#include "cong-eel.h"

#define DEBUG_LOG 0

#if 0
#define LOG_EVENT(msg,node) log_event (msg, node)
static void log_event (const gchar *msg, 
		       CongEditorNode *editor_node);

static void log_event (const gchar *msg, 
		       CongEditorNode *editor_node)
{
	gchar *desc = cong_node_debug_description (cong_editor_node_get_node (editor_node));

	g_message ("%s: %s", msg, desc);

	g_free (desc);
}
#else
#define LOG_EVENT(msg,node) ((void)0)
#endif

struct CongEditorLineManagerPrivate
{
	CongEditorWidget3* widget;

	/* Mapping from editor_node to per_node data: */
	GHashTable *hash_of_editor_node_to_data;
};

CONG_DEFINE_CLASS_BEGIN (CongEditorLineManager, cong_editor_line_manager, CONG_EDITOR_LINE_MANAGER, GObject, G_TYPE_OBJECT)
CONG_DEFINE_CLASS_END ()

CongAreaCreationGeometry*
cong_area_creation_geometry_new (CongEditorLineManager *line_manager,
			      CongEditorLineIter *line_iter);

void
cong_area_creation_geometry_free (CongAreaCreationGeometry *creation_geometry);

CongAreaCreationGeometry*
cong_area_creation_geometry_new (CongEditorLineManager *line_manager,
			      CongEditorLineIter *line_iter)
{
	CongAreaCreationGeometry *creation_geometry;
	
	g_return_val_if_fail (line_manager, NULL);
	g_return_val_if_fail (line_iter, NULL);
	
	creation_geometry = g_new0 (CongAreaCreationGeometry,1);

	creation_geometry->area_line = cong_editor_line_iter_get_line (line_iter);
	if (creation_geometry->area_line) {
		g_object_ref (G_OBJECT (creation_geometry->area_line));
	}
	creation_geometry->line_width = cong_editor_line_manager_get_line_width (line_manager,
									      line_iter);
	creation_geometry->line_indent = cong_editor_line_manager_get_current_indent (line_manager,
										   line_iter);
	return creation_geometry;	
}

void
cong_area_creation_geometry_free (CongAreaCreationGeometry *creation_geometry)
{
	g_return_if_fail (creation_geometry);

	if (creation_geometry->area_line) {
		g_object_unref (G_OBJECT (creation_geometry->area_line));
	}
	g_free (creation_geometry);
}


/* Data stored about each editor node: */
typedef struct PerNodeData PerNodeData;
struct PerNodeData
{
	CongAreaCreationGeometry *start_creation_geometry;
	CongAreaCreationGeometry *end_creation_geometry;

	/* Record of insertion position at the start of this node: */
	CongEditorLineIter *start_line_iter;

	/* Insertion position for areas _after_ this node: */
	CongEditorLineIter *end_line_iter;

	CongEditorCreationRecord *creation_record;

	gulong signal_handler_id;
};

/* Internal function declarations: */
static void
hash_value_destroy_func (gpointer data);

static PerNodeData*
get_data_for_node (CongEditorLineManager *line_manager,
		   CongEditorNode *editor_node);

static void
create_areas_for_node (CongEditorLineManager *line_manager,
		       CongEditorNode *editor_node,
		       CongEditorLineIter *start_line_iter);

static void
destroy_areas_for_node (CongEditorLineManager *line_manager,
			CongEditorNode *editor_node);

static void
regenerate_areas_for_node (CongEditorLineManager *line_manager,
			   CongEditorNode *editor_node,
			   CongEditorLineIter *start_line_iter);

static void
regenerate_successor_nodes (CongEditorLineManager *line_manager,
			    CongEditorNode *successor_node,
			    CongEditorLineIter *new_start_iter,
			    CongAreaCreationGeometry *new_start_creation_geometry);

static void
on_line_regeneration_required (CongEditorNode *editor_node,
			       gpointer user_data);


/* Exported function implementations: */
void
cong_editor_line_manager_construct (CongEditorLineManager *line_manager,
				    CongEditorWidget3 *widget)
{
	g_return_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager));

	PRIVATE (line_manager)->widget = widget;
	PRIVATE (line_manager)->hash_of_editor_node_to_data = g_hash_table_new_full (g_direct_hash,
										     g_direct_equal,
										     NULL,
										     hash_value_destroy_func);
}

static void 
cong_editor_line_manager_dispose (GObject *object)
{
	CongEditorLineManager *line_manager = CONG_EDITOR_LINE_MANAGER (object);

	g_hash_table_destroy (PRIVATE (line_manager)->hash_of_editor_node_to_data);
}

CongEditorWidget3*
cong_editor_line_manager_get_widget (CongEditorLineManager *line_manager)
{
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager), NULL);

	return PRIVATE (line_manager)->widget;
}

void 
cong_editor_line_manager_add_node (CongEditorLineManager *line_manager,
				   CongEditorNode *editor_node)
{
	CongEditorNode *editor_node_prev = cong_editor_node_get_prev (editor_node);
	PerNodeData *per_node_data;
	CongEditorLineIter *start_line_iter;

	/* Set up per_node_data for the new editor node: */
	{
		per_node_data = g_new0 (PerNodeData, 1);

		/* We're done; add this to the hash table: */
		g_hash_table_insert (PRIVATE (line_manager)->hash_of_editor_node_to_data,
				     editor_node,
				     per_node_data);
	}


	/* Set up the line_iter: */
	{
		if (editor_node_prev) {
			PerNodeData *per_node_data_prev = get_data_for_node (line_manager,
									     editor_node_prev);
			
			start_line_iter = cong_editor_line_iter_clone (per_node_data_prev->end_line_iter);
		} else {
			/* We have a start node; use the factory method: */
			start_line_iter = CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_LINE_MANAGER_CLASS,
										  line_manager,
										  make_iter,
										  (line_manager));
		}
	}
	
	per_node_data->signal_handler_id = g_signal_connect (G_OBJECT(editor_node),
							     "line_regeneration_required",
							     G_CALLBACK(on_line_regeneration_required),
							     line_manager);
	g_object_ref (G_OBJECT (line_manager));

	LOG_EVENT ("create areas for new node", editor_node);


	create_areas_for_node (line_manager,
			       editor_node,
			       start_line_iter);

	/* Potentially regenerate successors; starting at this new node's end point: */
	regenerate_successor_nodes (line_manager,
				    cong_editor_node_get_next (editor_node),
				    per_node_data->end_line_iter,
				    per_node_data->end_creation_geometry);
}

void
cong_editor_line_manager_remove_node (CongEditorLineManager *line_manager,
				      CongEditorNode *editor_node)
{
	PerNodeData *per_node_data;

	g_return_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager));
	g_return_if_fail (IS_CONG_EDITOR_NODE (editor_node));

	LOG_EVENT ("remove areas for dead node", editor_node);

	per_node_data = get_data_for_node (line_manager,
					   editor_node);
	g_assert (per_node_data);

	/* Disconnect from signals: */
	g_assert (per_node_data->signal_handler_id);
	g_signal_handler_disconnect (G_OBJECT(editor_node),
				     per_node_data->signal_handler_id);
	per_node_data->signal_handler_id = 0;
	g_object_unref (G_OBJECT (line_manager)); /* we added a reference with the signal */

	/* Destroy areas: */
	destroy_areas_for_node (line_manager,
				editor_node);

	/* Potentially regenerate successors; starting at this node's old start point: */
	regenerate_successor_nodes (line_manager,
				    cong_editor_node_get_next (editor_node),
				    per_node_data->start_line_iter,
				    per_node_data->start_creation_geometry);

	/* Remove from hash: */
	{
		g_hash_table_remove (PRIVATE (line_manager)->hash_of_editor_node_to_data,
				     editor_node);
	}
}

void
cong_editor_line_manager_begin_line (CongEditorLineManager *line_manager,
				     CongEditorCreationRecord *creation_record,
				     CongEditorLineIter *line_iter)
{
	CongEditorLineIter *iter_before;
	CongEditorLineIter *iter_after;

	g_return_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager));
	g_return_if_fail (IS_CONG_EDITOR_CREATION_RECORD (creation_record));
	g_return_if_fail (line_iter);

	iter_before = cong_editor_line_iter_clone (line_iter);

	CONG_EEL_CALL_METHOD (CONG_EDITOR_LINE_MANAGER_CLASS,
			      line_manager,
			      begin_line, 
			      (line_manager, creation_record, line_iter));

	iter_after = cong_editor_line_iter_clone (line_iter);

	/* Record the change: */
	cong_editor_creation_record_add_change (creation_record,
						CONG_EDITOR_CREATION_EVENT_BEGIN_LINE,
						iter_before,
						iter_after);
	g_object_unref (G_OBJECT (iter_before));
	g_object_unref (G_OBJECT (iter_after));

	/* We should now have a line at the iter: */
	g_assert (cong_editor_line_iter_get_line (line_iter));
}

void
cong_editor_line_manager_add_to_line (CongEditorLineManager *line_manager,
				      CongEditorCreationRecord *creation_record,
				      CongEditorLineIter *line_iter,
				      CongEditorArea *area)
{
	CongEditorLineIter *iter_before;
	CongEditorLineIter *iter_after;

	g_return_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager));
	g_return_if_fail (IS_CONG_EDITOR_CREATION_RECORD (creation_record));
	g_return_if_fail (line_iter);
	g_return_if_fail (IS_CONG_EDITOR_AREA (area));

	/* Line wrapping: */
	{
		/* The areas we are getting should hopefully either be exactly the correct size to line-wrap appropriately,
		   or should stop short due to things like span tags starting/stopping, or simply due to running out of content etc: */
		gint width_available = cong_editor_line_manager_get_current_width_available (line_manager, 
											     line_iter);
		gint area_width = cong_editor_area_get_requisition_width (area,
									  width_available);
		if (area_width>width_available) {
			/* Force a line-break: */
			cong_editor_line_manager_end_line (line_manager,
							   creation_record,
							   line_iter);
		}
	}

	iter_before = cong_editor_line_iter_clone (line_iter);

	CONG_EEL_CALL_METHOD (CONG_EDITOR_LINE_MANAGER_CLASS,
			      line_manager,
			      add_to_line, 
			      (line_manager, creation_record, line_iter, area));

	iter_after = cong_editor_line_iter_clone (line_iter);

	/* Record the change: */
	cong_editor_creation_record_add_change (creation_record,
						CONG_EDITOR_CREATION_EVENT_ADD_AREA,
						iter_before,
						iter_after);
	g_object_unref (G_OBJECT (iter_before));
	g_object_unref (G_OBJECT (iter_after));

}

void
cong_editor_line_manager_end_line (CongEditorLineManager *line_manager,
				   CongEditorCreationRecord *creation_record,
				   CongEditorLineIter *line_iter)
{
	CongEditorLineIter *iter_before;
	CongEditorLineIter *iter_after;

	g_return_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager));
	g_return_if_fail (IS_CONG_EDITOR_CREATION_RECORD (creation_record));
	g_return_if_fail (line_iter);

	iter_before = cong_editor_line_iter_clone (line_iter);

	CONG_EEL_CALL_METHOD (CONG_EDITOR_LINE_MANAGER_CLASS,
			      line_manager,
			      end_line,
			      (line_manager, creation_record, line_iter));

	iter_after = cong_editor_line_iter_clone (line_iter);

	/* Record the change: */
	cong_editor_creation_record_add_change (creation_record,
						CONG_EDITOR_CREATION_EVENT_END_LINE,
						iter_before,
						iter_after);
	g_object_unref (G_OBJECT (iter_before));
	g_object_unref (G_OBJECT (iter_after));
}

#if 1
#if DEBUG_LOG
static const gchar*
get_string_for_event_type (enum CongEditorCreationEvent event)
{
	switch (event) {
	default: g_assert_not_reached ();
	case CONG_EDITOR_CREATION_EVENT_BEGIN_LINE:
		return "BEGIN_LINE";

	case CONG_EDITOR_CREATION_EVENT_END_LINE:
		return "END_LINE";

	case CONG_EDITOR_CREATION_EVENT_ADD_AREA:
		return "ADD_AREA";
	}
}
#endif

void
cong_editor_line_manager_undo_change (CongEditorLineManager *line_manager,
				      enum CongEditorCreationEvent event,
				      CongEditorLineIter *iter_before,
				      CongEditorLineIter *iter_after)
{
	g_return_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager));
	g_return_if_fail (iter_before);
	g_return_if_fail (iter_after);

#if DEBUG_LOG
	g_message ("%s::undo_change (%s)", G_OBJECT_TYPE_NAME (G_OBJECT (line_manager)), get_string_for_event_type (event));
#endif

	CONG_EEL_CALL_METHOD (CONG_EDITOR_LINE_MANAGER_CLASS,
			      line_manager,
			      undo_change, 
			      (line_manager, 
			       event,
			       iter_before, 
			       iter_after));


}
#else
void
cong_editor_line_manager_delete_areas (CongEditorLineManager *line_manager,
				       CongEditorLineIter *start_iter,
				       CongEditorLineIter *end_iter)
{
	g_return_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager));
	g_return_if_fail (start_iter);
	g_return_if_fail (end_iter);

	CONG_EEL_CALL_METHOD (CONG_EDITOR_LINE_MANAGER_CLASS,
			      line_manager,
			      delete_areas, 
			      (line_manager, 
			       start_iter, 
			       end_iter));


}
#endif

gint
cong_editor_line_manager_get_line_width (CongEditorLineManager *line_manager,
					 CongEditorLineIter *line_iter)
{
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager), 0);
	g_return_val_if_fail (line_iter, 0);

	return CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_LINE_MANAGER_CLASS,
						       line_manager,
						       get_line_width,
						       (line_manager, line_iter));
}

gint
cong_editor_line_manager_get_current_indent (CongEditorLineManager *line_manager,
					     CongEditorLineIter *line_iter)
{
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager), 0);
	g_return_val_if_fail (line_iter, 0);

	return CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE (CONG_EDITOR_LINE_MANAGER_CLASS,
						       line_manager,
						       get_current_indent,
						       (line_manager, line_iter));
}

gint
cong_editor_line_manager_get_current_width_available (CongEditorLineManager *line_manager,
						      CongEditorLineIter *line_iter)
{
	gint line_width;
	gint current_indent;

	g_return_val_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager), 0);
	g_return_val_if_fail (line_iter, 0);

	line_width = cong_editor_line_manager_get_line_width (line_manager, line_iter);
	current_indent = cong_editor_line_manager_get_current_indent (line_manager, line_iter);

	return line_width - current_indent;
}

/* Internal function implementations: */
static void
hash_value_destroy_func (gpointer data)
{
	PerNodeData *per_node_data = (PerNodeData*)data;

	if (per_node_data->start_creation_geometry) {
		cong_area_creation_geometry_free (per_node_data->start_creation_geometry);
		per_node_data->start_creation_geometry = NULL;
	}

	if (per_node_data->end_creation_geometry) {
		cong_area_creation_geometry_free (per_node_data->end_creation_geometry);
		per_node_data->end_creation_geometry = NULL;
	}

	if (per_node_data->start_line_iter) {
		g_object_unref (G_OBJECT (per_node_data->start_line_iter));
		per_node_data->start_line_iter = NULL;
	}

	if (per_node_data->end_line_iter) {
		g_object_unref (G_OBJECT (per_node_data->end_line_iter));
		per_node_data->end_line_iter = NULL;
	}

	if (per_node_data->creation_record) {
		g_object_unref (G_OBJECT (per_node_data->creation_record));
		per_node_data->creation_record = NULL;
	}

	g_free (per_node_data);
}

static PerNodeData*
get_data_for_node (CongEditorLineManager *line_manager,
		   CongEditorNode *editor_node)
{	
	g_return_val_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager), NULL);
	g_return_val_if_fail (IS_CONG_EDITOR_NODE (editor_node), NULL);

	return (PerNodeData*)g_hash_table_lookup (PRIVATE (line_manager)->hash_of_editor_node_to_data,
						  editor_node);
}

static void
create_areas_for_node (CongEditorLineManager *line_manager,
		       CongEditorNode *editor_node,
		       CongEditorLineIter *start_line_iter)
{
	PerNodeData *per_node_data;

	g_return_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager));
	g_return_if_fail (IS_CONG_EDITOR_NODE (editor_node));

	/* Ensure arguments stay "live": */
	g_object_ref (G_OBJECT (start_line_iter));

	per_node_data = get_data_for_node (line_manager,
					   editor_node);
	g_assert (per_node_data);

	/* Set up creation record: */
	g_assert (NULL==per_node_data->creation_record);
	per_node_data->creation_record = cong_editor_creation_record_new (line_manager);

	/* Set up line iters: */
	{
		if (per_node_data->start_line_iter) {
			g_object_unref (G_OBJECT (per_node_data->start_line_iter));		
		}
		per_node_data->start_line_iter = start_line_iter;
		g_object_ref (G_OBJECT (per_node_data->start_line_iter));
		
		if (per_node_data->end_line_iter) {
			g_object_unref (G_OBJECT (per_node_data->end_line_iter));		
		}
		per_node_data->end_line_iter = cong_editor_line_iter_clone (start_line_iter);
	}
		
	/* Set up start_creation_geometry: */
	if (per_node_data->start_creation_geometry) {
		cong_area_creation_geometry_free (per_node_data->start_creation_geometry);
	}
	per_node_data->start_creation_geometry = cong_area_creation_geometry_new (line_manager,
										  per_node_data->end_line_iter);

	/* Invoke "create_areas" method for node: */
	{
		CongAreaCreationInfo creation_info;
		
		creation_info.line_manager = line_manager;
		creation_info.creation_record = per_node_data->creation_record;
		creation_info.line_iter = per_node_data->end_line_iter; /* note that this will be modified */
		
		CONG_EEL_CALL_METHOD (CONG_EDITOR_NODE_CLASS,
				      editor_node,
				      create_areas,
				      (editor_node, &creation_info));
	}

	/* Set up end_creation_geometry: */
	if (per_node_data->end_creation_geometry) {
		cong_area_creation_geometry_free (per_node_data->end_creation_geometry);
	}
	per_node_data->end_creation_geometry = cong_area_creation_geometry_new (line_manager,
										per_node_data->end_line_iter);

	/* Finished ensuring arguments stay "live": */
	g_object_unref (G_OBJECT (start_line_iter));
}

static void
destroy_areas_for_node (CongEditorLineManager *line_manager,
			CongEditorNode *editor_node)
{
	PerNodeData *per_node_data;

	g_return_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager));
	g_return_if_fail (IS_CONG_EDITOR_NODE (editor_node));

	per_node_data = get_data_for_node (line_manager,
					   editor_node);
	g_assert (per_node_data);

	g_assert (per_node_data->creation_record);
	g_assert (per_node_data->start_creation_geometry);
	g_assert (per_node_data->end_creation_geometry);

	/* Delete all areas recorded for this node: */
	cong_editor_creation_record_undo_changes (per_node_data->creation_record);
	g_object_unref (G_OBJECT (per_node_data->creation_record));
	per_node_data->creation_record = NULL;
}

static void
regenerate_areas_for_node (CongEditorLineManager *line_manager,
			   CongEditorNode *editor_node,
			   CongEditorLineIter *start_line_iter)
{
	g_return_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager));
	g_return_if_fail (IS_CONG_EDITOR_NODE (editor_node));

	LOG_EVENT ("regenerating areas for node", editor_node);

	/* Regenerate areas for this node: */
	destroy_areas_for_node (line_manager,
				editor_node);
	create_areas_for_node (line_manager,
			       editor_node,
			       start_line_iter);
}

static void
regenerate_successor_nodes (CongEditorLineManager *line_manager,
			    CongEditorNode *successor_node,
			    CongEditorLineIter *new_start_iter,
			    CongAreaCreationGeometry *new_start_creation_geometry)
{
	/* Potentially update successor nodes' area creation info, recreating areas as necessary, which may trigger further updates: */
	g_return_if_fail (IS_CONG_EDITOR_LINE_MANAGER (line_manager));
	g_return_if_fail (IS_CONG_EDITOR_LINE_ITER (new_start_iter));
	g_return_if_fail (new_start_creation_geometry);

	while (successor_node) {
		PerNodeData *per_node_data_successor = get_data_for_node (line_manager,
									  successor_node);	
		g_assert (per_node_data_successor);

		/* Compare old and new line iters for the successor: */
		if (cong_editor_node_needs_area_regeneration (successor_node,
							      per_node_data_successor->start_creation_geometry,
							      new_start_creation_geometry)) {
			/* Do the regeneration: */
			regenerate_areas_for_node (line_manager,
						   successor_node,
						   new_start_iter);

			/* Iterate onto next node: */
			new_start_iter = per_node_data_successor->end_line_iter;
			new_start_creation_geometry = per_node_data_successor->end_creation_geometry;
			successor_node = cong_editor_node_get_next (successor_node);

			/* FIXME: what about subtrees; should we recurse??? */
		} else {
			/* no regeneration necessary for this node; stop the iteration: */
			return;
		}
	}
}

static void
on_line_regeneration_required (CongEditorNode *editor_node,
			       gpointer user_data)
{
	CongEditorLineManager *line_manager = CONG_EDITOR_LINE_MANAGER (user_data);
	PerNodeData *per_node_data;

	per_node_data = get_data_for_node (line_manager,
					   editor_node);
	g_assert (per_node_data);

	LOG_EVENT ("on_line_regeneration_required for node", editor_node);

	/* 
	   We always regenerate this node's area: the subclass asked for it (e.g. the user has typed text and changed the content of a CongEditorNodeText)
	 */
	regenerate_areas_for_node (line_manager,
				   editor_node,
				   per_node_data->start_line_iter);

	/* Potentially regenerate successors: */
	regenerate_successor_nodes (line_manager,
				    cong_editor_node_get_next (editor_node),
				    per_node_data->end_line_iter,
				    per_node_data->end_creation_geometry);
}
