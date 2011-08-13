/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "global.h"
#include "cong-app.h"
#include "cong-primary-window.h"

int 
main( int   argc,
      char *argv[] )
{
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	cong_app_construct_singleton (argc, 
				      argv);

	/* This code requires the app singleton ptr to be set: */
	cong_primary_window_new(NULL);

	/* Various things require that a primary window exists: */
	if (cong_app_post_init_hack (cong_app_singleton())) {
		return 1;
	}

	/* The main loop: */
	gtk_main();

	/* Cleanup: */
	cong_app_destroy_singleton();

	return(0);
}
