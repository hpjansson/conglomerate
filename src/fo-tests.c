/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * fo-tests.c
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
void cong_gnome_print_render_xslfo(xmlDocPtr xml_doc, GnomePrintJob *gpm)
{
	GnomePrintContext *gpc;
	FoPrintContext *fpc;
	FoParserResult *parser_result;
	FoSolverResult *solver_result;

	g_return_if_fail(xml_doc);
	g_return_if_fail(gpm);

	gpc = gnome_print_job_get_context (gpm);

	fpc = fo_print_context_new_from_gnome_print(gpc);

#if 1
	{
		parser_result = fo_parser_result_new_from_xmldoc(xml_doc);

		if (parser_result) {

#if 1
			/* View solver result: */
			solver_result = fo_solver_result_new_from_parser_result(parser_result);

			if (solver_result) {
				fo_solver_result_render(solver_result, fpc);

				fo_solver_result_delete(solver_result);
			}
#else
			/* View parser result: */
			fo_parser_result_test_render(parser_result, fpc);
#endif

			fo_parser_result_delete(parser_result);

		}
	}
#else
	{
		GnomeFont *font;
		font = gnome_font_find_closest ("Helvetica", 12);

		gnome_print_beginpage (gpc, "1");

		gnome_print_setfont (gpc, font);
		gnome_print_moveto (gpc, 100, 400);
		gnome_print_show (gpc, "This will eventually be the text from the FO doc...");

		gnome_print_moveto (gpc, 100, 200);
		gnome_print_lineto (gpc, 200, 200);
		gnome_print_stroke (gpc);

		gnome_print_showpage (gpc);
	}
#endif

	fo_print_context_delete(fpc);
	
}
#endif
