/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-object.c
 *
 * Copyright (C) 2004 David Malcolm
 *
 * Conglomerate is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Conglomerate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#include "global.h"
#include "cong-object.h"

#define ENABLE_INSTANCE_TRACKING 0
#define ENABLE_LOGGING 0

/* Internal type aliases: */
typedef struct CongObjectDebugData CongObjectDebugData;
typedef struct CongObjectClassDebugData CongObjectClassDebugData;

/* Internal type declarations: */
struct CongObjectDebugData
{
	guint32 instance_count;
	GHashTable *hash_of_classes;
};

struct CongObjectClassDebugData
{
	guint32 instance_count;
#if ENABLE_INSTANCE_TRACKING
	GList *list_of_instances;
#endif
};

/* Internal function declarations: */
static CongObjectDebugData*
cong_object_debug_get_global_data (void);

static CongObjectClassDebugData*
cong_object_debug_get_class_data (GType type);

#if ENABLE_LOGGING
static void
cong_object_debug_log_event (GObject *object,
			     GType type,
			     const gchar *msg);
#endif

/* Exported function definitions: */
void 
cong_object_debug_instance_init (GObject *object,
				 GType type)
{
	CongObjectDebugData *debug_data;
	CongObjectClassDebugData *class_debug_data;
	
	g_assert (object);

	debug_data = cong_object_debug_get_global_data ();
	class_debug_data = cong_object_debug_get_class_data (type);
	g_assert (debug_data);
	g_assert (class_debug_data);

	debug_data->instance_count++;
	class_debug_data->instance_count++;

#if ENABLE_INSTANCE_TRACKING
	class_debug_data->list_of_instances = g_list_prepend (class_debug_data->list_of_instances, 
							      object);
#endif

#if ENABLE_LOGGING
	cong_object_debug_log_event (object,
				     type,
				     "instance_init");
#endif
}

void
cong_object_debug_finalize (GObject *object,
			    GType type)
{
	CongObjectDebugData *debug_data;
	CongObjectClassDebugData *class_debug_data;
	
	g_assert (object);

	debug_data = cong_object_debug_get_global_data ();
	class_debug_data = cong_object_debug_get_class_data (type);
	g_assert (debug_data);
	g_assert (class_debug_data);

	g_assert (debug_data->instance_count>0);
	debug_data->instance_count--;

	g_assert (class_debug_data->instance_count>0);
	class_debug_data->instance_count--;

#if ENABLE_INSTANCE_TRACKING
	class_debug_data->list_of_instances = g_list_remove (class_debug_data->list_of_instances, 
							     object);
#endif

#if 0
#error
	/* 
	   There seems to be a leak of CongEditorNodeText each time you repeat an undo/redo of applying a span tag to the selected text; probably should fix this next */

	/* Yes: it's leaking the CongEditorNodeText for the text node in the span tag */
#endif

#if ENABLE_LOGGING
	cong_object_debug_log_event (object,
				     type,
				     "finalize");
#endif
}

guint32
cong_object_debug_get_instance_count (void)
{
	CongObjectDebugData* debug_data;
	
	debug_data = cong_object_debug_get_global_data ();
	g_assert (debug_data);

	return debug_data->instance_count;
}

guint32
cong_object_debug_get_instance_count_for_class (GObjectClass *klass)
{
	return cong_object_debug_get_instance_count_for_type (G_OBJECT_CLASS_TYPE (klass));
}

guint32
cong_object_debug_get_instance_count_for_type (GType type)
{
	CongObjectClassDebugData *class_data = cong_object_debug_get_class_data (type);
	g_assert (class_data);
	
	return class_data->instance_count;
}

/*Internal function definitions: */
static CongObjectDebugData*
cong_object_debug_get_global_data (void)
{
	static CongObjectDebugData *global_debug_data;

	if (NULL==global_debug_data) {
		global_debug_data = g_new0 (CongObjectDebugData, 1);
		global_debug_data->hash_of_classes = g_hash_table_new (g_direct_hash,
								       g_direct_equal);
	}

	g_assert (global_debug_data);
	return global_debug_data;
}


static CongObjectClassDebugData*
cong_object_debug_get_class_data (GType type)
{
	CongObjectDebugData *debug_data;
	CongObjectClassDebugData *class_debug_data;

	debug_data = cong_object_debug_get_global_data ();
	class_debug_data = g_hash_table_lookup (debug_data->hash_of_classes,
						GINT_TO_POINTER (type));

	if (NULL==class_debug_data) {
		class_debug_data = g_new0 (CongObjectClassDebugData, 1);
		
		g_hash_table_insert (debug_data->hash_of_classes,
				     GINT_TO_POINTER (type),
				     class_debug_data);		
	}

	return class_debug_data;
}

#if 0
static void 
log_instances (GType type,
	       GObject *object,
	       gpointer user_data)
{
	g_message ("%p: instance of %s (subclass %s)", object, g_type_name (type), G_OBJECT_TYPE_NAME (object));
}
#endif
	
#if ENABLE_LOGGING
static void
cong_object_debug_log_event (GObject *object,
			     GType type,
			     const gchar *msg)
{
	g_assert (object);
	g_assert (msg);

	g_message ("%s: now %i objects (%i of %s)", 
		   msg,
		   cong_object_debug_get_instance_count (),
		   cong_object_debug_get_instance_count_for_type (type),
		   g_type_name (type));	

#if 0
	cong_object_debug_for_each_instance (type,
					     log_instances,
					     NULL);
#endif
}
#endif

struct class_hash_cb_data
{
	void 
	(*outer_callback) (GType type,
			   gpointer user_data);
	gpointer outer_user_data;
};

static void
class_hash_cb (gpointer key,
	       gpointer value,
	       gpointer user_data)
{
	struct class_hash_cb_data *cb_data = (struct class_hash_cb_data*)user_data;

	cb_data->outer_callback ((GType)key,
				 cb_data->outer_user_data);
}

void
cong_object_debug_for_each_class (void 
				  (*callback) (GType type,
					       gpointer user_data),
				  gpointer user_data)
{
	CongObjectDebugData *debug_data;
	struct class_hash_cb_data cb_data;
	cb_data.outer_callback = callback;
	cb_data.outer_user_data = user_data;
	
	debug_data = cong_object_debug_get_global_data ();

	g_hash_table_foreach (debug_data->hash_of_classes,
			      class_hash_cb,
			      &cb_data);
}

void
cong_object_debug_for_each_instance (GType type,
				     void 
				     (*callback) (GType type,
						  GObject *object,
						  gpointer user_data),
				     gpointer user_data)
{
#if ENABLE_INSTANCE_TRACKING
	GList *iter;
	CongObjectClassDebugData *class_debug_data;
	
	class_debug_data = cong_object_debug_get_class_data (type);
	g_assert (class_debug_data);
	
	for (iter=class_debug_data->list_of_instances; iter;iter=iter->next) {
		(*callback) (type,
			     G_OBJECT (iter->data),
			     user_data);
	}
#endif
}

enum {
	NAME_FIELD,
	N_FIELDS
};

struct add_object_data
{
	GtkTreeStore *store;
	GtkTreeIter   class_iter;
};

static void 
add_object_cb (GType type,
	       GObject *object,
	       gpointer user_data)
{
	struct add_object_data* add_obj_data = (struct add_object_data*)user_data;
	GtkTreeIter obj_iter;
	gchar *text;

	text = g_strdup_printf ("%p %s", object, G_OBJECT_TYPE_NAME (object));

	gtk_tree_store_append (add_obj_data->store, &obj_iter, &add_obj_data->class_iter);
	gtk_tree_store_set (add_obj_data->store, &obj_iter,
			    NAME_FIELD, text,
			    -1);
	g_free (text);
}

static void 
add_class_cb (GType type,
	      gpointer user_data)
{
	GtkTreeStore *store = GTK_TREE_STORE (user_data);
	struct add_object_data add_obj_data;
	add_obj_data.store = store;

	gtk_tree_store_append (store, &add_obj_data.class_iter, NULL);
	gtk_tree_store_set (store, &add_obj_data.class_iter,
			    NAME_FIELD, g_type_name (type),
			    -1);	
	
	cong_object_debug_for_each_instance (type,
					     add_object_cb,
					     &add_obj_data);
}

void
cong_object_debug_window (GtkWindow *parent_window)
{
	GtkDialog *dialog = GTK_DIALOG (gtk_dialog_new ());
	GtkWidget *tree_view;
	GtkTreeStore *store;

	store = gtk_tree_store_new (N_FIELDS,
				    G_TYPE_STRING);
	tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   tree_view);

	/* Set up tree model: */
	{
		cong_object_debug_for_each_class (add_class_cb,
						  store);
	}

	/* Set up tree view: */
	{
		GtkTreeViewColumn *column;
		GtkCellRenderer *renderer;

		renderer = gtk_cell_renderer_text_new ();
		
		column = gtk_tree_view_column_new_with_attributes (_("Class"), renderer,
								   "text", NAME_FIELD,
								   NULL);
		
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), 
					     column);
		
	}

	gtk_widget_show (tree_view);
	gtk_dialog_run (GTK_DIALOG (dialog));
}
