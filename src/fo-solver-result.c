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

/* FoSolverResult internals: */

/* FoSolverResult methods: */
FoSolverResult *fo_solver_result_new_from_parser_result(FoParserResult *parser_result)
{
	FoSolverResult *solver_result;
	
	g_return_val_if_fail(parser_result, NULL);

	solver_result = g_new0(FoSolverResult, 1);

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
	g_return_if_fail(result);
	g_return_if_fail(fpc);
  
	/* FIXME: unwritten */
}
