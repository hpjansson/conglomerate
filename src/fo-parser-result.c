/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * fo-parser-result.c
 *
 * Copyright (C) 2002 David Malcolm
 *
 * FIXME:  This file is currently licensed under the GPL, but I intend it to eventually become part of a library licensed under the LGPL
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#include "global.h"
#include "fo.h"

#if 0
/* FoParserResult internals: */
static FoUnit get_distance_property(xmlNodePtr node, const xmlChar *property)
{
	xmlChar *value = xmlGetProp(node, property);
	float number = 0.0f;
	int len;
	xmlChar *units;

	if (NULL==value) {
		g_message("missing property \"%s\"", property);
		return 0;
	}

	/* 
	   How to parse?  
	   
	   Units might be "in", "pt", "mm", "pc", "px"... other units?
	   
	   My initial implementation just used sscanf and didn't work.
	*/
	
	/* FIXME: bad char type */
	len = strlen(value);

	if (len>2) {
		units = value+len-2;

		if (0==xmlStrcmp(units,"mm")) {
			if (1==sscanf(value,"%fmm", &number)) {
				/* Millimeters */
				/* No conversion required */
				g_message("fubar");
			}
		} else if (0==xmlStrcmp(units,"in")) {
			if (1==sscanf(value, "%fin", &number)) {
				/* Inches */
				number *=25.4f; /* convert from inches to millimetres */
				/* FIXME: is this accurate enough? */
			}
		} else if (0==xmlStrcmp(units,"pt")) {
			if (1==sscanf(value, "%fpt", &number)) {
				/* Points */
				number *=1.0f; /* convert from points to millimetres */
				/* FIXME */
			}
		} else if (0==xmlStrcmp(units,"pc")) {
			if (1==sscanf(value, "%fpc", &number)) {
				/* Picas; a pica is equal to 12 points */
				number *=1.0f; /* convert from picas to millimetres */
				/* FIXME */
			}
		} else if (0==xmlStrcmp(units,"px")) {
			if (1==sscanf(value, "%fpx", &number)) {
				/* Pixels */
				number *=1.0f; /* convert from pixels to millimetres */
				/* FIXME */
			}
		} else {
			g_message("Failed to parse distance \"%s\"\n", value);
		}
	}
	

        g_message("%s =\"%s\" (interpreted as %fmm)\n", property, value, number);

	xmlFree(value);
	return number;
}

static void process_region(FoParserResult *result, FoSimplePageMaster *spm, xmlNodePtr node, enum FoRegionID region_id)
{
	FoRegion *region;

	region = spm->regions[region_id] = g_new0(FoRegion,1);
	FO_PARSER_OBJECT(region)->node = node;

	region->name = xmlGetProp(node, "region-name");

}

static FoSimplePageMaster *simple_page_master_get_master_for_next_page(FoPageSequenceGenerator *psg, 
								       int page_number, 
								       gboolean is_first_of_sequence,
								       gboolean is_last_of_sequence,
								       gboolean is_blank)
{
	/* SImple page masters return themselves when asked to generate a page */
	return FO_SIMPLE_PAGE_MASTER(psg);
}

struct FoPageSequenceGeneratorClass simple_page_master_class = {

	{
		{"simple_page_master_class"}, /* FoParserClass base; */
	},
	simple_page_master_get_master_for_next_page
};

static void process_simple_page_master(FoParserResult *result, xmlNodePtr node)
{
	xmlNodePtr iter;
	FoSimplePageMaster *simple_page_master;

	g_message("process_simple_page_master\n");

	simple_page_master = g_new0(FoSimplePageMaster,1);
	FO_OBJECT(simple_page_master)->klass = FO_CLASS(&simple_page_master_class);
	FO_PARSER_OBJECT(simple_page_master)->node = node;

	result->list_of_simple_page_master = g_list_append(result->list_of_simple_page_master, simple_page_master);

	simple_page_master->base.name = xmlGetProp(node, "master-name");
	simple_page_master->page_height = get_distance_property(node, "page-height");
	simple_page_master->page_width = get_distance_property(node, "page-width");
	simple_page_master->margin_top = get_distance_property(node, "margin-top");
	simple_page_master->margin_bottom = get_distance_property(node, "margin-bottom");
	simple_page_master->margin_left = get_distance_property(node, "margin-left");
	simple_page_master->margin_right = get_distance_property(node, "margin-right");

#if 0
	{
		xmlAttrPtr attr;

		for (attr = node->properties; attr; attr = attr->next) {
			g_message("%s = \"%s\"\n", attr->name, attr->children->content);
		}
	}
#endif

	for (iter = node->children; iter; iter = iter->next) {
		g_message("got <%s>\n", iter->name);

		if (0==xmlStrcmp(iter->name,"region-body") ) {
			process_region(result, simple_page_master, iter, FO_REGION_BODY);
		}
		if (0==xmlStrcmp(iter->name,"region-before") ) {
			process_region(result, simple_page_master, iter, FO_REGION_BEFORE);
		}
		if (0==xmlStrcmp(iter->name,"region-after") ) {
			process_region(result, simple_page_master, iter, FO_REGION_AFTER);
		}
		if (0==xmlStrcmp(iter->name,"region-start") ) {
			process_region(result, simple_page_master, iter, FO_REGION_START);
		}
		if (0==xmlStrcmp(iter->name,"region-end") ) {
			process_region(result, simple_page_master, iter, FO_REGION_END);
		}
	}

}

FoSubsequenceSpecifier *process_single_page_master_reference(FoParserResult *result, xmlNodePtr node)
{
	xmlChar* master_name;
	FoSS_SinglePageMasterReference *foss = g_new0(FoSS_SinglePageMasterReference, 1);
	FO_PARSER_OBJECT(foss)->node = node;

	master_name = xmlGetProp(node, "master-name");

	foss->spm = fo_parser_result_lookup_simple_page_master(result, master_name);

	xmlFree(master_name);

	return FO_SUBSEQUENCE_SPECIFIER(foss);
}

FoSubsequenceSpecifier *process_repeatable_page_master_reference(FoParserResult *result, xmlNodePtr node)
{
	xmlChar* master_name;
	FoSS_RepeatablePageMasterReference *foss = g_new0(FoSS_RepeatablePageMasterReference, 1);
	FO_PARSER_OBJECT(foss)->node = node;

	master_name = xmlGetProp(node, "master-name");

	foss->spm = fo_parser_result_lookup_simple_page_master(result, master_name);

	xmlFree(master_name);

	return FO_SUBSEQUENCE_SPECIFIER(foss);
}

FoConditionalPageMasterReference *process_conditional_page_master_reference(FoParserResult *result, xmlNodePtr node)
{
	xmlChar* master_reference;
	FoConditionalPageMasterReference *alternative = g_new0(FoConditionalPageMasterReference,1);
	FO_PARSER_OBJECT(alternative)->node = node;

	master_reference = xmlGetProp(node, "master-reference");

	alternative->spm = fo_parser_result_lookup_simple_page_master(result, master_reference);

	xmlFree(master_reference);
	
	return alternative;
}

FoSubsequenceSpecifier *process_repeatable_page_master_alternatives(FoParserResult *result, xmlNodePtr node)
{
	xmlNodePtr iter;
	FoSS_RepeatablePageMasterAlternatives *foss = g_new0(FoSS_RepeatablePageMasterAlternatives, 1);

	/* Iterate through child tree, building up a list of alternatives: */
	for (iter = node->children; iter; iter = iter->next) {
		if (0==xmlStrcmp(iter->name, "conditional-page-master-reference")) {
			FoConditionalPageMasterReference *alternative = process_conditional_page_master_reference(result, iter); 

			g_message("got alternative: <%s>\n", iter->name);

			foss->list_of_alternatives = g_list_append(foss->list_of_alternatives, alternative);
		}
	}

	return FO_SUBSEQUENCE_SPECIFIER(foss);
}

static FoSimplePageMaster *page_sequence_master_get_master_for_next_page(FoPageSequenceGenerator *psg, 
									 int page_number, 
									 int index_within_page_sequence,
									 gboolean is_first_of_sequence,
									 gboolean is_last_of_sequence,
									 gboolean is_blank)
{
	FoPageSequenceMaster *psm = FO_PAGE_SEQUENCE_MASTER(psg);
	
	/* Use the indexed subsequence specifier: */
	FoSubsequenceSpecifier *foss = g_list_nth_data(psm->list_of_subsequence_specifier, index_within_page_sequence);

	if (foss) {
/*  		#error see pages 48-49 */
	}
	g_assert(0);
	/* unwritten */

	return NULL;
}

struct FoPageSequenceGeneratorClass page_sequence_master_class = {

	{
		{"page_sequence_master_class"}, /* FoParserClass base; */
	},
	page_sequence_master_get_master_for_next_page
};

static FoPageSequenceMaster *process_page_sequence_master(FoParserResult *result, xmlNodePtr node)
{
	FoPageSequenceMaster *psm;
	xmlNodePtr iter;

	psm = g_new0(FoPageSequenceMaster,1);
	FO_OBJECT(psm)->klass = FO_CLASS(&page_sequence_master_class);
	FO_PARSER_OBJECT(psm)->node = node;

	psm->base.name = xmlGetProp(node, "master-name");

	g_message("process_page_sequence_master, name=\"%s\"\n", psm->base.name);

	/* Iterate through child tree, building up a list of subsequence specifiers: */
	for (iter = node->children; iter; iter = iter->next) {
		FoSubsequenceSpecifier *foss = NULL;

		g_message("got subsequence specifier: <%s>\n", iter->name);

		if (0==xmlStrcmp(iter->name, "single-page-master-reference")) {
			foss = process_single_page_master_reference(result, iter);
		} else if (0==xmlStrcmp(iter->name, "repeatable-page-master-reference")) {
			foss = process_repeatable_page_master_reference(result, iter);
		} else if (0==xmlStrcmp(iter->name, "repeatable-page-master-alternatives")) {
			foss = process_repeatable_page_master_alternatives(result, iter);
		}

		if (foss) {
			psm->list_of_subsequence_specifier = g_list_append(psm->list_of_subsequence_specifier, foss);
		}
	}

	return psm;
}

static void process_layout_master_set(FoParserResult *result, xmlNodePtr node)
{
	xmlNodePtr iter;

	g_message("process_layout_master_set\n");

	result->node_layout_master_set = iter;

	for (iter = node->children; iter; iter = iter->next) {

		g_message("got <%s>\n", iter->name);
		if (0==xmlStrcmp(iter->name,"simple-page-master") ) {
			process_simple_page_master(result, iter);
		}
		if (0==xmlStrcmp(iter->name,"page-sequence-master") ) {
			FoPageSequenceMaster *psm = process_page_sequence_master(result, iter);

			result->list_of_page_sequence_master = g_list_append(result->list_of_page_sequence_master, psm);
		}
	}

}

static void process_fo_recursive(FoParserResult *result, xmlNodePtr node)
{
	xmlNodePtr iter;

#if 0
	g_message("process_fo_recursive <%s>\n", node->name);

	if (node->type==XML_TEXT_NODE) {
		g_message("(text =\"%s\")\n", node->content);
	}

	if ( 0==xmlStrcmp(node->name,"block") ) {
		xmlAttrPtr attr;

		for (attr = node->properties; attr; attr = attr->next) {
			g_message("%s = \"%s\"\n", attr->name, attr->children->content);
		}
	}
#endif

	for (iter = node->children; iter; iter = iter->next) {
		process_fo_recursive(result, iter);
	}
}

static FoFlow *process_flow(FoParserResult *result, xmlNodePtr node)
{
	FoFlow *flow;
	xmlNodePtr iter;

	g_message("process flow\n");

	flow = g_new0(FoFlow,1);
	FO_PARSER_OBJECT(flow)->node = node;

	for (iter = node->children; iter; iter = iter->next) {
		process_fo_recursive(result, iter);
	}

	return flow;
}

static FoPageSequence *process_page_sequence(FoParserResult *result, xmlNodePtr node)
{
	FoPageSequence *page_sequence;
	xmlNodePtr iter;

	g_message("page_sequence\n");

	page_sequence = g_new0(FoPageSequence,1);
	FO_PARSER_OBJECT(page_sequence)->node = node;

	/* Generate pages either directly from a simple-page-master, or indirectly using page-sequence-master: */
	{
		xmlChar *master_name = xmlGetProp(node, "master-name");

		if (master_name) {
			g_message("Got master-name \"%s\"\n", master_name);

			page_sequence->psg = FO_PAGE_SEQUENCE_GENERATOR( fo_parser_result_lookup_simple_page_master(result, master_name) );
			
			g_free(master_name);

		} else {

			xmlChar *master_reference = xmlGetProp(node, "master-reference");

			if (master_reference) {
				g_message("Got master-reference \"%s\"\n", master_reference);

				page_sequence->psg = FO_PAGE_SEQUENCE_GENERATOR( fo_parser_result_lookup_page_sequence_master(result, master_reference) );

				g_free(master_reference);

			} else {
				g_message("Cannot determine how to generate page sequences\n");
			}

		}
	}

	g_assert(page_sequence->psg); /* for now */

	for (iter = node->children; iter; iter = iter->next) {
		if (0==xmlStrcmp(iter->name,"title") ) {
			g_message("title\n");
		}
		if (0==xmlStrcmp(iter->name,"static-content") ) {
			g_message("static_content\n");
		}
		if (0==xmlStrcmp(iter->name,"flow") ) {
			FoFlow *flow = process_flow(result, iter);
			page_sequence->flow = flow;
		}
	}

	return page_sequence;

}

static void locate_elements(FoParserResult *result)
{
	xmlNodePtr iter;

	g_message("locate_elements\n");

	result->node_root = result->xml_doc->children;

	for (iter = result->node_root->children; iter; iter = iter->next) {

		g_message("got <%s>\n", iter->name);
		if (0==xmlStrcmp(iter->name,"layout-master-set") ) {
			process_layout_master_set(result, iter);
		}
		if (0==xmlStrcmp(iter->name,"declarations") ) {
			result->node_declarations = iter;
		}
		if (0==xmlStrcmp(iter->name,"page-sequence") ) {
			FoPageSequence *page_sequence = process_page_sequence(result, iter);
			g_assert(page_sequence);
			g_assert(page_sequence->psg); /* for now */

			result->list_of_page_sequence = g_list_append(result->list_of_page_sequence, page_sequence);
		}
	}
	
}

#if 0
FoSimplePageMaster *fo_page_sequence_generator_get_master_for_next_page(FoPageSequenceGenerator *psg, 
									int page_number, 
									gboolean is_first_of_sequence,
									gboolean is_last_of_sequence,
									gboolean is_blank)
{
	struct FoPageSequenceGeneratorClass *klass;

	g_return_val_if_fail(psg, NULL);

	klass = (struct FoPageSequenceGeneratorClass*)(FO_PARSER_OBJECT(psg)->klass);
	g_assert(klass);
	g_assert(klass->get_master_for_next_page);

	return klass->get_master_for_next_page(psg, 
					       page_number, 
					       is_first_of_sequence,
					       is_last_of_sequence,
					       is_blank);
}
#endif

/* FoParserResult methods: */
FoParserResult *fo_parser_result_new_from_xmldoc(xmlDocPtr xml_doc)
{
	FoParserResult *result;

	g_return_val_if_fail(xml_doc, NULL);

	result = g_new0(FoParserResult,1);

	result->xml_doc = xml_doc;

	locate_elements(result);

	return result;
}

void fo_parser_result_delete(FoParserResult *result)
{
	/* FIXME: cleanup */

	g_free(result);
}

/* 
   Handy test method to render the result of parsing.
   
   Currently it outputs schematic versions of the various page descriptions.
*/
void fo_parser_result_test_render(FoParserResult *result, FoPrintContext *fpc)
{
	FoRect rect;
	GList *iter;
#if 1
	/* Render pages for each simple-page-master */
	for (iter = result->list_of_simple_page_master; iter; iter = iter->next) {
		FoSimplePageMaster *spm = iter->data;

		fo_print_context_beginpage(fpc, spm->base.name);

		/* Page width/height: */
		fo_rect_set_xywh(&rect, 0.0, 0.0, spm->page_width, spm->page_height);
		fo_rect_test_render(&rect, fpc, spm->base.name);

		/* Margin/the "content rectangle": */
		fo_rect_set_xyxy(&rect, spm->margin_bottom, spm->margin_left, spm->page_width-spm->margin_right, spm->page_height-spm->margin_top);
		fo_rect_test_render(&rect, fpc, "margin/content rectangle");
		

		/* Display regions: */
		{
			int i;
			for (i=0;i<NUM_FO_REGIONS; i++) {
				FoRegion *region = spm->regions[i];

				if (region) {
					fo_rect_set_xywh(&rect, 100.0, 250.0 + (i*50.0), 50.0, 50.0);
					fo_rect_test_render(&rect, fpc, region->name);
				}
			}
		}
	
		fo_print_context_showpage (fpc);		
	}
#else
	gnome_print_beginpage (gpc, "1");

	fo_rect_set(&rect, 100.0, 100.0, 500.0, 400.0);
	fo_rect_test_render(&rect, gpc, "Test area 1");

	fo_rect_set(&rect, 500.0, 100.0, 200.0, 200.0);
	fo_rect_test_render(&rect, gpc, "Test area 2");

	fo_rect_set(&rect, 100.0, 500.0, 100.0, 300.0);
	fo_rect_test_render(&rect, gpc, "Test area 3");
	
	gnome_print_showpage (gpc);
#endif
}

FoSimplePageMaster *fo_parser_result_lookup_simple_page_master(FoParserResult *result, xmlChar *name)
{
	GList *iter;

	g_return_val_if_fail(result, NULL);
	g_return_val_if_fail(name, NULL);

	for (iter = result->list_of_simple_page_master; iter; iter=iter->next) {
		FoSimplePageMaster *spm = iter->data;

		if (0==xmlStrcmp(spm->base.name, name)) {
			return spm;
		}
	}

	g_message("Failed to find <fo:simple-page-master> called \"%s\"\n", name);

	return NULL;
}

FoPageSequenceMaster *fo_parser_result_lookup_page_sequence_master(FoParserResult *result, xmlChar *name)
{
	GList *iter;

	g_return_val_if_fail(result, NULL);
	g_return_val_if_fail(name, NULL);

	for (iter = result->list_of_page_sequence_master; iter; iter=iter->next) {
		FoPageSequenceMaster *psm = iter->data;

		if (0==xmlStrcmp(psm->base.name, name)) {
			return psm;
		}
	}

	g_message("Failed to find <fo:page-sequence-master> called \"%s\"\n", name);

	return NULL;
}
#endif
