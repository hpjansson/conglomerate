/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service.h
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

#ifndef __CONG_SERVICE_H__
#define __CONG_SERVICE_H__

#include "cong-object.h"

G_BEGIN_DECLS

#define CONG_SERVICE_TYPE	  (cong_service_get_type ())
#define CONG_SERVICE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_SERVICE_TYPE, CongService)
#define CONG_SERVICE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_SERVICE_TYPE, CongServiceClass)
#define IS_CONG_SERVICE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_SERVICE_TYPE)

CONG_DECLARE_CLASS (CongService, cong_service, GObject)

CongService*
cong_service_construct (CongService* service,
			const gchar *name, 
			const gchar *description,
			const gchar *service_id);

const gchar* 
cong_service_get_name (CongService *service);

const gchar* 
cong_service_get_description (CongService *service);

const gchar* 
cong_service_get_id (CongService *service);

gchar* 
cong_service_get_gconf_namespace (CongService* service);

gchar* 
cong_service_get_gconf_key (CongService *service, 
			    const gchar *local_part);

G_END_DECLS

#endif



