#include <string.h>
#include <gtk/gtk.h>
#include <ttree.h>
#include <xml.h>
#include "global.h"

#include <unistd.h> /* for chdir */

int open_document_do(const char *doc_name, const char *ds_name)
{
	char *p;
	TTREE *xml_in;
	FILE *xml_f;

	the_globals.ds = cong_dispspec_new_from_file(ds_name);
	if (the_globals.ds==NULL) {
	  return(TRUE);  /* Invalid displayspec. */	  
	}

	xml_f = fopen(doc_name, "rt");
	if (!xml_f) {
	  g_warning("Problem opening doc file \"%s\"\n", doc_name);
	  return(TRUE);
	}

	p = strrchr(doc_name, '/');
	if (p)
	{
		*p = 0;
		chdir(doc_name);
	}
	
	xml_in = xml_f_to_ttree(xml_f, 0);
	if (!xml_in) {
	  g_warning("Problem parsing doc file \"%s\"\n", doc_name);
	  return(TRUE);  /* Invalid XML document. */
	}

	fclose(xml_f);	

	xml_t_trim(xml_in);
	the_globals.xv = xmlview_new(cong_document_new_from_ttree(xml_in), the_globals.ds);
	gtk_box_pack_start(GTK_BOX(cong_gui_get_root(&the_gui)), the_globals.xv->w, FALSE, FALSE, 0);

	return (TRUE);
}


gint open_document(GtkWidget *w, gpointer data)
{
	const char *doc_name, *ds_name;
	
	doc_name = get_file_name("Select an XML document");
	if (!doc_name) return(TRUE);

	ds_name = get_file_name("Select a matching displayspec");
	if (!ds_name) return(TRUE);

	open_document_do(doc_name, ds_name);
	return(TRUE);
}
