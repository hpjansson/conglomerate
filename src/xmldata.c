/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>                                                            
#include <gtk/gtk.h>                                                            

#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-util.h"
#include "cong-command.h"

#define DEBUG_VALID_INSERTIONS 0

char fake_data[] = "";

static gboolean cong_command_add_xml_add_required_sub_elements(CongCommand *cmd, CongNodePtr node);
static gboolean cong_command_add_xml_add_optional_text_nodes(CongCommand *cmd, xmlElementContentPtr content, xmlNodePtr node);
static gboolean cong_command_add_xml_add_required_content(CongCommand *cmd, xmlElementContentPtr content, xmlNodePtr node);
static gboolean cong_command_add_xml_add_required_content_choice(CongCommand *cmd, xmlElementContentPtr content, xmlNodePtr node);

static void xml_get_or_content_list(xmlElementContentPtr content, GList* list);
static gint xml_valid_get_potential_element_children(xmlElementContent *ctree, const xmlChar **list, int *len, int max);
static gint wrap_xml_valid_get_valid_elements(CongDocument *doc, xmlNode *parent, xmlNode *next_sibling, const xmlChar ** elements, gint max);
static GList *xml_filter_valid_children_with_dispspec(CongDispspec* ds, const xmlChar **elements, 
						      gint elements_length, enum CongElementType tag_type);
static GList* xml_get_elements_from_dispspec(CongDispspec* ds, enum CongElementType tag_type);


/* Other stuff: */
const gchar *xml_frag_data_nice(CongNodePtr x)
{
	const char *s;
	
	g_return_val_if_fail(cong_node_is_valid_cursor_location (x), NULL);
	
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
	       (cong_dispspec_element_span(ds, cong_node_xmlns(node), cong_node_name(node)) ) ) {

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
 * xml_add+required_children
 * @doc:
 * @node:
 *
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
 * Returns: a boolean telling whether the structure of the
 * node passed is now valid under a DTD.  If no DTD exists,
 * this will return FALSE.
 */
gboolean cong_command_add_xml_add_required_children(CongCommand *cmd, CongNodePtr node) {
	gboolean success;
	xmlNodePtr new_node;
	CongDocument *doc;

	g_return_val_if_fail (IS_CONG_COMMAND(cmd), FALSE);
	g_return_val_if_fail (node, FALSE);

	doc = cong_command_get_document (cmd);

	success = cong_command_add_xml_add_required_sub_elements(cmd, node);
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
		cong_command_add_node_set_parent(cmd, new_node, node);
	}

	return success;
}
		
/**
 * Helper function to add the required children
 * to a node.
 *
 * Returns whether the node is now valid
 */
static gboolean cong_command_add_xml_add_required_sub_elements(CongCommand *cmd, CongNodePtr node) {
	xmlDocPtr doc;
	xmlElementPtr elemDecl = NULL;
	xmlElementContentPtr content;
	const xmlChar *name;
	const xmlChar *prefix = NULL;
	gboolean extsubset = FALSE;

	g_return_val_if_fail (IS_CONG_COMMAND(cmd), FALSE);
	g_return_val_if_fail (node, FALSE);

	/*  check that node has embedded document */
	g_return_val_if_fail (node->doc, FALSE);

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

		return cong_command_add_xml_add_required_content(cmd, content, node);
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
static gboolean cong_command_add_xml_add_required_content (CongCommand *cmd, 
							   xmlElementContentPtr content, 
							   xmlNodePtr node) 
{
	xmlNodePtr new_node;
	CongDocument *cong_doc;

	g_return_val_if_fail (IS_CONG_COMMAND(cmd), FALSE);

	cong_doc = cong_command_get_document (cmd);
	
	/*  if we require an occurrence of this node */
	if ((content->ocur == XML_ELEMENT_CONTENT_ONCE) || (content->ocur == XML_ELEMENT_CONTENT_PLUS)) {
		switch(content->type) {
		case XML_ELEMENT_CONTENT_PCDATA: 
			/*  create a text node, add it */
			g_print("xml_add_required_content: adding new text node under node %s\n", node->name);
			new_node = cong_node_new_text("", cong_doc);
			cong_command_add_node_set_parent(cmd, new_node, node);
			return TRUE;
		case XML_ELEMENT_CONTENT_ELEMENT: 
			/*  ensure the element has a name */
			g_return_val_if_fail(content->name, FALSE);
			
			/*  create the element and add it */
			g_print("xml_add_required_content: adding new node %s under node %s\n", content->name, node->name);
			new_node = cong_node_new_element(content->prefix, content->name, cong_doc);
			cong_command_add_node_set_parent(cmd, new_node, node);
			
			/*  recur on the new node to add anything it needs */
			return cong_command_add_xml_add_required_sub_elements(cmd, new_node);

		case XML_ELEMENT_CONTENT_SEQ: 
			/*  seq -- add the first of the list, and */
			/*  recur on rest */
			if (!cong_command_add_xml_add_required_content(cmd, content->c1, node)) {
				return FALSE;
			}
			
			return cong_command_add_xml_add_required_content(cmd, content->c2, node);
		case XML_ELEMENT_CONTENT_OR:
			return cong_command_add_xml_add_required_content_choice(cmd, content, node);
		default:
			g_print("xml_add_required_content: Invalid content type: %d\n", content->type);
			return FALSE;
		}
	}
	/* for ? and * add only text nodes */
	else if ((content->ocur == XML_ELEMENT_CONTENT_OPT) || (content->ocur == XML_ELEMENT_CONTENT_MULT)) {
		/* we want to insert any text nodes */
		cong_command_add_xml_add_optional_text_nodes(cmd, content, node);
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
static gboolean cong_command_add_xml_add_required_content_choice(CongCommand *cmd, xmlElementContentPtr content, xmlNodePtr node) {
	GString *description;
	GList *list = NULL;
	CongNodePtr new_node;
	const xmlChar *names[256];
	gchar *element_name;
	gint i, response, size, length;
	CongDocument *cong_doc;

	g_return_val_if_fail (IS_CONG_COMMAND(cmd), FALSE);

	cong_doc = cong_command_get_document (cmd);

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
	g_string_printf(description, _("The element \"%s\" requires a child to be valid. Please choose one of the following child types."), /* FIXME Bz 123253 */
			node->name);
	
	/*  select a dialog element */
	element_name = string_selection_dialog(_("Required Children Choices"), description->str, list);

	/*  free the string and the list */
	g_string_free(description, FALSE);
	g_list_free(list);

	/*  if user didn't respond, return false */
	g_return_val_if_fail(element_name, FALSE);

	/*  add the element */
	/* FIXME:  we need to supply the correct namespace; need to provide ds element rather than a mere string... Hack to NULL ns for now :-( */
	new_node = cong_node_new_element(NULL, element_name, cong_doc);
	cong_command_add_node_set_parent(cmd, new_node, node);
	
	/*  free the returned string */
	g_free(element_name);

	/*  recur on the new node to add anything it needs */
	return cong_command_add_xml_add_required_sub_elements(cmd, new_node);
}
	
 
/**
 * Adds an optional text node when it
 * conforms to the content model, such that
 * the user may type.
 *
 * @return whether a text node has been added
 */
static gboolean cong_command_add_xml_add_optional_text_nodes(CongCommand *cmd, xmlElementContentPtr content, xmlNodePtr node) {
	xmlNodePtr new_node;
	CongDocument *cong_doc;

	g_return_val_if_fail (IS_CONG_COMMAND(cmd), FALSE);

	cong_doc = cong_command_get_document (cmd);

	if (content->type == XML_ELEMENT_CONTENT_PCDATA) {
		/*  create a text node, add it */
		g_print("xml_add_optional_text_nodes: adding new text node under node %s\n", node->name);
		new_node = cong_node_new_text("", cong_doc);
		cong_command_add_node_set_parent(cmd, new_node, node);
		return TRUE;
	}
	else if (content->type == XML_ELEMENT_CONTENT_OR) {
		/*  if we add a text node in this or, quit recurring through it */
		if (cong_command_add_xml_add_optional_text_nodes(cmd, content->c1, node)) {
			return TRUE;
		}
		/*  otherwise, keep search for a text node */
		else {
			return cong_command_add_xml_add_optional_text_nodes(cmd, content->c2, node);
		}
	}

	return FALSE;
}
/**** end of the add_required stuff ****/

/**
 * xml_valid_get_potential_element_children
 * @ctree:
 * @list:
 * @len:
 * @max:
 * 
 * Reimplementation of xmlValidGetPotentialChildren that *only*
 * returns children of element type, not of PCDATA type.
 *
 * Otherwise, behaves the same as xmlValidGetPotentialChildren.
 * 
 * Returns:
 */
static gint 
xml_valid_get_potential_element_children(xmlElementContent *ctree, 
					 const xmlChar **list,
					 gint *len, 
					 gint max) {
    gint i;

    if ((ctree == NULL) || (list == NULL) || (len == NULL))
        return(-1);

    g_return_val_if_fail (*len >=0, 0);
    g_return_val_if_fail (*len < max, max);

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
 * wrap_xml_valid_get_valid_elements
 * @doc:
 * @parent:
 * @next_sibling:
 * @elements:
 * @max:
 * 
 * Wrapper function for libxml's xmlValidGetValidElements that
 * handles the corner case of when the parent has no children.
 * xmlValidGetValidElements parameter structure cannot handle this case.
 *
 * Otherwise, behaves the same as xmlValidGetValidElements.
 * Being rewritten: now uses CongDocument, so may be able to get at DTD information even if none attached...
 * 
 * Returns:
 */
static gint 
wrap_xml_valid_get_valid_elements (CongDocument *doc, 
				   xmlNode *parent, 
				   xmlNode *next_sibling, 
				   const xmlChar ** elements, 
				   gint max)
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
		element_desc = cong_document_get_dtd_element (doc, 
							      parent);

		/*  if we still failed, give up */
		if (element_desc) {
			/*  get potential children */
			nb_elements = 0;
			nb_elements = xml_valid_get_potential_element_children(element_desc->content,
									       elements, 
									       &nb_elements, 
									       max);
		} else {
			return -1;
		}
	}
	else {
		/*  if we're inserting after the last child */
		if (next_sibling == NULL) {
			nb_elements = xmlValidGetValidElements (parent->last, 
								NULL, 
								elements, 
								max);
		} else {
			nb_elements = xmlValidGetValidElements (next_sibling->prev, 
								next_sibling, 
								elements, 
								max);
		}
	}

	return nb_elements;
}


#if 0
/* do all this using GList or a callback, rather than nasty bounded arrays */
#endif


#define MAX_ELEMENTS 256

/**
 * cong_document_get_valid_new_child_elements
 * @doc:
 * @node: node for which to get valid children
 * @tag_type: either CONG_ELEMENT_TYPE_STRUCTURAL, CONG_ELEMENT_TYPE_SPAN, or CONG_ELEMENT_TYPE_ALL
 * 
 * Get the set of valid children for a node.  If the document
 * contains a DTD, this function will return the intersection
 * of the valid children and elements in the display spec of 
 * type tag_type.
 * If the document does not contain a dtd, the function will fallback
 * on returning the elements in the display spec of type tag_type.
 *
 * Returns: GList of CongDispspecElement
 */
GList*
cong_document_get_valid_new_child_elements (CongDocument *doc,
					    CongNodePtr node, 
					    enum CongElementType tag_type)
{
	const xmlChar *elements[MAX_ELEMENTS];
	gint result;
	CongDispspec *ds = cong_document_get_dispspec(doc);

	switch (cong_node_type(node)) {
	default: return NULL;
	case CONG_NODE_TYPE_ELEMENT:
		result = wrap_xml_valid_get_valid_elements(doc, node, node->last, elements, MAX_ELEMENTS);
		if (result != -1) {
			g_assert (result<=MAX_ELEMENTS);
			return xml_filter_valid_children_with_dispspec(ds, elements, result, tag_type);
		}
		else {
			return xml_get_elements_from_dispspec(ds, tag_type);
		}
	}
}


/**
 * cong_document_get_valid_new_previous_sibling_elements
 * @doc:
 * @node: CongNodePtr node for which to get valid previous siblings
 * @tag_type: either CONG_ELEMENT_TYPE_STRUCTURAL, CONG_ELEMENT_TYPE_SPAN, or CONG_ELEMENT_TYPE_ALL
 * 
 * Get the set of valid previous siblings for a node.  If the document
 * contains a DTD, this function will return the intersection
 * of the valid previous siblings and elements in the display spec of 
 * type tag_type.
 * If the document does not contain a dtd, the function will fallback
 * on returning the elements in the display spec of type tag_type.
 *
 * Returns: GList of CongDispspecElement
 */
GList* 
cong_document_get_valid_new_previous_sibling_elements (CongDocument *doc,
						       CongNodePtr node, 
						       enum CongElementType tag_type)
{
	const xmlChar  *elements[MAX_ELEMENTS];
	gint result;
	CongDispspec *ds = cong_document_get_dispspec(doc); 
	
	result = wrap_xml_valid_get_valid_elements(doc, node->parent, node, elements, MAX_ELEMENTS);
	if (result != -1) {
		g_assert (result<=MAX_ELEMENTS);
		return xml_filter_valid_children_with_dispspec(ds, elements, result, tag_type);
	}
	else {
		return xml_get_elements_from_dispspec(ds, tag_type);
	}
}

/**
 * cong_document_get_valid_new_next_sibling_elements
 * @doc:
 * @node: node for which to get valid next siblings
 * @tag_type: either CONG_ELEMENT_TYPE_STRUCTURAL, CONG_ELEMENT_TYPE_SPAN, or CONG_ELEMENT_TYPE_ALL
 * 
 * Get the set of valid next siblings for a node.  If the document
 * contains a DTD, this function will return the intersection
 * of the valid next siblings and elements in the display spec of 
 * type tag_type.
 * If the document does not contain a dtd, the function will fallback
 * on returning the elements in the display spec of type tag_type.
 *
 * Returns: GList of CongDispspecElement
 */
GList* 
cong_document_get_valid_new_next_sibling_elements (CongDocument* doc, 
						   CongNodePtr node, 
						   enum CongElementType tag_type)
{
	const xmlChar  *elements[MAX_ELEMENTS];
	gint result;
	CongDispspec *ds = cong_document_get_dispspec(doc); 
		
	result = wrap_xml_valid_get_valid_elements(doc, node->parent, node->next, elements, MAX_ELEMENTS);
	if (result != -1) {
		g_assert (result<=MAX_ELEMENTS);
		return xml_filter_valid_children_with_dispspec(ds, elements, result, tag_type);
	}
	else {
		return xml_get_elements_from_dispspec(ds, tag_type);
	}
}

gboolean
should_include_element (CongDispspecElement *element,
			enum CongElementType tag_type )
{
	g_return_val_if_fail (element, FALSE);

	switch (tag_type) {
	default: g_assert_not_reached();
	case CONG_ELEMENT_TYPE_ALL: 
		return TRUE;
			    
	case CONG_ELEMENT_TYPE_STRUCTURAL: 
		if (cong_dispspec_element_is_structural(element)) {
			return TRUE;
		} else {
			/* plugins should also be listed (for now) */
			return cong_dispspec_element_type(element)==CONG_ELEMENT_TYPE_PLUGIN;
		}
		
	case CONG_ELEMENT_TYPE_SPAN: 
		return cong_dispspec_element_is_span(element);
	}
}

/**
 * xml_filter_valid_children_with_dispspec
 * @ds:
 * @elements:
 * @elements_length:
 * @tag_type:
 * 
 * Find the intersection of a list of valid children
 * and a displayspec, returning a list of the displayspec elements
 * that are in the intersection.
 * 
 * Returns:
 */
static GList *xml_filter_valid_children_with_dispspec(CongDispspec* ds, const xmlChar **elements, gint elements_length, enum CongElementType tag_type) {
	CongDispspecElement *element;
	GList *list = NULL;
	gint i;

	for (i = 0; i < elements_length; i++) {
#if DEBUG_VALID_INSERTIONS
		g_message ("got element <%s>", elements[i]);
#endif
		
		/* FIXME:  hack the ns to be NULL for now :-( */
		element = cong_dispspec_lookup_element(ds, NULL, elements[i]);
		if (element) {
			if (should_include_element (element,
						    tag_type )) {				
				list = g_list_prepend(list, element);
			}

		} else {
#if DEBUG_VALID_INSERTIONS
			g_message ("Element not in dispspec <%s>", elements[i]);
#endif
		}
	}

	return list;
}

/**
 * xml_get_elements_from_dispspec
 * @ds:
 * @tag_type:
 * 
 * Get all elements from the display spec that
 * are of tag_type.
 * 
 * Returns:
 */
static GList* xml_get_elements_from_dispspec(CongDispspec* ds, enum CongElementType tag_type) {
	CongDispspecElement *element;
	GList* list = NULL;

	for (element = cong_dispspec_get_first_element(ds); element; element = cong_dispspec_element_next(element)) {
		if (should_include_element (element,
					    tag_type)) {
			list = g_list_prepend(list, element);
		}
	}
	
	return list;
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
