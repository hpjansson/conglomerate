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
#include "cong-node-modification.h"
#include "cong-node-modification-make-orphan.h"
#include "cong-node-modification-add-after.h"
#include "cong-node-modification-add-before.h"
#include "cong-node-modification-set-parent.h"
#include "cong-node-modification-set-text.h"
#include "cong-node-modification-set-attribute.h"
#include "cong-node-modification-remove-attribute.h"

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
cong_command_add_selection_change (CongCommand *cmd)
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
cong_command_add_cursor_change (CongCommand *cmd)
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
