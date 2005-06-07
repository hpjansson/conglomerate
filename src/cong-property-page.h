/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-property-page.h
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

#ifndef __CONG_PROPERTY_PAGE_H__
#define __CONG_PROPERTY_PAGE_H__

#include "cong-document.h"
#include <glade/glade.h>

G_BEGIN_DECLS

typedef struct CongPropertyPage CongPropertyPage;
typedef struct CongPropertyPageClass CongPropertyPageClass;
typedef struct CongPropertyPageDetails CongPropertyPageDetails;

#define CONG_PROPERTY_PAGE_TYPE	      (cong_property_page_get_type ())
#define CONG_PROPERTY_PAGE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_PROPERTY_PAGE_TYPE, CongPropertyPage)
#define CONG_PROPERTY_PAGE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_PROPERTY_PAGE_TYPE, CongPropertyPageClass)
#define IS_CONG_PROPERTY_PAGE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_PROPERTY_PAGE_TYPE)

struct CongPropertyPage
{
	GtkHBox hbox;

	CongPropertyPageDetails *private;
};

struct CongPropertyPageClass
{
	GtkHBoxClass klass;

	void (*selection_change_handler) (CongPropertyPage *property_page);
};

GType
cong_property_page_get_type (void);

CongPropertyPage*
cong_property_page_construct (CongPropertyPage *property_page,
			      CongDocument *doc);

CongDocument*
cong_property_page_get_document (CongPropertyPage *property_page);

/* Macros for creating subclasses: */
#define CONG_PROPERTY_PAGE_BOILERPLATE(StudlyName, lower_name) \
GNOME_CLASS_BOILERPLATE(CongPropertyPage##StudlyName,    \
			cong_property_page_##lower_name, \
			CongPropertyPage,                \
			CONG_PROPERTY_PAGE_TYPE);


G_END_DECLS


#endif
