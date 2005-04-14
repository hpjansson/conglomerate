/*
 * cong-random-document.c
 *
 * Plugin for testing service of various kinds.  Not really intended for end-users.
 *
 * Copyright (C) 2005 David Malcolm
 *
 * Conglomerate is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Conglomerate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#include "global.h"
#include "cong-util.h"
#include "cong-app.h"
#include "cong-dispspec.h"
#include "cong-dispspec-element.h"
#include "cong-dispspec-registry.h"
#include "cong-dtd.h"
#include "cong-eel.h"

#define LOG_RANDOM1(x)       (g_message ((x)))
#define LOG_RANDOM2(x, a)    (g_message ((x), (a)))
#define LOG_RANDOM3(x, a, b) (g_message ((x), (a), (b)))

typedef struct RandomCreationInfo RandomCreationInfo;
struct RandomCreationInfo
{
	CongDispspec *dispspec;
	gboolean ensure_valid;
	int depth;
	GRand *random;
};

static void
populate_element (RandomCreationInfo *rci,
		  xmlDocPtr xml_doc,
		  xmlNodePtr xml_node,
		  int depth);

/**
 * generate_bool_for_opt:
 * @rci:
 *
 * TODO: Write me
 */
static gboolean
generate_bool_for_opt (RandomCreationInfo *rci)
{
	g_assert (rci);

	return g_rand_boolean (rci->random);
}

/**
 * generate_count_for_mult:
 * @rci:
 *
 * TODO: Write me
 */
static gint
generate_count_for_mult (RandomCreationInfo *rci)
{
	g_assert (rci);

	return g_rand_int_range (rci->random,
				 0,
				 6);
}

/**
 * generate_count_for_plus:
 * @rci:
 *
 * TODO: Write me
 */
static gint
generate_count_for_plus (RandomCreationInfo *rci)
{
	g_assert (rci);

	return g_rand_int_range (rci->random,
				 1,
				 7);
}

/**
 * generate_count_for_ocur:
 * @rci:
 * @ocur:
 *
 * TODO: Write me
 */
static gint
generate_count_for_ocur (RandomCreationInfo *rci,
			 xmlElementContentOccur ocur)
{
	g_assert (rci);

	switch (ocur) {
	default: g_assert_not_reached ();
	case XML_ELEMENT_CONTENT_ONCE:
		return 1;

	case XML_ELEMENT_CONTENT_OPT:
		return generate_bool_for_opt (rci)?1:0;

	case XML_ELEMENT_CONTENT_MULT:
		return generate_count_for_mult (rci);

	case XML_ELEMENT_CONTENT_PLUS:
		return generate_count_for_plus (rci);
	}
	
}

#if 0
gchar*
cong_dtd_generate_source_for_content (xmlElementContentPtr content)
{
	g_return_val_if_fail (content, NULL);

	switch (ocur) {
	default: g_assert_not_reached ();
	case XML_ELEMENT_CONTENT_ONCE:
		return 1;

	case XML_ELEMENT_CONTENT_OPT:
		return generate_bool_for_opt (rci)?1:0;

	case XML_ELEMENT_CONTENT_MULT:
		return generate_count_for_mult (rci);

	case XML_ELEMENT_CONTENT_PLUS:
		return generate_count_for_plus (rci);
	}
	
}
#endif

/**
 * random_unichar:
 * @rci:
 *
 * TODO: Write me
 */
static gunichar
random_unichar (RandomCreationInfo *rci)
{
	/* FIXME: probably should have a smarter system... */
	gunichar result;

	/* Have a high chance of spaces, to create word-breaking opportunities: */
	if (0==g_rand_int_range (rci->random, 0, 10)) {
		return ' ';
	}
	
	while (1) {
		result = g_rand_int_range (rci->random, 1, 65535);

		if (g_unichar_isdefined (result)) {
			if (!g_unichar_iscntrl (result)) {

#define UNICODE_VALID(Char)                   \
    ((Char) < 0x110000 &&                     \
     (((Char) & 0xFFFFF800) != 0xD800) &&     \
     ((Char) < 0xFDD0 || (Char) > 0xFDEF) &&  \
     ((Char) & 0xFFFE) != 0xFFFE)

				if (UNICODE_VALID (result)) {
					return result;
				}
			}
		}
	}
}

/**
 * random_text:
 * @rci:
 *
 * TODO: Write me
 */
static gchar*
random_text (RandomCreationInfo *rci)
{
	/* FIXME: should we translate the various strings in this function? */
	switch (g_rand_int_range (rci->random, 0, 3)) {
	default: g_assert_not_reached ();
	case 0:
		return g_strdup ("the quick brown fox jumps over the lazy dog");

	case 1:
		return g_strdup ("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");

	case 2:
		/* Generate an entirely random unicode string: */
		{
			#define MAX_LENGTH (50)
			gint count = g_rand_int_range (rci->random, 1, MAX_LENGTH);
			gint i;
			gunichar tmp_str[MAX_LENGTH+1];
			gchar *utf8_text;

			for (i=0;i<count;i++) {
				tmp_str[i]= random_unichar (rci);
			}
			tmp_str[i]=0;

			utf8_text = g_ucs4_to_utf8 (tmp_str,
						    count,
						    NULL,
						    NULL,
						    NULL);

			if (g_utf8_validate (utf8_text, -1, NULL)) {
				return utf8_text;
			} else {
				g_free (utf8_text);
				return g_strdup ("fubar");
			}

		}
	}
}

static void
populate_element_from_content (RandomCreationInfo *rci,
			       xmlDocPtr xml_doc,
			       xmlNodePtr xml_node,
			       int depth,
			       xmlElementContentPtr content)
{
	gint i;
	guint count;

	g_assert (content);

	count = generate_count_for_ocur (rci, content->ocur);
	
#if 0
	{
		gchar *frag = cong_dtd_generate_source_for_content (content);
		g_message ("got count of %i for %s", count, frag);
		g_free (frag);
	}
#endif

	for (i=0;i<count;i++) {	
		switch (content->type) {
		default: g_assert_not_reached ();
		case XML_ELEMENT_CONTENT_PCDATA:
			{
				gchar *text = random_text (rci);
				xmlNodePtr child_node = xmlNewDocText (xml_doc,
								       (const xmlChar*)text);
				g_free (text);

				xmlAddChild (xml_node, 
					     child_node);
			}
			break;
		case XML_ELEMENT_CONTENT_ELEMENT:
			{
				xmlNodePtr child_node = xmlNewDocNode (xml_doc,
								       NULL,
								       content->name,
								       (const xmlChar*)""); /* FIXME: namespace? */
				xmlAddChild (xml_node, 
					     child_node);
				populate_element (rci,
						  xml_doc,
						  child_node,
						  depth+1);
			}
			break;
		case XML_ELEMENT_CONTENT_SEQ:
			/* Do both c1 and c2 in sequence: */
			populate_element_from_content (rci,
						       xml_doc,
						       xml_node,
						       depth,
						       content->c1);
			populate_element_from_content (rci,
						       xml_doc,
						       xml_node,
						       depth,
						       content->c2);
			break;
		case XML_ELEMENT_CONTENT_OR:
			/* Do one of c1 or c2: */
			if (generate_bool_for_opt (rci)) {
				populate_element_from_content (rci,
							       xml_doc,
							       xml_node,
							       depth,
							       content->c1);
			} else {
				populate_element_from_content (rci,
							       xml_doc,
							       xml_node,
							       depth,
							       content->c2);
			}
			break;
		}
	}
}

static void
populate_element_from_dtd (RandomCreationInfo *rci,
			   xmlDocPtr xml_doc,
			   xmlNodePtr xml_node,
			   int depth,
			   xmlElementPtr element)
{
	g_assert (rci);
	g_assert (xml_doc);
	g_assert (xml_node);
	g_assert (element);

	switch (element->etype) {
	default: g_assert_not_reached ();
	case XML_ELEMENT_TYPE_UNDEFINED:
	case XML_ELEMENT_TYPE_EMPTY:
		/* do nothing */
		break;

	case XML_ELEMENT_TYPE_ANY:
		break;

	case XML_ELEMENT_TYPE_MIXED:
		break;

	case XML_ELEMENT_TYPE_ELEMENT:
		break;
	}

	if (element->content) {
		populate_element_from_content (rci,
					       xml_doc,
					       xml_node,
					       depth+1,
					       element->content);
	}



	/* FIXME: set up attributes! */
}

static void
populate_element (RandomCreationInfo *rci,
		  xmlDocPtr xml_doc,
		  xmlNodePtr xml_node,
		  int depth)
{
	g_assert (rci);
	g_assert (xml_doc);
	g_assert (xml_node);

	LOG_RANDOM3 ("populate_element (below <%s>, %i)", xml_node->name, depth);

	/* Safety cutoffs */
	{ 
		/* Stop if we've reached the maximum depth */
		if (depth>=rci->depth) {
			return;
		}
	}

	if (xml_doc->extSubset) {
		xmlElementPtr element = cong_dtd_element_get_element_for_node (xml_doc->extSubset,
									       xml_node);
		if (element) {
			populate_element_from_dtd (rci,
						   xml_doc,
						   xml_node,
						   depth,
						   element);
			return;
		}
	} 

	/* No DTD information was available for this node; randomly add content: */
	{
		gint child_count;
		gint i;
		
		/* Slow algorithm */
		guint num_elements = cong_dispspec_get_num_elements (rci->dispspec);

		child_count = g_rand_int_range (rci->random, 
						0,
						(rci->depth-depth));

		for (i=0;i<child_count;i++) {
			CongDispspecElement* ds_element;
			xmlNodePtr child_node;

			ds_element = cong_dispspec_get_element (rci->dispspec,
								g_rand_int_range (rci->random, 
										  0,
										  num_elements));
			g_assert (ds_element);

			child_node = xmlNewDocNode (xml_doc,
						    NULL,
						    (const xmlChar*)cong_dispspec_element_get_local_name (ds_element),
						    (const xmlChar*)"");
			if (cong_dispspec_element_get_ns_uri (ds_element)) {
				xmlNsPtr xml_ns = xmlNewNs (child_node, 
							    (const xmlChar*)cong_dispspec_element_get_ns_uri (ds_element),
							    NULL);
				xmlSetNs (child_node, 
					  xml_ns);	
			}

			xmlAddChild (xml_node,
				     child_node);
			
			populate_element (rci,
					  xml_doc,
					  child_node,
					  depth+1);
		}
	}
}

xmlDocPtr
cong_make_random_doc (CongDispspec *dispspec, 
		      gboolean ensure_valid,
		      int depth)
{
	RandomCreationInfo rci;
	xmlDocPtr xml_doc;
	xmlNodePtr root_node;
	const CongExternalDocumentModel* dtd_model;
	CongDispspecElement *ds_element_root;
	const gchar *root_element;

	g_return_val_if_fail (dispspec, NULL);

	rci.dispspec = dispspec;
	rci.ensure_valid = ensure_valid;
	rci.depth = depth;
	rci.random = g_rand_new ();

	ds_element_root = cong_dispspec_get_first_element (dispspec); /* FIXME */
	g_assert (ds_element_root);

	root_element = cong_dispspec_element_get_local_name (ds_element_root);
	g_assert (root_element);

	dtd_model = cong_dispspec_get_external_document_model (dispspec,
							       CONG_DOCUMENT_MODE_TYPE_DTD);

	xml_doc = xmlNewDoc ((const xmlChar*)"1.0");

	root_node = xmlNewDocNode (xml_doc,
				   NULL, /* xmlNsPtr ns, */
				   (const xmlChar*)root_element,
				   NULL);
	if (cong_dispspec_element_get_ns_uri (ds_element_root)) {
		xmlNsPtr xml_ns = xmlNewNs (root_node, 
					    (const xmlChar*)cong_dispspec_element_get_ns_uri (ds_element_root),
					    NULL);
		xmlSetNs (root_node, 
			  xml_ns);	
	}
	xmlDocSetRootElement (xml_doc,
			      root_node);

	if (dtd_model) {
		cong_util_add_external_dtd (xml_doc, 
					    root_element,
					    cong_external_document_model_get_public_id (dtd_model),
					    cong_external_document_model_get_system_id (dtd_model));
	}
	
	populate_element (&rci,
			  xml_doc,
			  root_node,
			  0);
	return xml_doc;
}

