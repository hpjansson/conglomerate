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

#include <stdio.h>
#include <glib.h>
#include <libxml/parser.h>
#include "cong-node.h"
#include "cong-app.h"
#include "cong-eel.h"
#include "cong-dtd.h"
#include "cong-util.h"
#include "cong-enum-mapping.h"

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
struct CongDispspecElementHeaderInfo
{
	gchar *xpath; /* if present, this is the XPath to use when determining the title of the tag */
	gchar *tagname; /* if xpath not present, then look for this tag below the main tag (deprecated) */
};

struct CongDispspecElement
{
	gchar *xmlns;
	gchar *tagname;
	gchar *username;
	gchar *short_desc;
	GdkPixbuf *icon16;
	enum CongWhitespaceHandling whitespace;

	enum CongElementType type;
	gboolean collapseto;

#if NEW_LOOK
	GdkColor col_array[CONG_DISPSPEC_GC_USAGE_NUM];
	GdkGC* gc_array[CONG_DISPSPEC_GC_USAGE_NUM];
#else
	GdkColor col;
	GdkGC* gc;
#endif

	CongDispspecElementHeaderInfo *header_info;

	gchar *editor_plugin_id;
	gchar *property_dialog_plugin_id;

	struct CongDispspecElement* next;	
};

struct SearchTreeKey
{
	gchar* xmlns;
	gchar* name;
};

typedef CongSerialisationFormat* CongSerialisationFormatPtr;

struct CongSerialisationFormat
{
	gchar *extension;
};

struct CongExternalDocumentModel
{
	enum CongDocumentModelType model_type;
	gchar *public_id;
	gchar *system_id;
};

struct CongDispspec
{
	/* The serialisation formats: */
	guint num_serialisation_formats;
	CongSerialisationFormat **serialisation_formats;

	/* The CongDocumentModels: */
	CongExternalDocumentModel *document_models[NUM_CONG_DOCUMENT_MODEL_TYPES];

	/* Implementation is an "intrusive list" of CongDispspecElement structs */ 
	CongDispspecElement* first;
	CongDispspecElement* last;

	/* We have a search tree, indexed by SearchTreeKeys */
	GTree *search_tree; 

	gchar *name;
	gchar *desc;
	gchar *filename_extension;
	GdkPixbuf *icon;

	CongDispspecElement *paragraph;
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

static CongSerialisationFormat*
parse_format (CongDispspec *ds, 
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

static CongDispspecElement*
cong_dispspec_element_new_from_xml_element (xmlDocPtr doc, 
					    xmlNodePtr xml_element);

/* Subroutines for converting a CongDispspec to XDS XML: */
static const gchar* element_type_to_string(enum CongElementType type);

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
static void 
add_xml_for_element (xmlDocPtr xml_doc, 
		     CongNodePtr element_list, 
		     CongDispspecElement *element);

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
cong_external_document_model_new (enum CongDocumentModelType model_type,
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

#if NEW_LOOK
/* Hackish colour calculations in RGB space (ugh!) */
static void generate_col(GdkColor *dst, const GdkColor *src, float bodge_factor)
{
	dst->red = src->red / bodge_factor;
	dst->green = src->green / bodge_factor;
	dst->blue = src->blue / bodge_factor;
}


unsigned int hacked_cols[3][CONG_DISPSPEC_GC_USAGE_NUM] =
{
	/* Blueish, section 1 in Joakim's mockup: */
	{ 0x6381ff, 0xd5d2ff, 0xe6e2ff,	0x414083 },

	/* Brownish; section 2 in the mockup: */
	{ 0xd5b69c, 0xe6ded5, 0xeee6de, 0x836141 },

	/* Dark brown; the underline in the mockup: */
	{ 0x632829, 0x632829, 0x632829, 0x632829 }
};

static void get_col(GdkColor *dst, const GdkColor *src, enum CongDispspecGCUsage usage)
{
	/* pick one of the test colour tables based on a dodgy hashing of the source colour: */
	col_to_gcol(dst, hacked_cols[(src->red>>8)%2][usage]);
}
#endif /* #if NEW_LOOK */

GdkGC*
generate_gc_for_col (const GdkColor *col)
{
	GdkGC *gc;

	g_return_val_if_fail (col, NULL);

	gc = gdk_gc_new(cong_gui_get_a_window()->window);
	gdk_gc_copy(gc, cong_gui_get_a_window()->style->white_gc);
	gdk_colormap_alloc_color(cong_gui_get_a_window()->style->colormap, col, FALSE, TRUE);
	gdk_gc_set_foreground(gc, col);	

	return gc;
}

static void cong_dispspec_element_init_col(CongDispspecElement* element, unsigned int col)
{
	g_assert(element);

#if NEW_LOOK
	{
		GdkColor gdk_col;
		int i;

		col_to_gcol(&gdk_col, col);

		for (i=0; i<CONG_DISPSPEC_GC_USAGE_NUM; i++) {
			GdkColor this_col;

#if 1
			get_col(&this_col, &gdk_col, i);
#else
			/* failed attempt to calculate relative colours */
			switch (i) {
			default: g_assert(0);
			case CONG_DISPSPEC_GC_USAGE_BOLD_LINE:
				/* Double intensity for this: */
				generate_col(&this_col,&gdk_col, 2.0f);
				break;

			case CONG_DISPSPEC_GC_USAGE_DIM_LINE:
				/* Use the exact colour for this */
				generate_col(&this_col,&gdk_col, 1.0f);
				break;

			case CONG_DISPSPEC_GC_USAGE_BACKGROUND:
				/* Use a 50-50 blend with white of the colour for this? */
				generate_col(&this_col,&gdk_col, 0.5);
				break;

			case CONG_DISPSPEC_GC_USAGE_TEXT:
				/* Triple intensity for this? */
				generate_col(&this_col,&gdk_col, 3.0f);
				break;
			}
#endif

			element->col_array[i] = this_col;
			element->gc_array[i] = generate_gc_for_col (&this_col);
		}
	}
#else
	col_to_gcol(&element->col, col);

	/* We don't make any attempt to share GCs between different elements for now */
	element->gc = gdk_gc_new(cong_gui_get_a_window()->window);
	gdk_gc_copy(element->gc, cong_gui_get_a_window()->style->white_gc);
	gdk_colormap_alloc_color(cong_gui_get_a_window()->style->colormap, &element->col, FALSE, TRUE);
	gdk_gc_set_foreground(element->gc, &element->col);
#endif	
}

/* Exported functions: */

/* Barebones constructor, used to implement other ones: */
CongDispspec* cong_dispspec_new(void)
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
GnomeVFSResult cong_dispspec_new_from_xds_file(GnomeVFSURI *uri, CongDispspec** ds)
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

CongDispspec* cong_dispspec_new_from_xds_buffer(const char *buffer, size_t size)
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
CongDispspec* cong_dispspec_new_generate_from_dtd (xmlDtdPtr dtd, 
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

CongDispspec *
cong_dispspec_new_generate_from_xml_file (xmlDocPtr doc,
					  const gchar *extension)
{
	CongDispspec *dispspec;

	g_return_val_if_fail (doc, NULL);
	g_return_val_if_fail (extension, NULL);

	dispspec = cong_dispspec_new();

	xml_to_dispspec (dispspec, 
			 doc, 
			 load_dtd (doc), 
			 extension);

	return dispspec;
}

void cong_dispspec_delete (CongDispspec *dispspec)
{
	CongDispspecElement *element;
	CongDispspecElement *next;

	g_return_if_fail(dispspec);

	/* FIXME: is this causing heap corruption? */
#if 0
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
#endif
}

xmlDocPtr cong_dispspec_make_xml(CongDispspec *dispspec)
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

		/* The elements: */
		{
			CongDispspecElement* element;
			
			for (element = dispspec->first; element; element=element->next) {
				add_xml_for_element(xml_doc, element_list, element);
			}
		}	
	}		

	/* FIXME: add <document-template> if necessary */

	return xml_doc;	
}

/* Data for the dispspec: */
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

guint
cong_dispspec_get_num_serialisation_formats (const CongDispspec *ds)
{
	g_return_val_if_fail (ds, 0);

	return ds->num_serialisation_formats;
}

const CongSerialisationFormat*
cong_dispspec_get_serialisation_format (const CongDispspec *ds,
					guint index)
{
	g_return_val_if_fail (ds, NULL);

	g_assert (index<ds->num_serialisation_formats);

	return ds->serialisation_formats[index];
}

/* Returns NULL if it can't find a serialisation format with that extension */
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
gboolean
cong_dispspec_matches_filename_extension (const CongDispspec *ds,
					  const gchar *extension)
{
	g_return_val_if_fail (ds, FALSE);

	return (NULL!=cong_dispspec_lookup_filename_extension (ds,
							       extension));
}

const CongExternalDocumentModel*
cong_dispspec_get_external_document_model (const CongDispspec *ds,
					   enum CongDocumentModelType model_type)
{
	g_return_val_if_fail(ds, NULL);
	g_return_val_if_fail(model_type<NUM_CONG_DOCUMENT_MODEL_TYPES, NULL);
	
	return ds->document_models[model_type];
}

GdkPixbuf*
cong_dispspec_get_icon(const CongDispspec *ds)
{
	g_return_val_if_fail(ds, NULL);

	if (ds->icon) {
		g_object_ref(G_OBJECT(ds->icon));
	}
	return ds->icon;
}

/* Getting at elements within a dispspec */
CongDispspecElement*
cong_dispspec_lookup_element(const CongDispspec *ds, const gchar* xmlns, const gchar* tagname)
{
	/* We use the GTree search structure for speed */
	CongDispspecElement *element;
	struct SearchTreeKey key;

	g_return_val_if_fail(ds, NULL);
	g_return_val_if_fail(tagname, NULL);

	g_assert(ds->search_tree);

	key.xmlns = (gchar*)xmlns;
	key.name = (gchar*)tagname;

	element =  g_tree_lookup(ds->search_tree, &key);

	return element;
}

CongDispspecElement*
cong_dispspec_lookup_node(const CongDispspec *ds, CongNodePtr node)
{
	g_return_val_if_fail(ds, NULL);
	g_return_val_if_fail(node, NULL);

	return cong_dispspec_lookup_element(ds, cong_node_xmlns(node), cong_node_name(node));
}

enum CongElementType
cong_dispspec_type(CongDispspec *ds, const gchar* xmlns, const gchar* tagname)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, xmlns, tagname);

	if (NULL==element) {
		return CONG_ELEMENT_TYPE_UNKNOWN;
	}

	return cong_dispspec_element_type(element);
}

CongDispspecElement*
cong_dispspec_get_first_element(CongDispspec *ds)
{
	g_return_val_if_fail(ds, NULL);

	return ds->first;
}

CongDispspecElement*
cong_dispspec_get_paragraph(CongDispspec *ds)
{
	g_return_val_if_fail(ds, NULL);

	return ds->paragraph;
}

/* Manipulating a dispspec: */
void cong_dispspec_add_element (CongDispspec* ds, 
				CongDispspecElement* element)
{
	struct SearchTreeKey *key;

	g_return_if_fail(ds);
	g_return_if_fail(element);

	g_assert(element->next==NULL);
	g_assert(element->tagname);

	if (ds->first) {
		g_assert(ds->last);
		ds->last->next = element;
	} else {
		ds->first = element;
	}

	ds->last = element;

	key = g_new0(struct SearchTreeKey, 1);
	if (element->xmlns) {
		key->xmlns = g_strdup(element->xmlns);
	}
	key->name = element->tagname;

	g_tree_insert(ds->search_tree, key, element);
}

/* Various functions that may get deprecated at some point: */
#if NEW_LOOK
#if 0
GdkGC *cong_dispspec_gc_get(CongDispspec *ds, CongNodePtr x, enum CongDispspecGCUsage usage)
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
GdkGC *cong_dispspec_name_gc_get(CongDispspec *ds, CongNodePtr t, int tog)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, cong_node_name(t));

	if (element) {
		return cong_dispspec_element_gc(element);
	} else {
		return NULL;
	}
}


GdkGC *cong_dispspec_gc_get(CongDispspec *ds, CongNodePtr x, int tog)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, xml_frag_name_nice(x));

	if (element) {
		return cong_dispspec_element_gc(element);
	} else {
		return NULL;
	}
}
#endif

const gchar *cong_dispspec_name_get(CongDispspec *ds, CongNodePtr node)
{
	CongDispspecElement* element;

	g_return_val_if_fail(ds, NULL);
	g_return_val_if_fail(node, NULL);
	g_return_val_if_fail(cong_node_type(node)==CONG_NODE_TYPE_ELEMENT, NULL);

	element = cong_dispspec_lookup_element(ds, cong_node_xmlns(node), xml_frag_name_nice(node));
	if (element) {
		return cong_dispspec_element_username(element);
	}
  
	return(xml_frag_name_nice(node));
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
gboolean cong_dispspec_element_structural(CongDispspec *ds, const gchar *xmlns, const char *name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, xmlns, name);

	if (NULL==element) {
		return FALSE;
	}

	return (CONG_ELEMENT_TYPE_STRUCTURAL == cong_dispspec_element_type(element));
}


gboolean cong_dispspec_element_collapse(CongDispspec *ds, const gchar *xmlns, const char *name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, xmlns, name);

	if (NULL==element) {
		return FALSE;
	}
	
	return cong_dispspec_element_collapseto(element);
}


gboolean cong_dispspec_element_span(CongDispspec *ds, const gchar *xmlns, const char *name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, xmlns, name);

	if (NULL==element) {
		return FALSE;
	}

	return (CONG_ELEMENT_TYPE_SPAN == cong_dispspec_element_type(element));
}


gboolean cong_dispspec_element_insert(CongDispspec *ds, const gchar *xmlns, const char *name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, xmlns, name);

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

	g_assert(a->name);
	g_assert(b->name);

	/* I believe the search will be faster if we order by name, then namespace: */
	name_test = strcmp(a->name, b->name);

	/* Names are different, sort initially on name ordering: */
	if (name_test!=0) {
		return name_test;
	}

	/* Names are the same; continue searching by namespace; order the NULL namespace before all others: */
	if (NULL == a->xmlns) {
		if (NULL == b->xmlns) {
			return 0;
		} else {
			return -1; /* a is less than b */
		}
	} else {
		/* "a" has non-NULL namespace: */
		if (NULL == b->xmlns) {
			return 1; /* a is greater than b */
		} else {
			/* Both have non-NULL namespaces; order based on them: */
			return strcmp(a->xmlns, b->xmlns);
		}
	}
}

static void key_destroy_func (struct SearchTreeKey *key)
{
	g_assert(key);

	if (key->xmlns) {
		g_free(key->xmlns);
	}

	g_assert(key->name);

	g_free(key->name);

	g_free(key);
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

	ds_element = cong_dispspec_element_new (element->prefix,
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
			if (0==strcmp(xml_dispspec->name,"dispspec")) {

				xmlNodePtr cur;
				
				DS_DEBUG_MSG1("got dispspec\n");

				for (cur = xml_dispspec->children; cur; cur=cur->next) {
					if (0==strcmp(cur->name,"element-list")) {
						
						xmlNodePtr xml_element;
						DS_DEBUG_MSG1("got element-list\n");

						for (xml_element = cur->children; xml_element; xml_element=xml_element->next) {
							if(xml_element->type==XML_ELEMENT_NODE)
							{
								CongDispspecElement* element = cong_dispspec_element_new_from_xml_element(doc, xml_element);
								
								cong_dispspec_add_element(ds,element);

								if (element->type==CONG_ELEMENT_TYPE_PARAGRAPH){
									ds->paragraph=element;
								}
							}
						}
						
					} else if (0==strcmp(cur->name,"metadata")) {
						parse_metadata(ds, doc, cur);
					} else if  (0==strcmp(cur->name,"serialisation")) {
						parse_serialisation (ds, doc, cur);
					} else if (0==strcmp(cur->name,"document-models")) {
						parse_document_models(ds, doc, cur);
					} else if (0==strcmp(cur->name, "document-template")) {
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
		if (cong_node_is_tag (xml_element, NULL,"format")) {
			ds->num_serialisation_formats++;
		}
	}

	ds->serialisation_formats = g_new0(CongSerialisationFormatPtr, ds->num_serialisation_formats);

	index = 0;
	for (xml_element = node->children; xml_element; xml_element=xml_element->next) {
		if (cong_node_is_tag (xml_element, NULL,"format")) {
			ds->serialisation_formats[index++] = parse_format (ds, doc, xml_element);
		}
	}
}

static CongSerialisationFormat*
parse_format (CongDispspec *ds, 
	      xmlDocPtr doc, 
	      xmlNodePtr node)
{
	gchar *extension;

	DS_DEBUG_MSG1("got format\n");

	extension = cong_node_get_attribute (node,
					     "extension");

	if (extension) {
		CongSerialisationFormat* result;

		result = cong_serialisation_format_new (extension);

		g_free (extension);

		return result;
	} else {
		return NULL;
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

static const CongEnumMapping document_model_enum_mapping[] =
{
	{"dtd", CONG_DOCUMENT_MODE_TYPE_DTD},
	{"w3c-xml-schema", CONG_DOCUMENT_MODE_TYPE_W3C_XML_SCHEMA},
	{"relax-ng-schema", CONG_DOCUMENT_MODE_TYPE_RELAX_NG_SCHEMA},
};


static void 
parse_external_document_model (CongDispspec *ds, 
			       xmlDocPtr doc, 
			       xmlNodePtr node)
{
	gchar *type;

	DS_DEBUG_MSG1("got external-document-model\n");


	type = cong_node_get_attribute (node, "type");

	if (type) {
		enum CongDocumentModelType model_type = cong_enum_mapping_lookup (document_model_enum_mapping,
										  sizeof(document_model_enum_mapping)/sizeof(CongEnumMapping),
										  "type",
										  CONG_DOCUMENT_MODE_TYPE_DTD);
		gchar *public_id = cong_node_get_attribute (node, "public-id");
		gchar *system_id = cong_node_get_attribute (node, "system-id");

		ds->document_models[model_type] = cong_external_document_model_new (model_type,
										    public_id,
										    system_id);
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
	
unsigned int get_rgb_hex(unsigned char *s)
{
  unsigned int col;

  col = 0;
  while (*s)
    {
      col <<= 4;
      if (isalpha(*s)) col |= tolower(*s) - 'a' + 10;
      else if (isdigit(*s)) col |= *s - '0';
      s++;
    }
  
  return(col);
}


void col_to_gcol(GdkColor *gcol, unsigned int col)
{
  gcol->blue = (col << 8) & 0xff00;
  gcol->green = (col & 0xff00);
  gcol->red = (col >> 8) & 0xff00;
}

/*******************************
   cong_dispspec_element stuff: 
*******************************/

/* Construction  */
CongDispspecElement*
cong_dispspec_element_new (const gchar* xmlns, 
			   const gchar* tagname, 
			   enum CongElementType type,
			   gboolean autogenerate_username)
{
	CongDispspecElement* element;

	g_return_val_if_fail(tagname,NULL);

	element = g_new0(CongDispspecElement,1);

	g_message("cong_dispspec_element_new (\"%s\",\"%s\",)", xmlns, tagname);

	if (xmlns) {
		element->xmlns = g_strdup(xmlns);
	}
	element->tagname = g_strdup(tagname);	

	if (autogenerate_username) {

		/* Try to prettify the username if possible: */
		element->username = cong_eel_prettify_xml_name_with_header_capitalisation(tagname);

	} else {
		/* username remains NULL */
	}

	element->whitespace = CONG_WHITESPACE_NORMALIZE;
	element->type = type;

	/* Extract colour: */
	{
#if NEW_LOOK
		unsigned int col = 0x00000000;  /* Black is default for the new look */
#else
		unsigned int col = 0x00ffffff;  /* White is default */
#endif

		cong_dispspec_element_init_col(element, col);
	}

	return element;
}

/* Destruction  */
void 
cong_dispspec_element_destroy (CongDispspecElement *element)
{
	g_return_if_fail (element);

	if (element->xmlns) {
		g_free (element->xmlns);
	}
	if (element->tagname) {
		g_free (element->tagname);
	}
	if (element->username) {
		g_free (element->username);
	}
	if (element->short_desc) {
		g_free (element->short_desc);
	}

	if (element->icon16) {
		g_object_unref (G_OBJECT(element->icon16));
	}

	/* FIXME: need to clean up this stuff: */
#if 0
#if NEW_LOOK
	GdkColor col_array[CONG_DISPSPEC_GC_USAGE_NUM];
	GdkGC* gc_array[CONG_DISPSPEC_GC_USAGE_NUM];
#else
	GdkColor col;
	GdkGC* gc;
#endif

	CongDispspecElementHeaderInfo *header_info;
#endif

	if (element->editor_plugin_id) {
		g_free (element->editor_plugin_id);
	}
	if (element->property_dialog_plugin_id) {
		g_free (element->property_dialog_plugin_id);
	}

	g_free (element);
	/* FIXME:  do we need to remove from the list? */
}


const gchar*
cong_dispspec_element_get_xmlns(CongDispspecElement *element)
{
	g_return_val_if_fail(element, NULL);

	return element->xmlns;
}

const gchar*
cong_dispspec_element_tagname(CongDispspecElement* element)
{
	g_return_val_if_fail(element, NULL);

	return element->tagname;
}

const gchar*
cong_dispspec_element_username(CongDispspecElement* element)
{
	g_return_val_if_fail(element, NULL);

	return element->username;
}

const gchar*
cong_dispspec_element_get_description(CongDispspecElement *element)
{
	g_return_val_if_fail(element,NULL);

	return element->short_desc;
}

GdkPixbuf*
cong_dispspec_element_get_icon(CongDispspecElement *element)
{
	g_return_val_if_fail(element, NULL);

	if (element->icon16) {
		g_object_ref(G_OBJECT(element->icon16));
	}
	return element->icon16;
}

CongDispspecElement*
cong_dispspec_element_next(CongDispspecElement* element)
{
	g_return_val_if_fail(element, NULL);

	return element->next;
}

enum CongElementType
cong_dispspec_element_type(CongDispspecElement *element)
{
	g_return_val_if_fail(element, CONG_ELEMENT_TYPE_UNKNOWN);

	return element->type;
}

enum CongWhitespaceHandling
cong_dispspec_element_get_whitespace (CongDispspecElement *element)
{
	g_return_val_if_fail (element, CONG_WHITESPACE_NORMALIZE);

	return element->whitespace;
}

void
cong_dispspec_element_set_whitespace (CongDispspecElement *element,
				      enum CongWhitespaceHandling whitespace)
{	
	g_return_if_fail (element);

	element->whitespace = whitespace;
}


gboolean
cong_dispspec_element_collapseto(CongDispspecElement *element)
{
	return element->collapseto;
}

gboolean
cong_dispspec_element_is_structural(CongDispspecElement *element)
{
	g_return_val_if_fail(element, FALSE);

	if (CONG_ELEMENT_TYPE_STRUCTURAL == cong_dispspec_element_type(element)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

gboolean
cong_dispspec_element_is_span(CongDispspecElement *element)
{
	g_return_val_if_fail(element, FALSE);

	if (CONG_ELEMENT_TYPE_SPAN == cong_dispspec_element_type(element)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

#if NEW_LOOK
GdkGC*
cong_dispspec_element_gc(CongDispspecElement *element, enum CongDispspecGCUsage usage)
{
	g_return_val_if_fail(element, NULL);
	g_return_val_if_fail(usage<CONG_DISPSPEC_GC_USAGE_NUM, NULL);

	return element->gc_array[usage];
}

const GdkColor*
cong_dispspec_element_col(CongDispspecElement *element, enum CongDispspecGCUsage usage)
{
	g_return_val_if_fail(element, NULL);
	g_return_val_if_fail(usage<CONG_DISPSPEC_GC_USAGE_NUM, NULL);

	return &element->col_array[usage];
}

#else
GdkGC*
cong_dispspec_element_gc(CongDispspecElement *element)
{
	g_return_val_if_fail(element, NULL);

	return element->gc;
}

const GdkColor*
cong_dispspec_element_col(CongDispspecElement *element)
{
	g_return_val_if_fail(element, NULL);

	return &element->col;
}
#endif

CongDispspecElementHeaderInfo*
cong_dispspec_element_header_info(CongDispspecElement *element)
{
	g_return_val_if_fail(element, NULL);

	return element->header_info;
}

gchar*
cong_dispspec_element_get_title(CongDispspecElement *element, CongNodePtr x)
{
	xmlXPathContextPtr ctxt;
	xmlXPathObjectPtr xpath_obj;

	g_return_val_if_fail(element, NULL);
	g_return_val_if_fail(element->header_info, NULL);
	g_return_val_if_fail(x, NULL);

	/* g_message("cong_dispspec_element_get_title for <%s>", element->tagname); */

	if (element->header_info->xpath) {
		gchar *result = NULL;

		/* g_message("searching xpath \"%s\"",element->header_info->xpath); */

		ctxt = xmlXPathNewContext(x->doc);

		ctxt->node = x;

		xpath_obj = xmlXPathEval(element->header_info->xpath,
					 ctxt);

		if (xpath_obj) {
			result = xmlXPathCastToString(xpath_obj);			
		} else {
			result = g_strdup(_("(xpath failed)"));
		}	

		xmlXPathFreeContext(ctxt);
		
		return result;
	} else if (element->header_info->tagname) {
		/* Search for a child node matching the tagname: */
		CongNodePtr i;

		/* g_message("searching for tag <%s>", element->header_info->tagname); */
		
		for (i = cong_node_first_child(x); i; i = cong_node_next(i) ) {
			
			/* printf("got node named \"%s\"\n", cong_node_name(i)); */			
			
			if (0==strcmp(cong_node_name(i), element->header_info->tagname)) {
				return xml_fetch_clean_data(i);
			}
		}
		
		/* Not found: */
		return NULL;
	} else {
		return NULL;
	}
}

gchar*
cong_dispspec_element_get_section_header_text(CongDispspecElement *element, CongNodePtr x)
{
	g_return_val_if_fail(element,NULL);
	g_return_val_if_fail(x,NULL);

	if (element->header_info) {
		
		gchar* title = cong_dispspec_element_get_title(element, x);

		if (title) {
			char *result = g_strdup_printf("%s : %s", cong_dispspec_element_username(element), title);
			g_free(title);

			return result;
		} else {
			/* FIXME:  should we display <untitled>?  or should this be a dispspec-specified per-element property? */
			return g_strdup_printf("%s : %s", cong_dispspec_element_username(element), _("<untitled>"));
		}		
	} else {

		/* printf("no header info for %s\n", cong_node_name(x)); */
		return g_strdup(cong_dispspec_element_username(element));
	}
}

CongFont*
cong_dispspec_element_get_font(CongDispspecElement *element, enum CongFontRole role)
{
	g_return_val_if_fail(element, NULL);
	g_return_val_if_fail(role<CONG_FONT_ROLE_NUM, NULL);

	/* fonts are currently a property of the app: */
	return cong_app_singleton()->fonts[role];

}

const gchar*
cong_dispspec_element_get_editor_plugin_id(CongDispspecElement *element)
{
	g_return_val_if_fail(element, NULL);

	return element->editor_plugin_id;
}

const gchar*
cong_dispspec_element_get_property_dialog_plugin_id(CongDispspecElement *element)
{
	g_return_val_if_fail(element, NULL);

	return element->property_dialog_plugin_id;
}

static const CongEnumMapping whitespace_numeration[] =
{
	{"preserve", CONG_WHITESPACE_PRESERVE},
	{"normalize", CONG_WHITESPACE_NORMALIZE}
};

CongDispspecElement*
cong_dispspec_element_new_from_xml_element(xmlDocPtr doc, xmlNodePtr xml_element)
{
	CongDispspecElement* element;

	DS_DEBUG_MSG1("got xml element\n");

	element = g_new0(CongDispspecElement,1);

	element->whitespace = CONG_WHITESPACE_NORMALIZE;

	/* Extract tag namespace: */
	{
		xmlChar* xmlns = xmlGetProp(xml_element, "ns");
		if (xmlns) {
			element->xmlns = g_strdup(xmlns);
		}
	}

	/* Extract tagname: */
	{
		xmlChar* tag = xmlGetProp(xml_element, "tag");
		if (tag) {
			element->tagname = g_strdup(tag);			
		} else {
			element->tagname = g_strdup("unknown-tag");
		}
	}

	/* Extract type: */
	{
		xmlChar* type = xmlGetProp(xml_element,"type");

		element->type = CONG_ELEMENT_TYPE_UNKNOWN;			
		
		if (type) {
			if (0==strcmp(type,"structural")) {
				element->type = CONG_ELEMENT_TYPE_STRUCTURAL;			
			} else if (0==strcmp(type,"span")) {
				element->type = CONG_ELEMENT_TYPE_SPAN;			
			} else if (0==strcmp(type,"insert")) {
				element->type = CONG_ELEMENT_TYPE_INSERT;			
			} else if (0==strcmp(type,"embed-external-file")) {
				element->type = CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE;
			} else if (0==strcmp(type,"paragraph")) {
				element->type = CONG_ELEMENT_TYPE_PARAGRAPH;
			} else if (0==strcmp(type,"plugin")) {
				xmlChar* id;

				element->type = CONG_ELEMENT_TYPE_PLUGIN;

				id = xmlGetProp(xml_element,"plugin-id");
				
				if (id) {
  					element->editor_plugin_id = g_strdup(id);
  				}
  			}
  		}
  	}

	/* Extract pixbuf: */
	{
		xmlChar* prop = xmlGetProp(xml_element, "icon");
		if (prop) {
			element->icon16 = cong_util_load_icon(prop);

			xmlFree(prop);
		}
	}

	/* Extract whitespace: */
	{
		xmlChar* prop = xmlGetProp(xml_element, "whitespace");
		if (prop) {
			element->whitespace = cong_enum_mapping_lookup (whitespace_numeration,
									sizeof(whitespace_numeration)/sizeof(CongEnumMapping),
									prop,
									CONG_WHITESPACE_NORMALIZE);
			xmlFree(prop);
		}
	}

  	/* Process children: */
  	{
  		xmlNodePtr child;

  		for (child = xml_element->children; child; child=child->next) {
  			/* Extract names: */
  			if (0==strcmp(child->name,"name")) {
  				xmlChar* str = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
  				if (str) {
  					element->username = g_strdup(str);					
  				}
  			}

  			/* Extract short-desc: */
  			if (0==strcmp(child->name,"short-desc")) {
  				xmlChar* str = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
  				if (str) {
  					element->short_desc = g_strdup(str);
  				}
  			}

  			/* Handle "collapseto": */
  			if (0==strcmp(child->name,"collapseto")) {
  				element->collapseto = TRUE;
  			}

  			/* Handle "header-info": */
  			if (0==strcmp(child->name,"header-info")) {
  				DS_DEBUG_MSG1("got header info\n");
  				element->header_info = g_new0(CongDispspecElementHeaderInfo,1);
				element->header_info->xpath = cong_node_get_attribute(child, "xpath");
				element->header_info->tagname = cong_node_get_attribute(child, "tag");
  			}

			/* Handle "property-dialog": */
  			if (0==strcmp(child->name,"property-dialog")) {
  				DS_DEBUG_MSG1("got property-dialog\n");
				
				element->property_dialog_plugin_id = cong_node_get_attribute(child, "plugin-id");
  			}
			
		}

		/* Supply defaults where children not found: */
		if (NULL==element->username) {
			element->username = g_strdup(element->tagname);
		}
	}

	/* Extract colour: */
	{
		xmlChar* col_text = xmlGetProp(xml_element,"color");
		unsigned int col;

		if (col_text) {
			col = get_rgb_hex(col_text);
		} else {
			col = 0x00ffffff;  /* White is default */
		}

		cong_dispspec_element_init_col(element, col);
	}

	return element;
}

const gchar*
cong_external_document_model_get_public_id (const CongExternalDocumentModel* model)
{
	g_return_val_if_fail (model, NULL);

	return model->public_id;
}

const gchar*
cong_external_document_model_get_system_id (const CongExternalDocumentModel* model)
{
	g_return_val_if_fail (model, NULL);

	return model->system_id;
}

/* Subroutines for converting a CongDispspec to XML XDS: */
static const gchar* element_type_to_string(enum CongElementType type) 
{
        switch(type) {
	default:
		g_assert_not_reached();
	case CONG_ELEMENT_TYPE_STRUCTURAL:
		return "structural";
	case CONG_ELEMENT_TYPE_SPAN:
		return "span";
	case CONG_ELEMENT_TYPE_INSERT:
		return "insert";
	case CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE:
		return "embed-external-file";
	case CONG_ELEMENT_TYPE_PARAGRAPH:
		return "paragraph";
	case CONG_ELEMENT_TYPE_PLUGIN:
		return "plugin";
	case CONG_ELEMENT_TYPE_UNKNOWN:
		return "unknown";
	case CONG_ELEMENT_TYPE_ALL:
		return "all";
        }
}

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
			CongNodePtr node_format;
			CongSerialisationFormat *format = dispspec->serialisation_formats[i];
			g_assert (format);

			node_format = xmlNewDocNode (xml_doc,
						     NULL,
						     "format",
						     NULL);

			xmlAddChild (node_serialisation, 
				     node_format);

			if (format->extension) {
				xmlSetProp (node_format,
					    "extension",
					    format->extension);
			}

			
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
		enum CongDocumentModelType model_type = (enum CongDocumentModelType)i;
		const CongExternalDocumentModel* model = cong_dispspec_get_external_document_model (dispspec,
												    model_type);

		if (model) {
			CongNodePtr node_document_model;
		
			node_document_model = xmlNewDocNode (xml_doc,
							     NULL,
							     "external-document-model",
							     NULL); 
			xmlAddChild (node_document_models,
				     node_document_model);

			xmlSetProp (node_document_model,
				    "type",
				    cong_enum_mapping_lookup_string (document_model_enum_mapping,
								     sizeof(document_model_enum_mapping)/sizeof(CongEnumMapping),
								     model_type)
				    );
			if (model->public_id) {
				xmlSetProp (node_document_model,
					    "public-id",
					    model->public_id);
			}
			if (model->system_id) {
				xmlSetProp (node_document_model,
					    "system-id",
					    model->system_id);
			}
		}		
	}
}

static void add_xml_for_element (xmlDocPtr xml_doc, 
				 CongNodePtr element_list, 
				 CongDispspecElement *element)
{
	CongNodePtr element_node;

	g_assert(element_list);
	g_assert(element);

	element_node = xmlNewDocNode (xml_doc,
				      NULL,
				      "element",
				      NULL);			
	xmlAddChild (element_list, element_node);

	if (element->xmlns) {
		xmlSetProp (element_node, "ns", element->xmlns);
	}

	g_assert (element->tagname);
	xmlSetProp (element_node, "tag", element->tagname);
	xmlSetProp (element_node, "type", element_type_to_string(element->type));

	/* Handle name: */
	if (element->username)
	{
		CongNodePtr name_node = xmlNewDocNode (xml_doc,
						       NULL,
						       "name",
						       element->username
						       );
		xmlSetProp (name_node, "locale", "en");

		xmlAddChild (element_node, name_node);		
	}
	
	/* Handle short-desc: */
	if (element->short_desc)
	{
		CongNodePtr desc_node = xmlNewDocNode (xml_doc,
						       NULL,
						       "short-desc",
						       element->short_desc
						       );
		xmlAddChild (element_node, desc_node);		
	}

	/* FIXME:  Need to store these: */
#if 0
	GdkPixbuf *icon16;

	gboolean collapseto;

#if NEW_LOOK
	GdkColor col_array[CONG_DISPSPEC_GC_USAGE_NUM];
	GdkGC* gc_array[CONG_DISPSPEC_GC_USAGE_NUM];
#else
	GdkColor col;
	GdkGC* gc;
#endif

	CongDispspecElementHeaderInfo *header_info;

	gchar *editor_plugin_id;
	gchar *property_dialog_plugin_id;
#endif
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

	if(!strcmp(element->tagname, xmlDocGetRootElement(node->doc)->name)) {
		return;
	}

	switch (cong_dispspec_type (dispspec, cong_node_xmlns(node->parent), node->parent->name))
	{
		case CONG_ELEMENT_TYPE_SPAN:
		{
			if (contains_carriage_return(xmlNodeGetContent (node)))
			{
				if (contains_text (xmlNodeGetContent (node)))
				{
					CongDispspecElement *ds_element = cong_dispspec_element_new (cong_node_xmlns(node->parent),
												     node->parent->name,
												     CONG_ELEMENT_TYPE_PARAGRAPH,
												     TRUE);
					g_assert (ds_element);
					cong_dispspec_add_element (dispspec, ds_element);
				}
				else
				{
					CongDispspecElement *ds_element = cong_dispspec_element_new (cong_node_xmlns(node->parent),
												     node->parent->name,
												     CONG_ELEMENT_TYPE_STRUCTURAL,
												     TRUE);
					g_assert (ds_element);
					cong_dispspec_add_element (dispspec, ds_element);
				}
			}
			break;
		}
		case CONG_ELEMENT_TYPE_STRUCTURAL:
		{
			if (contains_text (xmlNodeGetContent (node)))
			{
				CongDispspecElement *ds_element = cong_dispspec_element_new (cong_node_xmlns(node->parent),
											     node->parent->name,
											     CONG_ELEMENT_TYPE_PARAGRAPH,
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
				element =  cong_dispspec_lookup_element (dispspec, cong_node_xmlns(cur->parent), cur->parent->name);
				if (element) {
					promote_element (dispspec, element, cur);
				}
				else if (contains_text (xmlNodeGetContent (cur))) {
					if (contains_carriage_return(xmlNodeGetContent (cur))) {
						CongDispspecElement *ds_element = cong_dispspec_element_new (cong_node_xmlns(cur->parent),
													     cur->parent->name,
													     CONG_ELEMENT_TYPE_PARAGRAPH,
													     TRUE);
						g_assert (ds_element);
						cong_dispspec_add_element (dispspec, ds_element);
					} else {
						CongDispspecElement *ds_element = cong_dispspec_element_new (cong_node_xmlns(cur->parent),
													     cur->parent->name,
													     CONG_ELEMENT_TYPE_SPAN,
													     TRUE);
						g_assert (ds_element);
						cong_dispspec_add_element (dispspec, ds_element);
					}
				} else {
					CongDispspecElement *ds_element = cong_dispspec_element_new (cong_node_xmlns(cur->parent),
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
		element = cong_dispspec_lookup_element (dispspec, cong_node_xmlns(cur), cong_node_name(cur));
		if (NULL==element) {
			/* Then we've found an element that doesn't have any handler in the dispspec; better create a structural tag for it... */
			CongDispspecElement *ds_element = cong_dispspec_element_new (cong_node_xmlns(cur),
										     cong_node_name(cur),
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

		CongDispspecElement *ds_element = cong_dispspec_element_new (cong_node_xmlns(root_element),
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


xmlNodePtr
cong_dispspec_get_template(const CongDispspec *ds)
{
	return  ds->template;
}
