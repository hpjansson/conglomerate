/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-structural-tag.c
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
#include "cong-editor-area-structural-tag.h"
#include <libgnome/gnome-macros.h>

#include "cong-app.h"
#include "cong-editor-area-text.h"
#include "cong-editor-area-composer.h"
#include "cong-editor-area-spacer.h"
#include "cong-editor-area-pixbuf.h"
#include "cong-dispspec.h"

#define PRIVATE(x) ((x)->private)

#define V_SPACING (4)
#define H_SPACING (4)
#define H_INDENT (4)
#define FRAGMENT_WIDTH (45)

struct CongEditorAreaStructuralTagDetails
{
	CongDispspecElement *ds_element;

	CongEditorArea *title_vcompose;
	/****/ /* anon v-spacer */
	/****/ CongEditorArea *title_hcompose;
	/********/ /* anon h-spacer */
	/********/ CongEditorArea *title_pixbuf;
	/********/ CongEditorArea *title_text;
	/****/ /* anon v-spacer */
	CongEditorArea *inner_bin;
};

/* Method implementation prototypes: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect);

static void 
calc_requisition (CongEditorArea *area, 
		  int width_hint,
		  GtkRequisition *output);

static void
allocate_child_space (CongEditorArea *area);

static CongEditorArea*
for_all (CongEditorArea *editor_area, 
	 CongEditorAreaCallbackFunc func, 
	 gpointer user_data);

static void
add_child (CongEditorAreaContainer *area_container,
	   CongEditorArea *child);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaStructuralTag, 
			cong_editor_area_structural_tag,
			CongEditorAreaBin,
			CONG_EDITOR_AREA_BIN_TYPE );

static void
cong_editor_area_structural_tag_class_init (CongEditorAreaStructuralTagClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);
	CongEditorAreaContainerClass *container_klass = CONG_EDITOR_AREA_CONTAINER_CLASS(klass);

	area_klass->render_self = render_self;
	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
	area_klass->for_all = for_all;

	container_klass->add_child = add_child;

}

static void
cong_editor_area_structural_tag_instance_init (CongEditorAreaStructuralTag *area_structural_tag)
{
	area_structural_tag->private = g_new0(CongEditorAreaStructuralTagDetails,1);
}


/* Exported function definitions: */
CongEditorArea*
cong_editor_area_structural_tag_construct (CongEditorAreaStructuralTag *area_structural_tag,
					   CongEditorWidget3 *editor_widget,
					   CongDispspecElement *ds_element,
					   GdkPixbuf *pixbuf,
					   const gchar *text)
{
	g_return_val_if_fail (text, NULL);

	cong_editor_area_bin_construct (CONG_EDITOR_AREA_BIN(area_structural_tag),
					editor_widget);

	PRIVATE(area_structural_tag)->ds_element = ds_element;

	PRIVATE(area_structural_tag)->title_vcompose = cong_editor_area_composer_new (editor_widget,
										      GTK_ORIENTATION_VERTICAL,
										      0);

	/* Build the title bar: */
	{

		/* Add a v-spacer: */
		cong_editor_area_composer_pack (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_structural_tag)->title_vcompose),
						cong_editor_area_spacer_new (editor_widget,
									     GTK_ORIENTATION_VERTICAL,
									     V_SPACING),
						FALSE,
						FALSE,
						0);
		
		/* Add the h-composer: */
		PRIVATE(area_structural_tag)->title_hcompose = cong_editor_area_composer_new (editor_widget,
											      GTK_ORIENTATION_HORIZONTAL,
											      H_INDENT);
		cong_editor_area_composer_pack (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_structural_tag)->title_vcompose),
						PRIVATE(area_structural_tag)->title_hcompose,
						FALSE,
						FALSE,
						0);

		
		/* Add a v-spacer: */
		cong_editor_area_composer_pack (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_structural_tag)->title_vcompose),
						cong_editor_area_spacer_new (editor_widget,
									     GTK_ORIENTATION_VERTICAL,
									     V_SPACING),
						FALSE,
						FALSE,
						0);
		
		/* Build up the content of the h-composer: */
		{
			/* Add a h-spacer: */
			cong_editor_area_composer_pack (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_structural_tag)->title_hcompose),
							cong_editor_area_spacer_new (editor_widget,
										     GTK_ORIENTATION_HORIZONTAL,
										     H_INDENT),
							FALSE,
							FALSE,
							0);
			
			/* Add the pixbuf (if any): */
			if (pixbuf) {
				PRIVATE(area_structural_tag)->title_pixbuf = cong_editor_area_pixbuf_new (editor_widget,
													  pixbuf);
				cong_editor_area_composer_pack (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_structural_tag)->title_hcompose),
								PRIVATE(area_structural_tag)->title_pixbuf,
								FALSE,
								FALSE,
								0);		
			}
			
			/* Add the title text: */
			PRIVATE(area_structural_tag)->title_text = cong_editor_area_text_new (editor_widget,
											      cong_app_singleton()->fonts[CONG_FONT_ROLE_TITLE_TEXT], 
											      cong_dispspec_element_col (ds_element, CONG_DISPSPEC_GC_USAGE_TEXT),
											      text);
			cong_editor_area_composer_pack (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_structural_tag)->title_hcompose),
							PRIVATE(area_structural_tag)->title_text,
							FALSE,
							FALSE,
							0);		
		}
	}

	PRIVATE(area_structural_tag)->inner_bin = cong_editor_area_bin_new (editor_widget);

	cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (area_structural_tag),
								   PRIVATE(area_structural_tag)->title_vcompose);
	cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (area_structural_tag),
								   PRIVATE(area_structural_tag)->inner_bin);

	cong_editor_area_protected_set_parent (PRIVATE(area_structural_tag)->title_vcompose,
					       CONG_EDITOR_AREA (area_structural_tag));
	cong_editor_area_protected_set_parent (PRIVATE(area_structural_tag)->inner_bin,
					       CONG_EDITOR_AREA (area_structural_tag));

	return CONG_EDITOR_AREA (area_structural_tag);
}

CongEditorArea*
cong_editor_area_structural_tag_new (CongEditorWidget3 *editor_widget,
				     CongDispspecElement *ds_element,
				     GdkPixbuf *pixbuf,
				     const gchar *text)

{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_structural_tag_new(%s)", text);
#endif

	g_return_val_if_fail (editor_widget, NULL);
	g_return_val_if_fail (text, NULL);

	return cong_editor_area_structural_tag_construct
		(g_object_new (CONG_EDITOR_AREA_STRUCTURAL_TAG_TYPE, NULL),
		 editor_widget,
		 ds_element,
		 pixbuf,
		 text);
}

/* Method implementation definitions: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect)
{
	GdkGC *gc;
	CongEditorAreaStructuralTag *area_structural_tag = CONG_EDITOR_AREA_STRUCTURAL_TAG(area);
	CongDispspecElement *ds_element = PRIVATE(area_structural_tag)->ds_element;
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);
	const GtkRequisition *title_req;
	gint title_bar_height;

	gboolean expanded = TRUE;

	title_req = cong_editor_area_get_cached_requisition (PRIVATE(area_structural_tag)->title_vcompose);
	g_assert(title_req);

	title_bar_height = title_req->height;

	gc = cong_dispspec_element_gc (ds_element,
				       CONG_DISPSPEC_GC_USAGE_BOLD_LINE);
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
	gc = cong_dispspec_element_gc(ds_element, 
				      CONG_DISPSPEC_GC_USAGE_BACKGROUND);
	g_assert(gc);
	
	gdk_draw_rectangle (window, 
			    gc, 
			    TRUE, 
			    rect->x+1, rect->y+1, 
			    rect->width - 1, title_bar_height);
	
	/* Bottom */  
	if (1/*section_head->expanded*/) {
		gc = cong_dispspec_element_gc (ds_element, 
					       CONG_DISPSPEC_GC_USAGE_DIM_LINE);
		g_assert(gc);

		/* Bottom of title bar: */
		gdk_draw_line (window, 
			       gc, 
			       rect->x + 1, rect->y + title_bar_height+1,
			       rect->x + rect->width, rect->y + title_bar_height+1);

		/* Short horizontal line along very bottom of area: */
		draw_blended_line (GTK_WIDGET(cong_editor_area_get_widget (area)),
				   cong_dispspec_element_col (ds_element, 
							      CONG_DISPSPEC_GC_USAGE_BOLD_LINE),
				   rect->x, rect->y + rect->height-1,
				   rect->x + 45);

	} else {
		gc = cong_dispspec_element_gc (ds_element, 
					       CONG_DISPSPEC_GC_USAGE_BOLD_LINE);
		g_assert(gc);

		/* Bottom of title bar: */
		gdk_draw_line (window, 
			       gc, 
			       rect->x + 1, rect->y + title_bar_height+1,
			       rect->x + rect->width, rect->y + title_bar_height+1);
	}
}

static void 
calc_requisition (CongEditorArea *area, 
		  int width_hint,
		  GtkRequisition *output)
{
#if 0
	gint width;
#endif
	const GtkRequisition *title_req;
	const GtkRequisition *inner_req;

	CongEditorAreaStructuralTag *structural_tag = CONG_EDITOR_AREA_STRUCTURAL_TAG(area);

	title_req = cong_editor_area_get_requisition (PRIVATE(structural_tag)->title_vcompose,
						      width_hint-1);
	g_assert(title_req);
	
	inner_req = cong_editor_area_get_requisition (PRIVATE(structural_tag)->inner_bin,
						      width_hint-1);
	g_assert(inner_req);

#if 0
	width = 0;

	if (width < title_req->width) {
		width = title_req->width;
	}
	if (width < inner_req->width) {
		width = inner_req->width;
	}
#endif

	output->width = width_hint;
	output->height = title_req->height + inner_req->height+3;
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaStructuralTag *structural_tag = CONG_EDITOR_AREA_STRUCTURAL_TAG(area);
	const GdkRectangle *rect = cong_editor_area_get_window_coords(area);
	const GtkRequisition *title_req;

	PRIVATE(structural_tag)->title_vcompose;
	PRIVATE(structural_tag)->inner_bin;

	title_req = cong_editor_area_get_cached_requisition (PRIVATE(structural_tag)->title_vcompose);
	g_assert(title_req);

	cong_editor_area_set_allocation (PRIVATE(structural_tag)->title_vcompose,
					 rect->x+1,
					 rect->y+1,
					 rect->width-1,
					 title_req->height);

	cong_editor_area_set_allocation (PRIVATE(structural_tag)->inner_bin,
					 rect->x+1,
					 rect->y+2+title_req->height,
					 rect->width-1,
					 rect->height-(3+title_req->height));
}

static CongEditorArea*
for_all (CongEditorArea *editor_area, 
	 CongEditorAreaCallbackFunc func, 
	 gpointer user_data)
{
	CongEditorAreaStructuralTag *structural_tag = CONG_EDITOR_AREA_STRUCTURAL_TAG(editor_area);

	if ((*func)(PRIVATE(structural_tag)->title_vcompose, user_data)) {
		return PRIVATE(structural_tag)->title_vcompose;
	}

	if ((*func)(PRIVATE(structural_tag)->inner_bin, user_data)) {
		return PRIVATE(structural_tag)->inner_bin;
	}

	return NULL;
}

static void
add_child (CongEditorAreaContainer *area_container,
	   CongEditorArea *child)
{
	CongEditorAreaStructuralTag *structural_tag = CONG_EDITOR_AREA_STRUCTURAL_TAG(area_container);

	g_assert(PRIVATE(structural_tag)->inner_bin);

	cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER( PRIVATE(structural_tag)->inner_bin),
					       child);
}
