/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-modification-selection-change.h
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

#ifndef __CONG_MODIFICATION_SELECTION_CHANGE_H__
#define __CONG_MODIFICATION_SELECTION_CHANGE_H__

#include "cong-modification.h"

G_BEGIN_DECLS

#define CONG_MODIFICATION_SELECTION_CHANGE_TYPE	  (cong_modification_selection_change_get_type ())
#define CONG_MODIFICATION_SELECTION_CHANGE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_MODIFICATION_SELECTION_CHANGE_TYPE, CongModificationSelectionChange)
#define CONG_MODIFICATION_SELECTION_CHANGE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_MODIFICATION_SELECTION_CHANGE_TYPE, CongModificationSelectionChangeClass)
#define IS_CONG_MODIFICATION_SELECTION_CHANGE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_MODIFICATION_SELECTION_CHANGE_TYPE)

typedef struct _CongModificationSelectionChange CongModificationSelectionChange;
typedef struct _CongModificationSelectionChangeClass CongModificationSelectionChangeClass;
typedef struct _CongModificationSelectionChangeDetails CongModificationSelectionChangeDetails;

struct _CongModificationSelectionChange
{
	CongModification base;

	CongModificationSelectionChangeDetails *private;
};

struct _CongModificationSelectionChangeClass
{
	CongModificationClass klass;

	/* Methods? */
};

GType
cong_modification_selection_change_get_type (void);

CongModificationSelectionChange*
cong_modification_selection_change_construct (CongModificationSelectionChange *modification_selection_change,
					      CongDocument *doc,
					      const CongLocation *new_logical_start,
					      const CongLocation *new_logical_end);
CongModification*
cong_modification_selection_change_new (CongDocument *doc,
					const CongLocation *new_logical_start,
					const CongLocation *new_logical_end);

G_END_DECLS

#endif
