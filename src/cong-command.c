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
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "cong-app.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-command.h"
#include "cong-plugin.h"
#include "cong-modification.h"

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

/* Headers for the various compound modifications: */
#include "cong-range.h"
#include "cong-selection.h"

#define PRIVATE(x) ((x)->private)

struct CongCommandDetails
{
	CongDocument *doc;
	gchar *description;

	GList *list_of_modification;
};

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongCommand, 
			cong_command,
			GObject,
			G_TYPE_OBJECT );

static void
cong_command_class_init (CongCommandClass *klass)
{
}

static void
cong_command_instance_init (CongCommand *node)
{
	node->private = g_new0(CongCommandDetails,1);
}

CongCommand*
cong_command_construct (CongCommand *command,
			CongDocument *doc,
			const gchar *description)
{
	PRIVATE(command)->doc = doc;
	PRIVATE(command)->description = g_strdup (description);

	return command;
}

CongCommand*
cong_command_new (CongDocument *doc,
		  const gchar *description)
{
	return cong_command_construct (CONG_COMMAND(g_object_new (CONG_COMMAND_TYPE, NULL)),
				       doc,
				       description);
}

CongDocument*
cong_command_get_document (CongCommand *command)
{
	g_return_val_if_fail (IS_CONG_COMMAND(command), NULL);
	
	return PRIVATE(command)->doc;
}

const gchar*
cong_command_get_description (CongCommand *command)
{
	g_return_val_if_fail (IS_CONG_COMMAND(command), NULL);
	
	return PRIVATE(command)->description;
}

void
cong_command_undo (CongCommand *command)
{
	GList *iter;
	CongDocument* doc;

	g_return_if_fail (IS_CONG_COMMAND(command));

	g_message ("cong_command_undo(\"%s\")", cong_command_get_description(command));

	doc = cong_command_get_document (command);

	cong_document_begin_edit (doc);

	/* Start at end of modification list, iterate backwards up to front: */
	for (iter = g_list_last(PRIVATE(command)->list_of_modification); iter; iter=iter->prev) {
		CongModification *modification = CONG_MODIFICATION (iter->data);

		CONG_EEL_CALL_METHOD (CONG_MODIFICATION_CLASS, 
				      modification, 
				      undo, 
				      (modification));
	}

	cong_document_end_edit (doc);
}

void
cong_command_redo (CongCommand *command)
{
	GList *iter;
	CongDocument* doc;

	g_return_if_fail (IS_CONG_COMMAND(command));

	g_message ("cong_command_redo(\"%s\")", cong_command_get_description(command));

	doc = cong_command_get_document (command);

	cong_document_begin_edit (doc);

	/* Start at front of modification list: */
	for (iter = PRIVATE(command)->list_of_modification; iter; iter=iter->next) {
		CongModification *modification = CONG_MODIFICATION (iter->data);

		CONG_EEL_CALL_METHOD (CONG_MODIFICATION_CLASS, 
				      modification,
				      redo,
				      (modification));
	}

	cong_document_end_edit (doc);
}

/* Adding Atomic modifications: */
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

void 
cong_command_add_node_set_text (CongCommand *cmd, 
				CongNodePtr node, 
				const xmlChar *new_content)
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

void 
cong_command_add_node_set_attribute (CongCommand *cmd, 
				     CongNodePtr node, 
				     const xmlChar *name, 
				     const xmlChar *value)
{
	CongModification *modification;
	
	g_return_if_fail (IS_CONG_COMMAND(cmd));

	modification = cong_node_modification_set_attribute_new (cong_command_get_document(cmd),
								 node,
								 name,
								 value);
	cong_command_add_modification (cmd,
				       modification);
	g_object_unref (G_OBJECT(modification));
}

void 
cong_command_add_node_remove_attribute (CongCommand *cmd, 
					CongNodePtr node, 
					const xmlChar *name)
{
	CongModification *modification;
	
	g_return_if_fail (IS_CONG_COMMAND(cmd));

	modification = cong_node_modification_remove_attribute_new (cong_command_get_document(cmd),
								    node,
								    name);

	cong_command_add_modification (cmd,
				       modification);
	g_object_unref (G_OBJECT(modification));
}
	
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

void 
cong_command_add_set_external_dtd (CongCommand *cmd,
				   const gchar* root_element,
				   const gchar* public_id,
				   const gchar* system_id)
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

	g_assert_not_reached();

}

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

	g_assert_not_reached();
}

void
cong_command_add_node_free (CongCommand *cmd,
			    CongNodePtr node)
{
	g_assert_not_reached();
}

/* Adding Compound modifications: */
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

const CongLocation*
cong_cursor_get_location (const CongCursor *cursor)
{
	g_return_val_if_fail (cursor, NULL);

	return &cursor->location;
}

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
	cong_location_copy (&new_logical_sel_end, &old_logical_sel_start);
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

void 
cong_command_add_delete_range (CongCommand *cmd,
			       CongRange *range)
{
	CongDocument *doc;
	CongLocation loc0, loc1;
	CongNodePtr n0, n1, n2;
	
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

		if (loc0.byte_offset && cong_node_type(loc0.node) == CONG_NODE_TYPE_TEXT)
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

		if (loc1.byte_offset && cong_node_type(loc1.node) == CONG_NODE_TYPE_TEXT)
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

		if (cong_node_type(loc0.node) == CONG_NODE_TYPE_TEXT)
		{
			if (loc0.byte_offset == loc1.byte_offset) {
				/* The end is the beginning is the end */
			} else {
			
				/* Split up textual content of node: */
				gchar *text_before = g_strndup (loc0.node->content, loc0.byte_offset);

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
#if 1
			CONG_DO_UNIMPLEMENTED_DIALOG(NULL, "Deletion of a single non-textual node");
#else
			/* what should happen to cursor? */
			cong_util_remove_tag (doc, 
					      loc0.node);
#endif
		}
	}

	cong_document_end_edit (doc);
}

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

void 
cong_command_add_insert_text_at_cursor (CongCommand *cmd, 
					const gchar *string)
{
	CongDocument *doc;
	CongCursor *curs;
	CongLocation old_cursor_loc;
	CongLocation new_cursor_loc;
	xmlChar *new_content;
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

	new_content = xmlStrndup(old_cursor_loc.node->content, old_cursor_loc.byte_offset);
	CONG_VALIDATE_UTF8(new_content);

	new_content = xmlStrcat(new_content, string);
	CONG_VALIDATE_UTF8(new_content);

	CONG_VALIDATE_UTF8(old_cursor_loc.node->content+old_cursor_loc.byte_offset);
	new_content = xmlStrcat(new_content, old_cursor_loc.node->content+old_cursor_loc.byte_offset);
	CONG_VALIDATE_UTF8(new_content);

	cong_command_add_node_set_text (cmd, 
					old_cursor_loc.node,
					new_content);
	xmlFree(new_content);

	new_cursor_loc.byte_offset += byte_length;		
	CONG_VALIDATE_UTF8(new_cursor_loc.node->content+new_cursor_loc.byte_offset);

	cong_command_add_cursor_change (cmd,
					&new_cursor_loc);

	cong_document_end_edit (doc);
}

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

CongNodePtr
cong_command_add_xml_frag_data_nice_split2  (CongCommand *cmd, 
					     const CongLocation *loc)
{
	CongDocument *doc;
	CongNodePtr d = NULL;
	int len1, len2;

	g_return_val_if_fail (IS_CONG_COMMAND (cmd), NULL);
	g_return_val_if_fail (loc, NULL);
	g_return_val_if_fail (cong_location_exists(loc), NULL);
	g_return_val_if_fail ((cong_location_node_type(loc) == CONG_NODE_TYPE_TEXT), NULL);
	
	CONG_NODE_SELF_TEST(loc->node);

	doc = cong_command_get_document (cmd);

	/* Calculate segments */

	len1 = loc->byte_offset;
	len2 = cong_node_get_length(loc->node) - len1;

	g_assert (len1>=0);
	g_assert (len2>=0);

	if (len1>0 && len2>0) {
		d = cong_node_new_text("", doc);
	} else if (len1>0) {
		d = cong_node_new_text("", doc);

		/* Link it in */
		cong_command_add_node_add_before (cmd, 
						  d, 
						  loc->node);
		return(d);

	} else if (len2>0) {
		d = cong_node_new_text("", doc);
	} else {
		xmlChar* new_text = g_strndup(loc->node->content, len1); /* FIXME:  char type conversion? */

		/* Make split representation */
		d = cong_node_new_text_len(xml_frag_data_nice(loc->node) + len1, len2, doc); /* FIXME: check char ptr arithmetic; UTF8? */

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

void
cong_command_add_merge_adjacent_text_nodes (CongCommand *cmd)
{
	g_assert_not_reached();
}

void
cong_command_add_merge_adjacent_text_children_of_node (CongCommand *cmd, 
						       CongNodePtr node)
{
	g_assert_not_reached();
}

gboolean
cong_command_can_add_reparent_selection (CongCommand *cmd)
{
	g_assert_not_reached();
	return TRUE;
}

void
cong_command_add_reparent_selection (CongCommand *cmd, 
				     CongNodePtr node)
{
	g_assert_not_reached();

	/* cong_selection_reparent_all(selection, doc, new_element) */
}
