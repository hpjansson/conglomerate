/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <libxml/tree.h>
#include <gtk/gtk.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>


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
#define USE_PANGO 0

enum CongNodeType
{
	CONG_NODE_TYPE_UNKNOWN,
	CONG_NODE_TYPE_ELEMENT,
	CONG_NODE_TYPE_TEXT,
	CONG_NODE_TYPE_COMMENT,

	CONG_NODE_TYPE_NUM
};


enum CongElementType
{
	CONG_ELEMENT_TYPE_STRUCTURAL,
	CONG_ELEMENT_TYPE_SPAN,
	CONG_ELEMENT_TYPE_INSERT,

	CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE,

	CONG_ELEMENT_TYPE_PARAGRAPH,

	/* Other types?  Table? Plugin widget/Bonobo control? */

	CONG_ELEMENT_TYPE_UNKNOWN
};

typedef struct CongDocument CongDocument;
typedef struct CongView CongView;
typedef struct CongViewClass CongViewClass;
typedef struct CongDispspec CongDispspec;
typedef struct CongDispspecElement CongDispspecElement;
typedef struct CongDispspecElementHeaderInfo CongDispspecElementHeaderInfo;
typedef struct CongDispspecRegistry CongDispspecRegistry;

typedef struct CongFont CongFont;

typedef struct CongCursor CongCursor;
typedef struct CongSelection CongSelection;
typedef struct CongPrimaryWindow CongPrimaryWindow;
typedef struct CongTreeView CongTreeView;
typedef struct CongEditorView CongEditorView;
typedef struct CongSpanEditor CongSpanEditor; 

typedef xmlNodePtr CongNodePtr;
typedef xmlChar CongXMLChar;

const char* cong_node_name(CongNodePtr node);
CongNodePtr cong_node_prev(CongNodePtr node);
CongNodePtr cong_node_next(CongNodePtr node);
CongNodePtr cong_node_first_child(CongNodePtr node);
CongNodePtr cong_node_parent(CongNodePtr node);

enum CongNodeType cong_node_type(CongNodePtr node);

/* Handy debug method for writing log info: */
const gchar *cong_node_type_description(enum CongNodeType node_type);

/* Methods for accessing attribute values: */
CongXMLChar* cong_node_get_attribute(CongNodePtr node, const CongXMLChar* attribute_name);
/* caller responsible for frreing; will be NULL if not found in node and no default in DTD available */

/* Selftest methods: */
void cong_node_self_test(CongNodePtr node);
void cong_node_self_test_recursive(CongNodePtr node);

void cong_node_recursive_delete(CongDocument *doc, CongNodePtr node);
CongNodePtr cong_node_recursive_dup(CongNodePtr node);


#if 1
#define CONG_NODE_SELF_TEST(node) cong_node_self_test(node)
#else
#define CONG_NODE_SELF_TEST(node) ((void)0)
#endif

int cong_node_get_length(CongNodePtr node); /* get length of content; does not include the zero terminator (to correspond to the TTREE size field) */

/* Construction: */
CongNodePtr cong_node_new_element(const char *tagname);
CongNodePtr cong_node_new_text(const char *text);
CongNodePtr cong_node_new_text_len(const char *text, int len); /* FIXME: what character type ? */

/* Destruction: (the node has to have been unlinked from the tree already): */
void cong_node_free(CongNodePtr node);

/* 
   Direct tree manipulation; these functions are deprecated in favour of the cong_document_ versions below, which, in turn,
   will get converted to an apporach involving atomic and compound modification objects, which will give us Undo/Redo
*/
void cong_node_make_orphan(CongNodePtr node);
void cong_node_add_after(CongNodePtr node, CongNodePtr older_sibling);
void cong_node_add_before(CongNodePtr node, CongNodePtr younger_sibling);
void cong_node_set_parent(CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
void cong_node_set_text(CongNodePtr node, const xmlChar *new_content);

/**
   Struct representing a location within a document, with both a node ptr and a character offset into the text.
 */
typedef struct _CongLocation
{
	CongNodePtr tt_loc;
	int char_loc;
} CongLocation;

void
cong_location_set(CongLocation *loc, CongNodePtr tt, int offset);

void
cong_location_nullify(CongLocation *loc);

gboolean
cong_location_exists(CongLocation *loc);

gboolean
cong_location_equals(const CongLocation *loc0, const CongLocation *loc1);

enum CongNodeType
cong_location_node_type(CongLocation *loc);

char
cong_location_get_char(CongLocation *loc);

CongNodePtr
cong_location_xml_frag_data_nice_split2(CongDocument *doc, CongLocation *loc);

void
cong_location_insert_chars(CongDocument *doc, CongLocation *loc, const char* s);

void
cong_location_del_next_char(CongDocument *doc, CongLocation *loc);

CongNodePtr
cong_location_xml_frag_prev(CongLocation *loc);

CongNodePtr
cong_location_xml_frag_next(CongLocation *loc);

CongNodePtr
cong_location_node(CongLocation *loc);

CongNodePtr
cong_location_parent(CongLocation *loc);

void
cong_location_copy(CongLocation *dst, const CongLocation *src);

/**
   CongDocument functions
 */

/* takes ownership of xml_doc */
CongDocument*
cong_document_new_from_xmldoc(xmlDocPtr xml_doc, CongDispspec *ds, const gchar *url);

void
cong_document_delete(CongDocument *doc);

CongNodePtr
cong_document_get_root(CongDocument *doc);

CongDispspec*
cong_document_get_dispspec(CongDocument *doc);

gchar*
cong_document_get_filename(CongDocument *doc);
/* caller is responsible for freeeing */

gchar*
cong_document_get_parent_uri(CongDocument *doc);
/* caller is responsible for freeeing */

void
cong_document_save(CongDocument *doc, const char* filename);

gboolean
cong_document_is_modified(CongDocument *doc);

void
cong_document_set_modified(CongDocument *doc, gboolean modified);

void
cong_document_set_primary_window(CongDocument *doc, CongPrimaryWindow *window);

/* MVC-related methods on the document: */
void cong_document_coarse_update(CongDocument *doc);
void cong_document_node_make_orphan(CongDocument *doc, CongNodePtr node);
void cong_document_node_add_after(CongDocument *doc, CongNodePtr node, CongNodePtr older_sibling);
void cong_document_node_add_before(CongDocument *doc, CongNodePtr node, CongNodePtr younger_sibling);
void cong_document_node_set_parent(CongDocument *doc, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
void cong_document_node_set_text(CongDocument *doc, CongNodePtr node, const xmlChar *new_content);
void cong_document_tag_remove(CongDocument *doc, CongNodePtr x);


void cong_document_register_view(CongDocument *doc, CongView *view);
void cong_document_unregister_view(CongDocument *doc, CongView *view);

/* cursor and selections are now properties of the document: */
CongCursor* cong_document_get_cursor(CongDocument *doc);
CongSelection* cong_document_get_selection(CongDocument *doc);

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
	   GObject framework
	*/
	void (*on_document_coarse_update)(CongView *view);
	void (*on_document_node_make_orphan)(CongView *view, CongNodePtr node);
	void (*on_document_node_add_after)(CongView *view, CongNodePtr node, CongNodePtr older_sibling);
	void (*on_document_node_add_before)(CongView *view, CongNodePtr node, CongNodePtr younger_sibling);
	void (*on_document_node_set_parent)(CongView *view, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
	void (*on_document_node_set_text)(CongView *view, CongNodePtr node, const xmlChar *new_content);
};


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

#if USE_PANGO
	/* Experimental Pango support */
	PangoLayout *pango_layout;
#endif

};


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
	CongSpanEditor *xed;
	GtkWidget *w;
	GdkGC *gc;
	int x, y;
	int line;
	int on;

	/* Conceptual location */
	CongLocation location;

	int set;

	guint timeout_id;
};


struct CongSelection
{
	CongSpanEditor *xed;
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

struct CongFont
{
	GdkFont *gdk_font;
	int asc;
	int desc;
	
};


CongPrimaryWindow *cong_primary_window_new(CongDocument *doc);
void cong_primary_window_free(CongPrimaryWindow *primary_window);
void cong_primary_window_update_title(CongPrimaryWindow *primary_window);

CongTreeView *cong_tree_view_new(CongDocument *doc);
void cong_tree_view_free(CongTreeView *tree_view);
GtkWidget* cong_tree_view_get_widget(CongTreeView *tree_view);
GtkTreeStore* cong_tree_view_get_tree_store(CongTreeView *tree_view);

CongEditorView *cong_editor_view_new(CongDocument *doc);
void cong_editor_view_free(CongEditorView *editor_view);
GtkWidget* cong_editor_view_get_widget(CongEditorView *editor_view);

struct CongGlobals
{
	GnomeProgram *gnome_program;

	GList *primary_windows;

	CongFont *fonts[CONG_FONT_ROLE_NUM];

	GdkGC *insert_element_gc;

	CongNodePtr clipboard;

	CongDispspecRegistry* ds_registry;

	GtkWidget *popup;

#if USE_PANGO
	PangoContext *pango_context;
	PangoFontDescription *pango_font_description;
	PangoFont*  pango_font;
#endif
};

extern struct CongGlobals the_globals;

CongFont*
cong_font_load(const gchar *font_name);

void
cong_font_delete(CongFont *font);

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
int new_document();
int find_document();
int find_documentmetacaps();
int submit_do();
int gui_window_login_make(char **user_s, char **pass_s);

CongSpanEditor *xmledit_new(CongNodePtr x, CongDocument *doc, CongDispspec *displayspec);

CongFont*
cong_span_editor_get_font(CongSpanEditor *xed, enum CongFontRole role);

gint tree_new_sibling(GtkWidget *widget, CongNodePtr tag);
gint tree_new_sub_element(GtkWidget *widget, CongNodePtr tag);
gint tree_cut(GtkWidget *widget, CongNodePtr tag);
gint tree_copy(GtkWidget *widget, CongNodePtr tag);
gint tree_paste_under(GtkWidget *widget, CongNodePtr tag);
gint tree_paste_before(GtkWidget *widget, CongNodePtr tag);
gint tree_paste_after(GtkWidget *widget, CongNodePtr tag);

void cong_span_editor_cut(CongSpanEditor *span_editor);
void cong_span_editor_copy(CongSpanEditor *span_editor);
void cong_span_editor_paste(CongSpanEditor *span_editor, GtkWidget *widget);

gint xed_cut(GtkWidget *widget, struct CongSpanEditor *xed);
gint xed_copy(GtkWidget *widget, struct CongSpanEditor *xed);
gint xed_paste(GtkWidget *widget, struct CongSpanEditor *xed);

gint insert_meta_hook(GtkWidget *w, struct CongSpanEditor *xed);
gint insert_media_hook(GtkWidget *w, struct CongSpanEditor *xed);
gint select_vector(GtkWidget *w);

gint xed_insert_table(GtkWidget *w, struct CongSpanEditor *xed);

const char *xml_frag_data_nice(CongNodePtr x);
const char *xml_frag_name_nice(CongNodePtr x);

#if 0
SOCK *server_login();
#endif

struct pos *pos_physical_to_logical(struct CongSpanEditor *xed, int x, int y);
struct pos *pos_logical_to_physical(struct CongSpanEditor *xed, CongNodePtr node, int c);
struct pos *pos_logical_to_physical_new(struct CongSpanEditor *xed, CongLocation *loc);

CongNodePtr xml_frag_data_nice_split3(CongDocument *doc, CongNodePtr s, int c0, int c1);
CongNodePtr xml_frag_data_nice_split2(CongDocument *doc, CongNodePtr s, int c);

CongNodePtr cong_selection_reparent_all(CongSelection *selection, CongNodePtr p);
CongNodePtr xml_inner_span_element(CongDispspec *ds, CongNodePtr x);
CongNodePtr xml_outer_span_element(CongDispspec *ds, CongNodePtr x);
char *xml_fetch_clean_data(CongNodePtr x);

#if 0
TTREE *get_upper_section(TTREE *x);
#endif

char *tag_new_pick();

#if 0
GtkWidget *gui_metachoice_box(TTREE *choices, int many, int selectability, GtkSignalFunc *sig_func, char *id);
GtkWidget *gui_taxochoice_box(TTREE *choices, int many, int selectability, GtkSignalFunc *sig_func, char *id);
#endif

gint open_document(GtkWidget *w, gpointer data);
gint save_document(GtkWidget *w, gpointer data);

const char *get_file_name(char *title);
char *pick_structural_tag(CongDispspec *ds);

void open_document_do(const gchar *doc_name);

/* DHM: My new stuff goes here for now: */
#define UNUSED_VAR(x)

int gui_window_new_document_make();

struct xview *xmlview_new(CongDocument *doc);
void xmlview_destroy(int free_xml);

CongDispspec* cong_dispspec_new_from_ds_file(const char *name);
GnomeVFSResult cong_dispspec_new_from_xds_file(GnomeVFSURI *uri, CongDispspec** ds);
CongDispspec* cong_dispspec_new_from_xds_buffer(const char *buffer, size_t size);
CongDispspec* cong_dispspec_new_from_xml_file(xmlDocPtr doc);
void cong_dispspec_delete(CongDispspec *dispspec);

const gchar*
cong_dispspec_get_name(const CongDispspec *ds);

const gchar*
cong_dispspec_get_description(const CongDispspec *ds);

#if 0
char *cong_dispspec_name_name_get(CongDispspec *ds, TTREE *t);
#endif

#if NEW_LOOK
enum CongDispspecGCUsage
{
	CONG_DISPSPEC_GC_USAGE_BOLD_LINE,
	CONG_DISPSPEC_GC_USAGE_DIM_LINE,
	CONG_DISPSPEC_GC_USAGE_BACKGROUND,
	CONG_DISPSPEC_GC_USAGE_TEXT,

	CONG_DISPSPEC_GC_USAGE_NUM
};
GdkGC *cong_dispspec_gc_get(CongDispspec *ds, CongNodePtr x, enum CongDispspecGCUsage usage);
#else
#if 0
GdkGC *cong_dispspec_name_gc_get(CongDispspec *ds, TTREE *t, int tog);
#endif
GdkGC *cong_dispspec_gc_get(CongDispspec *ds, CongNodePtr x, int tog);
#endif
const char *cong_dispspec_name_get(CongDispspec *ds, CongNodePtr x);

gboolean cong_dispspec_element_structural(CongDispspec *ds, const char *name);
gboolean cong_dispspec_element_collapse(CongDispspec *ds, const char *name);
gboolean cong_dispspec_element_span(CongDispspec *ds, const char *name);
gboolean cong_dispspec_element_insert(CongDispspec *ds, const char *name);

enum CongElementType
cong_dispspec_type(CongDispspec *ds, const char* tagname);

/* New API for getting at elements within a dispspec */
CongDispspecElement*
cong_dispspec_lookup_element(const CongDispspec *ds, const char* tagname);

CongDispspecElement*
cong_dispspec_lookup_node(const CongDispspec *ds, CongNodePtr node);

CongDispspecElement*
cong_dispspec_get_first_element(CongDispspec *ds);

/* Will return NULL if no such tag exists */
CongDispspecElement*
cong_dispspec_get_paragraph(CongDispspec *ds);

/** Get the tagname in a parser-friendly form */
const char*
cong_dispspec_element_tagname(CongDispspecElement* element);

/** Get the name in a user-friendly form */
const char*
cong_dispspec_element_username(CongDispspecElement* element);

const char*
cong_dispspec_element_name_name_get(CongDispspecElement* element);

CongDispspecElement*
cong_dispspec_element_next(CongDispspecElement* element);

enum CongElementType
cong_dispspec_element_type(CongDispspecElement *element);

gboolean
cong_dispspec_element_collapseto(CongDispspecElement *element);

gboolean
cong_dispspec_element_is_structural(CongDispspecElement *element);

gboolean
cong_dispspec_element_is_span(CongDispspecElement *element);

unsigned int
cong_dispspec_element_color(CongDispspecElement *element);

#if NEW_LOOK
GdkGC*
cong_dispspec_element_gc(CongDispspecElement *element, enum CongDispspecGCUsage usage);

const GdkColor*
cong_dispspec_element_col(CongDispspecElement *element, enum CongDispspecGCUsage usage);
#else
GdkGC*
cong_dispspec_element_gc(CongDispspecElement *element);

const GdkColor*
cong_dispspec_element_col(CongDispspecElement *element);
#endif

CongDispspecElementHeaderInfo*
cong_dispspec_element_header_info(CongDispspecElement *element);

gchar*
cong_dispspec_get_section_header_text(CongDispspec *ds, CongNodePtr x);

CongFont*
cong_dispspec_element_get_font(CongDispspecElement *element, enum CongFontRole role);

void col_to_gcol(GdkColor *gcol, unsigned int col);
void cong_span_editor_redraw(CongSpanEditor *xed);

void cong_cursor_init(CongCursor *curs);
void cong_cursor_on(CongCursor *curs);
void cong_cursor_off(CongCursor *curs);
void cong_cursor_place_in_xed(CongCursor *curs, CongSpanEditor *xed, int x, int y);
gint cong_cursor_data_insert(CongCursor *curs, char *s);
int cong_cursor_paragraph_insert(CongCursor *curs);
void cong_cursor_prev_char(CongCursor *curs, CongSpanEditor *xed);
void cong_cursor_next_char(CongCursor *curs, CongSpanEditor *xed);
void cong_cursor_prev_line(CongCursor *curs, CongSpanEditor *xed);
void cong_cursor_next_line(CongCursor *curs, CongSpanEditor *xed);
void cong_cursor_del_prev_char(CongCursor *curs, CongSpanEditor *xed);
void cong_cursor_del_next_char(CongCursor *curs, CongSpanEditor *xed);

void cong_selection_init(CongSelection *selection);
void cong_selection_import(CongSelection *selection, GtkWidget* widget);
void cong_selection_draw(CongSelection *selection, CongCursor *curs);
void cong_selection_start_from_curs(CongSelection *selection, CongCursor *curs);
void cong_selection_end_from_curs(CongSelection *selection, CongCursor *curs);

void popup_show(GtkWidget *widget, GdkEventButton *bevent);
void popup_build(CongSpanEditor *xed);
void popup_init();

GtkWidget* tpopup_init(CongTreeView *cong_tree_view, CongNodePtr x);
gint tpopup_show(GtkWidget *widget, GdkEvent *event);

void xv_style_r(GtkWidget *widget, gpointer data);

const CongDispspec*
get_appropriate_dispspec(xmlDocPtr doc);

CongDispspec* query_for_forced_dispspec(gchar *what_failed, xmlDocPtr doc);

/* 
   Error handling facilities: 

   Although these have a "cong" prefix, I hope to eventually factor these out into a useful library for other apps to use (the prefix will then get changed).
*/

/**
 * Routine to manufacture a GNOME HIG-compliant error dialog.
 */
GtkDialog* 
cong_error_dialog_new(const gchar* what_failed, 
		      const gchar* why_failed, 
		      const gchar* suggestions);

/**
 * Routine to manufacture a GNOME HIG-compliant error dialog with a "convenience button" that does something relevant.
 */
GtkDialog* 
cong_error_dialog_new_with_convenience(const gchar* what_failed, 
				       const gchar* why_failed, 
				       const gchar* suggestions,
				       const gchar* convenience_label,
				       void (*convenience_action)(gpointer data),
				       gpointer convenience_data);

/**
 * Routine to run an error dialog.  Use in preference to gtk_dialog_run as it handles convenience buttons.
 */
void
cong_error_dialog_run(GtkDialog* dialog);

/**
 * Routine to run an error dialog and destroy it afterwards.  Use in preference to gtk_dialog_run as it handles convenience buttons.
 */
void
cong_error_dialog_do(GtkDialog* dialog);

/**
 * Routine to get at the application name in a form suitable for use in error reports.  Returns a freshly-allocated string.
 */
gchar* 
cong_error_get_appname(void);

/**
 * Routine to manufacture an error dialog for unimplemented functionality
 */
GtkDialog*
cong_error_dialog_new_unimplemented(const gchar* what_failed, const char* filename, int linenum);

#define CONG_DO_UNIMPLEMENTED_DIALOG(what_failed) (cong_error_dialog_do(cong_error_dialog_new_unimplemented(what_failed, __FILE__, __LINE__)))

/**
 * Routine to manufacture a "what failed" string for when File->Open fails.
 * @vfs_uri:  the URI from which you tried to open the file.
 */
gchar*
cong_error_what_failed_on_file_open_failure(const GnomeVFSURI* file_uri, gboolean transient);

/**
 * Routine to manufacture an error dialog for when File->Open fails.
 * @vfs_uri:  the URI from which you tried to open the file.
 */
GtkDialog*
cong_error_dialog_new_file_open_failed(const GnomeVFSURI* file_uri, gboolean transient, const gchar* why_failed, const gchar* suggestions);

/**
 * Routine to manufacture an error dialog for when File->Open fails.
 * @vfs_uri:  the URI to which you tried to save the file.
 */
GtkDialog*
cong_error_dialog_new_file_open_failed_with_convenience(const GnomeVFSURI* file_uri, 
							gboolean transient, 
							const gchar* why_failed, 
							const gchar* suggestions,
							const gchar* convenience_label,
							void (*convenience_action)(gpointer data),
							gpointer convenience_data);


/**
 * Routine to manufacture an error dialog for when File->Open fails.
 * @vfs_uri:  the URI to which you tried to save the file.
 * @vfs_result: the error code that occurred.
 */
GtkDialog*
cong_error_dialog_new_file_open_failed_from_vfs_result(const GnomeVFSURI* file_uri, GnomeVFSResult vfs_result);


/**
 * Routine to manufacture an error dialog for when File->Save (or File->Save as...) fails.
 * @vfs_uri:  the URI to which you tried to save the file.
 * @vfs_result: the error code that occurred.
 * @file_size: pointer to the size of the file if known, or NULL if not (useful if the error was due to lack of space)
 */
GtkDialog*
cong_error_dialog_new_file_save_failed(const GnomeVFSURI* file_uri, GnomeVFSResult vfs_result, const GnomeVFSFileSize* file_size);

/** 
 * A bunch of self-tests.
 */
void
cong_error_tests(void);

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

/* cong-dispspec-registry */
CongDispspecRegistry*
cong_dispspec_registry_new(const gchar* xds_directory);

void
cong_dispspec_registry_free(CongDispspecRegistry* registry);

unsigned int
cong_dispspec_registry_get_num(CongDispspecRegistry* registry);

const CongDispspec*
cong_dispspec_registry_get(CongDispspecRegistry* registry, unsigned int i);

void
cong_dispspec_registry_add(CongDispspecRegistry* registry, CongDispspec* ds);

void
cong_dispspec_registry_dump(CongDispspecRegistry* registry);

/* Menu hooks: */
void open_document_wrap(GtkWidget *widget, gpointer data);
void save_document_wrap(GtkWidget *widget, gpointer data);

void xed_cut_wrap(GtkWidget *widget, gpointer data);
void xed_copy_wrap(GtkWidget *widget, gpointer data);
void xed_paste_wrap(GtkWidget *widget, gpointer data);

void test_open_wrap(GtkWidget *widget, gpointer data);
void test_error_wrap(GtkWidget *widget, gpointer data);
void test_document_types_wrap(GtkWidget *widget, gpointer data);
void test_transform_wrap(GtkWidget *widget, gpointer data);
