/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-unknown-tag.c
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
#include "cong-editor-area-unknown-tag.h"
#include <libgnome/gnome-macros.h>

#include "cong-app.h"
#include "cong-editor-area-text.h"
#include "cong-editor-area-composer.h"
#include "cong-editor-area-spacer.h"
#include "cong-editor-area-expander.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorAreaUnknownTagDetails
{
	CongNodePtr xml_node;

	gulong handler_id_set_attribute;
	gulong handler_id_remove_attribute;

	CongEditorArea *outer_vcompose;
	CongEditorArea *top_row;
		
	/**/ CongEditorArea *namespace_decl_vcompose;
	/**/ CongEditorArea *attribute_vcompose;
	/**/ CongEditorArea *expander_row;
	/****/ CongEditorArea *inner_expander;

	/**/ CongEditorArea *inner_row;
 	/****/ CongEditorArea *inner_area;
};

static void
dispose (GObject *object);

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

static void
refresh_attribute_areas (CongEditorAreaUnknownTag *area_unknown_tag);

static void
add_areas_for_nsdef (CongEditorAreaUnknownTag *area_unknown_tag, 
		     CongEditorWidget3 *editor_widget,
		     xmlNsPtr ns);
static void
add_areas_for_attribute (CongEditorAreaUnknownTag *area_unknown_tag, 
			 CongEditorWidget3 *editor_widget,
			 xmlAttrPtr attr);

static void
on_node_set_attribute (CongDocument *doc,
		       CongNodePtr node, 
		       xmlNs *ns_ptr, 
		       const xmlChar *name, 
		       const xmlChar *value,
		       CongEditorAreaUnknownTag *area_unknown_tag);

static void
on_node_remove_attribute (CongDocument *doc,
			  CongNodePtr node, 
			  xmlNs *ns_ptr, 
			  const xmlChar *name, 
			  CongEditorAreaUnknownTag *area_unknown_tag);

static void
on_expansion_changed (CongEditorAreaExpander *area_expander,
		      gpointer user_data);

/* GObject boilerplate stuff: */
GNOME_CLASS_BOILERPLATE(CongEditorAreaUnknownTag, 
			cong_editor_area_unknown_tag,
			CongEditorAreaBin,
			CONG_EDITOR_AREA_BIN_TYPE );

static void
cong_editor_area_unknown_tag_class_init (CongEditorAreaUnknownTagClass *klass)
{
	CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass);
	CongEditorAreaContainerClass *container_klass = CONG_EDITOR_AREA_CONTAINER_CLASS(klass);

	G_OBJECT_CLASS (klass)->dispose = dispose;

	area_klass->render_self = render_self;
	area_klass->calc_requisition = calc_requisition;
	area_klass->allocate_child_space = allocate_child_space;
	area_klass->for_all = for_all;

	container_klass->add_child = add_child;

}

static void
cong_editor_area_unknown_tag_instance_init (CongEditorAreaUnknownTag *area_unknown_tag)
{
	area_unknown_tag->private = g_new0(CongEditorAreaUnknownTagDetails,1);
}

static void
dispose (GObject *object)
{
	CongEditorAreaUnknownTag *area_unknown_tag = CONG_EDITOR_AREA_UNKNOWN_TAG (object);
	CongDocument *doc = cong_editor_area_get_document (CONG_EDITOR_AREA (area_unknown_tag));

	g_signal_handler_disconnect (G_OBJECT(doc),
				     PRIVATE(area_unknown_tag)->handler_id_set_attribute);	
	g_signal_handler_disconnect (G_OBJECT(doc),
				     PRIVATE(area_unknown_tag)->handler_id_remove_attribute);	

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static const gchar*
cong_ui_get_colour_string(CongNodeType type)
{
	/* FIXME: this should be linked to the theme and/or the GtkSourceView settings */

	switch (type) {
	default: g_assert_not_reached();
	case CONG_NODE_TYPE_UNKNOWN:
		return "#000000";
	case CONG_NODE_TYPE_ELEMENT:
		return "#0080ff";
	case CONG_NODE_TYPE_ATTRIBUTE:
		return "#000000";
	case CONG_NODE_TYPE_TEXT:
		return "#ff0000";
	case CONG_NODE_TYPE_CDATA_SECTION:
		return "#000000";
	case CONG_NODE_TYPE_ENTITY_REF:
		return "#000000";
	case CONG_NODE_TYPE_ENTITY_NODE:
		return "#000000";
	case CONG_NODE_TYPE_PI:
		return "#000000";
	case CONG_NODE_TYPE_COMMENT:
		return "#0000FF";
	case CONG_NODE_TYPE_DOCUMENT:
		return "#0080ff";
	case CONG_NODE_TYPE_DOCUMENT_TYPE:
		return "#000000";
	case CONG_NODE_TYPE_DOCUMENT_FRAG:
		return "#000000";
	case CONG_NODE_TYPE_NOTATION:
		return "#000000";
	case CONG_NODE_TYPE_HTML_DOCUMENT:
		return "#000000";
	case CONG_NODE_TYPE_DTD:
		return "#0000FF";
	case CONG_NODE_TYPE_ELEMENT_DECL:
		return "#000000";
	case CONG_NODE_TYPE_ATRRIBUTE_DECL:
		return "#000000";
	case CONG_NODE_TYPE_ENTITY_DECL:
		return "#000000";
	case CONG_NODE_TYPE_NAMESPACE_DECL:
		return "#000000";
	case CONG_NODE_TYPE_XINCLUDE_START:
		return "#000000";
	case CONG_NODE_TYPE_XINCLUDE_END:
		return "#000000";
	}
}


/* Exported function definitions: */
/**
 * cong_editor_area_unknown_tag_construct:
 * @area_unknown_tag:
 * @editor_widget:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_unknown_tag_construct (CongEditorAreaUnknownTag *area_unknown_tag,
					CongEditorWidget3 *editor_widget,
					CongNodePtr node)
{
	gchar *tag_string;
	gchar *start_tag_string_begin;
	gchar *start_tag_string_end;
	gchar *end_tag_string;

	CongDocument *doc = cong_editor_widget3_get_document (editor_widget);

	const gchar *colour_string_element = cong_ui_get_colour_string(CONG_NODE_TYPE_ELEMENT);

	cong_editor_area_bin_construct (CONG_EDITOR_AREA_BIN(area_unknown_tag),
					editor_widget);

	PRIVATE(area_unknown_tag)->xml_node = node;

	if ((node->ns != NULL) && (node->ns->prefix != NULL)) {
		tag_string = g_strdup_printf ("%s:%s", node->ns->prefix, node->name);
	} else {
		tag_string = g_strdup (node->name);
	}

	start_tag_string_begin = g_strdup_printf("<span foreground=\"%s\">&lt;%s</span>",colour_string_element, tag_string);
	start_tag_string_end = g_strdup_printf("<span foreground=\"%s\">&gt;</span>",colour_string_element);
	end_tag_string = g_strdup_printf("<span foreground=\"%s\">&lt;/%s&gt;</span>",colour_string_element, tag_string);

	g_free (tag_string);

	PRIVATE(area_unknown_tag)->outer_vcompose = cong_editor_area_composer_new (editor_widget,
										   GTK_ORIENTATION_VERTICAL,
										   0);


	/* Set up top row: */
	{
		PRIVATE(area_unknown_tag)->top_row = cong_editor_area_composer_new (editor_widget,
										    GTK_ORIENTATION_HORIZONTAL,
										    0);
		
		cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER(PRIVATE(area_unknown_tag)->outer_vcompose),
						       PRIVATE(area_unknown_tag)->top_row, TRUE);
		
		/* Opening "<ns-prefix:element-name": */
		cong_editor_area_composer_pack_end ( CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_unknown_tag)->top_row),
						     cong_editor_area_text_new (editor_widget,
										cong_app_get_font (cong_app_singleton(),
												   CONG_FONT_ROLE_TITLE_TEXT),
										NULL,
										start_tag_string_begin,
										TRUE),
						     FALSE,
						     FALSE,
						     0);
		
		/* Namespace declarations: */
		{
			PRIVATE(area_unknown_tag)->namespace_decl_vcompose = cong_editor_area_composer_new (editor_widget,
													    GTK_ORIENTATION_VERTICAL,
													    0);
			cong_editor_area_composer_pack_end ( CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_unknown_tag)->top_row),
							     PRIVATE(area_unknown_tag)->namespace_decl_vcompose,
							     FALSE,
							     FALSE,
							     0);

			/* FIXME: connect to signal to handle updates */
			{
				xmlNsPtr iter;
				for (iter = node->nsDef; iter; iter = iter->next) {
					add_areas_for_nsdef (area_unknown_tag,
							     editor_widget,
							     iter);
				}
			}
		}


		/* Attributes: */
		{
			PRIVATE(area_unknown_tag)->attribute_vcompose = cong_editor_area_composer_new (editor_widget,
												       GTK_ORIENTATION_VERTICAL,
												       0);
			cong_editor_area_composer_pack_end ( CONG_EDITOR_AREA_COMPOSER (PRIVATE(area_unknown_tag)->top_row),
							     PRIVATE(area_unknown_tag)->attribute_vcompose,
							     FALSE,
							     FALSE,
							     0);
			refresh_attribute_areas (area_unknown_tag);
		}
		
		/* Closing ">" */
		cong_editor_area_composer_pack_end ( CONG_EDITOR_AREA_COMPOSER (PRIVATE(area_unknown_tag)->top_row),
						     cong_editor_area_text_new (editor_widget,
										cong_app_get_font (cong_app_singleton(),
												   CONG_FONT_ROLE_TITLE_TEXT),
										NULL,
										start_tag_string_end,
										TRUE),
						     FALSE,
						     FALSE,
						     0);
		/* FIXME: add expander to control display of child nodes */

	}

	
	PRIVATE(area_unknown_tag)->expander_row = cong_editor_area_composer_new (editor_widget,
						   GTK_ORIENTATION_HORIZONTAL,
						   0);
	cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER(PRIVATE(area_unknown_tag)->outer_vcompose),
					       PRIVATE(area_unknown_tag)->expander_row, TRUE);	

	PRIVATE(area_unknown_tag)->inner_expander = cong_editor_area_expander_new (editor_widget, TRUE);

	cong_editor_area_composer_pack_end (CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_unknown_tag)->expander_row),
					    PRIVATE(area_unknown_tag)->inner_expander,
					    FALSE,
					    FALSE,
					    0);		
	
	/* Inner container for children */				       
	{
		PRIVATE(area_unknown_tag)->inner_row = cong_editor_area_composer_new (editor_widget,
							   GTK_ORIENTATION_HORIZONTAL,
							   0);
	
		cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER(PRIVATE(area_unknown_tag)->outer_vcompose),
						       PRIVATE(area_unknown_tag)->inner_row, TRUE);	

		cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER(PRIVATE(area_unknown_tag)->inner_row),
						       cong_editor_area_spacer_new (editor_widget,
										    GTK_ORIENTATION_HORIZONTAL,
										    50), TRUE);

		PRIVATE(area_unknown_tag)->inner_area = cong_editor_area_bin_new (editor_widget);
		cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER(PRIVATE(area_unknown_tag)->inner_row),
						       PRIVATE(area_unknown_tag)->inner_area, TRUE);	

	}
	
	cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER(PRIVATE(area_unknown_tag)->outer_vcompose),
					       cong_editor_area_text_new (editor_widget,
									  cong_app_get_font (cong_app_singleton(),
											     CONG_FONT_ROLE_TITLE_TEXT), 
									  NULL,
									  end_tag_string,
									  TRUE),
					       TRUE);

	g_free (start_tag_string_begin);
	g_free (start_tag_string_end);
	g_free (end_tag_string);

	cong_editor_area_protected_postprocess_add_internal_child (CONG_EDITOR_AREA (area_unknown_tag),
								   PRIVATE(area_unknown_tag)->outer_vcompose);

	cong_editor_area_protected_set_parent (PRIVATE(area_unknown_tag)->outer_vcompose,
					       CONG_EDITOR_AREA (area_unknown_tag));

	/* FIXME: remove these handlers in dispose method */
	PRIVATE(area_unknown_tag)->handler_id_set_attribute = g_signal_connect_after (G_OBJECT (doc),
										      "node_set_attribute",
										      G_CALLBACK (on_node_set_attribute),
										      area_unknown_tag);
	PRIVATE(area_unknown_tag)->handler_id_remove_attribute = g_signal_connect_after (G_OBJECT (doc),
											 "node_remove_attribute",
											 G_CALLBACK (on_node_remove_attribute),
											 area_unknown_tag);

	g_signal_connect (G_OBJECT(PRIVATE(area_unknown_tag)->inner_expander),
			  "expansion_changed",
			  G_CALLBACK(on_expansion_changed),
			  area_unknown_tag);

	return CONG_EDITOR_AREA (area_unknown_tag);}

/**
 * cong_editor_area_unknown_tag_new:
 * @editor_widget:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongEditorArea*
cong_editor_area_unknown_tag_new (CongEditorWidget3 *editor_widget,
				  CongNodePtr node)

{
#if DEBUG_EDITOR_AREA_LIFETIMES
	g_message("cong_editor_area_unknown_tag_new(%s)", tagname);
#endif

	return cong_editor_area_unknown_tag_construct
		(g_object_new (CONG_EDITOR_AREA_UNKNOWN_TAG_TYPE, NULL),
		 editor_widget,
		 node);
}

/* Method implementation definitions: */
static void 
render_self (CongEditorArea *area,
	     const GdkRectangle *widget_rect)
{
	cong_editor_area_debug_render_state (area);
}

static gint
calc_requisition (CongEditorArea *area, 
		  GtkOrientation orientation,
		  int width_hint)
{
	CongEditorAreaUnknownTag *unknown_tag = CONG_EDITOR_AREA_UNKNOWN_TAG(area);

	if (PRIVATE(unknown_tag)->outer_vcompose) {

		return cong_editor_area_get_requisition (PRIVATE(unknown_tag)->outer_vcompose,
							 orientation,
							 width_hint);
	} else {
		return 0;
	}
}

static void
allocate_child_space (CongEditorArea *area)
{
	CongEditorAreaUnknownTag *unknown_tag = CONG_EDITOR_AREA_UNKNOWN_TAG(area);

	if (PRIVATE(unknown_tag)->outer_vcompose) {
		const GdkRectangle *rect = cong_editor_area_get_window_coords(area);

		cong_editor_area_set_allocation (PRIVATE(unknown_tag)->outer_vcompose,
						 rect->x,
						 rect->y,
						 rect->width,
						 rect->height);
	}

}

static CongEditorArea*
for_all (CongEditorArea *editor_area, 
	 CongEditorAreaCallbackFunc func, 
	 gpointer user_data)
{
	CongEditorAreaUnknownTag *unknown_tag = CONG_EDITOR_AREA_UNKNOWN_TAG(editor_area);

	if (PRIVATE(unknown_tag)->outer_vcompose) {
		if ((*func)(PRIVATE(unknown_tag)->outer_vcompose, user_data)) {
			return PRIVATE(unknown_tag)->outer_vcompose;
		}
	}

	return NULL;
}

static void
add_child (CongEditorAreaContainer *area_container,
	   CongEditorArea *child,
	   gboolean add_to_end)
{
	CongEditorAreaUnknownTag *unknown_tag = CONG_EDITOR_AREA_UNKNOWN_TAG(area_container);

	g_assert(PRIVATE(unknown_tag)->inner_area);

	cong_editor_area_container_add_child ( CONG_EDITOR_AREA_CONTAINER( PRIVATE(unknown_tag)->inner_area),
					       child,
					       add_to_end);
}

void
add_areas_for_nsdef (CongEditorAreaUnknownTag *area_unknown_tag, 
		     CongEditorWidget3 *editor_widget,
		     xmlNsPtr ns_ptr)
{
	gchar *ns_string;

	if (1/* (ns_ptr->type == XML_LOCAL_NAMESPACE) && (ns_ptr->href != NULL) */) {

		const gchar *colour_string_ns = cong_ui_get_colour_string(CONG_NODE_TYPE_NAMESPACE_DECL);	

		/* if (xmlStrEqual(ns_ptr->prefix, BAD_CAST "xml")) */
/* 			return; */

		/* Within the context of an element attributes */
		if (ns_ptr->prefix != NULL) {
			ns_string = g_strdup_printf (" <span foreground=\"%s\">xmlns:%s=\"%s\"</span>",colour_string_ns, ns_ptr->prefix, ns_ptr->href);
		} else {
			ns_string = g_strdup_printf (" <span foreground=\"%s\">xmlns=\"%s\"</span>",colour_string_ns, ns_ptr->href);
		}
		cong_editor_area_composer_pack_end ( CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_unknown_tag)->namespace_decl_vcompose),
						     cong_editor_area_text_new (editor_widget,
										cong_app_get_font (cong_app_singleton(),
												   CONG_FONT_ROLE_BODY_TEXT),
										NULL,
										ns_string,
										TRUE),
						     FALSE,
						     FALSE,
						     0);					

		g_free (ns_string);
	}

}

static void
add_areas_for_attribute (CongEditorAreaUnknownTag *area_unknown_tag, 
			 CongEditorWidget3 *editor_widget,
			 xmlAttrPtr attr)
{
	gchar *attribute_name;
	gchar *attr_string;

	const gchar *colour_string_attribute = cong_ui_get_colour_string(CONG_NODE_TYPE_ATTRIBUTE);

	if ((attr->ns != NULL) && (attr->ns->prefix != NULL)) {
		attribute_name = g_strdup_printf ("%s:%s", attr->ns->prefix, attr->name);
	} else {
		attribute_name = g_strdup (attr->name);
	}
					
	attr_string = g_strdup_printf (" <span foreground=\"%s\">%s=\"%s\"</span>",colour_string_attribute, attribute_name, attr->children->content);

	cong_editor_area_composer_pack_end ( CONG_EDITOR_AREA_COMPOSER(PRIVATE(area_unknown_tag)->attribute_vcompose),
					     cong_editor_area_text_new (editor_widget,
									cong_app_get_font (cong_app_singleton(),
											   CONG_FONT_ROLE_BODY_TEXT),
									NULL,
									attr_string,
									TRUE),
					     FALSE,
					     FALSE,
					     0);

	g_free (attribute_name);
	g_free (attr_string);					
}

static void
on_node_set_attribute (CongDocument *doc,
		       CongNodePtr node, 
		       xmlNs *ns_ptr, 
		       const xmlChar *name, 
		       const xmlChar *value,
		       CongEditorAreaUnknownTag *area_unknown_tag)
{
	if (node == PRIVATE(area_unknown_tag)->xml_node) {
		refresh_attribute_areas (area_unknown_tag);
	}
}

static void
on_node_remove_attribute (CongDocument *doc,
			  CongNodePtr node, 
			  xmlNs *ns_ptr, 
			  const xmlChar *name, 
			  CongEditorAreaUnknownTag *area_unknown_tag)
{
	if (node == PRIVATE(area_unknown_tag)->xml_node) {
		refresh_attribute_areas (area_unknown_tag);
	}
}

static void
refresh_attribute_areas (CongEditorAreaUnknownTag *area_unknown_tag)
{
	xmlAttrPtr iter;

	/* Remove old attribute areas: */
	cong_editor_area_remove_all_children ( CONG_EDITOR_AREA_CONTAINER(PRIVATE(area_unknown_tag)->attribute_vcompose));

	/* Add new attribute areas: */
	for (iter = PRIVATE (area_unknown_tag)->xml_node->properties; iter; iter = iter->next) {
		add_areas_for_attribute (area_unknown_tag,
					 cong_editor_area_get_widget (CONG_EDITOR_AREA (area_unknown_tag)),
					 iter);
	}
}


static void
on_expansion_changed (CongEditorAreaExpander *area_expander,
		      gpointer user_data)
{
	CongEditorAreaUnknownTag *area_unknown_tag = CONG_EDITOR_AREA_UNKNOWN_TAG (user_data);

	if (cong_editor_area_expander_get_state (CONG_EDITOR_AREA_EXPANDER(PRIVATE(area_unknown_tag)->inner_expander))) {
		cong_editor_area_show (PRIVATE(area_unknown_tag)->inner_area);
	} else {
		cong_editor_area_hide (PRIVATE(area_unknown_tag)->inner_area);
	}
	
	cong_editor_area_flush_requisition_cache (CONG_EDITOR_AREA(area_unknown_tag), GTK_ORIENTATION_VERTICAL);
}
