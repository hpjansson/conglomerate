/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-node-property-dialog.c
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
#include "cong-service-node-property-dialog.h"

struct CongServiceNodePropertyDialogPrivate
{
	CongCustomPropertyFactoryMethod factory_method;
	gpointer user_data;
};

CONG_DEFINE_CLASS (CongServiceNodePropertyDialog, cong_service_node_property_dialog, CONG_SERVICE_NODE_PROPERTY_DIALOG, CongService, CONG_SERVICE_TYPE)

/**
 * cong_service_node_property_dialog_construct:
 * @node_property_dialog:
 * @name:
 * @description:
 * @service_id:
 * @factory_method:
 * @user_data:
 *
 * TODO: Write me
 * Returns:
 */
CongServiceNodePropertyDialog*
cong_service_node_property_dialog_construct (CongServiceNodePropertyDialog* node_property_dialog,
					     const gchar *name, 
					     const gchar *description,
					     const gchar *service_id,
					     CongCustomPropertyFactoryMethod factory_method,
					     gpointer user_data)
{
	g_return_val_if_fail (IS_CONG_SERVICE_NODE_PROPERTY_DIALOG (node_property_dialog), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);

	cong_service_construct (CONG_SERVICE (node_property_dialog),
				name,
				description,
				service_id);
	PRIVATE (node_property_dialog)->factory_method = factory_method;
	PRIVATE (node_property_dialog)->user_data = user_data;

	return node_property_dialog;
}

/**
 * cong_custom_property_dialog_make:
 * @custom_property_dialog:
 * @doc:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
GtkWidget *
cong_custom_property_dialog_make(CongServiceNodePropertyDialog *node_property_dialog,
				 CongDocument *doc,
				 CongNodePtr node)
{
	g_return_val_if_fail (IS_CONG_SERVICE_NODE_PROPERTY_DIALOG (node_property_dialog), NULL);
	g_return_val_if_fail (doc, NULL);
	g_return_val_if_fail (node, NULL);

	g_assert (PRIVATE (node_property_dialog)->factory_method);

	return PRIVATE (node_property_dialog)->factory_method (node_property_dialog, 
							       doc, 
							       node);
}
