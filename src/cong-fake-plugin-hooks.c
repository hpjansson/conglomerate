/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-fake-plugin-hooks.c
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
#include "cong-plugin.h"
#include "cong-plugin-manager.h"
#include "cong-fake-plugin-hooks.h"


static void 
register_plugin (CongApp *app,
		 const gchar *id,
		 CongPluginCallbackRegister register_callback,
		 CongPluginCallbackConfigure configure_callback)
{
	g_return_if_fail (id);
	g_return_if_fail (register_callback);

	g_assert (cong_app_get_plugin_manager (app));

	cong_plugin_manager_register (cong_app_get_plugin_manager (app), 
				      id,
				      register_callback, 
				      configure_callback);
}

void
cong_fake_plugin_hook_register_the_whole_shebang (CongApp *app)
{
	/* For the moment, there aren't any actual plugins; instead we fake it. */
	register_plugin (app,"abiword",
			 plugin_abiword_plugin_register,
			 plugin_abiword_plugin_configure);

	register_plugin (app,"admonition",
			 plugin_admonition_plugin_register,
			 plugin_admonition_plugin_configure);

	register_plugin (app,"arbitrary",
			 plugin_arbitrary_plugin_register,
			 plugin_arbitrary_plugin_configure);

	register_plugin (app,"css",
			 plugin_css_plugin_register,
			 plugin_css_plugin_configure);

	register_plugin (app,"debug",
			 plugin_debug_plugin_register,
			 plugin_debug_plugin_configure);

	register_plugin (app,"docbook",
			 plugin_docbook_plugin_register,
			 plugin_docbook_plugin_configure);

	register_plugin (app,"empty",
			 plugin_empty_plugin_register,
			 plugin_empty_plugin_configure);

	register_plugin (app,"fo",
			 plugin_fo_plugin_register,
			 plugin_fo_plugin_configure);

	register_plugin (app,"html",
			 plugin_html_plugin_register,
			 plugin_html_plugin_configure);

	register_plugin (app,"lists",
			 plugin_lists_plugin_register,
			 plugin_lists_plugin_configure);

	register_plugin (app,"random",
			 plugin_random_plugin_register,
			 plugin_random_plugin_configure);

	register_plugin (app,"relaxng",
			 plugin_relaxng_plugin_register,
			 plugin_relaxng_plugin_configure);

	register_plugin (app,"sgml",
			 plugin_sgml_plugin_register,
			 plugin_sgml_plugin_configure);

	register_plugin (app,"table",
			 plugin_table_plugin_register,
			 plugin_table_plugin_configure);

	register_plugin (app,"tei",
			 plugin_tei_plugin_register,
			 plugin_tei_plugin_configure);

	register_plugin (app,"tests",
			 plugin_tests_plugin_register,
			 plugin_tests_plugin_configure);

	register_plugin (app,"validate",
			 plugin_validate_plugin_register,
			 plugin_validate_plugin_configure);

	register_plugin (app,"website",
			 plugin_website_plugin_register,
			 plugin_website_plugin_configure);

	register_plugin (app,"xsl",
			 plugin_xsl_plugin_register,
			 plugin_xsl_plugin_configure);

	register_plugin (app,"convert-case",
			 plugin_convert_case_plugin_register,
			 plugin_convert_case_plugin_configure);

	register_plugin (app,"cleanup-source",
			 plugin_cleanup_source_plugin_register,
			 plugin_cleanup_source_plugin_configure);

	register_plugin (app,"dtd",
			 plugin_dtd_plugin_register,
			 plugin_dtd_plugin_configure);

	register_plugin (app,"paragraph",
			 plugin_paragraph_plugin_register,
			 plugin_paragraph_plugin_configure);

	register_plugin (app,"save-dispspec",
			 plugin_save_dispspec_plugin_register,
			 plugin_save_dispspec_plugin_configure);

	register_plugin (app,"templates",
			 plugin_templates_plugin_register,
			 plugin_templates_plugin_configure);
}

