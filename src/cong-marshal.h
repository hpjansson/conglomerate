
#ifndef __cong_cclosure_marshal_MARSHAL_H__
#define __cong_cclosure_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* BOOLEAN:POINTER (cong.marshal:3) */
extern void cong_cclosure_marshal_BOOLEAN__POINTER (GClosure     *closure,
                                                    GValue       *return_value,
                                                    guint         n_param_values,
                                                    const GValue *param_values,
                                                    gpointer      invocation_hint,
                                                    gpointer      marshal_data);

/* VOID:POINTER (cong.marshal:6) */
#define cong_cclosure_marshal_VOID__POINTER	g_cclosure_marshal_VOID__POINTER

/* VOID:POINTER,POINTER (cong.marshal:9) */
extern void cong_cclosure_marshal_VOID__POINTER_POINTER (GClosure     *closure,
                                                         GValue       *return_value,
                                                         guint         n_param_values,
                                                         const GValue *param_values,
                                                         gpointer      invocation_hint,
                                                         gpointer      marshal_data);

/* VOID:POINTER,POINTER,BOOLEAN (cong.marshal:12) */
extern void cong_cclosure_marshal_VOID__POINTER_POINTER_BOOLEAN (GClosure     *closure,
                                                                 GValue       *return_value,
                                                                 guint         n_param_values,
                                                                 const GValue *param_values,
                                                                 gpointer      invocation_hint,
                                                                 gpointer      marshal_data);

/* VOID:POINTER,STRING (cong.marshal:15) */
extern void cong_cclosure_marshal_VOID__POINTER_STRING (GClosure     *closure,
                                                        GValue       *return_value,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint,
                                                        gpointer      marshal_data);

/* VOID:POINTER,POINTER,STRING (cong.marshal:18) */
extern void cong_cclosure_marshal_VOID__POINTER_POINTER_STRING (GClosure     *closure,
                                                                GValue       *return_value,
                                                                guint         n_param_values,
                                                                const GValue *param_values,
                                                                gpointer      invocation_hint,
                                                                gpointer      marshal_data);

/* VOID:POINTER,POINTER,STRING,STRING (cong.marshal:21) */
extern void cong_cclosure_marshal_VOID__POINTER_POINTER_STRING_STRING (GClosure     *closure,
                                                                       GValue       *return_value,
                                                                       guint         n_param_values,
                                                                       const GValue *param_values,
                                                                       gpointer      invocation_hint,
                                                                       gpointer      marshal_data);

/* VOID:STRING,STRING,STRING (cong.marshal:23) */
extern void cong_cclosure_marshal_VOID__STRING_STRING_STRING (GClosure     *closure,
                                                              GValue       *return_value,
                                                              guint         n_param_values,
                                                              const GValue *param_values,
                                                              gpointer      invocation_hint,
                                                              gpointer      marshal_data);

G_END_DECLS

#endif /* __cong_cclosure_marshal_MARSHAL_H__ */

