/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-modification-cursor-change.h
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

#ifndef __CONG_MODIFICATION_CURSOR_CHANGE_H__
#define __CONG_MODIFICATION_CURSOR_CHANGE_H__

#include "cong-modification.h"

G_BEGIN_DECLS

#define CONG_MODIFICATION_CURSOR_CHANGE_TYPE	  (cong_modification_cursor_change_get_type ())
#define CONG_MODIFICATION_CURSOR_CHANGE(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_MODIFICATION_CURSOR_CHANGE_TYPE, CongModificationCursorChange)
#define CONG_MODIFICATION_CURSOR_CHANGE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_MODIFICATION_CURSOR_CHANGE_TYPE, CongModificationCursorChangeClass)
#define IS_CONG_MODIFICATION_CURSOR_CHANGE(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_MODIFICATION_CURSOR_CHANGE_TYPE)

typedef struct CongModificationCursorChange CongModificationCursorChange;
typedef struct CongModificationCursorChangeClass CongModificationCursorChangeClass;
typedef struct CongModificationCursorChangeDetails CongModificationCursorChangeDetails;

struct CongModificationCursorChange
{
	CongModification base;

	CongModificationCursorChangeDetails *private;
};

struct CongModificationCursorChangeClass
{
	CongModificationClass klass;

	/* Methods? */
};

GType
cong_modification_cursor_change_get_type (void);

CongModificationCursorChange*
cong_modification_cursor_change_construct (CongModificationCursorChange *modification_cursor_change,
					   CongDocument *doc,
					   const CongLocation *new_location);
CongModification*
cong_modification_cursor_change_new (CongDocument *doc,
				     const CongLocation *new_location);

G_END_DECLS

#endif
