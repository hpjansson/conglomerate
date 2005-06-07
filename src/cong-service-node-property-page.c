/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-node-property-page.c
 *
 * Copyright (C) 2005 David Malcolm
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
#include "cong-service-node-property-page.h"

struct CongServiceNodePropertyPagePrivate
{
	CongCustomPropertyPageFactoryMethod factory_method;
	gpointer user_data;
};

CONG_DEFINE_CLASS (CongServiceNodePropertyPage, cong_service_node_property_page, CONG_SERVICE_NODE_PROPERTY_PAGE, CongService, CONG_SERVICE_TYPE)

/**
 * cong_service_node_property_page_construct:
 * @node_property_page:
 * @name:
 * @description:
 * @service_id:
 * @factory_method:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceNodePropertyPage*
cong_service_node_property_page_construct (CongServiceNodePropertyPage* node_property_page,
					     const gchar *name, 
					     const gchar *description,
					     const gchar *service_id,
					     CongCustomPropertyPageFactoryMethod factory_method,
					     gpointer user_data)
{
	g_return_val_if_fail (IS_CONG_SERVICE_NODE_PROPERTY_PAGE (node_property_page), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);

	cong_service_construct (CONG_SERVICE (node_property_page),
				name,
				description,
				service_id);
	PRIVATE (node_property_page)->factory_method = factory_method;
	PRIVATE (node_property_page)->user_data = user_data;

	return node_property_page;
}

/**
 * cong_custom_property_page_make:
 * @custom_property_page:
 * @doc:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget *
cong_custom_property_page_make (CongServiceNodePropertyPage *node_property_page,
			        CongDocument *doc)
{
	g_return_val_if_fail (IS_CONG_SERVICE_NODE_PROPERTY_PAGE (node_property_page), NULL);
	g_return_val_if_fail (doc, NULL);

	g_assert (PRIVATE (node_property_page)->factory_method);

	return PRIVATE (node_property_page)->factory_method (node_property_page, 
							     doc);
}
