/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <stdlib.h>

/* Was using GTK_ENABLE_BROKEN to port over from old GtkTree to new GtkTreeView code */
/* #define GTK_ENABLE_BROKEN */

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "global.h"

GtkStyle *style_white;

#include <bonobo/bonobo-widget.h>

#if 0
#include <eel/eel-gdk-extensions.h>
#endif

/* Avoid introducing an eel dependency for now by copying the code from eel-gdk-extensions.c and making the functions static: */
static guint32
not_eel_rgb16_to_rgb (gushort r, gushort g, gushort b)
{
	guint32 result;

	result = (0xff0000 | (r & 0xff00));
	result <<= 8;
	result |= ((g & 0xff00) | (b >> 8));

	return result;
}

static guint32
not_eel_gdk_color_to_rgb (const GdkColor *color)
{
	return not_eel_rgb16_to_rgb (color->red, color->green, color->blue);
}

static char *
not_eel_gdk_rgb_to_color_spec (const guint32 color)
{
	return g_strdup_printf ("#%06X", (guint) (color & 0xFFFFFF));
}


gchar*
get_col_string(const GdkColor* col)
{

	guint32 col32;

	g_return_val_if_fail(col,NULL);

	col32 = not_eel_gdk_color_to_rgb (col);
	
	return not_eel_gdk_rgb_to_color_spec(col32);
}

/* Dodgy blend func: */
void blend_col(GdkColor *dst, const GdkColor *src0, const GdkColor *src1, float proportion)
{
	float one_minus = 1.0f - proportion;

	dst->red = ((float)src1->red*proportion) + ((float)src0->red*one_minus);
	dst->green = ((float)src1->green*proportion) + ((float)src0->green*one_minus);
	dst->blue = ((float)src1->blue*proportion) + ((float)src0->blue*one_minus);
}

static GdkColor white = {0, 0xffff, 0xffff, 0xffff};

/* Dodgy hack to do lines that blend to white: */
void draw_blended_line(GtkWidget *w,
		       const GdkColor *col,
		       int x0, int y0,
		       int x1)
{
	/* inefficient: claim a gc to do this! (ideally we'd just invoke the hardware... grrr... arrg...) */
	GdkGC *gc = gdk_gc_new(w->window);
	int x;
	float proportion;
	GdkColor blended_col;

	g_assert(x0!=x1);

	for (x=x0; x<x1; x++) {
		proportion = (float)(x-x0)/(float)(x1-x0);
		proportion = (proportion>0.5f)?((proportion-0.5f)*2.0f):0.0f;

		blend_col(&blended_col, col, &white, proportion);

		gdk_colormap_alloc_color(cong_gui_get_window(&the_gui)->style->colormap, &blended_col, FALSE, TRUE);

		gdk_gc_set_foreground(gc,&blended_col);

		gdk_draw_point(w->window, gc, x, y0);
	}

	gdk_gc_unref(gc);
}

/*
  We handle folding by showing/hiding all but the first child of the vbox at the root of a xv_section_head.
  We store a boolean "expanded" data on the vbox.
  We also store a "dispspec" ptr to the dispspec on the vbox.
  We store a ptr to the vbox within the title/GtkDrawingArea called "vbox" so it can query this when it draws itself.
  Probably would be cleaner to have a new widget subclass...
 */

#define V_SPACING (4)
#define H_SPACING (4)
#define H_INDENT (4)
#define FRAGMENT_WIDTH (45)

static gint xv_section_head_expose(GtkWidget *w, GdkEventExpose *event, CongNodePtr x)
{
	GdkGC *gc;
	int str_width;
#if !NEW_XML_IMPLEMENTATION
	TTREE *n0;
#endif
	gchar *title_text;

	GtkWidget* vbox = GTK_WIDGET(g_object_get_data(G_OBJECT(w), "vbox"));
	gboolean expanded = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(vbox), "expanded"));
	CongDispspec *ds = g_object_get_data(G_OBJECT(vbox), "dispspec");

	CongDispspecElement *element = cong_dispspec_lookup_element(ds, cong_node_name(x));
	CongFont *title_font;

	element = cong_dispspec_lookup_element(ds, cong_node_name(x));
	g_assert(element);

 	title_font = cong_dispspec_element_get_font(element, CONG_FONT_ROLE_TITLE_TEXT);
	g_assert(title_font);

#if NEW_LOOK
	gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE);
	g_assert(gc);

	/* Draw the frame rectangle "open" on the right-hand side : */
	/* Top */
	gdk_draw_line(w->window, gc, 
		      H_SPACING, 0, 
		      w->allocation.width, 0);

	/* Left */
	gdk_draw_line(w->window, gc, 
		      H_SPACING, 0,
		      H_SPACING, expanded ? w->allocation.height-1 : w->allocation.height-1-V_SPACING);	

	/* Bottom */  
	if (expanded) {
		gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_DIM_LINE);
		g_assert(gc);
	}

	gdk_draw_line(w->window, gc, 
		      1 + H_SPACING, w->allocation.height-1-V_SPACING,
		      w->allocation.width, w->allocation.height-1-V_SPACING);

	/* Fill the inside of the rectangle: */
	gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_BACKGROUND);
	g_assert(gc);
	
	gdk_draw_rectangle(w->window, gc, 
			   TRUE, 
			   1 + H_SPACING, 1, 
			   w->allocation.width - 1 - H_SPACING, w->allocation.height - 2 - V_SPACING);

	/* Render the text: */
	title_text = cong_dispspec_get_section_header_text(ds,x);
	gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_TEXT);
	gdk_draw_string(w->window,
			title_font->gdk_font,
			gc, 
			H_SPACING + H_INDENT, 2 + title_font->asc,
			title_text);
	g_free(title_text);

	/* FIXME:  this will fail to update when the text is edited */
	
#else	
	str_width = gdk_string_width(title_font->gdk_font, xml_frag_name_nice(x));
	str_width = str_width > 300 ? str_width : 300;

	
	str_width = w->allocation.width - 4;
	
	gc = cong_dispspec_gc_get(ds, x, 0);
	if (!gc) gc = w->style->black_gc;
	
	/* Fill the header box */

	gdk_draw_rectangle(w->window, gc, 1, 1, 1, str_width + 2, w->allocation.height - 4);
	
	/* Frame the coloured rectangle */

	gdk_draw_line(w->window, w->style->black_gc, 
		      0, 0, 
		      str_width + 3, 0);
	gdk_draw_line(w->window, w->style->black_gc, 
		      expanded?3:0, w->allocation.height - 1 - 4,
		      str_width + 3, w->allocation.height - 1 - 4);
	gdk_draw_line(w->window, w->style->black_gc, 
		      str_width + 3, 0,
		      str_width + 3, w->allocation.height - 1 - 4);
	gdk_draw_line(w->window, w->style->black_gc, 
		      0, 0,
		      0, w->allocation.height - 1 - 4);

	/* White out at right */

	gdk_draw_rectangle(w->window, w->style->white_gc, 1, 
			   str_width + 4, 0,
			   w->allocation.width - (str_width + 4), w->allocation.height - 4);

	/* White out below */

	gdk_draw_rectangle(w->window, w->style->white_gc, 1,
			   expanded?4:0, w->allocation.height - 4,
			   w->allocation.width, w->allocation.height);

	/* Lines on left */

	if (expanded) {

	  /* Coloured line in the middle */
	  gdk_draw_line(w->window, gc, 1, w->allocation.height - 4, 1, w->allocation.height);
	  gdk_draw_line(w->window, gc, 2, w->allocation.height - 4, 2, w->allocation.height);

	  /* Black vertical lines right and left */
	  gdk_draw_line(w->window, w->style->black_gc, 
			0, w->allocation.height - 4, 
			0, w->allocation.height);

	  gdk_draw_line(w->window, w->style->black_gc, 
			3, w->allocation.height - 4, 
			3, w->allocation.height);

	}
	/* Section identifier */
	gdk_draw_string(w->window, title_font->gdk_font, w->style->black_gc, 4, 2 + title_font->asc,
									cong_dispspec_name_get(ds, x));

#if !NEW_XML_IMPLEMENTATION
	/* Metadata indicator */
	
	n0 = x->child;
	n0 = ttree_branch_walk_str(n0, "tag_span metadata");
	if (n0)
	{
		gdk_draw_string(w->window, title_font->gdk_font, w->style->black_gc, 
				w->allocation.width - 4 - gdk_string_width(title_font->gdk_font, "meta"),
				2 + title_font->asc, "meta");
	}

	/* Medios indicator */
	
	n0 = x->child;
	n0 = ttree_branch_walk_str(n0, "tag_span metadata tag_span metadata.sourceset");
	if (n0)
	{
		gdk_draw_string(w->window, title_font->gdk_font, w->style->black_gc, 
				w->allocation.width - 4 - gdk_string_width(title_font->gdk_font, "meta") -
				10 - gdk_string_width(title_font->gdk_font, "fuentes"),
				2 + title_font->asc, "fuentes");
	}
#endif /* #if !NEW_XML_IMPLEMENTATION */

#endif

	return TRUE;
}

struct section_fold_cb_data
{
  gboolean done_first;
  gboolean expanded;
  GtkVBox* vbox;
};

void xv_section_fold_cb(GtkWidget* widget, gpointer data)
{
  struct section_fold_cb_data* cb_data = (struct section_fold_cb_data*)data;

  /* Show/Hide all except for the first child: */
  if (cb_data->done_first) {
    if (cb_data->expanded) {
      gtk_widget_show(widget);
    } else {
      gtk_widget_hide(widget);
    }
  } else {
    cb_data->done_first = TRUE;
  }
}

static gint xv_section_head_button_press(GtkWidget *w, GdkEventButton *event, GtkVBox *vbox)
{
  struct section_fold_cb_data cb_data;

  gpointer expanded = g_object_get_data(G_OBJECT(vbox), "expanded");
  expanded = GINT_TO_POINTER(!expanded);
  g_object_set_data(G_OBJECT(vbox), "expanded", expanded);

  /* We will want a redraw of the drawing area, since the appearance of the title bar has changed: */
  gtk_widget_queue_draw_area(w, 0,0, w->allocation.width, w->allocation.height);

  cb_data.done_first = FALSE;
  cb_data.vbox = vbox;
  cb_data.expanded = (expanded!=NULL);

  printf("xv_section_head_button_press\n");

  gtk_container_forall(GTK_CONTAINER(vbox), 
		       xv_section_fold_cb,
		       &cb_data);

  return TRUE;
}

GtkWidget *xv_section_head(CongDispspec *ds, CongNodePtr x)
{
	UNUSED_VAR(TTREE *n0)
	GtkWidget *vbox, *title;

	CongDispspecElement *element = cong_dispspec_lookup_element(ds, cong_node_name(x));
	CongFont *title_font;

	element = cong_dispspec_lookup_element(ds, cong_node_name(x));
	g_assert(element);

 	title_font = cong_dispspec_element_get_font(element, CONG_FONT_ROLE_TITLE_TEXT);
	g_assert(title_font);

	vbox = gtk_vbox_new(FALSE, 0);
	title = gtk_drawing_area_new();

	g_object_set_data(G_OBJECT(vbox), "expanded", GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(vbox), "dispspec", ds);
	g_object_set_data(G_OBJECT(title), "vbox", vbox);

	gtk_box_pack_start(GTK_BOX(vbox), title, TRUE, TRUE, 0);
	gtk_drawing_area_size(GTK_DRAWING_AREA(title), 300, title_font->asc + title_font->desc + 4 /* up and down borders */ + 4 /* space below */);
	gtk_signal_connect(GTK_OBJECT(title), "expose_event",
			   (GtkSignalFunc) xv_section_head_expose, x);
	gtk_signal_connect(GTK_OBJECT(title), "configure_event",
			   (GtkSignalFunc) xv_section_head_expose, x);
	gtk_signal_connect(GTK_OBJECT(title), "button_press_event",
			   (GtkSignalFunc) xv_section_head_button_press, vbox);

	gtk_widget_set_events(title, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

	gtk_widget_show(title);
	gtk_widget_show(vbox);
	return(vbox);
}


/* 
   The fragment head:

   Currently just a GtkDrawingArea widget, with a "dispspec" field of data.  Probably should be a subclass.   
*/
static gint xv_fragment_head_expose(GtkWidget *w, GdkEventExpose *event, CongNodePtr x)
{
	GdkGC *gc;
	int str_width;
	int i;

	CongDispspec *ds = g_object_get_data(G_OBJECT(w), "dispspec");

	CongDispspecElement *element = cong_dispspec_lookup_element(ds, cong_node_name(x));
	CongFont *title_font;

	element = cong_dispspec_lookup_element(ds, cong_node_name(x));
	g_assert(element);

 	title_font = cong_dispspec_element_get_font(element, CONG_FONT_ROLE_TITLE_TEXT);
	g_assert(title_font);

	
#if NEW_LOOK
	/* Top: */
	draw_blended_line(w,
			  cong_dispspec_element_col(element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE),
			  H_SPACING + 1, 0, 
			  H_SPACING + FRAGMENT_WIDTH);

	gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE);
	g_assert(gc);

	/* Left: */
	gdk_draw_line(w->window, gc, 
		      H_SPACING, 0,
		      H_SPACING, w->allocation.height-1);	

	/* Bottom: */
	draw_blended_line(w,
			  cong_dispspec_element_col(element, CONG_DISPSPEC_GC_USAGE_DIM_LINE),
			  H_SPACING + 1, w->allocation.height-1-V_SPACING,
			  H_SPACING + FRAGMENT_WIDTH);

	/* Fill the inside of the rectangle: */
	gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_BACKGROUND);
	g_assert(gc);
	
	gdk_draw_rectangle(w->window, gc, 
			   TRUE, 
			   1 + H_SPACING, 1,
			   FRAGMENT_WIDTH, w->allocation.height - 2 - V_SPACING);

	/* Render the text: */
	gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_TEXT);
	gdk_draw_string(w->window,
			title_font->gdk_font,
			gc, 
			H_SPACING + H_INDENT, 2 + title_font->asc,
			cong_dispspec_element_username(element));

#else
	str_width = gdk_string_width(title_font->gdk_font, xml_frag_name_nice(x));
	str_width = str_width > 150 ? str_width : 150;

	gc = cong_dispspec_gc_get(ds, cong_node_parent(x), 0);
	if (!gc) gc = w->style->black_gc;

	/* Fill the header box */

	/* ========== */
	
	gdk_draw_rectangle(w->window, gc, 1, 1, 0, str_width + 2, w->allocation.height - 4);

	/*           / */

	for (i = 1; i < w->allocation.height; i++)
	{
		gdk_draw_line(w->window, gc, str_width + 3, i,
									str_width + (w->allocation.height - i + 1 - 4), i);
	}


	/* Frame the coloured <thing> */

	gdk_draw_line(w->window, w->style->black_gc, 3, 0, str_width + 3 + w->allocation.height - 4, 0);

	gdk_draw_line(w->window, w->style->black_gc, 3, w->allocation.height - 1 - 4,
								str_width + 3, w->allocation.height - 1 - 4);

	gdk_draw_line(w->window, w->style->black_gc,
								str_width + 2 + w->allocation.height - 4, 0,
								str_width + 3, w->allocation.height - 1 - 4);

	gdk_draw_line(w->window, w->style->black_gc, 0, 0,
								0, w->allocation.height - 1 - 4);

	/* White out at right */

	gdk_draw_rectangle(w->window, w->style->white_gc, 1,
										 str_width + 3 + w->allocation.height - 4, 0,
										 w->allocation.width - (str_width + 4 + w->allocation.height - 4),
										 w->allocation.height - 4);

	/* White out below */

	gdk_draw_rectangle(w->window, w->style->white_gc, 1,
										 4, w->allocation.height - 4,
										 w->allocation.width, w->allocation.height);

	/* Lines on left */
	
	/* Coloured line in the middle */

	gdk_draw_line(w->window, gc, 1, w->allocation.height - 4, 1, w->allocation.height);
	gdk_draw_line(w->window, gc, 2, w->allocation.height - 4, 2, w->allocation.height);

	/* Black vertical lines right and left */

	gdk_draw_line(w->window, w->style->black_gc, 0, w->allocation.height - 4, 0,
								w->allocation.height);

	gdk_draw_line(w->window, w->style->black_gc, 3, w->allocation.height - 4, 3,
								w->allocation.height);

	/* Section identifier */

	gdk_draw_string(w->window, title_font->gdk_font, w->style->black_gc, 4, 2 + title_font->asc,
									cong_dispspec_name_get(ds, x));
#endif
	
	return(TRUE);
}


GtkWidget *xv_fragment_head(CongDispspec *ds, CongNodePtr x)
{
	UNUSED_VAR(TTREE *n0)
	GtkWidget *title;

	CongDispspecElement *element = cong_dispspec_lookup_element(ds, cong_node_name(x));
	CongFont *title_font;

	element = cong_dispspec_lookup_element(ds, cong_node_name(x));
	g_assert(element);

 	title_font = cong_dispspec_element_get_font(element, CONG_FONT_ROLE_TITLE_TEXT);
	g_assert(title_font);

	title = gtk_drawing_area_new();
	g_object_set_data(G_OBJECT(title), "dispspec", ds);

	gtk_drawing_area_size(GTK_DRAWING_AREA(title), 200, title_font->asc + title_font->desc + 4 /* framing and inside space */ + 4 /* below space */);
	gtk_signal_connect(GTK_OBJECT(title), "expose_event",
			   (GtkSignalFunc) xv_fragment_head_expose, x);
	gtk_signal_connect(GTK_OBJECT(title), "configure_event",
			   (GtkSignalFunc) xv_fragment_head_expose, x);
	gtk_widget_set_events(title, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

	gtk_widget_show(title);
	return(title);
}


static gint xv_fragment_tail_expose(GtkWidget *w, GdkEventExpose *event, CongDispspecElement *element)
{
#if NEW_LOOK
	GdkGC *gc;

	/* Short horizontal line: */
	draw_blended_line(w,
			  cong_dispspec_element_col(element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE),
			  H_SPACING, 0, 
			  H_SPACING + 45);

	gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE);
	g_assert(gc);

	/* Line down left-hand side: */
	gdk_draw_line(w->window, gc, 
		      H_SPACING, 1,
		      H_SPACING, w->allocation.height-1);	

#else
	GdkGC *gc = cong_dispspec_element_gc(element);
	if (!gc) gc = w->style->black_gc;
	
	/* Coloured line in the middle */

	gdk_draw_line(w->window, gc, 1, 0, 1, w->allocation.height);
	gdk_draw_line(w->window, gc, 2, 0, 2, w->allocation.height);

	/* Black vertical line on left */

	gdk_draw_line(w->window, w->style->black_gc, 0, 0, 0,
								w->allocation.height);

	/* Fill */
	
	gdk_draw_line(w->window, gc, 3, 1, 6, 1);
	gdk_draw_line(w->window, gc, 3, 2, 5, 2);
	gdk_draw_line(w->window, gc, 3, 3, 4, 3);
	
  /* _ */
	
	gdk_draw_line(w->window, w->style->black_gc, 3, 0, 7, 0);
	
	/* / */
	
	gdk_draw_line(w->window, w->style->black_gc, 7, 0, 3, 4);
#endif
	
	return(TRUE);
}


GtkWidget *xv_fragment_tail(CongDispspec *ds, CongNodePtr x)
{
        UNUSED_VAR(GdkColor gcol)
	UNUSED_VAR(GtkStyle *style)
	GtkWidget *line;
	UNUSED_VAR(int i)
	CongDispspecElement *element = cong_dispspec_lookup_element(ds, cong_node_name(cong_node_parent(x)));

	line = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(line), 8, 5);
	gtk_signal_connect(GTK_OBJECT(line), "expose_event",
			   (GtkSignalFunc) xv_fragment_tail_expose, element);
	gtk_signal_connect(GTK_OBJECT(line), "configure_event",
			   (GtkSignalFunc) xv_fragment_tail_expose, element);
	gtk_widget_set_events(line, GDK_EXPOSURE_MASK);

	xv_style_r(line, style_white);
	gtk_widget_show(line);
	return(line);
}


static gint xv_section_indent_expose(GtkWidget *w, GdkEventExpose *event, CongDispspecElement *element)
{
#if NEW_LOOK
#else
	GdkGC *gc = cong_dispspec_element_gc(element);
	if (!gc) gc = w->style->black_gc;

	/* Coloured line in the middle */

	gdk_draw_line(w->window, gc, 1, 0, 1, w->allocation.height);
	gdk_draw_line(w->window, gc, 2, 0, 2, w->allocation.height);

	/* Black vertical lines right and left */

	gdk_draw_line(w->window, w->style->black_gc, 0, 0, 0,
		      w->allocation.height);

	gdk_draw_line(w->window, w->style->black_gc, 3, 0, 3,
		      w->allocation.height);
#endif
	
	return(TRUE);
}


static gint xv_section_vline_and_space_expose(GtkWidget *w, GdkEventExpose *event, CongDispspecElement *element)
{
#if NEW_LOOK
	/* Vertical line: */
	GdkGC *gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE);
	g_assert(gc);

	/* White space: */
	gdk_draw_rectangle(w->window, w->style->white_gc, 
			   TRUE, 
			   0,0,
			   w->allocation.width-1, w->allocation.height);

	/* Vertical line: */
	gdk_draw_line(w->window, gc, 
		      H_SPACING, 0, 
		      H_SPACING, w->allocation.height);


#else
	GdkGC *gc = cong_dispspec_element_gc(element);
	if (!gc) gc = w->style->black_gc;
	/* Coloured line in the middle */
	
	gdk_draw_line(w->window, gc, 1, 0, 1, w->allocation.height);
	gdk_draw_line(w->window, gc, 2, 0, 2, w->allocation.height);
	
	/* Black vertical lines right and left */

	gdk_draw_line(w->window, w->style->black_gc, 0, 0, 0,
		      w->allocation.height);

	gdk_draw_line(w->window, w->style->black_gc, 3, 0, 3,
		      w->allocation.height);

	/* White space */
	
	gdk_draw_rectangle(w->window, w->style->white_gc, 1, 4, 0, 5, w->allocation.height);
#endif	
	return(TRUE);
}


static gint xv_section_tail_expose(GtkWidget *w, GdkEventExpose *event, CongDispspecElement *element)
{
#if NEW_LOOK
	draw_blended_line(w,
			  cong_dispspec_element_col(element, CONG_DISPSPEC_GC_USAGE_BOLD_LINE),
			  H_SPACING, 0, 
			  H_SPACING + 180);
#else
  UNUSED_VAR(GdkColor gcol)

	GdkGC *gc = cong_dispspec_element_gc(element);
	if (!gc) gc = w->style->black_gc;
	
#if 0	
	/* Start with desired background fill colour */

	gtk_style_set_background(w->style, w->window, GTK_STATE_NORMAL);
#endif

	/* Clear to white */

	gdk_draw_rectangle(w->window, w->style->white_gc, 1, 0, 0,
										 w->allocation.width,
										 w->allocation.height);

	/* | */
	
	gdk_draw_line(w->window, w->style->black_gc, 0, 0, 0, 9);
	
	/* / */
	
	gdk_draw_line(w->window, w->style->black_gc, 0, 9, 9, 0);
	
	/* _ */
	
	gdk_draw_line(w->window, w->style->black_gc, 9, 0, 3, 0);

	/* Fill */

	gdk_draw_line(w->window, gc, 1, 0, 2, 0);
	gdk_draw_line(w->window, gc, 1, 1, 7, 1);
	gdk_draw_line(w->window, gc, 1, 2, 6, 2);
	gdk_draw_line(w->window, gc, 1, 3, 5, 3);
	gdk_draw_line(w->window, gc, 1, 4, 4, 4);
	gdk_draw_line(w->window, gc, 1, 5, 3, 5);
	gdk_draw_line(w->window, gc, 1, 6, 2, 6);
	gdk_draw_line(w->window, gc, 1, 7, 1, 7);
#endif
	
	return(TRUE);
}


GtkWidget *xv_section_vline(CongDispspec *ds, CongNodePtr x)
{
	GtkWidget *line;
	CongDispspecElement *element = cong_dispspec_lookup_element(ds, cong_node_name(x));

	line = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(line), 4, 0);
	gtk_signal_connect(GTK_OBJECT(line), "expose_event",
			   (GtkSignalFunc) xv_section_indent_expose, element);
	gtk_signal_connect(GTK_OBJECT(line), "configure_event",
			   (GtkSignalFunc) xv_section_indent_expose, element);
	gtk_widget_set_events(line, GDK_EXPOSURE_MASK);


	gtk_widget_show(line);
	return(line);
}


GtkWidget *xv_section_vline_and_space(CongDispspec *ds, CongNodePtr x)
{
	GtkWidget *line;
	CongDispspecElement *element = cong_dispspec_lookup_element(ds, cong_node_name(x));

	line = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(line), 8, 0);
	gtk_signal_connect(GTK_OBJECT(line), "expose_event",
			   (GtkSignalFunc) xv_section_vline_and_space_expose, element);
	gtk_signal_connect(GTK_OBJECT(line), "configure_event",
			   (GtkSignalFunc) xv_section_vline_and_space_expose, element);
	gtk_widget_set_events(line, GDK_EXPOSURE_MASK);


	gtk_widget_show(line);
	return(line);
}


GtkWidget *xv_section_data(CongNodePtr x, CongDispspec *ds, int collapsed)
{
	GtkWidget *hbox, *line;
	CongXMLEditor *xed;

	hbox = gtk_hbox_new(FALSE, 0);

	line = xv_section_vline_and_space(ds, collapsed ? cong_node_parent(cong_node_parent(x)) :
													cong_node_parent(x));

	gtk_box_pack_start(GTK_BOX(hbox), line, FALSE, TRUE, 0);

	xed = xmledit_new(x, ds);
#if 0
	gtk_box_pack_start(GTK_BOX(hbox), xed->w, FALSE, TRUE, 0);
#else
	gtk_box_pack_start(GTK_BOX(hbox), xed->w, TRUE, TRUE, 0);
#endif

#if 1
	gtk_widget_show(line);
	gtk_widget_show(hbox);
	gtk_widget_show(xed->w);
	gtk_widget_queue_resize(xed->w);
#endif
	return(hbox);
}


GtkWidget *xv_section_tail(CongDispspec *ds, CongNodePtr x)
{
	UNUSED_VAR(GdkColor gcol)
	UNUSED_VAR(GtkStyle *style)
	GtkWidget *line;
	UNUSED_VAR(int i)
	CongDispspecElement *element = cong_dispspec_lookup_element(ds, cong_node_name(x));

	line = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(line), 10, 10);
	gtk_signal_connect(GTK_OBJECT(line), "expose_event",
			   (GtkSignalFunc) xv_section_tail_expose, element);
	gtk_signal_connect(GTK_OBJECT(line), "configure_event",
			   (GtkSignalFunc) xv_section_tail_expose, element);
	gtk_widget_set_events(line, GDK_EXPOSURE_MASK);

	gtk_widget_show(line);
	return(line);
}

CongNodePtr xv_editor_elements_skip(CongNodePtr x, CongDispspec *ds)
{
	for ( ; x; x = cong_node_next(x))
	{
		enum CongNodeType node_type = cong_node_type(x);
		const char *name = xml_frag_name_nice(x);

		if (node_type == CONG_NODE_TYPE_ELEMENT && cong_dispspec_element_structural(ds, name))
		{
			return(cong_node_prev(x));
		}

		if (CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE==cong_dispspec_type(ds, name)) {
			return(cong_node_prev(x));
		}
	}

	return(x);
}

void xv_style_r(GtkWidget *widget, gpointer data)
{
	GtkStyle *style;

	style = (GtkStyle *) data;
	gtk_widget_set_style(widget, style);

	if (GTK_IS_CONTAINER(widget))
	{
		gtk_container_foreach(GTK_CONTAINER(widget),
													xv_style_r, style);
	}
}

GtkWidget* embedded_new(CongNodePtr x,CongDispspec *ds)
{
	GtkWidget *embedded;

#if 1
#if 0
	/* char* moniker = "file:/home/david/cdrom/readme.txt"; */
	/* char* moniker = "file:/home/david/Downloads/Backgrounds/artist.955353495.jpg"; */
	/* char* moniker = "file:/home/david/coding/conge-cvs-dhm/conge/src/icon_openfile.c"; */

	char* moniker = "file:./icon_openfile.c";
	/* 
	   This works, but we'll probably have to give some thought to search paths...
	   Should be relative to the document rather than to the PWD.
	 */

	/* char* moniker = "fubar"; */
	/* Test for broken moniker */
	
	/*   char* moniker = "file:/home/david/Downloads/Backgrounds/artist.956323194.jpg"; */
#endif

	GtkVBox *vbox;
	GtkHBox *hbox;
	GtkEntry* entry;
	GtkButton* browse;

	char* file_ref = "../src/ilogo.c"; /* FIXME: extract from file */
	char* moniker = g_strdup_printf("file:%s",file_ref);

	g_message("embedded_new\n");


	vbox = GTK_VBOX(gtk_vbox_new(FALSE,0));
	hbox = GTK_HBOX(gtk_hbox_new(FALSE,0));

	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(hbox), FALSE, TRUE, 0);

	entry = GTK_ENTRY(gtk_entry_new());
	gtk_entry_set_text(entry, file_ref);
	browse = GTK_BUTTON(gtk_button_new_with_label("Browse"));

	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(entry), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(browse), FALSE, TRUE, 0);

	
		
	embedded = bonobo_widget_new_control(moniker,NULL);

	g_free(moniker);
		
	/* error handling ? */
	if (NULL==embedded) {
		embedded = gtk_label_new("Conglomerate could not create a control for viewing this file's content.");
	}

	gtk_box_pack_start(GTK_BOX(vbox), embedded, FALSE, TRUE, 0);

	return GTK_WIDGET(vbox);
#else
	/* Create a simple test widget: */
	embedded = gtk_calendar_new();
	return embedded;
#endif
}

GtkWidget *xv_section_embedded(CongNodePtr x,CongDispspec *ds, int collapsed)
{
	GtkWidget *hbox, *line;
	GtkWidget *embedded;

	hbox = gtk_hbox_new(FALSE, 0);

	line = xv_section_vline_and_space(ds, collapsed ? cong_node_parent(cong_node_parent(x)) :
					  cong_node_parent(x));

	gtk_box_pack_start(GTK_BOX(hbox), line, FALSE, TRUE, 0);

	embedded = embedded_new(x, ds);
#if 0
	gtk_box_pack_start(GTK_BOX(hbox), embedded, FALSE, TRUE, 0);
#else
	gtk_box_pack_start(GTK_BOX(hbox), embedded, TRUE, TRUE, 0);
#endif

#if 1
	gtk_widget_show(line);
	gtk_widget_show(hbox);
	gtk_widget_show(embedded);
	gtk_widget_queue_resize(embedded);
#endif
	return(hbox);

}

GtkWidget *xv_element_new(CongDocument *doc, 
			  CongNodePtr x, 
			  CongDispspec *ds, 
			  GtkWidget *root, 
			  int collapsed, 
			  GtkTreeStore* store, 
			  GtkTreeIter* parent_iter)
{
	UNUSED_VAR(GdkGCValuesMask gc_values_mask = GDK_GC_FOREGROUND /* | GDK_GC_FONT */)
	UNUSED_VAR(GdkGCValues     gc_values)

	CongNodePtr x_orig;
	UNUSED_VAR(TTREE *n0)
	UNUSED_VAR(TTREE *n1)
	UNUSED_VAR(GtkWidget *frame)
	GtkWidget *sub = NULL, *hbox, *poot; /*  *glaebb_item, *glaebb_tree; */
	UNUSED_VAR(CongXMLEditor *xed)
	UNUSED_VAR(unsigned int col)
	UNUSED_VAR(int i)

	GtkTreeIter new_tree_iter;

	CongDispspecElement *element = cong_dispspec_lookup_element(ds, cong_node_name(x));
      	
#if 1
	gtk_tree_store_append (store, &new_tree_iter, parent_iter);

	gtk_tree_store_set (store, &new_tree_iter,
			    TREEVIEW_TITLE_COLUMN, cong_dispspec_get_section_header_text(ds, x),
			    TREEVIEW_NODE_COLUMN, x,
			    TREEVIEW_DOC_COLUMN, doc,
			    -1);

	if (element) {
#if NEW_LOOK
		const GdkColor *col = cong_dispspec_element_col(element, CONG_DISPSPEC_GC_USAGE_TEXT);
		/* We hope this will contrast well against white */
#else
		const GdkColor *col = cong_dispspec_element_col(element);
#endif

		gchar *col_string = get_col_string(col);

		gtk_tree_store_set (store, &new_tree_iter,
				    TREEVIEW_FOREGROUND_COLOR_COLUMN, col_string,
				    -1);

		g_free(col_string);

		/* Experimental attempt to show background colour; looks ugly */
#if 0 /* NEW_LOOK */
		col_string = get_col_string( cong_dispspec_element_col(element, CONG_DISPSPEC_GC_USAGE_BACKGROUND) );
		gtk_tree_store_set (store, &new_tree_iter,
				    TREEVIEW_BACKGROUND_COLOR_COLUMN, col_string,
				    -1);

		g_free(col_string);
#endif

	} else {
		/* Use red for "tag not found" errors: */ 
		gtk_tree_store_set (store, &new_tree_iter,
				    TREEVIEW_FOREGROUND_COLOR_COLUMN, "#ff0000", 
				    -1);
	}


	/* FIXME:  this will fail to update when the text is edited */
#else
	glaebb_tree = 0;

	glaebb_item = gtk_tree_item_new_with_label();
	gtk_tree_append(GTK_TREE(tree_parent), glaebb_item);
	tpopup_init(glaebb_item, x);

	gtk_widget_show(glaebb_item);
#endif

#if 0
	frame = gtk_event_box_new();
	gtk_widget_show(frame);
#endif


	x_orig = x;
#if 0	
	gtk_container_add(GTK_CONTAINER(frame), root);
#endif
	
	
	xv_style_r(root, style_white);


	x = cong_node_first_child(x);
	if (!x) return(0);

	for ( ; x; )
	{
		enum CongNodeType node_type = cong_node_type(x);
		const char *name = xml_frag_name_nice(x);

		/* g_message("Examining frag %s\n",name); */

		if (node_type == CONG_NODE_TYPE_ELEMENT)
		{
			if (cong_dispspec_element_structural(ds, name))
			{
				if (cong_dispspec_element_collapse(ds, name))
				{
#if 0
					if (!glaebb_tree)
					{
						glaebb_tree = gtk_tree_new();
						gtk_tree_item_set_subtree(GTK_TREE_ITEM(glaebb_item), glaebb_tree);
					}
#endif
					
					gtk_box_pack_start(GTK_BOX(root), xv_fragment_head(ds, x), FALSE, TRUE, 0);
					sub = xv_element_new(doc, x, ds, root, 1, store, &new_tree_iter);
					gtk_box_pack_start(GTK_BOX(root), xv_fragment_tail(ds, x), FALSE, TRUE, 0);
				}
				else
				{
					/* New structural element */
#if 0					
					if (!glaebb_tree)
					{
						glaebb_tree = gtk_tree_new();
						gtk_tree_item_set_subtree(GTK_TREE_ITEM(glaebb_item), glaebb_tree);
					}
#endif
					
					hbox = gtk_hbox_new(FALSE, 0);
					gtk_widget_show(hbox);

					gtk_box_pack_start(GTK_BOX(root), hbox, FALSE, TRUE, 0);
					gtk_box_pack_start(GTK_BOX(hbox), xv_section_vline_and_space(ds, cong_node_parent(x)), FALSE, TRUE, 0);
					xv_style_r(hbox, style_white);
					poot = xv_section_head(ds, x);
					gtk_box_pack_start(GTK_BOX(hbox), poot, TRUE, TRUE, 0);
					sub = xv_element_new(doc, x, ds, poot, 0, store, &new_tree_iter);


					sub = xv_section_tail(ds, x);
					xv_style_r(sub, style_white);
					gtk_box_pack_start(GTK_BOX(poot), sub, FALSE, TRUE, 0);
				}
			}
			else if (cong_dispspec_element_span(ds, name) ||
							 cong_dispspec_element_insert(ds, name))
			{
				/* New editor window */
				
				sub = xv_section_data(x, ds, collapsed);
				if (sub)
				{
					gtk_box_pack_start(GTK_BOX(root), sub, FALSE, TRUE, 0);
					xv_style_r(sub, style_white);
				}
				
				x = xv_editor_elements_skip(x, ds);
			} else if (CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE==cong_dispspec_type(ds, name)) {
#if 1
				hbox = gtk_hbox_new(FALSE, 0);
				gtk_widget_show(hbox);

				gtk_box_pack_start(GTK_BOX(root), hbox, FALSE, TRUE, 0);
				gtk_box_pack_start(GTK_BOX(hbox), xv_section_vline_and_space(ds, cong_node_parent(x)), FALSE, TRUE, 0);
				xv_style_r(hbox, style_white);
				poot = xv_section_head(ds, x);
				gtk_box_pack_start(GTK_BOX(hbox), poot, TRUE, TRUE, 0);
				/* xv_style_r(poot, style_white); */
				
				sub = xv_section_embedded(x,ds,collapsed);
				gtk_box_pack_start(GTK_BOX(poot), sub, FALSE, TRUE, 0);
					
				sub = xv_section_tail(ds, x);
				/* xv_style_r(sub, style_white); */
				gtk_box_pack_start(GTK_BOX(poot), sub, FALSE, TRUE, 0);

#else
				sub = xv_section_embedded(x,ds,collapsed);
				if (sub)
				{

					gtk_box_pack_start(GTK_BOX(root), sub, FALSE, TRUE, 0);
					xv_style_r(sub, style_white);
				}
#endif
			}
		}
		else if (node_type == CONG_NODE_TYPE_TEXT)
		{
			/* New editor window */

			sub = xv_section_data(x, ds, collapsed);
			if (sub)
			{
				gtk_box_pack_start(GTK_BOX(root), sub, FALSE, TRUE, 0);
				xv_style_r(sub, style_white);
			}
			
			x = xv_editor_elements_skip(x, ds);
		}

		if (x) {
			x = cong_node_next(x);
		}
	}

	xv_style_r(sub, style_white);
#if 1
	/* g_message("removed call to gtk_tree_item_expand\n"); */
#else
	gtk_tree_item_expand(GTK_TREE_ITEM(glaebb_item));
#endif
	return(root);

}

struct xview *xmlview_new(CongDocument *doc)
{
	struct xview *xv;
	GdkColor gcol;
	GtkWidget *w; /* , *glaebb_tree; */
	int i;
	CongNodePtr x;

	CongDispspec *displayspec;

	gchar* filename;

	GtkTreeIter root_iter;

	g_message("xmlview_new called\n");

	g_return_val_if_fail(doc, NULL);
	
	displayspec = cong_document_get_dispspec(doc);
	
	the_globals.curs.set = 0;
	the_globals.curs.xed = 0;
	the_globals.curs.w = 0;
	the_globals.selection.xed = 0;

#if 1
	cong_location_nullify(&the_globals.selection.loc0);
	cong_location_nullify(&the_globals.selection.loc1);
#else
	the_globals.selection.t0 = the_globals.selection.t1 = 0;
#endif
	
	xv = malloc(sizeof(*xv));
	memset(xv, 0, sizeof(*xv));


	xv->doc = doc;

	gcol.blue = 0xffff;
	gcol.green = 0xffff;
	gcol.red = 0xffff;

	style_white = gtk_widget_get_default_style();
	style_white = gtk_style_copy(style_white);

	gdk_colormap_alloc_color(cong_gui_get_window(&the_gui)->style->colormap, &gcol, 0, 1);

	for (i = 0; i < 5; i++) style_white->bg[i] = gcol;

	filename = cong_document_get_filename(doc);

	gtk_tree_store_append (cong_gui_get_tree_store(&the_gui), &root_iter, NULL);  /* Acquire a top-level iterator */
	gtk_tree_store_set (cong_gui_get_tree_store(&the_gui), &root_iter,
			    TREEVIEW_TITLE_COLUMN, filename,
			    TREEVIEW_NODE_COLUMN, cong_document_get_root(doc),
			    TREEVIEW_DOC_COLUMN, doc,
			    /* TREEVIEW_COLOR_COLUMN, g_strdup_printf("#305050"), */
			    -1);
	/* FIXME: What colour should the Document node be? */

	g_free(filename);

	xv->w = gtk_vbox_new(FALSE, 0);
#if 1
	gtk_widget_show(xv->w);
#endif

#if 1
	/* Don't ignore root element: */
	x = cong_document_get_root(doc);
#else	
	/* Ignore root element: */
	x = cong_node_first_child(cong_document_get_root(doc));
#endif

	for ( ; x; x = cong_node_next(x))
	{
		enum CongNodeType type = cong_node_type(x);

		const char *name = xml_frag_name_nice(x);

		g_message("examining frag \"%s\", type = %s\n", name, cong_node_type_description(type));
		
		if (type == CONG_NODE_TYPE_ELEMENT && cong_dispspec_element_structural(displayspec, name))
		{
			/* New element */
			GtkWidget* head = xv_section_head(displayspec, x);

			gtk_box_pack_start(GTK_BOX(xv->w), head, TRUE, TRUE, 0);
			
			xv_element_new(doc, x, displayspec, head, 0, cong_gui_get_tree_store(&the_gui), &root_iter);

			w = xv_section_tail(displayspec, x);
			xv_style_r(w, style_white);
			gtk_box_pack_start(GTK_BOX(head), w, FALSE, TRUE, 0);
		}
	}


#if 1
	printf("removed call to gtk_tree_item_expand\n");
#else
	gtk_tree_item_expand(GTK_TREE_ITEM(xv->tree));
#endif
	
	gtk_widget_show_all(xv->w);
	
	return(xv);
}

void xmlview_destroy(int free_xml)
{
	if (!the_globals.xv) return;

 	cong_gui_destroy_tree_store(&the_gui);
	gtk_widget_destroy(the_globals.xv->w);

#if 1
	if (free_xml) {
		cong_document_delete(the_globals.xv->doc);
	}
#else
	if (free_xml) ttree_branch_remove(the_globals.xv->x);
#endif
	
	free(the_globals.xv);
	the_globals.xv = 0;
}
