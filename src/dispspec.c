#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include <string.h>

#include <ttree.h>
#include <xml.h>

#include "global.h"

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

int ds_element_structural(TTREE *ds, char *name)
{
  TTREE *n0;

  DS_DEBUG_MSG2( "ds_element_structural(ds,\"%s\")\n", name );
  
  n0 = ttree_node_find1(ds, name, strlen(name), 0);
  if (!n0) {
    DS_DEBUG_MSG1("not found in dispspec, so not structural\n");
    return(0);
  }
  
  n0 = ttree_node_find1(n0, "type", 4, 0);
  if (!n0) {
    DS_DEBUG_MSG1("\"type\" not found, so not structural\n");
    return(0);
  }
  
  if (!n0->child) {
    DS_DEBUG_MSG1("No child found, so not structural\n");
    return(0);
  }
  
  if (!strcasecmp("structural", n0->child->data)) {
    DS_DEBUG_MSG1("Child has \"structural\" text, so it is structural\n");
    return(1);
  }

  DS_DEBUG_MSG1("Falling through, so not structural\n");
  
  return(0);
}


int ds_element_collapse(TTREE *ds, char *name)
{
  TTREE *n0;
  
  n0 = ttree_node_find1(ds, name, strlen(name), 0);
  if (!n0) return(0);
  
  n0 = ttree_node_find1(n0, "collapseto", 10, 0);
  if (n0) return(1);
  
  return(0);
}


int ds_element_span(TTREE *ds, char *name)
{
  TTREE *n0;
  
  n0 = ttree_node_find1(ds, name, strlen(name), 0);
  if (!n0) return(0);
  
  n0 = ttree_node_find1(n0, "type", 4, 0);
  if (!n0) return(0);
  
  if (!n0->child) return(0);
  
  if (!strcasecmp("span", n0->child->data)) return(1);
  
  return(0);
}


int ds_element_insert(TTREE *ds, char *name)
{
  TTREE *n0;
  
  n0 = ttree_node_find1(ds, name, strlen(name), 0);
  if (!n0) return(0);
  
  n0 = ttree_node_find1(n0, "type", 4, 0);
  if (!n0) return(0);
  
  if (!n0->child) return(0);
  
  if (!strcasecmp("insert", n0->child->data)) return(1);
  
  return(0);
}


TTREE *ds_get_first_structural(TTREE *ds)
{
	TTREE *n0, *n1;

	for (n0 = ds->child; n0; n0 = n0->next)
	{
    n1 = ttree_node_find1(n0, "type", 4, 0);
    if (!n1) continue;
  
    if (!n1->child) continue;
  
    if (!strcasecmp("structural", n1->child->data)) break;
	}
	
	return(n0);
}


TTREE *ds_get_next_structural(TTREE *prev)
{
	TTREE *n0, *n1;

	for (n0 = prev->next; n0; n0 = n0->next)
	{
    n1 = ttree_node_find1(n0, "type", 4, 0);
    if (!n1) continue;
  
    if (!n1->child) continue;
  
    if (!strcasecmp("structural", n1->child->data)) break;
	}
	
	return(n0);
}


TTREE *ds_get_first_span(TTREE *ds)
{
	TTREE *n0, *n1;

	for (n0 = ds->child; n0; n0 = n0->next)
	{
    n1 = ttree_node_find1(n0, "type", 4, 0);
    if (!n1) continue;
  
    if (!n1->child) continue;
  
    if (!strcasecmp("span", n1->child->data)) break;
	}
	
	return(n0);
}


TTREE *ds_get_next_span(TTREE *prev)
{
	TTREE *n0, *n1;

	for (n0 = prev->next; n0; n0 = n0->next)
	{
    n1 = ttree_node_find1(n0, "type", 4, 0);
    if (!n1) continue;
  
    if (!n1->child) continue;
  
    if (!strcasecmp("span", n1->child->data)) break;
	}
	
	return(n0);
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


unsigned int ds_colour_get(TTREE *ds, TTREE *x, int odd)
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


char *ds_name_get(TTREE *x)
{
  TTREE *n0, *n1;
  
  n0 = ttree_node_find1(ds_global, xml_frag_name_nice(x), strlen(xml_frag_name(x)), 0);
  if (n0)
    {
      n1 = ttree_node_find1(n0, "name", 4, 0);
      
      if (n1 && n1->child)
	{
	  return(n1->child->data);
	}
    }
  
  return(xml_frag_name_nice(x));
}


char *ds_name_name_get(TTREE *t)
{
  TTREE *n0, *n1;
  
  n0 = ttree_node_find1(ds_global, t->data, t->size, 0);
  if (n0)
    {
      n1 = ttree_node_find1(n0, "name", 4, 0);
      
      if (n1 && n1->child)
	{
	  return(n1->child->data);
	}
    }
  
  return(t->data);
}


GdkGC *ds_name_gc_get(TTREE *ds, TTREE *t, int tog)
{
  GdkGC *gc;
  TTREE *n0, *n1;
  
  n0 = ttree_node_find1(ds, t->data, t->size, 0);
  if (n0)
    {
      n1 = ttree_node_find1(n0, "color", 5, 0);
      if (!n1) n1 = ttree_node_find1(n0, "colour", 6, 0);
      
      if (n1 && n1->child && n1->child->child)
	{
	  return((GdkGC *) *((GdkGC **) n1->child->child->data));
	}
    }
  
  return(0);
}


GdkGC *ds_gc_get(TTREE *ds, TTREE *x, int tog)
{
  GdkGC *gc;
  TTREE *n0, *n1;
  
  n0 = ttree_node_find1(ds, xml_frag_name_nice(x), strlen(xml_frag_name(x)), 0);
  if (n0)
    {
      n1 = ttree_node_find1(n0, "color", 5, 0);
      if (!n1) n1 = ttree_node_find1(n0, "colour", 6, 0);
      
      if (n1 && n1->child && n1->child->child)
	{
	  return((GdkGC *) *((GdkGC **) n1->child->child->data));
	}
    }
  
  return(0);
}


void ds_init(TTREE *ds)
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
	      
	      gc = gdk_gc_new(window->window);
	      gdk_gc_copy(gc, window->style->white_gc);
	      col_to_gcol(&gcol, get_rgb_hex(n1->child->data));
	      gdk_colormap_alloc_color(window->style->colormap, &gcol, 0, 1);
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
	TTREE *n0, *n1;
  
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

	for (n0 = ds_get_first_structural(ds_global); n0; n0 = ds_get_next_structural(n0))
	{
    n1 = ttree_node_find1(n0, "name", 4, 0);
    if (!n1) continue;
		
		if (!n1->child) continue;
		n1 = n1->child;

    w1 = gtk_button_new_with_label(n1->data);
    gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
    gtk_widget_show(w1);
    gtk_signal_connect(GTK_OBJECT(w1), "clicked", (GtkSignalFunc) tag_new_picked, (gpointer) n0->data);
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
