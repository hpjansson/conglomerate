#include <string.h>
#include <gtk/gtk.h>
#include <ttree.h>
#include <xml.h>
#include "global.h"


int open_document_do(char *doc_name, char *ds_name)
{
	char *p;
	TTREE *ds_temp, *xml_in;
	FILE *xml_f;

	ds_temp = ttree_load(ds_name);
	if (!ds_temp) return(TRUE);  /* Invalid displayspec. */

	xml_f = fopen(doc_name, "rt");
	if (!xml_f) return(TRUE);

	p = strrchr(doc_name, '/');
	if (p)
	{
		*p = 0;
		chdir(doc_name);
	}
	
	xml_in = xml_f_to_ttree(xml_f, 0);
	if (!xml_in) return(TRUE);  /* Invalid XML document. */

	fclose(xml_f);
	
	ds_global = ds_temp;
	ds_init(ds_global);

	xml_t_trim(xml_in);
  xv = xmlview_new(xml_in, ds_global);
	gtk_box_pack_start(GTK_BOX(root), xv->w, FALSE, FALSE, 0);
}


gint open_document(GtkWidget *w, gpointer data)
{
	char *doc_name, *ds_name;
	
	doc_name = get_file_name("Select an XML document");
	if (!doc_name) return(TRUE);

	ds_name = get_file_name("Select a matching displayspec");
	if (!ds_name) return(TRUE);

	open_document_do(doc_name, ds_name);
	return(TRUE);
}
