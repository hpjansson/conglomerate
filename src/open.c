/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <string.h>
#include <gtk/gtk.h>
#include <ttree.h>
#include <xml.h>
#include "global.h"

#include <unistd.h> /* for chdir */

#include <libxml/tree.h>

#if 1
/* New implementation: */

gchar*
get_toplevel_tag(xmlDocPtr doc)
{
	xmlNodePtr xml_toplevel;

	g_return_val_if_fail(doc,NULL);
	
	for (xml_toplevel=doc->children; xml_toplevel; xml_toplevel = xml_toplevel->next) {
		if (xml_toplevel->type==XML_ELEMENT_NODE) {
			return g_strdup(xml_toplevel->name);
		}
	}

	return NULL;
}

/**
 * Routine to figure out an appropriate dispspec for use with this file.
 * Looks for a DTD; if found, it looks up the DTD in a mapping.
 * If this fails, it looks at the top-level tag and makes a guess, but asks the user for confirmation.
 * If this fails, it asks the user.
 */
const CongDispspec*
get_appropriate_dispspec(xmlDocPtr doc)
{
	gchar* toplevel_tag;

	g_return_val_if_fail(doc,NULL);

	/* FIXME: check for a DTD */
#if 0
	if (doc->) {
	}
#endif

	toplevel_tag = get_toplevel_tag(doc);

	if (toplevel_tag) {
		int i;
		
		g_message("Searching for a match against top-level tag <%s>\n", toplevel_tag);

		for (i=0;i<cong_dispspec_registry_get_num(the_globals.ds_registry);i++) {
			const CongDispspec* ds = cong_dispspec_registry_get(the_globals.ds_registry, i);

			CongDispspecElement* element = cong_dispspec_lookup_element(ds, toplevel_tag);
			
			if (element) {
				/* FIXME: check for appropriateness */
				g_free(toplevel_tag);
				return ds;
			}
		}
		
		/* No match */

		g_free(toplevel_tag);
	}

	/* Do a selection dialog for the user (including the ability to generate a new dispspec): */

	return NULL;
}

void open_document_do(const gchar* doc_name)
{
	char *p;
	TTREE *xml_in;
	FILE *xml_f;

	/* Use libxml to load the doc: */
	{
		xmlDocPtr doc=NULL;

		GnomeVFSURI* file_uri = gnome_vfs_uri_new(doc_name);

		/* Load using GnomeVFS: */
		{
			char* buffer;
			GnomeVFSFileSize size;
			GnomeVFSResult vfs_result = cong_vfs_new_buffer_from_file(doc_name, &buffer, &size);

			if (vfs_result!=GNOME_VFS_OK) {
				GtkDialog* dialog = cong_error_dialog_new_file_open_failed_from_vfs_result(file_uri, vfs_result);
			
				cong_error_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(GTK_WIDGET(dialog));

				gnome_vfs_uri_unref(file_uri);

				return;
			}

			g_assert(buffer);

			/* Parse the file from the buffer: */
#if 0
			// Can't use DocBook loader as it only supports loading from a file, not from memory
#else
			doc = xmlParseMemory(buffer, size);
#endif

			g_free(buffer);
		}

		if (NULL==doc) {
			GtkDialog* dialog = cong_error_dialog_new_file_open_failed_from_parser_error(file_uri);
				
			cong_error_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(GTK_WIDGET(dialog));

			gnome_vfs_uri_unref(file_uri);
						
			return;
		}

		gnome_vfs_uri_unref(file_uri);
		
		xml_in = convert_libxml_to_ttree_doc(doc);

		the_globals.ds = get_appropriate_dispspec(doc);

		xmlFreeDoc(doc);
	}


	the_globals.xv = xmlview_new(cong_document_new_from_ttree(xml_in), the_globals.ds);
	gtk_box_pack_start(GTK_BOX(cong_gui_get_root(&the_gui)), the_globals.xv->w, FALSE, FALSE, 0);

}

gint open_document(GtkWidget *w, gpointer data)
{
	const char *doc_name, *ds_name;
	
	doc_name = get_file_name("Select an XML document");
	if (!doc_name) return(TRUE);

	open_document_do(doc_name);
	return(TRUE);  
}
#else
int open_document_do(const char *doc_name, const char *ds_name)
{
	char *p;
	TTREE *xml_in;
	FILE *xml_f;

	the_globals.ds = cong_dispspec_new_from_ds_file(ds_name);
	if (the_globals.ds==NULL) {
	  return(TRUE);  /* Invalid displayspec. */	  
	}

	xml_f = fopen(doc_name, "rt");
	if (!xml_f) {
	  GtkWidget* dialog;

	  g_warning("Problem opening doc file \"%s\"\n", doc_name);

	  dialog = cong_error_dialog_new_file_open_failed(doc_name);
	  cong_error_dialog_run(GTK_DIALOG(dialog));
	  gtk_widget_destroy(dialog);

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

/* Old implementation: */
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
#endif
