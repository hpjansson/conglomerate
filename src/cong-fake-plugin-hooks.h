/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-fake-plugin-hooks.h
 *
 * Copyright (C) 2003 David Malcolm
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

#ifndef __CONG_FAKE_PLUGIN_HOOKS_H__
#define __CONG_FAKE_PLUGIN_HOOKS_H__

G_BEGIN_DECLS

/* Plugins at the moment are all compiled into the app; here are the symbols that would be dynamically extracted: */
/* plugin-cleanup-source.c: */
gboolean plugin_cleanup_source_plugin_register(CongPlugin *plugin);
gboolean plugin_cleanup_source_plugin_configure(CongPlugin *plugin);

/* plugin-convert-case.c: */
gboolean plugin_convert_case_plugin_register(CongPlugin *plugin);
gboolean plugin_convert_case_plugin_configure(CongPlugin *plugin);

/* plugin-docbook.c: */
gboolean plugin_docbook_plugin_register(CongPlugin *plugin);
gboolean plugin_docbook_plugin_configure(CongPlugin *plugin);

/* plugin-dtd.c: */
gboolean plugin_dtd_plugin_register(CongPlugin *plugin);
gboolean plugin_dtd_plugin_configure(CongPlugin *plugin);

/* plugin-empty.c: */
gboolean plugin_empty_plugin_register(CongPlugin *plugin);
gboolean plugin_empty_plugin_configure(CongPlugin *plugin);

/* plugin-fo.c: */
gboolean plugin_fo_plugin_register(CongPlugin *plugin);
gboolean plugin_fo_plugin_configure(CongPlugin *plugin);

/* plugin-lists.c: */
gboolean plugin_lists_plugin_register(CongPlugin *plugin);
gboolean plugin_lists_plugin_configure(CongPlugin *plugin);

/* plugin-sgml.c: */
gboolean plugin_sgml_plugin_register(CongPlugin *plugin);
gboolean plugin_sgml_plugin_configure(CongPlugin *plugin);

/* plugin-tests.c: */
gboolean plugin_tests_plugin_register(CongPlugin *plugin);
gboolean plugin_tests_plugin_configure(CongPlugin *plugin);

/* plugin-validate.c: */
gboolean plugin_validate_plugin_register(CongPlugin *plugin);
gboolean plugin_validate_plugin_configure(CongPlugin *plugin);

/* plugin-website.c: */
gboolean plugin_website_plugin_register(CongPlugin *plugin);
gboolean plugin_website_plugin_configure(CongPlugin *plugin);

/* plugin-xsl.c: */
gboolean plugin_xsl_plugin_register(CongPlugin *plugin);
gboolean plugin_xsl_plugin_configure(CongPlugin *plugin);

/* more plugins please! */

G_END_DECLS

#endif
