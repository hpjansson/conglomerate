/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <ttree.h>
#include <sock.h>


#define RELEASE 1
#undef WINDOWS_BUILD

#ifdef WINDOWS_BUILD
# define isblank(c) ((c) == ' ' || (c) == '\n' || (c) == '\r' || (c) == '\t')
#endif

enum
{
   TITLE_COLUMN,
   TTREE_COLUMN,
   N_COLUMNS
};

#if 0
#if 0
typedef struct foo { TTREE* tt;} cong_node_ptr;
#else
typedef TTREE* cong_node_ptr;
#endif
#endif

enum CongElementType
{
	CONG_ELEMENT_TYPE_STRUCTURAL,
	CONG_ELEMENT_TYPE_SPAN,
	CONG_ELEMENT_TYPE_INSERT,

	CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE,

	/* Other types?  Table? Plugin widget/Bonobo control? */

	CONG_ELEMENT_TYPE_UNKNOWN
};

typedef struct CongDispspec CongDispspec;
typedef struct CongDispspecElement CongDispspecElement;


/**
   Struct representing a location within a document, with both a node ptr and a character offset into the text.
 */
typedef struct _cong_location
{
  TTREE *tt_loc;
  int char_loc;
} cong_location;

void
cong_location_set(cong_location *loc, TTREE *tt, int offset);

void
cong_location_nullify(cong_location *loc);

gboolean
cong_location_exists(cong_location *loc);

gboolean
cong_location_equals(const cong_location *loc0, const cong_location *loc1);

int
cong_location_frag_type(cong_location *loc);

char
cong_location_get_char(cong_location *loc);

TTREE*
cong_location_xml_frag_data_nice_split2(cong_location *loc);

void
cong_location_insert_chars(cong_location *loc, const char* s);

void
cong_location_del_next_char(cong_location *loc);

TTREE*
cong_location_xml_frag_prev(cong_location *loc);

TTREE*
cong_location_xml_frag_next(cong_location *loc);

TTREE*
cong_location_node(cong_location *loc);

TTREE*
cong_location_parent(cong_location *loc);

void
cong_location_copy(cong_location *dst, const cong_location *src);

/**
   Struct representing a document, in an effort to decouple the code from TTREE
 */
typedef struct _cong_document cong_document;

cong_document*
cong_document_new_from_ttree(TTREE *tt);

void
cong_document_delete(cong_document *doc);

TTREE*
cong_document_get_root(cong_document *doc);

void
cong_document_save(cong_document *doc, const char* filename);

/* Include these here to help Cygwin a bit */

struct xed
{
	/* Display information */

	GtkWidget *e;  /* Eventbox */
	GtkWidget *w;  /* Drawing area */
	GdkPixmap *p;  /* Backing pixmap */
	GdkFont *f, *fm;

	CongDispspec *displayspec;
	
	int f_asc, f_desc;
	int fm_asc, fm_desc;
	int tag_height;
	
	int initial;
	int already_asked_for_size;

	/* Drawing information (temporary) */

	int mode;

#if 0
	cong_location draw_loc;
	cong_location draw_prev;
#else
	TTREE *draw_x;      /* XML node currently at */
	int draw_char;      /* Char to begin drawing (in node) */

	TTREE *draw_x_prev;
	int draw_char_prev;
#endif

	TTREE *draw_line_t;
	int draw_line;      /* Line to draw at (in display) */
	int draw_pos_x;     /* Pixel to start drawing at */
	int draw_pos_y;     /* Y position to start drawing at */
	int draw_tag_max;

	/* Data content */

	TTREE *x;
	TTREE *lines;
	TTREE *tags;

  /* Cursor information */

	TTREE *curs_x;      /* XML node currently at */
	int curs_char;      /* Cursor positioned after this char (in node) */
};


struct xview
{
	cong_document *doc;
	
  GtkWidget *w;
#if 1
#if 0
  GtkTreeView* treeview; /* the tree view */
  GtkTreeStore* treestore; /* the tree model */
#endif
#else
  GtkWidget *tree;
#endif
};


struct pos
{
  int x, y;
  int x_find;

  int line;
  TTREE *node;
  TTREE *node_last;
  TTREE *node_find;
  int c, c_given;
  int space;  /* 0 = have no space, 1 = have space */

  int word_width;

  int mode;  /* 0 = not found, 1 = found */
};



struct curs
{
	/* Visual representation */
	struct xed *xed;
	GtkWidget *w;
	GdkGC *gc;
	int x, y;
	int line;
	int on;

	/* Conceptual location */
	cong_location location;

	int set;
};


struct selection
{
	struct xed *xed;
	GdkGC *gc_0, *gc_1, *gc_2, *gc_3;  /* 0 is brightest, 3 is darkest */

	int x0, y0, x1, y1;

	cong_location loc0;
	cong_location loc1;
};


struct cong_globals
{
  struct xview *xv;
  struct curs curs;
  struct selection selection;

  GdkFont *f, *fm, *ft;
  GdkGC *insert_element_gc;
  int f_asc, f_desc, fm_asc, fm_desc, ft_asc, ft_desc;
  
  CongDispspec *ds;

  TTREE *vect_global;
  TTREE *medias_global;
  TTREE *class_global;
  TTREE *users_global;
  TTREE *user_data;
  struct xed *meta_xed;
  TTREE *clipboard;
  TTREE *insert_meta_section_tag;

  char *server, *user, *pass;

#if 0
  guint status_main_ctx;
#endif
};

extern struct cong_globals the_globals;
extern struct cong_gui the_gui;

GtkWidget* cong_gui_get_window(struct cong_gui* gui);
GtkWidget* cong_gui_get_popup(struct cong_gui* gui);
void cong_gui_set_popup(struct cong_gui* gui, GtkWidget* popup);
GtkWidget* cong_gui_get_button_submit(struct cong_gui* gui);
GtkTreeStore* cong_gui_get_tree_store(struct cong_gui* gui);
GtkTreeView* cong_gui_get_tree_view(struct cong_gui* gui);
GtkWidget* cong_gui_get_root(struct cong_gui* gui);
void cong_gui_destroy_tree_store(struct cong_gui* gui);

gint curs_blink();
int login();
int new_document();
int find_document();
int find_documentmetacaps();
int submit_do();
int gui_window_login_make(char **user_s, char **pass_s);
struct xed *xmledit_new();

gint tree_new_sibling(GtkWidget *widget, TTREE *tag);
gint tree_new_sub_element(GtkWidget *widget, TTREE *tag);
gint tree_cut(GtkWidget *widget, TTREE *tag);
gint tree_copy(GtkWidget *widget, TTREE *tag);
gint tree_paste_under(GtkWidget *widget, TTREE *tag);
gint tree_paste_before(GtkWidget *widget, TTREE *tag);
gint tree_paste_after(GtkWidget *widget, TTREE *tag);

gint xed_cut(GtkWidget *widget, struct xed *xed);
gint xed_copy(GtkWidget *widget, struct xed *xed);
gint xed_paste(GtkWidget *widget, struct xed *xed);

gint insert_meta_hook(GtkWidget *w, struct xed *xed);
gint insert_media_hook(GtkWidget *w, struct xed *xed);
gint select_vector(GtkWidget *w);

gint xed_insert_table(GtkWidget *w, struct xed *xed);

char *xml_frag_data_nice(TTREE *x);
char *xml_frag_name_nice(TTREE *x);

struct xview *xmlview_new(cong_document *doc, CongDispspec *displayspec);

#if 0
SOCK *server_login();
#endif

struct pos *pos_physical_to_logical(struct xed *xed, int x, int y);
struct pos *pos_logical_to_physical(struct xed *xed, TTREE *node, int c);
struct pos *pos_logical_to_physical_new(struct xed *xed, cong_location *loc);

TTREE *xml_frag_data_nice_split3(TTREE *s, int c0, int c1);
TTREE *xml_frag_data_nice_split2(TTREE *s, int c);

TTREE *selection_reparent_all(struct selection* selection, TTREE *p);
TTREE *xml_inner_span_element(TTREE *x);
TTREE *xml_outer_span_element(TTREE *x);
char *xml_fetch_clean_data(TTREE *x);

TTREE *get_upper_section(TTREE *x);

char *tag_new_pick();

GtkWidget *gui_metachoice_box(TTREE *choices, int many, int selectability, GtkSignalFunc *sig_func, char *id);
GtkWidget *gui_taxochoice_box(TTREE *choices, int many, int selectability, GtkSignalFunc *sig_func, char *id);

gint open_document(GtkWidget *w, gpointer data);
gint save_document(GtkWidget *w, gpointer data);

const char *get_file_name(char *title);
char *pick_structural_tag();

int open_document_do(const char *doc_name, const char *ds_name);

/* DHM: My new stuff goes here for now: */
#define UNUSED_VAR(x)

int gui_window_new_document_make();
void xmlview_destroy(int free_xml);

CongDispspec* cong_dispspec_new_from_ds_file(const char *name);
CongDispspec* cong_dispspec_new_from_xds_file(const char *name);
void cong_dispspec_delete(CongDispspec *dispspec);

char *cong_dispspec_name_name_get(TTREE *t);
GdkGC *cong_dispspec_name_gc_get(CongDispspec *ds, TTREE *t, int tog);
GdkGC *cong_dispspec_gc_get(CongDispspec *ds, TTREE *x, int tog);
char *cong_dispspec_name_get(TTREE *x);

gboolean cong_dispspec_element_structural(CongDispspec *ds, char *name);
gboolean cong_dispspec_element_collapse(CongDispspec *ds, char *name);
gboolean cong_dispspec_element_span(CongDispspec *ds, char *name);
gboolean cong_dispspec_element_insert(CongDispspec *ds, char *name);

enum CongElementType
cong_dispspec_type(CongDispspec *ds, const char* tagname);

/* New API for getting at elements within a dispspec */
CongDispspecElement*
cong_dispspec_lookup_element(CongDispspec *ds, const char* tagname);

CongDispspecElement*
cong_dispspec_get_first_element(CongDispspec *ds);

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

GdkGC*
cong_dispspec_element_gc(CongDispspecElement *element);

void col_to_gcol(GdkColor *gcol, unsigned int col);
void xed_redraw(struct xed *xed);

void curs_init(struct curs* curs);
void curs_on(struct curs* curs);
void curs_off(struct curs* curs);
void curs_place_in_xed(struct curs* curs, struct xed *xed, int x, int y);
gint curs_data_insert(struct curs* curs, char *s);
int curs_paragraph_insert(struct curs* curs);
void curs_prev_char(struct curs* curs, struct xed *xed);
void curs_next_char(struct curs* curs, struct xed *xed);
void curs_prev_line(struct curs* curs, struct xed *xed);
void curs_next_line(struct curs* curs, struct xed *xed);
void curs_del_prev_char(struct curs* curs, struct xed *xed);
void curs_del_next_char(struct curs* curs, struct xed *xed);

void selection_init(struct selection* selection);
void selection_import(struct selection* selection);
void selection_draw(struct selection* selection, struct curs* curs);
void selection_start_from_curs(struct selection* selection, struct curs* curs);
void selection_end_from_curs(struct selection* selection, struct curs* curs);

void popup_show(GtkWidget *widget, GdkEventButton *bevent);
void popup_build(struct xed *xed);
void popup_init();

#if 1
GtkWidget* tpopup_init(TTREE *x);
#else
void tpopup_init(GtkWidget *treeitem, TTREE *x);
#endif
gint tpopup_show(GtkWidget *widget, GdkEvent *event);

void xv_style_r(GtkWidget *widget, gpointer data);

/* Error handling facilities: */
GtkWidget* 
cong_error_dialog_new(const gchar* what_failed, 
		      const gchar* why_failed, 
		      const gchar* suggestions);

GtkWidget*
cong_error_dialog_new_file_open_failed(const gchar* filename);
/* should this be a URI? */

void
cong_error_tests(void);

