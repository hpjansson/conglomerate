/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include <string.h>

#include <ctype.h>

#include <ttree.h>
#include <xml.h>

#include "global.h"


/* Changing representation and file format: */
struct CongDispspecElement
{
	char* tagname;
	enum CongElementType type;
	gboolean collapseto;

	struct CongDispspecElement* next;	
};

struct _cong_dispspec
{
#if 0
	/* New implementation will be an "intrusive list" of CongDispspecElement structs */ 
	CongDispspecElement* first;
#else
  int dummy[128];
  TTREE *tt;
#endif
};

void cong_dispspec_init(TTREE *ds);

cong_dispspec* cong_dispspec_new_from_file(const char *name)
{
	cong_dispspec* ds;

	TTREE* tt = ttree_load((char*)name);
	if (!tt) {
		g_warning("Problem loading dispspec file \"%s\"\n", name);
		return NULL;  /* Invalid displayspec. */
	}

	ds = g_new(cong_dispspec,1);
	ds->tt = tt;

	cong_dispspec_init(tt);

	return ds;
}

void cong_dispspec_delete(cong_dispspec *dispspec)
{
  g_assert(0);
}

TTREE *get_upper_section(TTREE *x)
{
  TTREE *t, *r = 0;
  char *name;
  
  for (t = r = x; t; t = xml_frag_exit(t))
    {
      name = xml_frag_name_nice(t);
      if (!name) return(r);
      if (!strcasecmp("section", name)) r = t;
      else if (r != x) return(r);
    }
  
  return(x);
}

#if 0
#define DS_DEBUG_MSG1(x)    g_message((x))
#define DS_DEBUG_MSG2(x, a) g_message((x), (a))
#else
#define DS_DEBUG_MSG1(x)    ((void)0)
#define DS_DEBUG_MSG2(x, a) ((void)0)
#endif


gboolean cong_dispspec_element_structural(cong_dispspec *ds, char *name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, name);

	if (NULL==element) {
		return FALSE;
	}

	return (CONG_ELEMENT_TYPE_STRUCTURAL == cong_dispspec_element_type(element));
}


gboolean cong_dispspec_element_collapse(cong_dispspec *ds, char *name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, name);

	if (NULL==element) {
		return FALSE;
	}
	
	return cong_dispspec_element_collapseto(element);
}


gboolean cong_dispspec_element_span(cong_dispspec *ds, char *name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, name);

	if (NULL==element) {
		return FALSE;
	}

	return (CONG_ELEMENT_TYPE_SPAN == cong_dispspec_element_type(element));
}


gboolean cong_dispspec_element_insert(cong_dispspec *ds, char *name)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, name);

	if (NULL==element) {
		return FALSE;
	}

	return (CONG_ELEMENT_TYPE_INSERT == cong_dispspec_element_type(element));
}


unsigned int get_rgb_hex(unsigned char *s)
{
  unsigned int col;

  col = 0;
  while (*s)
    {
      col <<= 4;
      if (isalpha(*s)) col |= tolower(*s) - 'a' + 10;
      else if (isdigit(*s)) col |= *s - '0';
      s++;
    }
  
  return(col);
}


void col_to_gcol(GdkColor *gcol, unsigned int col)
{
  gcol->blue = (col << 8) & 0xff00;
  gcol->green = (col & 0xff00);
  gcol->red = (col >> 8) & 0xff00;
}


unsigned int cong_dispspec_colour_get(TTREE *ds, TTREE *x, int odd)
{
  TTREE *d;

  d = ttree_node_find1(ds, x->data, x->size, 0);
  if (d)
    {
      if ((d = ttree_node_find1(d, "color", 5, 0)) && d->child)
	return(get_rgb_hex(d->child->data));
    }
  
  return(0x00ffffff);  /* White is default */
}


char *cong_dispspec_name_get(TTREE *x)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(the_globals.ds, xml_frag_name_nice(x));
	if (element) {
		return (char*)cong_dispspec_element_username(element);
	}
  
	return(xml_frag_name_nice(x));
}


char *cong_dispspec_name_name_get(TTREE *t)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(the_globals.ds, t->data);
	if (element) {
		return (char*)cong_dispspec_element_username(element);
	}
  
	return(t->data);
}


GdkGC *cong_dispspec_name_gc_get(cong_dispspec *ds, TTREE *t, int tog)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, t->data);

	if (element) {
		return cong_dispspec_element_gc(element);
	} else {
		return NULL;
	}
}


GdkGC *cong_dispspec_gc_get(cong_dispspec *ds, TTREE *x, int tog)
{
	CongDispspecElement* element = cong_dispspec_lookup_element(ds, xml_frag_name_nice(x));

	if (element) {
		return cong_dispspec_element_gc(element);
	} else {
		return NULL;
	}
}


void cong_dispspec_init(TTREE *ds)
{
  TTREE *n0, *n1, *n2, *n3;
  GdkGC *gc;
  GdkColor gcol;
  
  for (n0 = ds->child; n0; n0 = n0->next)
    {
      gc = 0;
      
      /* Tag has colour specification? */
      n1 = ttree_node_find1(n0, "color", 5, 0);
      if (!n1) n1 = ttree_node_find1(n0, "colour", 6, 0);
      
      if (n1 && n1->child)
	{
	  /* Try to find an already allocated GC for the colour */
	  for (n2 = ds->child; n2 != n0; n2 = n2->next)
	    {
	      n3 = ttree_node_find1(n2, "color", 5, 0);
	      if (!n3) n3 = ttree_node_find1(n2, "colour", 6, 0);
	      
	      if (n3 && n3->child)
		{
		  if (!strcasecmp(n1->child->data, n3->child->data))
		    {
		      gc = (GdkGC *) *((GdkGC **) n3->child->child->data);
		      break;
		    }
		}
	    }
	  
	  /* Found a colour allocated earlier? */
	  
	  if (gc)
	    {
	      ttree_node_add(n1->child, (unsigned char *) &gc, sizeof(GdkGC *));
	    }
	  else
	    {
				/* No, allocate */
	      
	      gc = gdk_gc_new(cong_gui_get_window(&the_gui)->window);
	      gdk_gc_copy(gc, cong_gui_get_window(&the_gui)->style->white_gc);
	      col_to_gcol(&gcol, get_rgb_hex(n1->child->data));
	      gdk_colormap_alloc_color(cong_gui_get_window(&the_gui)->style->colormap, &gcol, 0, 1);
	      gdk_gc_set_foreground(gc, &gcol);
#if 0
	      gdk_rgb_gc_set_foreground(gc, get_rgb_hex(n1->child->data));
#endif
	      ttree_node_add(n1->child, (unsigned char *) &gc, sizeof(GdkGC *));
	    }
	}
    }
}


char *section_str = "section";


char *tag_picked_name;

void tag_new_picked(GtkWidget *w, char *name)
{
	tag_picked_name = name;
	gtk_main_quit();
}

GtkWidget *pickstruct;


char *pick_structural_tag()
{
  GtkWidget *w0, *w1;
  CongDispspecElement *n0;
  
	tag_picked_name = 0;
	
  pickstruct = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(pickstruct), 10);
  gtk_widget_set_usize(GTK_WIDGET(pickstruct), 240, 240);
  gtk_window_set_title(GTK_WINDOW(pickstruct), "Select element");
  gtk_window_set_position(GTK_WINDOW(pickstruct), GTK_WIN_POS_MOUSE);
  gtk_window_set_modal(GTK_WINDOW(pickstruct), 1);
  gtk_window_set_policy(GTK_WINDOW(pickstruct), 0, 0, 1);
  
  /* --- Window -> vbox --- */

  w0 = gtk_vbox_new(TRUE, 1);
  gtk_container_add(GTK_CONTAINER(pickstruct), w0);
  gtk_widget_show(w0);

  /* Window -> vbox -> buttons */
  for (n0 = cong_dispspec_get_first_element(the_globals.ds); n0; n0 = cong_dispspec_element_next(n0)) {
	  if (cong_dispspec_element_is_structural(n0)) {
		  w1 = gtk_button_new_with_label(cong_dispspec_element_username(n0));
		  gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
		  gtk_widget_show(w1);
		  gtk_signal_connect(GTK_OBJECT(w1), "clicked", (GtkSignalFunc) tag_new_picked, (gpointer) cong_dispspec_element_tagname(n0));
	  }
  }

  gtk_widget_show(pickstruct);
	gtk_main();
	gtk_widget_destroy(pickstruct);
  return(tag_picked_name);
}



#ifdef WINDOWS_BUILD
                                                                                
int strcasestr(char *haystack, char *needle)                                    
{
	  char *r;                                                                        
	  char *haystack_dup, *needle_dup;                                              
	                                                                                
	  haystack_dup = strdup(haystack);                                              
	  needle_dup = strdup(needle);                                                  
	                                                                                
	  strlwr(haystack_dup);                                                         
	  strlwr(needle_dup);                                                           
	                                                                                
	  r = strstr(haystack_dup, needle_dup);                                         
	                                                                                
	  free(haystack_dup);                                                           
	  free(needle_dup);                                                             
	                                                                                
	  return(r);                                                                    
}
#endif

/*
  For now we fake things, and pretend a CongDispspecElement* is a TTREE*
 */
CongDispspecElement*
cong_dispspec_lookup_element(cong_dispspec *ds, const char* tagname)
{
	TTREE* n0 = ttree_node_find1(ds->tt, (char*)tagname, strlen(tagname), 0);

	return (CongDispspecElement*)n0;
}

CongDispspecElement*
cong_dispspec_get_first_element(cong_dispspec *ds)
{
	return (CongDispspecElement*)(ds->tt->child);
}

const char*
cong_dispspec_element_tagname(CongDispspecElement* element)
{
	g_return_val_if_fail(element, NULL);

	return ((TTREE*)element)->data;
}

const char*
cong_dispspec_element_username(CongDispspecElement* element)
{
	TTREE* n1;

	g_return_val_if_fail(element, NULL);

	n1 = ttree_node_find1((TTREE*)element, "name", 4, 0);
	if (!n1) return "could not find name";
	
	if (!n1->child) return "no name specified";
	n1 = n1->child;

	return n1->data;
}

const char*
cong_dispspec_element_name_name_get(CongDispspecElement* element)
{
	g_return_val_if_fail(element, NULL);

	return cong_dispspec_name_name_get((TTREE*)element);
}

CongDispspecElement*
cong_dispspec_element_next(CongDispspecElement* element)
{
	g_return_val_if_fail(element, NULL);

	return (CongDispspecElement*)(((TTREE*)element)->next);
}

enum CongElementType
cong_dispspec_element_type(CongDispspecElement *element)
{
	TTREE *n0;
	
	g_return_val_if_fail(element, CONG_ELEMENT_TYPE_UNKNOWN);

	n0 = ttree_node_find1((TTREE*)element, "type", 4, 0);
	if (!n0) {
		DS_DEBUG_MSG1("\"type\" not found, so type is unknown\n");
		
		return CONG_ELEMENT_TYPE_UNKNOWN;
	}
	
	if (!n0->child) {
		DS_DEBUG_MSG1("No child found, so type is unknown\n");
	  
		return CONG_ELEMENT_TYPE_UNKNOWN;
	}

	if (!strcasecmp("structural", n0->child->data)) {
		DS_DEBUG_MSG1("Child has \"structural\" text, so it is structural\n");
		return CONG_ELEMENT_TYPE_STRUCTURAL;
	}

	if (!strcasecmp("span", n0->child->data)) {
		DS_DEBUG_MSG1("Child has \"span\" text, so it is a span\n");
		return CONG_ELEMENT_TYPE_SPAN;
	}  

	if (!strcasecmp("insert", n0->child->data)) {
		DS_DEBUG_MSG1("Child has \"insert\" text, so it is an insert\n");
		return CONG_ELEMENT_TYPE_INSERT;
	}

	return CONG_ELEMENT_TYPE_UNKNOWN;
}

gboolean
cong_dispspec_element_collapseto(CongDispspecElement *element)
{
	TTREE* n0 = ttree_node_find1((TTREE*)element, "collapseto", 10, 0);
	if (n0) return TRUE;
  
	return FALSE;
}


gboolean
cong_dispspec_element_is_structural(CongDispspecElement *element)
{
	g_return_val_if_fail(element, FALSE);

	if (CONG_ELEMENT_TYPE_STRUCTURAL == cong_dispspec_element_type(element)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

gboolean
cong_dispspec_element_is_span(CongDispspecElement *element)
{
	g_return_val_if_fail(element, FALSE);

	if (CONG_ELEMENT_TYPE_SPAN == cong_dispspec_element_type(element)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

GdkGC*
cong_dispspec_element_gc(CongDispspecElement *element)
{
	TTREE* n1;
	
	g_return_val_if_fail(element, NULL);

	n1 = ttree_node_find1((TTREE*)element, "color", 5, 0);
	if (!n1) n1 = ttree_node_find1((TTREE*)element, "colour", 6, 0);
      
	if (n1 && n1->child && n1->child->child) {
		return((GdkGC *) *((GdkGC **) n1->child->child->data));
	}

	return NULL;
}
