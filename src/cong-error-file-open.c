/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gtk/gtk.h>

#include "global.h"

void
cong_error_split_uri(const gchar* uri, gchar** filename_alone, gchar** path)
{
	g_return_if_fail(uri);
	g_return_if_fail(filename_alone);
	g_return_if_fail(path);

	g_message("stub implementation of cong_error_split_uri called\n");

	*filename_alone=g_strdup("fubar.xml");
	*path=g_strdup("/some_location/some_subdir");
}

GtkWidget*
cong_error_dialog_new_file_open_failed(const gchar* filename)
{
	/* should the filename be a URI? */

	GtkWidget* dialog;
       	gchar* what_failed;
	gchar* filename_alone;
	gchar* path;

	g_return_val_if_fail(filename, NULL);

	cong_error_split_uri(filename, &filename_alone, &path);

	g_assert(filename_alone);
	g_assert(path);

	what_failed = g_strdup_printf("%s cannot find \"%s\" at %s.","Conglomerate", filename_alone, path);

	g_free(filename_alone);
	g_free(path);

	dialog = cong_error_dialog_new(what_failed,
				       
				       "Either a file with that name is not present at the location specified, or the location does not exist",
				       /* FIXME:  isolate all these cases and handle separately.  Also deal separately with permission errors etc */

				       "(i) Try checking that you spelt the file's name correctly.  Remember that capitalisation is significant (\"MyFile\" is not the same as \"MYFILE\" or \"myfile\").\n"
				       "(ii) Try using the GNOME Search Tool to find your file.");


	g_free(what_failed);

	return dialog;
}
