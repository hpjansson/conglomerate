/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service.c
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
#include "cong-service.h"
#include "cong-plugin.h"

struct CongServicePrivate
{
	gchar *name;
	gchar *description;
	gchar *service_id;
};

CONG_DEFINE_CLASS_BEGIN (CongService, cong_service, CONG_SERVICE, GObject, G_TYPE_OBJECT)
CONG_DEFINE_CLASS_END ()
CONG_DEFINE_EMPTY_DISPOSE(cong_service)


/* Implementation of CongService: */
/**
 * cong_service_construct:
 * @service:
 * @name:
 * @description:
 * @service_id:
 *
 * TODO: Write me
 * Returns:
 */
CongService*
cong_service_construct (CongService* service,
			const gchar *name, 
			const gchar *description,
			const gchar *service_id)
{
	g_return_val_if_fail (IS_CONG_SERVICE (service), NULL);
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (description, NULL);
	g_return_val_if_fail (service_id, NULL);

	PRIVATE (service)->name = g_strdup (name);
	PRIVATE (service)->description = g_strdup (description);
	PRIVATE (service)->service_id = g_strdup (service_id);

	return service;
}

/**
 * cong_service_get_name:
 * @service:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_service_get_name (CongService *service)
{
	g_return_val_if_fail (IS_CONG_SERVICE (service), NULL);
	
	return PRIVATE (service)->name;
}

/**
 * cong_service_get_description:
 * @service:
 *
 * TODO: Write me
 * Returns:
 */
const gchar* 
cong_service_get_description (CongService *service)
{
	g_return_val_if_fail (IS_CONG_SERVICE (service), NULL);

	return PRIVATE (service)->description;
}

/**
 * cong_service_get_id:
 * @service:
 *
 * TODO: Write me
 * Returns:
 */
const gchar* 
cong_service_get_id (CongService *service)
{
	g_return_val_if_fail (IS_CONG_SERVICE (service), NULL);

	return PRIVATE (service)->service_id;
}

/**
 * cong_service_get_gconf_namespace:
 * @service:
 *
 * TODO: Write me
 * Returns:
 */
gchar* 
cong_service_get_gconf_namespace (CongService* service)
{
	g_return_val_if_fail (IS_CONG_SERVICE (service), NULL);

	g_assert (PRIVATE (service)->service_id);

	return g_strdup_printf (CONG_GCONF_PATH "services/%s", PRIVATE (service)->service_id);
}

/**
 * cong_service_get_gconf_key:
 * @service:
 * @local_part:
 * 
 * Convert a "local" GConf key for this plugin to a GConf key with a full-path.
 * e.g. converts "enable-fubar" to "/apps/conglomerate/plugins/docbook/enable-fubar"
 *   
 * Caller must delete returned string.
 * 
 * Returns:
 */
gchar* 
cong_service_get_gconf_key (CongService *service, 
			    const gchar *local_part)
{
	g_return_val_if_fail (IS_CONG_SERVICE (service), NULL);
	g_return_val_if_fail (local_part, NULL);

	g_assert (PRIVATE (service)->service_id);

	return g_strdup_printf (CONG_GCONF_PATH "services/%s/%s", PRIVATE (service)->service_id, local_part);
}
