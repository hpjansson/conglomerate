/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-command-history.h
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

#ifndef __CONG_COMMAND_HISTORY_H__
#define __CONG_COMMAND_HISTORY_H__

#include "cong-document.h"

G_BEGIN_DECLS

#define DEBUG_COMMAND_HISTORY_LIFETIMES 0

#define CONG_COMMAND_HISTORY_TYPE	  (cong_command_history_get_type ())
#define CONG_COMMAND_HISTORY(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_COMMAND_HISTORY_TYPE, CongCommandHistory)
#define CONG_COMMAND_HISTORY_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_COMMAND_HISTORY_TYPE, CongCommandHistoryClass)
#define IS_CONG_COMMAND_HISTORY(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_COMMAND_HISTORY_TYPE)

typedef struct CongCommandHistoryDetails CongCommandHistoryDetails;

struct CongCommandHistory
{
	GObject object;

	CongCommandHistoryDetails *private;
};

struct CongCommandHistoryClass
{
	GObjectClass klass;

	/* Methods? */
};

GType
cong_command_history_get_type (void);

CongCommandHistory*
cong_command_history_construct (CongCommandHistory *command_history);

CongCommandHistory*
cong_command_history_new (void);

void
cong_command_history_add_command (CongCommandHistory *command_history,
				  CongCommand *command);

gboolean
cong_command_history_can_undo (CongCommandHistory *command_history);

gboolean
cong_command_history_can_redo (CongCommandHistory *command_history);

void
cong_command_history_undo (CongCommandHistory *command_history);

void
cong_command_history_redo (CongCommandHistory *command_history);

CongCommand*
cong_command_history_get_next_undo_command (CongCommandHistory *command_history);

CongCommand*
cong_command_history_get_next_redo_command (CongCommandHistory *command_history);


G_END_DECLS

#endif
