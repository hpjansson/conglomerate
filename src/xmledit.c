/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include <ttree.h>
#include <xml.h>
#include <strtool.h>
#include "global.h"


GtkWidget*
cong_xml_editor_get_widget(CongXMLEditor *xed)
{
	g_return_val_if_fail(xed, NULL);

	return xed->w;
}

void add_stuff_to_tt(TTREE* tt, int pos_y, TTREE* x, int draw_char)
{
	ttree_node_add(tt, (unsigned char*)&pos_y, sizeof(int));
	ttree_node_add(tt, (unsigned char*)&x, sizeof(TTREE *));
	ttree_node_add(tt, (unsigned char*)&draw_char, sizeof(int));
}

void add_stuff(CongXMLEditor *xed, int pos_y, TTREE* x, int draw_char)
{
	add_stuff_to_tt(xed->layout_cache.draw_line_t, pos_y, x, draw_char);
}

TTREE *add_line_to_tt(TTREE* parent, TTREE* tt, int i)
{
	TTREE *new_line;

	g_return_val_if_fail(parent, NULL);

	new_line = ttree_node_add(parent, "line", 4);  /* Add line */
	ttree_node_add(new_line, (unsigned char*)&tt, sizeof(TTREE *));
	ttree_node_add(new_line, (unsigned char*)&i, sizeof(int));

	return new_line;
}

TTREE *cong_layout_cache_add_line(CongLayoutCache *layout_cache, TTREE* tt, int i)
{
	g_return_val_if_fail(layout_cache, NULL);

	return add_line_to_tt(layout_cache->lines, tt, i);
	
}

void add_line(CongXMLEditor *xed, TTREE* tt, int i)
{
	g_return_if_fail(xed);

	xed->layout_cache.draw_line_t = cong_layout_cache_add_line(&xed->layout_cache, tt, i);
}


#define ttree_node_add(a, b, c) ttree_node_add(a, (unsigned char *) b, c)


TTREE *xed_stack_top(CongXMLEditor *xed);
int xed_xml_content_draw(CongXMLEditor *xed, unsigned int mode);


/* Create a new backing pixmap of the appropriate size, redraw content */

static gint configure_event (GtkWidget *widget, GdkEventConfigure *event, CongXMLEditor *xed)
{
	struct pos *pos;
	UNUSED_VAR(GtkRequisition req)
	UNUSED_VAR(GtkAllocation alloc)
	int height;
	UNUSED_VAR(int width_old)

	struct selection* selection = &the_globals.selection;
	struct curs* curs = &the_globals.curs;
	
#if 0	
	if (!xed->p)
	{
		/* Get backing pixmap */

		xed->p = gdk_pixmap_new(widget->window,
					widget->allocation.width,
					height,
					-1);

		/* Make it white */

		gdk_draw_rectangle(xed->p,
				   widget->style->white_gc,
				   TRUE,
				   0, 0,
				   widget->allocation.width,
				   widget->allocation.height);
	}
#endif	

	if (xed->p)
	{
		gdk_draw_pixmap(widget->window,
				widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
				xed->p,
				0, 0,
				0, 0,
				widget->allocation.width, widget->allocation.height);
	}

	if (widget->allocation.width < 200)
	{
#ifndef RELEASE		
		printf("Allocating width of 200.\n");
#endif
		gtk_drawing_area_size(GTK_DRAWING_AREA(widget), 200, widget->allocation.height);
		gtk_widget_queue_resize(widget);
		return(TRUE);
	}

	/* Calculate height */
	cong_layout_cache_clear(&xed->layout_cache);

	xed->draw_x = NULL;  /* FIXME: Out-of-context root node */
	height = xed_xml_content_draw(xed, 0);  /* Or 0? */

	if ((height > xed->w->allocation.height + 2 ||
			height < xed->w->allocation.height - 2) && !xed->already_asked_for_size)
	{
#ifndef RELEASE		
		printf("Allocating height of %d.\n", height);
#endif

		gtk_drawing_area_size(GTK_DRAWING_AREA(xed->w), widget->allocation.width, height);

#if 0
		req.width = 200;
		req.height = height;
		gtk_widget_size_request(xed->w, &req);
#endif

#if 0		
		gtk_widget_queue_resize(xed->w);
#endif
		xed->already_asked_for_size = 1;
		return(TRUE);
	}

	/*
	if (xed->w->allocation.width != event->width ||
			xed->w->allocation.height != event->height)
 */ 
	{
		if (xed->p) gdk_pixmap_unref(xed->p);

		/* Get backing pixmap */

		xed->p = gdk_pixmap_new(widget->window,
					widget->allocation.width,
					height,
					-1);

		/* Make it white */

		gdk_draw_rectangle(xed->p,
				   widget->style->white_gc,
				   TRUE,
				   0, 0,
				   widget->allocation.width,
				   widget->allocation.height);

		if (xed == the_globals.curs.xed && cong_location_exists(&selection->loc0) && cong_location_exists(&selection->loc1))
		{
			pos = pos_logical_to_physical_new(xed, &selection->loc0);
			selection->x0 = pos->x;
			selection->y0 = pos->y;
			free(pos);

			pos = pos_logical_to_physical_new(xed, &selection->loc1);
			selection->x1 = pos->x;
			selection->y1 = pos->y;
			free(pos);
			selection_draw(selection, curs);
		}

		/* Redraw */

		cong_layout_cache_clear(&xed->layout_cache);

		xed->draw_x = NULL;  /* FIXME: Out-of-context root node */
		xed_xml_content_draw(xed, 1);
		if (xed == curs->xed) 
		{			
			pos = pos_logical_to_physical_new(xed, &curs->location);
			curs->x = pos->x;
			curs->y = pos->y;
			free(pos);
		}
	}

	xed->already_asked_for_size = 0;
	return TRUE;
}


void xed_redraw(CongXMLEditor *xed)
{
	UNUSED_VAR(GtkRequisition req)
	int height;

	/* Calculate height */
	cong_layout_cache_clear(&xed->layout_cache);

	xed->draw_x = NULL;  /* FIXME: Out-of-context root node */
	height = xed_xml_content_draw(xed, 0);

	if (height > xed->w->allocation.height + 2 ||
			height < xed->w->allocation.height - 2)
	{
/*		
		req.width = xed->w->allocation.width;
		req.height = height;
		gtk_widget_size_request(xed->w, &req);
*/
		gtk_drawing_area_size(GTK_DRAWING_AREA(xed->w), xed->w->allocation.width, height);
		gtk_widget_queue_resize(xed->w);
	}
	else
	{
		/* Make it white */

		gdk_draw_rectangle(xed->p,
				   xed->w->style->white_gc,
				   TRUE,
				   0, 0,
				   xed->w->allocation.width,
				   xed->w->allocation.height);
		
		/* Redraw */

		selection_draw(&the_globals.selection, &the_globals.curs);

		cong_layout_cache_clear(&xed->layout_cache);

		xed->draw_x = 0;  /* FIXME: Out-of-context root node */
		xed_xml_content_draw(xed, 1);
		
		gdk_draw_pixmap(xed->w->window,
				xed->w->style->fg_gc[GTK_WIDGET_STATE(xed->w)],
				xed->p,
				0, 0,
				0, 0,
				xed->w->allocation.width, xed->w->allocation.height);
	}
}


/* Redraw the area from the backing pixmap */

static gint expose_event (GtkWidget *widget, GdkEventExpose *event, CongXMLEditor *xed)
{
	if (xed->initial)
	{ xed->initial = 0; gtk_widget_queue_resize(widget); }
	
	else if (xed->p)
	{
		gdk_draw_pixmap(widget->window,
				widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
				xed->p,
				event->area.x, event->area.y,
				event->area.x, event->area.y,
				event->area.width, event->area.height);
	}

	return TRUE;
}


/* Mouse pointer enters */

static gint enter_notify_event(GtkWidget *widget, GdkEventExpose *event, CongXMLEditor *xed)
{
	GdkCursor* cursor;

	cursor = gdk_cursor_new(GDK_XTERM);
	gdk_window_set_cursor(cong_gui_get_window(&the_gui)->window, cursor);
	gdk_cursor_destroy(cursor);
	return(TRUE);
}


static gint leave_notify_event(GtkWidget *widget, GdkEventExpose *event, CongXMLEditor *xed)
{
  UNUSED_VAR(GdkCursor* cursor)
	
	gdk_window_set_cursor(cong_gui_get_window(&the_gui)->window, 0);
	return(TRUE);
}


static gint button_press_event(GtkWidget *widget, GdkEventButton *event, CongXMLEditor *xed)
{
	UNUSED_VAR(TTREE *dummy)
	UNUSED_VAR(TTREE *n)
	UNUSED_VAR(TTREE *r)
	
	if (event->button == 1)
	{
#ifndef RELEASE		
		printf("[Click] ");
#endif
		fflush(stdout);
		the_globals.curs.w = widget;
		gtk_widget_grab_focus(widget);
		gtk_widget_grab_default(widget);

		curs_place_in_xed(&the_globals.curs, xed, (int) event->x, (int) event->y);
		selection_start_from_curs(&the_globals.selection, &the_globals.curs);
		selection_end_from_curs(&the_globals.selection, &the_globals.curs);

		xed_redraw(xed);
/*		
		vect_win_update(xed);
 */
		return(TRUE);
	}
	else if (event->button == 3)
	{
		popup_build(xed);
		popup_show(cong_gui_get_popup(&the_gui), event);
		return(TRUE);
	}
	
	return(TRUE);
}


static gint key_press_event(GtkWidget *widget, GdkEventKey *event, CongXMLEditor *xed)
{
	struct pos *pos;
	UNUSED_VAR(char *s)
	int r = FALSE;

#ifndef RELEASE		
	printf("Keyval: %d, State: %d\n", event->keyval, event->state);
#endif
	
	if (event->state && event->state != 1) return(FALSE);
	
#if 0
	fputs(event->string, stdout);
#endif

	curs_off(&the_globals.curs);
		
	switch (event->keyval)
	{
		case GDK_Up:
		  curs_prev_line(&the_globals.curs, xed);
		  gtk_widget_grab_focus(widget);
		  gtk_widget_grab_default(widget);
		  r = TRUE;
		  break;
		case GDK_Down:
		  curs_next_line(&the_globals.curs, xed);
		  gtk_widget_grab_focus(widget);
		  gtk_widget_grab_default(widget);
		  r = TRUE;
		  break;
		case GDK_Left:
		  curs_prev_char(&the_globals.curs, xed);
		  gtk_widget_grab_focus(widget);
		  gtk_widget_grab_default(widget);
		  r = TRUE;
		  break;
		case GDK_Right:
		  curs_next_char(&the_globals.curs, xed);
		  gtk_widget_grab_focus(widget);
		  gtk_widget_grab_default(widget);
		  r = TRUE;
		  break;
		case GDK_BackSpace:
		  curs_del_prev_char(&the_globals.curs, xed);
		  gtk_widget_grab_focus(widget);
		  gtk_widget_grab_default(widget);
		  xed_redraw(xed);
		  r = TRUE;
		  break;
		case GDK_Delete:
		  curs_del_next_char(&the_globals.curs, xed);
		  gtk_widget_grab_focus(widget);
		  gtk_widget_grab_default(widget);
		  xed_redraw(xed);
		  r = TRUE;
		  break;
		case GDK_ISO_Enter:
		case GDK_Return:
		  curs_paragraph_insert(&the_globals.curs);
		  gtk_widget_grab_focus(widget);
		  gtk_widget_grab_default(widget);
		  xed_redraw(xed);
		  r = TRUE;
		  break;
		default:
		
		  if (event->length && event->string && strlen(event->string))
					curs_data_insert(&the_globals.curs, event->string);
		  xed_redraw(xed);
		  break;
	}

	pos = pos_logical_to_physical_new(xed, &the_globals.curs.location);
	the_globals.curs.x = pos->x;
	the_globals.curs.y = pos->y;
	the_globals.curs.line = pos->line;
	free(pos);

	curs_on(&the_globals.curs);
	the_globals.curs.on = 0;

#ifndef RELEASE
	s = xml_fetch_clean_data(xed->x->parent->parent);
	if (s) fputs(s, stdout);
	free(s);
#endif
	
	return(r);
}


static gint motion_notify_event(GtkWidget *widget, GdkEventMotion *event, CongXMLEditor *xed)
{
	if (!(event->state & GDK_BUTTON1_MASK)) return(FALSE);
	
	curs_place_in_xed(&the_globals.curs, xed, (int) event->x, (int) event->y);
	selection_end_from_curs(&the_globals.selection, &the_globals.curs);

	xed_redraw(xed);
	return(TRUE);
}


static gint popup_event(GtkWidget *widget, GdkEvent *event)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		GdkEventButton *bevent = (GdkEventButton *) event;
		if (bevent->button != 3) return(FALSE);

		printf("FIXME:  passing NULL for xed ptr to popup_build\n");
		popup_build(NULL);
		popup_show(cong_gui_get_popup(&the_gui), bevent);
		return(TRUE);
	}

	return(FALSE);
}


static gint selection_received_event(GtkWidget *w, GtkSelectionData *d, CongXMLEditor *xed)
{
  TTREE *dummy;                                                                 
	                                                                                
	#ifndef RELEASE                                                                 
	  printf("In selection_received_event().\n");                                   
	#endif                                                                          

	if (!d->data || d->length < 1) return(TRUE);
	
	  fwrite(d->data, d->length, 1, stdout);                                        
	  fputs("\n", stdout);                                                          
	                                                                                
	  dummy = ttree_node_add(0, "tag_span", 8);                                     
	  ttree_node_add(dummy, "dummy", 5);                                            
	  ttree_node_add(dummy->child, "data", 4);                                      
	  ttree_node_add(dummy->child->child, d->data, d->length);                      
	  the_globals.clipboard = dummy;                                                            
	/*                                                                              
	 *   dummy->child->parent = 0;                                                     
	 *   dummy->child = 0;                                                             
	 *   ttree_branch_remove(dummy);                                                   
	 * */                                                                              
	  xed_paste(the_globals.curs.w, the_globals.curs.xed);                                                  
	  return(TRUE);  
}


CongXMLEditor *xmledit_new(TTREE *x, CongDispspec *displayspec)
{
        UNUSED_VAR(GdkCursor* cursor)
	CongXMLEditor *xed;
	UNUSED_VAR(int sig)
	
	xed = malloc(sizeof(*xed));
	memset(xed, 0, sizeof(*xed));

	xed->x = x;
#if 0	
	xed->e = gtk_event_box_new();
#endif	
	xed->w = gtk_drawing_area_new();
#if 0	
	gtk_container_add(GTK_CONTAINER(xed->e), xed->w);
#endif

	gtk_drawing_area_size(GTK_DRAWING_AREA(xed->w), 200, 0);
	gtk_widget_set_usize(xed->w, 200, 0);

#if 0
	sig = gtk_signal_lookup("key_press_event", GTK_TYPE_WIDGET);
	gtk_signal_disconnect(GTK_OBJECT(xed->w), sig);
#endif
	
#if 1
	/* FIXME: g_message("DHM: removed call to gtk_signal_handlers_destroy here\n"); */
#else
	gtk_signal_handlers_destroy(GTK_OBJECT(xed->w));
#endif
	
  gtk_signal_connect (GTK_OBJECT (xed->w), "expose_event",
											(GtkSignalFunc) expose_event, xed);
	gtk_signal_connect (GTK_OBJECT(xed->w),"configure_event",
											(GtkSignalFunc) configure_event, xed);
	gtk_signal_connect_after (GTK_OBJECT (xed->w), "enter_notify_event",
											(GtkSignalFunc) enter_notify_event, xed);
	gtk_signal_connect_after (GTK_OBJECT (xed->w), "leave_notify_event",
											(GtkSignalFunc) leave_notify_event, xed);
	gtk_signal_connect_after (GTK_OBJECT (xed->w), "key_press_event",
											(GtkSignalFunc) key_press_event, xed);

#if 0	
	gtk_signal_connect_after (GTK_OBJECT (xed->w), "key_release_event",
											(GtkSignalFunc) key_press_event, xed);
#endif

	gtk_signal_connect (GTK_OBJECT (xed->w), "motion_notify_event",
											(GtkSignalFunc) motion_notify_event, xed);
	gtk_signal_connect (GTK_OBJECT (xed->w), "button_press_event",
											(GtkSignalFunc) button_press_event, xed);

  gtk_signal_connect (GTK_OBJECT (xed->w), "selection_received",                
											                      (GtkSignalFunc) selection_received_event, xed);           
	
#if 0	
	gtk_signal_connect_object(GTK_OBJECT(xed->w), "event",
														(GtkSignalFunc) popup_event, GTK_OBJECT(popup));
#endif

	
#if 0
	gtk_signal_connect (GTK_OBJECT (xed->w), "button_release_event",
											(GtkSignalFunc) button_press_event, xed);
#endif
	
	gtk_widget_set_events (xed->w, GDK_EXPOSURE_MASK
												 | GDK_ENTER_NOTIFY_MASK
												 | GDK_LEAVE_NOTIFY_MASK
												 | GDK_BUTTON_PRESS_MASK
												 | GDK_KEY_PRESS_MASK
/*												 | GDK_KEY_RELEASE_MASK */
												 | GDK_POINTER_MOTION_MASK
												 /* | GDK_POINTER_MOTION_HINT_MASK */);

	gtk_widget_set(xed->w, "can_focus", (gboolean) TRUE, 0);
	gtk_widget_set(xed->w, "can_default", (gboolean) TRUE, 0);
	
	xed->f = the_globals.f;
	xed->fm = the_globals.fm;

	xed->f_asc = the_globals.f_asc;
	xed->f_desc = the_globals.f_desc;
	xed->fm_asc = the_globals.fm_asc;
	xed->fm_desc = the_globals.fm_desc;

	xed->tag_height = (xed->fm_asc + xed->fm_desc) / 2;
	if (xed->tag_height < 3) xed->tag_height = 3;
	xed->tag_height += (xed->fm_asc + xed->fm_desc) / 2;

	/* g_message("xed used to clone the TTREE for the displayspec; it now shares it\n"); */
	xed->displayspec = displayspec;
	xed->initial = 1;
	return(xed);
}


void xed_char_put_at_curs(CongXMLEditor *xed, char c)
{
	GdkGC *gc;
	gc = xed->w->style->fg_gc[GTK_STATE_NORMAL];
	
	gdk_draw_text(xed->p, xed->f, gc, 0, 10, &c, 1);
}


void xed_str_put(CongXMLEditor *xed, char *s)
{
	GdkGC *gc;
	gc = xed->w->style->fg_gc[GTK_STATE_NORMAL];

	gdk_draw_string(xed->p, xed->f, gc, 
			xed->draw_pos_x,
			xed->draw_pos_y, 
			s);
}


void stack_print(CongLayoutStackEntry *t)
{
	printf("\n STACK:\n");

#if NEW_STACK_IMPLEMENTATION
	for (; t; t = t->above) {
		printf("<%s %d>\n", t->text, t->line);
	}
#else
	for (; t; t = t->parent) {
		printf("<%s %d>\n", t->data,
		       (int) *(t->child->next->data));
	}
#endif
	printf("\n\n");
}

#define DEBUG_STACK 0

void cong_layout_stack_push(CongLayoutStack *layout_stack, char* s, int line, int pos_x, TTREE *x, int lev)
{
	CongLayoutStackEntry *top_entry;
	CongLayoutStackEntry *new_entry;

	top_entry = cong_layout_stack_top(layout_stack);
#if DEBUG_STACK
	if (top_entry) stack_print(top_entry);
#endif

#if NEW_STACK_IMPLEMENTATION
	new_entry = g_new0(CongLayoutStackEntry,1);
	if (!layout_stack->bottom) layout_stack->bottom = new_entry;

	new_entry->text = g_strdup(s);
	new_entry->line = line;
	new_entry->pos_x = pos_x;
	new_entry->x = x;
	new_entry->lev = lev;	
	new_entry->above = NULL;
	new_entry->below = top_entry;

	if (top_entry) {
		g_assert(top_entry->above==NULL);
		top_entry->above = new_entry;
	}
	     
#else
	new_entry = ttree_node_add(top_entry, s, strlen(s));
	if (!layout_stack->bottom) layout_stack->bottom = new_entry;

	ttree_node_add(new_entry, &(line), sizeof(int));
	ttree_node_add(new_entry, &(pos_x), sizeof(int));
	ttree_node_add(new_entry, &x, sizeof(TTREE *));
	ttree_node_add(new_entry, &lev, sizeof(int));
#endif

#if DEBUG_STACK
	printf("[Tag Push] (%s) line=%d\n", s, line);
#endif

	top_entry = cong_layout_stack_top(layout_stack);
	g_assert(top_entry==new_entry);
#if DEBUG_STACK
	if (top_entry) stack_print(top_entry);
#endif
}

void xed_stack_push(CongXMLEditor *xed, char *s, TTREE *x, int n)
{
	int line, pos_x;
	int lev = 0;

	line = xed->draw_line;
	pos_x = xed->draw_pos_x;
	if (n)
	{
		line++;
		pos_x = 0;
	}

	cong_layout_stack_push(&xed->layout_stack, s, line, pos_x, x, lev);

}

void cong_layout_stack_change_level_of_top_tag(CongLayoutStack *layout_stack, int lev)
{
	CongLayoutStackEntry *t;

	t = cong_layout_stack_top(layout_stack);

#if NEW_STACK_IMPLEMENTATION
	t->lev=lev;
#else
	memcpy(t->child->next->next->next->data, &lev, sizeof(int));
#endif
}

void cong_layout_stack_elevate(CongLayoutStack *layout_stack)
{
	CongLayoutStackEntry *t;
	int i;

	t = cong_layout_stack_top(layout_stack);
	
#if NEW_STACK_IMPLEMENTATION
	for (i = 1; t && t->lev < i; t = t->below, i++) {
		t->lev++;
	}
#else
	for (i = 1; t && (int) *((int *) t->child->next->next->next->data) < i;
	     t = t->parent, i++) {
		((int) *((int *) t->child->next->next->next->data))++;
	}
#endif
}

void cong_layout_stack_compress(CongLayoutStack *layout_stack)
{
	CongLayoutStackEntry *t;
	int i;

	t = cong_layout_stack_top(layout_stack);

	for (i = 0; t; t = cong_layout_stack_entry_below(t), i++) {
#if NEW_STACK_IMPLEMENTATION
		t->lev = i;
#else
		((int) *((int *) t->child->next->next->next->data)) = i;
#endif
	}
}

void cong_layout_stack_pop(CongLayoutStack *layout_stack)
{
	CongLayoutStackEntry *t;

	g_return_if_fail(layout_stack);

	t = cong_layout_stack_bottom(layout_stack);
	if (!t) return;

	t = cong_layout_stack_top(layout_stack);
#if DEBUG_STACK
	if (t) stack_print(t);
#endif
	if (t == layout_stack->bottom) {
		layout_stack->bottom = NULL;
	}

#if NEW_STACK_IMPLEMENTATION
	g_assert(t->above==NULL);
	if (t->below) {
		g_assert(t->below->above==t);
		t->below->above=NULL;
	}

	g_assert(t->text);
	g_free(t->text);

	g_free(t);
#else
	ttree_branch_remove(t);
#endif

#if DEBUG_STACK
	printf("[Tag pop]\n");
#endif
}

void xed_stack_pop(CongXMLEditor *xed)
{
	cong_layout_stack_pop(&xed->layout_stack);
}

CongLayoutStackEntry*
cong_layout_stack_top(CongLayoutStack *layout_stack)
{
	CongLayoutStackEntry *t;

	t = layout_stack->bottom;
	if (!t) {
		return NULL;
	}

	for ( ; cong_layout_stack_entry_next(t); t = cong_layout_stack_entry_next(t)) {
		/* empty */
	}
	return(t);	
}

CongLayoutStackEntry*
cong_layout_stack_bottom(CongLayoutStack *layout_stack)
{
	return layout_stack->bottom;
}

int cong_layout_stack_depth(CongLayoutStack *layout_stack)
{
	CongLayoutStackEntry *t;
	int d;
	
	t = layout_stack->bottom;
	if (!t) return(0);
	
	for (d = 1; cong_layout_stack_entry_next(t); t = cong_layout_stack_entry_next(t)) {
		d++;
	}

	return(d);
}

CongLayoutStackEntry*
cong_layout_stack_entry_next(CongLayoutStackEntry *entry)
{
	g_return_val_if_fail(entry, NULL);

#if NEW_STACK_IMPLEMENTATION
	return entry->above;
#else
	return entry->child->next->next->next->next;
#endif
}

CongLayoutStackEntry*
cong_layout_stack_entry_below(CongLayoutStackEntry *entry)
{
	g_return_val_if_fail(entry, NULL);

#if NEW_STACK_IMPLEMENTATION
	return entry->below;
#else
	return entry->parent;
#endif
}

char *cong_layout_stack_entry_get_text(CongLayoutStackEntry *entry) 
{
	g_return_val_if_fail(entry, NULL);

#if NEW_STACK_IMPLEMENTATION
	return entry->text;
#else
	return entry->data;
#endif
}

int cong_layout_stack_entry_get_line(CongLayoutStackEntry *entry) 
{
	g_return_val_if_fail(entry, 0);

#if NEW_STACK_IMPLEMENTATION
	return entry->line;
#else
	return *((int*) entry->child->data);
#endif
}

int cong_layout_stack_entry_get_pos_x(CongLayoutStackEntry *entry)
{
	g_return_val_if_fail(entry, 0);

#if NEW_STACK_IMPLEMENTATION
	return entry->pos_x;
#else
	return *((int *) entry->child->next->data);
#endif
}

#if 0
TTREE *cong_layout_stack_entry_get_ttree_x(CongLayoutStackEntry *entry)
{
	g_return_val_if_fail(entry, NULL);

}
#endif

int cong_layout_stack_entry_get_lev(CongLayoutStackEntry *entry)
{
	g_return_val_if_fail(entry, 0);

#if NEW_STACK_IMPLEMENTATION
	return entry->lev;
#else
	return *((int *) entry->child->next->next->next->data);
#endif
}


TTREE *xed_line_last(CongXMLEditor *xed)
{
	return cong_layout_cache_get_last_line(&xed->layout_cache);
}


/* xed->draw_pos_x = last pixel of line */

GdkGC *get_gc_for_stack_entry(CongDispspec *ds, CongLayoutStackEntry *entry, int tog)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, cong_layout_stack_entry_get_text(entry));

	if (element) {
		return cong_dispspec_element_gc(element);
	} else {
		return NULL;
	}
}

const char *get_uistring_for_stack_entry(CongDispspec *ds, CongLayoutStackEntry *entry)
{
	const char* codename = cong_layout_stack_entry_get_text(entry);
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, codename);
	if (element) {
		return (char*)cong_dispspec_element_username(element);
	}
  
	return codename;
}		


void xed_xml_tags_draw_eol(CongXMLEditor *xed, int draw_tag_lev, int mode)
{
	TTREE *l;
	CongLayoutStackEntry *t;
	GdkGC *gc;
	int draw_pos_y;

	UNUSED_VAR(int line)
	int x0, x1, width, text_width;
	int y, text_y;
	
	CongDispspec *ds = xed->displayspec;

	UNUSED_VAR(int eol = 0)

#if 0
	printf("xed_xml_tags_draw_eol\n");
#endif

	l = xed_line_last(xed);

	/* Draw all tags starting on this line, spanning the end */

	t = cong_layout_stack_top(&xed->layout_stack);

	draw_pos_y = xed->draw_pos_y + xed->f_desc + 3;

	draw_pos_y += draw_tag_lev * xed->tag_height;


	for ( ; t && cong_layout_stack_entry_get_line(t) == xed->draw_line; t = cong_layout_stack_entry_below(t))
	{
		x0 = cong_layout_stack_entry_get_pos_x(t);
		x1 = xed->w->allocation.width;
		width = x1 - x0;

		draw_pos_y = cong_layout_stack_entry_get_lev(t) * xed->tag_height + xed->draw_pos_y + xed->f_desc + 3;

		y = draw_pos_y;

		if (mode == 1 && (gc = get_gc_for_stack_entry(xed->displayspec, t, 0)))
		{
			UNUSED_VAR(TTREE *n0)
			UNUSED_VAR(TTREE *n1)
			UNUSED_VAR(unsigned int col)
			
			/* Insert text if it fits */

			text_width = gdk_string_width(xed->fm, get_uistring_for_stack_entry(ds, t));
			if (text_width < width - 6)
			{
				text_y = y + (xed->fm_asc + xed->fm_desc) / 2;
				
				/* Draw text and lines */
				
				gdk_draw_line(xed->p, gc, x0, y, x0, y - 2);
				gdk_draw_string(xed->p, xed->fm, gc, x0 + 1 + (width - text_width) / 2,
												text_y, get_uistring_for_stack_entry(ds, t));
				gdk_draw_line(xed->p, gc, x0, y, x0 - 1 + (width - text_width) / 2, y);
				gdk_draw_line(xed->p, gc, x1 + 1 - (width - text_width) / 2, y, x1, y);
				gdk_draw_line(xed->p, gc, x1, y, x1, y - 2);
			}
			else
			{
				gdk_draw_line(xed->p, gc, x0, y, x0, y - 2);  /* Tick */
				gdk_draw_line(xed->p, gc, x0, y, x1, y);
			}
		}
	}

	/* Draw all tags spanning the whole line */

	if (t) {
		draw_pos_y = cong_layout_stack_entry_get_lev(t) * xed->tag_height + xed->draw_pos_y + xed->f_desc + 3;
	}

#if 0	
	xed->draw_pos_y = draw_pos_y;
#endif
	
	for ( ; t && cong_layout_stack_entry_get_line(t) <= xed->draw_line; t = cong_layout_stack_entry_below(t) )
	{
		x0 = 0;
		x1 = xed->w->allocation.width;
		width = x1;
		
#if 0		
		y = xed->draw_pos_y;
		xed->draw_pos_y += xed->tag_height;
#else
		y = draw_pos_y;
		draw_pos_y += xed->tag_height;
#endif

		if (mode == 1 && (gc = get_gc_for_stack_entry(xed->displayspec, t, 0)))
		{
			UNUSED_VAR(TTREE *n0)
			UNUSED_VAR(TTREE *n1)
			UNUSED_VAR(unsigned int col)
			
			/* Insert text if it fits */

			text_width = gdk_string_width(xed->fm, get_uistring_for_stack_entry(ds, t));
			if (text_width < width - 6)
			{
				text_y = y + (xed->fm_asc + xed->fm_desc) / 2;

				/* Draw text and lines */
				
				gdk_draw_string(xed->p, xed->fm, gc, x0 + 1 + (width - text_width) / 2,
												text_y, get_uistring_for_stack_entry(ds, t));
				gdk_draw_line(xed->p, gc, x0, y, x0 - 1 + (width - text_width) / 2, y);
				gdk_draw_line(xed->p, gc, x1 + 1 - (width - text_width) / 2, y, x1, y);
			}
			else gdk_draw_line(xed->p, gc, x0, y, x1, y);
		}
	}
	
	if (xed->draw_pos_y + xed->f_desc + 3 + (xed->draw_tag_max * xed->tag_height) > draw_pos_y)
		xed->draw_pos_y += xed->f_desc + 3 + (xed->draw_tag_max * xed->tag_height);
	else
		xed->draw_pos_y = draw_pos_y;
}


void xed_xml_tags_draw_eot(CongXMLEditor *xed, int draw_tag_lev, int mode)
{
	TTREE *l;
	CongLayoutStackEntry *t;
	GdkGC *gc;
	int draw_pos_y;
	int line;
	int x0, x1, width, text_width;
	int y, text_y;
	UNUSED_VAR(unsigned int col)

	CongDispspec *ds = xed->displayspec;

#if 0
	printf("xed_xml_tags_draw_eot\n");
#endif

	/* --- Set drawing style --- */
#if 0
	gc = gdk_gc_new(window->window);  /* FIXME: Application specific */
	gdk_gc_copy(gc, xed->w->style->fg_gc[GTK_STATE_NORMAL]);
#endif
	
	l = xed_line_last(xed);

	/* Draw tag on top of stack */

	draw_pos_y = xed->draw_pos_y + xed->f_desc + 3 + (xed->tag_height * draw_tag_lev);

	t = cong_layout_stack_top(&xed->layout_stack);
	line = cong_layout_stack_entry_get_line(t);

	if (line == xed->draw_line)  /* Opened on this line */
	{
		x0 = cong_layout_stack_entry_get_pos_x(t);
		x1 = xed->draw_pos_x;
#if 0
		printf("[Tag Draw] (%s) %d - %d.\n", t->data, x0, x1);
#endif
		width = x1 - x0;
	}
	else                         /* Opened on prior line */
	{
		x0 = 0;
		x1 = xed->draw_pos_x;
		width = x1;
	}

	y = draw_pos_y;
#if 0
	if (x1 > x0) { x1 -= 4; width -= 4; }
#endif
	if (mode == 1 && (gc = get_gc_for_stack_entry(ds, t, 0)))
	{
	  UNUSED_VAR(TTREE *n0)
	  UNUSED_VAR(TTREE *n1)

		/* Insert text if it fits */

		text_width = gdk_string_width(xed->fm, get_uistring_for_stack_entry(ds, t));
		if (text_width < width - 6)
		{
			text_y = y + (xed->fm_asc + xed->fm_desc) / 2;
			
			/* Draw text and lines */
			
			if (line == xed->draw_line) gdk_draw_line(xed->p, gc, x0, y, x0, y - 2);
			gdk_draw_string(xed->p, xed->fm, gc, x0 + 1 + (width - text_width) / 2,
					text_y, get_uistring_for_stack_entry(ds, t));
			gdk_draw_line(xed->p, gc, x0, y, x0 - 1 + (width - text_width) / 2, y);
			gdk_draw_line(xed->p, gc, x1 + 1 - (width - text_width) / 2, y, x1, y);
			gdk_draw_line(xed->p, gc, x1, y, x1, y - 2);
		}
		else
		{
			/* Draw lines */
			
			if (line == xed->draw_line) gdk_draw_line(xed->p, gc, x0, y, x0, y - 2);
			gdk_draw_line(xed->p, gc, x0, y, x1, y);
			gdk_draw_line(xed->p, gc, x1, y, x1, y - 2);
		}
	}
}


void xed_str_micro_put(CongXMLEditor *xed, char *s)
{
	GdkGC *gc;
	gc = xed->w->style->fg_gc[GTK_STATE_NORMAL];

	gdk_draw_string(xed->p, xed->fm, gc, 0, 30, s);
}


char *xed_word(TTREE *x, CongXMLEditor *xed, int *spc_before, int *spc_after)
{
	UNUSED_VAR(int i)
	char *p0, *p1;

	if (*spc_after) { *spc_before = 1; *spc_after = 0; }
	else *spc_before = 0;

	xed->draw_x_prev = x;
	xed->draw_char_prev = xed->draw_char;

	/* Skip spaces before word */

	for (p0 = xml_frag_data_nice(x) + xed->draw_char;
	     p0 && (*p0 == ' ' || *p0 == '\n'); p0++, xed->draw_char++) {
		*spc_before = 1;
	}

	if (!p0) return(0);
	
	/* Start at current position */
	/* FIXME: Not needed */

	p0 = xml_frag_data_nice(x) + xed->draw_char;

	/* Return NULL if node ended */

	if (!p0 || !*p0)
	{
		xed->draw_char = 0;
		return(0);
	}

	/* Span word: p0 = start, p1 = end */

	for (p1 = p0; *p1 && *p1 != ' ' && *p1 != '\n'; p1++) {
		/* empty */
	}

	if (p0 == p1) return(0);  /* FIXME: Should never happen */
	xed->draw_char += (p1 - p0);

	if (*p1) {
		*spc_after = 1;
	} else {
		*spc_after = 0;
	}
	return(strndup(p0, p1 - p0));
}


void xed_word_rewind(CongXMLEditor *xed)
{
	xed->draw_x = xed->draw_x_prev;
	xed->draw_char = xed->draw_char_prev;
}


/* --- */


int xed_word_first_would_wrap(TTREE *x, CongXMLEditor *xed)
{
	char *p0, *p1, *word;
	int width;

	x = xml_frag_enter(x);

	for ( ; x; x = xml_frag_next(x))
	{
		if (xml_frag_type(x) == XML_TAG_SPAN)
		{
			return(xed_word_first_would_wrap(x, xed));
		}
		else if (xml_frag_type(x) == XML_DATA)
		{
			/* Isolate first word */
			
			p0 = xml_frag_data_nice(x);
			
			/* Skip spaces before word */

			for ( ; *p0 == ' ' || *p0 == '\n'; p0++) {
				/* empty */
			}
			
			if (*p0)
			{
				/* Span word: p0 = start, p1 = end */

				for (p1 = p0; *p1 && *p1 != ' ' && *p1 != '\n'; p1++) {
					/* empty */
				}
				
				/* Get width of word */
				word = strndup(p0, p1 - p0);
				width = gdk_string_width(xed->f, word);
				free(word);

				/* Does it fit? */
				if (xed->draw_pos_x + width > xed->w->allocation.width) {
					return(1);
				} else {
					return(0);
				}
			}
		}
	}
	
	return(0);
}


/* --- */

int xed_xml_content_data(CongXMLEditor *xed, TTREE *x, int draw_tag_lev)
{
	char *word;
	int width;
	UNUSED_VAR(int wrap = 0)
	int spc_before = 0, spc_after = 0;

#if 0	
	xed->draw_char = 0;
#endif
	
	/* Lay out text */

	for (word = xed_word(x, xed, &spc_before, &spc_after); word; word = xed_word(x, xed, &spc_before, &spc_after))
	{
		/* Get width of word */
		width = gdk_string_width(xed->f, word);
		if (spc_before && xed->draw_pos_x > 1) {
			width += gdk_char_width(xed->f, ' ');
		}

		/* Does it fit? */
		if (xed->draw_pos_x && xed->draw_pos_x + width > xed->w->allocation.width)
		{
#if DEBUG_STACK
			printf("Wrap at x = %d.\n", xed->draw_pos_x);
#endif
			free(word);
			xed_word_rewind(xed);
			return(1);
		}

		/* Draw and move pen */

		if (spc_before && xed->draw_pos_x > 1) {
			xed->draw_pos_x += gdk_char_width(xed->f, ' ');
		}
		if (xed->mode == 1) {
			xed_str_put(xed, word);
		}

		width = gdk_string_width(xed->f, word);
		xed->draw_pos_x += width;  /* NOTE: Causes tag misalignment? (Nah...) */
		free(word);
	}
#if DEBUG_STACK
	printf("[Data] draw_pos_x == %d.\n", xed->draw_pos_x);
#endif
	if (spc_before) xed->draw_pos_x += gdk_char_width(xed->f, ' ');
	return(0);
}


/* --- */

int xed_xml_content_data_root(CongXMLEditor *xed, TTREE *x, int draw_tag_lev)
{
	char *word;
	int width;
	int wrap = 0;
	int spc_before = 0, spc_after = 0;

	xed->draw_char = 0;

	/* Lay out text */

	for (word = xed_word(x, xed, &spc_before, &spc_after); word; word = xed_word(x, xed, &spc_before, &spc_after))
	{
		/* Get width of word */
		width = gdk_string_width(xed->f, word);
		if (spc_before && xed->draw_pos_x > 1) {
			width += gdk_char_width(xed->f, ' ');
		}

		/* Does it fit? */
		if (xed->draw_pos_x + width > xed->w->allocation.width)
		{
			/* Linewrap */

			xed->draw_pos_y += 8;                      /* Fixed line spacing */
			xed->draw_pos_y += draw_tag_lev * xed->tag_height;

#if 1
			add_stuff(xed, xed->draw_pos_y, x, xed->draw_char);
#else
			ttree_node_add(xed->draw_line_t, &xed->draw_pos_y, sizeof(int));
			ttree_node_add(xed->draw_line_t, &x, sizeof(TTREE *));
			ttree_node_add(xed->draw_line_t, &xed->draw_char, sizeof(int));
#endif

#if 1
			add_line(xed, xed->draw_x_prev, xed->draw_char_prev);
#else
			xed->draw_line_t = ttree_node_add(xed->lines, "line", 4);  /* Add line */
			ttree_node_add(xed->draw_line_t, &xed->draw_x_prev, sizeof(TTREE *));
			ttree_node_add(xed->draw_line_t, &xed->draw_char_prev, sizeof(int));
#endif

			xed->draw_pos_y += xed->f_asc;             /* Super-baseline text */
			xed->draw_line += 1;                       /* Goto next line */
			xed->draw_pos_x = 1;                       /* Start at left margin */
			xed->draw_tag_max = 0;
			draw_tag_lev = 0;

			wrap = 1;
		}

		/* Draw and move pen */

		if (spc_before && xed->draw_pos_x > 1) {
			xed->draw_pos_x += gdk_char_width(xed->f, ' ');
		}
		if (xed->mode == 1) {
			xed_str_put(xed, word);
		}

		width = gdk_string_width(xed->f, word);
		xed->draw_pos_x += width;  /* NOTE: Causes tag misalignment? (Nah...) */
		free(word);
	}

	if (spc_before && xed->draw_pos_x > 1) {
		xed->draw_pos_x += gdk_char_width(xed->f, ' ');
	}
	return(wrap);
}


/* --- */


int xed_xml_depth(TTREE *x)
{
	int d = 0, d_max = 0;

	x = xml_frag_enter(x);

	for (d = d_max = 0; x; x = xml_frag_next(x))
	{
		if (xml_frag_type(x) == XML_TAG_SPAN)
		{
			d = xed_xml_depth(x);
			if (d > d_max) {
				d_max = d;
			}
		}
	}

	return(d_max + 1);
}


/* --- */

int xed_xml_depth_after_eol(CongXMLEditor *xed, TTREE *x)
{
	int d = 0, d_max = 0;

	for (d = d_max = 0; x; x = xml_frag_next(x))
	{
		if (xml_frag_type(x) == XML_TAG_SPAN)
		{
			d = xed_xml_depth_after_eol(xed, xml_frag_enter(x));
			if (d > d_max) {
				d_max = d;
			}
		}
	}
	
	return(d_max + 1);
}


/* --- */


int xed_xml_depth_before_eol(CongXMLEditor *xed, TTREE *x, int pos_x, int width, int *eol_p)
{
	int d = 0, d_max = 0;
	int eol = 0;

	x = xml_frag_enter(x);

	for (d = d_max = 0; x; x = xml_frag_next(x))
	{
		if (xml_frag_type(x) == XML_DATA)
		{
			pos_x += gdk_string_width(xed->f, xml_frag_data_nice(x));
			if (pos_x > width)
			{
				*eol_p = 1;
				return(d_max + 1);
			}
		}
		else if (xml_frag_type(x) == XML_TAG_SPAN)
		{
			d = xed_xml_depth_before_eol(xed, x, pos_x, width, &eol);
			if (d > d_max) {
				d_max = d;
			}
			if (eol) { 
				*eol_p = 1; 
				return(d_max + 1); 
			}
		}
	}

	return(d_max + 1);
}


/* --- */


int xed_xml_content_tag(CongXMLEditor *xed, TTREE *x)
{
	UNUSED_VAR(int hold = 0)
	int draw_tag_lev;
	UNUSED_VAR(int i0)
	UNUSED_VAR(int pushed = 0)
	int draw_tag_lev_new;
	int width;
	CongDispspec *ds = xed->displayspec;

#if 0
	draw_tag_lev = xed_xml_depth(x) - 1;
#else
	draw_tag_lev = 0;
#endif

#if 1
	if (xed_word_first_would_wrap(x, xed))
	{
		xed_stack_push(xed, xml_frag_name_nice(x), x, 1);
	}
	else
	{
#endif		
		cong_layout_stack_elevate(&xed->layout_stack);
		xed_stack_push(xed, xml_frag_name_nice(x), x, 0);
#if 1
	}
#endif

#if DEBUG_STACK
	printf("> %s (%d)  ", xml_frag_name_nice(x), draw_tag_lev);
	fflush(stdout);
#endif

	/* Goto first child fragment */

	x = xml_frag_enter(x);
	if (!x)
	{
		xed_stack_pop(xed);
		return(0);
	}

	xed->draw_char = 0;
	
	/* Process all children */

	for (; x; )
	{
		int type = xml_frag_type(x);
		char *name = xml_frag_name_nice(x);

		if (type == XML_TAG_SPAN && cong_dispspec_element_span(xed->displayspec, name) &&
				strcasecmp("table", name))
		{
			draw_tag_lev_new = xed_xml_content_tag(xed, x);
			if (draw_tag_lev_new > draw_tag_lev)
			{
				draw_tag_lev = draw_tag_lev_new;
			}
		}
		else if (type == XML_DATA)
		{
#if DEBUG_STACK
			printf("Data node, x = %d.\n", xed->draw_pos_x);
#endif			
			if (xed_xml_content_data(xed, x, draw_tag_lev + 1))
			{
				/* Linewrap */

				xed_xml_tags_draw_eol(xed, draw_tag_lev, xed->mode);
				xed->draw_pos_y += 8;                      /* Fixed line spacing */
#if 1
				add_stuff(xed, xed->draw_pos_y, x, xed->draw_char);
#else
				ttree_node_add(xed->draw_line_t, &xed->draw_pos_y, sizeof(int));
				ttree_node_add(xed->draw_line_t, &x, sizeof(TTREE *));
				ttree_node_add(xed->draw_line_t, &xed->draw_char, sizeof(int));
#endif

				xed->draw_line += 1;                       /* Goto next line */
				xed->draw_pos_x = 1;                       /* Start at left margin */
				xed->draw_tag_max = 0;

				add_line(xed, xed->draw_x_prev, xed->draw_char_prev);

				xed->draw_pos_y += xed->f_asc;             /* Super-baseline text */
				draw_tag_lev = 0;
				cong_layout_stack_compress(&xed->layout_stack);
				continue;
			}
		}
		else if (type == XML_TAG_EMPTY && CONG_ELEMENT_TYPE_PARAGRAPH==cong_dispspec_type(ds, name))
		{
			/* Paragraph. Implies linewrap */
#if 0
			printf("got paragraph in xed_xml_content_tag, tag=<%s>\n", name);
#endif
			
			if (xml_frag_next(x)) xed->draw_x_prev = xml_frag_next(x);
			else
			{
			  for (xed->draw_x_prev = x; ; )
			  {
				  xed->draw_x_prev = xml_frag_exit(xed->draw_x_prev);
				  if (xml_frag_next(xed->draw_x_prev))
				  {
					  xed->draw_x_prev = xml_frag_next(xed->draw_x_prev);
					  break;
				  }
			  }
			}
			
			xed->draw_char_prev = 0;

			/* Linewrap */

			xed_xml_tags_draw_eol(xed, draw_tag_lev, xed->mode);
			xed->draw_pos_y += 32;                      /* Fixed line spacing */

#if 1
			add_stuff(xed, xed->draw_pos_y, x, xed->draw_char);
#else
			ttree_node_add(xed->draw_line_t, &xed->draw_pos_y, sizeof(int));
			ttree_node_add(xed->draw_line_t, &x, sizeof(TTREE *));
			ttree_node_add(xed->draw_line_t, &xed->draw_char, sizeof(int));
#endif

			xed->draw_line += 1;                       /* Goto next line */
			xed->draw_pos_x = 1;                       /* Start at left margin */
			xed->draw_tag_max = 0;

#if 1
			add_line(xed, xed->draw_x_prev, xed->draw_char_prev);
#else
			xed->draw_line_t = ttree_node_add(xed->lines, "line", 4);  /* Add line */
			ttree_node_add(xed->draw_line_t, &xed->draw_x_prev, sizeof(TTREE *));
			ttree_node_add(xed->draw_line_t, &xed->draw_char_prev, sizeof(int));
#endif

			xed->draw_pos_y += xed->f_asc;             /* Super-baseline text */
			draw_tag_lev = 0;
			cong_layout_stack_compress(&xed->layout_stack);
		}
		else if (type == XML_TAG_SPAN && !strcasecmp("table", name))
		{
			/* TABLE. Identifier on separate line */

			if (xml_frag_next(x)) {
				xed->draw_x_prev = xml_frag_next(x);
			} else 	{
				for (xed->draw_x_prev = x; ; ) {
					xed->draw_x_prev = xml_frag_exit(xed->draw_x_prev);
					if (xml_frag_next(xed->draw_x_prev)) {
						xed->draw_x_prev = xml_frag_next(xed->draw_x_prev);
						break;
					}
				}
			}
			
			xed->draw_char_prev = 0;
			xed_xml_tags_draw_eol(xed, draw_tag_lev, xed->mode);
			xed->draw_pos_y += 8;                      /* Fixed line spacing */
			xed->draw_pos_y += draw_tag_lev * xed->tag_height;

			/* --- */
			
#if 1
			add_stuff(xed, xed->draw_pos_y, x, xed->draw_char);
#else
			ttree_node_add(xed->draw_line_t, &xed->draw_pos_y, sizeof(int));
			ttree_node_add(xed->draw_line_t, &x, sizeof(TTREE *));
			ttree_node_add(xed->draw_line_t, &xed->draw_char, sizeof(int));
#endif

#if 1
			add_line(xed, xed->draw_x_prev, xed->draw_char_prev);
#else
			xed->draw_line_t = ttree_node_add(xed->lines, "line", 4);  /* Add line */
			ttree_node_add(xed->draw_line_t, &xed->draw_x_prev, sizeof(TTREE *));
			ttree_node_add(xed->draw_line_t, &xed->draw_char_prev, sizeof(int));
#endif
			
			xed->draw_pos_y += xed->f_asc;             /* Super-baseline text */
			xed->draw_line += 1;                       /* Goto next line */
			xed->draw_pos_x = 1;                       /* Start at left margin */
			xed->draw_tag_max = 0;
			draw_tag_lev = 0;

			/* Draw table thing */
			
			width = gdk_string_width(xed->f, "[TABLE]");
			if (xed->mode == 1)
			{
				gdk_draw_rectangle(xed->p, the_globals.insert_element_gc,
						   TRUE, 2, xed->draw_pos_y - xed->f_asc - 1, xed->w->allocation.width - 4,
						   xed->f_asc + xed->f_desc + 2);
				gdk_draw_string(xed->p, xed->f, xed->w->style->fg_gc[GTK_STATE_NORMAL],
						(xed->w->allocation.width - width) / 2,
						xed->draw_pos_y, "[TABLE]");
			}
			
			xed->draw_char_prev = 0;
			xed->draw_pos_y += 8;                      /* Fixed line spacing */
			xed->draw_pos_y += draw_tag_lev * xed->tag_height;

			/* --- */
			
#if 1
			add_stuff(xed, xed->draw_pos_y, x, xed->draw_char);
#else
			ttree_node_add(xed->draw_line_t, &xed->draw_pos_y, sizeof(int));
			ttree_node_add(xed->draw_line_t, &x, sizeof(TTREE *));
			ttree_node_add(xed->draw_line_t, &xed->draw_char, sizeof(int));
#endif

#if 1
			add_line(xed, xed->draw_x_prev, xed->draw_char_prev);
#else
			xed->draw_line_t = ttree_node_add(xed->lines, "line", 4);  /* Add line */
			ttree_node_add(xed->draw_line_t, &xed->draw_x_prev, sizeof(TTREE *));
			ttree_node_add(xed->draw_line_t, &xed->draw_char_prev, sizeof(int));
#endif

			xed->draw_pos_y += xed->f_asc;             /* Super-baseline text */
			xed->draw_line += 1;                       /* Goto next line */
			xed->draw_pos_x = 1;                       /* Start at left margin */
			xed->draw_tag_max = 0;
			draw_tag_lev = 0;
		}
		
		xed->draw_char = 0;  /* Beginning of next data node */
		
		x = xml_frag_next(x);
	}

#if DEBUG_STACK
	printf("[Tag End] %d.\n", xed->draw_pos_x);
#endif
	
	xed_xml_tags_draw_eot(xed, draw_tag_lev, xed->mode);
	xed_stack_pop(xed);
#if DEBUG_STACK
	printf("< (%d)  ", draw_tag_lev);
	fflush(stdout);
#endif

	xed->draw_char = 0;
	draw_tag_lev += 1;
	if (draw_tag_lev > xed->draw_tag_max) {
		xed->draw_tag_max = draw_tag_lev;
	}
	return(draw_tag_lev);
}



/* modes:
 * 
 * 0 = calculate height only
 * 1 = draw and calculate height
 *
 */

int xed_xml_content_draw(CongXMLEditor *xed, unsigned int mode)
{
	TTREE *x;
	UNUSED_VAR(TTREE *first)
	UNUSED_VAR(int height = 0)
	int draw_tag_lev = 0, draw_tag_lev_new;
	int width;
	CongDispspec *ds = xed->displayspec;

#if 0
	printf("xed_xml_content_draw\n");
#endif

	xed->mode = mode;
	xed->draw_tag_max = 0;
	xed->draw_line = 0;
	xed->draw_pos_x = 1;
	xed->draw_pos_y = 0;
	xed->draw_x = xed->x;
	xed->draw_x_prev = 0;
	xed->draw_char_prev = 0;
	xed->draw_char = 0;
	xed->draw_pos_y = xed->f_asc;

	/* We start inside a parent element, which hopefully is structural. */
	
#if 0
	x = first = xml_frag_enter(x);
#endif

	cong_layout_cache_init(&xed->layout_cache);

#if 1
	add_line(xed, xed->x, xed->draw_char);
#else
	xed->draw_line_t = ttree_node_add(xed->lines, "line", 4);
	ttree_node_add(xed->draw_line_t, &xed->x, sizeof(TTREE *));
	ttree_node_add(xed->draw_line_t, &xed->draw_char, sizeof(int));
#endif

	for (x = xed->x; x; x = xml_frag_next(x))
	{
		int type = xml_frag_type(x);
		char *name = xml_frag_name_nice(x);

		if (type == XML_TAG_SPAN && strcasecmp("table", name)) {
			if (cong_dispspec_element_span(xed->displayspec, name) /* ||
										  cong_dispspec_element_insert(xed->displayspec, name) */ ) {
				draw_tag_lev_new = xed_xml_content_tag(xed, x);
				if (draw_tag_lev_new > draw_tag_lev) {
					draw_tag_lev = draw_tag_lev_new;
				}
			}
			else if (cong_dispspec_element_structural(xed->displayspec, name)) break;
		}
		else if (type == XML_DATA)
		{
			if (xed_xml_content_data_root(xed, x, draw_tag_lev)) { 
				draw_tag_lev = 0;
			}
		}
		else if (type == XML_TAG_EMPTY && CONG_ELEMENT_TYPE_PARAGRAPH==cong_dispspec_type(ds, name))
		{
			/* Linewrap */
#if 0
			printf("got paragraph in xed_xml_content_draw, name=<%s>\n",name);
#endif

			if (xml_frag_next(x)) {
				xed->draw_x_prev = xml_frag_next(x);
			}
			else
			{
				for (xed->draw_x_prev = x; ; ) {
					xed->draw_x_prev = xml_frag_exit(xed->draw_x_prev);
					if (xml_frag_next(xed->draw_x_prev)) {
						xed->draw_x_prev = xml_frag_next(xed->draw_x_prev);
						break;
					}
				}
			}
			
			xed->draw_char_prev = 0;
			
			/* --- */
			
			xed->draw_pos_y += 32;                      /* Fixed line spacing */
			xed->draw_pos_y += draw_tag_lev * xed->tag_height;

#if 1
			add_stuff(xed, xed->draw_pos_y, x, xed->draw_char);
#else
			ttree_node_add(xed->draw_line_t, &xed->draw_pos_y, sizeof(int));
			ttree_node_add(xed->draw_line_t, &x, sizeof(TTREE *));
			ttree_node_add(xed->draw_line_t, &xed->draw_char, sizeof(int));
#endif

#if 1
			add_line(xed, xed->draw_x_prev, xed->draw_char_prev);
#else
			xed->draw_line_t = ttree_node_add(xed->lines, "line", 4);  /* Add line */
			ttree_node_add(xed->draw_line_t, &xed->draw_x_prev, sizeof(TTREE *));
			ttree_node_add(xed->draw_line_t, &xed->draw_char_prev, sizeof(int));
#endif
			
			xed->draw_pos_y += xed->f_asc;             /* Super-baseline text */
			xed->draw_line += 1;                       /* Goto next line */
			xed->draw_pos_x = 1;                       /* Start at left margin */
			xed->draw_tag_max = 0;
			draw_tag_lev = 0;
		}
		else if (type == XML_TAG_SPAN && !strcasecmp("table", name))
		{
			/* TABLE. Identifier on separate line */

			if (xml_frag_next(x)) {
				xed->draw_x_prev = xml_frag_next(x);
			}
			else
			{
				for (xed->draw_x_prev = x; ; ) {
					xed->draw_x_prev = xml_frag_exit(xed->draw_x_prev);
					if (xml_frag_next(xed->draw_x_prev)) {
						xed->draw_x_prev = xml_frag_next(xed->draw_x_prev);
						break;
					}
				}
			}
			
			xed->draw_char_prev = 0;
			xed->draw_pos_y += 8;                      /* Fixed line spacing */
			xed->draw_pos_y += draw_tag_lev * xed->tag_height;

			/* --- */
			
#if 1
			add_stuff(xed, xed->draw_pos_y, x, xed->draw_char);
#else
			ttree_node_add(xed->draw_line_t, &xed->draw_pos_y, sizeof(int));
			ttree_node_add(xed->draw_line_t, &x, sizeof(TTREE *));
			ttree_node_add(xed->draw_line_t, &xed->draw_char, sizeof(int));
#endif

#if 1
			add_line(xed, xed->draw_x_prev, xed->draw_char_prev);
#else
			xed->draw_line_t = ttree_node_add(xed->lines, "line", 4);  /* Add line */
			ttree_node_add(xed->draw_line_t, &xed->draw_x_prev, sizeof(TTREE *));
			ttree_node_add(xed->draw_line_t, &xed->draw_char_prev, sizeof(int));
#endif
			
			xed->draw_pos_y += xed->f_asc;             /* Super-baseline text */
			xed->draw_line += 1;                       /* Goto next line */
			xed->draw_pos_x = 1;                       /* Start at left margin */
			xed->draw_tag_max = 0;
			draw_tag_lev = 0;

			/* Draw table thing */
			
			width = gdk_string_width(xed->f, "[TABLE]");
			if (xed->mode == 1)
			{
				gdk_draw_rectangle(xed->p, the_globals.insert_element_gc,
						   TRUE, 2, xed->draw_pos_y - xed->f_asc - 1, xed->w->allocation.width - 4,
						   xed->f_asc + xed->f_desc + 2);
				gdk_draw_string(xed->p, xed->f, xed->w->style->fg_gc[GTK_STATE_NORMAL],
						(xed->w->allocation.width - width) / 2,
						xed->draw_pos_y, "[TABLE]");
			}
			
			xed->draw_char_prev = 0;
			xed->draw_pos_y += 8;                      /* Fixed line spacing */
			xed->draw_pos_y += draw_tag_lev * xed->tag_height;

			/* --- */
			
#if 1
			add_stuff(xed, xed->draw_pos_y, x, xed->draw_char);
#else
			ttree_node_add(xed->draw_line_t, &xed->draw_pos_y, sizeof(int));
			ttree_node_add(xed->draw_line_t, &x, sizeof(TTREE *));
			ttree_node_add(xed->draw_line_t, &xed->draw_char, sizeof(int));
#endif

#if 1
			add_line(xed, xed->draw_x_prev, xed->draw_char_prev);
#else
			xed->draw_line_t = ttree_node_add(xed->lines, "line", 4);  /* Add line */
			ttree_node_add(xed->draw_line_t, &xed->draw_x_prev, sizeof(TTREE *));
			ttree_node_add(xed->draw_line_t, &xed->draw_char_prev, sizeof(int));
#endif

			xed->draw_pos_y += xed->f_asc;             /* Super-baseline text */
			xed->draw_line += 1;                       /* Goto next line */
			xed->draw_pos_x = 1;                       /* Start at left margin */
			xed->draw_tag_max = 0;
			draw_tag_lev = 0;
		}
	}

	xed_xml_tags_draw_eol(xed, draw_tag_lev, xed->mode);
#if 1
	add_stuff(xed, xed->draw_pos_y, x, xed->draw_char);
#else
	ttree_node_add(xed->draw_line_t, &xed->draw_pos_y, sizeof(int));
/*	ttree_node_add(xed->draw_line_t, &xed->x, sizeof(TTREE *)); */
	ttree_node_add(xed->draw_line_t, &x, sizeof(TTREE *));
	ttree_node_add(xed->draw_line_t, &xed->draw_char, sizeof(int));
#endif
	return(xed->draw_pos_y);
}


/* --- Cut/copy/paste --- */

void selection_curs_unset()
{
	the_globals.curs.set = 0;
	cong_location_nullify(&the_globals.curs.location);
	cong_location_nullify(&the_globals.selection.loc0);
	cong_location_nullify(&the_globals.selection.loc1);
}

void xed_cutcopy_update(struct curs* curs)
{
	if (!curs->xed->x)
	{
		CongDocument *doc = the_globals.xv->doc;
		xmlview_destroy(FALSE);
		the_globals.xv = xmlview_new(doc);
	}
	else
	{
		xed_redraw(curs->xed);
	}
}

gint xed_cut(GtkWidget *widget, CongXMLEditor *xed_disabled)
{
	TTREE *t;
	int replace_xed = 0;

	struct selection* selection = &the_globals.selection;
	struct curs* curs = &the_globals.curs;

	if (!curs->w || !curs->xed || !cong_location_exists(&curs->location)) return(TRUE);
	
	if (!(cong_location_exists(&selection->loc0) && cong_location_exists(&selection->loc1) &&
				cong_location_parent(&selection->loc0) == cong_location_parent(&selection->loc1))) return(TRUE);

	if (cong_location_equals(&selection->loc0, &selection->loc1)) return(TRUE);
	
	if (the_globals.clipboard) ttree_branch_remove(the_globals.clipboard);
	
	t = ttree_node_add(0, "tag_span", 8);
	ttree_node_add(t, "dummy", 5);

	if (selection->loc0.tt_loc == curs->xed->x) replace_xed = 1;
	
	selection_reparent_all(selection, t);

	if (t->prev)
	{
		t->prev->next = t->next;
		if (replace_xed) curs->xed->x = t->prev;
	}
	else
	{
		curs->xed->x = t->next;
		if (t->parent) t->parent->child = t->next;
	}
	
	if (t->next) t->next->prev = t->prev;
	t->prev = t->next = t->parent = 0;

	the_globals.clipboard = t;

	selection_curs_unset();

	xed_cutcopy_update(curs);

	return(TRUE);
}


gint xed_copy(GtkWidget *widget, CongXMLEditor *xed_disabled)
{
	TTREE *t, *t0 = 0;
	int replace_xed = 0;

	struct selection* selection = &the_globals.selection;
	struct curs* curs = &the_globals.curs;

	if (!curs->w || !curs->xed || !cong_location_exists(&curs->location)) return(TRUE);

	if (!(cong_location_exists(&selection->loc0) && cong_location_exists(&selection->loc1) &&
				cong_location_parent(&selection->loc0) == cong_location_parent(&selection->loc1))) return(TRUE);

	if (cong_location_equals(&selection->loc0, &selection->loc1)) return(TRUE);

	if (the_globals.clipboard) ttree_branch_remove(the_globals.clipboard);
	
	t = ttree_node_add(0, "tag_span", 8);
	ttree_node_add(t, "dummy", 5);

	if (selection->loc0.tt_loc == curs->xed->x) replace_xed = 1;
	selection_reparent_all(selection, t);
	the_globals.clipboard = ttree_branch_dup(t);

	if (t->child->child)
	{
		t->child->child->prev = t->prev;
		if (replace_xed) curs->xed->x = t->prev;
		t->child->child->parent = t->parent;
		
		for (t0 = t->child->child; t0->next; t0 = t0->next)
		{
			t0->parent = t->parent;
		}
		
		t0->parent = t->parent;
		t0->next = t->next;
	}

	if (t->prev) t->prev->next = t->child->child;
	else if (t->parent) t->parent->child = t->child->child;

	if (t->next) t->next->prev = t0;

	t->child->child = 0;
	t->next = 0;
	t->prev = 0;

	ttree_branch_remove(t);
	selection_curs_unset();

#ifndef RELEASE	
	if (t0) ttree_fsave(t0->parent->parent->parent, stdout);
#endif

	xed_cutcopy_update(curs);

	return(TRUE);
}


gint xed_paste(GtkWidget *widget, CongXMLEditor *xed_disabled)
{
	TTREE *t, *t0 = 0, *t1 = 0, *clip;

	struct selection* selection = &the_globals.selection;
	struct curs* curs = &the_globals.curs;
	CongDispspec *ds;
	
	if (!curs->w || !curs->xed || !cong_location_exists(&curs->location)) return(TRUE);

	ds = curs->xed->displayspec;

	if (!the_globals.clipboard)
	{
	  selection_import(selection);
	  return(TRUE);
	}

	if (!the_globals.clipboard->child || !the_globals.clipboard->child->child) return(TRUE);
	
	if (cong_dispspec_element_structural(ds, xml_frag_name_nice(the_globals.clipboard))) return(TRUE);
	
	if (cong_location_frag_type(&curs->location) == XML_DATA)
	{
		if (!curs->location.char_loc)
		{
			t0 = cong_location_xml_frag_prev(&curs->location);
			t1 = cong_location_node(&curs->location);
		}
		else if (!cong_location_get_char(&curs->location))
		{
			t0 = cong_location_node(&curs->location);
			t1 = cong_location_xml_frag_next(&curs->location);
		}
		else
		{
			/* Split data node */
			cong_location_xml_frag_data_nice_split2(&curs->location);

			curs->location.char_loc = 0;
			t0 = cong_location_node(&curs->location);
			t1 = cong_location_xml_frag_next(&curs->location);
			if (cong_location_xml_frag_next(&curs->location)) curs->location.tt_loc = cong_location_xml_frag_next(&curs->location);
		}
	}
	else t0 = cong_location_node(&curs->location);
	
	clip = ttree_branch_dup(the_globals.clipboard);
	t = clip->child->child;

	if (!t) return(TRUE);
	
	t->prev = t0;
	if (t0) t0->next = t;
	else curs->location.tt_loc->parent->child = t;

	for (; t->next; t = t->next)
	{
		t->parent = curs->location.tt_loc->parent;
	}
	t->parent = curs->location.tt_loc->parent;

	t->next = t1;
	if (t1) t1->prev = t;

	cong_location_nullify(&selection->loc0);
	cong_location_nullify(&selection->loc1);

	xed_redraw(curs->xed);
	return(TRUE);
}












