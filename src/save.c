#include <gtk/gtk.h>
#include <ttree.h>
#include <xml.h>
#include "global.h"


gint save_document(GtkWidget *w, gpointer data)
{
	char *doc_name, *ds_name, *p;
	TTREE *ds_temp, *xml_in;
	FILE *xml_f;

	doc_name = get_file_name("Save XML as...");
	if (!doc_name) return(TRUE);

	xml_f = fopen(doc_name, "wt");
	if (!xml_f) return(TRUE);

	xml_t_to_f(xv->x, xml_f);
	fclose(xml_f);
	
	return(TRUE);
}
