/** 
 *  cong-eel.c
 *
 *  Contains code copied and pasted from eel-2.0, to avoid dependency issues 
 *  Everything gets prefixed with a "cong_"
 */

#include "global.h"
#include "cong-eel.h"

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
 * Returns: An rgb value.
 *
 * Converts from a GdkColor stlye color to a gdk_rgb one.
 * Alpha gets set to fully opaque
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
