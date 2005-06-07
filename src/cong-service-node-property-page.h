/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-node-property-dialog.h
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

#ifndef __CONG_SERVICE_NODE_PROPERTY_PAGE_H__
#define __CONG_SERVICE_NODE_PROPERTY_PAGE_H__

#include "cong-plugin.h"
#include <gtk/gtkwidget.h>
#include "cong-document.h"

G_BEGIN_DECLS

#define CONG_SERVICE_NODE_PROPERTY_PAGE_TYPE	  (cong_service_node_property_page_get_type ())
#define CONG_SERVICE_NODE_PROPERTY_PAGE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_SERVICE_NODE_PROPERTY_PAGE_TYPE, CongServiceNodePropertyPage)
#define CONG_SERVICE_NODE_PROPERTY_PAGE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_SERVICE_NODE_PROPERTY_PAGE_TYPE, CongServiceNodePropertyPageClass)
#define IS_CONG_SERVICE_NODE_PROPERTY_PAGE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_SERVICE_NODE_PROPERTY_PAGE_TYPE)
CONG_DECLARE_CLASS (CongServiceNodePropertyPage, cong_service_node_property_page, CongService)

typedef GtkWidget* 
(*CongCustomPropertyPageFactoryMethod) (CongServiceNodePropertyPage *custom_property_page, 
					CongDocument *doc); 

CongServiceNodePropertyPage*
cong_service_node_property_page_construct (CongServiceNodePropertyPage* node_property_page,
					   const gchar *name, 
					   const gchar *description,
					   const gchar *service_id,
					   CongCustomPropertyPageFactoryMethod factory_method,
					   gpointer user_data);

CongServiceNodePropertyPage*
cong_plugin_register_custom_property_page (CongPlugin *plugin,
					   const gchar *name, 
					   const gchar *description,
					   const gchar *service_id,
					   CongCustomPropertyPageFactoryMethod factory_method,
					   gpointer user_data);

/* 
   Utility to reduce the number of strings needing translation (addressing bug #124780); this
   is a wrapper around cong_plugin_register_custom_property_page 
*/
CongServiceNodePropertyPage*
cong_plugin_register_custom_property_page_for_element (CongPlugin *plugin,
						       const gchar *element_name,
						       const gchar *service_id,
						       CongCustomPropertyPageFactoryMethod factory_method,
						       gpointer user_data);

void 
cong_plugin_for_each_custom_property_page (CongPlugin *plugin, 
					   void 
					   (*callback) (CongServiceNodePropertyPage *custom_property_page, 
							gpointer user_data), 
					   gpointer user_data);

GtkWidget*
cong_custom_property_page_make (CongServiceNodePropertyPage *custom_property_page,
				CongDocument *doc);

G_END_DECLS

#endif



