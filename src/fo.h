/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * fo.h
 *
 * Copyright (C) 2002 David Malcolm
 *
 * FIXME:  This file is currently licensed under the GPL, but is intended to eventually become part of a library licensed under the LGPL
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

/*
 * This is some experimental code to try to render XSL:FO to the GnomePrint interface
 * I don't expect this to be anywhere approaching standards-compliant anytime soon.
 * 
 * The idea is that we parse an xmlDoc into a FoParserResult, which stores a fast representation
 * of the content of the FO document.
 *
 * We can then run the "solver", building a FoSolverResult from the FoParserResult.
 * This pours the text into the flows, generating pages etc.
 * (not implemented yet)
 *
 * The FoSolverResult is a representation of all the actual pages generated, ready to be printed.
 * We can send this to the printer.  There are also test functions for sending the parser result
 * to the printer (to visualise the page layouts etc).
 *
 * We hide away GnomePrint via an abstract interface (FoPrintContext)
 *
 * Currently the structs have a new/delete interface; I will probably make them reference-counted
 * at some point.  Also, I may port the system to GObject at that time.
 */

#ifndef __FO_H__
#define __FO_H__

/* Include GnomePrint headers; these will eventually only be required by part of the interface */
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-master.h>

G_BEGIN_DECLS

/* Store internally as millimetres? Is that too simplistic? */
typedef gdouble FoUnit;

typedef struct FoRect FoRect;
typedef struct FoRegion FoRegion;
typedef struct FoSimplePageMaster FoSimplePageMaster;
typedef struct FoParserResult FoParserResult;
typedef struct FoSolverPage FoSolverPage;
typedef struct FoSolverResult FoSolverResult;
typedef struct FoPrintContext FoPrintContext;

enum FoRegionID
{
	FO_REGION_BODY,
	FO_REGION_BEFORE,
	FO_REGION_AFTER,
	FO_REGION_START,
	FO_REGION_END,
	NUM_FO_REGIONS
};

struct FoRect
{
	FoUnit x,y;
	FoUnit w,h;
};

struct FoRegion
{
#if 0
	border;
	padding;
	background;

	/* common margin properties: block; - body only ? */
	clip;
	column_count; /* only for "body" */
	column_gap; /* only for "body" */
	display_align;
	extent; /* for all except body */
	overflow;
	precedence; /* before/after only */
#endif
	
	gchar *name;

#if 0
	reference_orientation;
	writing_mode;
#endif
	
};

struct FoSimplePageMaster
{
	xmlNodePtr node;

	gchar *name;
	FoUnit page_height;
	FoUnit page_width;
	FoUnit margin_top;
	FoUnit margin_bottom;
	FoUnit margin_left;
	FoUnit margin_right;	

	FoRegion *regions[NUM_FO_REGIONS];
};

struct FoParserResult
{
	xmlDocPtr xml_doc;

	/* Stuff set up by parsing: */
	xmlNodePtr node_root;
	xmlNodePtr node_layout_master_set;
	xmlNodePtr node_declarations;

	GList *list_of_simple_page_master; /* of type FoSimplePageMaster */
};

struct FoSolverPage
{
	int dummy;
};

struct FoSolverResult
{
	GList *list_of_solver_page; /* of type FoSolverPage; */
};

/* FoRect methods: */
void fo_rect_set_xywh(FoRect *rect, FoUnit x, FoUnit y, FoUnit w, FoUnit h);
void fo_rect_set_xyxy(FoRect *rect, FoUnit x0, FoUnit y0, FoUnit x1, FoUnit y1);
void fo_rect_test_render(const FoRect *rect, FoPrintContext *fpc, const gchar *label);

/* FoParserResult methods: */
FoParserResult *fo_parser_result_new_from_xmldoc(xmlDocPtr xml_doc);
void fo_parser_result_delete(FoParserResult *result);
void fo_parser_result_test_render(FoParserResult *result, FoPrintContext *fpc);

/* FoSolverResult methods: */
FoSolverResult *fo_solver_result_new_from_parser_result(FoParserResult *parser_result);
void fo_solver_result_delete(FoSolverResult *result);
void fo_solver_result_render(FoSolverResult *result, FoPrintContext *fpc);

/* FoPrintContext methods: */
FoPrintContext *fo_print_context_new_from_gnome_print(GnomePrintContext *gpc);
void fo_print_context_delete(FoPrintContext *fpc);
void fo_print_context_beginpage(FoPrintContext *fpc, const gchar* name);
void fo_print_context_showpage (FoPrintContext *fpc);
void fo_print_context_test_rect(FoPrintContext *fpc, const FoRect *rect, const gchar *label);

G_END_DECLS

#endif
