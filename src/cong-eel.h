/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/** 
 *  cong-eel.c
 *
 *  Contains code copied and pasted from eel-2.0, to avoid dependency issues.
 *  Everything gets prefixed with a "cong_"
 */

#ifndef __CONG_EEL_H__
#define __CONG_EEL_H__

G_BEGIN_DECLS

/* From eel-gdk-extensions.h: */
gboolean            cong_eel_rectangle_contains             (const GdkRectangle  *rectangle,
							    int                  x,
							    int                  y);

guint32             cong_eel_rgb16_to_rgb                   (gushort              r,
							    gushort              g,
							    gushort              b);
guint32             cong_eel_gdk_color_to_rgb               (const GdkColor      *color);
char *              cong_eel_gdk_rgb_to_color_spec          (guint32              color);

/* This isn't from eel, but perhaps should be: */
GtkMenuItem*        cong_eel_option_menu_get_selected_menu_item (GtkOptionMenu *option_menu);

/* This isn't from eel, but perhaps should be: */
/* 
   Routines that take an xml tag or attribute name e.g. <conditional-page-master-reference>, split it into words, and apply the given capitalisation (see the GNOME HIG for descriptions of capitalisation) 
*/
gchar *cong_eel_prettify_xml_name_with_header_capitalisation(const gchar *xml_name);
gchar *cong_eel_prettify_xml_name_with_sentence_capitalisation(const gchar *xml_name);

/* Routine that sets a string ptr to point to a new value, and frees the old value (if any) */
void cong_eel_set_string(gchar **string, gchar *value);

G_END_DECLS

#endif
