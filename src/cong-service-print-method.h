/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-print-method.h
 *
 * Copyright (C) 2004 David Malcolm
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

#ifndef __CONG_PRINT_METHOD_H__
#define __CONG_PRINT_METHOD_H__


#include "cong-plugin.h"

G_BEGIN_DECLS

#define CONG_SERVICE_PRINT_METHOD_TYPE	  (cong_service_print_method_get_type ())
#define CONG_SERVICE_PRINT_METHOD(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_SERVICE_PRINT_METHOD_TYPE, CongServicePrintMethod)
#define CONG_SERVICE_PRINT_METHOD_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_SERVICE_PRINT_METHOD_TYPE, CongServicePrintMethodClass)
#define IS_CONG_SERVICE_PRINT_METHOD(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_SERVICE_PRINT_METHOD_TYPE)
CONG_DECLARE_CLASS (CongServicePrintMethod, cong_service_print_method, CongService)

#if ENABLE_PRINTING

typedef gboolean 
(*CongServicePrintMethodDocumentFilter) (CongServicePrintMethod *print_method, 
					 CongDocument *doc, 
					 gpointer user_data);

typedef void 
(*CongServicePrintMethodActionCallback) (CongServicePrintMethod *print_method, 
					 CongDocument *doc, 
					 GnomePrintContext *gpc, 
					 gpointer user_data, 
					 GtkWindow *toplevel_window);

CongServicePrintMethod *
cong_service_print_method_construct (CongServicePrintMethod *print_method,
				     const gchar *name, 
				     const gchar *description,
				     const gchar *service_id,
				     CongServicePrintMethodDocumentFilter doc_filter,
				     CongServicePrintMethodActionCallback action_callback,
				     gpointer user_data);

CongServicePrintMethod*
cong_plugin_register_print_method (CongPlugin *plugin, 
				   const gchar *name, 
				   const gchar *description,
				   const gchar *id,
				   CongServicePrintMethodDocumentFilter doc_filter,
				   CongServicePrintMethodActionCallback action_callback,
				   gpointer user_data);

gboolean
cong_print_method_supports_document (CongServicePrintMethod *print_method, 
				     CongDocument *doc);
void
cong_print_method_invoke (CongServicePrintMethod *print_method, 
			  CongDocument *doc, 
			  GnomePrintContext *gpc, 
			  GtkWindow *toplevel_window);

void 
cong_plugin_for_each_print_method (CongPlugin *plugin, 
				   void 
				   (*callback) (CongServicePrintMethod *print_method, 
						gpointer user_data), 
				   gpointer user_data);

G_END_DECLS

#endif

#endif



