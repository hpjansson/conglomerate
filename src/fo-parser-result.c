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

/* FoParserResult internals: */
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

static void process_flow(FoParserResult *result, xmlNodePtr node)
{
	xmlNodePtr iter;

	g_message("process flow\n");

	for (iter = node->children; iter; iter = iter->next) {
		process_fo_recursive(result, iter);
	}
	
}

static void process_page_sequence(FoParserResult *result, xmlNodePtr node)
{
	xmlNodePtr iter;

	g_message("page_sequence\n");

#if 0
	for (iter = node->children; iter; iter = iter->next) {
		if (0==xmlStrcmp(iter->name,"title") ) {
			g_message("title\n");
		}
		if (0==xmlStrcmp(iter->name,"static-content") ) {
			g_message("static_content\n");
		}
		if (0==xmlStrcmp(iter->name,"flow") ) {
			process_flow(result, iter);
		}
	}
#endif

}

static FoUnit get_distance_property(xmlNodePtr node, const xmlChar *property)
{
	xmlChar *value = xmlGetProp(node, property);
	float number = 0.0f;
	int len;
	xmlChar *units;

	g_assert(value);

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

	region->name = xmlGetProp(node, "region-name");

}

static void process_simple_page_master(FoParserResult *result, xmlNodePtr node)
{
	xmlNodePtr iter;
	FoSimplePageMaster *simple_page_master;

	g_message("process_simple_page_master\n");

	simple_page_master = g_new0(FoSimplePageMaster,1);

	result->list_of_simple_page_master = g_list_append(result->list_of_simple_page_master, simple_page_master);

	simple_page_master->name = xmlGetProp(node, "master-name");
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

static void process_page_sequence_master(FoParserResult *result, xmlNodePtr node)
{
	xmlNodePtr iter;

	g_message("process_page_sequence_master\n");

	for (iter = node->children; iter; iter = iter->next) {

		g_message("got <%s>\n", iter->name);
	}

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
			process_page_sequence_master(result, iter);
		}
	}

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
			process_page_sequence(result, iter);
		}
	}
	
}

static void generate_areas(FoParserResult *result)
{
}

static void calculate_positions_for_areas(FoParserResult *result)
{
}

/* FoParserResult methods: */
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

		fo_print_context_beginpage(fpc, spm->name);

		/* Page width/height: */
		fo_rect_set_xywh(&rect, 0.0, 0.0, spm->page_width, spm->page_height);
		fo_rect_test_render(&rect, fpc, spm->name);

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

FoParserResult *fo_parser_result_new_from_xmldoc(xmlDocPtr xml_doc)
{
	FoParserResult *result;

	g_return_val_if_fail(xml_doc, NULL);

	result = g_new0(FoParserResult,1);

	result->xml_doc = xml_doc;

	locate_elements(result);
	generate_areas(result);
	calculate_positions_for_areas(result);

	return result;
}

void fo_parser_result_delete(FoParserResult *result)
{
	/* FIXME: cleanup */

	g_free(result);
}
