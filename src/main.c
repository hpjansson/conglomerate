/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "global.h"
#include "cong-dialog.h"
#include "cong-error-dialog.h"
#include "cong-dispspec.h"
#include "cong-dispspec-registry.h"
#include "cong-document.h"
#include "cong-plugin.h"

#include <libxml/tree.h>

#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>

#define STYLESHEET_PATH "/usr/share/sgml/docbkxsl/"

#if 0
#include <libgtkhtml/gtkhtml.h>
#endif

#if PRINT_TESTS
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprintui/gnome-print-master-preview.h>
#endif

void get_example(GtkWidget *w, gpointer data);
gint set_vectors(GtkWidget *w, gpointer data);

void xed_cut_wrap(GtkWidget *widget, gpointer data) { xed_cut(widget, 0); }
void xed_copy_wrap(GtkWidget *widget, gpointer data) { xed_copy(widget, 0); }
void xed_paste_wrap(GtkWidget *widget, gpointer data) { xed_paste(widget, 0); }

/*
#define AUTOGENERATE_DS
*/

/*
  A routine that tries to load all the bytes requested from the handle into the buffer and bails out on any failure
 */
GnomeVFSResult
cong_vfs_read_bytes(GnomeVFSHandle* vfs_handle, char* buffer, GnomeVFSFileSize bytes)
{
	GnomeVFSFileSize bytes_read;
	GnomeVFSResult vfs_result = gnome_vfs_read(vfs_handle,buffer,bytes,&bytes_read);

	g_assert(bytes==bytes_read); /* for now */

	return vfs_result;
}

/* 
   A routine that tries to syncronously load a file into a buffer in memory (surely this exists already somewhere?)
*/
GnomeVFSResult
cong_vfs_new_buffer_from_file(const char* filename, char** buffer, GnomeVFSFileSize* size)
{
	GnomeVFSResult vfs_result;
	GnomeVFSURI* uri;

	g_return_val_if_fail(filename,GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(buffer,GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(size,GNOME_VFS_ERROR_BAD_PARAMETERS);

	uri = gnome_vfs_uri_new(filename);

	vfs_result = cong_vfs_new_buffer_from_uri(uri, buffer, size);

	gnome_vfs_uri_unref(uri);

	return vfs_result;
}

GnomeVFSResult
cong_vfs_new_buffer_from_uri(GnomeVFSURI* uri, char** buffer, GnomeVFSFileSize* size)
{
	GnomeVFSResult vfs_result;
	GnomeVFSHandle *vfs_handle;

	g_return_val_if_fail(uri,GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(buffer,GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail(size,GNOME_VFS_ERROR_BAD_PARAMETERS);

	vfs_result = gnome_vfs_open_uri(&vfs_handle,
					uri,
					GNOME_VFS_OPEN_READ);

	if (GNOME_VFS_OK!=vfs_result) {
		return vfs_result;
	} else {
		GnomeVFSFileInfo info;
		*buffer=NULL;
		
		/* Get the size of the file: */
		vfs_result = gnome_vfs_get_file_info_from_handle(vfs_handle,
								 &info,
								 GNOME_VFS_FILE_INFO_DEFAULT);
		if (GNOME_VFS_OK!=vfs_result) {
			gnome_vfs_close(vfs_handle);
			
			return vfs_result;
		}

		if (!(info.valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE)) {
			gnome_vfs_close(vfs_handle);
			
			return GNOME_VFS_ERROR_IO; /* FIXME: is this appropriate? */
		}

		
		/* Allocate the buffer: */
		*buffer = g_malloc(info.size);
		
		/* Read the file into the buffer: */
		vfs_result = cong_vfs_read_bytes(vfs_handle, *buffer, info.size);
		
		if (GNOME_VFS_OK!=vfs_result) {
			
			g_free(*buffer);
			gnome_vfs_close(vfs_handle);

			*buffer=NULL;
			
			return vfs_result;
		}
		
		gnome_vfs_close(vfs_handle);
		*size = info.size;

		return GNOME_VFS_OK;
	}
}

int test_open_do(const char *doc_name, const char *ds_name)
{
	return (TRUE);
}


gint test_open(GtkWidget *w, gpointer data)
{
#if 1
	const char *doc_name, *ds_name;
	
	doc_name = get_file_name("Select an XML document");
	if (!doc_name) return(TRUE);

	ds_name = get_file_name("Select a matching XDS displayspec");
	if (!ds_name) return(TRUE);

	test_open_do(doc_name, ds_name);
#else
	#if 0
	test_open_do("../examples/hacked_gconf.sgml", "../examples/docbook.xds");
	#else
	/* test_open_do("../examples/readme.xml", "../examples/readme.xds"); */
	test_open_do("../examples/test-docbook.xml", "../examples/docbook.xds");
	#endif
#endif
	return(TRUE);
}

void test_open_wrap(GtkWidget *widget, gpointer data) { test_open(widget, 0); }

gint test_error(GtkWidget *w, gpointer data)
{
	cong_error_tests();

	return TRUE;
}

void test_error_wrap(GtkWidget *widget, gpointer data) { test_error(widget, 0); }

enum
{
	DOCTYPELIST_NAME_COLUMN,
	DOCTYPELIST_DESCRIPTION_COLUMN,
	DOCTYPELIST_N_COLUMNS
};

gint test_document_types(GtkWidget *w, gpointer data)
{
	GtkWidget* dialog;
	GtkWidget* list_view;

	GtkListStore *store;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	dialog = gtk_dialog_new();

	gtk_window_set_title(GTK_WINDOW(dialog), "Document Types");

	store = gtk_list_store_new (DOCTYPELIST_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);

	list_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

	/* The view now holds a reference.  We can get rid of our own
	 * reference */
	g_object_unref (G_OBJECT (store));

	/* Populate the store based on the ds-registry: */
	{
		CongDispspecRegistry* registry = the_globals.ds_registry;
		int i;

		for (i=0;i<cong_dispspec_registry_get_num(registry);i++) {
			const CongDispspec* ds = cong_dispspec_registry_get(registry,i);
			
			GtkTreeIter iter;
			gtk_list_store_append (store, &iter);  /* Acquire an iterator */
			
			gtk_list_store_set (store, &iter,
					    DOCTYPELIST_NAME_COLUMN, cong_dispspec_get_name(ds),
					    DOCTYPELIST_DESCRIPTION_COLUMN, cong_dispspec_get_description(ds),
					    -1);
		}
	}

	renderer = gtk_cell_renderer_text_new ();

	column = gtk_tree_view_column_new_with_attributes ("Name", renderer,
							   "text", DOCTYPELIST_NAME_COLUMN,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (list_view), column);

	column = gtk_tree_view_column_new_with_attributes ("Description", renderer,
							   "text", DOCTYPELIST_DESCRIPTION_COLUMN,
							   NULL);

	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (list_view), column);

	gtk_widget_show (GTK_WIDGET(list_view));

	gtk_container_add (GTK_CONTAINER( GTK_DIALOG (dialog)->vbox ),
			   list_view);

	gtk_dialog_add_button(GTK_DIALOG(dialog),
			      "gtk-ok",
			      GTK_RESPONSE_OK);

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));

	return TRUE;
}

void test_document_types_wrap(GtkWidget *widget, gpointer data) { test_document_types(widget, 0); }

void open_preview_window_for_doc(xmlDocPtr doc)
{
#if 1
	/* Save it to a temp file and invoke user's favourite browser: */
	g_assert(0);
#else
	GtkWidget* html_view;
	HtmlDocument *html_document;

	g_return_if_fail(doc);

	html_view = html_view_new();

	html_document = ;

	html_view_set_document(HTML_VIEW(html_view), html_document);
#endif
}

void open_transformed_window_for_doc(xmlDocPtr doc)
{
	/* Hackish test: */

	const CongDispspec *ds;
	CongDocument *cong_doc;

	g_return_if_fail(doc);

	ds = get_appropriate_dispspec(doc);

	if (ds==NULL) {
		ds = query_for_forced_dispspec("Conglomerate cannot open the result of the transformation", doc);

		if (NULL==ds) {
			xmlFreeDoc(doc);
			return;
		}
	}

	g_assert(ds);
	cong_doc = cong_document_new_from_xmldoc(doc, ds, NULL); /* takes ownership of doc */

	cong_node_self_test_recursive(cong_document_get_root(cong_doc));

	g_assert(cong_doc);

	cong_primary_window_new(cong_doc);
}


void test_transform(CongPrimaryWindow *primary_window,
		    const gchar *stylesheet_filename)
{
	CongDocument *doc;

	/* Hackish test of libxslt */
	xsltStylesheetPtr xsl;
	xmlDocPtr input_clone;
	xmlDocPtr result;

#if 0
	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue(1);
#endif
	
	doc = cong_primary_window_get_document(primary_window);

	g_return_if_fail(doc);

	xsl = xsltParseStylesheetFile(stylesheet_filename);

	if (NULL==xsl) {
		gchar *why_failed = g_strdup_printf("There was a problem reading the stylesheet file \"%s\"",stylesheet_filename);

		GtkDialog* dialog = cong_error_dialog_new("Conglomerate could not transform the document",
							  why_failed,
							  "FIXME");
	
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		return;
	}

	/* DHM 14/11/2002:  document nodes seemed to being corrupted when applying the stylesheet.
	   So We now work with a clone of the document
	*/
	input_clone = xmlCopyDoc(cong_document_get_xml(doc), TRUE);
	g_assert(input_clone);

	result = xsltApplyStylesheet(xsl, input_clone, NULL);
	g_assert(result);

	xmlFreeDoc(input_clone);

	if (result->children==NULL) {
		gchar *why_failed = g_strdup_printf("There was a problem applying the stylesheet file");

		GtkDialog* dialog = cong_error_dialog_new("Conglomerate could not transform the document",
							  why_failed,
							  "FIXME");
	
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		return;
	}

#if 0
	open_preview_window_for_doc(result); /* takes ownership of the result */
#else
	open_transformed_window_for_doc(result); /* takes ownership of the result */
	/* FIXME: do as a document?  or have a special preview window? */
#endif
	
	xsltFreeStylesheet(xsl);

	/* do we need to clean up the globals? */
#if 0
	xmlSubstituteEntitiesDefault(0);
	xmlLoadExtDtdDefaultValue(0);
#endif
}

#if 1
#define DOCBOOK_TO_HTML_STYLESHEET_FILE (STYLESHEET_PATH "html/docbook.xsl")
#else
#define DOCBOOK_TO_HTML_STYLESHEET_FILE ("../examples/test-docbook-to-html.xsl")
#endif
#define DOCBOOK_TO_XHTML_STYLESHEET_FILE (STYLESHEET_PATH "xhtml/docbook.xsl")
#define DOCBOOK_TO_HTML_HELP_STYLESHEET_FILE (STYLESHEET_PATH "htmlhelp/htmlhelp.xsl")
#define DOCBOOK_TO_JAVAHELP_STYLESHEET_FILE (STYLESHEET_PATH "javahelp/javahelp.xsl")
#define DOCBOOK_TO_FO_STYLESHEET_FILE (STYLESHEET_PATH "fo/docbook.xsl")

void menu_callback_test_transform_docbook_to_html(gpointer callback_data,
				  guint callback_action,
				  GtkWidget *widget)
{
	test_transform(callback_data,
		       DOCBOOK_TO_HTML_STYLESHEET_FILE);
}
void menu_callback_test_transform_docbook_to_xhtml(gpointer callback_data,
						  guint callback_action,
						  GtkWidget *widget)
{
	test_transform(callback_data,
		       DOCBOOK_TO_XHTML_STYLESHEET_FILE);
}
void menu_callback_test_transform_docbook_to_html_help(gpointer callback_data,
						  guint callback_action,
						  GtkWidget *widget)
{
	test_transform(callback_data,
		       DOCBOOK_TO_HTML_HELP_STYLESHEET_FILE);
}
void menu_callback_test_transform_docbook_to_javahelp(gpointer callback_data,
						  guint callback_action,
						  GtkWidget *widget)
{
	test_transform(callback_data,
		       DOCBOOK_TO_JAVAHELP_STYLESHEET_FILE);
}
void menu_callback_test_transform_docbook_to_fo(gpointer callback_data,
				  guint callback_action,
				  GtkWidget *widget)
{
	test_transform(callback_data,
		       DOCBOOK_TO_FO_STYLESHEET_FILE);
}

#if PRINT_TESTS
static void
my_draw (GnomePrintContext *gpc)
{
	GnomeFont *font;

	font = gnome_font_find_closest ("Helvetica", 12);

	gnome_print_beginpage (gpc, "1");

	gnome_print_setfont (gpc, font);
	gnome_print_moveto (gpc, 100, 400);
	gnome_print_show (gpc, "Hello world\nThis is a test");

	gnome_print_moveto (gpc, 100, 200);
	gnome_print_lineto (gpc, 200, 200);
	gnome_print_stroke (gpc);

	gnome_print_showpage (gpc);
}

void menu_callback_test_preview_fo(gpointer callback_data,
				  guint callback_action,
				  GtkWidget *widget)
{
	/* Open a GnomePrint preview */
	xmlDocPtr xml_doc;
	GnomePrintMaster *gpm;
	GnomePrintContext *gpc;
	GtkWidget *preview_widget;
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc;

	gpm = gnome_print_master_new ();
	gpc = gnome_print_master_get_context (gpm);
#if 1

#if 1
	/* Grab XML from document; assume it's FO for now: */
	doc = cong_primary_window_get_document(primary_window);
	g_return_if_fail(doc);


	xml_doc = cong_document_get_xml(doc);
#else
	/* Load a hardcoded file off my hardddrive (to make it easier to test): */
	xml_doc = xmlParseFile("/home/david/coding/conge-cvs-dhm3/conge/examples/test-fo.xml");
#endif
	g_assert(xml_doc);

	/* Render the FO document to gnome_print: */
	cong_gnome_print_render_xslfo(xml_doc, gpm);
	
	xmlFreeDoc(xml_doc);
#else
	/* Just render some test stuff: */
	my_draw (gpc);
#endif

	gnome_print_master_close (gpm);

	preview_widget = gnome_print_master_preview_new (gpm, "Print Preview");
	gtk_widget_show(preview_widget);
}
#endif /* #if PRINT_TESTS */

void my_hash_scanner(void *payload, void *data, xmlChar *name)
{
	g_message("got name \"%s\"", name);
}

#if 0
gchar *get_element_content_string(xmlElementContentPtr content)
{
	g_return_val_if_fail(content, NULL);

	switch (content->type) {
	default: g_assert(0);
	case XML_ELEMENT_CONTENT_PCDATA:
	case XML_ELEMENT_CONTENT_ELEMENT:
	case XML_ELEMENT_CONTENT_SEQ:
	case XML_ELEMENT_CONTENT_OR:
		{
			gchar *lhs = c1->name;
		}


	}
}
#endif

gchar *get_enumeration_details(xmlEnumerationPtr enum_ptr)
{
	gchar *temp, *temp2;
	temp = g_strdup("(");

	while (enum_ptr) {
		temp2 = g_strdup_printf("%s %s%s", temp, enum_ptr->name, (enum_ptr->next!=NULL)?",":"");
		g_free(temp);
		temp = temp2;

		enum_ptr=enum_ptr->next;
	}

	temp2 = g_strdup_printf("%s )", temp);
	g_free(temp);
	return temp2;
}

gchar *get_attribute_debug_details(xmlAttributePtr attr)
{
	gchar *type_str = NULL;
	g_assert(attr);

	switch (attr->atype) {
	default: g_assert(0);
	case XML_ATTRIBUTE_CDATA:
		type_str = g_strdup("CDATA");
		break;

	case XML_ATTRIBUTE_ID:
		type_str = g_strdup("ID");
		break;

	case XML_ATTRIBUTE_IDREF:
		type_str = g_strdup("IDREF");
		break;

	case XML_ATTRIBUTE_IDREFS:
		type_str = g_strdup("IDREFS");
		break;

	case XML_ATTRIBUTE_ENTITY:
		type_str = g_strdup("ENTITY");
		break;

	case XML_ATTRIBUTE_ENTITIES:
		type_str = g_strdup("ENTITIES");
		break;

	case XML_ATTRIBUTE_NMTOKEN:
		type_str = g_strdup("NMTOKEN");
		break;

	case XML_ATTRIBUTE_NMTOKENS:
		type_str = g_strdup("NMTOKENS");
		break;

	case XML_ATTRIBUTE_ENUMERATION:
		type_str = get_enumeration_details(attr->tree);
		break;

	case XML_ATTRIBUTE_NOTATION:
		type_str = g_strdup("NOTATION");
		break;
	}

#if 0
	switch (attr->def) {
	}
#endif

	return type_str;

#if 0
    xmlAttributeType       atype;	/* The attribute type */
    xmlAttributeDefault      def;	/* the default */
    const xmlChar  *defaultValue;	/* or the default value */
    xmlEnumerationPtr       tree;       /* or the enumeration tree if any */
#endif

}

void element_hash_scanner(void *payload, void *data, xmlChar *name)
{
	xmlElementPtr element = payload;
	xmlAttributePtr attr;

	g_message("got element <%s>", name);
/*  	g_message("content = "); */

	/* List the attributes that apply to this element: */
	for (attr=element->attributes; attr; attr=attr->nexth) {
		gchar *details = get_attribute_debug_details(attr);
		g_message("attribute \"%s\": %s",attr->name, details);
		g_free(details);
	}
}

void entity_hash_scanner(void *payload, void *data, xmlChar *name)
{
	g_message("got entity \"%s\"", name);
}

void test_log_dtd(xmlDtdPtr dtd)
{
	g_message("Name \"%s\"\n", dtd->name);
	g_message("ExternalID:\"%s\"\n", dtd->ExternalID);
	g_message("SystemID:\"%s\"\n", dtd->SystemID);

#if 0
	g_message("notations\n");
	xmlHashScan(dtd->notations,
		    my_hash_scanner,
		    NULL);
#endif

	g_message("\nelements\n");
	xmlHashScan(dtd->elements,
		    element_hash_scanner,
		    NULL);

#if 0
	g_message("\nattributes\n");
	xmlHashScan(dtd->attributes,
		    my_hash_scanner,
		    NULL);
#endif

#if 0
	g_message("\nentities\n");
	xmlHashScan(dtd->entities,
		    entity_hash_scanner,
		    NULL);
#endif

#if 0
	g_message("\npentities\n");
	xmlHashScan(dtd->pentities,
		    my_hash_scanner,
		    NULL);
#endif
}

void menu_callback_test_dtd(gpointer callback_data,
			    guint callback_action,
			    GtkWidget *widget)
{
	CongPrimaryWindow *primary_window = callback_data;
	CongDocument *doc;
	xmlDocPtr xml_doc;


	doc = cong_primary_window_get_document(primary_window);
	g_return_if_fail(doc);

	xml_doc = cong_document_get_xml(doc);
	g_assert(xml_doc);
 
	if (xml_doc->intSubset) {
		g_message("Interior subset:\n");
		test_log_dtd(xml_doc->intSubset);
	} else {
		g_message("No interior subset\n");
	}
	
	if (xml_doc->extSubset) {
		g_message("Exterior subset:\n");
		test_log_dtd(xml_doc->extSubset);
	} else {
		g_message("No exterior subset\n");
	}
}

GtkWidget *test_dialog_new(void)
{
	GtkWidget *dialog;
	GtkWidget *tabs;
	CongDialogContent *basic_content;
	CongDialogCategory *general_category;
	CongDialogCategory *fubar_category;
	CongDialogContent *advanced_content;
	CongDialogCategory *morestuff_category;
	CongDialogCategory *yetmorestuff_category;

	dialog = gtk_dialog_new_with_buttons("Test Dialog",
					     NULL,
					     0,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_ACCEPT,
					     NULL);

	gtk_container_set_border_width(GTK_CONTAINER(dialog), 12);

	tabs = gtk_notebook_new();

	basic_content = cong_dialog_content_new(TRUE);
	general_category = cong_dialog_content_add_category(basic_content, "General");
	fubar_category = cong_dialog_content_add_category(basic_content, "Fubar");

	advanced_content = cong_dialog_content_new(TRUE);
	morestuff_category = cong_dialog_content_add_category(advanced_content, "More Stuff");
	yetmorestuff_category = cong_dialog_content_add_category(advanced_content, "Yet More Stuff");

	cong_dialog_category_add_field(general_category, "Foo", gtk_entry_new());
	cong_dialog_category_add_field(general_category, "Foo", gtk_entry_new());
	cong_dialog_category_add_field(general_category, "Foo", gtk_entry_new());
	cong_dialog_category_add_selflabelled_field(general_category, gtk_toggle_button_new_with_label("Bar") );
	cong_dialog_category_add_selflabelled_field(general_category, gtk_check_button_new_with_label("Crikey") );

	cong_dialog_category_add_field(fubar_category, "Foo", gtk_entry_new());
	cong_dialog_category_add_field(fubar_category, "Foo", gtk_entry_new());
	cong_dialog_category_add_field(fubar_category, "Foo", gtk_entry_new());
	cong_dialog_category_add_selflabelled_field(fubar_category, gtk_toggle_button_new_with_label("Bar") );
	cong_dialog_category_add_selflabelled_field(fubar_category, gtk_check_button_new_with_label("Crikey") );

	cong_dialog_category_add_field(morestuff_category, "Foo", gtk_entry_new());
	cong_dialog_category_add_field(morestuff_category, "Foo", gtk_entry_new());
	cong_dialog_category_add_field(morestuff_category, "Foo", gtk_entry_new());
	cong_dialog_category_add_selflabelled_field(morestuff_category, gtk_toggle_button_new_with_label("Bar") );
	cong_dialog_category_add_selflabelled_field(morestuff_category, gtk_check_button_new_with_label("Crikey") );

	cong_dialog_category_add_field(yetmorestuff_category, "Foo", gtk_entry_new());
	cong_dialog_category_add_field(yetmorestuff_category, "Foo", gtk_entry_new());
	cong_dialog_category_add_field(yetmorestuff_category, "Foo", gtk_entry_new());
	cong_dialog_category_add_selflabelled_field(yetmorestuff_category, gtk_toggle_button_new_with_label("Bar") );
	cong_dialog_category_add_selflabelled_field(yetmorestuff_category, gtk_check_button_new_with_label("Crikey") );
	
	gtk_notebook_append_page( GTK_NOTEBOOK(tabs),
				  cong_dialog_content_get_widget(basic_content),
				  gtk_label_new("Basic"));

	gtk_notebook_append_page( GTK_NOTEBOOK(tabs),
				  cong_dialog_content_get_widget(advanced_content),
				  gtk_label_new("Advanced"));

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   tabs);

	gtk_widget_show_all(dialog);

	return dialog;
}

void menu_callback_test_dialog(gpointer callback_data,
					guint callback_action,
					GtkWidget *widget)
{
	GtkWidget *dialog = test_dialog_new();

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


void insert_element_init()
{
	GdkColor gcol;

	the_globals.insert_element_gc = gdk_gc_new(cong_gui_get_a_window()->window);
	gdk_gc_copy(the_globals.insert_element_gc, cong_gui_get_a_window()->style->white_gc);
	col_to_gcol(&gcol, 0x00e0e0e0);
	gdk_colormap_alloc_color(cong_gui_get_a_window()->style->colormap, &gcol, 0, 1);
	gdk_gc_set_foreground(the_globals.insert_element_gc, &gcol);
}







void fonts_load()
{
	  the_globals.fonts[CONG_FONT_ROLE_BODY_TEXT] = cong_font_load("sans 10");
	  the_globals.fonts[CONG_FONT_ROLE_TITLE_TEXT] = cong_font_load("sans 12");
	  the_globals.fonts[CONG_FONT_ROLE_SPAN_TAG] = cong_font_load("sans 6");
}




void status_update()
{
  while (g_main_iteration(FALSE));
}


static gint popup_deactivate(GtkWidget *widget, GdkEvent *event)
{
	return(TRUE);
}

gboolean main_load_displayspecs(void)
{
#if 0
	gchar*      xds_directory = gnome_program_locate_file(the_gui.gnome_program,
							      GNOME_FILE_DOMAIN_APP_DATADIR,
							      "dispspec",
							      FALSE,
							      NULL);
#else
	gchar* current_dir = g_get_current_dir();
	gchar* xds_directory = g_strdup_printf("%s/../examples",current_dir);
	g_free(current_dir);
#endif

	if (xds_directory) {
		g_message("Loading xds files from \"%s\"\n", xds_directory);
		the_globals.ds_registry = cong_dispspec_registry_new(xds_directory);
		
		g_free(xds_directory);
		
		if (the_globals.ds_registry==NULL) {
			return FALSE;
		}
		
		cong_dispspec_registry_dump(the_globals.ds_registry);
	} else {
		GtkDialog* dialog = cong_error_dialog_new("Conglomerate could not find its registry of document types.",
							  "You must run the program from the \"src\" directory used to build it.",
							  "This is a known problem and will be fixed.");
		cong_error_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		return FALSE;
	}

	return TRUE;
}

void register_plugin(CongPluginCallbackRegister register_callback,
		     CongPluginCallbackConfigure configure_callback)
{
	g_return_if_fail(register_callback);

	g_assert(the_globals.plugin_manager);

	cong_plugin_manager_register(the_globals.plugin_manager,
				     register_callback, 
				     configure_callback);
}



void main_load_plugins(void)
{
	/* For the moment, there aren't any actual plugins; instead we fake it. */

	register_plugin(plugin_docbook_plugin_register,
			plugin_docbook_plugin_configure);
}

int main( int   argc,
	  char *argv[] )
{
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);


#if 0
	/* THIS SHOULD NEVER BE ENABLED IN CVS: */

	setenv("XML_CATALOG_FILES", "file:///home/david/garnome/etc/xml/catalog", TRUE);
#endif


#if 1
	the_globals.gnome_program = gnome_program_init("Conglomerate",
						       "0.1.0",
						       LIBGNOMEUI_MODULE,
						       argc,argv,
						       GNOME_PARAM_HUMAN_READABLE_NAME,
						       _("XML Editor"),
						       GNOME_PARAM_APP_DATADIR, DATADIR,
						       NULL);
#else
	gtk_init(&argc, &argv);
#endif


	fonts_load();
	popup_init(NULL); /* FIXME */

#if 0
	the_globals.pango_context = pango_context_new();

	pango_context_set_font_map( gdk_pango_context_get(), /*  the_globals.pango_context, */
				    pango_ft2_font_map_for_display() );
#endif


#if 0
	the_globals.pango_font_description = pango_font_description_new();

	pango_font_description_set_family(the_globals.pango_font_description,
					  "sans");
	pango_font_description_set_size(the_globals.pango_font_description,
			       PANGO_SCALE*12);

	pango_context_set_font_description(the_globals.pango_context,
					   the_globals.pango_font_description);


	the_globals.pango_font = pango_context_load_font(the_globals.pango_context,
							 the_globals.pango_font_description);

	g_assert(the_globals.pango_font);
#endif
	
	cong_primary_window_new(NULL);


	/* Load all the displayspec (xds) files: */
	if (!main_load_displayspecs()) {
		return 1;
	}

	/* 
	   Load all the plugins.  We do this after loading the xds files in case some of the plugins want to operate on the registry
	   of displayspecs
	 */
	the_globals.plugin_manager = cong_plugin_manager_new();
	main_load_plugins();

	insert_element_init();

	the_globals.clipboard = NULL;

	/* --- */
#if 0	
	if (argc > 2) open_document_do(argv[1], argv[2]);
#endif
	
	gtk_main();

	return(0);
}
