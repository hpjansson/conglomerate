/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-app.c
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

#include "global.h"
#include "cong-app.h"

#define PRIVATE(x) ((x)->private)

/* Internal data structure declarations: */
struct CongAppPrivate
{
	gchar *clipboard; /* can be NULL to signify nothing in clipboard*/
};


CongApp *the_singleton_app = NULL;

/* Internal function declarations: */
static CongApp*
cong_app_new(void);

static void
cong_app_free(CongApp *app);

/* Exported function definitions: */
CongApp*
cong_app_singleton(void)
{
	g_assert(the_singleton_app);

	return the_singleton_app;
}

void
cong_app_construct_singleton(void)
{
	g_assert(NULL == the_singleton_app);

	the_singleton_app = cong_app_new();
}

void
cong_app_destroy_singleton(void)
{
	g_assert(the_singleton_app);

	cong_app_free(the_singleton_app);

	the_singleton_app = NULL;
}


const gchar*
cong_app_get_clipboard (CongApp *app)
{
	g_return_val_if_fail (app, NULL);

	return PRIVATE(app)->clipboard;
}

void
cong_app_set_clipboard (CongApp *app, 
			const gchar* text)
{
	g_return_if_fail (app);
	/* text is allowed to be NULL */

	if (PRIVATE(app)->clipboard) {
		g_free(PRIVATE(app)->clipboard);
	}

	if (text) {
		PRIVATE(app)->clipboard = g_strdup(text);
	} else {
		PRIVATE(app)->clipboard = NULL;
	}

	g_message("Clipboard set to \"%s\"", text);

	/* emit signals? */
}

/* Internal function definitions: */
static CongApp*
cong_app_new(void)
{
	CongApp *app;

	app = g_new0(CongApp,1);
	app->private = g_new0(CongAppPrivate,1);


	/* Set up usage of GConf: */
	app->gconf_client = gconf_client_get_default();
	gconf_client_add_dir (app->gconf_client,
			      "/apps/conglomerate",
			      GCONF_CLIENT_PRELOAD_NONE,
			      NULL);

	return app;	
}

static void
cong_app_free(CongApp *app)
{
	g_return_if_fail (app);

	g_free(app->private);
	g_free(app);
}
