/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gtk/gtk.h>

#include "global.h"

void cong_error_tests(void)
{
	GtkWidget* dialog = cong_error_dialog_new(
#if 1
						  "app-name could not write \"filename\" to path",
#else
						  "<replaceable>app-name</replaceable> could not write <replaceable>filename</replaceable> to <replaceable>path</replaceable>",
#endif
						  "The file may be being accessed by another application or by a system task.",
						  "Wait a few seconds and then try again.  If that fails, try closing other applications using this file, or try saving to another location.");
	/* It doesn't work if we have unrecognised tags in the Pango markup */

	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);

}
