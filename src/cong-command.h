/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-command.h
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

#ifndef __CONG_COMMAND_H__
#define __CONG_COMMAND_H__

#include "cong-document.h"
#include "cong-modification.h"

G_BEGIN_DECLS

#define DEBUG_COMMAND_LIFETIMES 0

#define CONG_COMMAND_TYPE	  (cong_command_get_type ())
#define CONG_COMMAND(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_COMMAND_TYPE, CongCommand)
#define CONG_COMMAND_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_COMMAND_TYPE, CongCommandClass)
#define IS_CONG_COMMAND(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_COMMAND_TYPE)

typedef struct _CongCommandDetails CongCommandDetails;


struct _CongCommand
{
	GObject object;

	CongCommandDetails *priv;
};

struct _CongCommandClass
{
	GObjectClass klass;

	/* Methods? */
};

GType
cong_command_get_type (void);

CongCommand*
cong_command_construct (CongCommand *command,
			CongDocument *doc,
			const gchar *description,
			const gchar *consolidation_id);

CongCommand*
cong_command_private_new (CongDocument *doc,
			  const gchar *description,
			  const gchar *consolidation_id);

CongDocument*
cong_command_get_document (CongCommand *command);

const gchar*
cong_command_get_description (CongCommand *command);

const gchar*
cong_command_get_consolidation_id (CongCommand *command);

void
cong_command_undo (CongCommand *command);

void
cong_command_redo (CongCommand *command);

void
cong_command_merge (CongCommand *dst,
		    CongCommand *src);

gboolean
cong_command_has_ever_been_undone (CongCommand *cmd);

/* Adding Atomic Modifications: */
void
cong_command_add_modification (CongCommand *cmd,
			       CongModification *modification);

void
cong_command_add_node_make_orphan (CongCommand *cmd,
				   CongNodePtr node);

void
cong_command_add_node_add_after (CongCommand *cmd, 
				 CongNodePtr node, 
				 CongNodePtr older_sibling);

void 
cong_command_add_node_add_before (CongCommand *cmd, 
				  CongNodePtr node, 
				  CongNodePtr younger_sibling);

void 
cong_command_add_node_set_parent (CongCommand *cmd, 
				  CongNodePtr node,
				  CongNodePtr adoptive_parent); /* added to end of child list */

void 
cong_command_add_node_set_text (CongCommand *cmd, 
				CongNodePtr node, 
				const gchar *new_content);

void 
cong_command_add_node_set_attribute (CongCommand *cmd, 
				     CongNodePtr node, 
				     xmlNs *ns_ptr,
				     const gchar *name, 
				     const gchar *value);

void 
cong_command_add_node_remove_attribute (CongCommand *cmd, 
					CongNodePtr node, 
					xmlNs *ns_ptr,
					const gchar *name);
	
void 
cong_command_add_selection_change (CongCommand *cmd,
				   const CongLocation *new_logical_start,
				   const CongLocation *new_logical_end);

void 
cong_command_add_cursor_change (CongCommand *cmd,
				const CongLocation *new_location);

/* for now is always the external DTD */
void 
cong_command_add_set_dtd_ptr (CongCommand *cmd,
			      xmlDtdPtr dtd_ptr);

void 
cong_command_add_set_clipboard (CongCommand *cmd,
				const gchar* clipboard_source);

/* Adding Compound Modifications: */
void
cong_command_add_node_recursive_delete (CongCommand *cmd,
					CongNodePtr node);

/* Return true if you modify the location */
typedef gboolean
(*CongUpdateLocationCallback) (CongDocument *doc,
			       CongLocation *location, 
			       gpointer user_data);

/* Invoke the callback for the cursor and selection locations; allowing you to do something sane with them when manipulating the tree */
void
cong_command_for_each_location (CongCommand *cmd, 
				CongUpdateLocationCallback callback, 
				gpointer user_data);
void 
cong_command_add_delete_range (CongCommand *cmd,
			       CongRange *range);

void 
cong_command_add_delete_selection (CongCommand *cmd);

void 
cong_command_add_insert_text_at_cursor (CongCommand *cmd, 
					const gchar *string);

void 
cong_command_add_nullify_cursor (CongCommand *cmd);

void 
cong_command_add_nullify_selection (CongCommand *cmd);

CongNodePtr
cong_command_add_xml_frag_data_nice_split2  (CongCommand *cmd, 
					     const CongLocation *loc);

void
cong_command_add_merge_adjacent_text_nodes (CongCommand *cmd);

void
cong_command_add_merge_adjacent_text_children_of_node (CongCommand *cmd, 
						       CongNodePtr node);

gboolean
cong_command_can_add_reparent_selection (CongCommand *cmd,
					 CongNodePtr new_parent);

CongNodePtr
cong_command_add_reparent_selection (CongCommand *cmd, 
				     CongNodePtr node);

/* Splits a text/comment node in 3 and returns pointer to the middle one */
CongNodePtr
cong_command_add_node_split3 (CongCommand *cmd,
			      CongNodePtr node, 
			      int c0, 
			      int c1);

void 
cong_command_add_remove_tag (CongCommand *cmd,
			     CongNodePtr node);

void
cong_command_add_set_cursor_to_first_text_descendant (CongCommand *cmd,
						      CongNodePtr node);

void
cong_command_add_set_external_dtd (CongCommand *cmd,
				   const gchar* root_element,
				   const gchar* public_id,
				   const gchar* system_id);

gboolean 
cong_command_add_required_sub_elements (CongCommand *cmd,
				        CongNodePtr node);

G_END_DECLS

#endif
