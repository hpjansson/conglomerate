/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
  Testbed for working with MVC
 */

#include <gtk/gtk.h>
#include "global.h"
#include "cong-document.h"
#include "cong-view.h"

#define DEBUG_DOM_VIEW 0

enum {
	CONG_DOM_VIEW_TREEMODEL_COLUMN_TEXT,
	CONG_DOM_VIEW_TREEMODEL_COLUMN_NODE,
	CONG_DOM_VIEW_TREEMODEL_COLUMN_DOCUMENT,
	CONG_DOM_VIEW_TREEMODEL_NUM_COLUMNS
};

#define CONG_DOM_VIEW(x) ((CongDOMView*)(x))

typedef struct CongDOMView
{
	CongView view;

	struct CongDOMViewDetails *private;
} CongDOMView;

typedef struct CongDOMViewDetails
{
	CongDOMView *dom_view;
	GtkScrolledWindow *scrolled_window;
	GtkTreeStore *tree_store;
	GtkTreeView *tree_view;

} CongDOMViewDetails;

struct search_struct
{
	CongNodePtr node;

	gboolean found_it;	
	
	GtkTreeIter *tree_iter;
};

static gboolean search_for_node(GtkTreeModel *model,
				GtkTreePath *path,
				GtkTreeIter *iter,
				gpointer data)
{
	struct search_struct* search = (struct search_struct*)data;
	CongNodePtr this_node;
	
	g_assert(model);
	g_assert(path);
	g_assert(iter);
	g_assert(search);

	gtk_tree_model_get(model,
			   iter,
			   CONG_DOM_VIEW_TREEMODEL_COLUMN_NODE,
			   &this_node,
			   -1);

	if (this_node==search->node) {
		search->found_it = TRUE;
		*search->tree_iter = *iter;
		return TRUE;
	} else {
		return FALSE;
	}

}

static gboolean get_iter_for_node(CongDOMViewDetails *details, CongNodePtr node, GtkTreeIter* tree_iter)
{
	/* FIXME: this is O(n), it ought to be O(1), by adding some kind of search structure */
	struct search_struct search;
	
	search.node = node;
	search.found_it = FALSE;
	search.tree_iter = tree_iter;

	gtk_tree_model_foreach(GTK_TREE_MODEL(details->tree_store),
			       search_for_node,
			       &search);

	return search.found_it;
}

static gchar *clean_text(const gchar* txt) 
{
	gchar *cleaned;
	gchar *result;

	g_return_val_if_fail(txt, NULL);

	cleaned = cong_util_cleanup_text(txt);

	result = g_markup_escape_text(cleaned,
				      strlen(cleaned));
	g_free(cleaned);

	return result;
}

const gchar *cong_ui_get_colour_string(enum CongNodeType type)
{
	/* FIXME: this should be linked to the theme and/or the GtkSourceView settings */

	switch (type) {
	default: g_assert_not_reached();
	case CONG_NODE_TYPE_UNKNOWN:
		return "#000000";
	case CONG_NODE_TYPE_ELEMENT:
		return "#0080ff";
	case CONG_NODE_TYPE_ATTRIBUTE:
		return "#000000";
	case CONG_NODE_TYPE_TEXT:
		return "#ff0000";
	case CONG_NODE_TYPE_CDATA_SECTION:
		return "#000000";
	case CONG_NODE_TYPE_ENTITY_REF:
		return "#000000";
	case CONG_NODE_TYPE_ENTITY_NODE:
		return "#000000";
	case CONG_NODE_TYPE_PI:
		return "#000000";
	case CONG_NODE_TYPE_COMMENT:
		return "#0000FF";
	case CONG_NODE_TYPE_DOCUMENT:
		return "#0080ff";
	case CONG_NODE_TYPE_DOCUMENT_TYPE:
		return "#000000";
	case CONG_NODE_TYPE_DOCUMENT_FRAG:
		return "#000000";
	case CONG_NODE_TYPE_NOTATION:
		return "#000000";
	case CONG_NODE_TYPE_HTML_DOCUMENT:
		return "#000000";
	case CONG_NODE_TYPE_DTD:
		return "#0000FF";
	case CONG_NODE_TYPE_ELEMENT_DECL:
		return "#000000";
	case CONG_NODE_TYPE_ATRRIBUTE_DECL:
		return "#000000";
	case CONG_NODE_TYPE_ENTITY_DECL:
		return "#000000";
	case CONG_NODE_TYPE_NAMESPACE_DECL:
		return "#000000";
	case CONG_NODE_TYPE_XINCLUDE_START:
		return "#000000";
	case CONG_NODE_TYPE_XINCLUDE_END:
		return "#000000";
	}
}

static gchar *get_text_for_node(CongNodePtr node)
{
	enum CongNodeType node_type;
	const gchar *colour_string;
	const gchar *string_colour_string;
	gchar *text = NULL;
	gchar *cleaned_text = NULL;


	node_type = cong_node_type(node);
	
	colour_string = cong_ui_get_colour_string(node_type);
	string_colour_string = "#00FF00"; /* FIXME: get this from the theme and/or GtkSourceView settings */

	switch (node_type) {
	default: g_assert_not_reached();
	case CONG_NODE_TYPE_UNKNOWN: 
		text = g_strdup(_("UNKNOWN"));
		break;
	case CONG_NODE_TYPE_ELEMENT: 
		text = g_strdup_printf("<span foreground=\"%s\">&lt;%s&gt;</span>", colour_string, cong_node_name(node));
		/* FIXME: display any attributes */
		break;
	case CONG_NODE_TYPE_ATTRIBUTE:
		text = g_strdup_printf("ATTRIBUTE");
		break;		
	case CONG_NODE_TYPE_TEXT:
		cleaned_text = clean_text(node->content);
		text = g_strdup_printf(_("Text: \"<span foreground=\"%s\">%s</span>\""), colour_string, cleaned_text);
		g_free(cleaned_text);
		break;
	case CONG_NODE_TYPE_CDATA_SECTION:
		text = g_strdup_printf("CDATA SECTION");
		break;
	case CONG_NODE_TYPE_ENTITY_REF:
		text = g_strdup_printf("ENTITY REF: %s", node->name);
		break;
	case CONG_NODE_TYPE_ENTITY_NODE:
		text = g_strdup_printf("ENTITY");
		break;
	case CONG_NODE_TYPE_PI:
		text = g_strdup_printf("PI");
		break;
	case CONG_NODE_TYPE_COMMENT:
		cleaned_text = clean_text(node->content);
		text = g_strdup_printf(_("<span foreground=\"%s\">&lt;!-- %s --&gt;</span>"), colour_string, cleaned_text);
		g_free(cleaned_text);
		break;
	case CONG_NODE_TYPE_DOCUMENT:
		{
			xmlDocPtr doc = (xmlDocPtr)node;
			text = g_strdup_printf("<span foreground=\"%s\">&lt;?xml version="
					       "<span foreground=\"%s\">\"%s\"</span>"
					       " encoding="
					       "<span foreground=\"%s\">\"%s\"</span>"
					       " standalone="
					       "<span foreground=\"%s\">\"%s\"</span>"
					       "&gt;</span>",
					       
					       colour_string,

					       string_colour_string,
					       doc->version,

					       string_colour_string,
					       doc->encoding,

					       string_colour_string,
					       (doc->standalone?"yes":"no"));
		}
		break;
	case CONG_NODE_TYPE_DOCUMENT_TYPE:
		text = g_strdup_printf("DOCUMENT TYPE");
		break;
	case CONG_NODE_TYPE_DOCUMENT_FRAG:
		text = g_strdup_printf("DOCUMENT FRAG");
		break;
	case CONG_NODE_TYPE_NOTATION:
		text = g_strdup_printf("NOTATION");
		break;
	case CONG_NODE_TYPE_HTML_DOCUMENT:
		text = g_strdup_printf("HTML DOCUMENT");
		break;
	case CONG_NODE_TYPE_DTD:
		{
			text = g_strdup_printf("&lt;!DOCTYPE %s&gt;", node->name);
			/* FIXME: show any SYSTEM/PUBLIC DTD stuff */
		}
		break;
	case CONG_NODE_TYPE_ELEMENT_DECL:
		text = g_strdup_printf("ELEMENT DECL");
		break;
	case CONG_NODE_TYPE_ATRRIBUTE_DECL:
		text = g_strdup_printf("ATTRIBUTE DECL");
		break;
	case CONG_NODE_TYPE_ENTITY_DECL:
		text = g_strdup_printf("<span>&lt;!ENTITY %s ...&gt;</span>", node->name);
		break;
	case CONG_NODE_TYPE_NAMESPACE_DECL:
		text = g_strdup_printf("NAMESPACE DECL");
		break;
	case CONG_NODE_TYPE_XINCLUDE_START:
		text = g_strdup_printf("XINCLUDE START");
		break;
	case CONG_NODE_TYPE_XINCLUDE_END:
		text = g_strdup_printf("XINCLUDE END");
		break;		
	}

	return (text);
	
}

static void populate_tree_store_recursive(CongDOMViewDetails *details, CongNodePtr node, GtkTreeIter* tree_iter)
{
	CongDocument *doc;
	CongNodePtr node_iter;
	gchar* text;

	g_return_if_fail(details);
	g_return_if_fail(node);
	g_return_if_fail(tree_iter);

#if 0
	{
		gchar *test = cong_node_debug_description(node);
		g_message("populate_node_recursive: %s", test);
		g_free(test);
	}
#endif

	doc = cong_view_get_document(CONG_VIEW(details->dom_view));

	text = get_text_for_node(node);
	g_assert(text);

	gtk_tree_store_set (details->tree_store, 
			    tree_iter,
			    CONG_DOM_VIEW_TREEMODEL_COLUMN_TEXT, text,
			    CONG_DOM_VIEW_TREEMODEL_COLUMN_NODE, node,
			    CONG_DOM_VIEW_TREEMODEL_COLUMN_DOCUMENT, doc,
			    -1);

	g_free(text);

	if (cong_node_should_recurse(node)) {
		for (node_iter = cong_node_first_child(node); node_iter; node_iter = node_iter->next) {
			GtkTreeIter child_tree_iter;
			gtk_tree_store_append(details->tree_store, &child_tree_iter, tree_iter);
			
			populate_tree_store_recursive(details, node_iter, &child_tree_iter);
		}	
	}
}

static gint tree_popup_show(GtkWidget *widget, GdkEvent *event)
{
	GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_toplevel(widget));

	if (event->type == GDK_BUTTON_PRESS)
	{
		GdkEventButton *bevent = (GdkEventButton *) event;
		if (bevent->button != 3) return(FALSE);
		
 		/* printf("button 3\n"); */
 		{
			GtkTreePath* path;
			if ( gtk_tree_view_get_path_at_pos( GTK_TREE_VIEW(widget),
							    bevent->x,
							    bevent->y,
							    &path,
							    NULL,
							    NULL, 
							    NULL)
			     ) { 
				CongDOMView *cong_dom_view;
				GtkTreeModel* tree_model;
				GtkTreeIter iter;

				cong_dom_view = g_object_get_data(G_OBJECT(widget),
								   "cong_dom_view");
				g_assert(cong_dom_view);

				tree_model = GTK_TREE_MODEL(cong_dom_view->private->tree_store);
#if 0
				gchar* msg = gtk_tree_path_to_string(path);
				printf("right-click on path \"%s\"\n",msg);
				g_free(msg);
#endif
		    
				if ( gtk_tree_model_get_iter(tree_model, &iter, path) ) {
					CongNodePtr node;
					GtkWidget* menu;
					CongDocument* doc = cong_view_get_document(CONG_VIEW(cong_dom_view));
					
					gtk_tree_model_get(tree_model, &iter, CONG_DOM_VIEW_TREEMODEL_COLUMN_NODE, &node, -1);
					gtk_tree_model_get(tree_model, &iter, CONG_DOM_VIEW_TREEMODEL_COLUMN_DOCUMENT, &doc, -1);
					
					printf("got node \"%s\"\n",cong_dispspec_name_get(cong_document_get_dispspec(doc), node));
					
					menu = cong_ui_popup_init(doc, 
								  node, 
								  parent_window);
					gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, bevent->button,
						       bevent->time);		      
				}
				
				
				gtk_tree_path_free(path);		  
			}

 		}

		return(TRUE);
	}

	return(FALSE);
}

/* Prototypes of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent);
static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling);
static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling);
static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent); /* added to end of child list */
static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content);
static void on_selection_change(CongView *view);
static void on_cursor_change(CongView *view);

/* Definitions of the handler functions: */
static void on_document_node_make_orphan(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr former_parent)
{
	CongDOMView *test_view;
	GtkTreeIter tree_iter;

	g_return_if_fail(view);
	g_return_if_fail(node);

	#if DEBUG_DOM_VIEW
	g_message("CongDOMView - on_document_node_make_orphan\n");
	#endif

	if (before_event) {
		return;
	}

	test_view = CONG_DOM_VIEW(view);

	if ( get_iter_for_node(test_view->private, node, &tree_iter) ) {

		/* Remove this branch of the tree: */
		gtk_tree_store_remove(test_view->private->tree_store, &tree_iter);

	}
}

static void on_document_node_add_after(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr older_sibling)
{
	CongDOMView *test_view;
	GtkTreeIter tree_iter_sibling;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(older_sibling);

	#if DEBUG_DOM_VIEW
	g_message("CongDOMView - on_document_node_add_after\n");
	#endif

	if (before_event) {
		return;
	}

	test_view = CONG_DOM_VIEW(view);

	if ( get_iter_for_node(test_view->private, older_sibling, &tree_iter_sibling) ) {

		if ( get_iter_for_node(test_view->private, older_sibling->parent, &tree_iter_parent) ) {

			GtkTreeIter new_tree_iter;
			gtk_tree_store_insert_after(test_view->private->tree_store, &new_tree_iter, &tree_iter_parent, &tree_iter_sibling);

			populate_tree_store_recursive(test_view->private, node, &new_tree_iter);
		}
	}
}

static void on_document_node_add_before(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr younger_sibling)
{
	CongDOMView *test_view;
	GtkTreeIter tree_iter_sibling;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(younger_sibling);

	#if DEBUG_DOM_VIEW
	g_message("CongDOMView - on_document_node_add_before\n");
	#endif

	if (before_event) {
		return;
	}

	test_view = CONG_DOM_VIEW(view);

	if ( get_iter_for_node(test_view->private, younger_sibling, &tree_iter_sibling) ) {

		if ( get_iter_for_node(test_view->private, younger_sibling->parent, &tree_iter_parent) ) {
		
			GtkTreeIter new_tree_iter;
			gtk_tree_store_insert_before(test_view->private->tree_store, &new_tree_iter, &tree_iter_parent, &tree_iter_sibling);

			populate_tree_store_recursive(test_view->private, node, &new_tree_iter);
		}
	}
}

static void on_document_node_set_parent(CongView *view, gboolean before_event, CongNodePtr node, CongNodePtr adoptive_parent)
{
	CongDOMView *test_view;
	GtkTreeIter tree_iter_node;
	GtkTreeIter tree_iter_parent;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(adoptive_parent);

	#if DEBUG_DOM_VIEW
	g_message("CongDOMView - on_document_node_set_parent\n");
	#endif

	if (before_event) {
		return;
	}

	test_view = CONG_DOM_VIEW(view);

	if ( get_iter_for_node(test_view->private, node, &tree_iter_node) ) {
		/* Remove this branch of the tree: */
		gtk_tree_store_remove(test_view->private->tree_store, &tree_iter_node);
	}

	if ( get_iter_for_node(test_view->private, adoptive_parent, &tree_iter_parent) ) {
		GtkTreeIter new_tree_iter;
		gtk_tree_store_append(test_view->private->tree_store, &new_tree_iter, &tree_iter_parent);

		populate_tree_store_recursive(test_view->private, node, &new_tree_iter);
	}
}

static void on_document_node_set_text(CongView *view, gboolean before_event, CongNodePtr node, const xmlChar *new_content)
{
	CongDOMView *test_view;
	GtkTreeIter tree_iter;

	g_return_if_fail(view);
	g_return_if_fail(node);
	g_return_if_fail(new_content);

	#if DEBUG_DOM_VIEW
	g_message("CongDOMView - on_document_node_set_text\n");
	#endif

	if (before_event) {
		return;
	}

	test_view = CONG_DOM_VIEW(view);

	if ( get_iter_for_node(test_view->private, node, &tree_iter) ) {
		gchar *text = NULL;

		g_assert(cong_node_type(node) == CONG_NODE_TYPE_TEXT);

		text = get_text_for_node(node);
		g_assert(text);

		gtk_tree_store_set(test_view->private->tree_store, 
				   &tree_iter,
				   CONG_DOM_VIEW_TREEMODEL_COLUMN_TEXT, text,
				   -1);

		g_free(text);
	}
}

static void on_selection_change(CongView *view)
{
}

static void on_cursor_change(CongView *view)
{
}

GtkWidget *cong_dom_view_new(CongDocument *doc)
{
	CongDOMViewDetails *details;
	CongDOMView *view;
	GtkCellRenderer *renderer;
 	GtkTreeViewColumn *column;
	GtkTreeIter root_iter;
	CongNodePtr node_iter;

	g_return_val_if_fail(doc, NULL);

	view = g_new0(CongDOMView,1);
	details = g_new0(CongDOMViewDetails,1);
	
	view->private = details;

	details->dom_view=view;
	
	view->view.doc = doc;
	view->view.klass = g_new0(CongViewClass,1);
	view->view.klass->on_document_node_make_orphan = on_document_node_make_orphan;
	view->view.klass->on_document_node_add_after = on_document_node_add_after;
	view->view.klass->on_document_node_add_before = on_document_node_add_before;
	view->view.klass->on_document_node_set_parent = on_document_node_set_parent;
	view->view.klass->on_document_node_set_text = on_document_node_set_text;
	view->view.klass->on_selection_change = on_selection_change;
	view->view.klass->on_cursor_change = on_cursor_change;

	cong_document_register_view( doc, CONG_VIEW(view) );
	
	details->scrolled_window = GTK_SCROLLED_WINDOW( gtk_scrolled_window_new(NULL, NULL) );
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(details->scrolled_window), 
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(GTK_WIDGET(details->scrolled_window), 100, 50);

        details->tree_store = gtk_tree_store_new (CONG_DOM_VIEW_TREEMODEL_NUM_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_POINTER);
	details->tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model (GTK_TREE_MODEL(details->tree_store)));

	g_object_set_data(G_OBJECT(details->tree_view),
			  "cong_dom_view",
			  view);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(details->scrolled_window), GTK_WIDGET(details->tree_view));

	renderer = gtk_cell_renderer_text_new ();

	/* Create a column, associating the "text" attribute of the
	 * cell_renderer to the first column of the model */
	column = gtk_tree_view_column_new_with_attributes (_("Element"), 
							   renderer,
							   "markup", CONG_DOM_VIEW_TREEMODEL_COLUMN_TEXT,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (details->tree_view), column);

 	/* Wire up the context-menu callback */
 	gtk_signal_connect_object(GTK_OBJECT(details->tree_view), "event",
 				  (GtkSignalFunc) tree_popup_show, details->tree_view);

	gtk_widget_show(GTK_WIDGET(details->tree_view));
	gtk_widget_show(GTK_WIDGET(details->scrolled_window));

	/* Populate the tree: */
#if 0
	for (node_iter = cong_document_get_root(doc); node_iter; node_iter = node_iter->next) {
		gtk_tree_store_append (details->tree_store, &root_iter, NULL);  /* Acquire a top-level iterator */
		populate_tree_store_recursive(details, node_iter, &root_iter);
	}	
#else
	gtk_tree_store_append (details->tree_store, &root_iter, NULL);  /* Acquire a top-level iterator */
	populate_tree_store_recursive(details, (CongNodePtr)cong_document_get_xml(doc), &root_iter);
#endif

	return GTK_WIDGET(details->scrolled_window);	
}


