/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <stdlib.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "global.h"
#include "cong-document.h"
#include "cong-dispspec.h"
#include "cong-eel.h"

/* This file is mostly dead code from old editor widget; I'm keeping it for now because it has some Bonobo code...: */
#if 0

GtkStyle *style_white;

#include <bonobo/bonobo-widget.h>

#define CONG_EDITOR_VIEW(x) ((CongEditorView*)(x))

struct CongEditorView
{
	CongView view;
	GtkWidget *root;
	GtkWidget *inner;
};
#endif

#if 0
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
#endif

/**
 * get_col_string:
 * @col:
 *
 * TODO: Write me
 * Returns:
 */
gchar*
get_col_string(const GdkColor* col)
{

	guint32 col32;

	g_return_val_if_fail(col,NULL);

	col32 = cong_eel_gdk_color_to_rgb (col);
	
	return cong_eel_gdk_rgb_to_color_spec(col32);
}

#if 0
/*
  We handle folding by showing/hiding all but the first child of the vbox at the root of a xv_section_head.
  We have a CongSectionHead which stores relevant data; the vbox title widgets store pointers to this as userdata.
  Probably would be cleaner to have a new widget subclass...
 */
typedef struct CongSectionHead CongSectionHead;

struct CongSectionHead
{
	CongDocument *doc;
	CongNodePtr node;
	gboolean expanded;

	GtkWidget *vbox, *title;
};

#define V_SPACING (4)
#define H_SPACING (4)
#define H_INDENT (4)
#define FRAGMENT_WIDTH (45)

static gint xv_section_head_expose(GtkWidget *w, GdkEventExpose *event, CongNodePtr x)
{
	GdkGC *gc;
	int str_width;
	gchar *title_text;

	CongSectionHead *section_head;
	CongDispspec *ds;
	CongDispspecElement *element;
	CongFont *title_font;

	section_head = g_object_get_data(G_OBJECT(w), "section_head");
	g_assert(section_head);

	ds = cong_document_get_dispspec(section_head->doc);

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
		      H_SPACING, section_head->expanded ? w->allocation.height-1 : w->allocation.height-1-V_SPACING);	

	/* Bottom */  
	if (section_head->expanded) {
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
	title_text = cong_dispspec_element_get_section_header_text(element, x);
	gc = cong_dispspec_element_gc(element, CONG_DISPSPEC_GC_USAGE_TEXT);
	gdk_draw_string(w->window,
			title_font->gdk_font,
			gc, 
			H_SPACING + H_INDENT, 2 + title_font->asc,
			title_text);
	g_free(title_text);

	/* FIXME:  this will fail to update when the text is edited */

	/* Test of expanders: */
#if 0
	{
		GdkRectangle area;
		area.x=5;
		area.y=5;
		area.width=50;
		area.height=50;
		gtk_paint_expander(w->style,
				   w->window,
				   GTK_WIDGET_STATE(w),
				   &area,
				   NULL, /* w, */
				   "", /* const gchar *detail, */
				   5, /* gint x, */
				   5, /* gint y, */
				   expanded ? GTK_EXPANDER_EXPANDED : GTK_EXPANDER_COLLAPSED);
	}
#endif
	
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
		      section_head->expanded?3:0, w->allocation.height - 1 - 4,
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
			   section_head->expanded?4:0, w->allocation.height - 4,
			   w->allocation.width, w->allocation.height);

	/* Lines on left */

	if (section_head->expanded) {

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

#if 0
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
#endif

#endif

	return TRUE;
}

struct section_fold_cb_data
{
	gboolean done_first;
	gboolean expanded;
	GtkVBox* vbox;
};

static void xv_section_fold_cb(GtkWidget* widget, gpointer data)
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

	CongSectionHead *section_head = g_object_get_data(G_OBJECT(vbox), "section_head");
	g_assert(section_head);

	switch (event->button) {
	default: return FALSE;
	case 1: /* Normally the left mouse button: */
		{
			/* Toggle the expand/collapse status of the area: */
			section_head->expanded = !section_head->expanded;

			/* We will want a redraw of the drawing area, since the appearance of the title bar has changed: */
			gtk_widget_queue_draw_area(w, 0,0, w->allocation.width, w->allocation.height);

			cb_data.done_first = FALSE;
			cb_data.vbox = vbox;
			cb_data.expanded = section_head->expanded;

			g_message("xv_section_head_button_press");

			gtk_container_forall(GTK_CONTAINER(vbox), 
					     xv_section_fold_cb,
					     &cb_data);
			
			return TRUE;
		}
		break;
		
	case 3: /* Normally the right mouse button: */
		{
			do_node_heading_context_menu(section_head->doc,
						     section_head->node);
			return TRUE;
		}
		break;
	}
}

CongSectionHead *cong_section_head_new(CongDocument *doc, CongNodePtr x)
{
	CongDispspec *ds;
	CongSectionHead *section_head;
	CongDispspecElement *element;
	CongFont *title_font;

	g_return_val_if_fail(doc, NULL);
	g_return_val_if_fail(x, NULL);

	ds = cong_document_get_dispspec(doc);

	element = cong_dispspec_lookup_element(ds, cong_node_name(x));
	g_assert(element);

 	title_font = cong_dispspec_element_get_font(element, CONG_FONT_ROLE_TITLE_TEXT);
	g_assert(title_font);

	section_head = g_new0(CongSectionHead,1);
	section_head->doc = doc;
	section_head->node = x;
	section_head->expanded = TRUE;
	section_head->vbox = gtk_vbox_new(FALSE, 0);
	section_head->title = gtk_drawing_area_new();

	/* FIXME: this will leak */
	g_object_set_data(G_OBJECT(section_head->vbox), "section_head", section_head);
	g_object_set_data(G_OBJECT(section_head->title), "section_head", section_head);

	gtk_box_pack_start(GTK_BOX(section_head->vbox), section_head->title, TRUE, TRUE, 0);
	gtk_drawing_area_size(GTK_DRAWING_AREA(section_head->title), 300, title_font->asc + title_font->desc + 4 /* up and down borders */ + 4 /* space below */);
	gtk_signal_connect(GTK_OBJECT(section_head->title), "expose_event",
			   (GtkSignalFunc) xv_section_head_expose, x);
	gtk_signal_connect(GTK_OBJECT(section_head->title), "configure_event",
			   (GtkSignalFunc) xv_section_head_expose, x);
	gtk_signal_connect(GTK_OBJECT(section_head->title), "button_press_event",
			   (GtkSignalFunc) xv_section_head_button_press, section_head->vbox);

	gtk_widget_set_events(section_head->title, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

	gtk_widget_show(section_head->title);
	gtk_widget_show(section_head->vbox);
	return section_head;
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
	cong_util_draw_blended_line(w,
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
	cong_util_draw_blended_line(w,
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
	cong_util_draw_blended_line(w,
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


GtkWidget *xv_section_data(CongNodePtr x, CongDocument *doc, CongDispspec *ds, int collapsed)
{
	GtkWidget *hbox, *line;
	CongSpanEditor *xed;

	hbox = gtk_hbox_new(FALSE, 0);

	line = xv_section_vline_and_space(ds, 
					  collapsed ? cong_node_parent(cong_node_parent(x)) : cong_node_parent(x));

	gtk_box_pack_start(GTK_BOX(hbox), line, FALSE, TRUE, 0);

	xed = xmledit_new(x, doc, ds);
	/* FIXME: I believe these are getting leaked */

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

static gint xv_section_tail_expose(GtkWidget *w, GdkEventExpose *event, CongDispspecElement *element)
{
#if NEW_LOOK
	cong_util_draw_blended_line(w,
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
		CongNodeType node_type = cong_node_type(x);
		const char *name = xml_frag_name_nice(x);

#error
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

GtkWidget* embedded_new(CongDocument *doc, CongNodePtr x,CongDispspec *ds)
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

	char* file_ref; 
	char* doc_path;
	char* moniker;

	g_message("embedded_new\n");

#if 1
	/* Get file from node attribute: */
	file_ref = cong_node_get_attribute(x, "fileref"); /* FIXME: hardcoded attribute name for now */

	if (file_ref) {
		g_message("Got fileref \"%s\"\n", file_ref);
	} else {
		file_ref= g_strdup("");
	}

	/* Get at the document path: */
	doc_path = cong_document_get_parent_uri(doc);

	/* Generate moniker relative to the document path: */
	moniker = g_strdup_printf("%s/%s",doc_path,file_ref);

	g_free(doc_path);

	g_message("Trying moniker: \"%s\"\n",moniker);
#else
	file_ref = "../src/ilogo.c"; /* FIXME: extract from file */
	moniker = g_strdup_printf("file:%s",file_ref);
#endif



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

	g_free(file_ref);

	return GTK_WIDGET(vbox);
#else
	/* Create a simple test widget: */
	embedded = gtk_calendar_new();
	return embedded;
#endif
}

GtkWidget *xv_section_embedded(CongDocument *doc, CongNodePtr x,CongDispspec *ds, int collapsed)
{
	GtkWidget *hbox, *line;
	GtkWidget *embedded;

	hbox = gtk_hbox_new(FALSE, 0);

	line = xv_section_vline_and_space(ds, collapsed ? cong_node_parent(cong_node_parent(x)) :
					  cong_node_parent(x));

	gtk_box_pack_start(GTK_BOX(hbox), line, FALSE, TRUE, 0);

	embedded = embedded_new(doc, x, ds);
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


#if 0
typedef struct CongStructuralElement CongStructuralElement;

struct CongStructuralElement
{
	GtkWidget *hbox;
	CongSectionHead *section_head;
};

CongStructuralElement* cong_structural_element_new(CongDocument *doc, 
						   CongNodePtr x, 
						   GtkWidget *root,
						   GtkWidget **sub)
{
	CongDispspec *ds = cong_document_get_dispspec(doc);
	CongStructuralElement *structural_element = g_new0(CongStructuralElement,1);
	CongSectionHead *section_head;
	GtkWidget *poot;
					
	structural_element->hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(structural_element->hbox);

	gtk_box_pack_start(GTK_BOX(root), structural_element->hbox, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(structural_element->hbox), xv_section_vline_and_space(ds, cong_node_parent(x)), FALSE, TRUE, 0);
	xv_style_r(structural_element->hbox, style_white);
	section_head = cong_section_head_new(doc, x);

	poot = section_head->vbox;
	gtk_box_pack_start(GTK_BOX(structural_element->hbox), poot, TRUE, TRUE, 0);
	*sub = xv_element_new(doc, x, ds, poot, 0);

	*sub = xv_section_tail(ds, x);
	xv_style_r(*sub, style_white);
	gtk_box_pack_start(GTK_BOX(poot), *sub, FALSE, TRUE, 0);

	return structural_element;
}
#endif

void cong_editor_recursively_populate_ui(CongEditorView *editor_view,
					 CongNodePtr x, 
					 GtkWidget *root, 
					 int collapsed)
{
	CongNodePtr x_orig;
	GtkWidget *sub = NULL;
	CongDocument *doc;
	CongDispspec *ds;

	g_return_if_fail(editor_view);
	
	doc = editor_view->view.doc;
	ds = cong_document_get_dispspec(doc);
      	
	x_orig = x;
	
	xv_style_r(root, style_white);

	x = cong_node_first_child(x);
	if (!x) return;

	for ( ; x; )
	{
		CongNodeType node_type = cong_node_type(x);
		const char *name = xml_frag_name_nice(x);

		/* g_message("Examining frag %s\n",name); */

		if (node_type == CONG_NODE_TYPE_ELEMENT)
		{
			CongDispspecElement* element = cong_dispspec_lookup_element(ds, name);

			if (element) {
				if (cong_dispspec_element_is_structural(element)) {
					if (cong_dispspec_element_collapseto(element)) {
						gtk_box_pack_start(GTK_BOX(root), xv_fragment_head(ds, x), FALSE, TRUE, 0);
						
						/* Recurse here: */
						cong_editor_recursively_populate_ui(editor_view, x, root, TRUE);
						
						gtk_box_pack_start(GTK_BOX(root), xv_fragment_tail(ds, x), FALSE, TRUE, 0);
					} else {
						/* New structural element */
						CongSectionHead *section_head;
						GtkWidget *poot;
						GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
						gtk_widget_show(hbox);
						
						gtk_box_pack_start(GTK_BOX(root), hbox, FALSE, TRUE, 0);
						gtk_box_pack_start(GTK_BOX(hbox), xv_section_vline_and_space(ds, cong_node_parent(x)), FALSE, TRUE, 0);
						xv_style_r(hbox, style_white);
						section_head = cong_section_head_new(doc, x);
						poot = section_head->vbox;
						gtk_box_pack_start(GTK_BOX(hbox), poot, TRUE, TRUE, 0);
								
						/* Recurse here: */
						cong_editor_recursively_populate_ui(editor_view, x, poot, FALSE);
								
						sub = xv_section_tail(ds, x);
						xv_style_r(sub, style_white);
						gtk_box_pack_start(GTK_BOX(poot), sub, FALSE, TRUE, 0);
					}
				} else if (cong_dispspec_element_is_span(element) ||
					   CONG_ELEMENT_TYPE_INSERT == cong_dispspec_element_type(element)) {
				        /* New editor window */
				
					sub = xv_section_data(x, doc, ds, collapsed);
					if (sub) {
						gtk_box_pack_start(GTK_BOX(root), sub, FALSE, TRUE, 0);
						xv_style_r(sub, style_white);
					}
				
					x = xv_editor_elements_skip(x, ds);
				} else if (CONG_ELEMENT_TYPE_EMBED_EXTERNAL_FILE==cong_dispspec_element_type(element)) {
					CongSectionHead *section_head;
					GtkWidget *poot;
					GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
					gtk_widget_show(hbox);

					gtk_box_pack_start(GTK_BOX(root), hbox, FALSE, TRUE, 0);
					gtk_box_pack_start(GTK_BOX(hbox), xv_section_vline_and_space(ds, cong_node_parent(x)), FALSE, TRUE, 0);
					xv_style_r(hbox, style_white);
					section_head = cong_section_head_new(doc,x);
					poot = section_head->vbox;
					gtk_box_pack_start(GTK_BOX(hbox), poot, TRUE, TRUE, 0);
				        /* xv_style_r(poot, style_white); */
				
					sub = xv_section_embedded(doc, x,ds,collapsed);
					gtk_box_pack_start(GTK_BOX(poot), sub, FALSE, TRUE, 0);
					
					sub = xv_section_tail(ds, x);
				        /* xv_style_r(sub, style_white); */
					gtk_box_pack_start(GTK_BOX(poot), sub, FALSE, TRUE, 0);
				}
			}
		}
		else if (node_type == CONG_NODE_TYPE_TEXT)
		{
			/* New editor window */

			sub = xv_section_data(x, doc, ds, collapsed);
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
}

void cong_editor_populate_ui(CongEditorView *editor_view)
{
	GdkColor gcol;
	GtkWidget *w;
	int i;
	CongNodePtr x;

	CongDocument *doc;
	CongDispspec *displayspec;

	g_return_if_fail(editor_view);
	
	doc = editor_view->view.doc;
	displayspec = cong_document_get_dispspec(doc);
		
	gcol.blue = 0xffff;
	gcol.green = 0xffff;
	gcol.red = 0xffff;

	style_white = gtk_widget_get_default_style();
	style_white = gtk_style_copy(style_white);

	gdk_colormap_alloc_color(cong_gui_get_a_window()->style->colormap, &gcol, 0, 1);

	for (i = 0; i < 5; i++) style_white->bg[i] = gcol;


	editor_view->inner = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(editor_view->inner);

	gtk_box_pack_start(GTK_BOX(editor_view->root), editor_view->inner, FALSE, FALSE, 0);

	x = cong_document_get_root(doc);

	for ( ; x; x = cong_node_next(x))
	{
		CongNodeType type = cong_node_type(x);

		const char *name = xml_frag_name_nice(x);

		g_message("examining frag \"%s\", type = %s\n", name, cong_node_type_description(type));
		
		if (type == CONG_NODE_TYPE_ELEMENT && cong_dispspec_element_structural(displayspec, name))
		{
			/* New element */
			CongSectionHead *section_head;
			GtkWidget* head;
			section_head = cong_section_head_new(doc, x);
			head = section_head->vbox;

			gtk_box_pack_start(GTK_BOX(editor_view->inner), head, TRUE, TRUE, 0);
			
			cong_editor_recursively_populate_ui(editor_view, x, head, FALSE);

			w = xv_section_tail(displayspec, x);
			xv_style_r(w, style_white);
			gtk_box_pack_start(GTK_BOX(head), w, FALSE, TRUE, 0);
		}
	}

	gtk_widget_show_all(editor_view->inner);
	
}

void xmlview_destroy(int free_xml)
{
#if 1
	g_assert(0);
#else
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
#endif
}

/* Prototypes of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr former_parent);
static void on_document_node_add_after(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr older_sibling);
static void on_document_node_add_before(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr younger_sibling);
static void on_document_node_set_parent(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
static void on_document_node_set_text(CongView *view, gboolean before_change, CongNodePtr node, const xmlChar *new_content);
static void on_selection_change(CongView *view);
static void on_cursor_change(CongView *view);

#define DEBUG_EDITOR_VIEW 1

/* Definitions of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr former_parent)
{
	CongEditorView *editor_view;

	g_return_if_fail(view);
	g_return_if_fail(node);

	#if DEBUG_EDITOR_VIEW
	g_message("CongEditorView - on_document_node_make_orphan\n");
	#endif

	editor_view = CONG_EDITOR_VIEW(view);

}

static void on_document_node_add_after(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr older_sibling)
{
	CongEditorView *editor_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(older_sibling);

	#if DEBUG_EDITOR_VIEW
	g_message("CongEditorView - on_document_node_add_after\n");
	#endif

	editor_view = CONG_EDITOR_VIEW(view);

}

static void on_document_node_add_before(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr younger_sibling)
{
	CongEditorView *editor_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);

	#if DEBUG_EDITOR_VIEW
	g_message("CongEditorView - on_document_node_add_before\n");
	#endif

	editor_view = CONG_EDITOR_VIEW(view);

}

static void on_document_node_set_parent(CongView *view, gboolean before_change, CongNodePtr node, CongNodePtr adoptive_parent)
{
	CongEditorView *editor_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);

	#if DEBUG_EDITOR_VIEW
	g_message("CongEditorView - on_document_node_set_parent\n");
	#endif

	editor_view = CONG_EDITOR_VIEW(view);

}

static void on_document_node_set_text(CongView *view, gboolean before_change, CongNodePtr node, const xmlChar *new_content)
{
	CongEditorView *editor_view;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_EDITOR_VIEW
	g_message("CongEditorView - on_document_node_set_text\n");
	#endif

	editor_view = CONG_EDITOR_VIEW(view);

}

static void on_selection_change(CongView *view)
{
}

static void on_cursor_change(CongView *view)
{
}


CongEditorView *cong_editor_view_new(CongDocument *doc)
{
	CongEditorView *editor_view;
	g_return_val_if_fail(doc, NULL);

	editor_view = g_new0(CongEditorView, 1);

	editor_view->view.doc = doc;
	editor_view->view.klass = g_new0(CongViewClass,1);
	editor_view->view.klass->on_document_node_make_orphan = on_document_node_make_orphan;
	editor_view->view.klass->on_document_node_add_after = on_document_node_add_after;
	editor_view->view.klass->on_document_node_add_before = on_document_node_add_before;
	editor_view->view.klass->on_document_node_set_parent = on_document_node_set_parent;
	editor_view->view.klass->on_document_node_set_text = on_document_node_set_text;
	editor_view->view.klass->on_selection_change = on_selection_change;
	editor_view->view.klass->on_cursor_change = on_cursor_change;

	cong_document_register_view( doc, CONG_VIEW(editor_view) );

	editor_view->root = gtk_vbox_new(FALSE, 1);

	cong_editor_populate_ui(editor_view);
	gtk_widget_show(editor_view->root);

	return editor_view;
}

void cong_editor_view_free(CongEditorView *editor_view)
{
	g_return_if_fail(editor_view);
	
	/* FIXME: should we delete the widgetry here as well? */

	cong_document_unregister_view( editor_view->view.doc, CONG_VIEW(editor_view) );

	g_free(editor_view->view.klass);
	g_free(editor_view);
}

GtkWidget* cong_editor_view_get_widget(CongEditorView *editor_view)
{
	g_return_val_if_fail(editor_view,NULL);

	return editor_view->root;
}

#endif
