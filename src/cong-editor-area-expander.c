/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-expander.c
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
#include "cong-editor-area-expander.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaExpanderDetails
{
	gboolean target_state;
	GtkExpanderStyle target_expander_style;
	GtkExpanderStyle current_expander_style;
	guint timeout_handler_id;
};

enum {
	EXPANSION_CHANGED,

	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

/* Method implementation prototypes: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect);

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint);

static gboolean
on_button_press (CongEditorArea *editor_area, 
		 GdkEventButton *event,
		 gpointer user_data);

gint
on_timeout (gpointer data);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaExpander, 
			cong_editor_area_expander,
			CongEditorArea,
			CONG_EDITOR_AREA_TYPE );

static void
cong_editor_area_expander_class_init (CongEditorAreaExpanderClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);

	area_klass->render_self = render_self;
	area_klass->calc_requisition = calc_requisition;

	signals[EXPANSION_CHANGED] = g_signal_new ("expansion_changed",
						   CONG_EDITOR_AREA_EXPANDER_TYPE,
						   G_SIGNAL_RUN_FIRST,
						   0,
						   NULL, NULL,
						   g_cclosure_marshal_VOID__VOID,
						   G_TYPE_NONE,
						   0);
}

static void
cong_editor_area_expander_instance_init (CongEditorAreaExpander *area_expander)
{
	area_expander->private = g_new0(CongEditorAreaExpanderDetails,1);
}


static gboolean
on_signal_motion_notify_for_area (CongEditorArea *editor_area, 
				  GdkEventButton *event,
				  gpointer user_data)
{

	CongEditorWidget3* editor_widget;

	editor_widget = cong_editor_area_get_widget (editor_area);			

	cong_editor_widget3_set_prehighlight_editor_area (editor_widget,
							  editor_area);

	return TRUE;
}


void
cong_editor_area_connect_motion_notify_prelight (CongEditorArea *area)
{
	g_signal_connect (G_OBJECT(area),
			  "motion_notify_event",
			  G_CALLBACK(on_signal_motion_notify_for_area),
			  NULL);
}


/* Exported function definitions: */
CongEditorArea*
cong_editor_area_expander_construct (CongEditorAreaExpander *area_expander,
				     CongEditorWidget3 *editor_widget,
				     gboolean expanded)
{
	cong_editor_area_construct (CONG_EDITOR_AREA(area_expander),
				    editor_widget);

	PRIVATE (area_expander)->target_state = expanded;
	PRIVATE (area_expander)->target_expander_style = expanded ? GTK_EXPANDER_EXPANDED : GTK_EXPANDER_COLLAPSED;
	PRIVATE (area_expander)->current_expander_style = PRIVATE(area_expander)->target_expander_style;
	
	g_signal_connect (G_OBJECT(area_expander),
			  "button_press_event",
			  G_CALLBACK(on_button_press),
			  NULL);

	cong_editor_area_connect_motion_notify_prelight (CONG_EDITOR_AREA(area_expander));

	return CONG_EDITOR_AREA (area_expander);
}

CongEditorArea*
cong_editor_area_expander_new (CongEditorWidget3 *editor_widget,
			       gboolean expanded)
{
	return cong_editor_area_expander_construct
		(g_object_new (CONG_EDITOR_AREA_EXPANDER_TYPE, NULL),
		 editor_widget,
		 expanded);
}

gboolean
cong_editor_area_expander_get_state (CongEditorAreaExpander *area_expander)
{
	return PRIVATE (area_expander)->target_state;
}


/* Method implementation definitions: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect)
{
	CongEditorAreaExpander *area_expander = CONG_EDITOR_AREA_EXPANDER(area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);

	gtk_paint_expander (gtk_widget_get_style (GTK_WIDGET(cong_editor_area_get_widget (area))),
			    window,			    
			    cong_editor_area_get_state (area),
			    (GdkRectangle*)widget_rect,
			    GTK_WIDGET (cong_editor_area_get_widget (area)),
			    NULL,
			    rect->x+(rect->width/2),
			    rect->y+(rect->height/2),
			    PRIVATE(area_expander)->current_expander_style);
}

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
#if 0
	CongEditorAreaExpander *area_expander = CONG_EDITOR_AREA_EXPANDER(area);
#endif
	gint expander_size;
	
	gtk_widget_style_get (GTK_WIDGET (cong_editor_area_get_widget (area)),
			      "expander_size", &expander_size,
			      NULL);

	return expander_size;
}

static gboolean
on_button_press (CongEditorArea *editor_area, 
		 GdkEventButton *event,
		 gpointer user_data)
{
	CongEditorAreaExpander *area_expander = CONG_EDITOR_AREA_EXPANDER (editor_area);

	if (1==event->button) {
		/* Normally the left mouse button: */

		PRIVATE (area_expander)->target_state = !PRIVATE (area_expander)->target_state;
		PRIVATE (area_expander)->target_expander_style = PRIVATE (area_expander)->target_state ? GTK_EXPANDER_EXPANDED : GTK_EXPANDER_COLLAPSED;

		if (PRIVATE(area_expander)->target_expander_style == PRIVATE(area_expander)->current_expander_style) {
			if (PRIVATE(area_expander)->timeout_handler_id) {
				gtk_timeout_remove (PRIVATE(area_expander)->timeout_handler_id);
				PRIVATE(area_expander)->timeout_handler_id = 0;
			}

			g_signal_emit (G_OBJECT(editor_area),
				       signals[EXPANSION_CHANGED], 0);
			
		} else {
			if (!PRIVATE(area_expander)->timeout_handler_id) {
				PRIVATE(area_expander)->timeout_handler_id = gtk_timeout_add (10,
											      on_timeout,
											      area_expander);
			}
		}

		return TRUE;
	} else {
		return FALSE;
	}
}

gint
on_timeout (gpointer user_data)
{
	CongEditorAreaExpander *area_expander = CONG_EDITOR_AREA_EXPANDER (user_data);

	g_assert (area_expander);
	g_assert (PRIVATE(area_expander)->target_expander_style != PRIVATE(area_expander)->current_expander_style);

	if (PRIVATE(area_expander)->target_expander_style < PRIVATE(area_expander)->current_expander_style) {
		PRIVATE(area_expander)->current_expander_style--;
	} else {
		PRIVATE(area_expander)->current_expander_style++;
	}

	cong_editor_area_queue_redraw (CONG_EDITOR_AREA (area_expander));

	/* Have we reached the target style yet? */
	if (PRIVATE(area_expander)->target_expander_style == PRIVATE(area_expander)->current_expander_style) {
		
		g_signal_emit (G_OBJECT(area_expander),
			       signals[EXPANSION_CHANGED], 0);

		/* Stop animation: */
		PRIVATE(area_expander)->timeout_handler_id = 0;
		return FALSE;		
	} else {

		/* Keep animating: */
		return TRUE;
	}
	
}
