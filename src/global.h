/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

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

GtkWidget* cong_gui_get_a_window(void);

gint cong_cursor_blink();

void new_document(GtkWindow *parent_window);

gint tree_new_sibling(GtkWidget *widget, CongNodePtr tag);
gint tree_new_sub_element(GtkWidget *widget, CongNodePtr tag);
gint tree_properties(GtkWidget *widget, CongNodePtr tag);
gint tree_cut(GtkWidget *widget, CongNodePtr tag);
gint tree_copy(GtkWidget *widget, CongNodePtr tag);
gint tree_paste_under(GtkWidget *widget, CongNodePtr tag);
gint tree_paste_before(GtkWidget *widget, CongNodePtr tag);
gint tree_paste_after(GtkWidget *widget, CongNodePtr tag);

const char *xml_frag_data_nice(CongNodePtr x);
const char *xml_frag_name_nice(CongNodePtr x);

GList *xml_all_present_span_elements(CongDispspec *ds, CongNodePtr node);
GList *xml_all_valid_span_elements(CongDispspec *ds, CongNodePtr node);
char *xml_fetch_clean_data(CongNodePtr x);
gboolean xml_add_required_children(CongDocument *cong_doc, CongNodePtr node);

char *tag_new_pick();

void open_document(GtkWindow *parent_window);
gint save_document(CongDocument *doc, GtkWindow *parent_window);
gint save_document_as(CongDocument *doc, GtkWindow *parent_window);

char *pick_structural_tag(CongDispspec *ds);

void open_document_do(const gchar *doc_name, GtkWindow *parent_window);

#define UNUSED_VAR(x)

int gui_window_new_document_make();

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

/* Popup (context) menus for editor view: */
void editor_popup_show(GtkWidget *widget, GdkEventButton *bevent);
void editor_popup_init();

/* Popup (context) menus for tree view and for section headings: */
GtkWidget* cong_ui_popup_init(CongDocument *doc, CongNodePtr node, GtkWindow *parent_window);

/* dialog to select from a list of string */
gchar* string_selection_dialog(gchar *title, gchar *element_description, GList *elements);

void xv_style_r(GtkWidget *widget, gpointer data);

CongDispspec* 
query_for_forced_dispspec (gchar *what_failed, 
			   xmlDocPtr doc, 
			   GtkWindow* parent_window,
			   const gchar *filename_extension);

GtkWidget* make_uneditable_text(const gchar* text);

gchar*
get_col_string (const GdkColor* col);

/* Toolbar hooks: */
gint toolbar_callback_open(GtkWidget *widget, gpointer data);
gint toolbar_callback_save(GtkWidget *w, gpointer data);
gint toolbar_callback_cut(GtkWidget *w, gpointer data);
gint toolbar_callback_copy(GtkWidget *w, gpointer data);
gint toolbar_callback_paste(GtkWidget *w, gpointer data);

/* Menu hooks: */
void menu_callback_debug_error(gpointer callback_data,
			       guint callback_action,
			       GtkWidget *widget);
void menu_callback_debug_document_types(gpointer callback_data,
					guint callback_action,
					GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_html(gpointer callback_data,
						   guint callback_action,
						   GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_xhtml(gpointer callback_data,
						    guint callback_action,
						    GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_html_help(gpointer callback_data,
							guint callback_action,
							GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_javahelp(gpointer callback_data,
						       guint callback_action,
						       GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_fo(gpointer callback_data,
						 guint callback_action,
						 GtkWidget *widget);
void menu_callback_debug_preview_fo(gpointer callback_data,
				    guint callback_action,
				    GtkWidget *widget);

#if PRINT_TESTS
void cong_gnome_print_render_xslfo(xmlDocPtr xml_doc, GnomePrintMaster *gpm);
#endif
void menu_callback_debug_dtd(gpointer callback_data,
			    guint callback_action,
			    GtkWidget *widget);

void menu_callback_debug_dialog(gpointer callback_data,
				guint callback_action,
				GtkWidget *widget);

void cong_menus_create_items(GtkItemFactory *item_factory, 
			     CongPrimaryWindow *primary_window);

/* UI routines for invocation by menus/toolbars: */
void
cong_ui_file_import(GtkWindow *toplevel_window);

void
cong_ui_file_export(CongDocument *doc,
		    GtkWindow *toplevel_window);

GtkWidget*
cong_file_properties_dialog_new (CongDocument *doc, 
				 GtkWindow *parent_window);

/* Extensions to libxml: */
xmlAttrPtr	xmlNewProp_NUMBER	(xmlNodePtr node,
					 const xmlChar *name,
					 int value);

G_END_DECLS
