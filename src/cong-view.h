/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-view.h
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

#ifndef __CONG_VIEW_H__
#define __CONG_VIEW_H__

#define CONG_VIEW(x) ((CongView*)(x))

/* 
   CongView: a base class for views.  They register themselves with their document and get notified when it changes.

   Will eventually be ported to the GObject framework.
*/
struct CongView
{
	CongViewClass *klass;
	
	CongDocument *doc;
};

struct CongViewClass
{
	/* 
	   Hooks for the various change signals; eventually do this by listening to signals emitted from the document, porting to the standard 
	   GObject framework.

	   Many of the signals are sent twice; once before the change occurs, and once afterwards.  The boolean "before_change" is TRUE the first 
	   time and FALSE the second.
	*/
	void (*on_document_node_make_orphan)(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr former_parent);
	void (*on_document_node_add_after)(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr older_sibling);
	void (*on_document_node_add_before)(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr younger_sibling);
	void (*on_document_node_set_parent)(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
	void (*on_document_node_set_text)(CongView *view, gboolean before_change, CongNodePtr node, const xmlChar *new_content);
	void (*on_document_node_set_attribute)(CongView *view, gboolean before_change, CongNodePtr node, const xmlChar *name, const xmlChar *value);
	void (*on_document_node_remove_attribute)(CongView *view, gboolean before_change, CongNodePtr node, const xmlChar *name);
	void (*on_selection_change)(CongView *view);
	void (*on_cursor_change)(CongView *view);
};

typedef struct CongDocumentEvent CongDocumentEvent;

enum CongDocumentEventType
{
	CONG_DOCUMENT_EVENT_MAKE_ORPHAN,
	CONG_DOCUMENT_EVENT_ADD_AFTER,
	CONG_DOCUMENT_EVENT_ADD_BEFORE,
	CONG_DOCUMENT_EVENT_SET_PARENT,
	CONG_DOCUMENT_EVENT_SET_TEXT,
};

struct CongDocumentEvent
{
	gboolean before_event;

	enum CongDocumentEventType type;

	union
	{
		struct make_orphan {
			CongNodePtr node;
			CongNodePtr former_parent;
		} make_orphan;
		struct add_after {
			CongNodePtr node;
			CongNodePtr older_sibling;
		} add_after;
		struct add_before {
			CongNodePtr node;
			CongNodePtr younger_sibling;
		} add_before;
		struct set_parent {
			CongNodePtr node;
			CongNodePtr adoptive_parent;
		} set_parent;
		struct set_text {
			CongNodePtr node;
			const xmlChar *new_content;
		} set_text;
	} data;
};


CongDocument*
cong_view_get_document(CongView *view);

CongDispspec*
cong_view_get_dispspec(CongView *view);

#endif
