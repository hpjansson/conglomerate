/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-command-history.c
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
#include "cong-command-history.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#include "cong-document.h"
#include "cong-command.h"
#include "cong-command-history.h"

#define PRIVATE(x) ((x)->private)

enum {
	CHANGED,

	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

static void
emit_changed (CongCommandHistory *history);

struct CongCommandHistoryDetails
{
	GList *list_of_commands_to_undo; /* front of list is most recent command */
	GList *list_of_commands_to_redo; /* front of list is most recent command */
};

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongCommandHistory, 
			cong_command_history,
			GObject,
			G_TYPE_OBJECT );

static void
cong_command_history_class_init (CongCommandHistoryClass *klass)
{
	signals[CHANGED] = g_signal_new ("changed",
					 CONG_COMMAND_HISTORY_TYPE,
					 G_SIGNAL_RUN_FIRST,
					 0,
					 NULL, NULL,
					 g_cclosure_marshal_VOID__VOID,
					 G_TYPE_NONE,
					 0);
}

static void
cong_command_history_instance_init (CongCommandHistory *history)
{
	history->private = g_new0(CongCommandHistoryDetails,1);
}

/**
 * cong_command_history_construct:
 * @command_history:
 *
 * TODO: Write me
 * Returns:
 */
CongCommandHistory*
cong_command_history_construct (CongCommandHistory *command_history)
{
	return command_history;
}

/**
 * cong_command_history_new:
 *
 * TODO: Write me
 * Returns:
 */
CongCommandHistory*
cong_command_history_new (void)
{
	return cong_command_history_construct (CONG_COMMAND_HISTORY(g_object_new (CONG_COMMAND_HISTORY_TYPE, NULL)));
}

/**
 * cong_command_history_add_command:
 * @command_history:
 * @command:
 *
 * TODO: Write me
 */
void
cong_command_history_add_command (CongCommandHistory *command_history,
				  CongCommand *command)
{
	g_return_if_fail (IS_CONG_COMMAND_HISTORY(command_history));
	g_return_if_fail (IS_CONG_COMMAND(command));

	g_message ("cong_command_history_add_command (\"%s\")", cong_command_get_description (command));

	/* Add the command to head of "undo" list: */
	PRIVATE(command_history)->list_of_commands_to_undo = g_list_prepend (PRIVATE(command_history)->list_of_commands_to_undo,
									     command);
	g_object_ref (G_OBJECT (command));

	/* Empty the pending "redo" list: */
	while (PRIVATE(command_history)->list_of_commands_to_redo) {
		
		CongCommand *cmd = CONG_COMMAND(PRIVATE(command_history)->list_of_commands_to_redo->data);
		g_object_unref (G_OBJECT(cmd));

		PRIVATE(command_history)->list_of_commands_to_redo = g_list_delete_link (PRIVATE(command_history)->list_of_commands_to_redo,
											 PRIVATE(command_history)->list_of_commands_to_redo);
	}

	emit_changed (command_history);
}

/**
 * cong_command_history_can_undo:
 * @command_history:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_command_history_can_undo (CongCommandHistory *command_history)
{
	g_return_val_if_fail (IS_CONG_COMMAND_HISTORY(command_history), FALSE);

	return PRIVATE(command_history)->list_of_commands_to_undo!=NULL;
}

/**
 * cong_command_history_can_redo:
 * @command_history:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_command_history_can_redo (CongCommandHistory *command_history)
{
	g_return_val_if_fail (IS_CONG_COMMAND_HISTORY(command_history), FALSE);

	return PRIVATE(command_history)->list_of_commands_to_redo!=NULL;
}

/**
 * cong_command_history_undo:
 * @command_history:
 *
 * TODO: Write me
 */
void
cong_command_history_undo (CongCommandHistory *command_history)
{
	g_return_if_fail (IS_CONG_COMMAND_HISTORY(command_history));

#if 0
	g_message ("cong_command_history_undo()");
#endif

	if (PRIVATE(command_history)->list_of_commands_to_undo) {
		
		CongCommand *cmd = CONG_COMMAND(PRIVATE(command_history)->list_of_commands_to_undo->data);

		PRIVATE(command_history)->list_of_commands_to_undo = g_list_delete_link (PRIVATE(command_history)->list_of_commands_to_undo,
											 PRIVATE(command_history)->list_of_commands_to_undo);
		PRIVATE(command_history)->list_of_commands_to_redo = g_list_prepend (PRIVATE(command_history)->list_of_commands_to_redo,
										     cmd);

		cong_command_undo (cmd);

		emit_changed (command_history);
	} else {
		g_message ("Nothing to undo");
	}

}

/**
 * cong_command_history_redo:
 * @command_history:
 *
 * TODO: Write me
 */
void
cong_command_history_redo (CongCommandHistory *command_history)
{
	g_return_if_fail (IS_CONG_COMMAND_HISTORY(command_history));

#if 0
	g_message ("cong_command_history_redo()");
#endif

	if (PRIVATE(command_history)->list_of_commands_to_redo) {
		
		CongCommand *cmd = CONG_COMMAND(PRIVATE(command_history)->list_of_commands_to_redo->data);

		PRIVATE(command_history)->list_of_commands_to_redo = g_list_delete_link (PRIVATE(command_history)->list_of_commands_to_redo,
											 PRIVATE(command_history)->list_of_commands_to_redo);
		PRIVATE(command_history)->list_of_commands_to_undo = g_list_prepend (PRIVATE(command_history)->list_of_commands_to_undo,
										     cmd);

		cong_command_redo (cmd);

		emit_changed (command_history);
	} else {
		g_message ("Nothing to redo");
	}
}

/**
 * cong_command_history_get_next_undo_command:
 * @command_history:
 *
 * TODO: Write me
 * Returns:
 */
CongCommand*
cong_command_history_get_next_undo_command (CongCommandHistory *command_history)
{
	g_return_val_if_fail (IS_CONG_COMMAND_HISTORY(command_history), NULL);

	if (PRIVATE(command_history)->list_of_commands_to_undo) {
		return CONG_COMMAND (PRIVATE(command_history)->list_of_commands_to_undo->data);
	} else {
		return NULL;
	} 
}

/**
 * cong_command_history_get_next_redo_command:
 * @command_history:
 *
 * TODO: Write me
 * Returns:
 */
CongCommand*
cong_command_history_get_next_redo_command (CongCommandHistory *command_history)
{
	g_return_val_if_fail (IS_CONG_COMMAND_HISTORY(command_history), NULL);

	if (PRIVATE(command_history)->list_of_commands_to_redo) {
		return CONG_COMMAND (PRIVATE(command_history)->list_of_commands_to_redo->data);
	} else {
		return NULL;
	} 
}

/* Internal stuff: */
static void
emit_changed (CongCommandHistory *history)
{
	g_signal_emit (G_OBJECT(history),
		       signals[CHANGED], 0);
}
