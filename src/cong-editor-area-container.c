/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-container.c
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
#include "cong-editor-area-container.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

enum {
	CHILDREN_CHANGED,

	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

struct CongEditorAreaContainerDetails
{
	int dummy;
};

/* Method implementation prototypes: */
CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_area_container, add_child);
CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_area_container, add_child_after);
CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_area_container, remove_child);
CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_editor_area_container, remove_all_children);


/* Signal handler declarations: */
#if 0
static void 
handle_children_changed (CongEditorAreaContainer* area_container);
#endif

static void
on_child_flush_requisition_cache (CongEditorArea *child_area,
				  GtkOrientation orientation,
				  gpointer user_data);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaContainer, 
			cong_editor_area_container,
			CongEditorArea,
			CONG_EDITOR_AREA_TYPE );

static void
cong_editor_area_container_class_init (CongEditorAreaContainerClass *klass)
{
#if 0
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);
#endif

	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_area_container,
					      add_child);

	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_area_container,
					      add_child_after);

	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_area_container,
					      remove_child);

	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_editor_area_container,
					      remove_all_children);

	/* Set up the various signals: */
	signals[CHILDREN_CHANGED] = g_signal_new ("children_changed",
						  CONG_EDITOR_AREA_TYPE,
						  G_SIGNAL_RUN_FIRST,
						  G_STRUCT_OFFSET(CongEditorAreaContainerClass, children_changed),
						  NULL, NULL,
						  g_cclosure_marshal_VOID__VOID,
						  G_TYPE_NONE, 
						  0);


}

static void
cong_editor_area_container_instance_init (CongEditorAreaContainer *area_container)
{
	area_container->private = g_new0(CongEditorAreaContainerDetails,1);
}

/* Exported function definitions: */
/**
 * cong_editor_area_container_construct:
 * @area_container:
 * @editor_widget:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_container_construct (CongEditorAreaContainer *area_container,
				      CongEditorWidget3 *editor_widget)
{
	cong_editor_area_construct (CONG_EDITOR_AREA(area_container),
				    editor_widget);

	return CONG_EDITOR_AREA (area_container);
}

/**
 * cong_editor_area_container_add_child:
 * @area_container:
 * @child:
 *
 * TODO: Write me
 */
void
cong_editor_area_container_add_child ( CongEditorAreaContainer *area_container,
				       CongEditorArea *child)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA(area_container));
	g_return_if_fail (IS_CONG_EDITOR_AREA(child));

#if 0
	g_return_if_fail (NULL!= cong_editor_area_get_parent (child));
#endif

	CONG_EEL_CALL_METHOD (CONG_EDITOR_AREA_CONTAINER_CLASS,
			      area_container,
			      add_child, 
			      (area_container, child));

	cong_editor_area_container_children_changed ( area_container);
}

/**
 * cong_editor_area_container_add_child_after:
 * @area_container:
 * @new_child:
 * @relative_to:
 *
 * TODO: Write me
 */
void
cong_editor_area_container_add_child_after ( CongEditorAreaContainer *area_container,
					     CongEditorArea *new_child,
					     CongEditorArea *relative_to)
{
	g_return_if_fail (IS_CONG_EDITOR_AREA_CONTAINER(area_container));
	g_return_if_fail (IS_CONG_EDITOR_AREA(new_child));
	g_return_if_fail (IS_CONG_EDITOR_AREA(relative_to));

#if 0
	g_return_if_fail (NULL!= cong_editor_area_get_parent (child));
#endif

	CONG_EEL_CALL_METHOD (CONG_EDITOR_AREA_CONTAINER_CLASS,
			      area_container,
			      add_child_after, 
			      (area_container, new_child, relative_to));

	cong_editor_area_container_children_changed ( area_container);
}

/**
 * cong_editor_area_container_remove_child:
 * @area_container:
 * @child:
 *
 * TODO: Write me
 */
void
cong_editor_area_container_remove_child ( CongEditorAreaContainer *area_container,
					  CongEditorArea *child)
{
	g_return_if_fail (area_container);
	g_return_if_fail (child);

#if 0
	g_message ("cong_editor_area_container_remove_child(%s,%s)", 
		   G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(area_container)),
		   G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(child)));
#endif

#if 0
	g_return_if_fail (NULL!= cong_editor_area_get_parent (child));
#endif

	CONG_EEL_CALL_METHOD (CONG_EDITOR_AREA_CONTAINER_CLASS,
			      area_container,
			      remove_child, 
			      (area_container, child));

	cong_editor_area_container_children_changed ( area_container );
}

/**
 * cong_editor_area_remove_all_children:
 * @area_container:
 *
 * TODO: Write me
 */
void 
cong_editor_area_remove_all_children ( CongEditorAreaContainer *area_container)
{
	g_return_if_fail (area_container);

	CONG_EEL_CALL_METHOD (CONG_EDITOR_AREA_CONTAINER_CLASS,
			      area_container,
			      remove_all_children, 
			      (area_container));

	cong_editor_area_container_children_changed ( area_container );
}

/**
 * cong_editor_area_container_children_changed:
 * @area_container:
 *
 * TODO: Write me
 */
void
cong_editor_area_container_children_changed ( CongEditorAreaContainer *area_container)
{
	g_signal_emit (G_OBJECT(area_container),
		       signals[CHILDREN_CHANGED], 0);

	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_container), GTK_ORIENTATION_HORIZONTAL);
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_container), GTK_ORIENTATION_VERTICAL);
}

/* Protected:  For implementing subclasses */
/**
 * cong_editor_area_container_protected_postprocess_add_non_internal_child:
 * @area_container:
 * @child:
 *
 * TODO: Write me
 */
void
cong_editor_area_container_protected_postprocess_add_non_internal_child (CongEditorAreaContainer *area_container,
									 CongEditorArea *child)
{
	g_return_if_fail ( IS_CONG_EDITOR_AREA_CONTAINER(area_container));
	g_return_if_fail ( IS_CONG_EDITOR_AREA(child));

	g_signal_connect (G_OBJECT(child),
			  "flush_requisition_cache",
			  G_CALLBACK(on_child_flush_requisition_cache),
			  area_container);	
}


/* Method implementation definitions: */

/* Signal handler definitions: */
#if 0
static void 
handle_children_changed (CongEditorAreaContainer* area_container)
{
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_container), GTK_ORIENTATION_HORIZONTAL);
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_container), GTK_ORIENTATION_VERTICAL);
}
#endif

static void
on_child_flush_requisition_cache (CongEditorArea *child_area,
				  GtkOrientation orientation,
				  gpointer user_data)
{
	CongEditorAreaContainer *area_container = CONG_EDITOR_AREA_CONTAINER(user_data);

#if 0
	g_message("on_child_flush_requisition_cache");
#endif

	g_return_if_fail (IS_CONG_EDITOR_AREA(child_area) );
	
	/* One of children has changed its requisition; so must we: */
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_container), GTK_ORIENTATION_HORIZONTAL);
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_container), GTK_ORIENTATION_VERTICAL);
}
