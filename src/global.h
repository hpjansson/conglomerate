/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * global.h
 *
 * Copyright (C) 2004 David Malcolm
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxml/tree.h>
#include <gtk/gtk.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <gconf/gconf-client.h>

/* We include GnomeVFS stuff here to try to alleviate build problems on Fink: */
#include <libgnomevfs/gnome-vfs.h>

G_BEGIN_DECLS

#define CONG_GCONF_PATH "/apps/conglomerate/"

#define RELEASE 1
#undef WINDOWS_BUILD

#ifdef WINDOWS_BUILD
# define isblank(c) ((c) == ' ' || (c) == '\n' || (c) == '\r' || (c) == '\t')
#endif

#define NEW_LOOK 1

#if ENABLE_PRINTING
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#endif

#if 1
#define CONG_VALIDATE_UTF8(str) ({g_assert(g_utf8_validate((str), -1, NULL));})
#else
#define CONG_VALIDATE_UTF8(str) ((void)0)
#endif

#include "cong-node.h"
#include "cong-location.h"

enum CongWhitespaceHandling {
	CONG_WHITESPACE_NORMALIZE,
	CONG_WHITESPACE_PRESERVE
};


typedef struct CongRange CongRange;

struct CongCursor
{
	/* Visual representation */
	GdkGC *gc;

	gboolean on;

	/* Conceptual location at which stuff will be inserted i.e. the byte_offset is the offset of the character immediately following the caret. So it's zero for the beginning of the text */
	CongLocation location;

	guint timeout_id;

	CongDocument *doc;
};


enum CongFontRole
{
	CONG_FONT_ROLE_BODY_TEXT,
	CONG_FONT_ROLE_SPAN_TAG,
	CONG_FONT_ROLE_TITLE_TEXT,

	/* replaces: f, fm, ft in order */

	CONG_FONT_ROLE_NUM
};

enum CongDispspecGCUsage
{
	CONG_DISPSPEC_GC_USAGE_BOLD_LINE,
	CONG_DISPSPEC_GC_USAGE_DIM_LINE,
	CONG_DISPSPEC_GC_USAGE_BACKGROUND,
	CONG_DISPSPEC_GC_USAGE_TEXT,

	CONG_DISPSPEC_GC_USAGE_NUM
};


typedef struct CongPlugin CongPlugin;
typedef struct CongPluginManager CongPluginManager;

typedef struct CongService CongService;
typedef struct CongServiceDocumentFactory CongServiceDocumentFactory;
typedef struct CongServiceImporter CongServiceImporter;
typedef struct CongServiceExporter CongServiceExporter;
#if ENABLE_PRINTING
typedef struct CongServicePrintMethod CongServicePrintMethod;
#endif
typedef struct CongServiceEditorNodeFactory CongServiceEditorNodeFactory;

typedef struct CongServiceTool CongServiceTool;
typedef struct CongServiceDocTool CongServiceDocTool;
typedef struct CongServiceNodeTool CongServiceNodeTool;

typedef struct CongServiceNodePropertyDialog CongServiceNodePropertyDialog;

CongEditorView *cong_editor_view_new(CongDocument *doc);
void cong_editor_view_free(CongEditorView *editor_view);
GtkWidget* cong_editor_view_get_widget(CongEditorView *editor_view);

/* Various view subclasses: */
GtkWidget *cong_dom_view_new(CongDocument *doc);
GtkWidget *cong_source_view_new(CongDocument *doc);
GtkWidget *cong_debug_message_log_view_new(CongDocument *doc);
GtkWidget *cong_debug_signal_log_view_new(CongDocument *doc);
GtkWidget *cong_node_properties_dialog_new(CongDocument *doc, 
					   CongNodePtr node, 
					   GtkWindow *parent_window);

gint cong_cursor_blink();

const char *xml_frag_data_nice(CongNodePtr x);
const char *xml_frag_name_nice(CongNodePtr x);

GList *xml_all_present_span_elements(CongDispspec *ds, CongNodePtr node);
GList *xml_all_valid_span_elements(CongDispspec *ds, CongNodePtr node);
char *xml_fetch_clean_data(CongNodePtr x);
gboolean xml_add_required_children(CongDocument *cong_doc, CongNodePtr node);

#define UNUSED_VAR(x)

void col_to_gcol(GdkColor *gcol, unsigned int col);

/* Cursor methods: */
void cong_cursor_init(CongCursor *curs, CongDocument *doc);
void cong_cursor_uninit(CongCursor *curs);
void cong_cursor_on(CongCursor *curs);
void cong_cursor_off(CongCursor *curs);
int cong_cursor_paragraph_insert(CongCursor *curs);
gboolean cong_cursor_calc_prev_char(CongCursor *curs, CongDocument *doc, CongLocation *output_loc);
gboolean cong_cursor_calc_next_char(CongCursor *curs, CongDocument *doc, CongLocation *output_loc);
void cong_cursor_next_line(CongCursor *curs, CongSpanEditor *xed);
void cong_cursor_del_prev_char(CongCursor *curs, CongDocument *doc);
void cong_cursor_del_next_char(CongCursor *curs, CongDocument *doc);
void cong_cursor_home(CongCursor *curs, CongDocument *doc);
void cong_cursor_end(CongCursor *curs, CongDocument *doc);

const CongLocation*
cong_cursor_get_location (const CongCursor *cursor);

void xv_style_r(GtkWidget *widget, gpointer data);

CongDispspec* 
query_for_forced_dispspec (gchar *what_failed, 
			   xmlDocPtr doc, 
			   GtkWindow* parent_window,
			   const gchar *filename_extension);

GtkWidget* make_uneditable_text(const gchar* text);

gchar*
get_col_string (const GdkColor* col);

/* Extensions to libxml: */
xmlAttrPtr	xmlNewProp_NUMBER	(xmlNodePtr node,
					 const xmlChar *name,
					 int value);

G_END_DECLS
