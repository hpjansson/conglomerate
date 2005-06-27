/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-modification-set-dtd-ptr.h
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

#ifndef __CONG_MODIFICATION_SET_DTD_PTR_H__
#define __CONG_MODIFICATION_SET_DTD_PTR_H__

#include "cong-modification.h"

G_BEGIN_DECLS

#define CONG_MODIFICATION_SET_DTD_PTR_TYPE	  (cong_modification_set_dtd_ptr_get_type ())
#define CONG_MODIFICATION_SET_DTD_PTR(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_MODIFICATION_SET_DTD_PTR_TYPE, CongModificationSetDtdPtr)
#define CONG_MODIFICATION_SET_DTD_PTR_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_MODIFICATION_SET_DTD_PTR_TYPE, CongModificationSetDtdPtrClass)
#define IS_CONG_MODIFICATION_SET_DTD_PTR(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_MODIFICATION_SET_DTD_PTR_TYPE)

typedef struct _CongModificationSetDtdPtr CongModificationSetDtdPtr;
typedef struct _CongModificationSetDtdPtrClass CongModificationSetDtdPtrClass;
typedef struct _CongModificationSetDtdPtrDetails CongModificationSetDtdPtrDetails;

struct _CongModificationSetDtdPtr
{
	CongModification base;

	CongModificationSetDtdPtrDetails *private;
};

struct _CongModificationSetDtdPtrClass
{
	CongModificationClass klass;

	/* Methods? */
};

GType
cong_modification_set_dtd_ptr_get_type (void);

CongModificationSetDtdPtr*
cong_modification_set_dtd_ptr_construct (CongModificationSetDtdPtr *modification_set_dtd_ptr,
					 CongDocument *doc,
					 xmlDtdPtr dtd_ptr);
CongModification*
cong_modification_set_dtd_ptr_new (CongDocument *doc,
				   xmlDtdPtr dtd_ptr);

G_END_DECLS

#endif
