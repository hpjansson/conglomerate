/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-service-document-factory.h
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

#ifndef __CONG_SERVICE_DOCUMENT_FACTORY_H__
#define __CONG_SERVICE_DOCUMENT_FACTORY_H__

#include "cong-service.h"

G_BEGIN_DECLS

/* The File->New GUI: */
typedef struct CongNewFileAssistant CongNewFileAssistant;

#define CONG_SERVICE_DOCUMENT_FACTORY_TYPE	  (cong_service_document_factory_get_type ())
#define CONG_SERVICE_DOCUMENT_FACTORY(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_SERVICE_DOCUMENT_FACTORY_TYPE, CongServiceDocumentFactory)
#define CONG_SERVICE_DOCUMENT_FACTORY_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_SERVICE_DOCUMENT_FACTORY_TYPE, CongServiceDocumentFactoryClass)
#define IS_CONG_SERVICE_DOCUMENT_FACTORY(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_SERVICE_DOCUMENT_FACTORY_TYPE)

CONG_DECLARE_CLASS (CongServiceDocumentFactory, cong_service_document_factory, CongService)

/* Function pointers that are registered by plugins: */
typedef void 
(*CongServiceDocumentFactoryPageCreationCallback) (CongServiceDocumentFactory *factory, 
						   CongNewFileAssistant *assistant, 
						   gpointer user_data);

typedef void 
(*CongServiceDocumentFactoryActionCallback) (CongServiceDocumentFactory *factory, 
					     CongNewFileAssistant *assistant, 
					     gpointer user_data);

CongServiceDocumentFactory*
cong_service_document_factory_construct (CongServiceDocumentFactory* factory,
					 const gchar *name, 
					 const gchar *description,
					 const gchar *id,
					 CongServiceDocumentFactoryPageCreationCallback page_creation_callback,
					 CongServiceDocumentFactoryActionCallback action_callback,
					 const gchar *icon,
					 gpointer user_data);

CongServiceDocumentFactory*
cong_plugin_register_document_factory (CongPlugin *plugin, 
				       const gchar *name, 
				       const gchar *description,
				       const gchar *id,
				       CongServiceDocumentFactoryPageCreationCallback page_creation_callback,
				       CongServiceDocumentFactoryActionCallback action_callback,
				       const gchar *icon,
				       gpointer user_data);

void
cong_document_factory_invoke_page_creation_callback (CongServiceDocumentFactory *factory, 
						     CongNewFileAssistant *assistant);
void
cong_document_factory_invoke_action_callback (CongServiceDocumentFactory *factory, 
					      CongNewFileAssistant *assistant);
GdkPixbuf*
cong_document_factory_get_icon (CongServiceDocumentFactory *factory);

void 
cong_plugin_for_each_document_factory (CongPlugin *plugin, 
				       void 
				       (*callback) (CongServiceDocumentFactory *factory, 
						    gpointer user_data), 
				       gpointer user_data);

/* The File->New GUI: */

/* The DocumentFactory objects all create pages within one big Druid; the booleans provide hints to make
   navigation easier */
GnomeDruidPageStandard*
cong_new_file_assistant_new_page (CongNewFileAssistant *assistant, 
				  CongServiceDocumentFactory *document_factory, 
				  gboolean is_first_of_factory,
				  gboolean is_last_of_factory);
void 
cong_new_file_assistant_set_page (CongNewFileAssistant *assistant, 
				  GnomeDruidPage *page);

/* Method to get toplevel window of the assistant; useful when displaying error dialogs */
GtkWindow*
cong_new_file_assistant_get_toplevel (CongNewFileAssistant *assistant);

/* A way to associate data with a factory for a particular instance of the assistant; 
   useful for storing custom pages of the GUI for retrieval during the action callback */
void
cong_new_file_assistant_set_data_for_factory (CongNewFileAssistant *assistant,
					      CongServiceDocumentFactory *document_factory,
					      gpointer factory_data,
					      void (*free_func) (gpointer factory_data));

gpointer
cong_new_file_assistant_get_data_for_factory (CongNewFileAssistant *assistant,
					      CongServiceDocumentFactory *document_factory);


G_END_DECLS

#endif



