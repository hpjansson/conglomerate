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


#define SUPPORT_OLD_LOADERS 0

#if 0
#define DS_DEBUG_MSG1(x)    g_message((x))
#define DS_DEBUG_MSG2(x, a) g_message((x), (a))
#define DS_DEBUG_MSG3(x, a, b) g_message((x), (a), (b))
#else
#define DS_DEBUG_MSG1(x)    ((void)0)
#define DS_DEBUG_MSG2(x, a) ((void)0)
#define DS_DEBUG_MSG3(x, a, b) ((void)0)
#endif

struct CongDispspecElementHeaderInfo
{
	gchar *xpath; /* if present, this is the XPath to use when determining the title of the tag */
	gchar *tagname; /* if xpath not present, then look for this tag below the main tag (deprecated) */
};

struct CongDispspecElement
{
	gchar *tagname;
	gchar *username;
	gchar *short_desc;
	GdkPixbuf *icon16;

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

struct CongDispspec
{
	/* Implementation is an "intrusive list" of CongDispspecElement structs */ 
	CongDispspecElement* first;
	CongDispspecElement* last;

	/* We also store a tree of elements, for fast lookup by name: */ 
	GTree *tree;

	gchar *name;
	gchar *desc;
	GdkPixbuf *icon;

	CongDispspecElement *paragraph;
};

void cong_dispspec_add_element(CongDispspec* ds, CongDispspecElement* element);

#if SUPPORT_OLD_LOADERS
CongDispspecElement*
cong_dispspec_element_new_from_ttree(TTREE* tt);
#endif

CongDispspecElement*
cong_dispspec_element_new_from_xml_element(xmlDocPtr doc, xmlNodePtr xml_element);

CongDispspecElement*
cong_dispspec_element_new(const char* tagname, enum CongElementType type);

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
			GdkGC *gc = gdk_gc_new(cong_gui_get_a_window()->window);

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
			element->gc_array[i] = gc;
			gdk_gc_copy(gc, cong_gui_get_a_window()->style->white_gc);
			gdk_colormap_alloc_color(cong_gui_get_a_window()->style->colormap, &this_col, FALSE, TRUE);
			gdk_gc_set_foreground(gc, &this_col);
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

#if SUPPORT_OLD_LOADERS
CongDispspec* cong_dispspec_new_from_ds_file(const char *name)
{
	CongDispspec* ds;

	TTREE* tt = ttree_load((char*)name);
	if (!tt) {
		g_warning("Problem loading dispspec file \"%s\"\n", name);
		return NULL;  /* Invalid displayspec. */
	}

	ds = g_new0(CongDispspec,1);

	/* Convert the tree into the new representation: */
	{
		TTREE* child;
		for (child = tt->child; child; child=child->next) {
			CongDispspecElement* element = cong_dispspec_element_new_from_ttree(child);

			cong_dispspec_add_element(ds,element);
		}
	}	

	/* FIXME: release the TTREE */

	return ds;
}
#endif /* #if SUPPORT_OLD_LOADERS */

static CongDispspec* parse_xmldoc(xmlDocPtr doc);
static void parse_metadata(CongDispspec *ds, xmlDocPtr doc, xmlNodePtr node);

static gint tree_compare_func(gconstpointer a, gconstpointer b)
{
	return strcmp(a,b);
}

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

static CongDispspec* parse_xmldoc(xmlDocPtr doc)
{
	CongDispspec* ds = g_new0(CongDispspec,1);
	ds->tree = g_tree_new(tree_compare_func);

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
							CongDispspecElement* element = cong_dispspec_element_new_from_xml_element(doc, xml_element);
							
							cong_dispspec_add_element(ds,element);

							if (element->type==CONG_ELEMENT_TYPE_PARAGRAPH){
								ds->paragraph=element;
							}
						}
						
					} else if (0==strcmp(cur->name,"metadata")) {
						parse_metadata(ds, doc, cur);
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
	
void cong_dispspec_add_element(CongDispspec* ds, CongDispspecElement* element)
{
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

	g_tree_insert(ds->tree, element->tagname, element);
}

void cong_dispspec_delete(CongDispspec *dispspec)
{
	/* FIXME:  unimplemented */
	g_assert(0);
}

const gchar*
cong_dispspec_get_name(const CongDispspec *ds)
{
	g_return_val_if_fail(ds,NULL);

	if (ds->name) {
		return ds->name;
	} else {
		return _("unnamed");
	}

}

const gchar*
cong_dispspec_get_description(const CongDispspec *ds)
{
	g_return_val_if_fail(ds,NULL);

	if (ds->desc) {
		return ds->desc;
	} else {
		return _("No description available.");
	}
}

/* FIXME: These should be deprecated: */
#if 1
gboolean cong_dispspec_element_structural(CongDispspec *ds, const char *name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, name);

	if (NULL==element) {
		return FALSE;
	}

	return (CONG_ELEMENT_TYPE_STRUCTURAL == cong_dispspec_element_type(element));
}


gboolean cong_dispspec_element_collapse(CongDispspec *ds, const char *name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, name);

	if (NULL==element) {
		return FALSE;
	}
	
	return cong_dispspec_element_collapseto(element);
}


gboolean cong_dispspec_element_span(CongDispspec *ds, const char *name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, name);

	if (NULL==element) {
		return FALSE;
	}

	return (CONG_ELEMENT_TYPE_SPAN == cong_dispspec_element_type(element));
}


gboolean cong_dispspec_element_insert(CongDispspec *ds, const char *name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, name);

	if (NULL==element) {
		return FALSE;
	}

	return (CONG_ELEMENT_TYPE_INSERT == cong_dispspec_element_type(element));
}
#endif

enum CongElementType
cong_dispspec_type(CongDispspec *ds, const char* tagname)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, tagname);

	if (NULL==element) {
		return CONG_ELEMENT_TYPE_UNKNOWN;
	}

	return cong_dispspec_element_type(element);
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

const char *cong_dispspec_name_get(CongDispspec *ds, CongNodePtr x)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, xml_frag_name_nice(x));
	if (element) {
		return (char*)cong_dispspec_element_username(element);
	}
  
	return(xml_frag_name_nice(x));
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

char *section_str = "section";


#ifdef WINDOWS_BUILD
                                                                                
int strcasestr(char *haystack, char *needle)                                    
{
	  char *r;                                                                        
	  char *haystack_dup, *needle_dup;                                              
	                                                                                
	  haystack_dup = strdup(haystack);                                              
	  needle_dup = strdup(needle);                                                  
	                                                                                
	  strlwr(haystack_dup);                                                         
	  strlwr(needle_dup);                                                           
	                                                                                
	  r = strstr(haystack_dup, needle_dup);                                         
	                                                                                
	  free(haystack_dup);                                                           
	  free(needle_dup);                                                             
	                                                                                
	  return(r);                                                                    
}
#endif

/*
  We now use the GTree search structure for speed
 */
CongDispspecElement*
cong_dispspec_lookup_element(const CongDispspec *ds, const char* tagname)
{
	CongDispspecElement *element;

	g_return_val_if_fail(ds, NULL);
	g_return_val_if_fail(tagname, NULL);

	g_assert(ds->tree);

#if 1
	return g_tree_lookup(ds->tree, tagname);
#else
	element = ds->first;

	while (element) {
		g_assert(element->tagname);
		if (0==strcmp(element->tagname,tagname)) {
			return element;
		}

		element = element->next;
	}

	return NULL;
#endif
}

CongDispspecElement*
cong_dispspec_lookup_node(const CongDispspec *ds, CongNodePtr node)
{
	g_return_val_if_fail(ds, NULL);
	g_return_val_if_fail(node, NULL);

	return cong_dispspec_lookup_element(ds, cong_node_name(node));
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

/*******************************
   cong_dispspec_element stuff: 
*******************************/

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
	return the_globals.fonts[role];

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


#if SUPPORT_OLD_LOADERS
CongDispspecElement*
cong_dispspec_element_new_from_ttree(TTREE* tt)
{
	unsigned int col;

	CongDispspecElement* element = g_new0(CongDispspecElement,1);

	element->tagname = g_strdup(tt->data);
	element->username = g_strdup(cong_dispspec_ttree_username(tt));

	element->type = cong_dispspec_ttree_type(tt);
	element->collapseto = cong_dispspec_ttree_collapseto(tt);

	col = cong_dispspec_ttree_colour_get(tt);

	cong_dispspec_element_init_col(element, col);

	return element;
}
#endif /* #if SUPPORT_OLD_LOADERS */

CongDispspecElement*
cong_dispspec_element_new_from_xml_element(xmlDocPtr doc, xmlNodePtr xml_element)
{
	CongDispspecElement* element;

	DS_DEBUG_MSG1("got xml element\n");

	element = g_new0(CongDispspecElement,1);

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

CongDispspecElement*
cong_dispspec_element_new(const char* tagname, enum CongElementType type)
{
	CongDispspecElement* element;

	g_return_val_if_fail(tagname,NULL);

	element = g_new0(CongDispspecElement,1);

	element->tagname = g_strdup(tagname);	
	element->username = g_strdup(tagname);
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

/* ######################### Dispspec from xml ########################### */


static gboolean
can_contain_pcdata (xmlElementContentPtr content)
{
	if (content) {
		switch (content->type) {
		case XML_ELEMENT_CONTENT_PCDATA:
			return TRUE;
		}
		
		return can_contain_pcdata (content->c1)
			|| can_contain_pcdata (content->c2);
	} else {
		return FALSE;
	}
}

static void
handle_elements_from_dtd (void *payload, void *ds, xmlChar * name)
{
	xmlElementPtr element;
	CongDispspec *dispspec;

	dispspec = (CongDispspec *) ds;
	element = (xmlElementPtr) payload;

	g_assert (element);

	if (!element->content || can_contain_pcdata (element->content))
	{
		CongDispspecElement *ds_element = cong_dispspec_element_new (name,
									     CONG_ELEMENT_TYPE_SPAN);
		cong_dispspec_add_element (dispspec, ds_element);
	}
	else
	{
		CongDispspecElement *ds_element = cong_dispspec_element_new (name,
									     CONG_ELEMENT_TYPE_STRUCTURAL);
		cong_dispspec_add_element (dispspec, ds_element);
	}

}

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

	switch (cong_dispspec_type (dispspec, node->parent->name))
	{
		case CONG_ELEMENT_TYPE_SPAN:
		{
			if (contains_carriage_return(xmlNodeGetContent (node)))
			{
				if (contains_text (xmlNodeGetContent (node)))
				{
					CongDispspecElement *ds_element = cong_dispspec_element_new (node->parent->name,
												     CONG_ELEMENT_TYPE_PARAGRAPH);
					g_assert (ds_element);
					cong_dispspec_add_element (dispspec, ds_element);
				}
				else
				{
					CongDispspecElement *ds_element = cong_dispspec_element_new (node->parent->name,
												     CONG_ELEMENT_TYPE_STRUCTURAL);
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
				CongDispspecElement *ds_element = cong_dispspec_element_new (node->parent->name,
											     CONG_ELEMENT_TYPE_PARAGRAPH);
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
	g_assert (dispspec->tree);

	if (cur) {
		if (xmlNodeIsText (cur)) {
			element =  cong_dispspec_lookup_element (dispspec, cur->parent->name);
			if (element) {
				promote_element (dispspec, element, cur);
			}
			else if (contains_text (xmlNodeGetContent (cur))) {
				if (contains_carriage_return(xmlNodeGetContent (cur))) {
					CongDispspecElement *ds_element = cong_dispspec_element_new (cur->parent->name,
												     CONG_ELEMENT_TYPE_PARAGRAPH);
					g_assert (ds_element);
					cong_dispspec_add_element (dispspec, ds_element);
				} else {
					CongDispspecElement *ds_element = cong_dispspec_element_new (cur->parent->name,
												     CONG_ELEMENT_TYPE_SPAN);
					g_assert (ds_element);
					cong_dispspec_add_element (dispspec, ds_element);
				}
			} else {
				CongDispspecElement *ds_element = cong_dispspec_element_new (cur->parent->name,
											     CONG_ELEMENT_TYPE_STRUCTURAL);
				g_assert (ds_element);
				cong_dispspec_add_element (dispspec, ds_element);
			}
		}
	}
	
	cur = cur->xmlChildrenNode;
	
	while (cur != NULL) {
		handle_elements_from_xml (dispspec, cur);
		cur = cur->next;
	}
	return;
}

static void
xml_to_dispspec (CongDispspec * dispspec, xmlDocPtr doc, xmlDtdPtr dtd)
{
	if (doc)
	{
		CongDispspecElement *ds_element = cong_dispspec_element_new (xmlDocGetRootElement(doc)->name,
									     CONG_ELEMENT_TYPE_STRUCTURAL);
		g_assert (ds_element);
		cong_dispspec_add_element (dispspec, ds_element);
	}

	if (dtd)
	{
		xmlHashScan (dtd->elements, handle_elements_from_dtd, dispspec);
	}

	if (doc)
	{
		dispspec->name = g_strdup(doc->URL);
		handle_elements_from_xml (dispspec, xmlDocGetRootElement (doc));
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


CongDispspec *
cong_dispspec_new_from_xml_file (xmlDocPtr doc)
{
	CongDispspec *dispspec;

	dispspec = g_new0 (CongDispspec, 1);
	dispspec->tree = g_tree_new (tree_compare_func);

	xml_to_dispspec (dispspec, doc, load_dtd (doc));

	return dispspec;
}
