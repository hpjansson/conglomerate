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
	g_return_if_fail (IS_CONG_COMMAND(command));

	g_message ("cong_command_undo(\"%s\")", cong_command_get_description(command));
}

void
cong_command_redo (CongCommand *command)
{
	g_return_if_fail (IS_CONG_COMMAND(command));

	g_message ("cong_command_redo(\"%s\")", cong_command_get_description(command));
}
