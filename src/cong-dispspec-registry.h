/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-dispspec-registry.h
 *
 * Copyright (C) 2002 David Malcolm
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

#ifndef __CONG_DISPSPEC_REGISTRY_H__
#define __CONG_DISPSPEC_REGISTRY_H__

G_BEGIN_DECLS

/* cong-dispspec-registry */
CongDispspecRegistry*
cong_dispspec_registry_new(const gchar* xds_directory, GtkWindow *toplevel_window);

void
cong_dispspec_registry_add_dir(CongDispspecRegistry *registry, const gchar *xds_directory, GtkWindow *toplevel_window, gboolean raise_errs);

void
cong_dispspec_registry_free(CongDispspecRegistry* registry);

unsigned int
cong_dispspec_registry_get_num(CongDispspecRegistry* registry);

CongDispspec*
cong_dispspec_registry_get(CongDispspecRegistry* registry, unsigned int i);

void
cong_dispspec_registry_add(CongDispspecRegistry* registry, CongDispspec* ds);

void
cong_dispspec_registry_dump(CongDispspecRegistry* registry);

CongDispspec*
cong_dispspec_registry_get_appropriate_dispspec (CongDispspecRegistry* registry, 
						 xmlDocPtr doc,
						 const gchar *filename_extension);

CongDispspec*
cong_dispspec_registry_get_dispspec_for_dtd (CongDispspecRegistry* registry, 
					     xmlDocPtr xml_doc);					     

CongDispspecElement*
cong_dispspec_registry_get_dispspec_element_for_node (CongDispspecRegistry *registry, 
						      CongDispspec *default_ds, 
						      CongNodePtr node);


CongDispspecElement*
cong_dispspec_registry_get_dispspec_element_for_description (CongDispspecRegistry *registry, 
							     const CongElementDescription *desc,
							     const CongDispspec *default_ds);
G_END_DECLS

#endif
