/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "global.h"
#include "cong-app.h"
#include "cong-primary-window.h"

#include "cong-ui-hooks.h"

/*
#define AUTOGENERATE_DS
*/

/**
 * status_update:
 *
 * TODO: Write me
 */
void 
status_update()
{
  while (g_main_iteration(FALSE));
}


#if 0
static gint popup_deactivate(GtkWidget *widget, GdkEvent *event)
{
	return(TRUE);
}
#endif

int 
main( int   argc,
      char *argv[] )
{
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

#if 0
	g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL);
#endif

	cong_app_construct_singleton (argc, 
				      argv);

	/* This code requires the app singleton ptr to be set: */
	cong_primary_window_new(NULL);

	/* Various things require that a primary window exists: */
	if (cong_app_post_init_hack (cong_app_singleton())) {
		return 1;
	}

#if 1
	/* 
	   If we have any arguments, interpret the final one as the name of a file to be opened: 

	   FIXME: should we use popt?
	*/
	if (argc > 1) {
		open_document_do(argv[argc-1], NULL);
	}
#endif

	/* The main loop: */
	gtk_main();

	/* Cleanup: */
	cong_app_destroy_singleton();

	return(0);
}
