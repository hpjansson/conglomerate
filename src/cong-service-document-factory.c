/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-document-factory.c
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
#include "cong-service-document-factory.h"
#include "cong-util.h"

struct CongServiceDocumentFactoryPrivate
{
	CongServiceDocumentFactoryPageCreationCallback page_creation_callback;
	CongServiceDocumentFactoryActionCallback action_callback;

	gchar *icon;
	GdkPixbuf *icon16;

	gpointer user_data;
};

CONG_DEFINE_CLASS (CongServiceDocumentFactory, cong_service_document_factory, CONG_SERVICE_DOCUMENT_FACTORY, CongService, CONG_SERVICE_TYPE)

/**
 * cong_service_document_factory_construct:
 * @factory:
 * @name: 
 * @description:
 * @id:
 * @page_creation_callback:
 * @action_callback:
 * @icon:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceDocumentFactory*
cong_service_document_factory_construct (CongServiceDocumentFactory* factory,
					 const gchar *name, 
					 const gchar *description,
					 const gchar *id,
					 CongServiceDocumentFactoryPageCreationCallback page_creation_callback,
					 CongServiceDocumentFactoryActionCallback action_callback,
					 const gchar *icon,
					 gpointer user_data)
{
	g_return_val_if_fail (IS_CONG_SERVICE_DOCUMENT_FACTORY (factory), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (id, NULL);
	g_return_val_if_fail (page_creation_callback, NULL);
	g_return_val_if_fail (action_callback, NULL);
	/* icon is allowed to be NULL */

	cong_service_construct (CONG_SERVICE (factory),
				name,
				description,
				id);

	PRIVATE (factory)->page_creation_callback = page_creation_callback;
	PRIVATE (factory)->action_callback = action_callback;
	if (icon) {
		PRIVATE (factory)->icon = g_strdup(icon);
		PRIVATE (factory)->icon16 = cong_util_load_icon(icon);
	}
	PRIVATE (factory)->user_data = user_data;

	return factory;
	
}

/* Implementation of CongServiceDocumentFactory: */
/**
 * cong_document_factory_invoke_page_creation_callback:
 * @factory:
 * @assistant:
 *
 * TODO: Write me
 */
void 
cong_document_factory_invoke_page_creation_callback(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant)
{
	g_return_if_fail (IS_CONG_SERVICE_DOCUMENT_FACTORY (factory));
	g_return_if_fail (assistant);

#if 0
	g_message("page creation for document factory \"%s\"", cong_service_get_name(CONG_SERVICE(factory)));
#endif

	g_assert(PRIVATE (factory)->page_creation_callback);

	PRIVATE (factory)->page_creation_callback(factory, assistant, PRIVATE (factory)->user_data);
}

/**
 * cong_document_factory_invoke_action_callback:
 * @factory:
 * @assistant:
 *
 * TODO: Write me
 */
void 
cong_document_factory_invoke_action_callback(CongServiceDocumentFactory *factory, CongNewFileAssistant *assistant)
{
	g_return_if_fail (IS_CONG_SERVICE_DOCUMENT_FACTORY (factory));
	g_return_if_fail (assistant);

#if 0
	g_message("invoking action for document factory \"%s\"", cong_service_get_name(CONG_SERVICE(factory)));
#endif

	g_assert(PRIVATE (factory)->action_callback);

	PRIVATE (factory)->action_callback(factory, assistant, PRIVATE (factory)->user_data);
}

/**
 * cong_document_factory_get_icon:
 * @factory:
 *
 * TODO: Write me
 * Returns:
 */
GdkPixbuf *
cong_document_factory_get_icon(CongServiceDocumentFactory *factory)
{
	g_return_val_if_fail (IS_CONG_SERVICE_DOCUMENT_FACTORY (factory), NULL);

	if (PRIVATE (factory)->icon16) {
		g_object_ref(G_OBJECT(PRIVATE (factory)->icon16));
		return PRIVATE (factory)->icon16;
	} else {
		return NULL;
	}
}
