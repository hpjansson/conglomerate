/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-structural.c
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
#include "cong-editor-area-structural.h"
#include <libgnome/gnome-macros.h>

#include "cong-app.h"
#include "cong-editor-area-text.h"
#include "cong-editor-area-expander.h"
#include "cong-editor-area-composer.h"
#include "cong-editor-area-spacer.h"
#include "cong-editor-area-pixbuf.h"
#include "cong-util.h"
#include "cong-dispspec.h"

#define PRIVATE(x) ((x)->private)

#define V_SPACING (4)
#define H_SPACING (4)
#define H_INDENT (4)
#define FRAGMENT_WIDTH (45)

struct CongEditorAreaStructuralDetails
{
	GdkColor *col_array[CONG_DISPSPEC_GC_USAGE_NUM];
	GdkGC* gc_array[CONG_DISPSPEC_GC_USAGE_NUM];

	CongEditorArea *title_vcompose;
	/****/ /* anon v-spacer */
	/****/ CongEditorArea *title_hcompose;
	/********/ /* anon h-spacer */
	/********/ CongEditorArea *title_expander;
	/********/ CongEditorArea *title_pixbuf;
	/********/ CongEditorArea *title_text;
	/****/ /* anon v-spacer */
	CongEditorArea *inner_bin;
};

/* Method implementation prototypes: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect);

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint);

static void
allocate_child_space (CongEditorArea *area);

static CongEditorArea*
for_all (CongEditorArea *editor_area, 
	 CongEditorAreaCallbackFunc func, 
	 gpointer user_data);

static void
add_child (CongEditorAreaContainer *area_container,
	   CongEditorArea *child);

static void
on_expansion_changed (CongEditorAreaExpander *area_expander,
		      gpointer user_data);


static void 
state_changed (CongEditorArea *area);


/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaStructural, 
			cong_editor_area_structural,
			CongEditorAreaBin,
			CONG_EDITOR_AREA_BIN_TYPE );

static void
cong_editor_area_structural_class_init (CongEditorAreaStructuralClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);
	CongEditorAreaContainerClass *container_klass = CONG_EDITOR_AREA_CONTAINER_CLASS(klass);

	area_klass->render_self = render_self;
	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
	area_klass->for_all = for_all;
	area_klass->state_changed = state_changed;

	container_klass->add_child = add_child;
}

static void
cong_editor_area_structural_instance_init (CongEditorAreaStructural *area_structural)
{
	area_structural->private = g_new0(CongEditorAreaStructuralDetails,1);
}


/* Exported function definitions: */
/**
 * cong_editor_area_structural_construct:
 * @area_structural:
 * @editor_widget:
 * @pixbuf:
 * @text:
 * @col_bold:
 * @col_dim:
 * @col_background:
 * @col_text:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_structural_construct (CongEditorAreaStructural *area_structural,
				       CongEditorWidget3 *editor_widget,
				       GdkPixbuf *pixbuf,
				       const gchar *text,
				       const GdkColor *col_bold,
				       const GdkColor *col_dim,
				       const GdkColor *col_background,
				       const GdkColor *col_text)
{
	int i;

	g_return_val_if_fail (text, NULL);
	g_return_val_if_fail (col_bold, NULL);
	g_return_val_if_fail (col_dim, NULL);
	g_return_val_if_fail (col_background, NULL);
	g_return_val_if_fail (col_text, NULL);

	cong_editor_area_bin_construct (CONG_EDITOR_AREA_BIN(area_structural),
					editor_widget);

	PRIVATE(area_structural)->col_array[CONG_DISPSPEC_GC_USAGE_BOLD_LINE] = gdk_color_copy (col_bold);
	PRIVATE(area_structural)->col_array[CONG_DISPSPEC_GC_USAGE_DIM_LINE] = gdk_color_copy (col_dim);
	PRIVATE(area_structural)->col_array[CONG_DISPSPEC_GC_USAGE_BACKGROUND] = gdk_color_copy (col_background);
	PRIVATE(area_structural)->col_array[CONG_DISPSPEC_GC_USAGE_TEXT] = gdk_color_copy (col_text);

	for (i=0;i<CONG_DISPSPEC_GC_USAGE_NUM; i++) {
		PRIVATE(area_structural)->gc_array[i] = generate_gc_for_col (PRIVATE(area_structural)->col_array[i]);
	}


	PRIVATE(area_structural)->title_vcompose = cong_editor_area_composer_new (editor_widget,
										  GTK_ORIENTATION_VERTICAL,
										  0);

	/* Build the title bar: */
	{

		/* Add a v-spacer: */
		cong_editor_area_composer_pack_end (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_structural)->title_vcompose),
						    cong_editor_area_spacer_new (editor_widget,
										 GTK_ORIENTATION_VERTICAL,
										 V_SPACING),
						    FALSE,
						    FALSE,
						    0);
		
		/* Add the h-composer: */
		PRIVATE(area_structural)->title_hcompose = cong_editor_area_composer_new (editor_widget,
											      GTK_ORIENTATION_HORIZONTAL,
											      H_INDENT);
		cong_editor_area_composer_pack_end (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_structural)->title_vcompose),
						    PRIVATE(area_structural)->title_hcompose,
						    FALSE,
						    FALSE,
						    0);

		
		/* Add a v-spacer: */
		cong_editor_area_composer_pack_end (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_structural)->title_vcompose),
						    cong_editor_area_spacer_new (editor_widget,
										 GTK_ORIENTATION_VERTICAL,
										 V_SPACING),
						    FALSE,
						    FALSE,
						    0);
		
		/* Build up the content of the h-composer: */
		{
			/* Add a h-spacer: */
			cong_editor_area_composer_pack_end (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_structural)->title_hcompose),
							    cong_editor_area_spacer_new (editor_widget,
											 GTK_ORIENTATION_HORIZONTAL,
											 H_INDENT),
							    FALSE,
							    FALSE,
							    0);
			
			/* Add the expander: */
			PRIVATE(area_structural)->title_expander = cong_editor_area_expander_new (editor_widget,
												  TRUE);
			cong_editor_area_composer_pack_end (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_structural)->title_hcompose),
							    PRIVATE(area_structural)->title_expander,
							    FALSE,
							    FALSE,
							    0);		
			
			/* Add the pixbuf (if any): */
			if (pixbuf) {
				PRIVATE(area_structural)->title_pixbuf = cong_editor_area_pixbuf_new (editor_widget,
													  pixbuf);
				cong_editor_area_composer_pack_end (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_structural)->title_hcompose),
								    PRIVATE(area_structural)->title_pixbuf,
								    FALSE,
								    FALSE,
								    0);		
			}
			
			/* Add the title text: */
			PRIVATE(area_structural)->title_text = cong_editor_area_text_new (editor_widget,
											  cong_app_get_font (cong_app_singleton(),
													     CONG_FONT_ROLE_TITLE_TEXT),
											  PRIVATE(area_structural)->col_array[CONG_DISPSPEC_GC_USAGE_TEXT],
											  text,
											  FALSE);
			cong_editor_area_composer_pack_end (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_structural)->title_hcompose),
							    PRIVATE(area_structural)->title_text,
							    FALSE,
							    FALSE,
							    0);		
		}
	}

	PRIVATE(area_structural)->inner_bin = cong_editor_area_bin_new (editor_widget);

	cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (area_structural),
								   PRIVATE(area_structural)->title_vcompose);
	cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (area_structural),
								   PRIVATE(area_structural)->inner_bin);

	cong_editor_area_protected_set_parent (PRIVATE(area_structural)->title_vcompose,
					       CONG_EDITOR_AREA (area_structural));
	cong_editor_area_protected_set_parent (PRIVATE(area_structural)->inner_bin,
					       CONG_EDITOR_AREA (area_structural));

	g_signal_connect (G_OBJECT(PRIVATE(area_structural)->title_expander),
			  "expansion_changed",
			  G_CALLBACK(on_expansion_changed),
			  area_structural);



	return CONG_EDITOR_AREA (area_structural);
}

/**
 * cong_editor_area_structural_new:
 * @editor_widget:
 * @pixbuf:
 * @text:
 * @col_bold:
 * @col_dim:
 * @col_background:
 * @col_text:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_structural_new (CongEditorWidget3 *editor_widget,
				 GdkPixbuf *pixbuf,
				 const gchar *text,
				 const GdkColor *col_bold,
				 const GdkColor *col_dim,
				 const GdkColor *col_background,
				 const GdkColor *col_text)
{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_structural_new(%s)", text);
#endif

	g_return_val_if_fail (editor_widget, NULL);
	g_return_val_if_fail (text, NULL);

	return cong_editor_area_structural_construct
		(g_object_new (CONG_EDITOR_AREA_STRUCTURAL_TYPE, NULL),
		 editor_widget,
		 pixbuf,
		 text,
		 col_bold,
		 col_dim,
		 col_background,
		 col_text);
}

/* Method implementation definitions: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect)
{
	GdkGC *gc;
	CongEditorAreaStructural *area_structural = CONG_EDITOR_AREA_STRUCTURAL(area);
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);
#if 0
	const GtkRequisition *title_req;
#endif
	gint title_bar_height;

	gboolean expanded = TRUE;

#if 1
	title_bar_height = cong_editor_area_get_cached_requisition (PRIVATE(area_structural)->title_vcompose,
								    GTK_ORIENTATION_VERTICAL);
#else
	title_req = cong_editor_area_get_cached_requisition (PRIVATE(area_structural)->title_vcompose);
	g_assert(title_req);

	title_bar_height = title_req->height;
#endif

	gc = PRIVATE(area_structural)->gc_array[CONG_DISPSPEC_GC_USAGE_BOLD_LINE];
	g_assert(gc);

	/* Draw the frame rectangle "open" on the right-hand side : */
	/* Top */
	gdk_draw_line (window, 
		       gc, 
		       rect->x, rect->y, 
		       rect->x + rect->width, rect->y);

	/* Left */
	gdk_draw_line (window, 
		       gc, 
		       rect->x, rect->y,
		       rect->x, rect->y + (expanded ? rect->height-1 : title_bar_height));

	/* Fill the inside of the rectangle: */
	gc = PRIVATE(area_structural)->gc_array[CONG_DISPSPEC_GC_USAGE_BACKGROUND];
	g_assert(gc);
	
	gdk_draw_rectangle (window, 
			    gc, 
			    TRUE, 
			    rect->x+1, rect->y+1, 
			    rect->width - 1, title_bar_height);
	
	/* Bottom */  
	if (cong_editor_area_expander_get_state (CONG_EDITOR_AREA_EXPANDER(PRIVATE (area_structural)->title_expander))) {
		gc = PRIVATE(area_structural)->gc_array[CONG_DISPSPEC_GC_USAGE_DIM_LINE];
		g_assert(gc);

		/* Bottom of title bar: */
		gdk_draw_line (window, 
			       gc, 
			       rect->x + 1, rect->y + title_bar_height+1,
			       rect->x + rect->width, rect->y + title_bar_height+1);

		/* Short horizontal line along very bottom of area: */
		cong_util_draw_blended_line (GTK_WIDGET(cong_editor_area_get_widget (area)),
					     PRIVATE(area_structural)->col_array[CONG_DISPSPEC_GC_USAGE_BOLD_LINE],
					     rect->x, rect->y + rect->height-1,
					     rect->x + 45);

	} else {
		gc = PRIVATE(area_structural)->gc_array[CONG_DISPSPEC_GC_USAGE_BOLD_LINE];
		g_assert(gc);

		/* Bottom of title bar: */
		gdk_draw_line (window, 
			       gc, 
			       rect->x + 1, rect->y + title_bar_height+1,
			       rect->x + rect->width, rect->y + title_bar_height+1);
	}

	cong_editor_area_debug_render_state (area);
}

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	if (orientation==GTK_ORIENTATION_HORIZONTAL) {
		return width_hint;
	} else {
		gint title_req;
		
		CongEditorAreaStructural *structural = CONG_EDITOR_AREA_STRUCTURAL(area);

		title_req = cong_editor_area_get_requisition (PRIVATE(structural)->title_vcompose,
							      orientation,
							      width_hint-1);

		if (cong_editor_area_expander_get_state (CONG_EDITOR_AREA_EXPANDER (PRIVATE(structural)->title_expander))) {
			gint inner_req;
		
			inner_req = cong_editor_area_get_requisition (PRIVATE(structural)->inner_bin,
								      orientation,
								      width_hint-1);
			return title_req + inner_req+3;	
		} else {
			return title_req + 2;	
		}
	}
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaStructural *structural = CONG_EDITOR_AREA_STRUCTURAL(area);
	const GdkRectangle *rect = cong_editor_area_get_window_coords(area);
	guint title_req_height;

#if 0
	PRIVATE(structural)->title_vcompose;
	PRIVATE(structural)->inner_bin;
#endif

	title_req_height = cong_editor_area_get_cached_requisition (PRIVATE(structural)->title_vcompose,
								    GTK_ORIENTATION_VERTICAL);

	cong_editor_area_set_allocation (PRIVATE(structural)->title_vcompose,
					 rect->x+1,
					 rect->y+1,
					 rect->width-1,
					 title_req_height);

	cong_editor_area_set_allocation (PRIVATE(structural)->inner_bin,
					 rect->x+1,
					 rect->y+2+title_req_height,
					 rect->width-1,
					 rect->height-(3+title_req_height));
}

static CongEditorArea*
for_all (CongEditorArea *editor_area, 
	 CongEditorAreaCallbackFunc func, 
	 gpointer user_data)
{
	CongEditorAreaStructural *structural = CONG_EDITOR_AREA_STRUCTURAL(editor_area);

	if ((*func)(PRIVATE(structural)->title_vcompose, user_data)) {
		return PRIVATE(structural)->title_vcompose;
	}

	if ((*func)(PRIVATE(structural)->inner_bin, user_data)) {
		return PRIVATE(structural)->inner_bin;
	}

	return NULL;
}

static void
add_child (CongEditorAreaContainer *area_container,
	   CongEditorArea *child)
{
	CongEditorAreaStructural *structural = CONG_EDITOR_AREA_STRUCTURAL(area_container);

	g_assert(PRIVATE(structural)->inner_bin);

	cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER( PRIVATE(structural)->inner_bin),
					       child);
}

static void
on_expansion_changed (CongEditorAreaExpander *area_expander,
		      gpointer user_data)
{
	CongEditorAreaStructural *area_structural = CONG_EDITOR_AREA_STRUCTURAL (user_data);

	if (cong_editor_area_expander_get_state (CONG_EDITOR_AREA_EXPANDER(PRIVATE(area_structural)->title_expander))) {
		cong_editor_area_show (PRIVATE(area_structural)->inner_bin);
	} else {
		cong_editor_area_hide (PRIVATE(area_structural)->inner_bin);
	}
	
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_structural),
						  GTK_ORIENTATION_VERTICAL );
}

static void 
state_changed (CongEditorArea *area)
{
	CongEditorAreaStructural *area_structural = CONG_EDITOR_AREA_STRUCTURAL (area);
	GtkStateType new_state = cong_editor_area_get_state (area);	

	/* Transmit state to pixbuf: */
	if (PRIVATE(area_structural)->title_pixbuf) {
		cong_editor_area_set_state (PRIVATE(area_structural)->title_pixbuf,
					    new_state);
	}
}
