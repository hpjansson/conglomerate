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
#if ENABLE_PRINTING
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>

G_BEGIN_DECLS

/* Store internally as millimetres? Is that too simplistic? */
typedef gdouble FoUnit;

typedef struct FoRect FoRect;

typedef struct FoObject FoObject;
typedef struct FoClass FoClass;
#define FO_OBJECT(x) ((FoObject*)(x))
#define FO_CLASS(x) ((FoClass*)(x))

typedef struct FoParserObject FoParserObject;
typedef struct FoParserClass FoParserClass;
#define FO_PARSER_OBJECT(x) ((FoParserObject*)(x))
#define FO_PARSER_CLASS(x) ((FoParserClass*)(x))

typedef struct FoRegion FoRegion;

typedef struct FoPageSequenceGenerator FoPageSequenceGenerator;
#define FO_PAGE_SEQUENCE_GENERATOR(x) ((FoPageSequenceGenerator*)(x))

typedef struct FoSimplePageMaster FoSimplePageMaster;
#define FO_SIMPLE_PAGE_MASTER(x) ((FoSimplePageMaster*)(x))

typedef struct FoSubsequenceSpecifier FoSubsequenceSpecifier;
#define FO_SUBSEQUENCE_SPECIFIER(x) ((FoSubsequenceSpecifier*)(x))

typedef struct FoSS_SinglePageMasterReference FoSS_SinglePageMasterReference;
typedef struct FoSS_RepeatablePageMasterReference FoSS_RepeatablePageMasterReference;
typedef struct FoSS_RepeatablePageMasterAlternatives FoSS_RepeatablePageMasterAlternatives;
typedef struct FoConditionalPageMasterReference FoConditionalPageMasterReference; 

typedef struct FoPageSequenceMaster FoPageSequenceMaster;
#define FO_PAGE_SEQUENCE_MASTER(x) ((FoPageSequenceMaster*)(x))

typedef struct FoFlow FoFlow;

typedef struct FoPageSequence FoPageSequence;

typedef struct FoParserResult FoParserResult;

typedef struct FoSolverArea FoSolverArea;
typedef struct FoSolverAreaClass FoSolverAreaClass;
#define FO_SOLVER_AREA(x) ((FoSolverArea*)(x))
#define FO_SOLVER_AREA_CLASS(x) ((FoSolverAreaClass*)(x))

typedef struct FoSolverBlockArea FoSolverBlockArea;
#define FO_SOLVER_BLOCK_AREA(x) ((FoSolverBlockArea*)(x))

typedef struct FoSolverLineArea FoSolverLineArea;
#define FO_SOLVER_LINE_AREA(x) ((FoSolverLineArea*)(x))

typedef struct FoSolverInlineArea FoSolverInlineArea;
#define FO_SOLVER_INLINE_AREA(x) ((FoSolverInlineArea*)(x))

typedef struct FoSolverTestRect FoSolverTestRect;
#define FO_SOLVER_TEST_RECT(x) ((FoSolverTestRect*)(x))

typedef struct FoSolverRect FoSolverRect;
#define FO_SOLVER_RECT(x) ((FoSolverRect*)(x))

typedef struct FoSolverText FoSolverText;
#define FO_SOLVER_TEXT(x) ((FoSolverText*)(x))

typedef struct FoSolverPage FoSolverPage;
#define FO_SOLVER_PAGE(x) ((FoSolverPage*)(x))

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

struct FoObject
{
	const FoClass *klass;
};

struct FoClass
{
	const gchar *name;
};

struct FoParserObject
{
	FoObject base;
	xmlNodePtr node;
};

struct FoParserClass
{
	FoClass base;
};

struct FoRegion
{
	FoParserObject base;
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

struct FoPageSequenceGenerator
{
	FoParserObject base;

	gchar *name;
};

struct FoPageSequenceGeneratorClass
{
	FoParserClass base;
	FoSimplePageMaster *(*get_master_for_next_page)(FoPageSequenceGenerator *psg, 
							int page_number, 
							int index_within_page_sequence,
							gboolean is_first_of_sequence,
							gboolean is_last_of_sequence,
							gboolean is_blank);

};

struct FoSimplePageMaster
{
	FoPageSequenceGenerator base;

	FoUnit page_height;
	FoUnit page_width;
	FoUnit margin_top;
	FoUnit margin_bottom;
	FoUnit margin_left;
	FoUnit margin_right;	

	FoRegion *regions[NUM_FO_REGIONS];
};

struct FoSubsequenceSpecifier
{
	FoParserObject base;
};

struct FoSS_SinglePageMasterReference
{
	FoSubsequenceSpecifier foss;

	FoSimplePageMaster *spm;
};

struct FoSS_RepeatablePageMasterReference
{
	FoSubsequenceSpecifier foss;

	FoSimplePageMaster *spm;
};

struct FoSS_RepeatablePageMasterAlternatives
{
	FoSubsequenceSpecifier foss;

	GList *list_of_alternatives; /* of type FoConditionalPageMasterReference */
};

struct FoConditionalPageMasterReference
{
	FoParserObject base;

	FoSimplePageMaster *spm;
};

struct FoPageSequenceMaster
{
	FoPageSequenceGenerator base;

	GList *list_of_subsequence_specifier; /* of type FoSubsequenceSpecifier */
};

struct FoFlow
{
	FoParserObject base;
};

struct FoPageSequence
{
	FoParserObject base;

	FoPageSequenceGenerator *psg; /* will be either a FoSimplePageMaster or a FoPageSequenceMaster */
#if 0
	FoTitle *title;
	FoStaticContent *static_content; /* should be a list */
#endif
	FoFlow *flow;
};

struct FoParserResult
{
	xmlDocPtr xml_doc;

	/* Stuff set up by parsing: */
	xmlNodePtr node_root;
	xmlNodePtr node_layout_master_set;
	xmlNodePtr node_declarations;

	GList *list_of_simple_page_master; /* of type FoSimplePageMaster */
	GList *list_of_page_sequence_master; /* of type FoPageSequenceMaster */
	GList *list_of_page_sequence; /* of type FoPageSequence */
};

struct FoSolverArea
{
	FoObject base;

	FoParserObject *parser_object; /* can be NULL e.g. for FoLineArea */

	/* Position relative to parent's origin: */
	FoUnit relative_x;
	FoUnit relative_y;

	/* Size: */
	FoUnit width;
	FoUnit height;

	/* Tree hierarchy: */
	FoSolverArea *parent;
	GList *children; /* of type FoSolverArea */	
};

struct FoSolverAreaClass
{
	FoClass base;

	void (*render)(FoSolverArea *area,
		       FoPrintContext *fpc);
		       
};

struct FoSolverBlockArea
{
	FoSolverArea area;
};

struct FoSolverLineArea
{
	FoSolverArea area;
};

struct FoSolverInlineArea
{
	FoSolverArea area;
};

struct FoSolverTestRect
{
	FoSolverArea area;
	FoRect rect;
	gchar *text;
};

struct FoSolverRect
{
	FoSolverArea area;
	FoRect rect;

	/* offsets its children by its origin */
};

struct FoSolverText
{
	FoSolverInlineArea area;

	PangoGlyphString *glyph_string;
	PangoFont *pango_font;
	PangoRectangle ink_rect;
	PangoRectangle logical_rect;
	gchar *text;
};

struct FoSolverPage
{
	FoSolverArea area;

	FoSimplePageMaster *spm;

};

struct FoSolverResult
{
	FoParserResult *parser_result;

	GList *list_of_solver_page; /* of type FoSolverPage; */
	int num_pages;

	/* Internal state whilst generating the result: */
	PangoContext *pango_context;
	PangoFontMap *pango_font_map;
 
	FoPageSequence *current_ps;

#if 0
	FoSolverPage *current_page;
	FoUnit insertion_y;
#endif
};

/* FoRect methods: */
void fo_rect_set_xywh(FoRect *rect, FoUnit x, FoUnit y, FoUnit w, FoUnit h);
void fo_rect_set_xyxy(FoRect *rect, FoUnit x0, FoUnit y0, FoUnit x1, FoUnit y1);
void fo_rect_test_render(const FoRect *rect, FoPrintContext *fpc, const gchar *label);

/**
   Method to evaluate which SimplePageMaster to use.

   SimplePageMasters return themselves.
   PageSequenceMasters use their own logic...
 */
FoSimplePageMaster *fo_page_sequence_generator_get_master_for_next_page(FoPageSequenceGenerator *psg, 
									int page_number, 
									int index_within_page_sequence,
									gboolean is_first_of_sequence,
									gboolean is_last_of_sequence,
									gboolean is_blank);

/* FoParserResult methods: */
FoParserResult *fo_parser_result_new_from_xmldoc(xmlDocPtr xml_doc);
void fo_parser_result_delete(FoParserResult *result);
void fo_parser_result_test_render(FoParserResult *result, FoPrintContext *fpc);
FoSimplePageMaster *fo_parser_result_lookup_simple_page_master(FoParserResult *result, xmlChar *name);
FoPageSequenceMaster *fo_parser_result_lookup_page_sequence_master(FoParserResult *result, xmlChar *name);


void fo_solver_area_render_recursive(FoSolverArea *area, FoPrintContext *fpc);

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
void fo_print_context_push_state(FoPrintContext *fpc);
void fo_print_context_pop_state(FoPrintContext *fpc);
void fo_print_context_translate(FoPrintContext *fpc, FoUnit x, FoUnit y);

G_END_DECLS
#endif /* #if ENABLE_PRINTING */


#endif
