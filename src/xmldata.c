/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>                                                            
#include <gtk/gtk.h>                                                            

#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "cong-document.h"
#include "cong-dispspec.h"

char fake_data[] = "";

static gboolean xml_add_required_sub_elements(CongDocument *cong_doc, CongNodePtr node);
static gboolean xml_add_optional_text_nodes(CongDocument *cong_doc, xmlElementContentPtr content, xmlNodePtr node);
static gboolean xml_add_required_content(CongDocument *cong_doc, xmlElementContentPtr content, xmlNodePtr node);
static gboolean xml_add_required_content_choice(CongDocument *cong_doc, xmlElementContentPtr content, xmlNodePtr node);
static void xml_get_or_content_list(xmlElementContentPtr content, GList* list);
static gint xml_valid_get_potential_element_children(xmlElementContent *ctree, const xmlChar **list, int *len, int max);
static gint wrap_xml_valid_get_valid_elements(xmlNode *parent, xmlNode *next_sibling, const xmlChar ** elements, gint max);
static GList *xml_filter_valid_children_with_dispspec(CongDispspec* ds, const xmlChar **elements, 
						      gint elements_length, enum CongElementType tag_type);
static GList* xml_get_elements_from_dispspec(CongDispspec* ds, enum CongElementType tag_type);

const char* cong_node_name(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return node->name;
}

CongNodePtr cong_node_prev(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return node->prev;	
}

CongNodePtr cong_node_next(CongNodePtr node)
{
	g_assert(node);
	g_return_val_if_fail(node, NULL);

	return node->next;
}

CongNodePtr cong_node_first_child(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return node->children;
}

CongNodePtr cong_node_parent(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return node->parent;
}

enum CongNodeType cong_node_type(CongNodePtr node)
{
	g_return_val_if_fail(node, CONG_NODE_TYPE_UNKNOWN);

	switch (node->type) {

	default: g_assert_not_reached();

	case XML_ELEMENT_NODE: 
		return CONG_NODE_TYPE_ELEMENT;
	case XML_ATTRIBUTE_NODE:
		return CONG_NODE_TYPE_ATTRIBUTE;
	case XML_TEXT_NODE: 
		return CONG_NODE_TYPE_TEXT;
	case XML_CDATA_SECTION_NODE:
		return CONG_NODE_TYPE_CDATA_SECTION;
	case XML_ENTITY_REF_NODE:
		return CONG_NODE_TYPE_ENTITY_REF;
	case XML_ENTITY_NODE:
		return CONG_NODE_TYPE_ENTITY_NODE;
	case XML_PI_NODE:
		return CONG_NODE_TYPE_PI;
	case XML_COMMENT_NODE: 
		return CONG_NODE_TYPE_COMMENT;
	case XML_DOCUMENT_NODE:
		return CONG_NODE_TYPE_DOCUMENT;
	case XML_DOCUMENT_TYPE_NODE:
		return CONG_NODE_TYPE_DOCUMENT_TYPE;
	case XML_DOCUMENT_FRAG_NODE:
		return CONG_NODE_TYPE_DOCUMENT_FRAG;
	case XML_NOTATION_NODE:
		return CONG_NODE_TYPE_NOTATION;
	case XML_HTML_DOCUMENT_NODE:
		return CONG_NODE_TYPE_HTML_DOCUMENT;
	case XML_DTD_NODE:
		return CONG_NODE_TYPE_DTD;
	case XML_ELEMENT_DECL:
		return CONG_NODE_TYPE_ELEMENT_DECL;
	case XML_ATTRIBUTE_DECL:
		return CONG_NODE_TYPE_ATRRIBUTE_DECL;
	case XML_ENTITY_DECL:
		return CONG_NODE_TYPE_ENTITY_DECL;
	case XML_NAMESPACE_DECL:
		return CONG_NODE_TYPE_NAMESPACE_DECL;
	case XML_XINCLUDE_START:
		return CONG_NODE_TYPE_XINCLUDE_START;
	case XML_XINCLUDE_END:
		return CONG_NODE_TYPE_XINCLUDE_END;
#ifdef LIBXML_DOCB_ENABLED
	case XML_DOCB_DOCUMENT_NODE:
#endif
		return CONG_NODE_TYPE_UNKNOWN;
	}

}

gboolean cong_node_is_tag(CongNodePtr node, const CongXMLChar *tagname)
{
	/* FIXME: what about namespaces? */

	g_return_val_if_fail(node, FALSE);
	g_return_val_if_fail(tagname, FALSE);

	return 0==strcmp(tagname, node->name);
}

/* Method for getting an XPath to the node: */
gchar *cong_node_get_path(CongNodePtr node)
{
	g_return_val_if_fail(node, NULL);

	return xmlGetNodePath(node);
}

gchar *cong_node_debug_description(CongNodePtr node)
{
	gchar *result = NULL; 
	gchar *xpath;
	gchar *cleaned_text = NULL; 

	g_return_val_if_fail(node, NULL);

	xpath = cong_node_get_path(node);

	switch (node->type) {

	default: g_assert_not_reached();

	case XML_ELEMENT_NODE: 
		result = g_strdup_printf("%s at %p, name=\"%s\", path=\"%s\"",
					 cong_node_type_description(cong_node_type(node)),
					 node,
					 node->name,
					 xpath);
		break;
	case XML_ATTRIBUTE_NODE:
	case XML_TEXT_NODE: 
		cleaned_text = cong_util_cleanup_text(node->content);
		result = g_strdup_printf("%s at %p, name=\"%s\", content=\"%s\", path=\"%s\"",
					 cong_node_type_description(cong_node_type(node)),
					 node,
					 node->name,
					 cleaned_text,
					 xpath);
		break;
	case XML_CDATA_SECTION_NODE:
	case XML_ENTITY_REF_NODE:
	case XML_ENTITY_NODE:
	case XML_PI_NODE:
	case XML_COMMENT_NODE: 
		cleaned_text = cong_util_cleanup_text(node->content);
		result = g_strdup_printf("%s at %p, name=\"%s\", content=\"%s\", path=\"%s\"",
					 cong_node_type_description(cong_node_type(node)),
					 node,
					 node->name,
					 cleaned_text,
					 xpath);
		break;
	case XML_DOCUMENT_NODE:
	case XML_DOCUMENT_TYPE_NODE:
	case XML_DOCUMENT_FRAG_NODE:
	case XML_NOTATION_NODE:
	case XML_HTML_DOCUMENT_NODE:
	case XML_DTD_NODE:
	case XML_ELEMENT_DECL:
	case XML_ATTRIBUTE_DECL:
	case XML_ENTITY_DECL:
	case XML_NAMESPACE_DECL:
	case XML_XINCLUDE_START:
	case XML_XINCLUDE_END:
#ifdef LIBXML_DOCB_ENABLED
	case XML_DOCB_DOCUMENT_NODE:
#endif
		result = g_strdup_printf("%s at %p, name=\"%s\", path=\"%s\"",
					 cong_node_type_description(cong_node_type(node)),
					 node,
					 node->name,
					 xpath);
		break;
		
	}

	if (cleaned_text) {
		g_free(cleaned_text);
	}
	g_free(xpath);

	return result;
}

static const gchar* node_type_names[CONG_NODE_TYPE_NUM]=
{
	"CONG_NODE_TYPE_UNKNOWN",

	"CONG_NODE_TYPE_ELEMENT",
	"CONG_NODE_TYPE_ATTRIBUTE",
	"CONG_NODE_TYPE_TEXT",
	"CONG_NODE_TYPE_CDATA_SECTION",
	"CONG_NODE_TYPE_ENTITY_REF",
	"CONG_NODE_TYPE_ENTITY_NODE",
	"CONG_NODE_TYPE_PI",
	"CONG_NODE_TYPE_COMMENT",
	"CONG_NODE_TYPE_DOCUMENT",
	"CONG_NODE_TYPE_DOCUMENT_TYPE",
	"CONG_NODE_TYPE_DOCUMENT_FRAG",
	"CONG_NODE_TYPE_NOTATION",
	"CONG_NODE_TYPE_HTML_DOCUMENT",
	"CONG_NODE_TYPE_DTD",
	"CONG_NODE_TYPE_ELEMENT_DECL",
	"CONG_NODE_TYPE_ATTRIBUTE_DECL",
	"CONG_NODE_TYPE_ENTITY_DECL",
	"CONG_NODE_TYPE_NAMESPACE_DECL",
	"CONG_NODE_TYPE_XINCLUDE_START",
	"CONG_NODE_TYPE_XINCLUDE_END"
};
	


const gchar *cong_node_type_description(enum CongNodeType node_type)
{
	g_return_val_if_fail(node_type<CONG_NODE_TYPE_NUM,"(invalid type)");

	return node_type_names[node_type];

}

/* Methods for accessing attribute values: */
CongXMLChar* cong_node_get_attribute(CongNodePtr node, const CongXMLChar* attribute_name)
{
	g_return_val_if_fail(node, NULL);
	g_return_val_if_fail(attribute_name, NULL);

	return xmlGetProp(node, attribute_name);
}

void cong_node_self_test(CongNodePtr node)
{
	CongNodePtr iter;

	g_return_if_fail(node);

	g_assert(node->doc);

	if (node->content) {
		g_assert(g_utf8_validate(node->content,-1,NULL));
	}

	if (node->prev) {
		g_assert(node->prev->next==node);
		g_assert(node->prev->parent == node->parent);
		g_assert(node->parent);
		g_assert(node->parent->children!=node);
	} else {
		if (node->parent) {
			g_assert(node->parent->children==node);
		}
	}

	if (node->next) {
		g_assert(node->next->prev==node);
		g_assert(node->next->parent == node->parent);
		g_assert(node->parent);
		g_assert(node->parent->last!=node);
	} else {
		if (node->parent) {
			g_assert(node->parent->last==node);
		}
	}

	for (iter=node->children; iter; iter=iter->next) {
		g_assert(iter->parent == node);
	}

	if (node->parent) {
		/* Check forwards for node under parent: */
		for (iter=node->parent->children; iter; iter=iter->next) {
			if (iter==node) {
				break;
			}
		}
		g_assert(iter==node);

		/* Check backwards for node under parent: */
		for (iter=node->parent->last; iter; iter=iter->prev) {
			if (iter==node) {
				break;
			}
		}
		g_assert(iter==node);
	}
}

void cong_node_self_test_recursive(CongNodePtr node)
{
	CongNodePtr iter;

	g_return_if_fail(node);

	cong_node_self_test(node);

	for (iter=node->children; iter; iter=iter->next) {
		cong_node_self_test_recursive(iter);
	}
}

int cong_node_get_length(CongNodePtr node)
{
	/* get length of content; does not include the zero terminator */
	g_return_val_if_fail( (cong_node_type(node) == CONG_NODE_TYPE_TEXT) || (cong_node_type(node) == CONG_NODE_TYPE_COMMENT), 0);

	return xmlStrlen(node->content);
	
}

gboolean cong_node_should_recurse(CongNodePtr node)
{
	g_return_val_if_fail(node, FALSE);

	switch (node->type) {
	case XML_ELEMENT_NODE:
	case XML_ATTRIBUTE_NODE:
		return TRUE;
	case XML_TEXT_NODE:
	case XML_CDATA_SECTION_NODE:
		return TRUE;

	case XML_ENTITY_REF_NODE:
		return FALSE;
	case XML_ENTITY_NODE:
	case XML_PI_NODE:
	case XML_COMMENT_NODE:
	case XML_DOCUMENT_NODE:
	case XML_DOCUMENT_TYPE_NODE:
	case XML_DOCUMENT_FRAG_NODE:
	case XML_NOTATION_NODE:
	case XML_HTML_DOCUMENT_NODE:
	case XML_DTD_NODE:
	case XML_ELEMENT_DECL:
	case XML_ATTRIBUTE_DECL:
	case XML_ENTITY_DECL:
	case XML_NAMESPACE_DECL:
	case XML_XINCLUDE_START:
	case XML_XINCLUDE_END:
#ifdef LIBXML_DOCB_ENABLED
	case XML_DOCB_DOCUMENT_NODE:
#endif
		return TRUE;
	}
}

/* Construction: */
CongNodePtr cong_node_new_element(const char *tagname, CongDocument *doc)
{
	g_return_val_if_fail(tagname, NULL);
	g_return_val_if_fail(doc, NULL);

	return xmlNewDocNode(cong_document_get_xml(doc), NULL, tagname, NULL); /* FIXME: audit the character types here */
}

CongNodePtr cong_node_new_text(const char *text, CongDocument *doc)
{
	return cong_node_new_text_len(text, strlen(text),doc);
}

CongNodePtr cong_node_new_text_len(const char *text, int len, CongDocument *doc)
{
	g_return_val_if_fail(text, NULL);
	g_return_val_if_fail(doc, NULL);

	return xmlNewDocTextLen(cong_document_get_xml(doc), text, len); /* FIXME: audit the character types here */
}

/* Destruction: (the node has to have been unlinked from the tree already): */

void cong_node_free(CongNodePtr node)
{
	g_return_if_fail(node);

	xmlFreeNode(node);
}


void cong_node_recursive_delete(CongDocument *doc, CongNodePtr node)
{
	CongNodePtr iter, next;

	CONG_NODE_SELF_TEST(node);

	iter = node->children; 

	while (iter) {
		next = iter->next;

		CONG_NODE_SELF_TEST(iter);
		
		cong_node_recursive_delete(doc, iter);

		iter = next;
	}

	g_assert(node->children==NULL);
	g_assert(node->last==NULL);

	cong_document_node_make_orphan(doc, node);

	cong_node_free(node);
}

CongNodePtr cong_node_recursive_dup(CongNodePtr node)
{
	return xmlCopyNode(node, TRUE);
}

/* Tree manipulation: */
void cong_node_private_make_orphan(CongNodePtr node)
{
	CongNodePtr former_parent;
	CongNodePtr former_prev;
	CongNodePtr former_next;

	g_return_if_fail(node);

	CONG_NODE_SELF_TEST(node);

	former_parent = node->parent;
	former_prev = node->prev;
	former_next = node->next;

	if (former_parent) {
		CONG_NODE_SELF_TEST(former_parent);
	}
	
	if (former_prev) {
		CONG_NODE_SELF_TEST(former_prev);
	}

	if (former_next) {
		CONG_NODE_SELF_TEST(former_next);	
	}

	if (node->parent) {

		CONG_NODE_SELF_TEST(node->parent);

		if (node->prev) {
			g_assert(node->parent->children != node);
			g_assert(former_prev->next == node);

			former_prev->next = former_next;
			node->prev = NULL;
		} else {
			g_assert(node->parent->children == node);
			node->parent->children = former_next;
		}
		
		if (node->next) {
			g_assert(node->parent->last != node);
			g_assert(former_next->prev == node);

			former_next->prev = former_prev;
			node->next = NULL;
		} else {
			g_assert(node->parent->last == node);
			node->parent->last = former_prev;
		}

		node->parent = NULL;
	 
	} else {
		g_assert(node->prev == NULL);
		g_assert(node->next == NULL);
	}


	/* Postconditions: */
	{
		g_assert(node->parent == NULL);
		g_assert(node->prev == NULL);
		g_assert(node->next == NULL);

		CONG_NODE_SELF_TEST(node);

		if (former_parent) {
			CONG_NODE_SELF_TEST(former_parent);
		}

		if (former_prev) {
			g_assert(former_prev->next == former_next);
			CONG_NODE_SELF_TEST(former_prev);
		}
		if (former_next) {
			g_assert(former_next->prev == former_prev);
			CONG_NODE_SELF_TEST(former_next);
		}
	}
}

void cong_node_private_add_after(CongNodePtr node, CongNodePtr older_sibling)
{
	g_return_if_fail(node);
	g_return_if_fail(older_sibling);
	g_return_if_fail(older_sibling->parent);
	g_return_if_fail(node!=older_sibling);

	CONG_NODE_SELF_TEST(node);
	CONG_NODE_SELF_TEST(older_sibling);

	cong_node_private_make_orphan(node);
#if 1
	node->parent = older_sibling->parent;
	node->prev = older_sibling;

	if (older_sibling->next) {
		g_assert(older_sibling->parent->last!=older_sibling);
		g_assert(older_sibling->next->prev==older_sibling);

		node->next = older_sibling->next;
		older_sibling->next->prev = node;

	} else {
		g_assert(older_sibling->parent->last==older_sibling);

		older_sibling->parent->last = node;
	}

	older_sibling->next = node;

	/* Postconditions: */
	{
		g_assert( older_sibling->next == node );
		g_assert( older_sibling == node->prev );
		g_assert( older_sibling->parent == node->parent );

		CONG_NODE_SELF_TEST(node);
		CONG_NODE_SELF_TEST(older_sibling);
		CONG_NODE_SELF_TEST(older_sibling->parent);

		if (node->next) {
			CONG_NODE_SELF_TEST(node->next);
		}
	}
#endif
#if 0
	xmlAddNextSibling(older_sibling, node);
#endif

}

void cong_node_private_add_before(CongNodePtr node, CongNodePtr younger_sibling)
{
	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);
	g_return_if_fail(younger_sibling->parent);
	g_return_if_fail(node!=younger_sibling);

	CONG_NODE_SELF_TEST(node);
	CONG_NODE_SELF_TEST(younger_sibling);

	cong_node_private_make_orphan(node);

#if 1
	node->parent = younger_sibling->parent;
	node->next = younger_sibling;

	if (younger_sibling->prev) {
		g_assert(younger_sibling->parent->children!=younger_sibling);
		g_assert(younger_sibling->prev->next==younger_sibling);

		node->prev = younger_sibling->prev;
		younger_sibling->prev->next = node;

	} else {
		g_assert(younger_sibling->parent->children==younger_sibling);

		younger_sibling->parent->children = node;
	}

	younger_sibling->prev = node;

	/* Postconditions: */
	{
		g_assert( younger_sibling->prev == node );
		g_assert( younger_sibling == node->next );
		g_assert( younger_sibling->parent == node->parent );

		CONG_NODE_SELF_TEST(node);
		CONG_NODE_SELF_TEST(younger_sibling);
		CONG_NODE_SELF_TEST(younger_sibling->parent);

		if (node->prev) {
			CONG_NODE_SELF_TEST(node->prev);
		}
	}
#endif
#if 0
	xmlAddPrevSibling(younger_sibling, node);
#endif

}

void cong_node_private_set_parent(CongNodePtr node, CongNodePtr adoptive_parent)
{
	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);
	g_return_if_fail(node!=adoptive_parent);

	CONG_NODE_SELF_TEST(node);
	CONG_NODE_SELF_TEST(adoptive_parent);

	cong_node_private_make_orphan(node);

#if 1
	if (adoptive_parent->last) {
		cong_node_private_add_after(node, adoptive_parent->last);
	} else {
		g_assert(adoptive_parent->children == NULL);

		adoptive_parent->children = node;
		adoptive_parent->last = node;
		node->parent = adoptive_parent;
	}

	/* Postconditions: */
	{
		g_assert(node->parent == adoptive_parent);
		g_assert(adoptive_parent->last == node);
		g_assert(node->next == NULL);
		CONG_NODE_SELF_TEST(node);
		CONG_NODE_SELF_TEST(adoptive_parent);
	}
#endif
#if 0
	xmlAddChild(adoptive_parent, node);
#endif

}

void cong_node_private_set_text(CongNodePtr node, const xmlChar *new_content)
{
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	xmlNodeSetContent(node, new_content);
}

void cong_node_private_set_attribute(CongNodePtr node, const xmlChar *name, const xmlChar *value)
{
	g_return_if_fail(node);
	g_return_if_fail(name);
	g_return_if_fail(value);

	xmlSetProp(node, name, value);
}

void cong_node_private_remove_attribute(CongNodePtr node, const xmlChar *name)
{
	g_return_if_fail(node);
	g_return_if_fail(name);

	xmlUnsetProp(node, name);
}

const gchar *xml_frag_data_nice(CongNodePtr x)
{
	const char *s;
	
	g_return_val_if_fail(x->type==XML_TEXT_NODE, NULL);
	
	s = x->content; /* FIXME:  hackish cast from xmlChar* to char* */
	if (!s) s = fake_data;
	
	return(s);
}


const gchar *xml_frag_name_nice(CongNodePtr x)
{
	const char *s;
	
	s = cong_node_name(x);
	if (!s) s = fake_data;

	return(s);
}


/* Tested and works */

static char *cat_string(char *head, const char *tail)
{
	char *new = g_malloc(strlen(head) + strlen(tail) + 1);
	strcpy(new, head);
	strcat(new, tail);
	g_free(head);

	return(new);
}

/* Recursively traverses the document from that node concatenating the character data into a string */
char *xml_fetch_clean_data(CongNodePtr x)
{
	CongNodePtr n0;
	char *s = 0, *s_sub;

	n0 = cong_node_first_child(x);
	if (!n0) return NULL;

	s = malloc(1);
	*s = '\0';
	
	for ( ; n0; n0 = cong_node_next(n0))
	{
		if (cong_node_type(n0) == CONG_NODE_TYPE_TEXT)
		{
			s = cat_string(s, xml_frag_data_nice(n0));
		}
		else if (cong_node_type(n0) == CONG_NODE_TYPE_ELEMENT)
		{
			s_sub = xml_fetch_clean_data(n0);
			if (s_sub)
			{
				s = cat_string(s, s_sub);
				free(s_sub);
			}
		}
	}
	
	return(s);
}


GList* xml_all_present_span_elements(CongDispspec *ds, CongNodePtr node) 
{
	GList* list = NULL;

	g_return_val_if_fail(ds, NULL);
	g_return_val_if_fail(node, NULL);
	g_return_val_if_fail( cong_node_type(node) == CONG_NODE_TYPE_TEXT, NULL);

	/*  we should be at text node.  grab span element above */
	if (node->parent) {
	       node = node->parent;
	} 
	else {
	       return NULL;
	}
	
	while( (cong_node_type(node) == CONG_NODE_TYPE_ELEMENT) && 
	       (cong_dispspec_element_span(ds, cong_node_name(node)) ) ) {

		/*  prepend node to list */
		list = g_list_prepend(list, (gpointer *) node);

		/*  move up tree */
		if (node->parent) {
			node = node->parent;
		}
		else {
			break;
		}
	}

	return list;
}

GList* xml_all_valid_span_elements(CongDispspec *ds, CongNodePtr node) 
{
	GList* list = NULL;
	CongDispspecElement *ds_element;

	g_return_val_if_fail(ds, NULL);
	g_return_val_if_fail(node, NULL);

	/*  we should be at text node.  grab span element above */
	if (node->parent) {
	       node = node->parent;
	} 
	else {
	       return NULL;
	}

	/* FIXME: this adds all span tags; it makes no validity checks */

	for (ds_element = cong_dispspec_get_first_element(ds); ds_element; ds_element = cong_dispspec_element_next(ds_element))
	{
		if (cong_dispspec_element_is_span(ds_element)) {
			/*  prepend node to list */
			list = g_list_prepend(list, (gpointer *) ds_element);			
		}
	}

	return list;
}

/**
 * In the presence of a DTD, adds all children required
 * by the DTD to the node.  This is a recursive process,
 * and as such will add subchildren if necessary.
 * Also, this function adds any optional text nodes, such
 * that the user may type. Along the way, if there are a
 * set of elements that the user is required to choose one
 * of, the user will be prompted with a dialog.  If the
 * user aborts, this function will remove any added
 * children and default to just adding one text node
 * to the input node.
 *
 * If no DTD is present, this function will default to
 * simply adding a text node under the node as well.
 *
 * Returns a boolean telling whether the structure of the
 * node passed is now valid under a DTD.  If no DTD exists,
 * this will return FALSE.
 */
gboolean xml_add_required_children(CongDocument *doc, CongNodePtr node) {
	gboolean success;
	xmlNodePtr new_node;

	success = xml_add_required_sub_elements(doc, node);
	/*  if we fail, probably b/c of no DTD, */
	/*  clean up the inside of the node  */
	/*  and just toss in a text node */
	if (! success) {
		/*  free all children of the node */
		if (node->children != NULL) {
			g_assert(0); /* FIXME:  need to signal this to the MVC framework */
			xmlFreeNodeList(node->children);
		}
		
		/*  set the node to empty */
		node->children = NULL;

		/*  add a text node */
		new_node = cong_node_new_text("", doc);
		cong_document_node_set_parent(doc, new_node, node);
	}

	return success;
}
		
/**
 * Helper function to get the xmlElementPtr within the DTD for a node
 */
xmlElementPtr xml_get_dtd_element(CongDocument *cong_doc, CongNodePtr node) {
	xmlDocPtr doc;
	xmlElementPtr elemDecl = NULL;
	const xmlChar *prefix = NULL;
	gboolean extsubset = FALSE;

	/*  check cong_doc */
	g_return_val_if_fail(cong_doc, NULL);

	/*  check node */
	g_return_val_if_fail(node, NULL);
	
	/*  check that node has embedded document */
	g_return_val_if_fail(node->doc, NULL);

	/*  set document */
	doc = node->doc;
	
	/*  check that document has DTD */
	g_return_val_if_fail(doc->intSubset || doc->extSubset, NULL);

	if (node->type != XML_ELEMENT_NODE) { return NULL; }

	/*  ensure element has a name */
	g_return_val_if_fail(node->name, NULL);

	/*
	 * Fetch the declaration for the qualified name.
	 */
	if ((node->ns != NULL) && (node->ns->prefix != NULL)) {
		prefix = node->ns->prefix;
	}
	
	/*  search the internal subset DTD for a description of this elemenet */
	if (prefix != NULL) {
		elemDecl = xmlGetDtdQElementDesc(doc->intSubset,
						 node->name, prefix);
	}
	
	/*  if that didn't work, try the external subset */
	if ((elemDecl == NULL) && (doc->extSubset != NULL)) {
	    elemDecl = xmlGetDtdQElementDesc(doc->extSubset,
		                             node->name, prefix);
	    if (elemDecl != NULL) {
		    extsubset = TRUE;
	    }
	}

	/*
	 * If the qualified name didn't work, try the
	 * non-qualified name.
	 * Fetch the declaration for the non qualified name
	 * This is "non-strict" validation should be done on the
	 * full QName but in that case being flexible makes sense.
	 */
	if (elemDecl == NULL) {
		elemDecl = xmlGetDtdElementDesc(doc->intSubset, node->name);
	}

	if ((elemDecl == NULL) && (doc->extSubset != NULL)) {
		elemDecl = xmlGetDtdElementDesc(doc->extSubset, node->name);
		if (elemDecl != NULL) {
			extsubset = TRUE;
		}
	}

	return elemDecl;
}


/**
 * Helper function to add the required children
 * to a node.
 *
 * Returns whether the node is now valid
 */
static gboolean xml_add_required_sub_elements(CongDocument *cong_doc, CongNodePtr node) {
	xmlDocPtr doc;
	xmlElementPtr elemDecl = NULL;
	xmlElementContentPtr content;
	const xmlChar *name;
	const xmlChar *prefix = NULL;
	gboolean extsubset = FALSE;

	/*  check cong_doc */
	g_return_val_if_fail(cong_doc, FALSE);

	/*  check node */
	g_return_val_if_fail(node, FALSE);
	
	/*  check that node has embedded document */
	g_return_val_if_fail(node->doc, FALSE);

	/*  set document */
	doc = node->doc;
	
	/*  check that document has DTD */
	g_return_val_if_fail(doc->intSubset || doc->extSubset, FALSE);

	/*  if this is not an element node, it has no children */
	if (node->type != XML_ELEMENT_NODE) { return TRUE; }

	/*  ensure element has a name */
	g_return_val_if_fail(node->name, FALSE);

	/*
	 * Fetch the declaration for the qualified name.
	 */
	if ((node->ns != NULL) && (node->ns->prefix != NULL)) {
		prefix = node->ns->prefix;
	}
	
	/*  search the internal subset DTD for a description of this elemenet */
	if (prefix != NULL) {
		elemDecl = xmlGetDtdQElementDesc(doc->intSubset,
						 node->name, prefix);
	}
	
	/*  if that didn't work, try the external subset */
	if ((elemDecl == NULL) && (doc->extSubset != NULL)) {
	    elemDecl = xmlGetDtdQElementDesc(doc->extSubset,
		                             node->name, prefix);
	    if (elemDecl != NULL) {
		    extsubset = TRUE;
	    }
	}

	/*
	 * If the qualified name didn't work, try the
	 * non-qualified name.
	 * Fetch the declaration for the non qualified name
	 * This is "non-strict" validation should be done on the
	 * full QName but in that case being flexible makes sense.
	 */
	if (elemDecl == NULL) {
		elemDecl = xmlGetDtdElementDesc(doc->intSubset, node->name);
	}

	if ((elemDecl == NULL) && (doc->extSubset != NULL)) {
		elemDecl = xmlGetDtdElementDesc(doc->extSubset, node->name);
		if (elemDecl != NULL) {
			extsubset = TRUE;
		}
	}

	/*  if it's still null, give up */
	g_return_val_if_fail(elemDecl, FALSE);

	switch (elemDecl->etype) {
	case XML_ELEMENT_TYPE_UNDEFINED:
		return FALSE;
		
	case XML_ELEMENT_TYPE_EMPTY:
		/*  doesn't need any elements */
		return TRUE;
	case XML_ELEMENT_TYPE_ANY:
		/*  free-for-all!  no elements required. */
		return TRUE;
	case XML_ELEMENT_TYPE_MIXED:
		/*  fall through to element case */
	case XML_ELEMENT_TYPE_ELEMENT:
		/*  get the content pointer */
		content = elemDecl->content;
		
		g_return_val_if_fail(content, FALSE);

		return xml_add_required_content(cong_doc, content, node);
	default:
		g_print("Invalid Element type: %d\n", elemDecl->etype);
		return FALSE;
	}
}

/**
 * Adds required children from an
 * input content model.
 *
 * Returns whether all required content was added
 */
static gboolean xml_add_required_content(CongDocument *cong_doc, xmlElementContentPtr content, xmlNodePtr node) {
	xmlNodePtr new_node;
	
	/*  if we require an occurrence of this node */
	if ((content->ocur == XML_ELEMENT_CONTENT_ONCE) || (content->ocur == XML_ELEMENT_CONTENT_PLUS)) {
		switch(content->type) {
		case XML_ELEMENT_CONTENT_PCDATA: 
			/*  create a text node, add it */
			g_print("xml_add_required_content: adding new text node under node %s\n", node->name);
			new_node = cong_node_new_text("", cong_doc);
			cong_document_node_set_parent(cong_doc, new_node, node);
			return TRUE;
		case XML_ELEMENT_CONTENT_ELEMENT: 
			/*  ensure the element has a name */
			g_return_val_if_fail(content->name, FALSE);
			
			/*  create the element and add it */
			g_print("xml_add_required_content: adding new node %s under node %s\n", content->name, node->name);
			new_node = cong_node_new_element(content->name, cong_doc);
			cong_document_node_set_parent(cong_doc, new_node, node);
			
			/*  recur on the new node to add anything it needs */
			return xml_add_required_sub_elements(cong_doc, new_node);

		case XML_ELEMENT_CONTENT_SEQ: 
			/*  seq -- add the first of the list, and */
			/*  recur on rest */
			if (!xml_add_required_content(cong_doc, content->c1, node)) {
				return FALSE;
			}
			
			return xml_add_required_content(cong_doc, content->c2, node);
		case XML_ELEMENT_CONTENT_OR:
			return xml_add_required_content_choice(cong_doc, content, node);
		default:
			g_print("xml_add_required_content: Invalid content type: %d\n", content->type);
			return FALSE;
		}
	}
	/* for ? and * add only text nodes */
	else if ((content->ocur == XML_ELEMENT_CONTENT_OPT) || (content->ocur == XML_ELEMENT_CONTENT_MULT)) {
		/* we want to insert any text nodes */
		xml_add_optional_text_nodes(cong_doc, content, node);
		return TRUE;
	}
	else {
		g_print("xml_add_required_content: Invalid content occurrence value: %d\n", content->type);
		return FALSE;
	}
}

/**
 * Allows the user to select between a set
 * of choices for a required content element.
 * Adds that choice to the node.
 *
 * Returns whether the choice was correctly selected and added
 */
static gboolean xml_add_required_content_choice(CongDocument *cong_doc, xmlElementContentPtr content, xmlNodePtr node) {
	GString *description;
	GList *list = NULL;
	CongNodePtr new_node;
	const xmlChar *names[256];
	gchar *element_name;
	gint i, response, size, length;

	/*  get potential children for this content element */
	size = 0;
	length = xmlValidGetPotentialChildren(content, names, &size, 256);
	if (length == -1) {
		return FALSE;
	}

	/*  turn array into a GList to pass to a dialog */
	for (i = 0; i < length; i++) {
		list = g_list_prepend(list, (gpointer)(names[i]));
	}
	
	/*  sort the list in alphabetical order */
	list = g_list_sort(list, (GCompareFunc) strcmp );

	/*  create description for dialog */
	description = g_string_new("");
	g_string_printf(description, _("The element \"%s\" requires a child to be valid.  Please choose one of the following child types."),
			node->name);
	
	/*  select a dialog element */
	element_name = string_selection_dialog(_("Required Children Choices"), description->str, list);

	/*  free the string and the list */
	g_string_free(description, FALSE);
	g_list_free(list);

	/*  if user didn't respond, return false */
	g_return_val_if_fail(element_name, FALSE);

	/*  add the element */
	new_node = cong_node_new_element(element_name, cong_doc);
	cong_document_node_set_parent(cong_doc, new_node, node);
	
	/*  free the returned string */
	g_free(element_name);

	/*  recur on the new node to add anything it needs */
	return xml_add_required_sub_elements(cong_doc, new_node);
}
	
 
/**
 * Adds an optional text node when it
 * conforms to the content model, such that
 * the user may type.
 *
 * @return whether a text node has been added
 */
static gboolean xml_add_optional_text_nodes(CongDocument *cong_doc, xmlElementContentPtr content, xmlNodePtr node) {
	xmlNodePtr new_node;

	if (content->type == XML_ELEMENT_CONTENT_PCDATA) {
		/*  create a text node, add it */
		g_print("xml_add_optional_text_nodes: adding new text node under node %s\n", node->name);
		new_node = cong_node_new_text("", cong_doc);
		cong_document_node_set_parent(cong_doc, new_node, node);
		return TRUE;
	}
	else if (content->type == XML_ELEMENT_CONTENT_OR) {
		/*  if we add a text node in this or, quit recurring through it */
		if (xml_add_optional_text_nodes(cong_doc, content->c1, node)) {
			return TRUE;
		}
		/*  otherwise, keep search for a text node */
		else {
			return xml_add_optional_text_nodes(cong_doc, content->c2, node);
		}
	}

	return FALSE;
}

/**
 * Reimplementation of xmlValidGetPotentialChildren that *only*
 * returns children of element type, not of PCDATA type.
 *
 * Otherwise, behaves the same as xmlValidGetPotentialChildren.
 */
static gint xml_valid_get_potential_element_children(xmlElementContent *ctree, const xmlChar **list,
						     gint *len, gint max) {
    gint i;

    if ((ctree == NULL) || (list == NULL) || (len == NULL))
        return(-1);
    if (*len >= max) return(*len);

    switch (ctree->type) {
    case XML_ELEMENT_CONTENT_PCDATA: 
	    break;
    case XML_ELEMENT_CONTENT_ELEMENT: 
	    for (i = 0; i < *len;i++) {
		    if (xmlStrEqual(ctree->name, list[i])) return(*len);
	    }
	    list[(*len)++] = ctree->name;
	    break;
    case XML_ELEMENT_CONTENT_SEQ: 
	    xml_valid_get_potential_element_children(ctree->c1, list, len, max);
	    xml_valid_get_potential_element_children(ctree->c2, list, len, max);
	    break;
    case XML_ELEMENT_CONTENT_OR:
	    xml_valid_get_potential_element_children(ctree->c1, list, len, max);
	    xml_valid_get_potential_element_children(ctree->c2, list, len, max);
	    break;
    }
    
    return(*len);
}

/**
 * Wrapper function for libxml's xmlValidGetValidElements that
 * handles the corner case of when the parent has no children.
 * xmlValidGetValidElements parameter structure cannot handle this case.
 *
 * Otherwise, behaves the same as xmlValidGetValidElements.
 */
static gint wrap_xml_valid_get_valid_elements(xmlNode *parent, xmlNode *next_sibling, const xmlChar ** elements, gint max) 
{
	xmlElement *element_desc;
	gint nb_elements, i;
	
	if (parent == NULL) {
		/*  nothing to work with */
		if (next_sibling == NULL) {
			return -1;
		}
		/*  root node -- no valid siblings */
		else {
			return 0;
		}
	}

	/* if parent is document node -- no possible insertions */
	if (parent->type == XML_DOCUMENT_NODE) {
		return 0;
	}

	/*  ensure that the parent has a document */
	g_return_val_if_fail(parent->name, -1);
	g_return_val_if_fail(parent->doc, -1);

	/*  if no children under parent, use xmlValidGetPotentialChildren */
	if (parent->children == NULL) {
		/*  try to find the element description for the parent */
		/*  internal subset */
		element_desc = xmlGetDtdElementDesc(parent->doc->intSubset,
						    parent->name);
		/*  if that failed, try external subset */
		if ((element_desc == NULL) && (parent->doc->extSubset != NULL)) {
			element_desc = xmlGetDtdElementDesc(parent->doc->extSubset,
							    parent->name);
		}
		/*  if we still failed, give up */
		g_return_val_if_fail(element_desc, -1);
		
		/*  get potential children */
		nb_elements = xml_valid_get_potential_element_children(element_desc->content,
								       elements, &nb_elements, max);

	}
	else {
		/*  if we're inserting after the last child */
		if (next_sibling == NULL) {
			nb_elements = xmlValidGetValidElements(parent->last, NULL, elements, max);
		}
		else {
			nb_elements = xmlValidGetValidElements(next_sibling->prev, next_sibling, elements, max);
		}
	}

	return nb_elements;
}
		
	

/**
 * Get the set of valid children for a node.  If the document
 * contains a DTD, this function will return the intersection
 * of the valid children and elements in the display spec of 
 * type tag_type.
 * If the document does not contain a dtd, the function will fallback
 * on returning the elements in the display spec of type tag_type.
 *
 * @param ds CongDispsec* display spec
 * @param node CongNodePtr node for which to get valid children
 * @param tag_type either CONG_ELEMENT_TYPE_STRUCTURAL, CONG_ELEMENT_TYPE_SPAN, or CONG_ELEMENT_TYPE_ALL
 * @return GList of CongDispspecElement
 */
GList* xml_get_valid_children(CongDispspec* ds, CongNodePtr node, enum CongElementType tag_type) 
{
	const xmlChar  *elements[256];
	gint result;

	result = wrap_xml_valid_get_valid_elements(node, node->last, elements, 256);
	if (result != -1) {
		return xml_filter_valid_children_with_dispspec(ds, elements, result, tag_type);
	}
	else {
		return xml_get_elements_from_dispspec(ds, tag_type);
	}
}


/**
 * Get the set of valid previous siblings for a node.  If the document
 * contains a DTD, this function will return the intersection
 * of the valid previous siblings and elements in the display spec of 
 * type tag_type.
 * If the document does not contain a dtd, the function will fallback
 * on returning the elements in the display spec of type tag_type.
 *
 * @param ds CongDispsec* display spec
 * @param node CongNodePtr node for which to get valid previous siblings
 * @param tag_type either CONG_ELEMENT_TYPE_STRUCTURAL, CONG_ELEMENT_TYPE_SPAN, or CONG_ELEMENT_TYPE_ALL
 * @return GList of CongDispspecElement
 */
GList* xml_get_valid_previous_sibling(CongDispspec* ds, CongNodePtr node, enum CongElementType tag_type) {
	const xmlChar  *elements[256];
	gint result;
	
	result = wrap_xml_valid_get_valid_elements(node->parent, node, elements, 256);
	if (result != -1) {
		return xml_filter_valid_children_with_dispspec(ds, elements, result, tag_type);
	}
	else {
		return xml_get_elements_from_dispspec(ds, tag_type);
	}
}

/**
 * Get the set of valid next siblings for a node.  If the document
 * contains a DTD, this function will return the intersection
 * of the valid next siblings and elements in the display spec of 
 * type tag_type.
 * If the document does not contain a dtd, the function will fallback
 * on returning the elements in the display spec of type tag_type.
 *
 * @param ds CongDispsec* display spec
 * @param node CongNodePtr node for which to get valid next siblings
 * @param tag_type either CONG_ELEMENT_TYPE_STRUCTURAL, CONG_ELEMENT_TYPE_SPAN, or CONG_ELEMENT_TYPE_ALL
 * @return GList of CongDispspecElement
 */
GList* xml_get_valid_next_sibling(CongDispspec* ds, CongNodePtr node, enum CongElementType tag_type) {
	const xmlChar  *elements[256];
	gint result;
		
	result = wrap_xml_valid_get_valid_elements(node->parent, node->next, elements, 256);
	if (result != -1) {
		return xml_filter_valid_children_with_dispspec(ds, elements, result, tag_type);
	}
	else {
		return xml_get_elements_from_dispspec(ds, tag_type);
	}
}

/**
 * Find the intersection of a list of valid children
 * and a displayspec, returning a list of the displayspec elements
 * that are in the intersection.
 */
static GList *xml_filter_valid_children_with_dispspec(CongDispspec* ds, const xmlChar **elements, gint elements_length, enum CongElementType tag_type) {
	CongDispspecElement *element;
	GList *list = NULL;
	gint i;

	for (i = 0; i < elements_length; i++) {
		element = cong_dispspec_lookup_element(ds, elements[i]);
		if ( ( element != NULL ) &&
		     ( (tag_type == CONG_ELEMENT_TYPE_ALL) ||
		       ( ( tag_type == CONG_ELEMENT_TYPE_STRUCTURAL ) && (cong_dispspec_element_is_structural(element) )) ||
		       ( ( tag_type == CONG_ELEMENT_TYPE_SPAN ) && (cong_dispspec_element_is_span(element) ) ) ) ) { 

			list = g_list_prepend(list, element);
		}
	}

	return list;
}

/**
 * Get all elements from the display spec that
 * are of tag_type.
 */
static GList* xml_get_elements_from_dispspec(CongDispspec* ds, enum CongElementType tag_type) {
	CongDispspecElement *element;
	GList* list = NULL;

	for (element = cong_dispspec_get_first_element(ds); element; element = cong_dispspec_element_next(element)) {
		if ( (tag_type == CONG_ELEMENT_TYPE_ALL) || 
		     ( (tag_type == CONG_ELEMENT_TYPE_STRUCTURAL ) && (cong_dispspec_element_is_structural(element)) ) ||
		     ( (tag_type == CONG_ELEMENT_TYPE_SPAN ) && (cong_dispspec_element_is_span(element)) ) ) {
			list = g_list_prepend(list, element);
		}
	}
	
	return list;
}

/*
  Function to remove a node x from the tree; all its children become children of x's parents in the natural place in the tree.
 */
void xml_tag_remove(CongDocument *doc, CongNodePtr x)
{
	CongNodePtr n0;
	CongNodePtr n0_next;

	g_return_if_fail(x);

	/* GREP FOR MVC */

#if 1
	for (n0 = x->children; n0; n0 = n0_next) {
		n0_next = n0->next;
		
		cong_document_node_add_before(doc, n0, x);
	}

	cong_document_node_make_orphan(doc, x);

	cong_node_free(x);
#else
	n0 = cong_node_first_child(x);

	if (n0) {
		n0->prev = x->prev;
	}

	if (NULL==x->prev) {
		x->parent->children = n0;
	} else {
		x->prev->next = n0;
	}

	for (; n0->next; n0 = n0->next) {
		n0->parent = x->parent;
	}
	n0->parent = x->parent;

	n0->next = x->next;
	if (x->next) {
		x->next->prev = n0;
	} else {
		x->parent->last = n0;
	}
	
	x->next = NULL;
	x->prev = NULL;
	x->parent = NULL;
	x->children = NULL;
	x->last = NULL;

	xmlFreeNode(x);
#endif
}

GnomeVFSResult
cong_xml_save_to_vfs(xmlDocPtr doc_ptr, 
		     GnomeVFSURI *file_uri,	
		     GnomeVFSFileSize *output_file_size)
{
	xmlChar* mem;
	int size;
	GnomeVFSResult vfs_result;
	GnomeVFSHandle *vfs_handle;
	GnomeVFSFileSize written_size;

	g_return_val_if_fail(doc_ptr, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(file_uri, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(output_file_size, GNOME_VFS_ERROR_BAD_PARAMETERS);

	/* Dump to a memory buffer. then write out buffer to GnomeVFS: */
	xmlDocDumpMemory(doc_ptr,
			 &mem,
			 &size);
	g_assert(mem);
	g_assert(size>0);

	*output_file_size = size;

	vfs_result = gnome_vfs_create_uri(&vfs_handle,
					  file_uri,
					  GNOME_VFS_OPEN_WRITE,
					  FALSE,
					  0644
					);

	if (vfs_result != GNOME_VFS_OK) {
		return vfs_result;
	}

	vfs_result = gnome_vfs_write(vfs_handle,
				     mem,
				     *output_file_size,
				     &written_size);

	if (vfs_result != GNOME_VFS_OK) {
		gnome_vfs_close(vfs_handle);
		return vfs_result;
	}

	g_assert(*output_file_size == written_size);

	vfs_result = gnome_vfs_close(vfs_handle);

	return vfs_result;
}

/* Extensions to libxml: */
xmlAttrPtr	xmlNewProp_NUMBER	(xmlNodePtr node,
					 const xmlChar *name,
					 int value)
{
	gchar *textual_value = g_strdup_printf("%i", value);
	xmlAttrPtr attr = xmlNewProp(node, name, textual_value);

	g_free(textual_value);

	return attr;
}
