/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * fo-solver-result.c
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
void fo_solver_area_add_child(FoSolverArea *area, FoSolverArea *child)
{
	g_assert(child->parent==NULL);

	area->children = g_list_append(area->children, child);
	child->parent = area;
}

void fo_solver_area_render_recursive(FoSolverArea *area, FoPrintContext *fpc)
{
	GList *iter;

	g_return_if_fail(area);
	g_return_if_fail(fpc);

	fo_print_context_push_state(fpc);

	/* Render this, if appropriate: */
	if (FO_OBJECT(area)->klass) {
		if (FO_SOLVER_AREA_CLASS(FO_OBJECT(area)->klass)->render) {
			FO_SOLVER_AREA_CLASS(FO_OBJECT(area)->klass)->render(area,
									     fpc);
		}
	}

	/* Recurse through children: */
	for (iter = area->children; iter; iter=iter->next) {
		FoSolverArea *child = iter->data;

		fo_solver_area_render_recursive(child, fpc);
	}

	fo_print_context_pop_state(fpc);
};

/* FoSolverResult internals: */
void test_rect_render(FoSolverArea *area,
		      FoPrintContext *fpc)
{
	FoSolverTestRect *test_rect = FO_SOLVER_TEST_RECT(area);
	
	fo_rect_test_render(&test_rect->rect, fpc, test_rect->text);	
}

const FoSolverAreaClass test_rect_class =
{
	{"test_rect_class"},
	test_rect_render
};

FoSolverTestRect *fo_solver_test_rect_new(const FoRect *rect, const gchar *text)
{
	FoSolverTestRect *test_rect;

	test_rect = g_new0(FoSolverTestRect,1);
	FO_OBJECT(test_rect)->klass = FO_CLASS(&test_rect_class);

	test_rect->rect = *rect;
	test_rect->text = g_strdup(text);

	return test_rect;
};

void rect_render(FoSolverArea *area,
		 FoPrintContext *fpc)
{
	FoSolverRect *rect = FO_SOLVER_RECT(area);
	
	fo_print_context_translate(fpc, rect->rect.x, rect->rect.y);
}

const FoSolverAreaClass rect_class =
{
	{"rect_class"},
	rect_render
};

FoSolverRect *fo_solver_rect_new(const FoRect *rect)
{
	FoSolverRect *solver_rect;

	solver_rect = g_new0(FoSolverRect,1);
	FO_OBJECT(solver_rect)->klass = FO_CLASS(&rect_class);

	solver_rect->rect = *rect;

	return solver_rect;
};

void text_render(FoSolverArea *area,
		 FoPrintContext *fpc)
{
	FoSolverText *text = FO_SOLVER_TEXT(area);

	FoRect rect;

	fo_rect_set_xywh(&rect, ((double)rand()*5000.0)/(double)RAND_MAX,((double)rand()*5000.0)/(double)RAND_MAX,200,20);
	
	fo_rect_test_render(&rect, fpc, text->text);
}

const FoSolverAreaClass text_class =
{
	{"text_class"},
	text_render
};

const FoSolverAreaClass page_class =
{
	{"page_class"},

	NULL		       
};

FoSolverPage *fo_solver_page_new(FoSimplePageMaster *spm)
{
	FoSolverPage *page = g_new0(FoSolverPage,1);

	FO_OBJECT(page)->klass = FO_CLASS(&page_class);

	page->spm = spm;

	return page;
}


static void add_page(FoSolverResult *solver_result, FoSolverPage *page)
{
	solver_result->list_of_solver_page = g_list_append(solver_result->list_of_solver_page, page);
	solver_result->num_pages++;
#if 0
	solver_result->current_page = page;
	solver_result->insertion_y = 26*30.0;
#endif
}

static void pour_single_area(FoSolverResult *solver_result, FoSolverArea *area, FoSolverArea *insertion_area)
{
#if 1
	fo_solver_area_add_child(insertion_area, area);	
#else
	FoRect rect;
	FoSolverRect *solver_rect;

	if (solver_result->insertion_y<30.0f) {
		/* We've run out of room; start a new page: */
		
		FoSimplePageMaster *spm;
		FoSolverPage *page;

		/* generate a page; uses first spm for now: */
		spm = solver_result->parser_result->list_of_simple_page_master->data;
		page = fo_solver_page_new(spm);
			
		add_page(solver_result, page);
	}

	fo_rect_set_xywh(&rect, 150.0, solver_result->insertion_y, 200.0, 20.0);

	solver_rect = fo_solver_rect_new(&rect);

	fo_solver_area_add_child(FO_SOLVER_AREA(solver_result->current_page), FO_SOLVER_AREA(solver_rect));
	fo_solver_area_add_child(FO_SOLVER_AREA(solver_rect), FO_SOLVER_AREA(area));
	solver_result->insertion_y-=30.0;
#endif
}

/* Currently very hackish: */
static void pour_flow_recursive(FoSolverResult *solver_result, xmlNodePtr node, FoSolverArea *insertion_area)
{
	xmlNodePtr iter;

	/* Handle this node: */
	
	if (0==xmlStrcmp(node->name, "block")) {
		/* create block area(s) */
		FoSolverBlockArea *block_area;

		block_area = g_new0(FoSolverBlockArea,1);

		pour_single_area(solver_result, FO_SOLVER_AREA(block_area), insertion_area );

		insertion_area = FO_SOLVER_AREA(block_area);

	} else if (0==xmlStrcmp(node->name, "inline")) {
		/* create inline area(s) */
		FoSolverInlineArea *inline_area;

		inline_area = g_new0(FoSolverInlineArea,1);

		pour_single_area(solver_result, FO_SOLVER_AREA(inline_area), insertion_area );

		insertion_area = FO_SOLVER_AREA(inline_area);

	} else if (node->type==XML_TEXT_NODE) {

		/* Split the text into a list of PangoItem:  */
		{
			PangoAttrList* attr_list;
			PangoAttribute *attribute;
			GList *list_of_pango_item;
			GList *iter;

			attr_list = pango_attr_list_new();
				
			attribute = pango_attr_family_new("Serif");
				/* attribute = pango_attr_family_new("Sans"); */
				/* attribute = pango_attr_family_new("Monospace"); */
			
			/* is this code necessary? */
			attribute->start_index = 0;
			attribute->end_index = 5; /* g_strlen(node->content); */

			pango_attr_list_insert( attr_list, attribute );
				
			list_of_pango_item = pango_itemize(solver_result->pango_context,
							   node->content,
							   0, /*  int start_index, */
							   strlen(node->content), /* int length (in bytes, hence strlen is appropriate) */
							   attr_list, /*  PangoAttrList *attrs, */
							   NULL /*  PangoAttrIterator *cached_iter */);

			/* For each PangoItem: */ /* FIXME: should there always be one at the moment? */
			for (iter = list_of_pango_item; iter; iter=iter->next) {
				PangoItem *item = iter->data;
				PangoLogAttr *attrs;
				int i;
				int start_of_line;

				#if 0
				{
					gchar *item_text = g_strndup(node->content + item->offset, item->num_chars);

					g_message("got pango item: \"%s\"\n", item_text);

					g_free(item_text);					
				}
				#endif

				attrs = g_new0(PangoLogAttr, item->num_chars+1);

				/* Calculate linebreak properties of the characters in the text: */
				pango_break(node->content + item->offset,
					    item->length,
					    &item->analysis,
					    attrs,
					    item->num_chars+1);

				start_of_line = 0;
				for (i=0;i<item->num_chars;i++) {
					if (attrs[i].is_line_break) {
						/*  g_message("char %i is possible line break\n", i); */

						if (i>start_of_line) {
							/* Then we have a run of text between two possible line-breaks; convert it to a glyph string */
							FoSolverText *solver_text;

							solver_text = g_new0(FoSolverText,1);
							FO_OBJECT(solver_text)->klass = FO_CLASS(&text_class);
							solver_text->glyph_string = pango_glyph_string_new();
							solver_text->pango_font = item->analysis.font;
							solver_text->text = g_strndup(g_utf8_offset_to_pointer(node->content + item->offset, start_of_line), 
										      i-start_of_line);

							/* Convert characters into glyphs: */
							pango_shape( g_utf8_offset_to_pointer(node->content + item->offset, start_of_line),
								     g_utf8_offset_to_pointer(node->content + item->offset, i) - g_utf8_offset_to_pointer(node->content + item->offset, start_of_line),
								     &item->analysis,
								     solver_text->glyph_string);

							/* Find out the extent of the string: */
							pango_glyph_string_extents( solver_text->glyph_string,
										    solver_text->pango_font,
										    &solver_text->ink_rect,
										    &solver_text->logical_rect);

							/* Add an area: */
							pour_single_area(solver_result, FO_SOLVER_AREA(solver_text), insertion_area );
						}

						start_of_line = i+1;
					}
				}
				
				g_free(attrs);
			}
		}

	}

	/* Recurse through children: */
	for (iter = node->children; iter; iter = iter->next) {
		pour_flow_recursive(solver_result, iter, insertion_area);
	}

}

static void pour_flow(FoSolverResult *solver_result, FoFlow *flow)
{
	pour_flow_recursive(solver_result, FO_PARSER_OBJECT(flow)->node, FO_SOLVER_AREA(solver_result->list_of_solver_page->data));
}

void run_solver(FoSolverResult *solver_result, FoParserResult *parser_result)
{
	GList *iter;

	solver_result->parser_result = parser_result;

	solver_result->pango_context = pango_context_new();

	solver_result->pango_font_map = pango_ft2_font_map_for_display(); /* FIXME: will this be available? */

	pango_context_set_font_map(solver_result->pango_context,
				   solver_result->pango_font_map);



	{
		PangoFontFamily **families;
		int n_families;
		int i;

		pango_font_map_list_families(solver_result->pango_font_map,
                                             &families,
                                             &n_families);

		for (i=0;i<n_families;i++) {
			g_message("Got font family \"%s\"\n", pango_font_family_get_name (families[i]));			
		}

		g_free(families);
	}

	/* For now, iterate through all page-sequences in the parser-result, generating 5 pages for each? */
	for (iter = parser_result->list_of_page_sequence; iter; iter = iter->next) {
		FoPageSequence *ps;
		FoSimplePageMaster *spm;
		FoSolverPage *page;
		int i;
		
		ps = iter->data;
		g_assert(ps->psg);

		solver_result->current_ps = ps;

#if 1

		/* generate a page; uses first spm for now: */
		spm = parser_result->list_of_simple_page_master->data;
		page = fo_solver_page_new(spm);

		add_page(solver_result, page);

		/* Generate some content: */
		{
#if 1 
			if (ps->flow) {
				pour_flow(solver_result, ps->flow);
			}
#else
			FoRect rect;

			fo_rect_set_xywh(&rect, 150.0, 100.0, 200.0, 200.0);
			fo_solver_area_add_child(FO_SOLVER_AREA(page), FO_SOLVER_AREA(fo_solver_test_rect_new(&rect, "test1")));

			fo_rect_set_xywh(&rect, 250.0, 100.0, 200.0, 200.0);
			fo_solver_area_add_child(FO_SOLVER_AREA(page), FO_SOLVER_AREA(fo_solver_test_rect_new(&rect, "test2")));

			fo_rect_set_xywh(&rect, 150.0, 150.0, 200.0, 200.0);
			fo_solver_area_add_child(FO_SOLVER_AREA(page), FO_SOLVER_AREA(fo_solver_test_rect_new(&rect, "test3")));
#endif
		}
#else
		for (i=0;i<5;i++) {

			spm = fo_page_sequence_generator_get_master_for_next_page(ps->psg,
										  (solver_result->num_pages+1),
										  (i==0), /* gboolean is_first_of_sequence, */
										  (i==4), /* gboolean is_last_of_sequence, */
										  FALSE /* gboolean is_blank */);

			g_assert(spm);
			g_message("Using spm \"%s\"\n", spm->base.name);
		}
#endif
	}

	
}

/* FoSolverResult methods: */
FoSolverResult *fo_solver_result_new_from_parser_result(FoParserResult *parser_result)
{
	FoSolverResult *solver_result;
	
	g_return_val_if_fail(parser_result, NULL);

	solver_result = g_new0(FoSolverResult, 1);

	run_solver(solver_result, parser_result);

	return solver_result;  
}

void fo_solver_result_delete(FoSolverResult *result)
{
	g_return_if_fail(result);

	/* FIXME: cleanup! */

	g_free(result);
}

void fo_solver_result_render(FoSolverResult *result, FoPrintContext *fpc)
{
	GList *iter;

	g_return_if_fail(result);
	g_return_if_fail(fpc);

	/* This algorithm is done; we simply need to generate the areas... */
	for (iter = result->list_of_solver_page; iter; iter = iter->next) {
		FoSolverPage *page = iter->data;
		FoRect rect;

		fo_print_context_beginpage(fpc, page->spm->base.name);

		fo_solver_area_render_recursive(FO_SOLVER_AREA(page), fpc);
	
		fo_print_context_showpage (fpc);	

	}

}

#endif
