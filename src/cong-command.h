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

G_BEGIN_DECLS

#define DEBUG_COMMAND_LIFETIMES 0

#define CONG_COMMAND_TYPE	  (cong_command_get_type ())
#define CONG_COMMAND(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_COMMAND_TYPE, CongCommand)
#define CONG_COMMAND_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_COMMAND_TYPE, CongCommandClass)
#define IS_CONG_COMMAND(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_COMMAND_TYPE)

typedef struct CongCommandDetails CongCommandDetails;


/**
 */
struct CongCommand
{
	GObject object;

	CongCommandDetails *private;
};

struct CongCommandClass
{
	GObjectClass klass;

	/* Methods? */
};

GType
cong_command_get_type (void);

CongCommand*
cong_command_construct (CongCommand *command,
			CongDocument *doc,
			const gchar *description);

CongCommand*
cong_command_new (CongDocument *doc,
		  const gchar *description);

CongDocument*
cong_command_get_document (CongCommand *command);

const gchar*
cong_command_get_description (CongCommand *command);

void
cong_command_undo (CongCommand *command);

void
cong_command_redo (CongCommand *command);

/* Modification: */
void
cong_command_add_node_make_orphan (CongCommand *command,
				   CongNodePtr node);

void
cong_command_add_node_add_after (CongDocument *doc, 
				 CongNodePtr node, 
				 CongNodePtr older_sibling);

void 
cong_command_add_node_add_before (CongDocument *doc, 
				  CongNodePtr node, 
				  CongNodePtr younger_sibling);

void 
cong_command_add_node_set_parent (CongDocument *doc, 
				  CongNodePtr node,
				  CongNodePtr adoptive_parent); /* added to end of child list */

void 
cong_command_add_node_set_text (CongDocument *doc, 
				CongNodePtr node, 
				const xmlChar *new_content);

void 
cong_command_add_node_set_attribute (CongDocument *doc, 
				     CongNodePtr node, 
				     const xmlChar *name, 
				     const xmlChar *value);

void 
cong_command_add_node_remove_attribute (CongDocument *doc, 
					CongNodePtr node, 
					const xmlChar *name);
	
void 
cong_command_add_selection_change (CongDocument *doc);

void 
cong_command_add_cursor_change (CongDocument *doc);

void 
cong_command_add_set_external_dtd (CongDocument *doc,
				   const gchar* root_element,
				   const gchar* public_id,
				   const gchar* system_id);



G_END_DECLS

#endif
