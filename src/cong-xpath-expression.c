/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-xpath-expression.c
 *
 * Copyright (C) 2003 David Malcolm
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
#include <libgnome/gnome-macros.h>
#include "cong-eel.h"
#include "cong-xpath-expression.h"
#include <libxml/tree.h>
#include <libxml/debugXML.h>
#include <libxml/xpath.h>

#define PRIVATE(x) ((x)->private)

enum {
	STRING_RESULT_CHANGED,

	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

struct CongXPathExpressionDetails
{
	CongDocument *doc;
	CongNodePtr context_node;

	gchar *string_expression;
	gchar *cached_string_result;

	xmlXPathContextPtr ctxt;
};

/* Declarations of the GObject handlers: */
static void
finalize (GObject *object);

static void
dispose (GObject *object);

/* Implementation details: */
static gchar*
evaluate_expression (CongXPathExpression *xpath_expression);

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongXPathExpression, 
			cong_xpath_expression,
			GObject,
			G_TYPE_OBJECT );

static void
cong_xpath_expression_class_init (CongXPathExpressionClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	signals[STRING_RESULT_CHANGED] = g_signal_new ("string_result_changed",
						       CONG_XPATH_EXPRESSION_TYPE,
						       G_SIGNAL_RUN_FIRST,
						       0,
						       NULL, NULL,
						       g_cclosure_marshal_VOID__VOID,
						       G_TYPE_NONE, 
						       0);
}

static void
cong_xpath_expression_instance_init (CongXPathExpression *node)
{
	node->private = g_new0(CongXPathExpressionDetails,1);
}

CongXPathExpression*
cong_xpath_expression_construct (CongXPathExpression *xpath_expression,
				 CongDocument *doc,
				 CongNodePtr context_node,
				 const gchar *xpath_expression_string)
{
	PRIVATE(xpath_expression)->doc = doc;
	PRIVATE(xpath_expression)->context_node = context_node;
	PRIVATE(xpath_expression)->string_expression = g_strdup (xpath_expression_string);

	PRIVATE(xpath_expression)->ctxt = xmlXPathNewContext(context_node->doc);
	PRIVATE(xpath_expression)->ctxt->node = context_node;

	return xpath_expression;
}

CongXPathExpression*
cong_xpath_expression_new (CongDocument *doc,
			   CongNodePtr context_node,
			   const gchar *xpath_expression_string)
{
	g_return_val_if_fail (doc, NULL);
	g_return_val_if_fail (context_node, NULL);
	g_return_val_if_fail (xpath_expression_string, NULL);

	return cong_xpath_expression_construct
		(g_object_new (CONG_XPATH_EXPRESSION_TYPE, NULL),
		 doc,
		 context_node,
		 xpath_expression_string);
}

CongDocument*
cong_xpath_expression_get_document (CongXPathExpression *xpath_expression)
{
	g_return_val_if_fail (xpath_expression, NULL);
	
	return PRIVATE(xpath_expression)->doc;
}

CongNodePtr
cong_xpath_expression_get_context_node (CongXPathExpression *xpath_expression)
{
	g_return_val_if_fail (xpath_expression, NULL);

	return PRIVATE(xpath_expression)->context_node;
}

const gchar*
cong_xpath_expression_get_string_result (CongXPathExpression *xpath_expression)
{
	g_return_val_if_fail (xpath_expression, NULL);

	if (NULL==PRIVATE(xpath_expression)->cached_string_result) {

		PRIVATE(xpath_expression)->cached_string_result = evaluate_expression (xpath_expression); /* for now */	
	}

	return PRIVATE(xpath_expression)->cached_string_result;
}

static void
finalize (GObject *object)
{
	CongXPathExpression *xpath_expression = CONG_XPATH_EXPRESSION (object);

	g_free (xpath_expression->private);
	xpath_expression->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
dispose (GObject *object)
{
	CongXPathExpression *xpath_expression = CONG_XPATH_EXPRESSION (object);

	g_assert (xpath_expression->private);

	if (PRIVATE (xpath_expression)->string_expression) {
		g_free (PRIVATE (xpath_expression)->string_expression);
		PRIVATE (xpath_expression)->string_expression = NULL;
	}
	
	if (PRIVATE (xpath_expression)->cached_string_result) {
		g_free (PRIVATE (xpath_expression)->cached_string_result);
		PRIVATE (xpath_expression)->cached_string_result = NULL;
	}

	xmlXPathFreeContext (PRIVATE(xpath_expression)->ctxt);

	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

/* Implementation details: */
static gchar*
evaluate_expression (CongXPathExpression *xpath_expression)
{
	xmlXPathObjectPtr xpath_obj;

	gchar *result = NULL;

	g_return_val_if_fail (IS_CONG_XPATH_EXPRESSION (xpath_expression), NULL);
	
	/* g_message("searching xpath \"%s\"",element->header_info->xpath); */

	/* FIXME: compile the path */
	xpath_obj = xmlXPathEval(PRIVATE(xpath_expression)->string_expression,
				 PRIVATE(xpath_expression)->ctxt);	

	if (xpath_obj) {
		result = xmlXPathCastToString(xpath_obj);			
	} else {
		result = g_strdup(_("(xpath failed)"));
	}	

	return result;
}
