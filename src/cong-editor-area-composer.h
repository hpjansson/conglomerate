/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-editor-area-composer.h
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

#ifndef __CONG_EDITOR_AREA_COMPOSER_H__
#define __CONG_EDITOR_AREA_COMPOSER_H__

#include "cong-editor-area-container.h"

G_BEGIN_DECLS

typedef struct CongEditorAreaComposer CongEditorAreaComposer;
typedef struct CongEditorAreaComposerClass CongEditorAreaComposerClass;
typedef struct CongEditorAreaComposerDetails CongEditorAreaComposerDetails;

#define CONG_EDITOR_AREA_COMPOSER_TYPE	   (cong_editor_area_composer_get_type ())
#define CONG_EDITOR_AREA_COMPOSER(obj)         G_TYPE_CHECK_INSTANCE_CAST (obj, CONG_EDITOR_AREA_COMPOSER_TYPE, CongEditorAreaComposer)
#define CONG_EDITOR_AREA_COMPOSER_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, CONG_EDITOR_AREA_COMPOSER_TYPE, CongEditorAreaComposerClass)
#define IS_CONG_EDITOR_AREA_COMPOSER(obj)      G_TYPE_CHECK_INSTANCE_TYPE (obj, CONG_EDITOR_AREA_COMPOSER_TYPE)

struct CongEditorAreaComposer
{
	CongEditorAreaContainer area;

	CongEditorAreaComposerDetails *private;
};

struct CongEditorAreaComposerClass
{
	CongEditorAreaContainerClass klass;
};

GType
cong_editor_area_composer_get_type (void);

CongEditorArea*
cong_editor_area_composer_construct (CongEditorAreaComposer *area_composer,
				     CongEditorWidget3 *editor_widget,
				     GtkOrientation orientation,
				     guint spacing);

CongEditorArea*
cong_editor_area_composer_new (CongEditorWidget3 *editor_widget,
			       GtkOrientation orientation,
			       guint spacing);

/* (Options mostly correspond to those of gtk_box_pack_start; doesn't put extra_padding at end of composer though) */
void
cong_editor_area_composer_pack_start (CongEditorAreaComposer *area_composer,
				      CongEditorArea *child,
				      gboolean expand,
				      gboolean fill,
				      guint extra_padding);
void
cong_editor_area_composer_pack_end (CongEditorAreaComposer *area_composer,
				    CongEditorArea *child,
				    gboolean expand,
				    gboolean fill,
				    guint extra_padding);

void
cong_editor_area_composer_pack_after (CongEditorAreaComposer *area_composer,
				      CongEditorArea *new_child,
				      CongEditorArea *relative_to,
				      gboolean expand,
				      gboolean fill,
				      guint extra_padding);

void 
cong_editor_area_composer_set_child_packing (CongEditorAreaComposer *area_composer,
					     CongEditorArea *child,
					     gboolean expand,
					     gboolean fill,
					     guint extra_padding);

GList*
cong_editor_area_composer_get_child_area_iter_first (CongEditorAreaComposer *area_composer);

CongEditorArea*
cong_editor_area_composer_get_child_area (CongEditorAreaComposer *area_composer,
					  GList *iter);


G_END_DECLS

#endif
