/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <string.h>
#include <gtk/gtk.h>
#include "global.h"
#include "cong-dispspec.h"
#include "cong-dispspec-registry.h"
#include "cong-error-dialog.h"
#include "cong-app.h"
#include "cong-util.h"

struct CongDispspecRegistry
{
	int num;
	CongDispspec** array;
};

/* Add all disspecs found in directory to an existing DispspecRegistry */
/**
 * cong_dispspec_registry_add_dir:
 * @registry:
 * @xds_directory:
 * @toplevel_window:
 * @raise_errs:
 *
 * TODO: Write me
 */
void
cong_dispspec_registry_add_dir(CongDispspecRegistry *registry, const gchar *xds_directory, GtkWindow *toplevel_window, gboolean raise_errs)
{
	gboolean result;
	GFile *xds_dir;
	GFileEnumerator *directory;
	GFileInfo *info;
	GError *error = NULL;

	xds_dir = g_file_new_for_path(xds_directory);

	/* Scan the directory for xds files: */

	directory = g_file_enumerate_children(xds_dir,
	                                      G_FILE_ATTRIBUTE_STANDARD_NAME,
	                                      G_FILE_QUERY_INFO_NONE,
	                                      NULL,
	                                      &error);

	if(!directory) {
		if(raise_errs) {
			GtkDialog* dialog = cong_error_dialog_new_from_file_operation_failure(toplevel_window,
			                                                                      _("Conglomerate could not read its registry of document types."),
			                                                                      xds_dir,
			                                                                      error,
			                                                                      _("Conglomerate attempted to look at all the files in the location."));
			cong_error_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(GTK_WIDGET(dialog));
		}

		g_object_unref(xds_dir);
		return;
	}

	while((info = g_file_enumerator_next_file(directory, NULL, &error)) != NULL) {
		const char *rel_path = g_file_info_get_name(info);
		/* FIXME: Ultimately we should do a MIME-lookup */
		/* Search for strings that are terminated with ".xds" */
		if (g_str_has_suffix(rel_path, ".xds")) {
			/* This looks like an xds file: */
			/* get at name */
			GFile *file = g_file_get_child(xds_dir, rel_path);
			char *filename = g_file_get_path(file);
			CongDispspec* ds;
			gboolean result;

			result = cong_dispspec_new_from_xds_file(file, &ds, &error);
			if (result == TRUE) {
				if (ds!=NULL) {
					cong_dispspec_registry_add(registry, ds);
				} else {
					g_warning("Problem parsing xds file: %s.\n", filename);
				}
			} else {
				g_warning("Problem loading xds file: %s.\n",filename);
			}

			g_free(filename);
			g_object_unref(file);
		}
		g_object_unref(info);
	}

	if(raise_errs && error) {
		GtkDialog* dialog = cong_error_dialog_new_from_file_operation_failure(toplevel_window,
		                                                                      _("Conglomerate failed while reading its registry of document types."),
		                                                                      xds_dir,
		                                                                      error,
		                                                                      _("Conglomerate attempted to look at all the files in the location."));
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
	}

	result = g_file_enumerator_close(directory, NULL, &error);
	if(!result) {
		g_warning("Error closing directory '%s': %s", xds_directory, error->message);
		g_error_free(error);
	}

	g_object_unref(directory);
	g_object_unref(xds_dir);
}	

/* Create a new DispspecRegistry.
   If a directory is specified, read disppsec files from it and insert them into registry.  If not, returned registry is empty. */
/**
 * cong_dispspec_registry_new:
 * @xds_directory:
 * @toplevel_window:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspecRegistry*
cong_dispspec_registry_new(const gchar* xds_directory, GtkWindow *toplevel_window)
{
	CongDispspecRegistry* registry;

	registry = g_new0(CongDispspecRegistry,1);

	if (xds_directory != NULL) {
		cong_dispspec_registry_add_dir (registry, xds_directory, toplevel_window, 1);
	}

	return registry;
}

/**
 * cong_dispspec_registry_free:
 * @registry:
 *
 * This function is currently just a false assertion
 */
void
cong_dispspec_registry_free(CongDispspecRegistry* registry)
{
	g_assert(0); /* for now */
}

/**
 * cong_dispspec_registry_get_num:
 * @registry:
 *
 * TODO: Write me
 * Returns:
 */
unsigned int
cong_dispspec_registry_get_num(CongDispspecRegistry* registry)
{
	g_return_val_if_fail(registry,0);

	return registry->num;
}

/**
 * cong_dispspec_registry_get:
 * @registry:
 * @i:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspec*
cong_dispspec_registry_get(CongDispspecRegistry* registry, unsigned int i)
{
	g_return_val_if_fail(registry,NULL);
	g_return_val_if_fail(i<registry->num,NULL);

	return registry->array[i];
}

/**
 * cong_dispspec_registry_add:
 * @registry:
 * @ds:
 *
 * TODO: Write me
 */
void
cong_dispspec_registry_add(CongDispspecRegistry* registry, CongDispspec* ds)
{
	registry->num++;

	registry->array = g_realloc(registry->array, sizeof(CongDispspec*)*registry->num);

	registry->array[registry->num-1]=ds;
}

/**
 * cong_dispspec_registry_dump:
 * @registry:
 *
 * TODO: Write me
 */
void
cong_dispspec_registry_dump(CongDispspecRegistry* registry)
{
	int i;

	g_message ("CongDispspecRegistry contains %i dispspec(s)\n", registry->num);

	for (i=0;i<registry->num;i++) {
		CongDispspec* ds = registry->array[i];
		
		g_message ("ds[%i] = \"%s\"\n", i, cong_dispspec_get_name(ds));
	}
}

#if 0
static gboolean
get_toplevel_tag(xmlDocPtr doc, gchar** xmlns, gchar** tagname)
{
	xmlNodePtr xml_toplevel;

	g_return_val_if_fail(doc, FALSE);
	g_return_val_if_fail(xmlns, FALSE);
	g_return_val_if_fail(tagname, FALSE);

	
	for (xml_toplevel=doc->children; xml_toplevel; xml_toplevel = xml_toplevel->next) {
		if (xml_toplevel->type==XML_ELEMENT_NODE) {
			*xmlns = g_strdup(cong_node_xmlns(xml_toplevel));
			*tagname = g_strdup(xml_toplevel->name);
			return TRUE;
		}
	}

	return FALSE;
}
#endif

/**
 * cong_cell_renderer_simple_new:
 *
 * TODO: Write me
 */
GtkCellRenderer*
cong_cell_renderer_simple_new (void)
{
	GtkCellRenderer* renderer = gtk_cell_renderer_text_new ();

	g_object_set (G_OBJECT (renderer),
		      "text",
		      "fubar",
		      NULL);

	return renderer;
}


struct CongDispspecCoverage
{
	CongDispspec *ds;
	gdouble coverage;
};

enum {
	COVERAGELISTSTORE_DS_FIELD,
	COVERAGELISTSTORE_COVERAGE_FIELD,

	COVERAGELISTSTORE_COVERAGE_NUM_FIELDS
};

#if 0
static CongDispspec*
run_coverage_selector_dialog (struct CongDispspecCoverage *coverage_array,
			      guint count)
{
	GladeXML *xml;
	GtkTreeView *tree_view;
	GtkListStore *list_store;
	int i;

	g_assert (coverage_array);

	xml = cong_util_load_glade_file ("conglomerate/glade/cong-dispspec-selector.glade",
					 NULL,
					 NULL,
					 NULL);
	tree_view = GTK_TREE_VIEW (glade_xml_get_widget(xml, "treeview1"));
	list_store = gtk_list_store_new (COVERAGELISTSTORE_COVERAGE_NUM_FIELDS, G_TYPE_POINTER, G_TYPE_DOUBLE);

	gtk_tree_view_set_model (tree_view,
				 GTK_TREE_MODEL (list_store));
	g_object_unref (G_OBJECT (list_store));

	/* Populate the list store: */
	{
		GtkTreeIter iter;

		for (i=0;i<count;i++) {
			gtk_list_store_append (list_store, &iter);
			
			gtk_list_store_set (list_store, &iter,
					    COVERAGELISTSTORE_DS_FIELD, coverage_array[i].ds,
					    COVERAGELISTSTORE_COVERAGE_FIELD, coverage_array[i].coverage,
					    -1);
		}
		
		/* Add a new document type entry: */
		gtk_list_store_append (list_store, &iter);
		
		gtk_list_store_set (list_store, &iter,
				    COVERAGELISTSTORE_DS_FIELD, NULL,
				    -1);
	}

	/* Create columns & renderers: */
	{
		GtkTreeViewColumn *column;
		GtkCellRenderer *renderer;

		/* File type column: */
		{
			column = gtk_tree_view_column_new();
			gtk_tree_view_column_set_title(column, _("File Type"));

			renderer = cong_cell_renderer_simple_new ();
			gtk_tree_view_column_pack_start (column, renderer, FALSE);
			
			gtk_tree_view_append_column (tree_view, column);
		}

		/* Coverage column: */
		{
			column = gtk_tree_view_column_new();
			gtk_tree_view_column_set_title(column, _("Coverage"));

			renderer = cong_cell_renderer_simple_new ();
			gtk_tree_view_column_pack_start (column, renderer, FALSE);
			
			gtk_tree_view_append_column (tree_view, column);
		}
		
		/* Description column: */
		{
			column = gtk_tree_view_column_new();
			gtk_tree_view_column_set_title(column, _("Description"));

			renderer = cong_cell_renderer_simple_new ();
			gtk_tree_view_column_pack_start (column, renderer, FALSE);
			
			gtk_tree_view_append_column (tree_view, column);
		}
	}

	gtk_dialog_run (GTK_DIALOG (glade_xml_get_widget (xml, "coverage_dialog")));

	g_object_unref (G_OBJECT (xml));

	return NULL; 
}
#endif


/**
 * cong_dispspec_registry_get_appropriate_dispspec:
 * @registry:
 * @doc:
 * @filename_extension:
 * 
 * Routine to figure out an appropriate default dispspec for use with this file.
 * Looks for a DTD; if found, it looks up the DTD in a mapping.
 * If this fails, it looks at the top-level tag and makes a guess, but asks the user for confirmation.
 * If this fails, it asks the user.
 * 
 * Returns: The appropriate dispspec for use with the file.
 */
CongDispspec*
cong_dispspec_registry_get_appropriate_dispspec (CongDispspecRegistry* registry, 
						 xmlDocPtr doc,
						 const gchar *filename_extension)
{
#if 0
	gchar* toplevel_xmlns;
	gchar* toplevel_tag;
#endif

	g_return_val_if_fail (registry,NULL);
	g_return_val_if_fail (doc,NULL);

	/* Check for a DTD match: */
	{
		CongDispspec *ds = cong_dispspec_registry_get_dispspec_for_dtd (registry, 
										doc);

		if (ds) {
			return ds;
		}
	}

#if 1
	/* Scan the entire document and figure out what tags are present and how well they match those in the various dispspecs, and then produce a sorted list of dispspecs to choose from: */
	/* We only look at elements without namespaces for this calculation */
	{
		int i;
		struct CongDispspecCoverage *coverage_array;
#if 0
		CongDispspec *ds;
#endif

		coverage_array = g_new0 (struct CongDispspecCoverage, cong_dispspec_registry_get_num (registry));
	       
		for (i=0;i<cong_dispspec_registry_get_num (registry);i++) {
			coverage_array[i].ds = cong_dispspec_registry_get (registry, i);
			coverage_array[i].coverage = cong_dispspec_calculate_coverage (coverage_array[i].ds,
										       doc);
			g_message ("coverage of %s = %f", 
				   cong_dispspec_get_name (coverage_array[i].ds), 
				   coverage_array[i].coverage);
		}


		/* FIXME:  Do a selection dialog for the user (including the ability to generate a new dispspec): */
#if 0
		/* bug #133405 */
		ds = run_coverage_selector_dialog (coverage_array,
						   cong_dispspec_registry_get_num (registry));
		g_free (coverage_array);

		if (ds) {
			return ds;
		}
#else
		/* For now, just pick the DS with the best coverage: */
		{
			gdouble best_coverage = 0.8; /* have a threshold before a dispspec is even considered.  It might be that none are good enough */
			CongDispspec *best_dispspec = NULL;

			for (i=0;i<cong_dispspec_registry_get_num (registry);i++) {
				if (coverage_array[i].coverage>best_coverage) {
					best_coverage = coverage_array[i].coverage;
					best_dispspec = coverage_array[i].ds;
				}
			}

			g_free (coverage_array);

			return best_dispspec;
		}
#endif

	}
#else
	/* Otherwise, check for a matching top-level element:*/
	if (get_toplevel_tag(doc, &toplevel_xmlns, &toplevel_tag)) {
		int i;
		
		g_message("Searching for a match against top-level tag <%s>\n", toplevel_tag);

		for (i=0;i<cong_dispspec_registry_get_num(registry);i++) {
			CongDispspec* ds = cong_dispspec_registry_get(registry, i);

			CongDispspecElement* element = cong_dispspec_lookup_element(ds, toplevel_xmlns, toplevel_tag);
			
			if (element) {
				/* FIXME: check for appropriateness */
				g_free(toplevel_xmlns);
				g_free(toplevel_tag);
				return ds;
			}
		}
		
		/* No match */
		g_free(toplevel_xmlns);
		g_free(toplevel_tag);
	}
#endif


	return NULL;
}

CongDispspec*
cong_dispspec_registry_get_dispspec_for_dtd (CongDispspecRegistry* registry, 
					     xmlDocPtr xml_doc)
{
	g_return_val_if_fail (registry,NULL);
	g_return_val_if_fail (xml_doc,NULL);

	if (xml_doc->extSubset) {

		/* Check for matching PUBLIC ID: */
		if (xml_doc->extSubset->ExternalID) {
			int i;

			for (i=0;i<cong_dispspec_registry_get_num (registry);i++) {
				CongDispspec* ds = cong_dispspec_registry_get (registry, i);

				const CongExternalDocumentModel* dtd = cong_dispspec_get_external_document_model (ds, CONG_DOCUMENT_MODE_TYPE_DTD);

				if (dtd) {
					if (cong_external_document_model_get_public_id (dtd)) {
						if (0==strcmp ((const char*)xml_doc->extSubset->ExternalID, cong_external_document_model_get_public_id (dtd))) {
							/* g_message("Found display spec based on matching public ID of DTD:\n    %s\n", xml_doc->extSubset->ExternalID); */
							return ds;
						}
					}
				}
			}
		}

		/* Check for matching SYSTEM ID: */
		if (xml_doc->extSubset->SystemID) {
			const xmlChar *sysID = xml_doc->extSubset->SystemID;
			int i;
			
			for (i=0;i<cong_dispspec_registry_get_num (registry);i++) {
				CongDispspec *ds = cong_dispspec_registry_get (registry, i);
				const CongExternalDocumentModel *model;
				const char *this_id = NULL;
				
				model = cong_dispspec_get_external_document_model (ds, CONG_DOCUMENT_MODE_TYPE_DTD);
				if (model != NULL) {
					this_id = cong_external_document_model_get_system_id (model);
					if (this_id != NULL) {
						if (!strcmp ((const gchar*)sysID, this_id)) {
							g_message("Found display spec based on matching system ID of DTD\n    %s\n", sysID);
							return ds;
						}
					}
				}
			}
		}
	}

	/* No match found: */
	return NULL;
}


CongDispspecElement*
cong_dispspec_registry_get_dispspec_element_for_node (CongDispspecRegistry *registry, 
						      CongDispspec *default_ds, 
						      CongNodePtr node)
{
	CongDispspecElement *result;
	int i;

	g_return_val_if_fail (registry, NULL);
	g_return_val_if_fail (node, NULL);

	/* Search for noe in default ds, if any: */
	if (default_ds) {
		result = cong_dispspec_lookup_node (default_ds, 
						    node);

		if (result) {
			return result;
		}
	}

	/* If the node is in a namespace, look in the various other dispspecs: */
	if (node->ns) {
		for (i=0;i<registry->num;i++) {
			result = cong_dispspec_lookup_node (registry->array[i], 
							    node);
			
			if (result) {
				return result;
			}
		}	
	}
	
	return NULL;
}

CongDispspecElement*
cong_dispspec_registry_get_dispspec_element_for_description (CongDispspecRegistry *registry, 
							     const CongElementDescription *desc,
							     const CongDispspec *default_ds)
{
	CongDispspecElement *result;
	int i;

	g_return_val_if_fail (registry, NULL);
	g_return_val_if_fail (desc, NULL);

	/* Search in default ds, if any: */
	if (default_ds) {
		result = cong_dispspec_lookup_element (default_ds, 
						       desc->ns_uri,
						       desc->local_name);

		if (result) {
			return result;
		}
	}

	/* If the node is in a namespace, look in the various other dispspecs: */
	if (desc->ns_uri) {
		for (i=0;i<registry->num;i++) {
			result = cong_dispspec_lookup_element (registry->array[i], 
							       desc->ns_uri,
							       desc->local_name);
			
			if (result) {
				return result;
			}
		}	
	}
	
	return NULL;
}
