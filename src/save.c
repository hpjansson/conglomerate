#include <gtk/gtk.h>
#include <ttree.h>
#include <xml.h>

#include "global.h"


gint save_document(GtkWidget *w, gpointer data)
{
	const char *doc_name;
	UNUSED_VAR(char *ds_name)
	UNUSED_VAR(char *p)
	UNUSED_VAR(TTREE *ds_temp)
	UNUSED_VAR(TTREE *xml_in)
	FILE *xml_f;

	doc_name = get_file_name("Save XML as...");
	if (!doc_name) return(TRUE);

	xml_f = fopen(doc_name, "wt");
	if (!xml_f) return(TRUE);

	xml_t_to_f(the_globals.xv->x, xml_f);
	fclose(xml_f);
	
	return(TRUE);
}
