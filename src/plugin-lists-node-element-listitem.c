/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * plugin-lists-node-element-listitem.c
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
 * Fragments of code based upon libxslt: numbers.c
 */

#include "global.h"
#include "plugin-lists-node-element-listitem.h"
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"
#include "cong-util.h"
#include "cong-enum-mapping.h"

#include "plugin-lists-area-listitem.h"

#define PRIVATE(x) ((x)->private)

struct CongEditorNodeElementListitemDetails
{
	int dummy;
};

static CongEditorArea*
generate_block_area (CongEditorNode *editor_node);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongEditorNodeElementListitem, 
			cong_editor_node_element_listitem,
			CongEditorNodeElement,
			CONG_EDITOR_NODE_ELEMENT_TYPE );

static void
cong_editor_node_element_listitem_class_init (CongEditorNodeElementListitemClass *klass)
{
	CongEditorNodeClass *node_klass = CONG_EDITOR_NODE_CLASS(klass);

	node_klass->generate_block_area = generate_block_area;
}

static void
cong_editor_node_element_listitem_instance_init (CongEditorNodeElementListitem *node_element_listitem)
{
	node_element_listitem->private = g_new0(CongEditorNodeElementListitemDetails,1);
}

CongEditorNodeElementListitem*
cong_editor_node_element_listitem_construct (CongEditorNodeElementListitem *editor_node_element_listitem,
					     CongEditorWidget3* editor_widget,
					     CongNodePtr node,
					     CongEditorNode *traversal_parent)
{
	cong_editor_node_element_construct (CONG_EDITOR_NODE_ELEMENT (editor_node_element_listitem),
					    editor_widget,
					    node,
					    traversal_parent);

	return editor_node_element_listitem;
}

CongEditorNode*
cong_editor_node_element_listitem_new (CongEditorWidget3* widget,
				       CongNodePtr node,
				       CongEditorNode *traversal_parent)
{
#if DEBUG_EDITOR_NODE_LIFETIMES
	g_message("cong_editor_node_element_listitem_new(%s)", node->name);
#endif

	return CONG_EDITOR_NODE( cong_editor_node_element_listitem_construct
				 (g_object_new (CONG_EDITOR_NODE_ELEMENT_LISTITEM_TYPE, NULL),
				  widget,
				  node,
				  traversal_parent));
}

enum CongNumeration {
	CONG_NUMERATION_ARABIC,
	CONG_NUMERATION_LOWER_ALPHA,
	CONG_NUMERATION_LOWER_ROMAN,
	CONG_NUMERATION_UPPER_ALPHA,
	CONG_NUMERATION_UPPER_ROMAN       
};


/* Code adapted from libxslt: */
static const gchar alpha_upper_list[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const gchar alpha_lower_list[] = "abcdefghijklmnopqrstuvwxyz";


/*
 * 0 should return empty
 * 1..26 should return "a".."z"
 * 27..52 should return "aa".."az"
 */
gchar*
cong_util_format_number_alpha (guint number,
			       gboolean is_upper)
{
	gchar *result = g_strdup("");
	const gchar *alpha_list;
	guint alpha_size = (guint)sizeof(alpha_upper_list);
	
	alpha_list = (is_upper) ? alpha_upper_list : alpha_lower_list;

	while (number>0) {
		gchar tmp[2];
		tmp[0] = alpha_list[((number-1) % alpha_size)];
		tmp[1] = '\0';

		cong_util_prepend (&result, tmp);
		number /= alpha_size;
	}

	return result;
}

gchar*
cong_util_format_number_roman (guint number,
			       gboolean is_upper)
{
	gchar *result = g_strdup("");

    /*
     * Based on an example by Jim Walsh
     */
    while (number >= 1000.0) {
	cong_util_append(&result, (is_upper) ? "M" : "m");
	number -= 1000.0;
    }
    if (number >= 900.0) {
	cong_util_append(&result, (is_upper) ? "CM" : "cm");
	number -= 900.0;
    }
    while (number >= 500.0) {
	cong_util_append(&result, (is_upper) ? "D" : "d");
	number -= 500.0;
    }
    if (number >= 400.0) {
	cong_util_append(&result, (is_upper) ? "CD" : "cd");
	number -= 400.0;
    }
    while (number >= 100.0) {
	cong_util_append(&result, (is_upper) ? "C" : "c");
	number -= 100.0;
    }
    if (number >= 90.0) {
	cong_util_append(&result, (is_upper) ? "XC" : "xc");
	number -= 90.0;
    }
    while (number >= 50.0) {
	cong_util_append(&result, (is_upper) ? "L" : "l");
	number -= 50.0;
    }
    if (number >= 40.0) {
	cong_util_append(&result, (is_upper) ? "XL" : "xl");
	number -= 40.0;
    }
    while (number >= 10.0) {
	cong_util_append(&result, (is_upper) ? "X" : "x");
	number -= 10.0;
    }
    if (number >= 9.0) {
	cong_util_append(&result, (is_upper) ? "IX" : "ix");
	number -= 9.0;
    }
    while (number >= 5.0) {
	cong_util_append(&result, (is_upper) ? "V" : "v");
	number -= 5.0;
    }
    if (number >= 4.0) {
	cong_util_append(&result, (is_upper) ? "IV" : "iv");
	number -= 4.0;
    }
    while (number >= 1.0) {
	cong_util_append(&result, (is_upper) ? "I" : "i");
	number--;
    }

    return result;
}

gchar*
cong_util_numeration (enum CongNumeration numeration, 
		      guint number)
{
	switch (numeration) {
	default: g_assert_not_reached();
	case CONG_NUMERATION_ARABIC:
		return g_strdup_printf("%i", number);

	case CONG_NUMERATION_LOWER_ALPHA:
		return cong_util_format_number_alpha (number, FALSE);

	case CONG_NUMERATION_LOWER_ROMAN:
		return cong_util_format_number_roman (number, FALSE);

	case CONG_NUMERATION_UPPER_ALPHA:
		return cong_util_format_number_alpha (number, TRUE);

	case CONG_NUMERATION_UPPER_ROMAN:
		return cong_util_format_number_roman (number, TRUE);
	}
}

static const CongEnumMapping docbook_orderedlist_numeration[] =
{
	{"arabic", CONG_NUMERATION_ARABIC},
	{"loweralpha", CONG_NUMERATION_LOWER_ALPHA},
	{"lowerroman", CONG_NUMERATION_LOWER_ROMAN},
	{"upperalpha", CONG_NUMERATION_UPPER_ALPHA},
	{"upperroman", CONG_NUMERATION_UPPER_ROMAN}
};


enum CongNumeration
cong_util_get_numeration_from_docbook_orderedlist_attr (const gchar *numeration_attr)
{
	return cong_enum_mapping_lookup (docbook_orderedlist_numeration,
					 sizeof(docbook_orderedlist_numeration)/sizeof(CongEnumMapping),
					 numeration_attr,
					 CONG_NUMERATION_ARABIC);
}

gchar*
cong_util_string_from_unichar (gunichar ch) 
{
	gunichar str[2];

	str[0]=ch;
	str[1]=0;

	return g_ucs4_to_utf8 (str,
			       2,
			       NULL,
			       NULL,
			       NULL);
}

#define BULLET_UNICHAR (0x2022)

guint
get_child_index(CongEditorNodeElementListitem* listitem)
{
	CongNodePtr node_listitem;
	CongNodePtr node_parent;
	CongNodePtr node_child;
	guint item_count;

	g_return_val_if_fail (IS_CONG_EDITOR_NODE_ELEMENT_LISTITEM(listitem), 0);

	node_listitem = cong_editor_node_get_node (CONG_EDITOR_NODE(listitem));
	node_parent = cong_node_parent (node_listitem);
	node_child = node_parent->children;

	item_count = 0;
	while (node_child) {
		if (node_child == node_listitem) {
			return item_count+1;
		}
		
		if (cong_node_is_tag(node_child, NULL, "listitem")) {
			item_count++;
		}
		
		node_child = node_child->next;
	}

	return item_count+1;
}


gchar*
cong_editor_node_element_listitem_calculate_label (CongEditorNodeElementListitem* listitem)
{
	CongNodePtr node_listitem;
	CongNodePtr node_parent;

	g_return_val_if_fail (IS_CONG_EDITOR_NODE_ELEMENT_LISTITEM(listitem), NULL);

	node_listitem = cong_editor_node_get_node (CONG_EDITOR_NODE(listitem));
	node_parent = cong_node_parent (node_listitem);
	
	/*DocBook-specific hackery: */
	
	if (node_parent==NULL) {
		return cong_util_string_from_unichar (BULLET_UNICHAR);
	}

	if (cong_node_is_tag(node_parent, NULL, "itemizedlist")) {

		return cong_util_string_from_unichar (BULLET_UNICHAR);

	} else if (cong_node_is_tag(node_parent, NULL, "orderedlist")) {
		CongXMLChar* attr_numeration = cong_node_get_attribute(node_parent, "numeration");
		enum CongNumeration numeration = cong_util_get_numeration_from_docbook_orderedlist_attr (attr_numeration);
		guint child_index = get_child_index(listitem);

		g_free (attr_numeration);

		return cong_util_numeration (numeration, child_index);
	}

	return cong_util_string_from_unichar (BULLET_UNICHAR);	
}


static CongEditorArea*
generate_block_area (CongEditorNode *editor_node)
{
	CongEditorArea *new_area;
	gchar* label;

	g_return_val_if_fail (editor_node, NULL);

	label = cong_editor_node_element_listitem_calculate_label (CONG_EDITOR_NODE_ELEMENT_LISTITEM(editor_node));

	new_area = cong_editor_area_listitem_new (cong_editor_node_get_widget (editor_node),
						  label);

	g_free (label);

	/* Connect to various signals: */
	/* FIXME */

	cong_editor_area_connect_node_signals (new_area,
					       editor_node);

	return new_area;
}
