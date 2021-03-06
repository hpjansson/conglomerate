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
#include <libxml/xpath.h>
#include "cong-node.h"
#include "cong-app.h"
#include "cong-eel.h"
#include "cong-dtd.h"
#include "cong-util.h"
#include "cong-enum-mapping.h"
#include "cong-ui-hooks.h"

struct CongDispspecElementHeaderInfo
{
	gchar *xpath; /* if present, this is the XPath to use when determining the title of the tag */
};

struct CongDispspecElement
{
	CongDispspec *ds;

	/* Local name within namespace (if any); must be non-NULL */
	gchar *local_name;

	GHashTable *hash_of_language_to_user_name;
	GHashTable *hash_of_language_to_short_desc;

	gchar *icon_name;
	GdkPixbuf *icon16;
	CongWhitespaceHandling whitespace;

	CongElementType type;
	gboolean collapseto;

	GdkColor col_array[CONG_DISPSPEC_GC_USAGE_NUM];
	GdkGC* gc_array[CONG_DISPSPEC_GC_USAGE_NUM];

	CongDispspecElementHeaderInfo *header_info;

	gchar *editor_service_id;
	gchar *property_dialog_service_id;

	gboolean should_spellcheck;

	GHashTable *key_value_hash;
};

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
/* Internal function prototypes: */
static gpointer
find_best_value_for_language (GHashTable *hash_of_language);

static const gchar*
find_best_string_for_language (GHashTable *hash_of_language);

/* Use gxx to generate XML load/save routines: */
static const CongEnumMapping type_numeration[] =
{
	{"structural", CONG_ELEMENT_TYPE_STRUCTURAL},
	{"span", CONG_ELEMENT_TYPE_SPAN},
	{"insert", CONG_ELEMENT_TYPE_INSERT},
	{"embed-external-file", CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE},
	{"plugin", CONG_ELEMENT_TYPE_PLUGIN},
	{"all", CONG_ELEMENT_TYPE_ALL}
};

static const CongEnumMapping whitespace_numeration[] =
{
	{"preserve", CONG_WHITESPACE_PRESERVE},
	{"normalize", CONG_WHITESPACE_NORMALIZE}
};

/**
 * gxx_callback_construct_dispspec_element:
 *
 * TODO: Write me
 */
CongDispspecElement*
gxx_callback_construct_dispspec_element(void)
{
	CongDispspecElement *element = g_new0(CongDispspecElement,1);

	element->whitespace = CONG_WHITESPACE_NORMALIZE;

	element->hash_of_language_to_user_name = g_hash_table_new_full (cong_str_or_null_hash,
									cong_str_or_null_equal,
									g_free,
									g_free);
	element->hash_of_language_to_short_desc = g_hash_table_new_full (cong_str_or_null_hash,
									 cong_str_or_null_equal,
									 g_free,
									 g_free);	
	element->key_value_hash = g_hash_table_new_full (g_str_hash,
							 g_str_equal,
							 g_free,
							 g_free);

	return element;
}

#include "gxx-declare-object-from-xml-tree.h"
#include "cong-dispspec-element-gxx.h"

#include "gxx-object-from-xml-tree.h"
#include "cong-dispspec-element-gxx.h"

#include "gxx-declare-object-to-xml-tree.h"
#include "cong-dispspec-element-gxx.h"

#include "gxx-object-to-xml-tree.h"
#include "cong-dispspec-element-gxx.h"

/* Random other stuff: */
#if 0
/* Hackish colour calculations in RGB space (ugh!) */
static void generate_col(GdkColor *dst, const GdkColor *src, float bodge_factor)
{
	dst->red = src->red / bodge_factor;
	dst->green = src->green / bodge_factor;
	dst->blue = src->blue / bodge_factor;
}
#endif


unsigned int hacked_cols[3][CONG_DISPSPEC_GC_USAGE_NUM] =
{
	/* Blueish, section 1 in Joakim's mockup: */
	{ 0x6381ff, 0xd5d2ff, 0xe6e2ff,	0x414083 },

	/* Brownish; section 2 in the mockup: */
	{ 0xd5b69c, 0xe6ded5, 0xeee6de, 0x836141 },

	/* Dark brown; the underline in the mockup: */
	{ 0x632829, 0x632829, 0x632829, 0x632829 }
};

static void get_col(GdkColor *dst, const GdkColor *src, CongDispspecGCUsage usage)
{
	/* pick one of the test colour tables based on a dodgy hashing of the source colour: */
	col_to_gcol(dst, hacked_cols[(src->red>>8)%2][usage]);
}

/**
 * generate_gc_for_col:
 * @col:
 *
 * TODO: Write me
 * Returns:
 */
GdkGC*
generate_gc_for_col (const GdkColor *col)
{
	GdkGC *gc;

	g_return_val_if_fail (col, NULL);

	gc = gdk_gc_new(cong_gui_get_a_window()->window);
	gdk_gc_copy(gc, cong_gui_get_a_window()->style->white_gc);
	gdk_colormap_alloc_color(cong_gui_get_a_window()->style->colormap, (GdkColor*)col, FALSE, TRUE);
	gdk_gc_set_foreground(gc, (GdkColor*)col);	

	return gc;
}

static void cong_dispspec_element_init_col(CongDispspecElement* element, unsigned int col)
{
	g_assert(element);

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
}

/* Exported functions: */

/** 
 * cong_dispspec_element_new:
 * @ds: the #CongDispspec this will be added to
 * @local_name: the local name for the new element; must be non-NULL
 * @type:
 * @autogenerate_username:  if TRUE, then generate a sane user-visible name for the element,
 * using "header capitalisation"
 *
 * Constructs a new #CongDispspecElement, initialising fields to sane defaults.
 *
 * Returns: a freshly allocated #CongDispspecElement 
 **/
CongDispspecElement*
cong_dispspec_element_new (CongDispspec *ds,
			   const gchar* local_name, 
			   CongElementType type,
			   gboolean autogenerate_username)
{
	CongDispspecElement* element;

	g_return_val_if_fail (local_name, NULL);
	
	g_message("cong_dispspec_element_new (\"%s\",)", local_name);

	/* Use shared constructor code: */
	element = gxx_callback_construct_dispspec_element();

	element->ds = ds;

	element->local_name = g_strdup(local_name);

	if (autogenerate_username) {
		/* Try to prettify the username if possible; 
		   FIXME: which language should this go into? */
		g_hash_table_insert (element->hash_of_language_to_user_name,
				     "C",
				     cong_eel_prettify_xml_name_with_header_capitalisation (local_name));
	} else {
		/* username remains unset */
	}

	element->type = type;

	element->should_spellcheck = TRUE;

	/* Extract colour: */
	{
		unsigned int col = 0x00000000;  /* Black is default for the new look */

		cong_dispspec_element_init_col(element, col);
	}

	return element;
}

/* Destruction  */
/**
 * cong_dispspec_element_destroy:
 * @element:
 *
 * TODO: Write me
 */
void 
cong_dispspec_element_destroy (CongDispspecElement *element)
{
	g_return_if_fail (element);

	/* FIXME: could autogenerate this code: */
	if (element->local_name) {
		g_free (element->local_name);
	}
	g_hash_table_destroy (element->hash_of_language_to_user_name);
	g_hash_table_destroy (element->hash_of_language_to_short_desc);

	if (element->icon16) {
		g_object_unref (G_OBJECT(element->icon16));
	}

	/* FIXME: need to clean up this stuff: */
#if 0
	GdkColor col_array[CONG_DISPSPEC_GC_USAGE_NUM];
	GdkGC* gc_array[CONG_DISPSPEC_GC_USAGE_NUM];

	CongDispspecElementHeaderInfo *header_info;
#endif

	if (element->editor_service_id) {
		g_free (element->editor_service_id);
	}
	if (element->property_dialog_service_id) {
		g_free (element->property_dialog_service_id);
	}

	g_free (element);
	/* FIXME:  do we need to remove from the list? */
}

/** 
 * cong_dispspec_element_get_dispspec:
 * @element: the element in question
 * 
 * Returns: the #CongDispspec containing this element
 */
CongDispspec*
cong_dispspec_element_get_dispspec (const CongDispspecElement *element)
{
	g_return_val_if_fail (element, NULL);

	return element->ds;
}


/** 
 * cong_dispspec_element_get_ns_uri:
 * @element: the element in question
 * 
 * Returns: the namespace URI for this kind of element, or NULL if none
 */
const gchar*
cong_dispspec_element_get_ns_uri (const CongDispspecElement *element)
{
	g_return_val_if_fail (element, NULL);

	if (element->ds) {
		return cong_dispspec_get_ns_uri (element->ds);
	} else {
		return NULL;
	}
}

/**
 * cong_dispspec_element_get_local_name:
 * @element: the element in question
 * 
 * Returns: the local name (relative to its namespace, if any) for this kind of element
 */
const gchar*
cong_dispspec_element_get_local_name(const CongDispspecElement* element)
{
	g_return_val_if_fail (element, NULL);

	return element->local_name;
}

/**
 * cong_dispspec_element_username:
 * @element:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_dispspec_element_username(CongDispspecElement* element)
{
	const gchar *result;
	g_return_val_if_fail(element, NULL);

	result = find_best_string_for_language (element->hash_of_language_to_user_name);

	if (result) {
		return result;
	} else {
		return element->local_name;
	}
}

/**
 * cong_dispspec_element_get_description:
 * @element:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_dispspec_element_get_description(CongDispspecElement *element)
{
	g_return_val_if_fail(element,NULL);

	return find_best_string_for_language (element->hash_of_language_to_short_desc);
}

/**
 * cong_dispspec_element_get_icon:
 * @element:
 *
 * TODO: Write me
 * Returns:
 */
GdkPixbuf*
cong_dispspec_element_get_icon(CongDispspecElement *element)
{
	g_return_val_if_fail(element, NULL);

	if (element->icon16) {
		g_object_ref(G_OBJECT(element->icon16));
	}
	return element->icon16;
}

/**
 * cong_dispspec_element_get_value_for_key:
 * @key: the key
 * @element:  the dispspec element
 * 
 * Dispspec elements support a list of key/value string pairs; this is intended as a mechanism
 * to allow plugins to have arbitrary data whilst having a DTD for xds files.
 * 
 * Returns:  the value, if found, or NULL if not present.
 */
const gchar*
cong_dispspec_element_get_value_for_key (const gchar *key, 
					 const CongDispspecElement *element)
{
	g_return_val_if_fail (element, NULL);
	g_return_val_if_fail (key, NULL);

	return g_hash_table_lookup (element->key_value_hash,
				    key);
}

/**
 * cong_dispspec_element_type:
 * @element:
 *
 * TODO: Write me
 * Returns:
 */
CongElementType
cong_dispspec_element_type(CongDispspecElement *element)
{
	g_return_val_if_fail(element, CONG_ELEMENT_TYPE_UNKNOWN);

	return element->type;
}

/**
 * cong_dispspec_element_get_whitespace:
 * @element:  The element of the display spec
 *
 * Get the #CongWhitespaceHandling behaviour for this element
 * Returns:  
 */
CongWhitespaceHandling
cong_dispspec_element_get_whitespace (CongDispspecElement *element)
{
	g_return_val_if_fail (element, CONG_WHITESPACE_NORMALIZE);

	return element->whitespace;
}

/**
 * cong_dispspec_element_set_whitespace:
 * @element:  The element of the display spec
 * @whitespace: The new value for whitespace handling
 *
 * Set the #CongWhitespaceHandling behaviour for this element
 */
void
cong_dispspec_element_set_whitespace (CongDispspecElement *element,
				      CongWhitespaceHandling whitespace)
{	
	g_return_if_fail (element);

	element->whitespace = whitespace;
}

/**
 * cong_dispspec_element_collapseto:
 * @element:
 *
 * TODO: Write me
 * Returns:
 */
gboolean
cong_dispspec_element_collapseto(CongDispspecElement *element)
{
	return element->collapseto;
}

/**
 * cong_dispspec_element_is_structural:
 * @element:
 *
 * TODO: Write me
 * Returns:
 */
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

/**
 * cong_dispspec_element_is_span:
 * @element:
 *
 * TODO: Write me
 * Returns:
 */
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

/**
 * cong_dispspec_element_gc:
 * @element:
 * @usage:
 *
 * TODO: Write me
 * Returns:
 */
GdkGC*
cong_dispspec_element_gc(CongDispspecElement *element, CongDispspecGCUsage usage)
{
	g_return_val_if_fail(element, NULL);
	g_return_val_if_fail(usage<CONG_DISPSPEC_GC_USAGE_NUM, NULL);

	return element->gc_array[usage];
}

/**
 * cong_dispspec_element_col:
 * @element:
 * @usage:
 *
 * TODO: Write me
 * Returns:
 */
const GdkColor*
cong_dispspec_element_col(CongDispspecElement *element, CongDispspecGCUsage usage)
{
	g_return_val_if_fail(element, NULL);
	g_return_val_if_fail(usage<CONG_DISPSPEC_GC_USAGE_NUM, NULL);

	return &element->col_array[usage];
}

/**
 * cong_dispspec_element_header_info:
 * @element:
 *
 * TODO: Write me
 * Returns:
 */
CongDispspecElementHeaderInfo*
cong_dispspec_element_header_info(CongDispspecElement *element)
{
	g_return_val_if_fail(element, NULL);

	return element->header_info;
}

/**
 * cong_dispspec_element_header_info_get_xpath_expression:
 * @header_info:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_dispspec_element_header_info_get_xpath_expression (CongDispspecElementHeaderInfo *header_info)
{
	g_return_val_if_fail (header_info, NULL);

	return header_info->xpath;
}

/**
 * cong_dispspec_element_get_title:
 * @element:
 * @x:
 *
 * TODO: Write me
 * Returns:
 */
gchar*
cong_dispspec_element_get_title (CongDispspecElement *element, 
				 CongNodePtr x)
{
	xmlXPathContextPtr ctxt;
	xmlXPathObjectPtr xpath_obj;
	const gchar *xpath_string;

	g_return_val_if_fail (element, NULL);
	g_return_val_if_fail (element->header_info, NULL);
	g_return_val_if_fail (x, NULL);

	xpath_string = cong_dispspec_element_header_info_get_xpath_expression (element->header_info);

	if (xpath_string) {

		gchar *result = NULL;
		
		/* g_message("searching xpath \"%s\"",element->header_info->xpath); */
		
		ctxt = xmlXPathNewContext (x->doc);
		
		ctxt->node = x;
		
		xpath_obj = xmlXPathEval ((const xmlChar*)xpath_string,
					  ctxt);	
		if (xpath_obj) {
			xmlChar *xmlResult = xmlXPathCastToString (xpath_obj);
			result = g_strdup ((const gchar*)xmlResult);
			xmlFree (xmlResult);
		} else {
			result = g_strdup (_("(xpath failed)"));
		}	
		
		xmlXPathFreeContext (ctxt);
		
		return result;
	} else {
		return NULL;
	}		
}

/**
 * cong_dispspec_element_get_section_header_text:
 * @element:
 * @x:
 *
 * TODO: Write me
 * Returns:
 */
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

/**
 * cong_dispspec_element_get_font:
 * @element:
 * @role:
 *
 * TODO: Write me
 * Returns:
 */
CongFont*
cong_dispspec_element_get_font(CongDispspecElement *element, CongFontRole role)
{
	g_return_val_if_fail(element, NULL);
	g_return_val_if_fail(role<CONG_FONT_ROLE_NUM, NULL);

	/* fonts are currently a property of the app: */
	return cong_app_get_font (cong_app_singleton(),
				  role);
}

/**
 * cong_dispspec_element_get_editor_service_id:
 * @element:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_dispspec_element_get_editor_service_id(CongDispspecElement *element)
{
	g_return_val_if_fail(element, NULL);

	return element->editor_service_id;
}

/**
 * cong_dispspec_element_get_property_dialog_service_id:
 * @element:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_dispspec_element_get_property_dialog_service_id(CongDispspecElement *element)
{
	g_return_val_if_fail(element, NULL);

	return element->property_dialog_service_id;
}

/**
 * cong_dispspec_element_from_xml:
 * @ds: the #CongDispspec this will be added to
 * @xml_element:  the element within the XML document
 *
 * Create an #CongDispspecElement from an xds XML representation
 *
 * Returns: a newly-created #CongDispspecElement
 */
CongDispspecElement*
cong_dispspec_element_from_xml (CongDispspec *ds,
				xmlNodePtr xml_element)
{
	CongDispspecElement* element;

	g_return_val_if_fail (cong_node_is_element (xml_element, NULL, "element"), NULL);

	DS_DEBUG_MSG1("got xml element\n");

	element = gxx_generated_object_from_xml_tree_fn_dispspec_element (xml_element);

	element->ds = ds;

	if (element->type == CONG_ELEMENT_TYPE_PLUGIN) {
		xmlChar* id = xmlGetProp(xml_element,(const xmlChar*)"service-id");
		
		if (id) {
			element->editor_service_id = g_strdup ((const gchar*)id);
			xmlFree (id);
		}
	}

	/* Load pixbuf: */
	if (element->icon_name)
	{
		element->icon16 = cong_util_load_icon(element->icon_name);
	}

  	/* Process children: */
  	{
  		xmlNodePtr child;

  		for (child = xml_element->children; child; child=child->next) {

  			/* Handle "collapseto": */
  			if (0==strcmp((const char*)child->name,"collapseto")) {
  				element->collapseto = TRUE;
  			}

			/* Handle "property-dialog": */
  			if (0==strcmp((const char*)child->name,"property-dialog")) {
  				DS_DEBUG_MSG1("got property-dialog\n");
				
				element->property_dialog_service_id = cong_node_get_attribute(child, NULL, "service-id");
  			}

			/* Handle "key-value-list": */
			if ( cong_node_is_element (child, NULL, "key-value-list")) {
				xmlNodePtr key_value_iter;
				
  				DS_DEBUG_MSG1("got key-value-list\n");
				
				for (key_value_iter = child->children; key_value_iter; key_value_iter=key_value_iter->next) {
					if (cong_node_is_element (key_value_iter, NULL, "key-value-pair")) {
						DS_DEBUG_MSG1("got key-value-pair\n");
						
						g_hash_table_insert (element->key_value_hash,
								     cong_node_get_attribute (key_value_iter, NULL, "key"),
								     cong_node_get_attribute (key_value_iter, NULL, "value"));
					}
				}
		            	
			}

#if 0			
			/* Supply defaults where children not found: */
			if (NULL==element->username) {
				element->username = g_strdup(element->tagname);
			}
#endif
		}
	}

	/* Extract colour: */
	{
		xmlChar* col_text = xmlGetProp(xml_element,(const xmlChar*)"color");
		unsigned int col;

		if (col_text) {
			col = cong_util_get_int_from_rgb_hex ((const gchar*)col_text);
		} else {
			col = 0x00ffffff;  /* White is default */
		}

		cong_dispspec_element_init_col(element, col);
	}

	return element;
}

/**
 * cong_dispspec_element_to_xml:
 * @element:  the dispspec element we are adding
 * @xml_doc:  the XML document we are creating
 *
 * Create an element tag suitable for adding to an element-list tag within an xds XML document.
 *
 * Returns: a newly-created #xmlNodePtr
 */
xmlNodePtr
cong_dispspec_element_to_xml (const CongDispspecElement *element,
			      xmlDocPtr xml_doc)
{
	CongNodePtr xml_node;

	g_assert(element);
	g_assert(xml_doc);

	xml_node = gxx_generated_object_to_xml_tree_fn_dispspec_element (element, xml_doc);

	/* FIXME:  Need to store these: */
#if 0
	gboolean collapseto;

	GdkColor col_array[CONG_DISPSPEC_GC_USAGE_NUM];
	GdkGC* gc_array[CONG_DISPSPEC_GC_USAGE_NUM];

	gchar *editor_service_id;
	gchar *property_dialog_service_id;
#endif

	return xml_node;
}

static gpointer
find_best_value_for_language (GHashTable *hash_of_language)
{
	const char * const *language_list;

	g_assert (hash_of_language);

	for (language_list = g_get_language_names (); *language_list; language_list++) {
		gpointer value = g_hash_table_lookup (hash_of_language,
						      *language_list);

		if (value) {
			return value;
		}					       
	}

	return g_hash_table_lookup (hash_of_language,
				    NULL);
}

static const gchar*
find_best_string_for_language (GHashTable *hash_of_language)
{
	return (const gchar*)find_best_value_for_language (hash_of_language);
}

CongElementDescription*
cong_dispspec_element_make_element_description (const CongDispspecElement *ds_element)
{
	g_return_val_if_fail (ds_element, NULL);

	return cong_element_description_new (cong_dispspec_element_get_ns_uri (ds_element),
					     ds_element->local_name);
}

gboolean
cong_dispspec_element_should_spellcheck (const CongDispspecElement *ds_element)
{
	g_return_val_if_fail (ds_element, FALSE);

	return ds_element->should_spellcheck;
}
