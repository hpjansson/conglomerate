/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-marshal.h
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

#ifndef __CONG_MARSHAL_H__
#define __CONG_MARSHAL_H__

#if 0
#include <gobject/gmarshal.h>
#endif

G_BEGIN_DECLS

/**
   Marshalling functions
 */
extern void
cong_cclosure_marshal_BOOLEAN__POINTER (GClosure     *closure,
					GValue       *return_value,
					guint         n_param_values,
					const GValue *param_values,
					gpointer      invocation_hint,
					gpointer      marshal_data);

extern void
cong_cclosure_marshal_VOID__CONGNODEPTR (GClosure     *closure,
					 GValue       *return_value,
					 guint         n_param_values,
					 const GValue *param_values,
					 gpointer      invocation_hint,
					 gpointer      marshal_data);

extern void
cong_cclosure_marshal_VOID__CONGNODEPTR_CONGNODEPTR (GClosure     *closure,
						     GValue       *return_value,
						     guint         n_param_values,
						     const GValue *param_values,
						     gpointer      invocation_hint,
						     gpointer      marshal_data);


extern void
cong_cclosure_marshal_VOID__CONGNODEPTR_STRING (GClosure     *closure,
						GValue       *return_value,
						guint         n_param_values,
						const GValue *param_values,
						gpointer      invocation_hint,
						gpointer      marshal_data);


extern void
cong_cclosure_marshal_VOID__CONGNODEPTR_STRING_STRING (GClosure     *closure,
						       GValue       *return_value,
						       guint         n_param_values,
						       const GValue *param_values,
						       gpointer      invocation_hint,
						       gpointer      marshal_data);

extern void
cong_cclosure_marshal_VOID__CONGNODEPTR_POINTER_STRING (GClosure     *closure,
							GValue       *return_value,
							guint         n_param_values,
							const GValue *param_values,
							gpointer      invocation_hint,
							gpointer      marshal_data);


extern void
cong_cclosure_marshal_VOID__CONGNODEPTR_POINTER_STRING_STRING (GClosure     *closure,
							       GValue       *return_value,
							       guint         n_param_values,
							       const GValue *param_values,
							       gpointer      invocation_hint,
							       gpointer      marshal_data);


extern void
cong_cclosure_marshal_VOID__STRING_STRING_STRING (GClosure     *closure,
						  GValue       *return_value,
						  guint         n_param_values,
						  const GValue *param_values,
						  gpointer      invocation_hint,
						  gpointer      marshal_data);


G_END_DECLS

#endif
