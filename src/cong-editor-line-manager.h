/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-line-manager.h
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

#ifndef __CONG_EDITOR_LINE_MANAGER_H__
#define __CONG_EDITOR_LINE_MANAGER_H__

#include "cong-document.h"
#include "cong-editor-widget.h"
#include "cong-object.h"

G_BEGIN_DECLS

enum CongEditorCreationEvent
{
	CONG_EDITOR_CREATION_EVENT_BEGIN_LINE,
	CONG_EDITOR_CREATION_EVENT_END_LINE,
	CONG_EDITOR_CREATION_EVENT_ADD_AREA
};

#define DEBUG_EDITOR_LINE_MANAGER_LIFETIMES 0

#define CONG_EDITOR_LINE_MANAGER_TYPE	      (cong_editor_line_manager_get_type ())
#define CONG_EDITOR_LINE_MANAGER(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_LINE_MANAGER_TYPE, CongEditorLineManager)
#define CONG_EDITOR_LINE_MANAGER_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_LINE_MANAGER_TYPE, CongEditorLineManagerClass)
#define IS_CONG_EDITOR_LINE_MANAGER(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_LINE_MANAGER_TYPE)

CONG_DECLARE_CLASS_BEGIN (CongEditorLineManager, cong_editor_line_manager, GObject)

     /* FIXME: all of these methods are temporary for now: */

     /* Factory method for making an iterator of the appropriate type, at the start of the possible areas for this manager: */
     CongEditorLineIter* (*make_iter) (CongEditorLineManager *line_manager);

     /* Manipulating the lines: */
     /* Creating areas: */
     void (*begin_line) (CongEditorLineManager *line_manager,
			 CongEditorCreationRecord *creation_record,
			 CongEditorLineIter *line_iter);
     void (*add_to_line) (CongEditorLineManager *line_manager,
			  CongEditorCreationRecord *creation_record,
			  CongEditorLineIter *line_iter,
			  CongEditorArea *area);
     void (*end_line) (CongEditorLineManager *line_manager,
		       CongEditorCreationRecord *creation_record,
		       CongEditorLineIter *line_iter);

     /* Destroying them: */
#if 1
     void (*undo_change) (CongEditorLineManager *line_manager,
			  enum CongEditorCreationEvent event,
			  CongEditorLineIter *iter_before,
			  CongEditorLineIter *iter_after);
#else
     void (*delete_areas) (CongEditorLineManager *line_manager,
			   CongEditorLineIter *start_iter,
			   CongEditorLineIter *end_iter);
#endif

     /* Getting data about the lines: */
     gint (*get_line_width) (CongEditorLineManager *line_manager,
			     CongEditorLineIter *line_iter);
     gint (*get_current_indent) (CongEditorLineManager *line_manager,
				 CongEditorLineIter *line_iter);

CONG_DECLARE_CLASS_END ()

void
cong_editor_line_manager_construct (CongEditorLineManager *line_manager,
				    CongEditorWidget3 *widget);

CongEditorWidget3*
cong_editor_line_manager_get_widget (CongEditorLineManager *line_manager);

void 
cong_editor_line_manager_add_node (CongEditorLineManager *line_manager,
				   CongEditorNode *node);
void 
cong_editor_line_manager_remove_node (CongEditorLineManager *line_manager,
				      CongEditorNode *node);

/* V-Func invocation: */
void
cong_editor_line_manager_begin_line (CongEditorLineManager *line_manager,
				     CongEditorCreationRecord *creation_record,
				     CongEditorLineIter *line_iter);

void
cong_editor_line_manager_add_to_line (CongEditorLineManager *line_manager,
				      CongEditorCreationRecord *creation_record,
				      CongEditorLineIter *line_iter,
				      CongEditorArea *area);

void
cong_editor_line_manager_end_line (CongEditorLineManager *line_manager,
				   CongEditorCreationRecord *creation_record,
				   CongEditorLineIter *line_iter);

#if 1
void 
cong_editor_line_manager_undo_change (CongEditorLineManager *line_manager,
				      enum CongEditorCreationEvent event,
				      CongEditorLineIter *iter_before,
				      CongEditorLineIter *iter_after);
#else
void 
cong_editor_line_manager_delete_areas (CongEditorLineManager *line_manager,
				       CongEditorLineIter *start_iter,
				       CongEditorLineIter *end_iter);
#endif

gint
cong_editor_line_manager_get_line_width (CongEditorLineManager *line_manager,
					 CongEditorLineIter *line_iter);

gint
cong_editor_line_manager_get_current_indent (CongEditorLineManager *line_manager,
					     CongEditorLineIter *line_iter);

gint
cong_editor_line_manager_get_current_width_available (CongEditorLineManager *line_manager,
						      CongEditorLineIter *line_iter);

G_END_DECLS

#endif
