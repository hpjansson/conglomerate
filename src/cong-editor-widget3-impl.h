/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-widget-impl.h
 *
 * Copyright (C) 2002 David Malcolm
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


/*
 * Implementation details for the CongEditorWidget
 */
#ifndef __CONG_EDITOR_WIDGET3_IMPL_H__
#define __CONG_EDITOR_WIDGET3_IMPL_H__

#include "cong-view.h"

G_BEGIN_DECLS

/* currently implemented as a GtkDrawingArea with user_data "details" pointing to a CongEditorWidget3Details */

/*
*/

#if 0
struct CongElementEditor
{
	const CongElementEditorClass *klass;

	CongEditorWidget3 *widget;

	/* An element editor actually applies to a range of sibling elements; could be a single node: */
	CongNodePtr first_node;
	CongNodePtr final_node;

	GdkRectangle window_area; /* allocated area in window space */
	GtkRequisition requisition;
};

struct CongElementEditorClass
{
	/* Methods? */
	const gchar *name;
	void (*on_recursive_delete)(CongElementEditor *element_editor);
	void (*on_recursive_self_test)(CongElementEditor *element_editor);
	gboolean (*on_document_event)(CongElementEditor *element_editor, CongDocumentEvent *event);
	void (*get_size_requisition)(CongElementEditor *element_editor, int width_hint);
	void (*allocate_child_space)(CongElementEditor *element_editor);
	void (*recursive_render)(CongElementEditor *element_editor, const GdkRectangle *window_rect);
	void (*on_button_press)(CongElementEditor *element_editor, GdkEventButton *event);
	void (*on_motion_notify)(CongElementEditor *element_editor, GdkEventMotion *event);
	void (*on_key_press)(CongElementEditor *element_editor, GdkEventKey *event);
};

void cong_element_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect);
void cong_element_editor_on_button_press(CongElementEditor *element_editor, GdkEventButton *event);
void cong_element_editor_on_motion_notify(CongElementEditor *element_editor, GdkEventMotion *event);
void cong_element_editor_on_key_press(CongElementEditor *element_editor, GdkEventKey *event);
#endif

typedef struct CongEditorWidget3Details CongEditorWidget3Details;
typedef struct CongEditorWidget3View CongEditorWidget3View;
#define CONG_EDITOR_WIDGET3_VIEW(x) ((CongEditorWidget3View*)(x))

struct CongEditorWidget3Details
{
	CongEditorWidget3 *widget;
	CongEditorWidget3View *view;

#if 0
	CongElementEditor *root_editor;

	GHashTable *hash_of_node_to_editor; /* holds all nodes which are _directly_ under the control of an editor; descendants aren't stored, but can't be determined. */
#endif
};

/* The widget "owns" a CongView, which in turn, holds a ptr back to its widget. */
struct CongEditorWidget3View
{
	CongView view;

	CongEditorWidget3 *widget;
};

/* Macro for getting details of a widget; this will eventually be a simple field lookup */
#define GET_DETAILS(editor_widget) ((CongEditorWidget3Details*)(g_object_get_data(G_OBJECT(editor_widget), "details")))

#if 0
CongNodePtr cong_element_editor_get_first_node(CongElementEditor *element_editor);
CongNodePtr cong_element_editor_get_final_node(CongElementEditor *element_editor);
gboolean cong_element_editor_responsible_for_node(CongElementEditor *element_editor, CongNodePtr node);
void cong_element_editor_recursive_delete(CongElementEditor *element_editor);
void cong_element_editor_recursive_self_test(CongElementEditor *element_editor);
gboolean cong_element_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event);
void cong_element_editor_get_size_requisition(CongElementEditor *element_editor, int width_hint);
void cong_element_editor_set_allocation(CongElementEditor *element_editor,
					gint x,
					gint y,
					gint width,
					gint height);
/* these are in window coords */
#endif

#if 0
CongElementEditor *cong_dummy_element_editor_new(CongEditorWidget3 *widget, CongNodePtr node, const gchar* message);
CongElementEditor *cong_section_head_editor_new(CongEditorWidget3 *widget, CongNodePtr node);
CongElementEditor *cong_span_text_editor_new(CongEditorWidget3 *widget, CongNodePtr first_node, CongNodePtr final_node);
#endif


G_END_DECLS

#endif
