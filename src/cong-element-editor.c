/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-element-editor.c
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

#include "global.h"
#include "cong-editor-widget-impl.h"

CongNodePtr cong_element_editor_get_first_node(CongElementEditor *element_editor)
{
	g_return_val_if_fail(element_editor, NULL);
	
	return element_editor->first_node;
}

CongNodePtr cong_element_editor_get_final_node(CongElementEditor *element_editor)
{
	g_return_val_if_fail(element_editor, NULL);
	
	return element_editor->final_node;
}

gboolean cong_element_editor_responsible_for_node(CongElementEditor *element_editor, CongNodePtr node)
{
	/* returns TRUE if the node is within the "sibling range" (as opposed to being a descendant) */

	CongNodePtr iter;

	g_return_val_if_fail(element_editor, FALSE);
	g_return_val_if_fail(node, FALSE);

	g_assert(element_editor->first_node);
	g_assert(element_editor->final_node);
	g_assert(element_editor->first_node->parent == element_editor->final_node->parent);

	for (iter=element_editor->first_node; iter!=element_editor->final_node; iter=iter->next) {
		g_assert(iter);

		if (iter==node) {
			return TRUE;
		}
	}

	return (iter==element_editor->final_node);
}

void cong_element_editor_recursive_delete(CongElementEditor *element_editor)
{
	g_return_if_fail(element_editor);

	g_assert(element_editor->klass);
	g_assert(element_editor->klass->on_recursive_delete);

	cong_editor_widget2_unregister_element_editor(element_editor->widget, element_editor);

	element_editor->klass->on_recursive_delete(element_editor);
}

void cong_element_editor_recursive_self_test(CongElementEditor *element_editor)
{
	g_return_if_fail(element_editor);

	g_assert(element_editor->klass);
	g_assert(element_editor->klass->on_recursive_self_test);

	return element_editor->klass->on_recursive_self_test(element_editor);
}

gboolean cong_element_editor_on_document_event(CongElementEditor *element_editor, CongDocumentEvent *event)
{
	g_return_val_if_fail(element_editor, FALSE);
	g_return_val_if_fail(event, FALSE);

	g_assert(element_editor->klass);
	g_assert(element_editor->klass->on_document_event);

	return element_editor->klass->on_document_event(element_editor, event);	
}

void cong_element_editor_get_size_requisition(CongElementEditor *element_editor, int width_hint)
{
	g_return_if_fail(element_editor);

	g_assert(element_editor->klass);
	g_assert(element_editor->klass->get_size_requisition);

	element_editor->klass->get_size_requisition(element_editor, width_hint);	
}

void cong_element_editor_set_allocation(CongElementEditor *element_editor,
					gint x,
					gint y,
					gint width,
					gint height)
/* these are in window coords */
{
	g_return_if_fail(element_editor);
	
	element_editor->window_area.x = x;
	element_editor->window_area.y = y;
	element_editor->window_area.width = width;
	element_editor->window_area.height = height;

	/* Call hook to recursively allocate space to children: */
	g_assert(element_editor->klass);
	g_assert(element_editor->klass->allocate_child_space);

	element_editor->klass->allocate_child_space(element_editor);
}

void cong_element_editor_recursive_render(CongElementEditor *element_editor, const GdkRectangle *window_rect)
{
	g_return_if_fail(element_editor);
	g_return_if_fail(window_rect);

	g_assert(element_editor->klass);
	g_assert(element_editor->klass->recursive_render);

	element_editor->klass->recursive_render(element_editor, window_rect);
}

void cong_element_editor_on_button_press(CongElementEditor *element_editor, GdkEventButton *event)
{
	g_return_if_fail(element_editor);
	g_return_if_fail(event);

	g_assert(element_editor->klass);
	g_assert(element_editor->klass->on_button_press);

	element_editor->klass->on_button_press(element_editor, event);
}

void cong_element_editor_on_motion_notify(CongElementEditor *element_editor, GdkEventMotion *event)
{
	g_return_if_fail(element_editor);
	g_return_if_fail(event);

	g_assert(element_editor->klass);
	g_assert(element_editor->klass->on_motion_notify);

	element_editor->klass->on_motion_notify(element_editor, event);
}

void cong_element_editor_on_key_press(CongElementEditor *element_editor, GdkEventKey *event)
{
	g_return_if_fail(element_editor);
	g_return_if_fail(event);

	g_assert(element_editor->klass);
	g_assert(element_editor->klass->on_key_press);

	element_editor->klass->on_key_press(element_editor, event);
}
