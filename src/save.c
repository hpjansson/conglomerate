#include <gtk/gtk.h>

#include "global.h"


gint save_document(GtkWidget *w, gpointer data)
{
	const char *doc_name;
	UNUSED_VAR(char *ds_name)
	UNUSED_VAR(char *p)

	doc_name = get_file_name("Save XML as...");
	if (!doc_name) return(TRUE);

	cong_document_save(the_globals.xv->doc, doc_name);

	return(TRUE);
}
