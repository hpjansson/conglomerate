/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"
#include "cong-selection.h"
#include <libgnome/gnome-macros.h>
#include "cong-util.h"
#include "cong-marshal.h"
#include "cong-document-traversal.h"
#include "cong-traversal-node.h"

enum {
	TRAVERSAL_NODE_ADDED,
	TRAVERSAL_NODE_REMOVED,

	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};


#if 1
typedef struct NodeMapping NodeMapping;

/* A per-xml-node struct that maps from traversal_parent ptrs to TraversalNode objects: */
struct NodeMapping
{
	CongNodePtr xml_node;
	GHashTable *hash_of_traversal_parent_to_traversal_node;
};

NodeMapping*
cong_node_mapping_new (CongNodePtr xml_node);

void
cong_node_mapping_free (NodeMapping* mapping);

void
cong_node_mapping_add_editor_node (NodeMapping* mapping,
				   CongTraversalNode *traversal_parent,
				   CongTraversalNode *editor_node);
void
cong_node_mapping_remove_editor_node (NodeMapping* mapping,
				      CongTraversalNode *traversal_parent);

void
cong_node_mapping_for_each_traversal_node (NodeMapping* mapping,
					   CongDocumentTraversal *doc_traversal,
					   void (*callback) (CongDocumentTraversal *doc_traversal, 
							     CongTraversalNode *traversal_node, 
							     gpointer user_data),
					   gpointer user_data);

static void  
value_destroy_func (gpointer data)
{
	NodeMapping *mapping = (NodeMapping*)data;

	cong_node_mapping_free (mapping);
}
#endif

/* Internal functions: */
static void
finalize (GObject *object);

static void
dispose (GObject *object);

gboolean
should_have_traversal_node (CongNodePtr node);

static CongTraversalNode*
recursive_create_traversal_nodes (CongDocumentTraversal *doc_traversal, 
				  CongNodePtr xml_node,
				  CongTraversalNode *parent_traversal_node);

static void
recursive_destroy_traversal_nodes (CongDocumentTraversal *doc_traversal, 
				   CongNodePtr xml_node,
				   CongTraversalNode *traversal_node);

static void
recursive_create_traversal_nodes_for_xml_node (CongDocumentTraversal *doc_traversal,
					       CongNodePtr xml_node);

static void
recursive_destroy_traversal_nodes_for_xml_node (CongDocumentTraversal *doc_traversal,
						CongNodePtr xml_node);

/* Signal handling callbacks: */
/* Callbacks attached before the default handler: */
static void 
on_signal_make_orphan_notify_before (CongDocument *doc, 
				     CongNodePtr node, 
				     gpointer user_data);
static void 
on_signal_set_parent_notify_before (CongDocument *doc, 
				    CongNodePtr node, 
				    CongNodePtr adoptive_parent, 
				    gpointer user_data);

/* Callbacks attached after the default handler: */
static void 
on_signal_add_after_notify_after (CongDocument *doc, 
				  CongNodePtr node, 
				  CongNodePtr older_sibling, 
				  gpointer user_data);
static void 
on_signal_add_before_notify_after (CongDocument *doc, 
				   CongNodePtr node, 
				   CongNodePtr younger_sibling, 
				   gpointer user_data);
static void 
on_signal_set_parent_notify_after (CongDocument *doc, 
				   CongNodePtr node, 
				   CongNodePtr adoptive_parent, 
				   gpointer user_data);


#define LOG_TRAVERSAL_NODES 0

#if LOG_TRAVERSAL_NODES
#define LOG_TRAVERSAL_NODE1(x) g_message(x)
#define LOG_TRAVERSAL_NODE2(x, a) g_message((x), (a))
#else
#define LOG_TRAVERSAL_NODE1(x) ((void)0)
#define LOG_TRAVERSAL_NODE2(x, a) ((void)0)
#endif

#define PRIVATE(x) ((x)->private)


struct CongDocumentTraversalDetails
{
	CongDocument *doc;

	GHashTable *hash_of_xml_node_to_node_mapping;
	CongTraversalNode *root_traversal_node;

};

/* Exported function definitions: */
GNOME_CLASS_BOILERPLATE(CongDocumentTraversal, 
			cong_document_traversal,
			GObject,
			G_TYPE_OBJECT );

static void
cong_document_traversal_class_init (CongDocumentTraversalClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = finalize;
	G_OBJECT_CLASS (klass)->dispose = dispose;

	/* Set up signals: */
	signals[TRAVERSAL_NODE_ADDED] = g_signal_new ("traversal_node_added",
						      CONG_DOCUMENT_TRAVERSAL_TYPE,
						      G_SIGNAL_RUN_LAST,
						      0,
						      NULL, NULL,
						      g_cclosure_marshal_VOID__POINTER,
						      G_TYPE_NONE, 
						      1, G_TYPE_POINTER);
	
	signals[TRAVERSAL_NODE_REMOVED] = g_signal_new ("traversal_node_removed",
							CONG_DOCUMENT_TRAVERSAL_TYPE,
							G_SIGNAL_RUN_LAST,
							0,
							NULL, NULL,
							g_cclosure_marshal_VOID__POINTER,
							G_TYPE_NONE, 
							1, G_TYPE_POINTER);
}

static void
cong_document_traversal_instance_init (CongDocumentTraversal *doc)
{
	doc->private = g_new0(CongDocumentTraversalDetails,1);
}

CongDocumentTraversal*
cong_document_traversal_construct (CongDocumentTraversal *doc_traversal,
				   CongDocument *doc)
{
	g_return_val_if_fail (IS_CONG_DOCUMENT_TRAVERSAL (doc_traversal), NULL);
	g_return_val_if_fail (IS_CONG_DOCUMENT (doc), NULL);

	PRIVATE(doc_traversal)->doc = doc;
	PRIVATE(doc_traversal)->hash_of_xml_node_to_node_mapping = g_hash_table_new_full (NULL,
											  NULL,
											  NULL,
											  value_destroy_func);
	PRIVATE(doc_traversal)->root_traversal_node = recursive_create_traversal_nodes (doc_traversal,
											(CongNodePtr)cong_document_get_xml (doc), 
											NULL);

	/* Connect to signals: */
	g_signal_connect (G_OBJECT(doc), "node_make_orphan", G_CALLBACK(on_signal_make_orphan_notify_before), doc_traversal);
	g_signal_connect_after (G_OBJECT(doc), "node_add_after", G_CALLBACK(on_signal_add_after_notify_after), doc_traversal);
	g_signal_connect_after (G_OBJECT(doc), "node_add_before", G_CALLBACK(on_signal_add_before_notify_after), doc_traversal);
	g_signal_connect (G_OBJECT(doc), "node_set_parent", G_CALLBACK(on_signal_set_parent_notify_before), doc_traversal);
	g_signal_connect_after (G_OBJECT(doc), "node_set_parent", G_CALLBACK(on_signal_set_parent_notify_after), doc_traversal);

	return doc_traversal;
}

CongDocumentTraversal*
cong_document_traversal_new (CongDocument *doc)
{
	return cong_document_traversal_construct (g_object_new (CONG_DOCUMENT_TRAVERSAL_TYPE, NULL),
						  doc);
}

CongDocument*
cong_document_traversal_get_document(CongDocumentTraversal *doc_traversal)
{
	g_return_val_if_fail (IS_CONG_DOCUMENT_TRAVERSAL(doc_traversal), NULL);

	return PRIVATE(doc_traversal)->doc;
}

CongTraversalNode*
cong_document_traversal_get_root_traversal_node (CongDocumentTraversal *doc_traversal)
{
	g_return_val_if_fail (IS_CONG_DOCUMENT_TRAVERSAL(doc_traversal), NULL);
	
	return cong_document_traversal_get_traversal_node (doc_traversal,
							   (xmlNodePtr)cong_document_get_xml (PRIVATE(doc_traversal)->doc),
							   NULL);
}

void
cong_document_traversal_for_each_traversal_node (CongDocumentTraversal *doc_traversal,
						 CongNodePtr xml_node,
						 void (*callback) (CongDocumentTraversal *doc_traversal, 
								   CongTraversalNode *traversal_node, 
								   gpointer user_data),
						 gpointer user_data)
{
	NodeMapping *node_mapping;

	g_return_if_fail (IS_CONG_DOCUMENT_TRAVERSAL (doc_traversal));
	g_return_if_fail (xml_node);
	g_return_if_fail (callback);

	node_mapping = g_hash_table_lookup (PRIVATE(doc_traversal)->hash_of_xml_node_to_node_mapping,
					    xml_node);

	if (node_mapping) {
		cong_node_mapping_for_each_traversal_node (node_mapping,
							   doc_traversal,
							   callback,
							   user_data);
	}
}

CongTraversalNode*
cong_document_traversal_get_traversal_node (CongDocumentTraversal *doc_traversal,
					    CongNodePtr xml_node,
					    CongTraversalNode *traversal_parent)
{
	NodeMapping *node_mapping;

	g_return_val_if_fail (IS_CONG_DOCUMENT_TRAVERSAL(doc_traversal), NULL);
	g_return_val_if_fail (xml_node, NULL);


	node_mapping = g_hash_table_lookup (PRIVATE(doc_traversal)->hash_of_xml_node_to_node_mapping,
					    xml_node);

	if (node_mapping) {
		g_assert (node_mapping->xml_node == xml_node);

		return CONG_TRAVERSAL_NODE (g_hash_table_lookup (node_mapping->hash_of_traversal_parent_to_traversal_node,
								 traversal_parent));
	} else {
		g_message ("Node not found in CongDocumentTraversal");
		return NULL;
	}
}

static void
foreach_cb_store (gpointer key,
		  gpointer value,
		  gpointer user_data)
{
	CongTraversalNode **result = (CongTraversalNode**)user_data;

	*result = CONG_TRAVERSAL_NODE (value);
}

CongTraversalNode*
cong_document_traversal_get_a_traversal_node (CongDocumentTraversal *doc_traversal,
					      CongNodePtr xml_node)
{
	NodeMapping *node_mapping;

	g_return_val_if_fail (IS_CONG_DOCUMENT_TRAVERSAL(doc_traversal), NULL);
	g_return_val_if_fail (xml_node, NULL);

	node_mapping = g_hash_table_lookup (PRIVATE(doc_traversal)->hash_of_xml_node_to_node_mapping,
					    xml_node);

	if (node_mapping) {
		/* Return any value in table: */
		CongTraversalNode *result = NULL;
		g_assert (node_mapping->xml_node == xml_node);

		g_hash_table_foreach (node_mapping->hash_of_traversal_parent_to_traversal_node,
				      foreach_cb_store,
				      &result);
		return result;

	} else {
		g_message ("Node not found in CongDocumentTraversal");
		return NULL;
	}	
}

/* Internal function definitions: */
NodeMapping*
cong_node_mapping_new (CongNodePtr xml_node)
{
	NodeMapping *mapping;

	g_assert (xml_node);

	mapping = g_new0(NodeMapping, 1);

	mapping->xml_node = xml_node;
	mapping->hash_of_traversal_parent_to_traversal_node = g_hash_table_new (NULL, 
										NULL);

	return mapping;
}

void
cong_node_mapping_free (NodeMapping* mapping)
{
	g_assert(mapping);

	g_hash_table_destroy ( mapping->hash_of_traversal_parent_to_traversal_node);

	g_free (mapping);
}

void
cong_node_mapping_add_traversal_node (NodeMapping* mapping,
				      CongTraversalNode *traversal_parent,
				      CongTraversalNode *traversal_node)
{
	g_assert(mapping);
	g_assert(traversal_node);
	
	g_assert(mapping->hash_of_traversal_parent_to_traversal_node);

	g_hash_table_insert (mapping->hash_of_traversal_parent_to_traversal_node,
			     traversal_parent,
			     traversal_node);
	
}

void
cong_node_mapping_remove_traversal_node (NodeMapping* mapping,
					 CongTraversalNode *traversal_parent)
{
	g_assert(mapping);
	g_assert(mapping->hash_of_traversal_parent_to_traversal_node);

	g_hash_table_remove (mapping->hash_of_traversal_parent_to_traversal_node,
			     traversal_parent);
}

struct node_mapping_foreach_data
{
	CongDocumentTraversal *doc_traversal;
	void (*outer_callback) (CongDocumentTraversal *doc_traversal, CongTraversalNode *traversal_node, gpointer user_data);
	gpointer outer_user_data;

};

static void
node_mapping_foreach_cb (gpointer key,
			 gpointer value,
			 gpointer user_data)
{
	struct node_mapping_foreach_data *foreach_data = (struct node_mapping_foreach_data *)user_data;

	foreach_data->outer_callback (foreach_data->doc_traversal,
				      CONG_TRAVERSAL_NODE (value),
				      foreach_data->outer_user_data);
}

void
cong_node_mapping_for_each_traversal_node (NodeMapping* node_mapping,
					   CongDocumentTraversal *doc_traversal,
					   void (*callback) (CongDocumentTraversal *doc_traversal, CongTraversalNode *traversal_node, gpointer user_data),
					   gpointer user_data)
{
	struct node_mapping_foreach_data foreach_data;

	g_assert (node_mapping);
	g_assert (doc_traversal);
	g_assert (callback);

	foreach_data.doc_traversal = doc_traversal;
	foreach_data.outer_callback = callback;
	foreach_data.outer_user_data = user_data;

	g_hash_table_foreach (node_mapping->hash_of_traversal_parent_to_traversal_node,
			      node_mapping_foreach_cb,
			      &foreach_data);
}

static void
add_node_mapping (CongDocumentTraversal *doc_traversal,
		  CongNodePtr xml_node,
		  CongTraversalNode *traversal_node,
		  CongTraversalNode *traversal_parent)
{
	NodeMapping *node_mapping;

	g_assert (IS_CONG_DOCUMENT_TRAVERSAL (doc_traversal));
	g_assert (xml_node);
	g_assert (traversal_node);

	/* Claim our reference on the editor node: */
	g_object_ref (G_OBJECT(traversal_node));


	node_mapping = g_hash_table_lookup (PRIVATE(doc_traversal)->hash_of_xml_node_to_node_mapping,
					    xml_node);

	if (NULL==node_mapping) {
		node_mapping = cong_node_mapping_new (xml_node);

		g_hash_table_insert (PRIVATE(doc_traversal)->hash_of_xml_node_to_node_mapping,
				     xml_node,
				     node_mapping);
	}

	g_assert (node_mapping);

	cong_node_mapping_add_traversal_node (node_mapping,
					      traversal_parent,
					      traversal_node);
}

static void
remove_node_mapping (CongDocumentTraversal *doc_traversal,
		     CongNodePtr xml_node,
		     CongTraversalNode *traversal_node,
		     CongTraversalNode *traversal_parent)
{
	NodeMapping *node_mapping;

	g_assert (IS_CONG_DOCUMENT_TRAVERSAL (doc_traversal));
	g_assert (xml_node);
	g_assert (traversal_node);

	node_mapping = g_hash_table_lookup (PRIVATE(doc_traversal)->hash_of_xml_node_to_node_mapping,
					    xml_node);

	g_assert(node_mapping);

	cong_node_mapping_remove_traversal_node (node_mapping,
						traversal_parent);

	/* FIXME: is there a cheaper way to do this? */
	if (0==g_hash_table_size(PRIVATE(doc_traversal)->hash_of_xml_node_to_node_mapping)) {
		g_hash_table_remove (PRIVATE(doc_traversal)->hash_of_xml_node_to_node_mapping,
				     xml_node);
	}

#if 0
	CONG_EEL_LOG_REF_COUNT("redundant editor node", G_OBJECT(traversal_node));
#endif
	
	/* Release our reference on the traversal_node: */
	g_object_unref (traversal_node);
}

static CongTraversalNode*
recursive_create_traversal_nodes (CongDocumentTraversal *doc_traversal,
				  CongNodePtr xml_node,
				  CongTraversalNode *traversal_parent)
{
	CongTraversalNode *traversal_node;


#if LOG_TRAVERSAL_NODES
	{
		gchar *node_desc = cong_node_debug_description (xml_node);

		LOG_TRAVERSAL_NODE2("recursive_create_traversal_nodes: %s", node_desc);

		g_free (node_desc);
	}
#endif

	g_assert(should_have_traversal_node (xml_node));

	/* Add this node: */
	{
		traversal_node = cong_traversal_node_new (doc_traversal,
							  xml_node,
							  traversal_parent);
		g_assert(traversal_node);

		add_node_mapping (doc_traversal,
				  xml_node,
				  traversal_node,
				  traversal_parent);

		/* Our initial reference is now held by the node mapping: */
		g_object_unref (G_OBJECT(traversal_node));

	}

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc_traversal),
		       signals[TRAVERSAL_NODE_ADDED], 0,
		       traversal_node);

	
	/* Recurse: */
	if (xml_node->type==XML_ENTITY_REF_NODE) {
		/* Only visit the explicit child - this is the relevant definition: */
		recursive_create_traversal_nodes (doc_traversal, 
						  xml_node->children,
						  traversal_node);
		
	} else {
		CongNodePtr iter;

		/* Visit every child: */
		for (iter = xml_node->children; iter; iter=iter->next) {
			recursive_create_traversal_nodes (doc_traversal, 
							  iter,
							  traversal_node);
		}
	}

	return traversal_node;
}

static void 
recursive_destroy_traversal_nodes (CongDocumentTraversal *doc_traversal,
				   CongNodePtr xml_node,
				   CongTraversalNode *traversal_parent)
{
	CongTraversalNode *traversal_node;

#if LOG_TRAVERSAL_NODES
	{
		gchar *node_desc = cong_node_debug_description (xml_node);

		LOG_TRAVERSAL_NODE2("recursive_destroy_traversal_nodes: %s", node_desc);

		g_free (node_desc);
	}
#endif

	g_assert(should_have_traversal_node (xml_node));

	traversal_node = cong_document_traversal_get_traversal_node (doc_traversal,
								     xml_node,
								     traversal_parent);
	g_assert(traversal_node);

	/* Recurse: */
	if (xml_node->type==XML_ENTITY_REF_NODE) {
		/* Only visit the explicit child - this is the relevant definition: */
		recursive_destroy_traversal_nodes (doc_traversal, 
						   xml_node->children,
						   traversal_node);		
	} else {		
		CongNodePtr iter;

		for (iter = xml_node->children; iter; iter=iter->next) {
			recursive_destroy_traversal_nodes (doc_traversal, 
							   iter,
							   traversal_node);		
		}
	}

	/* Emit signal: */
	g_signal_emit (G_OBJECT(doc_traversal),
		       signals[TRAVERSAL_NODE_REMOVED], 0,
		       traversal_node);

	/* Remove this traversal_node: */
	remove_node_mapping (doc_traversal,
			     xml_node,
			     traversal_node,
			     traversal_parent);
}

static void 
recursive_create_nodes_cb (CongDocumentTraversal *doc_traversal, 
			   CongTraversalNode *parent_traversal_node, 
			   gpointer user_data)
{
	xmlNodePtr xml_node;
	g_assert (IS_CONG_DOCUMENT_TRAVERSAL (doc_traversal));
	g_assert (IS_CONG_TRAVERSAL_NODE (parent_traversal_node));

	xml_node = (xmlNodePtr)user_data;
	g_assert (xml_node->parent == cong_traversal_node_get_node (parent_traversal_node));
	
	recursive_create_traversal_nodes (doc_traversal,
					  xml_node,
					  parent_traversal_node);
}

static void
recursive_create_traversal_nodes_for_xml_node (CongDocumentTraversal *doc_traversal,
					       CongNodePtr xml_node)
{
	g_assert (IS_CONG_DOCUMENT_TRAVERSAL(doc_traversal));
	g_assert (xml_node);
	g_assert (xml_node->parent);

	/* Create children below each traversal parent: */
	cong_document_traversal_for_each_traversal_node (doc_traversal,
							 xml_node->parent,
							 recursive_create_nodes_cb,
							 xml_node);
}

static void 
recursive_destroy_nodes_cb (CongDocumentTraversal *doc_traversal, 
			    CongTraversalNode *parent_traversal_node, 
			    gpointer user_data)
{
	xmlNodePtr xml_node;
	g_assert (IS_CONG_DOCUMENT_TRAVERSAL (doc_traversal));
	g_assert (IS_CONG_TRAVERSAL_NODE (parent_traversal_node));

	xml_node = (xmlNodePtr)user_data;
	g_assert (xml_node->parent == cong_traversal_node_get_node (parent_traversal_node));
	
	recursive_destroy_traversal_nodes (doc_traversal,
					   xml_node,
					   parent_traversal_node);
}

static void
recursive_destroy_traversal_nodes_for_xml_node (CongDocumentTraversal *doc_traversal, 
						CongNodePtr xml_node)
{
	g_assert (IS_CONG_DOCUMENT_TRAVERSAL(doc_traversal));
	g_assert (xml_node);

	if (xml_node->parent) {
		/* Destroy children below each traversal parent: */
		cong_document_traversal_for_each_traversal_node (doc_traversal,
								 xml_node->parent,
								 recursive_destroy_nodes_cb,
								 xml_node);
	}
}


/* Internal function definitions: */
static void
finalize (GObject *object)
{
	CongDocumentTraversal *doc_traversal = CONG_DOCUMENT_TRAVERSAL (object);

	g_message ("CongDocumentTraversal::finalize");
	
	g_free (doc_traversal->private);
	doc_traversal->private = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
dispose (GObject *object)
{
	CongDocumentTraversal *doc_traversal = CONG_DOCUMENT_TRAVERSAL (object);

	g_message ("CongDocumentTraversal::dispose");

	g_assert (doc_traversal->private);

	/* FIXME: */
	
	/* Call the parent method: */		
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

gboolean
should_have_traversal_node (CongNodePtr node)
{
	if (node->parent) {
		return should_have_traversal_node (node->parent);
	} else {
		return cong_node_type(node)==CONG_NODE_TYPE_DOCUMENT;
	}
}

/* Signal handling callbacks: */
/* Callbacks attached before the default handler: */
static void 
on_signal_make_orphan_notify_before (CongDocument *doc, 
				     CongNodePtr node, 
				     gpointer user_data)
{
	CongDocumentTraversal *doc_traversal = CONG_DOCUMENT_TRAVERSAL (user_data);
	g_assert (node);

	/* Remove any traversal node: */
	if (should_have_traversal_node(node)) {
		recursive_destroy_traversal_nodes_for_xml_node (doc_traversal, 
								node);
	} else {
#if 0
		g_assert(!cong_document_traversal_has_traversal_node_for_node (doc_traversal,node));
#endif
	}

}

static void 
on_signal_set_parent_notify_before (CongDocument *doc, 
				    CongNodePtr node, 
				    CongNodePtr adoptive_parent, 
				    gpointer user_data)
{
	CongDocumentTraversal *doc_traversal = CONG_DOCUMENT_TRAVERSAL (user_data);
	g_assert (node);

	/* Remove any traversal node: */
	if (should_have_traversal_node(node)) {
		recursive_destroy_traversal_nodes_for_xml_node (doc_traversal, 
								node);
	} else {
#if 0
		g_assert(!cong_document_traversal_has_traversal_node_for_node (doc_traversal,node));
#endif
	}	
}

/* Callbacks attached after the default handler: */
static void on_signal_add_after_notify_after (CongDocument *doc, 
				       CongNodePtr node, 
				       CongNodePtr older_sibling, 
				       gpointer user_data)
{
	CongDocumentTraversal *doc_traversal = CONG_DOCUMENT_TRAVERSAL (user_data);
	g_assert (node);

	/* Create any traversal nodes: */
	recursive_create_traversal_nodes_for_xml_node (doc_traversal,
						       node);

}
static void on_signal_add_before_notify_after (CongDocument *doc, 
					CongNodePtr node, 
					CongNodePtr younger_sibling, 
					gpointer user_data)
{
	CongDocumentTraversal *doc_traversal = CONG_DOCUMENT_TRAVERSAL (user_data);
	g_assert (node);

	/* Create any traversal nodes: */
	recursive_create_traversal_nodes_for_xml_node (doc_traversal, 
						       node);


}
static void on_signal_set_parent_notify_after (CongDocument *doc, 
					CongNodePtr node, 
					CongNodePtr adoptive_parent, 
					gpointer user_data)
{	
	CongDocumentTraversal *doc_traversal = CONG_DOCUMENT_TRAVERSAL (user_data);
	g_assert (node);

	/* Create any traversal nodes: */
	recursive_create_traversal_nodes_for_xml_node (doc_traversal, 
						       node);
}
