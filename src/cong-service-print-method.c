/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-print-method.c
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

#include "global.h"
#include "cong-service-print-method.h"

#if ENABLE_PRINTING
struct CongServicePrintMethodPrivate
{
	CongServicePrintMethodDocumentFilter doc_filter;
	CongServicePrintMethodActionCallback action_callback;
	gpointer user_data;
};
CONG_DEFINE_CLASS (CongServicePrintMethod, cong_service_print_method, CONG_SERVICE_PRINT_METHOD, CongService, CONG_SERVICE_TYPE)

/**
 * cong_service_print_method_construct:
 * @print_method:
 * @name:
 * @description:
 * @service_id:
 * @doc_filter:
 * @action_callback:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServicePrintMethod *
cong_service_print_method_construct (CongServicePrintMethod *print_method,
				     const gchar *name, 
				     const gchar *description,
				     const gchar *service_id,
				     CongServicePrintMethodDocumentFilter doc_filter,
				     CongServicePrintMethodActionCallback action_callback,
				     gpointer user_data)
{
	g_return_val_if_fail (IS_CONG_SERVICE_PRINT_METHOD (print_method), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);
	g_return_val_if_fail (doc_filter, NULL);
	g_return_val_if_fail (action_callback, NULL);

	cong_service_construct (CONG_SERVICE (print_method),
				name,
				description,
				service_id);
	PRIVATE (print_method)->doc_filter = doc_filter;
	PRIVATE (print_method)->action_callback = action_callback;
	PRIVATE (print_method)->user_data = user_data;

	return print_method;
}

/* Implementation of CongServicePrintMethod: */
/**
 * cong_print_method_supports_document:
 * @print_method:
 * @doc:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
cong_print_method_supports_document(CongServicePrintMethod *print_method, CongDocument *doc)
{
	g_return_val_if_fail (IS_CONG_SERVICE_PRINT_METHOD (print_method), FALSE);
	g_return_val_if_fail (doc, FALSE);

	g_assert (PRIVATE (print_method)->doc_filter);

	return PRIVATE (print_method)->doc_filter (print_method, 
						   doc, 
						   PRIVATE (print_method)->user_data);
}

/**
 * cong_print_method_invoke:
 * @print_method:
 * @doc:
 * @gpc:
 * @toplevel_window:
 *
 * TODO: Write me
 */
void 
cong_print_method_invoke(CongServicePrintMethod *print_method, CongDocument *doc, GnomePrintContext *gpc, GtkWindow *toplevel_window)
{
	g_return_if_fail (IS_CONG_SERVICE_PRINT_METHOD (print_method));
	g_return_if_fail (doc);
	g_return_if_fail (gpc);
	
	g_assert (PRIVATE (print_method)->action_callback);

	return PRIVATE (print_method)->action_callback (print_method, 
							doc, 
							gpc, 
							PRIVATE (print_method)->user_data, toplevel_window);
}

#endif
