#include <ttree.h>

#include <stdlib.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "global.h"


GtkWidget *tchoice_win;

struct tchoice_data
{
	TTREE *choices;
	int last_was_insertion;
	GtkTreeItem *item_expanded;
};


GtkWidget *gui_tchoice_entry(TTREE *choices);

TTREE *ttree_branch_find1_beg(TTREE *t, char *str)
{
	TTREE *ret = 0;
	int len;
	
	len = strlen(str);
	
	for (t = t->child; t; t = t->next)
	{
		if (len <= t->size && !strncasecmp(str, t->data, len)) break;
		
		if (t->child) ret = ttree_branch_find1_beg(t, str);
		if (ret)
		{
			t = ret;
			break;
		}
	}

	return(t);
}


void gui_tchoice_expand_to_item(GtkTreeItem *w)
{
	GtkArg a;
	GtkWidget *wp;
	
	a.name = "parent";
	a.type = GTK_TYPE_POINTER;
	
	gtk_widget_get(GTK_WIDGET(w), &a);
	wp = GTK_VALUE_POINTER(a);
	
	if (!wp) return;
	
	/* wp is now tree branch */
	
	wp = GTK_TREE(wp)->tree_owner;
	if (!wp) return;

	/* wp is now parent item */

	gtk_tree_item_expand(GTK_TREE_ITEM(wp));

	gui_tchoice_expand_to_item(GTK_TREE_ITEM(wp));  /* Recurse */
	return;
}


void gui_tchoice_collapse_to_item(GtkTreeItem *w)
{
	GtkArg a;
	GtkWidget *wp;
	
	a.name = "parent";
	a.type = GTK_TYPE_POINTER;
	
	gtk_widget_get(GTK_WIDGET(w), &a);
	wp = GTK_VALUE_POINTER(a);
	
	if (!wp) return;
	
	/* wp is now tree branch */
	
	wp = GTK_TREE(wp)->tree_owner;
	if (!wp) return;

	/* wp is now parent item */

	gtk_tree_item_collapse(GTK_TREE_ITEM(wp));

	gui_tchoice_collapse_to_item(GTK_TREE_ITEM(wp));  /* Recurse */
	return;
}


gint tchoice_select_expand(GtkWidget *w)
{
#if 0	
	gtk_tree_item_expand(GTK_TREE_ITEM(w));
#endif
	gtk_tree_item_deselect(GTK_TREE_ITEM(w));
	return(TRUE);
}


gint gui_tchoice_item_set(GtkWidget *w, TTREE *t)
{
#ifndef RELEASE	
	printf("Item set.\n");
#endif	
	ttree_node_add(t, "chosen", 6);
	return(FALSE);
}


gint gui_tchoice_item_clear(GtkWidget *w, TTREE *t)
{
#ifndef RELEASE
	printf("Item cleared.\n");
#endif
	t = ttree_node_find1(t, "chosen", 6, 0);
	if (t) ttree_branch_remove(t);
	return(FALSE);
}


GtkWidget *tchoice_build(TTREE *c, GtkWidget *item, int many, int selectability, GtkSignalFunc *sig_func, char *id)
{
	GtkWidget *w0, *w1, *lab;
	TTREE *t, *n0;
	int have_subtree = 0;


	for (c = c->child; c; c = c->next)
	{
		if (selectability && !strcmp("selectable", c->data)) continue;
		else if (!strcmp("medias", c->data)) continue;
		else if (!strcmp("id", c->data)) continue;
		else if (!strcmp("widget", c->data)) continue;
		else if (!strcmp("chosen", c->data)) continue;

		if (id)
		{
			n0 = ttree_branch_walk_str(c, "medias");
			if (n0 && !ttree_node_find1(n0, id, strlen(id), 0)) continue;
		}
		
		w0 = gtk_tree_item_new();
		lab = gtk_label_new(c->data);
		gtk_misc_set_alignment(GTK_MISC(lab), 0.0, 0.5);
		gtk_container_add(GTK_CONTAINER(w0), lab);
		gtk_widget_show(lab);

		n0 = ttree_node_add(c, "widget", 6);
		ttree_node_add(n0, (unsigned char *) &w0, sizeof(w0));

		if (!have_subtree)
		{
			w1 = gtk_tree_new();
			if (many) gtk_tree_set_selection_mode(GTK_TREE(w1), GTK_SELECTION_MULTIPLE);
			else gtk_tree_set_selection_mode(GTK_TREE(w1), GTK_SELECTION_SINGLE);

			if (item) gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), w1);
			else
			{
				gtk_widget_show(w1);
			}
			
			have_subtree = 1;
		}

		gtk_tree_append(GTK_TREE(w1), w0);

		if (selectability)
		{
			if (!ttree_branch_walk_str(c, "selectable"))
			{
				gtk_widget_set_sensitive(lab, 0);
				gtk_signal_connect_after(GTK_OBJECT(w0), "select", GTK_SIGNAL_FUNC(tchoice_select_expand), 0);
			}
			else
			{
				if (ttree_branch_walk_str(c, "chosen"))
				{
					gtk_tree_item_select(GTK_TREE_ITEM(w0));
					gui_tchoice_expand_to_item(GTK_TREE_ITEM(w0));
				}
				
				gtk_signal_connect(GTK_OBJECT(w0), "select", GTK_SIGNAL_FUNC(gui_tchoice_item_set), c);
				gtk_signal_connect(GTK_OBJECT(w0), "deselect", GTK_SIGNAL_FUNC(gui_tchoice_item_clear), c);
			}
		}
		else
		{
			if (ttree_branch_walk_str(c, "chosen"))
			{
				gtk_tree_item_select(GTK_TREE_ITEM(w0));
				gui_tchoice_expand_to_item(GTK_TREE_ITEM(w0));
			}
				
			gtk_signal_connect(GTK_OBJECT(w0), "select", GTK_SIGNAL_FUNC(gui_tchoice_item_set), c);
			gtk_signal_connect(GTK_OBJECT(w0), "deselect", GTK_SIGNAL_FUNC(gui_tchoice_item_clear), c);
		}

		gtk_widget_show(w0);

		/* TODO: Connect signal to item */

		if (sig_func) gtk_signal_connect(GTK_OBJECT(w0), "select", GTK_SIGNAL_FUNC(sig_func), c);
		
		tchoice_build(c, w0, many, selectability, sig_func, id);
	}

	return(w1);
}


void tchoice_tree_additives_destroy(TTREE *t)
{
	TTREE *n0;
	
	for (t = t->child; t; )
	{
		if (!strcmp("widget", t->data))
		{
			n0 = t->next;
			ttree_branch_remove(t);
			t = n0;
			if (t) continue;
			else break;
		}
		else if (!strcmp("chosen", t->data))
		{
			n0 = t->next;
			ttree_branch_remove(t);
			t = n0;
			if (t) continue;
			else break;
		}
		
		if (t->child) tchoice_tree_additives_destroy(t);
		t = t->next;
	}
}


void tchoice_destroy(TTREE *c)
{
	tchoice_tree_additives_destroy(c);
}


GtkWidget *gui_tchoice_box(TTREE *choices, int many, int selectability, GtkSignalFunc *sig_func, char *id)
{
	GtkWidget *w0, *w1, *w2;

	w0 = gtk_vbox_new(FALSE, 2);
	gtk_widget_show(w0);
	
  w1 = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(w1), GTK_POLICY_AUTOMATIC,
																 GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(w0), w1, TRUE, TRUE, 0);
	gtk_widget_show(w1);

	w2 = tchoice_build(choices, 0, many, selectability, sig_func, id);
	
#if 0	
	if (many) gtk_tree_set_selection_mode(GTK_TREE(w2), GTK_SELECTION_MULTIPLE);
#endif
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(w1), w2);
	gtk_widget_show(w2);
	gtk_widget_queue_draw(w2);
	
	w1 = gui_tchoice_entry(choices);
	gtk_box_pack_start(GTK_BOX(w0), w1, FALSE, TRUE, 0);
	gtk_widget_show(w1);

	return(w0);
}


void gui_tchoice_win(char *type, TTREE *choices, int many, int selectability)
{
	GtkWidget *w0, *w1, *w2;
	char buf[512];

	asprintf(buf, "Select %s", type);

  tchoice_win = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_container_set_border_width(GTK_CONTAINER(tchoice_win), 10);
	gtk_window_set_default_size(GTK_WINDOW(tchoice_win), 360, 240);
	gtk_window_set_title(GTK_WINDOW(tchoice_win), buf);
	gtk_window_set_position(GTK_WINDOW(tchoice_win), GTK_WIN_POS_MOUSE);
	gtk_window_set_modal(GTK_WINDOW(tchoice_win), 1);
	gtk_window_set_policy(GTK_WINDOW(tchoice_win), 1, 1, 0);

	w0 = gtk_vbox_new(FALSE, 4);
	gtk_container_add(GTK_CONTAINER(tchoice_win), w0);
	gtk_widget_show(w0);
	
	gtk_box_pack_start(GTK_BOX(w0), gui_tchoice_box(choices, many, selectability, 0, 0),
										 TRUE, TRUE, 0);

	/* Put button */
	
	w2 = gtk_button_new_with_label("Ok");
	gtk_box_pack_start(GTK_BOX(w0), w2, FALSE, FALSE, 0);
	gtk_widget_show(w2);

	/* Splash */
	
	gtk_widget_show(tchoice_win);
	
	return;
}


gint gui_tchoice_entry_insert(GtkEditable *w, gchar *text, int length, gint *position, struct tchoice_data *td)
{
	TTREE *t;
	char *entry_str;
	int pos;

	if (!length) return(TRUE);

	gtk_signal_handler_block_by_func(GTK_OBJECT(w), GTK_SIGNAL_FUNC(gui_tchoice_entry_insert), td);
	gtk_editable_insert_text(w, text, length, position);
	gtk_signal_handler_unblock_by_func(GTK_OBJECT(w), GTK_SIGNAL_FUNC(gui_tchoice_entry_insert), td);
	gtk_signal_emit_stop_by_name(GTK_OBJECT(w), "insert_text");

	td->last_was_insertion = 1;
	gtk_editable_select_region(w, 0 /* strlen(entry_str) */, 1);
	return(TRUE);
}


gint gui_tchoice_entry_selection(GtkEditable *w, GdkEventSelection *e, struct tchoice_data *td)
{
	TTREE *t, *t0;
	char *entry_str;
	int pos;

#ifndef RELEASE	
	printf("start=%d, end=%d, has=%d\n", w->selection_start_pos,
				 w->selection_end_pos, w->has_selection);
#endif
	
	if (!td->last_was_insertion) return(FALSE);
	td->last_was_insertion = 0;

	entry_str = gtk_entry_get_text(GTK_ENTRY(w));

	if (!entry_str || !strlen(entry_str) ||
			!(t = ttree_branch_find1_beg(td->choices, entry_str)))
	{
		if (td->item_expanded)
		{
			gui_tchoice_collapse_to_item(GTK_TREE_ITEM(td->item_expanded));
			td->item_expanded = 0;
		}

		return(FALSE);
	}

	/* Try to show its branch */

	t0 = ttree_node_find1(t, "widget", 6, 0);
	if (t0 && t0->child && GTK_TREE_ITEM((*((GtkWidget **) t0->child->data))) != GTK_TREE_ITEM(td->item_expanded))
	{
		if (td->item_expanded)
			gui_tchoice_collapse_to_item(GTK_TREE_ITEM(td->item_expanded));
		
		t0 = t0->child;
		gui_tchoice_expand_to_item(GTK_TREE_ITEM(*((GtkWidget **) t0->data)));
		td->item_expanded = GTK_TREE_ITEM(*((GtkWidget **) t0->data));
#if 0
		/* Needs some work */
		gtk_widget_set_state(GTK_WIDGET(*((GtkWidget **) t0->data)), GTK_STATE_ACTIVE);
#endif
	}

	/* Do the other stuff */
	
	pos = gtk_editable_get_position(w);
	gtk_signal_handler_block_by_func(GTK_OBJECT(w), GTK_SIGNAL_FUNC(gui_tchoice_entry_insert), td);
	gtk_entry_set_text(GTK_ENTRY(w), t->data);
	gtk_signal_handler_unblock_by_func(GTK_OBJECT(w), GTK_SIGNAL_FUNC(gui_tchoice_entry_insert), td);

	gtk_editable_select_region(w, pos /* strlen(entry_str) */, -1);

	gtk_signal_emit_stop_by_name(GTK_OBJECT(w), "selection_clear_event");
	return(TRUE);
}


gint gui_tchoice_entry_activate(GtkEntry *w, struct tchoice_data *td)
{
	TTREE *t;
	char *entry_str;
	
	entry_str = gtk_entry_get_text(GTK_ENTRY(w));

	if (!entry_str) return(FALSE);
	if (!strlen(entry_str)) return(FALSE);

	t = ttree_branch_find1_beg(td->choices, entry_str);
	if (!t) return(FALSE);  /* Blink red, maybe? */

	t = ttree_node_find1(t, "widget", 6, 0);
	if (!t) return(FALSE);  /* Found entry not in list (no item widget) */
	
	/* Found it */
	
	t = t->child;
	gui_tchoice_expand_to_item(GTK_TREE_ITEM(*((GtkWidget **) t->data)));
	gtk_widget_activate(GTK_WIDGET(*((GtkWidget **) t->data)));

	td->item_expanded = 0;
	gtk_entry_set_text(w, "");
	
	return(FALSE);
}


GtkWidget *gui_tchoice_entry(TTREE *choices)
{
	GtkWidget *w0, *w1;
	struct tchoice_data *td;

	td = malloc(sizeof(*td));
	td->choices = choices;
	td->last_was_insertion = 0;
	td->item_expanded = 0;

	w0 = gtk_entry_new();

	gtk_signal_connect(GTK_OBJECT(w0), "insert_text", GTK_SIGNAL_FUNC(gui_tchoice_entry_insert), (gpointer) td);
	gtk_signal_connect(GTK_OBJECT(w0), "selection_clear_event", GTK_SIGNAL_FUNC(gui_tchoice_entry_selection), (gpointer) td);
	gtk_signal_connect(GTK_OBJECT(w0), "activate", GTK_SIGNAL_FUNC(gui_tchoice_entry_activate), (gpointer) td);
	return(w0);
}
