/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-entity-decl.c
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
#include "cong-editor-area-entity-decl.h"
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

struct CongEditorAreaEntityDeclDetails
{
	CongEditorArea *outer_hcompose;
	/****/ CongEditorArea *title_text;
	/****/ CongEditorArea *inner_bin;
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
	   CongEditorArea *child,
	   gboolean add_to_end);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaEntityDecl, 
			cong_editor_area_entity_decl,
			CongEditorAreaBin,
			CONG_EDITOR_AREA_BIN_TYPE );

static void
cong_editor_area_entity_decl_class_init (CongEditorAreaEntityDeclClass *klass)
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
cong_editor_area_entity_decl_instance_init (CongEditorAreaEntityDecl *area_entity_decl)
{
	area_entity_decl->private = g_new0(CongEditorAreaEntityDeclDetails,1);
}


/* Exported function definitions: */
/**
 * cong_editor_area_entity_decl_construct:
 * @area_entity_decl:
 * @editor_widget:
 * @text:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_entity_decl_construct (CongEditorAreaEntityDecl *area_entity_decl,
					   CongEditorWidget3 *editor_widget,
					   const gchar *text)
{
	gchar *markup;

	g_return_val_if_fail (text, NULL);

	cong_editor_area_bin_construct (CONG_EDITOR_AREA_BIN(area_entity_decl),
					editor_widget);

	PRIVATE(area_entity_decl)->outer_hcompose = cong_editor_area_composer_new (editor_widget,
										   GTK_ORIENTATION_HORIZONTAL,
										   0);
	/* Add the title text: */
	markup = g_strdup_printf ("\"<span>%s</span>\" =", text);

	PRIVATE(area_entity_decl)->title_text = cong_editor_area_text_new (editor_widget,
									   cong_app_get_font (cong_app_singleton(),
											      CONG_FONT_ROLE_TITLE_TEXT), 
									   NULL,
									   markup,
									   TRUE);
	g_free(markup);

	cong_editor_area_composer_pack_end (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_entity_decl)->outer_hcompose),
					    PRIVATE(area_entity_decl)->title_text,
					    FALSE,
					    FALSE,
					    0);


	PRIVATE(area_entity_decl)->inner_bin = cong_editor_area_bin_new (editor_widget);

	cong_editor_area_composer_pack_end (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_entity_decl)->outer_hcompose),
					    PRIVATE(area_entity_decl)->inner_bin,
					    TRUE,
					    TRUE,
					    0);


	cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (area_entity_decl),
								   PRIVATE(area_entity_decl)->outer_hcompose);

	cong_editor_area_protected_set_parent (PRIVATE(area_entity_decl)->outer_hcompose,
					       CONG_EDITOR_AREA (area_entity_decl));

	return CONG_EDITOR_AREA (area_entity_decl);
}

/**
 * cong_editor_area_entity_decl_new:
 * @editor_widget:
 * @text:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_entity_decl_new (CongEditorWidget3 *editor_widget,
				  const gchar *text)

{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_entity_decl_new(%s)", text);
#endif

	g_return_val_if_fail (editor_widget, NULL);
	g_return_val_if_fail (text, NULL);

	return cong_editor_area_entity_decl_construct
		(g_object_new (CONG_EDITOR_AREA_ENTITY_DECL_TYPE, NULL),
		 editor_widget,
		 text);
}

/* Method implementation definitions: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect)
{
#if 0
	GdkGC *gc;
	CongEditorAreaEntityDecl *area_entity_decl = CONG_EDITOR_AREA_ENTITY_DECL(area);
	CongDispspecElement *ds_element = PRIVATE(area_entity_decl)->ds_element;
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);
#if 0
	const GtkRequisition *title_req;
#endif
	gint title_bar_height;

	gboolean expanded = TRUE;

#if 1
	title_bar_height = cong_editor_area_get_cached_requisition (PRIVATE(area_entity_decl)->title_vcompose,
								    GTK_ORIENTATION_VERTICAL);
#else
	title_req = cong_editor_area_get_cached_requisition (PRIVATE(area_entity_decl)->title_vcompose);
	g_assert(title_req);

	title_bar_height = title_req->height;
#endif

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
		cong_util_draw_blended_line (GTK_WIDGET(cong_editor_area_get_widget (area)),
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
#endif
}

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	CongEditorAreaEntityDecl *entity_decl = CONG_EDITOR_AREA_ENTITY_DECL(area);

	return cong_editor_area_get_requisition (PRIVATE(entity_decl)->outer_hcompose,
						 orientation,
						 width_hint);
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaEntityDecl *entity_decl = CONG_EDITOR_AREA_ENTITY_DECL(area);
	const GdkRectangle *rect = cong_editor_area_get_window_coords(area);

	cong_editor_area_set_allocation (PRIVATE(entity_decl)->outer_hcompose,
					 rect->x,
					 rect->y,
					 rect->width,
					 rect->height);
}

static CongEditorArea*
for_all (CongEditorArea *editor_area, 
	 CongEditorAreaCallbackFunc func, 
	 gpointer user_data)
{
	CongEditorAreaEntityDecl *entity_decl = CONG_EDITOR_AREA_ENTITY_DECL(editor_area);

	if ((*func)(PRIVATE(entity_decl)->outer_hcompose, user_data)) {
		return PRIVATE(entity_decl)->outer_hcompose;
	}

	return NULL;
}

static void
add_child (CongEditorAreaContainer *area_container,
	   CongEditorArea *child,
	   gboolean add_to_end)
{
	CongEditorAreaEntityDecl *entity_decl = CONG_EDITOR_AREA_ENTITY_DECL(area_container);

	g_assert(PRIVATE(entity_decl)->inner_bin);

	cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER( PRIVATE(entity_decl)->inner_bin),
					       child,
					       add_to_end);
}
