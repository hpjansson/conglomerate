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


/* Include these here to help Cygwin a bit */

struct xed
{
  /* Display information */

	GtkWidget *e;  /* Eventbox */
  GtkWidget *w;  /* Drawing area */
  GdkPixmap *p;  /* Backing pixmap */
	GdkFont *f, *fm;

	TTREE *displayspec;
	
	int f_asc, f_desc;
	int fm_asc, fm_desc;
	int tag_height;
	
	int initial;
	int already_asked_for_size;

	/* Drawing information (temporary) */

	int mode;

	TTREE *draw_x;      /* XML node currently at */
	int draw_char;      /* Char to begin drawing (in node) */

	TTREE *draw_x_prev;
	int draw_char_prev;

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
  TTREE *x;
	
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
	TTREE *t;
	int c;

	int set;
};


struct selection
{
	struct xed *xed;
	GdkGC *gc_0, *gc_1, *gc_2, *gc_3;  /* 0 is brightest, 3 is darkest */

	int x0, y0, x1, y1;

	TTREE *t0, *t1;
	int c0, c1;
};


struct cong_globals
{
  struct xview *xv;
  struct curs curs;
  struct selection selection;

  GdkFont *f, *fm, *ft;
  GdkGC *insert_element_gc;
  int f_asc, f_desc, fm_asc, fm_desc, ft_asc, ft_desc;
  
  TTREE *ds_global;
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

struct xview *xmlview_new(TTREE *x, TTREE *displayspec);

#if 0
SOCK *server_login();
#endif

struct pos *pos_physical_to_logical(struct xed *xed, int x, int y);
struct pos *pos_logical_to_physical(struct xed *xed, TTREE *node, int c);

TTREE *xml_frag_data_nice_split3(TTREE *s, int c0, int c1);
TTREE *xml_frag_data_nice_split2(TTREE *s, int c);

TTREE *selection_reparent_all(struct selection* selection, TTREE *p);
TTREE *xml_inner_span_element(TTREE *x);
TTREE *xml_outer_span_element(TTREE *x);
char *xml_fetch_clean_data(TTREE *x);

char *ds_name_name_get(TTREE *t);
GdkGC *ds_name_gc_get(TTREE *ds, TTREE *t, int tog);
GdkGC *ds_gc_get(TTREE *ds, TTREE *x, int tog);
char *ds_name_get(TTREE *x);
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

int ds_element_structural(TTREE *ds, char *name);
int ds_element_collapse(TTREE *ds, char *name);
int ds_element_span(TTREE *ds, char *name);
int ds_element_insert(TTREE *ds, char *name);
TTREE *ds_get_first_structural(TTREE *ds);
TTREE *ds_get_next_structural(TTREE *prev);
TTREE *ds_get_first_span(TTREE *ds);
TTREE *ds_get_next_span(TTREE *prev);
void ds_init(TTREE *ds);


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
