/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxml/tree.h>
#include <gtk/gtk.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gconf/gconf-client.h>

G_BEGIN_DECLS

#define CONG_GCONF_PATH "/apps/conglomerate/"

#define RELEASE 1
#undef WINDOWS_BUILD

#ifdef WINDOWS_BUILD
# define isblank(c) ((c) == ' ' || (c) == '\n' || (c) == '\r' || (c) == '\t')
#endif

/* FIXME: eventually move this to cong-tree-view.c, along with bits of popup.c: */
enum
{
	TREEVIEW_TITLE_COLUMN,
	TREEVIEW_NODE_COLUMN,
	TREEVIEW_DOC_COLUMN,
	TREEVIEW_FOREGROUND_COLOR_COLUMN,
	TREEVIEW_BACKGROUND_COLOR_COLUMN,
	TREEVIEW_N_COLUMNS
};

#define NEW_LOOK 1
#define PRINT_TESTS 0
#define USE_CONG_EDITOR_WIDGET 1

#include <libgnomeprint/gnome-print.h>
#if PRINT_TESTS
#include <libgnomeprint/gnome-print-master.h>
#endif

#if 1
#define CONG_VALIDATE_UTF8(str) (g_assert(g_utf8_validate((str), -1, NULL)))
#else
#define CONG_VALIDATE_UTF8(str) ((void)0)
#endif

#include "cong-node.h"
#include "cong-location.h"

GtkWidget*
cong_span_editor_get_widget(CongSpanEditor *xed);

#if 0
GdkFont*
cong_span_editor_get_font(CongSpanEditor *xed);
#endif

CongDispspec*
cong_span_editor_get_dispspec(CongSpanEditor *xed);

/*
  Code to handle cached layout information.  Stored during rendering, used 
  when handling cursor movement.
 */

typedef struct CongLayoutCache
{
	struct CongLayoutLine *first_line;
	struct CongLayoutLine *last_line;

} CongLayoutCache;

typedef struct CongLayoutLine
{
	struct CongLayoutLine *next;
	struct CongLayoutLine *prev;

	/* Data set by add_line: */
	CongNodePtr tt;
	int i;  /* c_given */
	
	/* Data set by add stuff: */
	gboolean got_rest_of_data;
	int pos_y;
	CongNodePtr x;
	int draw_char;  /* Seems to be unused */

} CongLayoutLine;

void
cong_layout_cache_init(CongLayoutCache *layout_cache);

void
cong_layout_cache_clear(CongLayoutCache *layout_cache);

CongLayoutLine*
cong_layout_cache_get_line_by_y_coord(CongLayoutCache *layout_cache, int y);

CongLayoutLine*
cong_layout_cache_get_line_by_index(CongLayoutCache *layout_cache, int i);

CongLayoutLine*
cong_layout_cache_get_last_line(CongLayoutCache *layout_cache);

CongLayoutLine*
cong_layout_line_get_next(CongLayoutLine *line);

CongLayoutLine*
cong_layout_line_get_prev(CongLayoutLine *line);

int
cong_layout_line_get_second_y(CongLayoutLine *line);

CongNodePtr
cong_layout_line_get_node(CongLayoutLine *line);

CongNodePtr
cong_layout_line_get_node_last(CongLayoutLine *line);

int
cong_layout_line_get_c_given(CongLayoutLine *line);


/*
  The tag stack.  Used for rendering the nested underlinings for span tags.
 */
typedef struct CongLayoutStackEntry
{
	char *text;
	int line;
	int pos_x;
	CongNodePtr x;
	int lev;
	struct CongLayoutStackEntry *above;
	struct CongLayoutStackEntry *below;

} CongLayoutStackEntry;

typedef struct CongLayoutStack
{
	CongLayoutStackEntry *bottom;
	
} CongLayoutStack;

CongLayoutStackEntry*
cong_layout_stack_top(CongLayoutStack *layout_stack);

CongLayoutStackEntry*
cong_layout_stack_bottom(CongLayoutStack *layout_stack);

void
cong_layout_stack_push(CongLayoutStack *layout_stack, const char* s, int line, int pos_x, CongNodePtr x, int lev);



/* gets the next entry i.e. the entry above, heading from bottom to top */
CongLayoutStackEntry*
cong_layout_stack_entry_next(CongLayoutStackEntry *entry);

/* gets the parent entry i.e. the entry below */
CongLayoutStackEntry*
cong_layout_stack_entry_below(CongLayoutStackEntry *entry);

/* Various access methods: */
int cong_layout_stack_entry_get_line(CongLayoutStackEntry *entry);
int cong_layout_stack_entry_get_pos_x(CongLayoutStackEntry *entry);
#if 0
TTREE *cong_layout_stack_entry_get_ttree_x(CongLayoutStackEntry *entry);
#endif
int cong_layout_stack_entry_get_lev(CongLayoutStackEntry *entry);


#if !USE_CONG_EDITOR_WIDGET
/* modes:
 * 
 * 0 = calculate height only
 * 1 = draw and calculate height
 *
 */
enum CongDrawMode
{
	CONG_DRAW_MODE_CALCULATE_HEIGHT,
	CONG_DRAW_MODE_CALCULATE_HEIGHT_AND_DRAW
};

#define CONG_SPAN_EDITOR(x) ((CongSpanEditor*)(x))

struct CongSpanEditor
{
	CongView view;

	/* Display information */

	GtkWidget *e;  /* Eventbox */
	GtkWidget *w;  /* Drawing area */
	GdkPixmap *p;  /* Backing pixmap */

	CongDispspec *displayspec;
	CongDocument *doc;
	
	int tag_height;
	
	int initial;
	int already_asked_for_size;

	/* Drawing information (temporary) */
	enum CongDrawMode mode;

	CongNodePtr draw_x;      /* XML node currently at */

	CongNodePtr draw_x_prev;
	int draw_char_prev;

	int draw_line;      /* Line to draw at (in display) */
	int draw_char;      /* Char to begin drawing (in node) */
	int draw_pos_x;     /* Pixel to start drawing at */
	int draw_pos_y;     /* Y position to start drawing at */
	int draw_tag_max;


	/* Data content */

	CongNodePtr x;

	CongLayoutCache layout_cache;
	CongLayoutStack layout_stack;

	/* Cursor information */
#if 0
	TTREE *curs_x;      /* XML node currently at */
#endif
	int curs_char;      /* Cursor positioned after this char (in node) */

};
#endif /* #if !USE_CONG_EDITOR_WIDGET */

struct pos
{
	int x, y;
	int x_find;

	int line;
	CongNodePtr node;
	CongNodePtr node_last;
	CongNodePtr node_find;

	int c, c_given;
	int space;  /* 0 = have no space, 1 = have space */

	int word_width;
	
	int mode;  /* 0 = not found, 1 = found */
};



struct CongCursor
{
	/* Visual representation */
#if !USE_CONG_EDITOR_WIDGET
	CongSpanEditor *xed;
#endif

	GtkWidget *w;
	GdkGC *gc;
	int x, y;
	int line;
	int on;

	/* Conceptual location */
	CongLocation location;

	int set;

	guint timeout_id;

	CongDocument *doc;
};


struct CongSelection
{
#if !USE_CONG_EDITOR_WIDGET
	CongSpanEditor *xed;
#endif
	GdkGC *gc_0, *gc_1, *gc_2, *gc_3;  /* 0 is brightest, 3 is darkest */

	int x0, y0, x1, y1;

	CongLocation loc0;
	CongLocation loc1;
};

enum CongFontRole
{
	CONG_FONT_ROLE_BODY_TEXT,
	CONG_FONT_ROLE_SPAN_TAG,
	CONG_FONT_ROLE_TITLE_TEXT,

	/* replaces: f, fm, ft in order */

	CONG_FONT_ROLE_NUM
};

CongPrimaryWindow *cong_primary_window_new(CongDocument *doc);
void cong_primary_window_free(CongPrimaryWindow *primary_window);
CongDocument *cong_primary_window_get_document(CongPrimaryWindow *primary_window);
void cong_primary_window_update_title(CongPrimaryWindow *primary_window);
GtkWindow *cong_primary_window_get_toplevel(CongPrimaryWindow *primary_window);

CongTreeView *cong_tree_view_new(CongDocument *doc);
void cong_tree_view_free(CongTreeView *tree_view);
GtkWidget* cong_tree_view_get_widget(CongTreeView *tree_view);
GtkTreeStore* cong_tree_view_get_tree_store(CongTreeView *tree_view);

CongEditorView *cong_editor_view_new(CongDocument *doc);
void cong_editor_view_free(CongEditorView *editor_view);
GtkWidget* cong_editor_view_get_widget(CongEditorView *editor_view);

GtkWidget *cong_test_view_new(CongDocument *doc);
GtkWidget *cong_source_view_new(CongDocument *doc);

#if 1
GtkWidget* cong_gui_get_a_window(void);
#else
GtkWidget* cong_gui_get_window(struct cong_gui* gui);
GtkWidget* cong_gui_get_popup(struct cong_gui* gui);
void cong_gui_set_popup(struct cong_gui* gui, GtkWidget* popup);
GtkWidget* cong_gui_get_button_submit(struct cong_gui* gui);
GtkTreeStore* cong_gui_get_tree_store(struct cong_gui* gui);
GtkTreeView* cong_gui_get_tree_view(struct cong_gui* gui);
GtkWidget* cong_gui_get_root(struct cong_gui* gui);
void cong_gui_destroy_tree_store(struct cong_gui* gui);
#endif

gint cong_cursor_blink();
int login();
void new_document(GtkWindow *parent_window);
int find_document();
int find_documentmetacaps();
int submit_do();
int gui_window_login_make(char **user_s, char **pass_s);

CongSpanEditor *xmledit_new(CongNodePtr x, CongDocument *doc, CongDispspec *displayspec);

CongFont*
cong_span_editor_get_font(CongSpanEditor *xed, enum CongFontRole role);

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

#if !USE_CONG_EDITOR_WIDGET
struct pos *pos_physical_to_logical(struct CongSpanEditor *xed, int x, int y);
struct pos *pos_logical_to_physical(struct CongSpanEditor *xed, CongNodePtr node, int c);
struct pos *pos_logical_to_physical_new(struct CongSpanEditor *xed, CongLocation *loc);
#endif

CongNodePtr xml_frag_data_nice_split3(CongDocument *doc, CongNodePtr s, int c0, int c1);
CongNodePtr xml_frag_data_nice_split2(CongDocument *doc, CongNodePtr s, int c);

CongNodePtr cong_selection_reparent_all(CongSelection *selection, CongDocument *doc, CongNodePtr p);
void cong_selection_delete(CongSelection *selection, CongDocument *doc);

GList *xml_all_span_elements(CongDispspec *ds, CongNodePtr node);
char *xml_fetch_clean_data(CongNodePtr x);
gboolean xml_add_required_children(CongDocument *cong_doc, CongNodePtr node);
xmlElementPtr xml_get_dtd_element(CongDocument *cong_doc, CongNodePtr node);
GList* xml_get_valid_children(CongDispspec* ds, CongNodePtr node, enum CongElementType tag_type);
GList* xml_get_valid_previous_sibling(CongDispspec* ds, CongNodePtr node, enum CongElementType tag_type);
GList* xml_get_valid_next_sibling(CongDispspec* ds, CongNodePtr node, enum CongElementType tag_type);

#if 0
TTREE *get_upper_section(TTREE *x);
#endif

char *tag_new_pick();

void open_document(GtkWindow *parent_window);
gint save_document(CongDocument *doc, GtkWindow *parent_window);
gint save_document_as(CongDocument *doc, GtkWindow *parent_window);



gchar *cong_get_file_name(const gchar *title, 
			  const gchar *filename,
			  GtkWindow *parent_window);

char *pick_structural_tag(CongDispspec *ds);

void open_document_do(const gchar *doc_name, GtkWindow *parent_window);

/* DHM: My new stuff goes here for now: */
#define UNUSED_VAR(x)

int gui_window_new_document_make();

struct xview *xmlview_new(CongDocument *doc);
void xmlview_destroy(int free_xml);

void col_to_gcol(GdkColor *gcol, unsigned int col);
void cong_span_editor_redraw(CongSpanEditor *xed);

void cong_cursor_init(CongCursor *curs, CongDocument *doc);
void cong_cursor_uninit(CongCursor *curs);
void cong_cursor_on(CongCursor *curs);
void cong_cursor_off(CongCursor *curs);
#if !USE_CONG_EDITOR_WIDGET
void cong_cursor_place_in_xed(CongCursor *curs, CongSpanEditor *xed, int x, int y);
#endif
gint cong_cursor_data_insert(CongCursor *curs, char *s);
int cong_cursor_paragraph_insert(CongCursor *curs);
gboolean cong_cursor_calc_prev_char(CongCursor *curs, CongDocument *doc, CongLocation *output_loc);
gboolean cong_cursor_calc_next_char(CongCursor *curs, CongDocument *doc, CongLocation *output_loc);
#if !USE_CONG_EDITOR_WIDGET
void cong_cursor_prev_line(CongCursor *curs, CongSpanEditor *xed);
#endif
void cong_cursor_next_line(CongCursor *curs, CongSpanEditor *xed);
void cong_cursor_del_prev_char(CongCursor *curs, CongDocument *doc);
void cong_cursor_del_next_char(CongCursor *curs, CongDocument *doc);
void cong_cursor_home(CongCursor *curs, CongDocument *doc);
void cong_cursor_end(CongCursor *curs, CongDocument *doc);

void cong_selection_init(CongSelection *selection);
void cong_selection_import(CongSelection *selection, GtkWidget* widget);
void cong_selection_draw(CongSelection *selection, CongCursor *curs);
void cong_selection_start_from_curs(CongSelection *selection, CongCursor *curs);
void cong_selection_end_from_curs(CongSelection *selection, CongCursor *curs);

/* Popup (context) menus for editor view: */
void editor_popup_show(GtkWidget *widget, GdkEventButton *bevent);
void editor_popup_build(CongDocument *doc, GtkWindow *parent_window);
void editor_popup_init();

/* Popup (context) menus for tree view: */
GtkWidget* tree_popup_init(CongTreeView *cong_tree_view, CongNodePtr x, GtkWindow *parent_window);
gint tree_popup_show(GtkWidget *widget, GdkEvent *event);

/* dialog to select from a list of string */
gchar* string_selection_dialog(gchar *title, gchar *element_description, GList *elements);

void xv_style_r(GtkWidget *widget, gpointer data);

CongDispspec* query_for_forced_dispspec(gchar *what_failed, xmlDocPtr doc, GtkWindow* parent_window);

/**
 * A routine that tries to syncronously load a file into a buffer in memory (surely this exists already somewhere?)
 * (I believe that CVS gnome-vfs has a routine gnome_vfs_read_entire_file that does this)
*/
GnomeVFSResult
cong_vfs_new_buffer_from_file(const char* filename, char** buffer, GnomeVFSFileSize* size);

/**
 * A routine that tries to syncronously load a file into a buffer in memory (surely this exists already somewhere?)
*/
GnomeVFSResult
cong_vfs_new_buffer_from_uri(GnomeVFSURI* uri, char** buffer, GnomeVFSFileSize* size);

/* Toolbar hooks: */
gint toolbar_callback_open(GtkWidget *widget, gpointer data);
gint toolbar_callback_save_as(GtkWidget *w, gpointer data);
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

/* Experimental new implementation of the editor as a custom widget; to be a fully MVC view from the beginning; currently it's a GtkDrawingArea */
typedef GtkDrawingArea CongEditorWidget;
typedef struct CongElementEditor CongElementEditor;

GtkWidget *cong_editor_widget_new(CongDocument *doc);
CongDocument *cong_editor_widget_get_document(CongEditorWidget *editor_widget);
CongDispspec *cong_editor_widget_get_dispspec(CongEditorWidget *editor_widget);
void cong_editor_widget_force_layout_update(CongEditorWidget *editor_widget);
#define CONG_EDITOR_WIDGET(x) ((CongEditorWidget*)(x))

/* PLUGIN INTERFACE: 
   These types are fully opaque, to try to minimise ABI issues.
*/
typedef struct CongPlugin CongPlugin;
typedef struct CongPluginManager CongPluginManager;

typedef struct CongFunctionality CongFunctionality;
#define CONG_FUNCTIONALITY(x) ((CongFunctionality*)(x))

/* The following are all castable to CongFunctionality: */
typedef struct CongDocumentFactory CongDocumentFactory;
typedef struct CongImporter CongImporter;
typedef struct CongExporter CongExporter;
typedef struct CongPrintMethod CongPrintMethod;
typedef struct CongThumbnailer CongThumbnailer;
typedef struct CongPluginEditorElement CongPluginEditorElement;

/* The File->New GUI: */
typedef struct CongNewFileAssistant CongNewFileAssistant;


/* Function pointers to be exposed by .so/.dll files: */
typedef gboolean (*CongPluginCallbackInit)(CongPlugin *plugin); /* exposed as "plugin_init"? */
typedef gboolean (*CongPluginCallbackUninit)(CongPlugin *plugin); /* exposed as "plugin_uninit"? */
typedef gboolean (*CongPluginCallbackRegister)(CongPlugin *plugin); /* exposed as "plugin_register"? */
typedef gboolean (*CongPluginCallbackConfigure)(CongPlugin *plugin);  /* exposed as "plugin_configure"? legitimate for it not to be present */

/* Function pointers that are registered by plugins: */
typedef void (*CongDocumentFactoryPageCreationCallback)(CongDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data);
typedef void (*CongDocumentFactoryActionCallback)(CongDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data);
typedef gboolean (*CongImporterMimeFilter)(CongImporter *importer, const gchar *mime_type, gpointer user_data);
typedef void (*CongImporterActionCallback)(CongImporter *importer, const gchar *uri, const gchar *mime_type, gpointer user_data, GtkWindow *toplevel_window);
typedef gboolean (*CongExporterFpiFilter)(CongExporter *exporter, const gchar *fpi, gpointer user_data);
typedef void (*CongExporterActionCallback)(CongExporter *exporter, CongDocument *doc, const gchar *uri, gpointer user_data, GtkWindow *toplevel_window);
typedef gboolean (*CongPrintMethodFpiFilter)(CongPrintMethod *print_method, const gchar *fpi, gpointer user_data);
typedef void (*CongPrintMethodActionCallback)(CongPrintMethod *print_method, CongDocument *doc, GnomePrintContext *gpc, gpointer user_data, GtkWindow *toplevel_window);
typedef CongElementEditor* (*CongEditorElementFactoryMethod)(CongPluginEditorElement *plugin_editor_element, CongEditorWidget *editor_widget, CongNodePtr node, gpointer user_data);


/* The globals: */
struct CongGlobals
{
	GnomeProgram *gnome_program;

	CongPluginManager *plugin_manager;

	GList *primary_windows;

	CongFont *fonts[CONG_FONT_ROLE_NUM];

	GdkGC *insert_element_gc;

	CongNodePtr clipboard;

	CongDispspecRegistry* ds_registry;

	GtkWidget *popup;

	GConfClient* gconf_client;
};

extern struct CongGlobals the_globals;

void cong_menus_create_items(GtkItemFactory *item_factory, 
			     CongPrimaryWindow *primary_window);

void do_node_heading_context_menu(CongDocument *doc, CongNodePtr node);

/* UI routines for invocation by menus/toolbars: */
void
cong_ui_file_import(GtkWindow *toplevel_window);

void
cong_ui_file_export(CongDocument *doc,
		    GtkWindow *toplevel_window);

GnomeVFSResult
cong_xml_save_to_vfs(xmlDocPtr doc_ptr, 
		     GnomeVFSURI *file_uri,	
		     GnomeVFSFileSize *output_file_size);

/* Extensions to libxml: */
xmlAttrPtr	xmlNewProp_NUMBER	(xmlNodePtr node,
					 const xmlChar *name,
					 int value);

/* Handy utility functions: */

/**
   Dave Malcolm:
   Norman Walsh's stylesheets for DocBook seem to be present on every modern Linux
   distribution I've tried.  Unfortunately, there doesn't yet seem to
   be a standard about where they should be installed.

   The functions are currently hardcoded to assume Red Hat 8, and can be hacked to support SuSE 7.1

   If you have another Linux distro, or one of the BSDs, or Solaris etc, then please let the conglomerate-devel mailing list
   know where the standard location is (hopefully we can agree on a standard for this!)

   These functions hide this problem:
*/
const gchar*
cong_utils_get_norman_walsh_stylesheet_path(void);

gchar*
cong_utils_get_norman_walsh_stylesheet(const gchar *stylesheet_relative_path);

G_END_DECLS
