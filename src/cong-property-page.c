/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-property-page.c
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
#include <glib.h>
#include "cong-property-page.h"
#include "cong-eel.h"
#include "cong-util.h"
#include "cong-command.h"

#define PRIVATE(x) ((x)->private)

/* Internal types: */
struct _CongPropertyPageDetails
{
	CongDocument *doc;
	gulong handler_id_selection_change;
};

/* Internal function declarations: */
static void
finalize (GObject *object);

static void
dispose (GObject *object);

static void
on_selection_change (CongDocument *doc, 
		     CongPropertyPage *property_page);

/* Exported function definitions: */
G_DEFINE_TYPE(CongPropertyPage,
              cong_property_page,
              GTK_TYPE_HBOX);

CONG_EEL_IMPLEMENT_MUST_OVERRIDE_SIGNAL (cong_property_page, selection_change_handler);

static void
cong_property_page_class_init (CongPropertyPageClass *klass)
{
	CONG_EEL_ASSIGN_MUST_OVERRIDE_SIGNAL (klass,
					      cong_property_page,
					      selection_change_handler);
	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;
}

static void
cong_property_page_init (CongPropertyPage *property_page)
{
	property_page->private = g_new0(CongPropertyPageDetails,1);
}

/**
 * cong_property_page_construct:
 * @property_page:
 * @doc:
 * @node:
 * @ns_ptr:
 * @property_name:
 *
 * TODO: Write me
 * Returns:
 */
CongPropertyPage*
cong_property_page_construct (CongPropertyPage *property_page,
			      CongDocument *doc)
{
	g_return_val_if_fail (IS_CONG_PROPERTY_PAGE(property_page), NULL);

	PRIVATE(property_page)->doc = doc;
	g_object_ref(doc);

	PRIVATE(property_page)->handler_id_selection_change = g_signal_connect_after (G_OBJECT(doc),
										      "selection_change",
										      G_CALLBACK(on_selection_change),
										      property_page);
	return CONG_PROPERTY_PAGE (property_page);
}

/**
 * cong_property_page_get_document:
 * @property_page:
 *
 * TODO: Write me
 * Returns:
 */
CongDocument*
cong_property_page_get_document (CongPropertyPage *property_page)
{
	g_return_val_if_fail (IS_CONG_PROPERTY_PAGE(property_page), NULL);

	return PRIVATE(property_page)->doc; 
}

/* Internal function definitions: */
static void
finalize (GObject *object)
{
	CongPropertyPage *property_page = CONG_PROPERTY_PAGE(object);
	
	g_free (property_page->private);
	property_page->private = NULL;
	
	G_OBJECT_CLASS (cong_property_page_parent_class)->finalize (object);
}

static void
dispose (GObject *object)
{
	CongPropertyPage *property_page = CONG_PROPERTY_PAGE(object);

	if (PRIVATE(property_page)->doc) {
	
		g_signal_handler_disconnect (G_OBJECT (PRIVATE(property_page)->doc),
					     PRIVATE(property_page)->handler_id_selection_change);
		PRIVATE(property_page)->handler_id_selection_change = 0;

		g_object_unref (G_OBJECT (PRIVATE(property_page)->doc));
		PRIVATE(property_page)->doc = NULL;
	}
		
	G_OBJECT_CLASS (cong_property_page_parent_class)->dispose (object);
}

static void
on_selection_change (CongDocument *doc, 
		     CongPropertyPage *property_page)
{
	g_return_if_fail (IS_CONG_PROPERTY_PAGE(property_page));

	CONG_EEL_CALL_METHOD (CONG_PROPERTY_PAGE_CLASS,
			      property_page,
			      selection_change_handler, 
			      (property_page));
}
