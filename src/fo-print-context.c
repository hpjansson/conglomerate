/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * fo-print-context.c
 *
 * Copyright (C) 2002 David Malcolm
 *
 * FIXME:  This file is currently licensed under the GPL, but I intend it to eventually become part of a library licensed under the LGPL
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#include "global.h"
#include "fo.h"

struct FoPrintContext
{
	GnomePrintContext *gpc;
};

/* FoPrintContext internals: */

/* FoPrintContext methods: */
FoPrintContext *fo_print_context_new_from_gnome_print(GnomePrintContext *gpc)
{
	FoPrintContext *fpc;
	
	g_return_val_if_fail(gpc, NULL);

	fpc = g_new0(FoPrintContext, 1);

	fpc->gpc = gpc;

	return fpc;  
}

void fo_print_context_delete(FoPrintContext *fpc)
{
	g_return_if_fail(fpc);

	g_free(fpc);

	/* (No other cleanup required) */
}

void fo_print_context_beginpage(FoPrintContext *fpc, const gchar* name)
{
	g_return_if_fail(fpc);
	g_return_if_fail(name);

	g_assert(fpc->gpc);

	gnome_print_beginpage (fpc->gpc, name);
}

void fo_print_context_showpage (FoPrintContext *fpc)
{
	g_return_if_fail(fpc);

	g_assert(fpc->gpc);

	gnome_print_showpage (fpc->gpc);
}

void fo_print_context_test_rect(FoPrintContext *fpc, const FoRect *rect, const gchar *label)
{
	GnomeFont *font;

	g_return_if_fail(fpc);

	g_assert(fpc->gpc);

	if (label) {
		font = gnome_font_find_closest ("Helvetica", 12);

		gnome_print_setfont (fpc->gpc, font);
		gnome_print_moveto (fpc->gpc, rect->x, rect->y);
		gnome_print_show (fpc->gpc, label);
	}

	gnome_print_rect_stroked(fpc->gpc, rect->x, rect->y, rect->w, rect->h);
	
}

void fo_print_context_push_state(FoPrintContext *fpc)
{
	g_return_if_fail(fpc);

	g_assert(fpc->gpc);

	gnome_print_gsave(fpc->gpc);
}

void fo_print_context_pop_state(FoPrintContext *fpc)
{
	g_return_if_fail(fpc);

	g_assert(fpc->gpc);

	gnome_print_grestore(fpc->gpc);
}

void fo_print_context_translate(FoPrintContext *fpc, FoUnit x, FoUnit y)
{
	g_return_if_fail(fpc);

	g_assert(fpc->gpc);
	gnome_print_translate(fpc->gpc,
			      x,
			      y);
}
