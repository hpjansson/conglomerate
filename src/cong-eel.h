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

#if 1
#define CONG_EEL_LOG_REF_COUNT(name, obj) cong_eel_log_ref_count(name, obj)
#else
#define CONG_EEL_LOG_REF_COUNT(name, obj) ((void)0)
#endif

void 
cong_eel_log_ref_count (const gchar *name, 
			GObject *obj);


void            
cong_eel_rectangle_construct (GdkRectangle  *rectangle,
			      gint                  x,
			      gint                  y,
			      gint                  w,
			      gint                  h);

gchar*
cong_eel_pango_layout_line_get_text (PangoLayoutLine *layout_line);

/* From eel-gdk-extensions.h: */
gboolean            cong_eel_rectangle_contains             (const GdkRectangle  *rectangle,
							    int                  x,
							    int                  y);

guint32             cong_eel_rgb16_to_rgb                   (gushort              r,
							    gushort              g,
							    gushort              b);
guint32             cong_eel_gdk_color_to_rgb               (const GdkColor      *color);
char *              cong_eel_gdk_rgb_to_color_spec          (guint32              color);

/* From eel-gtk-macros.h: */
/* Call a parent class version of a virtual function (or default
 * signal handler since that's the same thing). Nice because it
 * documents what it's doing and there is less chance for a
 * typo. Depends on the parent class pointer having the conventional
 * name "parent_class" as the boilerplate macro above does it.
 */
#define CONG_EEL_CALL_PARENT(parent_class_cast_macro, signal, parameters)          \
                                                                              \
G_STMT_START {                                                                \
	if (parent_class_cast_macro (parent_class)->signal != NULL) {         \
		(* parent_class_cast_macro (parent_class)->signal) parameters;\
        }                                                                     \
} G_STMT_END

/* Same thing, for functions with a return value. */
#define CONG_EEL_CALL_PARENT_WITH_RETURN_VALUE(parent_class_cast_macro, signal,    \
                                          parameters)                         \
                                                                              \
(parent_class_cast_macro (parent_class)->signal == NULL)                      \
	? 0                                                                   \
	: ((* parent_class_cast_macro (parent_class)->signal) parameters)

/* Call a virtual function. Useful when the virtual function is not a
 * signal, otherwise you want to gtk_signal emit. Nice because it
 * documents what it's doing and there is less chance for a typo.
 */
#define CONG_EEL_CALL_METHOD(class_cast_macro, object, signal, parameters)         \
                                                                              \
G_STMT_START {                                                                \
	if (class_cast_macro (G_OBJECT_GET_CLASS (object))->signal != NULL) { \
		(* class_cast_macro (G_OBJECT_GET_CLASS (object))->signal)    \
                parameters;                                                   \
	}                                                                     \
} G_STMT_END

/* Same thing, for functions with a return value. */
#define CONG_EEL_CALL_METHOD_WITH_RETURN_VALUE(class_cast_macro, object, signal,   \
                                          parameters)                         \
                                                                              \
(class_cast_macro (G_OBJECT_GET_CLASS (object))->signal == NULL)              \
	? 0                                                                   \
	: ((* class_cast_macro (G_OBJECT_GET_CLASS (object))->signal)         \
           parameters)                                                        \

#ifndef G_DISABLE_ASSERT

/* Define a signal that is not implemented by this class but must be 
 * implemented by subclasses. This macro should be used inside the
 * class initialization function. The companion macro CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL
 * must be used earlier in the file. Called like this:
 * 
 * CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
 *					 fm_directory_view,
 *					 clear); 
 */
#define CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL(class_pointer, prefix, signal)        \
                                                                              \
* (void (**)(void)) & (class_pointer)->signal = prefix##_unimplemented_##signal

/* Provide a debug-only implementation of a signal that must be implemented
 * by subclasses. The debug-only implementation fires a warning if it is called.
 * This macro should be placed as if it were a function, earlier in the file
 * than the class initialization function. Called like this:
 * 
 * CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (fm_directory_view, clear);
 */
#define CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL(prefix, signal)                    \
                                                                              \
static void                                                                   \
prefix##_unimplemented_##signal (void)                                        \
{                                                                             \
	g_warning ("failed to override signal " #prefix "->" #signal);        \
}

#else /* G_DISABLE_ASSERT */

#define CONG_EEL_DEFINE_MUST_OVERRIDE_SIGNAL(class_cast_macro, class_pointer, prefix, signal)
#define CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL(prefix, signal)
#define CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL(class_pointer, prefix, signal)

#endif /* G_DISABLE_ASSERT */

/* Access a method. */
#define CONG_EEL_ACCESS_METHOD(class_cast_macro, object, method)                   \
(class_cast_macro (G_OBJECT_GET_CLASS (object))->method)

/* Invoke a method for a given object. */
#define CONG_EEL_INVOKE_METHOD(class_cast_macro, object, method, parameters)       \
((* CONG_EEL_ACCESS_METHOD (class_cast_macro, object, method)) parameters)

/* Assert the non-nullness of a method for a given object. */
#define CONG_EEL_ASSERT_METHOD(class_cast_macro, object, method)                   \
g_assert (CONG_EEL_ACCESS_METHOD (class_cast_macro, object, method) != NULL)

/* Invoke a method if it ain't null. */
#define CONG_EEL_INVOKE_METHOD_IF(class_cast_macro, object, method, parameters)    \
(CONG_EEL_ACCESS_METHOD (class_cast_macro, object, method) ? 0 :                   \
	CONG_EEL_INVOKE_METHOD (class_cast_macro, object, method, parameters))


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
