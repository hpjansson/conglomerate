/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/** 
 *  cong-eel.c
 *
 *  Contains code copied and pasted from eel-2.0, to avoid dependency issues 
 *  Everything gets prefixed with a "cong_"
 */

#include "global.h"
#include "cong-eel.h"
#include "cong-ui-hooks.h"


void 
cong_eel_log_ref_count (const gchar *name, 
			GObject *obj)
{
#if 0
	g_message("Current ref count of %s = %i (%s)", 
		  name,
		  obj->ref_count,
		  G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(obj)));
#endif
}

void            
cong_eel_rectangle_construct (GdkRectangle  *rectangle,
			      gint                  x,
			      gint                  y,
			      gint                  w,
			      gint                  h)
{
	g_return_if_fail (rectangle);
	rectangle->x = x;
	rectangle->y = y;
	rectangle->width = w;
	rectangle->height = h;
}

gchar*
cong_eel_pango_layout_line_get_text (PangoLayoutLine *layout_line)
{
	g_return_val_if_fail (layout_line, NULL);

	return g_strndup (pango_layout_get_text (layout_line->layout) + layout_line->start_index, 
			  layout_line->length);
}

void
cong_eel_rgb_to_gdk_color (GdkColor             *color,
			   guchar               r,
			   guchar               g,
			   guchar               b)
{
	gboolean result;

	g_return_if_fail (color);

	color->red = (r<<8);
	color->green = (g<<8);
	color->blue = (b<<8);

	result = gdk_colormap_alloc_color (cong_gui_get_a_window()->style->colormap, 
					   color, 
					   FALSE, 
					   TRUE);
	g_assert (result);
}


/**
 * cong_eel_rectangle_contains:
 * @rectangle: Rectangle possibly containing a point.
 * @x: X coordinate of a point.
 * @y: Y coordinate of a point.
 *
 * Retun TRUE if point is contained inside a rectangle
 */
gboolean
cong_eel_rectangle_contains (const GdkRectangle *rectangle, 
			     int x, 
			     int y)
{
	g_return_val_if_fail (rectangle != NULL, FALSE);
	return rectangle->x <= x && rectangle->x + rectangle->width >= x
		&& rectangle->y <= y && rectangle->y + rectangle->height >= y;
}


guint32
cong_eel_rgb16_to_rgb (gushort r, gushort g, gushort b)
{
	guint32 result;

	result = (0xff0000 | (r & 0xff00));
	result <<= 8;
	result |= ((g & 0xff00) | (b >> 8));

	return result;
}

/**
 * cong_eel_gdk_color_to_rgb
 * @color: A GdkColor style color.
 *
 * Converts from a GdkColor stlye color to a gdk_rgb one.
 * Alpha gets set to fully opaque
 * 
 * Returns: An rgb value.
 */
guint32
cong_eel_gdk_color_to_rgb (const GdkColor *color)
{
	return cong_eel_rgb16_to_rgb (color->red, color->green, color->blue);
}

/**
 * cong_eel_gdk_rgb_to_color_spec
 * @color: a gdk_rgb style value.
 *
 * Converts from a gdk_rgb value style to a string color spec.
 * The gdk_rgb color alpha channel is ignored.
 * 
 * Return value: a newly allocated color spec.
 */
char *
cong_eel_gdk_rgb_to_color_spec (const guint32 color)
{
	return g_strdup_printf ("#%06X", (guint) (color & 0xFFFFFF));
}

GtkMenuItem* 
cong_eel_option_menu_get_selected_menu_item(GtkOptionMenu *option_menu)
{
	gint selected_index;
	GList *list_of_menu_items;
	GtkMenuItem *selected_menu_item;

	g_return_val_if_fail (option_menu, NULL);

	selected_index = gtk_option_menu_get_history (option_menu);

	if (selected_index==-1) {
		/* Nothing selected: */
		return NULL;
	}

	list_of_menu_items = gtk_container_get_children (GTK_CONTAINER(gtk_option_menu_get_menu(option_menu)));
	g_assert(list_of_menu_items);

	selected_menu_item = g_list_nth_data (list_of_menu_items, selected_index);

	g_list_free (list_of_menu_items);

	return selected_menu_item;

}

static gchar* cong_eel_utf8_capitalise(const gchar *str)
{
	gchar *result;
	gunichar*   unichar;

	g_return_val_if_fail (str, NULL);

	unichar = g_utf8_to_ucs4_fast (str, -1, NULL);

	/* FIXME: is this correct? */
	unichar[0] = g_unichar_toupper(unichar[0]);

	result = g_ucs4_to_utf8(unichar, -1, NULL, NULL, NULL);

	g_free(unichar);

	return result;
}

/**
 * split_xmlname:
 * @xml_name:
 * 
 * Splits things like "this-is-a-tag" into its individual words.
 * Can handle hyphenated words.
 * FIXME:  Add support for splitting caps-seperated things like "ThisIsATag"
 * FIXME:  Probably should convert everything to lower case as well on output
 * FIXME:  To be really smart, we could try to spot acronyms and fully capitalise them...  would require a dictionary though.
 * 
 * Returns:
 */
static gchar** split_xmlname(const gchar *xml_name)
{
	/* FIXME: is this really UTF-8 safe? */
	return g_strsplit (xml_name,
			   "-",
			   0);
}

gchar *cong_eel_prettify_xml_name_with_header_capitalisation(const gchar *xml_name)
{
	gchar *result = NULL;
	gchar** string_array;
	gchar** iter;

	g_return_val_if_fail(xml_name, NULL);

	string_array = split_xmlname(xml_name);

	for (iter = string_array; *iter; iter++) {
		cong_eel_set_string (iter, cong_eel_utf8_capitalise(*iter));
	}

	result = g_strjoinv (" ",
			     string_array);

	g_strfreev(string_array);

	return result;
}

gchar *cong_eel_prettify_xml_name_with_sentence_capitalisation(const gchar *xml_name)
{
	gchar *result = NULL;
	gchar** string_array;
	gchar** iter;

	g_return_val_if_fail(xml_name, NULL);

	string_array = split_xmlname(xml_name);

	/* Capitalise first word, if there is one: */
	if (*string_array) {
		cong_eel_set_string (&string_array[0], cong_eel_utf8_capitalise(string_array[0]));
	}
	for (iter = string_array; *iter; iter++) {
		/* FIXME: unwritten */
	}

	result = g_strjoinv (" ",
			     string_array);

	g_strfreev(string_array);

	return result;
}

/* Routine that sets a string ptr to point to a new value, and frees the old value (if any) */
void cong_eel_set_string(gchar **string, gchar *value)
{
	gchar *old_value;

	g_return_if_fail (string);

	old_value = *string;

	*string = value;

	if (old_value) {
		g_free (old_value);
	}
}

void 
cong_eel_draw_pixbuf (GdkDrawable *drawable,
		      GdkGC *gc,
		      GdkPixbuf *pixbuf,
		      gint src_x,
		      gint src_y,
		      gint dest_x,
		      gint dest_y,
		      gint width,
		      gint height,
		      GdkRgbDither dither,
		      gint x_dither,
		      gint y_dither)
{
#if 0
	/* Code for Gdk-2.2 and later: */
	gdk_draw_pixbuf (drawable,
			 gc,
			 pixbuf,
			 src_x,
			 src_y,
			 dest_x,
			 dest_y,
			 -1,
			 -1,
			 dither,
			 x_dither,
			 y_dither);
#else
	/* Code for pre Gdk-2.2: */
	gdk_pixbuf_render_to_drawable (pixbuf,
				       drawable,
				       gc,
				       src_x,
				       src_y,
				       dest_x,
				       dest_y,
				       -1,
				       -1,
				       dither,
				       x_dither,
				       y_dither);
#endif
}
