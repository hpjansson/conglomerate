/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-creation-record.h
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

#ifndef __CONG_EDITOR_CREATION_RECORD_H__
#define __CONG_EDITOR_CREATION_RECORD_H__

#include "cong-object.h"
#include "cong-editor-line-manager.h"

G_BEGIN_DECLS

#define CONG_EDITOR_CREATION_RECORD_TYPE	  (cong_editor_creation_record_get_type ())
#define CONG_EDITOR_CREATION_RECORD(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_CREATION_RECORD_TYPE, CongEditorCreationRecord)
#define CONG_EDITOR_CREATION_RECORD_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_CREATION_RECORD_TYPE, CongEditorCreationRecordClass)
#define IS_CONG_EDITOR_CREATION_RECORD(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_CREATION_RECORD_TYPE)

CONG_DECLARE_CLASS (CongEditorCreationRecord, cong_editor_creation_record, GObject)
     
CongEditorCreationRecord*
cong_editor_creation_record_construct (CongEditorCreationRecord *creation_record,
				       CongEditorLineManager *line_manager);

CongEditorCreationRecord*
cong_editor_creation_record_new (CongEditorLineManager *line_manager);

/* Recording things: */
void
cong_editor_creation_record_add_change (CongEditorCreationRecord *creation_record,
					enum CongEditorCreationEvent event,
					CongEditorLineIter *iter_before,
					CongEditorLineIter *iter_after);
/* Destroying it all: */
void
cong_editor_creation_record_undo_changes (CongEditorCreationRecord *creation_record);

G_END_DECLS

#endif



