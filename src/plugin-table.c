/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-table.c
 *
 * Plugin for table support
 * * Copyright (C) 2004 David Malcolm
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
#include "cong-plugin.h"

#include "cong-eel.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-font.h"

#include "cong-fake-plugin-hooks.h"
#include "cong-service-editor-node-factory.h"

#include "cong-editor-area.h"
#include "cong-editor-node.h"

#include "cong-editor-area-border.h"
#include "cong-ui-hooks.h"

#define CONG_EDITOR_AREA_DECLARE_SUBCLASS(SubclassName, subclass_name, BaseClassName, base_class_name) \
  typedef struct CongEditorArea##SubclassName CongEditorArea##SubclassName; \
  typedef struct CongEditorArea##SubclassName##Class CongEditorArea##SubclassName##Class; \
  typedef struct CongEditorArea##SubclassName##Private CongEditorArea##SubclassName##Private; \
  struct CongEditorArea##SubclassName \
  { \
	  BaseClassName area; \
	  CongEditorArea##SubclassName##Private *priv; \
  }; \
  struct CongEditorArea##SubclassName##Class \
  { \
	  BaseClassName##Class klass; \
  }; \
  GType \
  cong_editor_area_##subclass_name##_get_type (void); \
  CongEditorArea##SubclassName * \
  cong_editor_area_##subclass_name##_construct (CongEditorArea##SubclassName *editor_area_##subclass_name, \
						CongEditorWidget3* widget); \
  CongEditorArea* \
  cong_editor_area_##subclass_name##_new (CongEditorWidget3* widget);


#define CONG_EDITOR_AREA_DECLARE_PLUGIN_SUBCLASS(SubclassName, subclass_name, BaseClassName, base_class_name) \
  CONG_EDITOR_AREA_DECLARE_SUBCLASS(SubclassName, subclass_name, BaseClassName, base_class_name)

#define CONG_EDITOR_AREA_DECLARE_RENDER_SELF(subclass_name) \
  static void \
  cong_editor_area_##subclass_name##_render_self (CongEditorArea *area, \
			                          const GdkRectangle *widget_rect);


#define CONG_EDITOR_AREA_CONNECT_RENDER_SELF(subclass_name) \
  { \
	  CongEditorAreaClass *area_klass = CONG_EDITOR_AREA_CLASS(klass); \
	  area_klass->render_self = cong_editor_area_##subclass_name##_render_self; \
  }

#define CONG_EDITOR_AREA_DEFINE_SUBCLASS(SubclassName, subclass_name, SUBCLASS_MACRO, BaseClassName, base_class_name) \
   /* CONG_EDITOR_AREA_DECLARE_HOOKS(subclass_name) */ \
   CONG_DEFINE_CLASS_BEGIN(CongEditorArea##SubclassName, cong_editor_area_##subclass_name, CONG_EDITOR_AREA_##SUBCLASS_MACRO, BaseClassName, base_class_name##_get_type () ) \
      CONG_EDITOR_AREA_CONNECT_RENDER_SELF(subclass_name) \
   CONG_DEFINE_CLASS_END() \
   /* CONG_EDITOR_AREA_IMPLEMENT_NEW(element_##subclass_name) */ \
   CongEditorArea* \
   cong_editor_area_##subclass_name##_new (CongEditorWidget3 *editor_widget) \
   { \
	     return CONG_EDITOR_AREA (cong_editor_area_##subclass_name##_construct \
				      (g_object_new (cong_editor_area_##subclass_name##_get_type (), NULL), \
				       editor_widget)); \
   }

#define CONG_EDITOR_AREA_DEFINE_PLUGIN_SUBCLASS(SubclassName, subclass_name, SUBCLASS_MACRO, BaseClassName, base_class_name) \
  CONG_EDITOR_AREA_DEFINE_SUBCLASS(SubclassName, subclass_name, SUBCLASS_MACRO, BaseClassName, base_class_name)

#define CONG_EDITOR_AREA_IMPLEMENT_DISPOSE_BEGIN(SubclassName, subclass_name, SUBCLASS_MACRO) \
     CONG_OBJECT_IMPLEMENT_DISPOSE_BEGIN(CongEditorArea##SubclassName, cong_editor_area_##subclass_name, CONG_EDITOR_AREA_##SUBCLASS_MACRO, editor_area_##subclass_name)

#define CONG_EDITOR_AREA_IMPLEMENT_DISPOSE_END(subclass_name) \
     CONG_OBJECT_IMPLEMENT_DISPOSE_END(cong_editor_area_##subclass_name)

#define CONG_EDITOR_AREA_IMPLEMENT_EMPTY_DISPOSE(subclass_name) \
     CONG_DEFINE_EMPTY_DISPOSE(cong_editor_area_##subclass_name)

#define CONG_EDITOR_AREA_DEFINE_EMPTY_CONSTRUCT(SubclassName, subclass_name, base_class_name) \
      CongEditorArea##SubclassName* \
      cong_editor_area_##subclass_name##_construct (CongEditorArea##SubclassName *editor_area_##subclass_name,\
      					            CongEditorWidget3* editor_widget) \
      {\
      	base_class_name##_construct (BASE_CLASS_MACRO (editor_area_##subclass_name),\
      				    editor_widget); \
      	return editor_area_##subclass_name;\
      }

G_BEGIN_DECLS

/* 
   EditorNode subclass declarations: 
*/
#define CONG_EDITOR_NODE_ELEMENT_TABLE_GROUP_TYPE	      (cong_editor_node_element_table_group_get_type ())
#define CONG_EDITOR_NODE_ELEMENT_TABLE_GROUP(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_NODE_ELEMENT_TABLE_GROUP_TYPE, CongEditorNodeElementTableGroup)
#define CONG_EDITOR_NODE_ELEMENT_TABLE_GROUP_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_NODE_ELEMENT_TABLE_GROUP_TYPE, CongEditorNodeElementTableGroupClass)
#define IS_CONG_EDITOR_NODE_ELEMENT_TABLE_GROUP(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_NODE_ELEMENT_TABLE_GROUP_TYPE)
CONG_EDITOR_NODE_DECLARE_PLUGIN_SUBCLASS(TableGroup, table_group)

#define CONG_EDITOR_NODE_ELEMENT_TABLE_ROW_TYPE	      (cong_editor_node_element_table_row_get_type ())
#define CONG_EDITOR_NODE_ELEMENT_TABLE_ROW(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_NODE_ELEMENT_TABLE_ROW_TYPE, CongEditorNodeElementTableRow)
#define CONG_EDITOR_NODE_ELEMENT_TABLE_ROW_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_NODE_ELEMENT_TABLE_ROW_TYPE, CongEditorNodeElementTableRowClass)
#define IS_CONG_EDITOR_NODE_ELEMENT_TABLE_ROW(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_NODE_ELEMENT_TABLE_ROW_TYPE)
CONG_EDITOR_NODE_DECLARE_PLUGIN_SUBCLASS(TableRow, table_row)

#define CONG_EDITOR_NODE_ELEMENT_TABLE_ENTRY_TYPE	      (cong_editor_node_element_table_entry_get_type ())
#define CONG_EDITOR_NODE_ELEMENT_TABLE_ENTRY(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_NODE_ELEMENT_TABLE_ENTRY_TYPE, CongEditorNodeElementTableEntry)
#define CONG_EDITOR_NODE_ELEMENT_TABLE_ENTRY_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_NODE_ELEMENT_TABLE_ENTRY_TYPE, CongEditorNodeElementTableEntryClass)
#define IS_CONG_EDITOR_NODE_ELEMENT_TABLE_ENTRY(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_NODE_ELEMENT_TABLE_ENTRY_TYPE)
CONG_EDITOR_NODE_DECLARE_PLUGIN_SUBCLASS(TableEntry, table_entry)

/* 
   EditorArea subclass declarations: 
*/
#define CONG_EDITOR_AREA_TABLE_GROUP_TYPE	        (cong_editor_area_table_group_get_type ())
#define CONG_EDITOR_AREA_TABLE_GROUP(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_TABLE_GROUP_TYPE, CongEditorAreaTableGroup)
#define CONG_EDITOR_AREA_TABLE_GROUP_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_TABLE_GROUP_TYPE, CongEditorAreaTableGroupClass)
#define IS_CONG_EDITOR_AREA_TABLE_GROUP(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_TABLE_GROUP_TYPE)
CONG_EDITOR_AREA_DECLARE_PLUGIN_SUBCLASS(TableGroup, table_group, CongEditorAreaBorder, cong_editor_area_border)
CONG_EDITOR_AREA_DECLARE_RENDER_SELF(table_group)

#define CONG_EDITOR_AREA_TABLE_ROW_TYPE	        (cong_editor_area_table_row_get_type ())
#define CONG_EDITOR_AREA_TABLE_ROW(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_TABLE_ROW_TYPE, CongEditorAreaTableRow)
#define CONG_EDITOR_AREA_TABLE_ROW_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_TABLE_ROW_TYPE, CongEditorAreaTableRowClass)
#define IS_CONG_EDITOR_AREA_TABLE_ROW(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_TABLE_ROW_TYPE)
CONG_EDITOR_AREA_DECLARE_PLUGIN_SUBCLASS(TableRow, table_row, CongEditorAreaBorder, cong_editor_area_border)
CONG_EDITOR_AREA_DECLARE_RENDER_SELF(table_row)

#define CONG_EDITOR_AREA_TABLE_ENTRY_TYPE	        (cong_editor_area_table_entry_get_type ())
#define CONG_EDITOR_AREA_TABLE_ENTRY(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_TABLE_ENTRY_TYPE, CongEditorAreaTableEntry)
#define CONG_EDITOR_AREA_TABLE_ENTRY_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_TABLE_ENTRY_TYPE, CongEditorAreaTableEntryClass)
#define IS_CONG_EDITOR_AREA_TABLE_ENTRY(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_TABLE_ENTRY_TYPE)
CONG_EDITOR_AREA_DECLARE_PLUGIN_SUBCLASS(TableEntry, table_entry, CongEditorAreaBorder, cong_editor_area_border)
CONG_EDITOR_AREA_DECLARE_RENDER_SELF(table_entry)

G_END_DECLS


/* 
   EditorNode subclass definitions: 
*/
CONG_EDITOR_NODE_DEFINE_PLUGIN_SUBCLASS(TableGroup, table_group, CONG_EDITOR_NODE_ELEMENT_TABLE_GROUP, cong_editor_area_table_group_new)
CONG_EDITOR_NODE_DEFINE_PLUGIN_SUBCLASS(TableRow, table_row, CONG_EDITOR_NODE_ELEMENT_TABLE_ROW, cong_editor_area_table_row_new)
CONG_EDITOR_NODE_DEFINE_PLUGIN_SUBCLASS(TableEntry, table_entry, CONG_EDITOR_NODE_ELEMENT_TABLE_ENTRY, cong_editor_area_table_entry_new)

/* 
   EditorArea subclass definitions: 
*/
#if 1
struct CongEditorAreaTableGroupPrivate
{
	GdkGC *gc;
};

struct CongEditorAreaTableRowPrivate
{
	GdkGC *gc;
};

struct CongEditorAreaTableEntryPrivate
{
	GdkGC *gc;
};

CONG_EDITOR_AREA_DEFINE_PLUGIN_SUBCLASS(TableGroup, table_group, TABLE_GROUP, CongEditorAreaBorder, cong_editor_area_border)
CONG_EDITOR_AREA_DEFINE_PLUGIN_SUBCLASS(TableRow, table_row, TABLE_ROW, CongEditorAreaBorder, cong_editor_area_border)
CONG_EDITOR_AREA_DEFINE_PLUGIN_SUBCLASS(TableEntry, table_entry, TABLE_ENTRY, CongEditorAreaBorder, cong_editor_area_border)

CONG_EDITOR_AREA_IMPLEMENT_EMPTY_DISPOSE(table_group)
CONG_EDITOR_AREA_IMPLEMENT_EMPTY_DISPOSE(table_row)
CONG_EDITOR_AREA_IMPLEMENT_EMPTY_DISPOSE(table_entry)


/* Table group areas: */
CongEditorAreaTableGroup*
cong_editor_area_table_group_construct (CongEditorAreaTableGroup *editor_area_table_group,
					CongEditorWidget3 *editor_widget)
{
	GdkGC *gc;
	GdkColor color;
	
	cong_editor_area_border_construct (CONG_EDITOR_AREA_BORDER (editor_area_table_group),
					   editor_widget,
					   5,
					   5,
					   5,
					   5);
	gc = gdk_gc_new (cong_gui_get_a_window()->window);
	gdk_gc_set_line_attributes (gc,
				    1,
				    GDK_LINE_ON_OFF_DASH,
				    GDK_CAP_NOT_LAST,
				    GDK_JOIN_MITER);
	cong_eel_rgb_to_gdk_color (&color, 0xff, 0x00, 0x00);	
	gdk_gc_set_foreground (gc, &color);
	PRIVATE(editor_area_table_group)->gc = gc;
	
	return editor_area_table_group;
}

static void 
cong_editor_area_table_group_render_self (CongEditorArea *area,
					  const GdkRectangle *widget_rect)
{
	CongEditorAreaTableGroup *area_table_group = CONG_EDITOR_AREA_TABLE_GROUP(area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);

	gdk_draw_rectangle (GDK_DRAWABLE(window),
			    PRIVATE(area_table_group)->gc,
			    FALSE,
			    rect->x+2,
			    rect->y+2,
			    rect->width-3,
			    rect->height-3);

	cong_editor_area_debug_render_state (area);
}

/* Table row areas: */
CongEditorAreaTableRow*
cong_editor_area_table_row_construct (CongEditorAreaTableRow *editor_area_table_row,
				      CongEditorWidget3 *editor_widget)
{
	GdkGC *gc;
	GdkColor color;

	cong_editor_area_border_construct (CONG_EDITOR_AREA_BORDER (editor_area_table_row),
					   editor_widget,
					   5,
					   5,
					   5,
					   5);
	gc = gdk_gc_new (cong_gui_get_a_window()->window);
	gdk_gc_set_line_attributes (gc,
				    1,
				    GDK_LINE_ON_OFF_DASH,
				    GDK_CAP_NOT_LAST,
				    GDK_JOIN_MITER);
	cong_eel_rgb_to_gdk_color (&color, 0x00, 0xff, 0x00);	
	gdk_gc_set_foreground (gc, &color);
	PRIVATE(editor_area_table_row)->gc = gc;

	return editor_area_table_row;
}

static void 
cong_editor_area_table_row_render_self (CongEditorArea *area,
					const GdkRectangle *widget_rect)
{
	CongEditorAreaTableRow *area_table_row = CONG_EDITOR_AREA_TABLE_ROW(area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);

	gdk_draw_rectangle (GDK_DRAWABLE(window),
			    PRIVATE(area_table_row)->gc,
			    FALSE,
			    rect->x+2,
			    rect->y+2,
			    rect->width-3,
			    rect->height-3);

	cong_editor_area_debug_render_state (area);
}

/* Table entry areas: */
CongEditorAreaTableEntry*
cong_editor_area_table_entry_construct (CongEditorAreaTableEntry *editor_area_table_entry,
					CongEditorWidget3 *editor_widget)
{
	GdkGC *gc;
	GdkColor color;

	cong_editor_area_border_construct (CONG_EDITOR_AREA_BORDER (editor_area_table_entry),
					   editor_widget,
					   5,
					   5,
					   5,
					   5);
	gc = gdk_gc_new (cong_gui_get_a_window()->window);
	gdk_gc_set_line_attributes (gc,
				    1,
				    GDK_LINE_ON_OFF_DASH,
				    GDK_CAP_NOT_LAST,
				    GDK_JOIN_MITER);
	cong_eel_rgb_to_gdk_color (&color, 0x00, 0x00, 0xff);	
	gdk_gc_set_foreground (gc, &color);
	PRIVATE(editor_area_table_entry)->gc = gc;

	return editor_area_table_entry;
}

static void 
cong_editor_area_table_entry_render_self (CongEditorArea *area,
					  const GdkRectangle *widget_rect)
{
	CongEditorAreaTableEntry *area_table_entry = CONG_EDITOR_AREA_TABLE_ENTRY(area);
	GdkWindow *window = cong_editor_area_get_gdk_window(area);
	const GdkRectangle* rect = cong_editor_area_get_window_coords (area);

	gdk_draw_rectangle (GDK_DRAWABLE(window),
			    PRIVATE(area_table_entry)->gc,
			    FALSE,
			    rect->x+2,
			    rect->y+2,
			    rect->width-3,
			    rect->height-3);

	cong_editor_area_debug_render_state (area);
}


#else
CongEditorArea*
cong_editor_area_paragraph_new (CongEditorWidget3 *editor_widget);

CongEditorArea*
cong_editor_area_table_group_new (CongEditorWidget3 *editor_widget)
{
	return cong_editor_area_paragraph_new (editor_widget);
}
CongEditorArea*
cong_editor_area_table_row_new (CongEditorWidget3 *editor_widget)
{
	return cong_editor_area_paragraph_new (editor_widget);
}
CongEditorArea*
cong_editor_area_table_entry_new (CongEditorWidget3 *editor_widget)
{
	return cong_editor_area_paragraph_new (editor_widget);
}
#endif

/* fixme */

static CongEditorNodeElement*  
manufacture_editor_node_table_group (CongServiceEditorNodeFactory *plugin_editor_node_factory, 
				     CongEditorWidget3 *editor_widget, 
				     CongTraversalNode *traversal_node,
				     gpointer user_data)
{
	return CONG_EDITOR_NODE_ELEMENT( cong_editor_node_element_table_group_new (editor_widget,
										   traversal_node));
}
static CongEditorNodeElement*  
manufacture_editor_node_table_row (CongServiceEditorNodeFactory *plugin_editor_node_factory, 
				   CongEditorWidget3 *editor_widget, 
				   CongTraversalNode *traversal_node,
				   gpointer user_data)
{
	return CONG_EDITOR_NODE_ELEMENT( cong_editor_node_element_table_row_new (editor_widget,
										 traversal_node));
}
static CongEditorNodeElement*  
manufacture_editor_node_table_entry (CongServiceEditorNodeFactory *plugin_editor_node_factory, 
				     CongEditorWidget3 *editor_widget, 
				     CongTraversalNode *traversal_node,
				     gpointer user_data)
{
	return CONG_EDITOR_NODE_ELEMENT( cong_editor_node_element_table_entry_new (editor_widget,
										   traversal_node));
}

/* would be exposed as "plugin_register"? */
/**
 * plugin_table_plugin_register:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_table_plugin_register(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);
	
	cong_plugin_register_editor_node_factory (plugin, 
						  _("Table Group"), 
						  "foo",
						  "table-group",
						  manufacture_editor_node_table_group,
						  NULL);
	cong_plugin_register_editor_node_factory (plugin, 
						  _("Table Row"), 
						  "foo",
						  "table-row",
						  manufacture_editor_node_table_row,
						  NULL);
	cong_plugin_register_editor_node_factory (plugin, 
						  _("Table Entry"), 
						  "foo",
						  "table-entry",
						  manufacture_editor_node_table_entry,
						  NULL);
	return TRUE;
}

/* exposed as "plugin_configure"? legitimate for it not to be present */
/**
 * plugin_table_plugin_configure:
 * @plugin:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
plugin_table_plugin_configure(CongPlugin *plugin)
{
	g_return_val_if_fail(plugin, FALSE);

	return TRUE;
}
