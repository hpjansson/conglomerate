/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-marshal.c
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
#include "cong-marshal.h"

/* Marshaller implementations: */
void
cong_cclosure_marshal_BOOLEAN__POINTER (GClosure     *closure,
					GValue       *return_value,
					guint         n_param_values,
					const GValue *param_values,
					gpointer      invocation_hint,
					gpointer      marshal_data)
{
	typedef gboolean (*GMarshalFunc_BOOLEAN__POINTER) (gpointer     data1,
							   gpointer     arg_1,
							   gpointer     data2);
	register GMarshalFunc_BOOLEAN__POINTER callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	gboolean result;
	
	g_return_if_fail (n_param_values == 2);

	if (G_CCLOSURE_SWAP_DATA (closure))
		{
			data1 = closure->data;
			data2 = g_value_peek_pointer (param_values + 0);
		}
	else
		{
			data1 = g_value_peek_pointer (param_values + 0);
			data2 = closure->data;
		}
	callback = (GMarshalFunc_BOOLEAN__POINTER) (marshal_data ? marshal_data : cc->callback);
	
	result = callback (data1,
			   g_value_get_pointer (param_values + 1),
			   data2);

	g_value_set_boolean (return_value, result);
}

void 
cong_cclosure_marshal_VOID__CONGNODEPTR (GClosure     *closure,
					 GValue       *return_value,
					 guint         n_param_values,
					 const GValue *param_values,
					 gpointer      invocation_hint,
					 gpointer      marshal_data)
{
	typedef void (*GMarshalFunc_VOID__CONGNODEPTR) (gpointer     data1,
							CongNodePtr  arg_1,
							gpointer     data2);
	register GMarshalFunc_VOID__CONGNODEPTR callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	
	g_return_if_fail (n_param_values == 2);
	
	if (G_CCLOSURE_SWAP_DATA (closure))
		{
			data1 = closure->data;
			data2 = g_value_peek_pointer (param_values + 0);
		}
	else
		{
			data1 = g_value_peek_pointer (param_values + 0);
			data2 = closure->data;
		}
	callback = (GMarshalFunc_VOID__CONGNODEPTR) (marshal_data ? marshal_data : cc->callback);
	
	callback (data1,
		  g_value_get_pointer (param_values + 1),
		  data2);
}

void 
cong_cclosure_marshal_VOID__CONGNODEPTR_CONGNODEPTR (GClosure     *closure,
						     GValue       *return_value,
						     guint         n_param_values,
						     const GValue *param_values,
						     gpointer      invocation_hint,
						     gpointer      marshal_data)
{
	typedef void (*GMarshalFunc_VOID__CONGNODEPTR_CONGNODEPTR) (gpointer     data1,
								    CongNodePtr  arg_1,
								    CongNodePtr  arg_2,
								    gpointer     data2);
	register GMarshalFunc_VOID__CONGNODEPTR_CONGNODEPTR callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	
	g_return_if_fail (n_param_values == 3);
	
	if (G_CCLOSURE_SWAP_DATA (closure))
		{
			data1 = closure->data;
			data2 = g_value_peek_pointer (param_values + 0);
		}
	else
		{
			data1 = g_value_peek_pointer (param_values + 0);
			data2 = closure->data;
		}
	callback = (GMarshalFunc_VOID__CONGNODEPTR_CONGNODEPTR) (marshal_data ? marshal_data : cc->callback);
	
	callback (data1,
		  g_value_get_pointer (param_values + 1),
		  g_value_get_pointer (param_values + 2),
		  data2);
}

void 
cong_cclosure_marshal_VOID__CONGNODEPTR_STRING (GClosure     *closure,
						GValue       *return_value,
						guint         n_param_values,
						const GValue *param_values,
						gpointer      invocation_hint,
						gpointer      marshal_data)
{
	typedef void (*GMarshalFunc_VOID__CONGNODEPTR_STRING) (gpointer     data1,
							       CongNodePtr  arg_1,
							       const gchar *arg_2,
							       gpointer     data2);
	register GMarshalFunc_VOID__CONGNODEPTR_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	
	g_return_if_fail (n_param_values == 3);
	
	if (G_CCLOSURE_SWAP_DATA (closure))
		{
			data1 = closure->data;
			data2 = g_value_peek_pointer (param_values + 0);
		}
	else
		{
			data1 = g_value_peek_pointer (param_values + 0);
			data2 = closure->data;
		}
	callback = (GMarshalFunc_VOID__CONGNODEPTR_STRING) (marshal_data ? marshal_data : cc->callback);
	
	callback (data1,
		  g_value_get_pointer (param_values + 1),
		  g_value_get_string (param_values + 2),
		  data2);
}

void 
cong_cclosure_marshal_VOID__CONGNODEPTR_POINTER_STRING (GClosure     *closure,
							GValue       *return_value,
							guint         n_param_values,
							const GValue *param_values,
							gpointer      invocation_hint,
							gpointer      marshal_data)
{
	typedef void (*GMarshalFunc_VOID__CONGNODEPTR_POINTER_STRING) (gpointer     data1,
								       CongNodePtr  arg_1,
								       gpointer     arg_3,
								       const gchar *arg_4,
								       gpointer     data2);
	register GMarshalFunc_VOID__CONGNODEPTR_POINTER_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	
	g_return_if_fail (n_param_values == 4);
	
	if (G_CCLOSURE_SWAP_DATA (closure))
		{
			data1 = closure->data;
			data2 = g_value_peek_pointer (param_values + 0);
		}
	else
		{
			data1 = g_value_peek_pointer (param_values + 0);
			data2 = closure->data;
		}
	callback = (GMarshalFunc_VOID__CONGNODEPTR_POINTER_STRING) (marshal_data ? marshal_data : cc->callback);
	
	callback (data1,
		  g_value_get_pointer (param_values + 1),
		  g_value_get_pointer (param_values + 2),
		  g_value_get_string (param_values + 3),
		  data2);
}

void 
cong_cclosure_marshal_VOID__CONGNODEPTR_STRING_STRING (GClosure     *closure,
						       GValue       *return_value,
						       guint         n_param_values,
						       const GValue *param_values,
						       gpointer      invocation_hint,
						       gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__CONGNODEPTR_STRING_STRING) (gpointer     data1,
								CongNodePtr  arg_1,
								const gchar *arg_2,
								const gchar *arg_3,
								gpointer     data2);
	register GMarshalFunc_VOID__CONGNODEPTR_STRING_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	
	g_return_if_fail (n_param_values == 4);
	
	if (G_CCLOSURE_SWAP_DATA (closure))
		{
			data1 = closure->data;
			data2 = g_value_peek_pointer (param_values + 0);
		}
	else
		{
			data1 = g_value_peek_pointer (param_values + 0);
			data2 = closure->data;
		}
	callback = (GMarshalFunc_VOID__CONGNODEPTR_STRING_STRING) (marshal_data ? marshal_data : cc->callback);
	
	callback (data1,
		  g_value_get_pointer (param_values + 1),
		  g_value_get_string (param_values + 2),
		  g_value_get_string (param_values + 3),
		  data2);
}

void 
cong_cclosure_marshal_VOID__CONGNODEPTR_POINTER_STRING_STRING (GClosure     *closure,
							     GValue       *return_value,
							     guint         n_param_values,
							     const GValue *param_values,
							     gpointer      invocation_hint,
							     gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__CONGNODEPTR_POINTER_STRING_STRING) (gpointer     data1,
									CongNodePtr  arg_1,
									gpointer     arg_2,
									const gchar *arg_3,
									const gchar *arg_4,
									gpointer     data2);
	register GMarshalFunc_VOID__CONGNODEPTR_POINTER_STRING_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	
	g_return_if_fail (n_param_values == 5);
	
	if (G_CCLOSURE_SWAP_DATA (closure))
		{
			data1 = closure->data;
			data2 = g_value_peek_pointer (param_values + 0);
		}
	else
		{
			data1 = g_value_peek_pointer (param_values + 0);
			data2 = closure->data;
		}
	callback = (GMarshalFunc_VOID__CONGNODEPTR_POINTER_STRING_STRING) (marshal_data ? marshal_data : cc->callback);
	
	callback (data1,
		  g_value_get_pointer (param_values + 1),
		  g_value_get_pointer (param_values + 2),
		  g_value_get_string (param_values + 3),
		  g_value_get_string (param_values + 4),
		  data2);
}

void 
cong_cclosure_marshal_VOID__STRING_STRING_STRING (GClosure     *closure,
						  GValue       *return_value,
						  guint         n_param_values,
						  const GValue *param_values,
						  gpointer      invocation_hint,
						  gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__STRING_STRING_STRING) (gpointer     data1,
							   const gchar *arg_1,
							   const gchar *arg_2,
							   const gchar *arg_3,
							   gpointer     data2);
	register GMarshalFunc_VOID__STRING_STRING_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	
	g_return_if_fail (n_param_values == 4);
	
	if (G_CCLOSURE_SWAP_DATA (closure))
		{
			data1 = closure->data;
			data2 = g_value_peek_pointer (param_values + 0);
		}
	else
		{
			data1 = g_value_peek_pointer (param_values + 0);
			data2 = closure->data;
		}
	callback = (GMarshalFunc_VOID__STRING_STRING_STRING) (marshal_data ? marshal_data : cc->callback);
	
	callback (data1,
		  g_value_get_string (param_values + 1),
		  g_value_get_string (param_values + 2),
		  g_value_get_string (param_values + 3),
		  data2);
}

