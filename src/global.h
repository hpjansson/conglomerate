#include <ttree.h>
#include <sock.h>


#define RELEASE 1
#undef WINDOWS_BUILD

#ifdef WINDOWS_BUILD
# define isblank(c) ((c) == ' ' || (c) == '\n' || (c) == '\r' || (c) == '\t')
#endif

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
	GtkWidget *tree;
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


extern struct xview *xv;
extern struct curs curs;
extern struct selection selection;

extern GtkWidget *window, *popup, *tree, *status, *root, *butt_new, *butt_submit, *butt_find;
extern GdkFont *f, *fm, *ft;
extern GdkGC *insert_element_gc;
extern int f_asc, f_desc, fm_asc, fm_desc, ft_asc, ft_desc;

extern TTREE *ds_global;
extern TTREE *vect_global;
extern TTREE *medias_global;
extern TTREE *class_global;
extern TTREE *users_global;
extern TTREE *user_data;
extern struct xed *meta_xed;
extern TTREE *clipboard;
extern TTREE *insert_meta_section_tag;

extern char *server, *user, *pass;

extern guint status_main_ctx;

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
SOCK *server_login();

struct pos *pos_physical_to_logical(struct xed *xed, int x, int y);
struct pos *pos_logical_to_physical(struct xed *xed, TTREE *node, int c);

TTREE *xml_frag_data_nice_split3(TTREE *s, int c0, int c1);
TTREE *xml_frag_data_nice_split2(TTREE *s, int c);

TTREE *selection_reparent_all(TTREE *p);
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

char *get_file_name(char *title);
char *pick_structural_tag();

int open_document_do(char *doc_name, char *ds_name);
