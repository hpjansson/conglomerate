/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * fo-rect.c
 *
 * Copyright (C) 2002 David Malcolm
 *
 * FIXME:  This file is currently licensed under the GPL, but I intend it to eventually become part of a library licensed under the LGPL
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#include "global.h"
#include "fo.h"

/* FoRect methods: */
void fo_rect_set_xywh(FoRect *rect, FoUnit x, FoUnit y, FoUnit w, FoUnit h)
{
	g_return_if_fail(rect);

	rect->x=x;
	rect->y=y;
	rect->w=w;
	rect->h=h;
}

void fo_rect_set_xyxy(FoRect *rect, FoUnit x0, FoUnit y0, FoUnit x1, FoUnit y1)
{
	g_return_if_fail(rect);

	rect->x=x0;
	rect->y=y0;
	rect->w=x1-x0;
	rect->h=y1-y0;
}

void fo_rect_test_render(const FoRect *rect, FoPrintContext *fpc, const gchar *label)
{
	fo_print_context_test_rect(fpc, rect, label);
}

