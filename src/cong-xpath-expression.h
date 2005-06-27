/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-xpath-expression.h
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

#ifndef __CONG_XPATH_EXPRESSION_H__
#define __CONG_XPATH_EXPRESSION_H__

#include "cong-node.h"
#include "cong-document.h"

G_BEGIN_DECLS

#define DEBUG_XPATH_EXPRESSION_LIFETIMES 0

#define CONG_XPATH_EXPRESSION_TYPE	      (cong_xpath_expression_get_type ())
#define CONG_XPATH_EXPRESSION(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_XPATH_EXPRESSION_TYPE, CongXPathExpression)
#define CONG_XPATH_EXPRESSION_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_XPATH_EXPRESSION_TYPE, CongXPathExpressionClass)
#define IS_CONG_XPATH_EXPRESSION(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_XPATH_EXPRESSION_TYPE)

typedef struct _CongXPathExpression CongXPathExpression;
typedef struct _CongXPathExpressionClass CongXPathExpressionClass;
typedef struct _CongXPathExpressionDetails CongXPathExpressionDetails;

/**
 * CongXPathExpression
 * 
 */
struct _CongXPathExpression
{
	GObject object;

	CongXPathExpressionDetails *private;
};

struct _CongXPathExpressionClass
{
	GObjectClass klass;

	/* Methods? */
};

GType
cong_xpath_expression_get_type (void);

CongXPathExpression*
cong_xpath_expression_construct (CongXPathExpression *xpath_expression,
				 CongDocument *doc,
				 CongNodePtr context_node,
				 const gchar *xpath_expression_string);

CongXPathExpression*
cong_xpath_expression_new (CongDocument *doc,
			   CongNodePtr context_node,
			   const gchar *xpath_expression_string);

CongDocument*
cong_xpath_expression_get_document (CongXPathExpression *xpath_expression);

CongNodePtr
cong_xpath_expression_get_context_node (CongXPathExpression *xpath_expression);

const gchar*
cong_xpath_expression_get_string_result (CongXPathExpression *xpath_expression);


G_END_DECLS

#endif
