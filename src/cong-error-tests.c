/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gtk/gtk.h>

#include "global.h"
#include "cong-error-dialog.h"

void cong_error_test_file_ops(const gchar* filename, GnomeVFSResult vfs_result)
{
	GtkDialog* dialog;

	GnomeVFSURI* vfs_uri = gnome_vfs_uri_new(filename);
	GnomeVFSFileSize file_size = 5*1024*1024; /* 5 megabytes, for a test */

	g_message("Testing File->Open failure of \"%s\" with result %s\n", filename, gnome_vfs_result_to_string(vfs_result));

	dialog = cong_error_dialog_new_file_open_failed_from_vfs_result(vfs_uri, vfs_result);
	cong_error_dialog_run(GTK_DIALOG(dialog));		
	gtk_widget_destroy(GTK_WIDGET(dialog));    

	g_message("Testing File->Save failure of \"%s\" with result %s\n", filename, gnome_vfs_result_to_string(vfs_result));

	dialog = cong_error_dialog_new_file_save_failed(vfs_uri, vfs_result, &file_size);
	cong_error_dialog_run(GTK_DIALOG(dialog));		
	gtk_widget_destroy(GTK_WIDGET(dialog));    

	gnome_vfs_uri_unref(vfs_uri);
}

void cong_error_tests(void)
{
	int i;
	for (i=(int)GNOME_VFS_ERROR_NOT_FOUND;i<(int)GNOME_VFS_NUM_ERRORS;i++) {

		#define TEST_URI ("file:///home/david/test.xml")
		// #define TEST_URI ("file:///home/david/fubar.xml")
		// #define TEST_URI ("file:///home/david/faq.html")
		// #define TEST_URI ("http://www.fubar.com/fubar.xml")
		// #define TEST_URI ("ftp://www.fubar.com/fubar.xml")


		cong_error_test_file_ops(TEST_URI, (GnomeVFSResult)i);
	}
}
