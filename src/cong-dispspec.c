/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include <string.h>

#include <ctype.h>

#include <libxml/tree.h>
#include <libxml/debugXML.h>
#include "global.h"
#include "cong-dispspec.h"
#include "cong-dispspec-element.h"

#include <stdio.h>
#include <glib.h>
#include <libxml/parser.h>
#include "cong-node.h"
#include "cong-app.h"
#include "cong-eel.h"
#include "cong-dtd.h"
#include "cong-util.h"
#include "cong-enum-mapping.h"
#include "cong-vfs.h"

#if 0
#define DS_DEBUG_MSG1(x)    g_message((x))
#define DS_DEBUG_MSG2(x, a) g_message((x), (a))
#define DS_DEBUG_MSG3(x, a, b) g_message((x), (a), (b))
#else
#define DS_DEBUG_MSG1(x)    ((void)0)
#define DS_DEBUG_MSG2(x, a) ((void)0)
#define DS_DEBUG_MSG3(x, a, b) ((void)0)
#endif

/* Internal data structure declarations: */
struct SearchTreeKey
{
	gchar* ns_uri;
	gchar* local_name;
};

typedef CongSerialisationFormat* CongSerialisationFormatPtr;

struct CongSerialisationFormat
{
	gchar *extension;
};

struct CongExternalDocumentModel
{
	CongDocumentModelType model_type;
	gchar *public_id;
	gchar *system_id;
};

struct CongDispspec
{
	/* URI of namespace, or NULL: */
	gchar *ns_uri;

	/* The serialisation formats: */
	guint num_serialisation_formats;
	CongSerialisationFormat **serialisation_formats;

	/* The CongDocumentModels: */
	CongExternalDocumentModel *document_models[NUM_CONG_DOCUMENT_MODEL_TYPES];

	GList *list_of_elements;

	/* We have a search tree, indexed by SearchTreeKeys */
	GTree *search_tree; 

	gchar *name;
	gchar *desc;
	gchar *filename_extension;
	GdkPixbuf *icon;

	GList *list_of_typical_prefix; /* list of gchar* */

	xmlNodePtr template;
};

/* Internal function declarations: */

/* Search tree callbacks: */
static gint key_compare_func (struct SearchTreeKey *a,
			      struct SearchTreeKey *b,
			      gpointer user_data);
static void key_destroy_func (struct SearchTreeKey *key);
static void value_destroy_func (gpointer data);

/* Subroutines for generating a CongDispspec from a DTD: */
static void
element_callback_generate_dispspec_from_dtd (void *payload, void *ds, xmlChar * name);

/* Subroutines for converting XDS XML to a CongDispspec: */
static CongDispspec* 
parse_xmldoc (xmlDocPtr doc);

static void 
parse_metadata (CongDispspec *ds, 
		xmlDocPtr doc, 
		xmlNodePtr node);

static void 
parse_serialisation (CongDispspec *ds, 
		     xmlDocPtr doc, 
		     xmlNodePtr node);

static void 
parse_document_models (CongDispspec *ds, 
		       xmlDocPtr doc, 
		       xmlNodePtr node);

static void 
parse_external_document_model (CongDispspec *ds, 
			       xmlDocPtr doc, 
			       xmlNodePtr node);

static void 
parse_template (CongDispspec *ds, 
		xmlNodePtr node);

/* Subroutines for converting a CongDispspec to XDS XML: */
static void 
add_xml_for_metadata (xmlDocPtr xml_doc, 
		      CongNodePtr root, 
		      CongDispspec *dispspec);

static void 
add_xml_for_serialisation_formats  (xmlDocPtr xml_doc, 
				    CongNodePtr root, 
				    CongDispspec *dispspec);

static void
add_xml_for_document_models (xmlDocPtr xml_doc, 
			     CongNodePtr root, 
			     CongDispspec *dispspec);

/* Subroutines for generating a CongDispspec from arbitrtary XML docs: */
static gboolean
contains_text (const xmlChar* string);

static gboolean
contains_carriage_return (const xmlChar* string);

static void
promote_element (CongDispspec * dispspec, 
		 CongDispspecElement * element,
		 xmlNodePtr node);

static void
handle_elements_from_xml (CongDispspec * dispspec, xmlNodePtr cur);

static void
ensure_all_elements_covered (CongDispspec * dispspec, xmlNodePtr cur);

static void
xml_to_dispspec (CongDispspec *dispspec, 
		 xmlDocPtr doc, 
		 xmlDtdPtr dtd,
		 const gchar *extension);

static xmlDtdPtr
load_dtd (xmlDocPtr doc);

static CongSerialisationFormat*
cong_serialisation_format_new (const gchar *extension)
{
	CongSerialisationFormat* format;

	g_return_val_if_fail (extension, NULL);

	format = g_new0(CongSerialisationFormat, 1);

	format->extension = g_strdup(extension);

	return format;
}

static CongExternalDocumentModel*
cong_external_document_model_new (CongDocumentModelType model_type,
				  const gchar *public_id,
				  const gchar *system_id)
{
	CongExternalDocumentModel* model;

	model = g_new0 (CongExternalDocumentModel, 1);

	model->model_type = model_type;

	model->public_id = g_strdup (public_id);
	model->system_id = g_strdup (system_id);
	
	return model;
}

static CongExternalDocumentModel*
make_model_from_dtd (xmlDtdPtr dtd)
{
	g_return_val_if_fail (dtd, NULL);


	return cong_external_document_model_new (CONG_DOCUMENT_MODE_TYPE_DTD,
						 dtd->ExternalID,
						 dtd->SystemID);
}

/* Exported functions: */

/* Barebones constructor, used to implement other ones: */
/**
 * cong_dispspec_new:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspec* 
cong_dispspec_new(void)
{
	CongDispspec *dispspec;
	
	dispspec = g_new0 (CongDispspec, 1);
	dispspec->search_tree = g_tree_new_full((GCompareDataFunc)key_compare_func,
						NULL,
						(GDestroyNotify)key_destroy_func,
						value_destroy_func);

	return dispspec;
}

/* Constructors that use the standard format: */
/**
 * cong_dispspec_new_from_xds_file:
 * @uri:
 * @ds:
 *
 * TODO: Write me
 * Returns:
 */
GnomeVFSResult 
cong_dispspec_new_from_xds_file(GnomeVFSURI *uri, CongDispspec** ds)
{
	char* buffer;
	GnomeVFSFileSize size;
	GnomeVFSResult vfs_result;

	g_return_val_if_fail(uri, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(ds, GNOME_VFS_ERROR_BAD_PARAMETERS);

	vfs_result = cong_vfs_new_buffer_from_uri(uri, &buffer, &size);

	if (vfs_result!=GNOME_VFS_OK) {
		return vfs_result;
	}

	*ds = cong_dispspec_new_from_xds_buffer(buffer, size);

	g_free (buffer);

	return vfs_result;	
}

/**
 * cong_dispspec_new_from_xds_buffer:
 * @buffer:
 * @size:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspec* 
cong_dispspec_new_from_xds_buffer(const char *buffer, size_t size)
{
	CongDispspec* ds;

	xmlDocPtr doc = xmlParseMemory(buffer, size);

	if (NULL==doc) {
		return NULL;
	}

	ds = parse_xmldoc(doc);

	xmlFreeDoc(doc);

	return ds;	
}



/* Constructors that try to generate from another format: */
/**
 * cong_dispspec_new_generate_from_dtd:
 * @dtd:
 * @name:
 * @description:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspec* 
cong_dispspec_new_generate_from_dtd (xmlDtdPtr dtd, 
				     const gchar *name, 
				     const gchar *description)
{
	CongDispspec *ds;

	g_return_val_if_fail (dtd, NULL);

	ds = cong_dispspec_new();

	if (name) {
		ds->name = g_strdup(name);
	}

	if (description) {
		ds->desc = g_strdup(description);
	}

	ds->document_models[CONG_DOCUMENT_MODE_TYPE_DTD] = make_model_from_dtd (dtd);
	
	/* Traverse the DTD; building stuff */
	xmlHashScan (dtd->elements, element_callback_generate_dispspec_from_dtd, ds);

	return ds;
}

/**
 * cong_dispspec_new_generate_from_xml_file:
 * @doc:
 * @extension:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspec *
cong_dispspec_new_generate_from_xml_file (xmlDocPtr doc,
					  const gchar *extension)
{
	CongDispspec *dispspec;

	g_return_val_if_fail (doc, NULL);
	/* Extension can be NULL if none was found */

	dispspec = cong_dispspec_new();

	xml_to_dispspec (dispspec, 
			 doc, 
			 load_dtd (doc), 
			 extension);

	return dispspec;
}

/**
 * cong_dispspec_delete:
 * @dispspec:
 *
 * TODO: Write me
 */
void 
cong_dispspec_delete (CongDispspec *dispspec)
{
	/* FIXME: is this causing heap corruption? */
#if 0
	CongDispspecElement *element;
	CongDispspecElement *next;

	g_return_if_fail(dispspec);

	/* Destroy elements: */
	for (element = dispspec->first; element; element=next) {
		next = element->next;
		cong_dispspec_element_destroy (element);
	}

	/* Destroy search tree: */
	g_assert (dispspec->search_tree);
	g_tree_destroy (dispspec->search_tree);
		
	if (dispspec->name) {
		g_free (dispspec->name);
	}
	if (dispspec->desc) {
		g_free (dispspec->desc);
	} 
	if (dispspec->icon) {
		g_object_unref (G_OBJECT(dispspec->icon));
	}
	if (dispspec->ns_uri) {
		g_free (dispspec->ns_uri);
	}
#endif
}

/**
 * cong_dispspec_make_xml:
 * @dispspec:
 *
 * TODO: Write me
 * Returns:
 */
xmlDocPtr 
cong_dispspec_make_xml(CongDispspec *dispspec)
{
	xmlDocPtr xml_doc;
	CongNodePtr root_node;

	g_return_val_if_fail (dispspec, NULL);
	
	
	/* Build up the document and its content: */
	xml_doc = xmlNewDoc("1.0");
	
	root_node = xmlNewDocNode(xml_doc,
				  NULL, /* xmlNsPtr ns, */
				  "dispspec",
				  NULL);
	
	xmlDocSetRootElement(xml_doc,
			     root_node);
	
	/* Add the metadata node: */
	add_xml_for_metadata(xml_doc, root_node, dispspec);

	/* Add the serialisation formats: */
	add_xml_for_serialisation_formats(xml_doc, root_node, dispspec);

	add_xml_for_document_models (xml_doc, root_node, dispspec);
	
	/* The <element-list> node: */
	{
		CongNodePtr element_list;
		
		element_list = xmlNewDocNode(xml_doc,
					     NULL,
					     "element-list",
					     NULL);			
		xmlAddChild(root_node, element_list);
		
		if (dispspec->ns_uri) {
			xmlSetProp (element_list, "nsURI", dispspec->ns_uri);
		}

		/* Typical prefixes: */
		{
			GList *iter;
			for (iter = dispspec->list_of_typical_prefix; iter; iter=iter->next) {
				const gchar* prefix = (const gchar*)(iter->data);
				CongNodePtr prefix_node = xmlNewDocNode (xml_doc,
									 NULL,
									 "typical-prefix",
									 NULL);			
				xmlAddChild (element_list, prefix_node);
				if (prefix) {
					xmlSetProp (prefix_node, "prefix", prefix);
				}
			}
		}

		/* The elements: */
		{
			GList *iter;
			for (iter = dispspec->list_of_elements; iter; iter=iter->next) {
				CongDispspecElement *element = (CongDispspecElement *)iter->data;
			
				xmlNodePtr element_as_xml = cong_dispspec_element_to_xml (element,
											  xml_doc);
				g_assert (element_as_xml);

				xmlAddChild (element_list, element_as_xml);
			}
		}	
	}		

	/* FIXME: add <document-template> if necessary */

	return xml_doc;	
}

/* Data for the dispspec: */
/**
 * cong_dispspec_get_name:
 * @ds:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_dispspec_get_name (const CongDispspec *ds)
{
	g_return_val_if_fail(ds,NULL);

	if (ds->name) {
		return ds->name;
	} else {
		return _("unnamed");
	}

}

/**
 * cong_dispspec_get_description:
 * @ds:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_dispspec_get_description (const CongDispspec *ds)
{
	g_return_val_if_fail(ds,NULL);

	if (ds->desc) {
		return ds->desc;
	} else {
		return _("No description available.");
	}
}

/** 
 * cong_dispspec_get_ns_uri:
 * @element: the dispspec in question
 * 
 * Returns: the namespace URI for this dispspec, or NULL if none
 */
const gchar*
cong_dispspec_get_ns_uri (CongDispspec *dispspec)
{
	g_return_val_if_fail (dispspec, NULL);

	return dispspec->ns_uri;
}

/**
 * cong_dispspec_get_num_serialisation_formats:
 * @ds:
 *
 * TODO: Write me
 * Returns:
 */
guint
cong_dispspec_get_num_serialisation_formats (const CongDispspec *ds)
{
	g_return_val_if_fail (ds, 0);

	return ds->num_serialisation_formats;
}

/**
 * cong_dispspec_get_serialisation_format:
 * @ds:
 * @index:
 *
 * TODO: Write me
 * Returns:
 */
const CongSerialisationFormat*
cong_dispspec_get_serialisation_format (const CongDispspec *ds,
					guint index)
{
	g_return_val_if_fail (ds, NULL);

	g_assert (index<ds->num_serialisation_formats);

	return ds->serialisation_formats[index];
}

/* Returns NULL if it can't find a serialisation format with that extension */
/**
 * cong_dispspec_lookup_filename_extension:
 * @ds:
 * @extension:
 *
 * TODO: Write me
 * Returns:
 */
const CongSerialisationFormat*
cong_dispspec_lookup_filename_extension (const CongDispspec *ds,
					 const gchar *extension)
{
	guint i;
	g_return_val_if_fail (ds, NULL);

	for (i=0;i<ds->num_serialisation_formats;i++) {
		const CongSerialisationFormat* format = ds->serialisation_formats[i];
		g_assert (format);
	
		if (0==strcmp(extension, format->extension)) {
			return format;
		}
	}

	return NULL;
}

/* Returns whether the dispspec uses that extension */
/**
 * cong_dispspec_matches_filename_extension:
 * @ds:
 * @extension:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_dispspec_matches_filename_extension (const CongDispspec *ds,
					  const gchar *extension)
{
	g_return_val_if_fail (ds, FALSE);

	return (NULL!=cong_dispspec_lookup_filename_extension (ds,
							       extension));
}

/**
 * cong_dispspec_get_external_document_model:
 * @ds:
 * @model_type:
 *
 * TODO: Write me
 * Returns:
 */
const CongExternalDocumentModel*
cong_dispspec_get_external_document_model (const CongDispspec *ds,
					   CongDocumentModelType model_type)
{
	g_return_val_if_fail(ds, NULL);
	g_return_val_if_fail(model_type<NUM_CONG_DOCUMENT_MODEL_TYPES, NULL);
	
	return ds->document_models[model_type];
}

/**
 * cong_dispspec_get_icon:
 * @ds:
 *
 * TODO: Write me
 * Returns:
 */
GdkPixbuf*
cong_dispspec_get_icon(const CongDispspec *ds)
{
	g_return_val_if_fail(ds, NULL);

	if (ds->icon) {
		g_object_ref(G_OBJECT(ds->icon));
	}
	return ds->icon;
}

/**
 * cong_dispspec_for_each_typical_prefix:
 * @ds:
 * @callback: function to be called for each typical prefix used by the namespace of this ds (if any)
 *
 * Iterates over all prefixes typically used for the namespace of this dispspec (if any).
 * Note that NULL is a valid value for the prefix, meaning that using the default namespace is typical.
 */
void
cong_dispspec_for_each_typical_prefix (const CongDispspec *ds,
				       void
				       (*callback) (const CongDispspec *ds,
						    const gchar *prefix, /* can be NULL */
						    gpointer user_data),
				       gpointer user_data)
{
	GList *iter;

	g_return_if_fail (ds);
	g_return_if_fail (callback);
	
	for (iter = ds->list_of_typical_prefix; iter; iter=iter->next) {
		(*callback) (ds, (const gchar*)(iter->data), user_data);
	}

}


/* Getting at elements within a dispspec */
/**
 * cong_dispspec_lookup_element:
 * @ds:
 * @ns_uri:
 * @local_name:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspecElement*
cong_dispspec_lookup_element (const CongDispspec *ds, 
			      const gchar* ns_uri, 
			      const gchar* local_name)
{
	/* We use the GTree search structure for speed */
	CongDispspecElement *element;
	struct SearchTreeKey key;

	g_return_val_if_fail (ds, NULL);
	g_return_val_if_fail (local_name, NULL);

	g_assert(ds->search_tree);

	if (!cong_util_ns_uri_equality (ns_uri, ds->ns_uri)) {
		return FALSE;
	}

	/* FIXME: we don't need the URI checking per-element anymore: */
	key.ns_uri = (gchar*)ns_uri;
	key.local_name = (gchar*)local_name;

	element =  g_tree_lookup (ds->search_tree, &key);

	return element;
}

/**
 * cong_dispspec_lookup_node:
 * @ds:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspecElement*
cong_dispspec_lookup_node(const CongDispspec *ds, CongNodePtr node)
{
	g_return_val_if_fail(ds, NULL);
	g_return_val_if_fail(node, NULL);

	return cong_dispspec_lookup_element (ds, 
					     cong_node_get_ns_uri(node), 
					     cong_node_get_local_name(node));
}

/**
 * cong_dispspec_type:
 * @ds:
 * @ns_uri:
 * @local_name:
 *
 * TODO: Write me
 */
CongElementType
cong_dispspec_type (CongDispspec *ds, 
		    const gchar* ns_uri, 
		    const gchar* local_name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element (ds, 
								     ns_uri, 
								     local_name);

	if (NULL==element) {
		return CONG_ELEMENT_TYPE_UNKNOWN;
	}

	return cong_dispspec_element_type(element);
}

/**
 * cong_dispspec_get_first_element:
 * @ds:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspecElement*
cong_dispspec_get_first_element(CongDispspec *ds)
{
	g_return_val_if_fail(ds, NULL);

	if (ds->list_of_elements) {
		return (CongDispspecElement*)(ds->list_of_elements->data);
	} else {
		return NULL;
	}
}

/**
 * cong_dispspec_for_each_element:
 * @ds:
 * @callback:
 * @user_data:
 *
 * TODO: Write me
 */
void
cong_dispspec_for_each_element (CongDispspec *ds, 
				void
				(*callback) (CongDispspec *ds,
					     CongDispspecElement *ds_element,
					     gpointer user_data),
				gpointer user_data)
{
	GList *iter;
	
	g_return_if_fail (ds);
	g_return_if_fail (callback);

	for (iter=ds->list_of_elements; iter; iter=iter->next) {
		(*callback) (ds,
			     (CongDispspecElement*)iter->data,
			     user_data);
	}
}


/* Manipulating a dispspec: */
/**
 * cong_dispspec_add_element:
 * @ds:
 * @element:
 *
 * TODO: Write me
 */
void 
cong_dispspec_add_element (CongDispspec* ds, 
			   CongDispspecElement* element)
{
	struct SearchTreeKey *key;

	g_return_if_fail(ds);
	g_return_if_fail(element);

	ds->list_of_elements = g_list_append (ds->list_of_elements,
					      element);

	key = g_new0(struct SearchTreeKey, 1);
	if (cong_dispspec_element_get_ns_uri (element)) {
		key->ns_uri = g_strdup (cong_dispspec_element_get_ns_uri (element));
	}
	g_assert (cong_dispspec_element_get_local_name (element));
	key->local_name = g_strdup (cong_dispspec_element_get_local_name (element));
	/* FIXME: does this leak? */

	g_tree_insert(ds->search_tree, key, element);
}

/**
 * cong_dispspec_get_num_elements:
 * @ds:
 *
 * TODO: Write me
 * Returns:
 */
guint
cong_dispspec_get_num_elements (CongDispspec *ds)
{
	guint count;
	GList *iter;

	g_return_val_if_fail (ds, 0);

	/* o(n): */

	count = 0;
	for (iter = ds->list_of_elements; iter; iter=iter->next) {
		count++;
	}
	return count;	
}

/**
 * cong_dispspec_get_element:
 * @ds:
 * @index:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspecElement*
cong_dispspec_get_element (CongDispspec *ds,
			   guint index)
{
	guint count;
	GList *iter;

	g_return_val_if_fail (ds, NULL);

	/* o(n): */

	count = 0;
	for (iter = ds->list_of_elements; iter; iter=iter->next) {
		if (count==index) {
			return (CongDispspecElement*)iter->data;
		}

		count++;
	}
	return NULL;	
}

/* Various functions that may get deprecated at some point: */
#if NEW_LOOK
#if 0
GdkGC *cong_dispspec_gc_get(CongDispspec *ds, CongNodePtr x, CongDispspecGCUsage usage)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, );

	if (element) {
		return cong_dispspec_element_gc(element, usage);
	} else {
		return NULL;
	}
}
#endif
#else
/**
 * cong_dispspec_name_gc_get:
 * @ds:
 * @t:
 * @tog:
 *
 * TODO: Write me
 * Returns:
 */
GdkGC *
cong_dispspec_name_gc_get(CongDispspec *ds, CongNodePtr t, int tog)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, cong_node_name(t));

	if (element) {
		return cong_dispspec_element_gc(element);
	} else {
		return NULL;
	}
}

GdkGC *
cong_dispspec_gc_get(CongDispspec *ds, CongNodePtr x, int tog)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, xml_frag_name_nice(x));

	if (element) {
		return cong_dispspec_element_gc(element);
	} else {
		return NULL;
	}
}
#endif

/**
 * cong_dispspec_name_get:
 * @ds:
 * @node:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_dispspec_name_get (CongDispspec *ds, 
			CongNodePtr node)
{
	CongDispspecElement* element;

	g_return_val_if_fail (ds, NULL);
	g_return_val_if_fail (node, NULL);
	g_return_val_if_fail (cong_node_type(node)==CONG_NODE_TYPE_ELEMENT, NULL);

	element = cong_dispspec_lookup_node (ds, node);
	if (element) {
		return cong_dispspec_element_username(element);
	}
  
	return cong_node_get_local_name (node);
}

#if 0
char *cong_dispspec_name_name_get(CongDispspec *ds, TTREE *t)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, t->data);
	if (element) {
		return (char*)cong_dispspec_element_username(element);
	}
  
	return(t->data);
}
#endif

#if 1
/**
 * cong_dispspec_element_structural:
 * @ds:
 * @ns_uri:
 * @local_name:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
cong_dispspec_element_structural (CongDispspec *ds, 
				  const gchar *ns_uri, 
				  const char *local_name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element (ds, ns_uri, local_name);

	if (NULL==element) {
		return FALSE;
	}

	return (CONG_ELEMENT_TYPE_STRUCTURAL == cong_dispspec_element_type(element));
}

/**
 * cong_dispspec_element_collapse:
 * @ds:
 * @ns_uri:
 * @local_name:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
cong_dispspec_element_collapse (CongDispspec *ds, 
				const gchar *ns_uri, 
				const char *local_name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element (ds, ns_uri, local_name);

	if (NULL==element) {
		return FALSE;
	}
	
	return cong_dispspec_element_collapseto(element);
}

/**
 * cong_dispspec_element_span:
 * @ds:
 * @ns_uri:
 * @local_name:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_dispspec_element_span (CongDispspec *ds, 
			    const gchar *ns_uri, 
			    const char *local_name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element (ds, ns_uri, local_name);

	if (NULL==element) {
		return FALSE;
	}

	return (CONG_ELEMENT_TYPE_SPAN == cong_dispspec_element_type(element));
}

/**
 * cong_dispspec_element_insert:
 * @ds:
 * @ns_uri:
 * @local_name:
 *
 * TODO: Write me
 * Returns:
 */
gboolean 
cong_dispspec_element_insert (CongDispspec *ds, 
			      const gchar *ns_uri, 
			      const char *local_name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, ns_uri, local_name);

	if (NULL==element) {
		return FALSE;
	}

	return (CONG_ELEMENT_TYPE_INSERT == cong_dispspec_element_type(element));
}
#endif

/* ################## Internal function definitions: ################################ */
/* Search tree callbacks: */
static gint key_compare_func (struct SearchTreeKey *a,
			      struct SearchTreeKey *b,
			      gpointer user_data)
{
	gint name_test;

	g_assert(a);
	g_assert(b);

	g_assert(a->local_name);
	g_assert(b->local_name);

	/* I believe the search will be faster if we order by local_name, then namespace: */
	name_test = strcmp(a->local_name, b->local_name);

	/* Names are different, sort initially on local_name ordering: */
	if (name_test!=0) {
		return name_test;
	}

	return cong_util_ns_uri_sort_order (a->ns_uri, 
					    b->ns_uri);
}

static void key_destroy_func (struct SearchTreeKey *key)
{
	g_assert(key);

	if (key->ns_uri) {
		g_free (key->ns_uri);
	}

	g_assert (key->local_name);

	g_free (key->local_name);

	g_free (key);
}

static void value_destroy_func (gpointer data) 
{
	g_assert(data);

	/* data is a CongDispspecElement; leave it alone */
}

/* Subroutines for generating a CongDispspec from a DTD: */
static void
element_callback_generate_dispspec_from_dtd (void *payload, void *ds, xmlChar * name)
{
	xmlElementPtr element;
	CongDispspec *dispspec;
	CongDispspecElement *ds_element;

	dispspec = (CongDispspec *) ds;
	element = (xmlElementPtr) payload;

	g_assert (dispspec);
	g_assert (element);

	ds_element = cong_dispspec_element_new (dispspec,
						/* element->prefix, */
						name,
						cong_dtd_element_guess_dispspec_type (element),
						TRUE);

	cong_dispspec_add_element (dispspec, ds_element);
}

/* Subroutines for converting XDS XML to a CongDispspec: */
static CongDispspec* parse_xmldoc(xmlDocPtr doc)
{
	CongDispspec* ds = cong_dispspec_new();

	/* Convert the XML into our internal representation: */
	if (doc->children)
	{
		xmlNodePtr xml_dispspec;

		for (xml_dispspec=doc->children; xml_dispspec; xml_dispspec = xml_dispspec->next) {
			if (cong_node_is_element (xml_dispspec, NULL, "dispspec")) {

				xmlNodePtr cur;
				
				DS_DEBUG_MSG1("got dispspec\n");

				for (cur = xml_dispspec->children; cur; cur=cur->next) {
					if (cong_node_is_element (cur, NULL, "element-list")) {
						
						xmlNodePtr xml_element;
						DS_DEBUG_MSG1("got element-list\n");

						ds->ns_uri = cong_node_get_attribute (cur, NULL, "nsURI");

						for (xml_element = cur->children; xml_element; xml_element=xml_element->next) {
							if (cong_node_is_element ( xml_element, NULL, "typical-prefix")) {
								gchar *attr = cong_node_get_attribute (xml_element, NULL, "prefix");

								ds->list_of_typical_prefix = g_list_append (ds->list_of_typical_prefix,
													    attr);
							}

							if (cong_node_is_element ( xml_element, NULL, "element")) {
								CongDispspecElement* element = cong_dispspec_element_from_xml (ds,
															       xml_element);
								
								cong_dispspec_add_element(ds,element);
							}
						}
						
					} else if (cong_node_is_element (cur, NULL, "metadata")) {
						parse_metadata(ds, doc, cur);
					} else if  (cong_node_is_element (cur, NULL, "serialisation")) {
						parse_serialisation (ds, doc, cur);
					} else if (cong_node_is_element (cur, NULL, "document-models")) {
						parse_document_models(ds, doc, cur);
					} else if (cong_node_is_element (cur, NULL,  "document-template")) {
						parse_template(ds, cur);
					}
				}
			}
		}
	}	

	return ds;	
}


static void
parse_metadata(CongDispspec *ds, xmlDocPtr doc, xmlNodePtr node)
{
	xmlNodePtr xml_element;
	DS_DEBUG_MSG1("got metadata\n");
	
	for (xml_element = node->children; xml_element; xml_element=xml_element->next) {
		if (0==strcmp(xml_element->name,"name")) {
			xmlChar* str = xmlNodeListGetString(doc, xml_element->xmlChildrenNode, 1);
			if (str) {
				ds->name = g_strdup(str);
			}
		}

		if (0==strcmp(xml_element->name,"description")) {
			xmlChar* str = xmlNodeListGetString(doc, xml_element->xmlChildrenNode, 1);
			if (str) {
				ds->desc = g_strdup(str);
			}
		}
	}
}


static const CongEnumMapping document_model_enum_mapping[] =
{
	{"dtd", CONG_DOCUMENT_MODE_TYPE_DTD},
	{"w3c-xml-schema", CONG_DOCUMENT_MODE_TYPE_W3C_XML_SCHEMA},
	{"relax-ng-schema", CONG_DOCUMENT_MODE_TYPE_RELAX_NG_SCHEMA},
};

#include "gxx-object-from-xml-tree.h"
#include "cong-dispspec-gxx.h"

#include "gxx-object-to-xml-tree.h"
#include "cong-dispspec-gxx.h"

static void 
parse_serialisation (CongDispspec *ds, 
		     xmlDocPtr doc, 
		     xmlNodePtr node)
{
	guint index;
	xmlNodePtr xml_element;
	DS_DEBUG_MSG1("got serialisation\n");

	g_assert (0==ds->num_serialisation_formats);
	g_assert (NULL==ds->serialisation_formats);
	
	for (xml_element = node->children; xml_element; xml_element=xml_element->next) {
		if (cong_node_is_element (xml_element, NULL,"format")) {
			ds->num_serialisation_formats++;
		}
	}

	ds->serialisation_formats = g_new0(CongSerialisationFormatPtr, ds->num_serialisation_formats);

	index = 0;
	for (xml_element = node->children; xml_element; xml_element=xml_element->next) {
		if (cong_node_is_element (xml_element, NULL,"format")) {
			ds->serialisation_formats[index++] = gxx_generated_object_from_xml_tree_fn_serialisation_format (xml_element);
		}
	}
}

static void 
parse_document_models (CongDispspec *ds, 
		       xmlDocPtr doc, 
		       xmlNodePtr node)
{
	xmlNodePtr xml_element;

	DS_DEBUG_MSG1("got document-models\n");

	for (xml_element = node->children; xml_element; xml_element=xml_element->next) {
		if (0==strcmp(xml_element->name,"external-document-model")) {
			parse_external_document_model (ds, 
						       doc, 
						       xml_element);
		}
	}

}


static void 
parse_external_document_model (CongDispspec *ds, 
			       xmlDocPtr doc, 
			       xmlNodePtr node)
{
	gchar *type;

	DS_DEBUG_MSG1("got external-document-model\n");
	type = cong_node_get_attribute (node, NULL, "type");

	if (type) {
		CongDocumentModelType model_type = cong_enum_mapping_lookup (document_model_enum_mapping,
										  sizeof(document_model_enum_mapping)/sizeof(CongEnumMapping),
										  "type",
										  CONG_DOCUMENT_MODE_TYPE_DTD);
#if 1
		ds->document_models[model_type] = gxx_generated_object_from_xml_tree_fn_external_document_model (node);
#else

		gchar *public_id = cong_node_get_attribute (node, "public-id");
		gchar *system_id = cong_node_get_attribute (node, "system-id");

		ds->document_models[model_type] = cong_external_document_model_new (model_type,
										    public_id,
										    system_id);
#endif
		g_free (type);

	} else {
		g_message ("Missing document-model type");
	}
}

static void
parse_template(CongDispspec *ds, xmlNodePtr node)
{
	for (node = node->children; node; node=node->next) {
		if(node->type == XML_ELEMENT_NODE)
		{
			ds->template = xmlCopyNode(node, TRUE);
			return;
		}
	}
}
	
/**
 * col_to_gcol:
 * @gcol:
 * @col:
 *
 * TODO: Write me
 */
void 
col_to_gcol(GdkColor *gcol, unsigned int col)
{
  gcol->blue = (col << 8) & 0xff00;
  gcol->green = (col & 0xff00);
  gcol->red = (col >> 8) & 0xff00;
}

/**
 * cong_external_document_model_get_public_id:
 * @model:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_external_document_model_get_public_id (const CongExternalDocumentModel* model)
{
	g_return_val_if_fail (model, NULL);

	return model->public_id;
}

/**
 * cong_external_document_model_get_system_id:
 * @model:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_external_document_model_get_system_id (const CongExternalDocumentModel* model)
{
	g_return_val_if_fail (model, NULL);

	return model->system_id;
}

/* Subroutines for converting a CongDispspec to XML XDS: */
static void add_xml_for_metadata (xmlDocPtr xml_doc, 
				  CongNodePtr root, 
				  CongDispspec *dispspec)
{
	CongNodePtr metadata;
	
	metadata = xmlNewDocNode(xml_doc,
				 NULL,
				 "metadata",
				 NULL);			
	xmlAddChild(root, metadata);
	
	if (dispspec->name) {
		
		xmlAddChild (metadata, 
			     xmlNewDocRawNode (xml_doc,
					       NULL,
					       "name",
					       dispspec->name)
			     ); 
	}
	if (dispspec->desc) {
		xmlAddChild (metadata, 
			     xmlNewDocRawNode (xml_doc,
					       NULL,
					       "description",
					       dispspec->desc)
			     );
	}
	
	/* FIXME: we can't yet save the icon name */

}

static void 
add_xml_for_serialisation_formats  (xmlDocPtr xml_doc, 
				    CongNodePtr root, 
				    CongDispspec *dispspec)
{
	if (dispspec->serialisation_formats) {
		guint i;
		CongNodePtr node_serialisation;

		node_serialisation = xmlNewDocNode (xml_doc,
						    NULL,
						    "serialisation",
						    NULL);
		xmlAddChild (root, 
			     node_serialisation);

		for (i=0;i<dispspec->num_serialisation_formats; i++) {
			CongSerialisationFormat *format = dispspec->serialisation_formats[i];
			g_assert (format);

			xmlAddChild (node_serialisation,
				     gxx_generated_object_to_xml_tree_fn_serialisation_format (format, xml_doc));
		}
	}
}

static void
add_xml_for_document_models (xmlDocPtr xml_doc, 
			     CongNodePtr root, 
			     CongDispspec *dispspec)
{
	CongNodePtr node_document_models;
	int i;
	
	node_document_models = xmlNewDocNode(xml_doc,
					     NULL,
					     "document-models",
					     NULL);			
	xmlAddChild (root, 
		     node_document_models);
	
	for (i=0; i<NUM_CONG_DOCUMENT_MODEL_TYPES; i++) {
		CongDocumentModelType model_type = (CongDocumentModelType)i;

		const CongExternalDocumentModel* model = cong_dispspec_get_external_document_model (dispspec,
												    model_type);

		if (model) {
			xmlAddChild (node_document_models,
				     gxx_generated_object_to_xml_tree_fn_external_document_model (model, xml_doc));
		}		
	}
}


/* Subroutines for generating a CongDispspec from arbitrary XML docs: */
/** 
    Does the string contain any non-whitespace characters? 
*/
static gboolean
contains_text (const xmlChar* string)
{
	const gchar *iter;

	g_return_val_if_fail(string, FALSE);

	iter = string;

	while (*iter) {
		if ( !g_unichar_isspace(g_utf8_get_char(iter))) {
			return TRUE;
		}

		iter = g_utf8_next_char(iter);
	}

	return FALSE;
}

/**
   Does the string contain any carriage returns?
 */
static gboolean
contains_carriage_return (const xmlChar* string)
{
	const gchar *iter;

	g_return_val_if_fail(string, FALSE);

	iter = string;

	while (*iter) {
		if ( '\n' == g_utf8_get_char(iter)) {
			return TRUE;
		}

		iter = g_utf8_next_char(iter);
	}

	return FALSE;	
}

static void
promote_element (CongDispspec * dispspec, 
		 CongDispspecElement * element,
		 xmlNodePtr node)
{

	if (!strcmp (cong_dispspec_element_get_local_name (element), xmlDocGetRootElement (node->doc)->name)) {
		return;
	}

	switch (cong_dispspec_type (dispspec, cong_node_get_ns_uri (node->parent), node->parent->name))
	{
	default: break;
		case CONG_ELEMENT_TYPE_SPAN:
		{
			if (contains_carriage_return(xmlNodeGetContent (node)))
			{
				CongDispspecElement *ds_element = cong_dispspec_element_new (dispspec,
											     node->parent->name,
											     CONG_ELEMENT_TYPE_STRUCTURAL,
											     TRUE);
				g_assert (ds_element);
				cong_dispspec_add_element (dispspec, ds_element);
			}
			break;
		}
		case CONG_ELEMENT_TYPE_STRUCTURAL:
		{
			if (contains_text (xmlNodeGetContent (node)))
			{
				CongDispspecElement *ds_element = cong_dispspec_element_new (dispspec,
											     node->parent->name,
											     CONG_ELEMENT_TYPE_STRUCTURAL,
											     TRUE);
				g_assert (ds_element);
				cong_dispspec_add_element (dispspec, ds_element);
			}
			break;
		}
	}
}

static void
handle_elements_from_xml (CongDispspec * dispspec, xmlNodePtr cur)
{
	CongDispspecElement *element;
	g_assert (dispspec);
	g_assert (dispspec->search_tree);

	if (cur) {
		if (xmlNodeIsText (cur)) {
			if (cur->parent->type==XML_ELEMENT_NODE) {
				element =  cong_dispspec_lookup_element (dispspec, cong_node_get_ns_uri (cur->parent), cur->parent->name);
				if (element) {
					promote_element (dispspec, element, cur);
				}
				else if (contains_text (xmlNodeGetContent (cur))) {
					if (contains_carriage_return(xmlNodeGetContent (cur))) {
						CongDispspecElement *ds_element = cong_dispspec_element_new (dispspec,
													     cur->parent->name,
													     CONG_ELEMENT_TYPE_STRUCTURAL,
													     TRUE);
						g_assert (ds_element);
						cong_dispspec_add_element (dispspec, ds_element);
					} else {
						CongDispspecElement *ds_element = cong_dispspec_element_new (dispspec,
													     cur->parent->name,
													     CONG_ELEMENT_TYPE_SPAN,
													     TRUE);
						g_assert (ds_element);
						cong_dispspec_add_element (dispspec, ds_element);
					}
				} else {
					CongDispspecElement *ds_element = cong_dispspec_element_new (dispspec,
												     cur->parent->name,
												     CONG_ELEMENT_TYPE_STRUCTURAL,
												     TRUE);
					g_assert (ds_element);
					cong_dispspec_add_element (dispspec, ds_element);
				}
			}
		}
	}

	/* Recurse over children: */
	
	cur = cur->xmlChildrenNode;
	
	while (cur != NULL) {
		handle_elements_from_xml (dispspec, cur);
		cur = cur->next;
	}
	return;
}

static void
ensure_all_elements_covered (CongDispspec * dispspec, xmlNodePtr cur)
{
	CongDispspecElement *element;

	g_assert (dispspec);
	g_assert (cur);

	if (CONG_NODE_TYPE_ELEMENT==cong_node_type(cur)) {
		element = cong_dispspec_lookup_element (dispspec, cong_node_get_ns_uri (cur), cong_node_get_local_name (cur));
		if (NULL==element) {
			/* Then we've found an element that doesn't have any handler in the dispspec; better create a structural tag for it... */
			CongDispspecElement *ds_element = cong_dispspec_element_new (dispspec,
										     cong_node_get_local_name (cur),
										     CONG_ELEMENT_TYPE_STRUCTURAL,
										     TRUE);
			g_assert (ds_element);
			cong_dispspec_add_element (dispspec, ds_element);
		}
	}

	/* traverse children: */
	cur = cur->xmlChildrenNode;
	
	while (cur != NULL) {
		ensure_all_elements_covered (dispspec, cur);
		cur = cur->next;
	}
}

static void
xml_to_dispspec (CongDispspec *dispspec, 
		 xmlDocPtr doc, 
		 xmlDtdPtr dtd,
		 const gchar *extension)
{
	if (extension) {
		g_assert (0==dispspec->num_serialisation_formats);
		g_assert (NULL==dispspec->serialisation_formats);

		if (0!=strcmp("xml", extension)) {
			dispspec->num_serialisation_formats = 1;
			dispspec->serialisation_formats = g_new0 (CongSerialisationFormatPtr, 1);
			dispspec->serialisation_formats[0] = cong_serialisation_format_new (extension);
		}
	}

	if (doc)
	{
		xmlNodePtr root_element = xmlDocGetRootElement(doc);

		CongDispspecElement *ds_element = cong_dispspec_element_new (dispspec,
									     root_element->name,
									     CONG_ELEMENT_TYPE_STRUCTURAL,
									     TRUE);
		g_assert (ds_element);
		cong_dispspec_add_element (dispspec, ds_element);
	}

	if (dtd)
	{
		dispspec->document_models[CONG_DOCUMENT_MODE_TYPE_DTD] = make_model_from_dtd (dtd);

		xmlHashScan (dtd->elements, element_callback_generate_dispspec_from_dtd, dispspec);
	}

	if (doc)
	{
		dispspec->name = g_strdup(doc->URL);
		handle_elements_from_xml (dispspec, xmlDocGetRootElement (doc));
		ensure_all_elements_covered(dispspec, xmlDocGetRootElement (doc));
	}
}

static xmlDtdPtr
load_dtd (xmlDocPtr doc)
{
	if (doc->intSubset)
	{
		return xmlParseDTD (doc->intSubset->SystemID, doc->intSubset->SystemID);
	}
	else
	{
		if (doc->extSubset)
		{
			return xmlParseDTD (doc->extSubset->SystemID,
					    doc->extSubset->SystemID);
		}
	}
	return NULL;
}

/**
 * cong_dispspec_get_template:
 * @ds:
 *
 * TODO: Write me
 * Returns:
 */
xmlNodePtr
cong_dispspec_get_template(const CongDispspec *ds)
{
	return  ds->template;
}

struct CoverageData
{
	guint total_elements;
	guint covered_elements;
};

static void
calc_coverage_recursive (const CongDispspec *ds,
			 CongNodePtr node,
			 struct CoverageData *coverage_data)
{
	CongNodePtr iter;

	g_assert (node);

	if (node->type == XML_ELEMENT_NODE) {
		/* Only deal with elements outside a namespace: */
		if (NULL==node->ns) {
			coverage_data->total_elements++;
			
			if (NULL!=cong_dispspec_lookup_node (ds, node)) {
				coverage_data->covered_elements++;
			}
		}
	}

	/* FIXME:  this gives slightly skewed results for entities */
	if (node->type==XML_ENTITY_REF_NODE) {
		calc_coverage_recursive (ds, 
					 node->children,
					 coverage_data);
	} else {						  
		for (iter = node->children; iter; iter=iter->next) {
			calc_coverage_recursive (ds,
						 iter,
						 coverage_data);
		}
	}
}

/**
 * cong_dispspec_calculate_coverage:
 * @ds: a dispspec
 * @xml_doc: a document
 *
 * Calculate the proportion of elements without namespaces in the document that
 * are covered by this dispspec.
 * Returns: a fraction between 0.0 and 1.0.  If all elements have namespaces, 0.0 is returned
 */
gdouble
cong_dispspec_calculate_coverage (const CongDispspec *ds,
				  xmlDocPtr xml_doc)
{
	struct CoverageData coverage_data;

	g_return_val_if_fail (ds, 0.0);
	g_return_val_if_fail (xml_doc, 0.0);

	coverage_data.total_elements = 0;
	coverage_data.covered_elements = 0;

	calc_coverage_recursive (ds,
				 (CongNodePtr)xml_doc,
				 &coverage_data);

	if (coverage_data.total_elements>0) {
		return ((gdouble)coverage_data.covered_elements)/((gdouble)coverage_data.total_elements);
	} else {
		return 0.0;
	}
}
