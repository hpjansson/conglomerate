/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <string.h>
#include <gtk/gtk.h>
#include "global.h"
#include "cong-dispspec.h"
#include "cong-dispspec-registry.h"
#include "cong-error-dialog.h"

struct CongDispspecRegistry
{
	int num;
	CongDispspec** array;
};

struct LoadingDetails
{
	CongDispspecRegistry* registry;
	GnomeVFSURI* path_uri;
};

gboolean
visit_func(const gchar *rel_path,
	   GnomeVFSFileInfo *info,
	   gboolean recursing_will_loop,
	   gpointer data,
	   gboolean *recurse)
{
	struct LoadingDetails* details = (struct LoadingDetails*)data;
	char* match;

	/* g_message("visit_func called\n"); */

	/* FIXME: Ultimately we should do a MIME-lookup */
	/* Search for strings that are terminated with ".xds" */
	match=strstr(rel_path,".xds");
	if (match && match[4]=='\0') {
		/* This looks like an xds file: */
		/* get at name */
		GnomeVFSURI* uri_filename = gnome_vfs_uri_append_string(details->path_uri,
									rel_path);

		CongDispspec* ds;
		GnomeVFSResult vfs_result = cong_dispspec_new_from_xds_file(uri_filename, &ds);

		if (vfs_result==GNOME_VFS_OK) {
			
			if (ds!=NULL) {
				cong_dispspec_registry_add(details->registry, ds);
			} else {
				g_message("Problem parsing xds file\n");
			}
		} else {
			g_message("Problem loading xds file\n");
		}

		gnome_vfs_uri_unref(uri_filename);
	}
	
	return TRUE;
}

CongDispspecRegistry*
cong_dispspec_registry_new(const gchar* xds_directory, GtkWindow *toplevel_window)
{
	CongDispspecRegistry* registry;
	GnomeVFSResult vfs_result;
	struct LoadingDetails details;

	g_return_val_if_fail(xds_directory, NULL);

	registry = g_new0(CongDispspecRegistry,1);

	details.registry=registry;
	details.path_uri=gnome_vfs_uri_new(xds_directory);

	/* Scan the directory for xds files: */
	vfs_result = gnome_vfs_directory_visit(xds_directory,
					       GNOME_VFS_FILE_INFO_DEFAULT,
					       GNOME_VFS_DIRECTORY_VISIT_DEFAULT,
					       visit_func,
					       (gpointer)&details);

	if (vfs_result!=GNOME_VFS_OK) {
		GtkDialog* dialog = cong_error_dialog_new_from_file_operation_failure(toplevel_window,
										      _("Conglomerate could not read its registry of document types."),
										      details.path_uri, 
										      vfs_result, 
										      _("Conglomerate attempted to look at all the files in the location."));
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));

		gnome_vfs_uri_unref(details.path_uri);

		g_free(registry);
		
		return NULL;
	}

	gnome_vfs_uri_unref(details.path_uri);

	return registry;
}

void
cong_dispspec_registry_free(CongDispspecRegistry* registry)
{
	g_assert(0); /* for now */
}

unsigned int
cong_dispspec_registry_get_num(CongDispspecRegistry* registry)
{
	g_return_val_if_fail(registry,0);

	return registry->num;
}

CongDispspec*
cong_dispspec_registry_get(CongDispspecRegistry* registry, unsigned int i)
{
	g_return_val_if_fail(registry,NULL);
	g_return_val_if_fail(i<registry->num,NULL);

	return registry->array[i];
}

void
cong_dispspec_registry_add(CongDispspecRegistry* registry, CongDispspec* ds)
{
	registry->num++;

	registry->array = g_realloc(registry->array, sizeof(CongDispspec*)*registry->num);

	registry->array[registry->num-1]=ds;
}

void
cong_dispspec_registry_dump(CongDispspecRegistry* registry)
{
	int i;

	printf("CongDispspecRegistry contains %i dispspec(s)\n", registry->num);

	for (i=0;i<registry->num;i++) {
		CongDispspec* ds = registry->array[i];
		
		printf("ds[%i] = \"%s\"\n", i, cong_dispspec_get_name(ds));
	}
}

static gchar*
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
CongDispspec*
cong_dispspec_registry_get_appropriate_dispspec(xmlDocPtr doc)
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
			CongDispspec* ds = cong_dispspec_registry_get(the_globals.ds_registry, i);

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

	/* FIXME:  Do a selection dialog for the user (including the ability to generate a new dispspec): */

	return NULL;
}
