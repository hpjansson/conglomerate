/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-container.h
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

#ifndef __CONG_EDITOR_AREA_CONTAINER_H__
#define __CONG_EDITOR_AREA_CONTAINER_H__

#include "cong-editor-area.h"

G_BEGIN_DECLS

typedef struct CongEditorAreaContainer CongEditorAreaContainer;
typedef struct CongEditorAreaContainerClass CongEditorAreaContainerClass;
typedef struct CongEditorAreaContainerDetails CongEditorAreaContainerDetails;

#define CONG_EDITOR_AREA_CONTAINER_TYPE	   (cong_editor_area_container_get_type ())
#define CONG_EDITOR_AREA_CONTAINER(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_CONTAINER_TYPE, CongEditorAreaContainer)
#define CONG_EDITOR_AREA_CONTAINER_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_CONTAINER_TYPE, CongEditorAreaContainerClass)
#define IS_CONG_EDITOR_AREA_CONTAINER(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_CONTAINER_TYPE)

/**
 * Containers hold a reference on their children.
 * 
 * Areas don't know which their parents are (for now)
 */
struct CongEditorAreaContainer
{
	CongEditorArea area;

	CongEditorAreaContainerDetails *private;
};

struct CongEditorAreaContainerClass
{
	CongEditorAreaClass klass;

	void (*add_child) ( CongEditorAreaContainer *area_container,
			    CongEditorArea *child,
			    gboolean add_to_end);

	void (*add_child_after) ( CongEditorAreaContainer *area_container,
				  CongEditorArea *new_child,
				  CongEditorArea *relative_to);

	void (*remove_child) ( CongEditorAreaContainer *area_container,
			       CongEditorArea *child);

	void (*remove_all_children) ( CongEditorAreaContainer *area_container);

	void (*for_each) (CongEditorArea *editor_area, 
			  CongEditorAreaCallbackFunc func, 
			  gpointer user_data);

	void (*children_changed) (CongEditorAreaContainer* area_container);
};

GType
cong_editor_area_container_get_type (void);

CongEditorArea*
cong_editor_area_container_construct (CongEditorAreaContainer *area_container,
				      CongEditorWidget3 *editor_widget);

void
cong_editor_area_container_add_child (CongEditorAreaContainer *area_container,
				      CongEditorArea *child,
				      gboolean add_to_end);

void
cong_editor_area_container_add_child_after (CongEditorAreaContainer *area_container,
					    CongEditorArea *new_child,
					    CongEditorArea *relative_to);

void
cong_editor_area_container_remove_child (CongEditorAreaContainer *area_container,
					 CongEditorArea *child);

void 
cong_editor_area_remove_all_children ( CongEditorAreaContainer *area_container);

void
cong_editor_area_container_children_changed (CongEditorAreaContainer *area_container);


/* Iterate over all "non-internal" children: */
void 
cong_editor_area_container_for_each (CongEditorArea *editor_area, 
				     CongEditorAreaCallbackFunc func, 
				     gpointer user_data);

/* Protected:  For implementing subclasses */
void
cong_editor_area_container_protected_postprocess_add_non_internal_child (CongEditorAreaContainer *area_container,
									 CongEditorArea *child);

G_END_DECLS

#endif
