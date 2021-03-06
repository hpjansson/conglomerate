/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-command.c
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
#include "cong-command.h"
#include "cong-eel.h"
#include "cong-util.h"

#include "cong-app.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-command.h"
#include "cong-modification.h"
#include "cong-object.h"

#include "cong-error-dialog.h"

/* Headers for the various atomic modifications: */
#include "cong-node-modification.h"
#include "cong-node-modification-make-orphan.h"
#include "cong-node-modification-add-after.h"
#include "cong-node-modification-add-before.h"
#include "cong-node-modification-set-parent.h"
#include "cong-node-modification-set-text.h"
#include "cong-node-modification-set-attribute.h"
#include "cong-node-modification-remove-attribute.h"
#include "cong-modification-selection-change.h"
#include "cong-modification-cursor-change.h"
#include "cong-modification-set-dtd-ptr.h"

/* Headers for the various compound modifications: */
#include "cong-range.h"
#include "cong-selection.h"

#define DEBUG_MODIFICATIONS 0

struct _CongCommandDetails
{
	CongDocument *doc;
	gchar *description;
	gchar *consolidation_id;
	gboolean has_ever_been_undone;
	GList *list_of_modification;

	gboolean doc_modified_before;
};

/* Exported function definitions: */
G_DEFINE_TYPE(CongCommand,
              cong_command,
              G_TYPE_OBJECT);

static void
cong_command_class_init (CongCommandClass *klass)
{
}

static void
cong_command_init (CongCommand *node)
{
	node->priv = g_new0(CongCommandDetails,1);
}

/**
 * cong_command_construct:
 * @command:
 * @doc:
 * @description:
 * @consolidation_id:
 *
 * TODO: Write me
 * Returns:
 */
CongCommand*
cong_command_construct (CongCommand *command,
			CongDocument *doc,
			const gchar *description,
			const gchar *consolidation_id)
{
	PRIVATE(command)->doc = doc;
	PRIVATE(command)->description = g_strdup (description);
	if (consolidation_id) {
		PRIVATE(command)->consolidation_id = g_strdup (consolidation_id);
	}
	PRIVATE(command)->has_ever_been_undone = FALSE;
	PRIVATE(command)->doc_modified_before = cong_document_is_modified (doc);

	return command;
}

/**
 * cong_command_private_new:
 * @doc: The #CongDocument upon which the command is to act.
 * @description: Human-readable, translated name for this command, as it will appear in the undo/redo history
 * widget
 * @consolidation_id: A string ID (or NULL) for this command to allow multiple similar commands to be consolidated into 
 * a single command.  For example, multiple characters being typed at the keboard can be merged into a single "Typing" command.
 * 
 * Should only be called by the internals of #CongDocument; if you wish to create a #CongCommand you should call cong_document_begin_command()
 * instead.
 *
 * Returns:  the new #CongCommand
 *
 */
CongCommand*
cong_command_private_new (CongDocument *doc,
			  const gchar *description,
			  const gchar *consolidation_id)
{
	return cong_command_construct (CONG_COMMAND(g_object_new (CONG_COMMAND_TYPE, NULL)),
				       doc,
				       description,
				       consolidation_id);
}

/**
 * cong_command_get_document:
 * @command:
 *
 * TODO: Write me
 * Returns:
 */
CongDocument*
cong_command_get_document (CongCommand *command)
{
	g_return_val_if_fail (IS_CONG_COMMAND(command), NULL);
	
	return PRIVATE(command)->doc;
}

/**
 * cong_command_get_description:
 * @command: a command
 *
 * Returns: the human-readable description of this command
 */
const gchar*
cong_command_get_description (CongCommand *command)
{
	g_return_val_if_fail (IS_CONG_COMMAND(command), NULL);
	
	return PRIVATE(command)->description;
}

/**
 * cong_command_get_consolidation_id:
 * @command:  The relevant #CongCommand 
 *
 * Gets the ID (or NULL) of the command used for consolidating multiple similar operations into a single entry in the undo/redo history
 *
 * Returns: a constant string, or NULL if no merging is to occur
 */
const gchar*
cong_command_get_consolidation_id (CongCommand *command)
{
	g_return_val_if_fail (IS_CONG_COMMAND(command), NULL);
	
	return PRIVATE(command)->consolidation_id;
}

/**
 * cong_command_undo:
 * @command: a command
 *
 * Undoes the command.  All modifications contained within the command are undone from the document (in reverse order), the document's "is-modified" flag is set to whatever it was when the command was created.
 */
void
cong_command_undo (CongCommand *command)
{
	GList *iter;
	CongDocument* doc;

	g_return_if_fail (IS_CONG_COMMAND(command));

	/* g_message ("cong_command_undo(\"%s\")", cong_command_get_description(command)); */

	PRIVATE(command)->has_ever_been_undone = TRUE;

	doc = cong_command_get_document (command);

	cong_document_begin_edit (doc);

	/* Start at end of modification list, iterate backwards up to front: */
	for (iter = g_list_last(PRIVATE(command)->list_of_modification); iter; iter=iter->prev) {
		CongModification *modification = CONG_MODIFICATION (iter->data);

#if DEBUG_MODIFICATIONS
		g_message ("undoing modification %s", G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(modification)));
#endif

		CONG_EEL_CALL_METHOD (CONG_MODIFICATION_CLASS, 
				      modification, 
				      undo, 
				      (modification));
	}

	cong_document_set_modified (doc,
				    PRIVATE(command)->doc_modified_before);

	cong_document_end_edit (doc);
}

/**
 * cong_command_redo:
 * @command:
 *
 * Redoes a command that has previously been undone.  Replays all the modifications on the document in order from start to finish.
 */
void
cong_command_redo (CongCommand *command)
{
	GList *iter;
	CongDocument* doc;

	g_return_if_fail (IS_CONG_COMMAND(command));

	/* g_message ("cong_command_redo(\"%s\")", cong_command_get_description(command)); */

	doc = cong_command_get_document (command);

	cong_document_begin_edit (doc);

	/* Start at front of modification list: */
	for (iter = PRIVATE(command)->list_of_modification; iter; iter=iter->next) {
		CongModification *modification = CONG_MODIFICATION (iter->data);

#if DEBUG_MODIFICATIONS
		g_message ("redoing modification %s", G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(modification)));
#endif

		CONG_EEL_CALL_METHOD (CONG_MODIFICATION_CLASS, 
				      modification,
				      redo,
				      (modification));
	}

	cong_document_end_edit (doc);
}

/**
 * cong_command_merge: *
 * @dst: The #CongCommand into which the modifications are to be added
 * @src: The #CongCommand from which the modifications are to be taken
 *
 * Takes all of the modifications from @src and places them on the end of @dst.  Only to be used by the internals of the undo/redo management
 *
 */
void
cong_command_merge (CongCommand *dst,
		    CongCommand *src)
{
	g_return_if_fail (IS_CONG_COMMAND(dst));
	g_return_if_fail (IS_CONG_COMMAND(src));

	PRIVATE(dst)->list_of_modification = g_list_concat (PRIVATE(dst)->list_of_modification,
							    PRIVATE(src)->list_of_modification);

	PRIVATE(src)->list_of_modification = NULL;
}

/**
 * cong_command_has_ever_been_undone: *
 * @cmd:
 *
 * A function used by the command consolidation/merging system.  If you undo then redo a command,
 * further similar operations should get separate entries in the undo/redo histroy, rather than being
 * merged.
 *
 * Returns: A #gboolean, answering the question "has this command ever been undone?"
 */
gboolean
cong_command_has_ever_been_undone (CongCommand *cmd)
{
	g_return_val_if_fail (IS_CONG_COMMAND(cmd), FALSE);

	return PRIVATE(cmd)->has_ever_been_undone;
}


/* Adding Atomic modifications: */
/**
 * cong_command_add_modification:
 * @cmd:
 * @modification:
 *
 * TODO: Write me
 */
void
cong_command_add_modification (CongCommand *cmd,
			       CongModification *modification)
{
	g_return_if_fail (IS_CONG_COMMAND(cmd));
	g_return_if_fail (IS_CONG_MODIFICATION(modification));

	PRIVATE(cmd)->list_of_modification = g_list_append (PRIVATE(cmd)->list_of_modification,
							    modification);

	g_object_ref (G_OBJECT(modification));

	/* Carry out the modification (by calling its redo method): */
	CONG_EEL_CALL_METHOD (CONG_MODIFICATION_CLASS, 
			      modification,
			      redo,
			      (modification));
}

/**
 * cong_command_add_node_make_orphan:
 * @cmd:
 * @node:
 *
 * TODO: Write me
 */
void
cong_command_add_node_make_orphan (CongCommand *cmd,
				   CongNodePtr node)
{
	CongModification *modification;
	
	g_return_if_fail (IS_CONG_COMMAND(cmd));

	modification = cong_node_modification_make_orphan_new (cong_command_get_document(cmd),
							       node);

	cong_command_add_modification (cmd,
				       modification);
	g_object_unref (G_OBJECT(modification));

}

/**
 * cong_command_add_node_add_after:
 * @cmd:
 * @node:
 * @older_sibling:
 *
 * TODO: Write me
 */
void
cong_command_add_node_add_after (CongCommand *cmd, 
				 CongNodePtr node, 
				 CongNodePtr older_sibling)
{
	CongModification *modification;

	g_return_if_fail (IS_CONG_COMMAND(cmd));
	
	modification = cong_node_modification_add_after_new (cong_command_get_document(cmd),
							     node,
							     older_sibling);
	cong_command_add_modification (cmd,
				       modification);
	g_object_unref (G_OBJECT(modification));
}

/**
 * cong_command_add_node_add_before:
 * @cmd:
 * @node:
 * @younger_sibling:
 *
 * TODO: Write me
 */
void 
cong_command_add_node_add_before (CongCommand *cmd, 
				  CongNodePtr node, 
				  CongNodePtr younger_sibling)
{
	CongModification *modification;

	g_return_if_fail (IS_CONG_COMMAND(cmd));

	modification = cong_node_modification_add_before_new (cong_command_get_document(cmd),
							      node,
							      younger_sibling);
	cong_command_add_modification (cmd,
				       modification);
	g_object_unref (G_OBJECT(modification));
}

/**
 * cong_command_add_node_set_parent:
 * @cmd:
 * @node:
 * @adoptive_parent:
 *
 * TODO: Write me
 */
void 
cong_command_add_node_set_parent (CongCommand *cmd, 
				  CongNodePtr node,
				  CongNodePtr adoptive_parent)
{
	CongModification *modification;
	
	g_return_if_fail (IS_CONG_COMMAND(cmd));

	modification = cong_node_modification_set_parent_new (cong_command_get_document(cmd),
							      node,
							      adoptive_parent);
	cong_command_add_modification (cmd,
				       modification);
	g_object_unref (G_OBJECT(modification));
}

/**
 * cong_command_add_node_set_text:
 * @cmd:
 * @node:
 * @new_content:
 *
 * TODO: Write me
 */
void 
cong_command_add_node_set_text (CongCommand *cmd, 
				CongNodePtr node, 
				const gchar *new_content)
{
	CongModification *modification;
	
	g_return_if_fail (IS_CONG_COMMAND(cmd));

	modification = cong_node_modification_set_text_new (cong_command_get_document(cmd),
							    node,
							    new_content);
	cong_command_add_modification (cmd,
				       modification);
	g_object_unref (G_OBJECT(modification));
}

/**
 * cong_command_add_node_set_attribute:
 * @cmd:
 * @node:
 * @ns_ptr:
 * @name:
 * @value:
 *
 * TODO: Write me
 */
void 
cong_command_add_node_set_attribute (CongCommand *cmd, 
				     CongNodePtr node, 
				     xmlNs *ns_ptr,
				     const gchar *name, 
				     const gchar *value)
{
	CongModification *modification;
	
	g_return_if_fail (IS_CONG_COMMAND(cmd));

	modification = cong_node_modification_set_attribute_new (cong_command_get_document(cmd),
								 node,
								 ns_ptr,
								 name,
								 value);
	cong_command_add_modification (cmd,
				       modification);
	g_object_unref (G_OBJECT(modification));
}

/**
 * cong_command_add_node_remove_attribute:
 * @cmd:
 * @node:
 * @ns_ptr:
 * @name:
 *
 * TODO: Write me
 */
void 
cong_command_add_node_remove_attribute (CongCommand *cmd, 
					CongNodePtr node, 
					xmlNs *ns_ptr,
					const gchar *name)
{
	CongModification *modification;
	
	g_return_if_fail (IS_CONG_COMMAND(cmd));

	modification = cong_node_modification_remove_attribute_new (cong_command_get_document(cmd),
								    node,
								    ns_ptr,
								    name);

	cong_command_add_modification (cmd,
				       modification);
	g_object_unref (G_OBJECT(modification));
}
	
/**
 * cong_command_add_selection_change:
 * @cmd:
 * @new_logical_start:
 * @new_logical_end:
 *
 * TODO: Write me
 */
void 
cong_command_add_selection_change (CongCommand *cmd,
				   const CongLocation *new_logical_start,
				   const CongLocation *new_logical_end)
{
	CongModification *modification;
	
	g_return_if_fail (IS_CONG_COMMAND(cmd));
	g_return_if_fail (cong_location_is_valid (new_logical_start));
	g_return_if_fail (cong_location_is_valid (new_logical_end));

	modification = cong_modification_selection_change_new (cong_command_get_document(cmd),
							       new_logical_start,
							       new_logical_end);
	cong_command_add_modification (cmd,
				       modification);
	g_object_unref (G_OBJECT(modification));
}

/**
 * cong_command_add_cursor_change:
 * @cmd:
 * @new_location:
 *
 * TODO: Write me
 */
void 
cong_command_add_cursor_change (CongCommand *cmd,
				const CongLocation *new_location)
{
	CongModification *modification;
	
	g_return_if_fail (IS_CONG_COMMAND(cmd));
	g_return_if_fail (cong_location_is_valid (new_location));

	modification = cong_modification_cursor_change_new (cong_command_get_document(cmd),
							    new_location);
	cong_command_add_modification (cmd,
				       modification);
	g_object_unref (G_OBJECT(modification));
}

/**
 * cong_command_add_set_dtd_ptr:
 * @cmd:
 * @dtd_ptr:
 *
 * TODO: Write me
 */
void 
cong_command_add_set_dtd_ptr (CongCommand *cmd,
			      xmlDtdPtr dtd_ptr)
{
	CongModification *modification;
	
	g_return_if_fail (IS_CONG_COMMAND(cmd));

	modification = cong_modification_set_dtd_ptr_new (cong_command_get_document(cmd),
							  dtd_ptr);
	cong_command_add_modification (cmd,
				       modification);
	g_object_unref (G_OBJECT(modification));
}

#if 0
void 
cong_command_add_set_clipboard (CongCommand *cmd,
				const gchar* clipboard_source)
{
	CongModification *modification;
	
	g_return_if_fail (IS_CONG_COMMAND(cmd));

#if 0
	modification = cong_node_modification_set_text_new (cong_command_get_document(cmd),
							    node,
							    new_content);
	cong_command_add_modification (cmd,
				       modification);
	g_object_unref (G_OBJECT(modification));
#endif

#error
	g_assert_not_reached();
}
#endif

/**
 * cong_command_add_node_free:
 * @cmd:
 * @node:
 *
 * This function is not currently implemented
 */
void
cong_command_add_node_free (CongCommand *cmd,
			    CongNodePtr node)
{
	/* FIXME: unwritten */
}

/* Adding Compound modifications: */
/**
 * cong_command_add_node_recursive_delete:
 * @cmd:
 * @node:
 *
 * TODO: Write me
 */
void
cong_command_add_node_recursive_delete (CongCommand *cmd,
					CongNodePtr node)
{
	CongDocument *doc;
	CongNodePtr iter, next;

	g_return_if_fail (IS_CONG_COMMAND (cmd));
	g_return_if_fail (node);

	doc = cong_command_get_document (cmd);

	CONG_NODE_SELF_TEST(node);

	/* You must ensure the cursor and/or selection locations don't retain a pointer to this node: */
	if (doc) {
		g_assert (cong_document_get_cursor (doc)->location.node!=node);
		g_assert (cong_selection_get_logical_start (cong_document_get_selection (doc))->node!=node);
		g_assert (cong_selection_get_logical_end (cong_document_get_selection (doc))->node!=node);
	}

	cong_document_begin_edit (doc);

	iter = node->children; 

	while (iter) {
		next = iter->next;

		CONG_NODE_SELF_TEST(iter);
		
		cong_command_add_node_recursive_delete (cmd, iter);

		iter = next;
	}

	g_assert(node->children==NULL);
	g_assert(node->last==NULL);

	if (node->parent) {
		cong_command_add_node_make_orphan (cmd, node);
	}

	/* FIXME: should we really have this one: What about refcounts etc? */
	cong_command_add_node_free (cmd, node);	

	cong_document_end_edit (doc);
}

/**
 * cong_cursor_get_location:
 * @cursor:
 *
 * TODO: Write me
 * Returns:
 */
const CongLocation*
cong_cursor_get_location (const CongCursor *cursor)
{
	g_return_val_if_fail (cursor, NULL);

	return &cursor->location;
}

/**
 * cong_command_for_each_location:
 * @cmd:
 * @callback:
 * @user_data:
 *
 * TODO: Write me
 */
void
cong_command_for_each_location (CongCommand *cmd, 
				CongUpdateLocationCallback callback, 
				gpointer user_data)
{
	CongDocument *doc;
	gboolean selection_change = FALSE;
	CongLocation old_cursor_location;
	CongLocation new_cursor_location;
	CongLocation old_logical_sel_start;
	CongLocation old_logical_sel_end;
	CongLocation new_logical_sel_start;
	CongLocation new_logical_sel_end;

	g_return_if_fail (IS_CONG_COMMAND (cmd));
	g_return_if_fail (callback);

	doc = cong_command_get_document (cmd);

	cong_location_copy (&old_cursor_location, cong_cursor_get_location (cong_document_get_cursor (doc)));
	cong_location_copy (&old_logical_sel_start, cong_selection_get_logical_start (cong_document_get_selection (doc)));
	cong_location_copy (&old_logical_sel_end, cong_selection_get_logical_end (cong_document_get_selection (doc)));

	cong_location_copy (&new_cursor_location, &old_cursor_location);
	cong_location_copy (&new_logical_sel_start, &old_logical_sel_start);
	cong_location_copy (&new_logical_sel_end, &old_logical_sel_end);

#if 0
	g_message ("test for update of cursor location from (%p,%i)",cong_document_get_cursor (doc)->location.node, cong_document_get_cursor (doc)->location.byte_offset);
#endif

	if ( callback (doc,
		       &new_cursor_location,
		       user_data)) {
#if 0
		g_message ("update of cursor location to (%p,%i)",new_cursor_location.node, new_cursor_location.byte_offset);
#endif

		cong_command_add_cursor_change (cmd,
						&new_cursor_location);
	}

	if ( callback (doc,
		       &new_logical_sel_start,
		       user_data)) {
		selection_change = TRUE;
	}
	
	if (callback (doc,
		      &new_logical_sel_end,
		      user_data)) {
		selection_change = TRUE;
	}
	
	if (selection_change) {
		cong_command_add_selection_change (cmd,
						   &new_logical_sel_start,
						   &new_logical_sel_end);
	}
}

static gboolean
pre_node_deletion_update_location_callback (CongDocument *doc,
					    CongLocation *location, 
					    gpointer user_data)
{
	CongNodePtr node = user_data;
	if (location->node == node) {

		/* FIXME: do this for now: */
		cong_location_nullify (location);
		return TRUE;
	}

	return FALSE;
}

struct text_deletion_userdata
{
	CongNodePtr node;
	int start_byte_offset;
	int end_byte_offset;
};

static gboolean
text_deletion_update_location_callback (CongDocument *doc,
					CongLocation *location, 
					gpointer user_data)
{
	struct text_deletion_userdata *text_deletion_userdata = user_data;

	if (location->node == text_deletion_userdata->node) {
		if (location->byte_offset <= text_deletion_userdata->start_byte_offset) {
			return FALSE;
		} else {
			if (location->byte_offset < text_deletion_userdata->end_byte_offset) {
				cong_location_nullify (location);
				return TRUE;
			} else {
				location->byte_offset -= (text_deletion_userdata->end_byte_offset - text_deletion_userdata->start_byte_offset);
				
				return TRUE;				
			}
		}
	} 

	return FALSE;
}

static gboolean
recursive_node_deletion_update_location_callback (CongDocument *doc,
						  CongLocation *location, 
						  gpointer user_data)
{
	CongNodePtr node = user_data;

	if ((location->node==node) || cong_node_is_descendant_of (location->node, node)) {

		cong_location_nullify (location);
		return TRUE;
	}

	return FALSE;
}

/**
 * cong_command_add_delete_range:
 * @cmd:
 * @range: a range within the document; both start and end must have the same parent, so that proper nesting is maintained
 *
 * Utility function to add a series of modifications to the given command.
 *
 * Deletes the given range within the document (can include multiple nodes).  Updates cursor and selection accordingly.
 */
void 
cong_command_add_delete_range (CongCommand *cmd,
			       CongRange *range)
{
	CongDocument *doc;
	CongLocation loc0, loc1;
	CongNodePtr n0, n2;
	
	g_return_if_fail (IS_CONG_COMMAND (cmd));
	g_return_if_fail(range);

	doc = cong_command_get_document (cmd);

	/* Validate range */
	g_return_if_fail( cong_location_exists(&range->loc0) );
	g_return_if_fail( cong_location_exists(&range->loc1) );
	g_return_if_fail( cong_location_parent(&range->loc0) == cong_location_parent(&range->loc1) );
	/* both must be children of the same parent to maintain proper nesting */

	cong_document_begin_edit (doc);

	/* --- Processing for multiple nodes --- */
	if (range->loc0.node != range->loc1.node)
	{
		CongNodePtr prev_node;
	
		/* Range is valid, now order first/last nodes */
		
		for (n0 = range->loc0.node; n0 && n0 != range->loc1.node; n0 = n0->next) ;
		
		if (!n0)
		{
			cong_location_copy(&loc0, &range->loc1);
			cong_location_copy(&loc1, &range->loc0);
		}
		else
		{
			cong_location_copy(&loc0, &range->loc0);
			cong_location_copy(&loc1, &range->loc1);
		}

		/* Split, first */

		if (loc0.byte_offset && cong_node_type_is_textual (cong_node_type(loc0.node)))
		{
			prev_node = cong_command_add_xml_frag_data_nice_split2 (cmd, &loc0);
			g_assert(prev_node);

			loc0.node = range->loc0.node = prev_node->next;
		} else {
			prev_node = loc0.node;
		}
		
		/* prev_node holds the previous node */

		/* Reparent, first & middle */
		for (n0 = loc0.node; n0 != loc1.node; n0 = n2) {
			n2 = n0->next;

			CONG_NODE_SELF_TEST(n0);

			cong_command_for_each_location (cmd, 
							pre_node_deletion_update_location_callback, 
							n0);

			cong_command_add_node_recursive_delete (cmd,
								n0);
		}

		/* Split, last */

		if (loc1.byte_offset && cong_node_type_is_textual (cong_node_type(loc1.node)))
		{
			loc1.node = cong_command_add_xml_frag_data_nice_split2(cmd, &loc1);
		}

		/* Delete last */
		cong_command_for_each_location (cmd, 
						pre_node_deletion_update_location_callback, 
						loc1.node);

		cong_command_add_node_recursive_delete (cmd,
							loc1.node);
	}

	/* --- Processing for single node (loc0.node == loc1.node) --- */

	else
	{
		/* Sort out the ordering: */
		if (range->loc0.byte_offset < range->loc1.byte_offset)
		{
			cong_location_copy(&loc0,&range->loc0);
			cong_location_copy(&loc1,&range->loc1);
		}
		else
		{
			cong_location_copy(&loc0,&range->loc1);
			cong_location_copy(&loc1,&range->loc0);
		}

		if (cong_node_supports_byte_offsets (loc0.node))
		{
			if (loc0.byte_offset == loc1.byte_offset) {
				/* The end is the beginning is the end */
			} else {
			
				/* Split up textual content of node: */
				gchar *text_before = g_strndup ((gchar*)loc0.node->content, loc0.byte_offset);

				gchar *new_text = g_strdup_printf("%s%s",text_before, loc1.node->content + loc1.byte_offset);

				struct text_deletion_userdata text_deletion_userdata;				
				text_deletion_userdata.node = loc0.node;
				text_deletion_userdata.start_byte_offset = loc0.byte_offset;
				text_deletion_userdata.end_byte_offset = loc1.byte_offset;

				cong_command_for_each_location (cmd,
								text_deletion_update_location_callback, 
								&text_deletion_userdata);

				cong_command_add_node_set_text (cmd,
								loc0.node,
								new_text);

				g_free (text_before);
				g_free (new_text);

				/* what should happen to cursor? */
			}
		} else {
			/* Delete entire node: */
			cong_command_for_each_location (cmd,
							recursive_node_deletion_update_location_callback,
							loc0.node);

			cong_command_add_node_recursive_delete (cmd,
								loc0.node);
		}
	}

	cong_document_end_edit (doc);
}

/**
 * cong_command_add_delete_selection:
 * @cmd:
 *
 * Utility function to add a series of modifications to the given command.
 *
 * Deletes the current selection within the document, updating cursor and selection accordingly.
 */
void 
cong_command_add_delete_selection (CongCommand *cmd)
{
	CongDocument *doc;
	CongSelection *selection;
	CongRange *range;

	g_return_if_fail (IS_CONG_COMMAND (cmd));

	doc = cong_command_get_document (cmd);
	selection = cong_document_get_selection(doc);
	range = cong_selection_get_ordered_range (selection);


	cong_document_begin_edit (doc);

	cong_command_add_delete_range (cmd, 
				       cong_selection_get_ordered_range (selection));


	cong_command_add_nullify_selection (cmd);

	cong_document_end_edit (doc);
}

/**
 * cong_command_add_insert_text_at_cursor:
 * @cmd:
 * @string: a UTF-8 string
 *
 * Utility function to add a series of modifications to the given command.
 *
 * Inserts the given text at the cursor, moving the cursor to the end of the inserted text.
 */
void 
cong_command_add_insert_text_at_cursor (CongCommand *cmd, 
					const gchar *string)
{
	CongDocument *doc;
	CongCursor *curs;
	CongLocation old_cursor_loc;
	CongLocation new_cursor_loc;
	gchar *initial_part;
	gchar *new_content;
	int byte_length;

	g_return_if_fail (IS_CONG_COMMAND (cmd));
	g_return_if_fail (string);
	g_return_if_fail (g_utf8_validate(string, -1, NULL));

	doc = cong_command_get_document (cmd);
	curs = cong_document_get_cursor (doc);

	if (!cong_location_exists(&curs->location)) return;

	if (!cong_node_is_valid_cursor_location (curs->location.node)) {
		return;
	}

	cong_location_copy (&old_cursor_loc, &curs->location);
	cong_location_copy (&new_cursor_loc, &curs->location);

	cong_document_begin_edit (doc);

	byte_length = strlen(string);

	initial_part = g_strndup ((const gchar*)old_cursor_loc.node->content, old_cursor_loc.byte_offset);
	CONG_VALIDATE_UTF8(initial_part);

	CONG_VALIDATE_UTF8((const gchar*)old_cursor_loc.node->content+old_cursor_loc.byte_offset);

	new_content = g_strconcat (initial_part,
				   string, 
				   (const gchar*)old_cursor_loc.node->content+old_cursor_loc.byte_offset, 
				   NULL);
	CONG_VALIDATE_UTF8(new_content);
	g_free (initial_part);

	cong_command_add_node_set_text (cmd, 
					old_cursor_loc.node,
					new_content);
	g_free (new_content);

	new_cursor_loc.byte_offset += byte_length;
	CONG_VALIDATE_UTF8((const gchar*)new_cursor_loc.node->content+new_cursor_loc.byte_offset);

	cong_command_add_cursor_change (cmd,
					&new_cursor_loc);

	cong_document_end_edit (doc);
}

/**
 * cong_command_add_nullify_cursor:
 * @cmd:
 *
 * TODO: Write me
 */
void 
cong_command_add_nullify_cursor (CongCommand *cmd)
{
	CongDocument *doc;
	CongLocation new_logical_loc;

	g_return_if_fail (IS_CONG_COMMAND (cmd));

	doc = cong_command_get_document (cmd);

	cong_location_nullify (&new_logical_loc);

	cong_command_add_cursor_change (cmd,
					&new_logical_loc);
}

/**
 * cong_command_add_nullify_selection:
 * @cmd:
 *
 * TODO: Write me
 */
void 
cong_command_add_nullify_selection (CongCommand *cmd)
{
	CongDocument *doc;
	CongSelection *selection;
	CongLocation new_logical_loc;

	g_return_if_fail (IS_CONG_COMMAND (cmd));

	doc = cong_command_get_document (cmd);
	selection = cong_document_get_selection(doc);

	cong_location_nullify (&new_logical_loc);

	cong_command_add_selection_change (cmd,
					   &new_logical_loc,
					   &new_logical_loc);	
}

/**
 * cong_command_add_xml_frag_data_nice_split2:
 * @cmd:
 * @loc:
 *
 * TODO: Write me
 * Returns:
 */
CongNodePtr
cong_command_add_xml_frag_data_nice_split2  (CongCommand *cmd, 
					     const CongLocation *loc)
{
	CongDocument *doc;
	CongNodePtr d = NULL;
	int len1, len2;
	CongNodeType node_type;

	g_return_val_if_fail (IS_CONG_COMMAND (cmd), NULL);
	g_return_val_if_fail (loc, NULL);
	g_return_val_if_fail (cong_location_exists(loc), NULL);
	g_return_val_if_fail (cong_node_type_is_textual (cong_location_node_type(loc)), NULL);

	node_type = cong_location_node_type(loc);
	
	CONG_NODE_SELF_TEST(loc->node);

	doc = cong_command_get_document (cmd);

	/* Calculate segments */

	len1 = loc->byte_offset;
	len2 = cong_node_get_length(loc->node) - len1;

	g_assert (len1>=0);
	g_assert (len2>=0);

	if (len1==0 && len2==0) {
		d = cong_node_new_textual (node_type, "", doc);
	} else if (len1==0) {
		d = cong_node_new_textual (node_type, "", doc);

		/* Link it in */
		cong_command_add_node_add_before (cmd, 
						  d, 
						  loc->node);
		return(d);

	} else if (len2==0) {
		d = cong_node_new_textual (node_type, "", doc);
	} else {
		gchar* new_text = g_strndup((const gchar*)loc->node->content, len1);

		/* Make split representation */
		d = cong_node_new_textual_len (node_type, cong_node_safe_get_content(loc->node) + len1, len2, doc); /* FIXME: check char ptr arithmetic; UTF8? */

		/* Shrink original node */
		cong_command_add_node_set_text(cmd, 
					       loc->node, 
					       new_text);

		g_free(new_text);
	}

	g_assert(d);

	/* Link it in */
	cong_command_add_node_add_after (cmd, 
					 d, 
					 loc->node);

	CONG_NODE_SELF_TEST(loc->node);

	return(loc->node);
}

static gboolean
merge_text_update_location_callback (CongDocument *doc,
				     CongLocation *location, 
				     gpointer user_data)
{
	CongNodePtr affected_node = user_data;

	g_assert (affected_node);
	g_assert (affected_node->prev);

	g_assert (cong_node_type (affected_node)==CONG_NODE_TYPE_TEXT);
	g_assert (cong_node_type (affected_node->prev)==CONG_NODE_TYPE_TEXT);

	if (location->node == affected_node->prev) {
		
		location->node = affected_node;

		return TRUE;

	} else if (location->node == affected_node) {

		location->byte_offset += strlen((const char*)affected_node->prev->content);

		return TRUE;
	}

	return FALSE;
}

static gboolean 
merge_adjacent_text_callback (CongDocument *doc, 
			      CongNodePtr node, 
			      gpointer user_data, 
			      guint recursion_level)
{
	CongCommand *cmd = CONG_COMMAND (user_data);

	/* We have to "look behind" at the previous sibling, since the iteration moes forward: */
	if (node->prev) {
		if (cong_node_type(node)==CONG_NODE_TYPE_TEXT) {
			if (cong_node_type(node->prev)==CONG_NODE_TYPE_TEXT) {
				/* Merge preceding node's text into this one, then delete it: */
				gchar *new_text;

				new_text = g_strdup_printf("%s%s", node->prev->content, node->content);

				cong_command_add_node_set_text (cmd, node, new_text);
				g_free (new_text);

				/* Update cursor and selection if necessary: */
				cong_command_for_each_location (cmd,
								merge_text_update_location_callback, 
								node);

				cong_command_add_node_recursive_delete (cmd, node->prev);
			}			
		}
	}

	/* Keep going: */
	return FALSE;
}

/**
 * cong_command_add_merge_adjacent_text_nodes:
 * @cmd:
 *
 * Utility function to add a series of modifications to the given command.
 * 
 * Searches the entire document, looking for text nodes adjacent to other text nodes, merging them together.
 */
void
cong_command_add_merge_adjacent_text_nodes (CongCommand *cmd)
{
	CongDocument *doc;

	g_return_if_fail (IS_CONG_COMMAND(cmd));

	doc = cong_command_get_document (cmd);

	cong_document_begin_edit (doc);

	cong_document_for_each_node (doc, merge_adjacent_text_callback, cmd);

	cong_document_end_edit (doc);
}

/**
 * cong_command_add_merge_adjacent_text_children_of_node:
 * @cmd:
 * @node:
 *
 * Utility function to add a series of modifications to the given command.
 * 
 * Searches direct children of the given node, looking for text nodes adjacent to other text nodes, merging them together.
 */
void
cong_command_add_merge_adjacent_text_children_of_node (CongCommand *cmd, 
						       CongNodePtr node)
{
	CongDocument *doc;

	g_return_if_fail (IS_CONG_COMMAND(cmd));

	doc = cong_command_get_document (cmd);

	cong_document_begin_edit (doc);

	cong_document_for_each_child_of_node (doc, node, merge_adjacent_text_callback, cmd);

	cong_document_end_edit (doc);
}

/**
 * cong_command_can_add_reparent_selection:
 * @cmd:
 * @new_parent:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_command_can_add_reparent_selection (CongCommand *cmd,
					 CongNodePtr new_parent)
{
	CongSelection *selection = cong_document_get_selection (cong_command_get_document (cmd));
	const CongLocation *logical_start = cong_selection_get_logical_start (selection);
	const CongLocation *logical_end = cong_selection_get_logical_start (selection);

	g_return_val_if_fail (IS_CONG_COMMAND(cmd), FALSE);

	/* Validate selection */
	g_return_val_if_fail (cong_location_exists (logical_start), FALSE );
	g_return_val_if_fail (cong_location_exists (logical_end), FALSE );
	g_return_val_if_fail (cong_location_parent (logical_start) == cong_location_parent (logical_end), FALSE);

	g_return_val_if_fail (new_parent, FALSE);

	return TRUE;
}

/**
 * cong_command_add_reparent_selection:
 * @cmd:
 * @node:
 *
 * Utility function to add a series of modifications to the given command.
 * 
 * Splits the selected nodes as necessary and adds as a child of the input node
 *
 * Returns:
 */
CongNodePtr
cong_command_add_reparent_selection (CongCommand *cmd, 
				     CongNodePtr node)
{
	CongDocument *doc;
	CongSelection *selection;
	const CongLocation *ordered_start;
	const CongLocation *ordered_end;
	CongLocation loc0;
	CongLocation loc1;
	CongLocation new_selection_start;
	CongLocation new_selection_end;

	g_return_val_if_fail (IS_CONG_COMMAND(cmd), NULL);
	g_return_val_if_fail (node, NULL);

	doc = cong_command_get_document (cmd);
	selection = cong_document_get_selection (doc);

	ordered_start = cong_selection_get_ordered_start (selection);
	ordered_end = cong_selection_get_ordered_end (selection);

	/* Validate selection */
	g_return_val_if_fail (cong_location_exists (ordered_start), NULL );
	g_return_val_if_fail (cong_location_exists (ordered_end), NULL );
	g_return_val_if_fail (cong_location_parent (ordered_start) == cong_location_parent(ordered_end), NULL);

	cong_location_copy (&loc0, ordered_start);
	cong_location_copy (&loc1, ordered_end);

	cong_location_copy (&new_selection_start, ordered_start);
	cong_location_copy (&new_selection_end, ordered_end);

	CONG_NODE_SELF_TEST(node);

	/* --- Processing for multiple nodes --- */
	if (loc0.node != loc1.node)
	{
		CongNodePtr prev_node;
		CongNodePtr iter, iter_next;

		cong_document_begin_edit(doc);
	
		/* Split, first */

		if (loc0.byte_offset && cong_node_type_is_textual (cong_node_type(loc0.node)))
		{
			prev_node = cong_command_add_xml_frag_data_nice_split2(cmd, &loc0);
			g_assert(prev_node);

			new_selection_start.node = loc0.node = prev_node->next;
		} else {
			prev_node = loc0.node;
		}

		new_selection_start.byte_offset = 0;

		/* prev_node holds the previous node */

		/* Position new_parent within the tree: */
		if (prev_node) {
			cong_command_add_node_add_after(cmd, node, prev_node);
			CONG_NODE_SELF_TEST(prev_node);
		} else {
			cong_command_add_node_set_parent(cmd, node, loc0.node->parent);
		}

		/* Reparent, first & middle */
		for (iter = loc0.node; iter != loc1.node; iter = iter_next) {
			iter_next = iter->next;

			CONG_NODE_SELF_TEST(iter);
			CONG_NODE_SELF_TEST(node);

			cong_command_add_node_set_parent(cmd, iter, node);			

			CONG_NODE_SELF_TEST(iter);
			CONG_NODE_SELF_TEST(node);
		}

		/* Split, last */

		if (loc1.byte_offset && cong_node_type_is_textual (cong_node_type(loc1.node)))
		{
			loc1.node = cong_command_add_xml_frag_data_nice_split2(cmd, &loc1);
			new_selection_end.node = loc1.node->next;
		}

		new_selection_end.byte_offset = 0;

		/* Reparent, last */
		cong_command_add_node_set_parent(cmd, loc1.node, node);

		cong_command_add_selection_change (cmd,
						   &new_selection_start,
						   &new_selection_end);

		cong_document_end_edit(doc);
		
		return(prev_node);
	}

	/* --- Processing for single node (loc0.node == loc1.node) --- */

	else
	{
		cong_document_begin_edit(doc);

		if (cong_node_type_is_textual (cong_node_type(loc0.node)))
		{
			if (loc0.byte_offset == loc1.byte_offset) {
				cong_document_end_edit(doc);

				return NULL; /* The end is the beginning is the end */
			}
			
			loc0.node = loc1.node = cong_command_add_node_split3(cmd, loc0.node, loc0.byte_offset, loc1.byte_offset);
		}

		new_selection_start.byte_offset = 0;
		new_selection_end.byte_offset = 0;
		
		/* Position new_parent where the selection was: */
		if (loc0.node->prev) {
			cong_command_add_node_add_after(cmd, node, loc0.node->prev);
		} else {
			cong_command_add_node_set_parent(cmd, node, loc0.node->parent);
		}
		/* Move the selection below new_parent: */
		cong_command_add_node_set_parent(cmd, cong_selection_get_ordered_start (selection)->node, node);

		cong_command_add_selection_change (cmd,
						   &new_selection_start,
						   &new_selection_end);

		cong_document_end_edit(doc);

		/* Return node before new_parent's new position (I think): */
		return node->prev;
	}
}

struct split3_userdata
{
	CongNodePtr node;
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
	
	if (location->node == split3_data->node) {
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

/**
 * cong_command_add_node_split3:
 * @cmd:
 * @node:
 * @c0:
 * @c1:
 *
 * Utility function to add a series of modifications to the given command.
 *
 * Splits a text or comment node into 3 nodes, and returns a pointer to the middle one
 *
 * Returns: the middle node of the three newly-created nodes
 */
CongNodePtr
cong_command_add_node_split3 (CongCommand *cmd, 
			      CongNodePtr node, 
			      int c0, 
			      int c1)
{
	CongDocument *doc;
	CongNodePtr d1, d2, d3;
	int len1, len2, len3;
	CongNodeType node_type;

	g_return_val_if_fail (IS_CONG_COMMAND (cmd), NULL);
	g_return_val_if_fail (cong_node_type_is_textual (cong_node_type(node)), NULL);

	node_type = cong_node_type (node);

	CONG_NODE_SELF_TEST(node);

	doc = cong_command_get_document (cmd);
	
	/* Calculate segments */
	if (cong_node_get_length(node) < c1) c1 = cong_node_get_length(node);
	if (c1 < c0) c1 = c0;
	
	len1 = c0;
	len2 = c1 - c0;
	len3 = cong_node_get_length(node) - c1;

	/* Make split representation */
	d1 = cong_node_new_textual_len (node_type, cong_node_safe_get_content(node), len1, doc); /* FIXME:  audit the char types here, and the char pointer arithmetic. UTF8? */
	d2 = cong_node_new_textual_len (node_type, cong_node_safe_get_content(node) + len1, len2, doc);
	d3 = cong_node_new_textual_len (node_type, cong_node_safe_get_content(node) + len1 + len2, len3, doc);

	cong_document_begin_edit(doc);

	/* Link it in */
	cong_command_add_node_add_after(cmd, d1, node);
	cong_command_add_node_add_after(cmd, d2, d1);
	cong_command_add_node_add_after(cmd, d3, d2);
	cong_command_add_node_make_orphan(cmd, node);

	/* Update the cursor and selection as necessary: */
	{
		struct split3_userdata user_data;
		
		user_data.node = node;
		user_data.c0 = c0;
		user_data.c1 = c1;
		user_data.d1 = d1;
		user_data.d2 = d2;
		user_data.d3 = d3;
		
		cong_command_for_each_location (cmd, 
						split3_location_callback,
						&user_data);
	}

	/* Unlink old node */
	cong_command_add_node_recursive_delete (cmd, node);

	cong_document_end_edit(doc);

	CONG_NODE_SELF_TEST(d2);

	return(d2);
}

/**
 * cong_command_add_remove_tag:
 * @cmd:
 * @node: a node
 *
 * Utility function to add a series of modifications to the given command.
 *
 * Removes the given node from the tree, moving all of its children into the space it occupied.
 */
void 
cong_command_add_remove_tag (CongCommand *cmd,
			     CongNodePtr node)
{
	CongDocument *doc;
	CongNodePtr iter;
	CongNodePtr iter_next;


	g_return_if_fail (IS_CONG_COMMAND (cmd));
	g_return_if_fail(node);

	doc = cong_command_get_document (cmd);

	cong_document_begin_edit (doc);

	for (iter = node->children; iter; iter = iter_next) {
		iter_next = iter->next;
		
		cong_command_add_node_add_before (cmd, iter, node);
	}

	cong_command_add_node_make_orphan (cmd, node);

	cong_command_add_node_recursive_delete (cmd, node);

	cong_document_end_edit (doc);
}

/**
 * cong_command_add_set_cursor_to_first_text_descendant:
 * @cmd:
 * @node:
 *
 * TODO: Write me
 */
void
cong_command_add_set_cursor_to_first_text_descendant (CongCommand *cmd,
						      CongNodePtr node)
{
	CongDocument *doc;
	CongNodePtr cursor_node;

	g_return_if_fail (IS_CONG_COMMAND (cmd));
	g_return_if_fail (node);

	doc = cong_command_get_document (cmd);

	cursor_node = cong_node_get_first_text_node_descendant (node);
	
	if (cursor_node) {
		CongLocation new_location;

		cong_document_begin_edit (doc);

		cong_location_set_to_start_of_node (&new_location,
						    cursor_node);

		cong_command_add_cursor_change (cmd,
						&new_location);

		cong_document_end_edit (doc);		
	}
}

/**
 * cong_command_add_set_external_dtd:
 * @cmd: a command
 * @root_element: the root element of the document
 * @public_id:
 * @system_id:
 *
 * Utility function to add a series of modifications to the given command.
 * 
 * Sets an external DTD on the document, or removes it if NULL is given
 */
void
cong_command_add_set_external_dtd (CongCommand *cmd,
				   const gchar* root_element,
				   const gchar* public_id,
				   const gchar* system_id)
{
	CongDocument *doc;
	xmlDocPtr xml_doc;

	g_return_if_fail (IS_CONG_COMMAND (cmd));

	doc = cong_command_get_document (cmd);
	xml_doc = cong_document_get_xml (doc);
	
	/* Remove any existing DTD: */
	{
		if (xml_doc->extSubset) {
			cong_command_add_node_make_orphan (cmd,
							   (xmlNodePtr)xml_doc->extSubset);
			
			cong_command_add_node_recursive_delete (cmd, 
								(xmlNodePtr)xml_doc->extSubset);
			
			cong_command_add_set_dtd_ptr (cmd,
						      NULL);
		}
	}
	
	/* Add the new DTD (if any): */
	if (root_element) {
		xmlDtdPtr dtd_ptr = cong_util_make_dtd (xml_doc,
							root_element,
							public_id, 
							system_id);
		
		if (dtd_ptr) {			
			if (xml_doc->children) {
				cong_command_add_node_add_before (cmd,
								  (xmlNodePtr)dtd_ptr,
								  (xmlNodePtr)xml_doc->children);
			} else {
				cong_command_add_node_set_parent (cmd,
								  (xmlNodePtr)dtd_ptr,
								  (xmlNodePtr)xml_doc);
			}
			
			/* Ensure the DTD ptr is still set up within the xml_doc; the tree manipulation seems to make it lose the extSubset pointer: */
			cong_command_add_set_dtd_ptr (cmd,
						      dtd_ptr);
		}
	}
}
