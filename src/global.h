/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxml/tree.h>
#include <gtk/gtk.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gconf/gconf-client.h>

G_BEGIN_DECLS

#define CONG_GCONF_PATH "/apps/conglomerate/"

#define RELEASE 1
#undef WINDOWS_BUILD

#ifdef WINDOWS_BUILD
# define isblank(c) ((c) == ' ' || (c) == '\n' || (c) == '\r' || (c) == '\t')
#endif

#define NEW_LOOK 1

#if ENABLE_PRINTING
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#endif

#if 1
#define CONG_VALIDATE_UTF8(str) (g_assert(g_utf8_validate((str), -1, NULL)))
#else
#define CONG_VALIDATE_UTF8(str) ((void)0)
#endif

#include "cong-node.h"
#include "cong-location.h"

struct CongCursor
{
	/* Visual representation */
	GdkGC *gc;

	gboolean on;

	/* Conceptual location */
	CongLocation location;

	guint timeout_id;

	CongDocument *doc;
};


struct CongSelection
{
	GdkGC *gc_valid; /* corresponds to value gc_0 in old implementation */
	GdkGC *gc_invalid;   /* corresponds to value gc_3 in old implementation */

	CongLocation loc0;
	CongLocation loc1;
};

enum CongFontRole
{
	CONG_FONT_ROLE_BODY_TEXT,
	CONG_FONT_ROLE_SPAN_TAG,
	CONG_FONT_ROLE_TITLE_TEXT,

	/* replaces: f, fm, ft in order */

	CONG_FONT_ROLE_NUM
};

CongEditorView *cong_editor_view_new(CongDocument *doc);
void cong_editor_view_free(CongEditorView *editor_view);
GtkWidget* cong_editor_view_get_widget(CongEditorView *editor_view);

/* Various view subclasses: */
GtkWidget *cong_dom_view_new(CongDocument *doc);
GtkWidget *cong_source_view_new(CongDocument *doc);
GtkWidget *cong_debug_message_log_view_new(CongDocument *doc);
GtkWidget *cong_debug_signal_log_view_new(CongDocument *doc);
GtkWidget *cong_node_properties_dialog_new(CongDocument *doc, 
					   CongNodePtr node, 
					   GtkWindow *parent_window);

GtkWidget* cong_gui_get_a_window(void);

gint cong_cursor_blink();

void new_document(GtkWindow *parent_window);

gint tree_new_sibling(GtkWidget *widget, CongNodePtr tag);
gint tree_new_sub_element(GtkWidget *widget, CongNodePtr tag);
gint tree_properties(GtkWidget *widget, CongNodePtr tag);
gint tree_cut(GtkWidget *widget, CongNodePtr tag);
gint tree_copy(GtkWidget *widget, CongNodePtr tag);
gint tree_paste_under(GtkWidget *widget, CongNodePtr tag);
gint tree_paste_before(GtkWidget *widget, CongNodePtr tag);
gint tree_paste_after(GtkWidget *widget, CongNodePtr tag);

const char *xml_frag_data_nice(CongNodePtr x);
const char *xml_frag_name_nice(CongNodePtr x);

CongNodePtr xml_frag_data_nice_split3(CongDocument *doc, CongNodePtr s, int c0, int c1);
CongNodePtr xml_frag_data_nice_split2(CongDocument *doc, CongNodePtr s, int c);

CongNodePtr cong_selection_reparent_all(CongSelection *selection, CongDocument *doc, CongNodePtr p);
void cong_selection_delete(CongSelection *selection, CongDocument *doc);

GList *xml_all_present_span_elements(CongDispspec *ds, CongNodePtr node);
GList *xml_all_valid_span_elements(CongDispspec *ds, CongNodePtr node);
char *xml_fetch_clean_data(CongNodePtr x);
gboolean xml_add_required_children(CongDocument *cong_doc, CongNodePtr node);
xmlElementPtr xml_get_dtd_element(CongDocument *cong_doc, CongNodePtr node);
GList* xml_get_valid_children(CongDispspec* ds, CongNodePtr node, enum CongElementType tag_type);
GList* xml_get_valid_previous_sibling(CongDispspec* ds, CongNodePtr node, enum CongElementType tag_type);
GList* xml_get_valid_next_sibling(CongDispspec* ds, CongNodePtr node, enum CongElementType tag_type);

char *tag_new_pick();

void open_document(GtkWindow *parent_window);
gint save_document(CongDocument *doc, GtkWindow *parent_window);
gint save_document_as(CongDocument *doc, GtkWindow *parent_window);

gchar *cong_get_file_name(const gchar *title, 
			  const gchar *filename,
			  GtkWindow *parent_window);

char *pick_structural_tag(CongDispspec *ds);

void open_document_do(const gchar *doc_name, GtkWindow *parent_window);

#define UNUSED_VAR(x)

int gui_window_new_document_make();

void col_to_gcol(GdkColor *gcol, unsigned int col);

/* Cursor methods: */
void cong_cursor_init(CongCursor *curs, CongDocument *doc);
void cong_cursor_uninit(CongCursor *curs);
void cong_cursor_on(CongCursor *curs);
void cong_cursor_off(CongCursor *curs);
gint cong_cursor_data_insert(CongCursor *curs, char *s);
int cong_cursor_paragraph_insert(CongCursor *curs);
gboolean cong_cursor_calc_prev_char(CongCursor *curs, CongDocument *doc, CongLocation *output_loc);
gboolean cong_cursor_calc_next_char(CongCursor *curs, CongDocument *doc, CongLocation *output_loc);
void cong_cursor_next_line(CongCursor *curs, CongSpanEditor *xed);
void cong_cursor_del_prev_char(CongCursor *curs, CongDocument *doc);
void cong_cursor_del_next_char(CongCursor *curs, CongDocument *doc);
void cong_cursor_home(CongCursor *curs, CongDocument *doc);
void cong_cursor_end(CongCursor *curs, CongDocument *doc);

/* Selection methods: */
void cong_selection_init(CongSelection *selection);
void cong_selection_import(CongSelection *selection, GtkWidget* widget);
void cong_selection_draw(CongSelection *selection, CongCursor *curs);
void cong_selection_start_from_curs(CongSelection *selection, CongCursor *curs);
void cong_selection_end_from_curs(CongSelection *selection, CongCursor *curs);

/* Popup (context) menus for editor view: */
void editor_popup_show(GtkWidget *widget, GdkEventButton *bevent);
void editor_popup_build(CongDocument *doc, GtkWindow *parent_window);
void editor_popup_init();

/* Popup (context) menus for tree view and for section headings: */
GtkWidget* cong_ui_popup_init(CongDocument *doc, CongNodePtr node, GtkWindow *parent_window);

/* dialog to select from a list of string */
gchar* string_selection_dialog(gchar *title, gchar *element_description, GList *elements);

void xv_style_r(GtkWidget *widget, gpointer data);

CongDispspec* query_for_forced_dispspec(gchar *what_failed, xmlDocPtr doc, GtkWindow* parent_window);

/**
 * A routine that tries to syncronously load a file into a buffer in memory (surely this exists already somewhere?)
 * (I believe that CVS gnome-vfs has a routine gnome_vfs_read_entire_file that does this)
*/
GnomeVFSResult
cong_vfs_new_buffer_from_file(const char* filename, char** buffer, GnomeVFSFileSize* size);

/**
 * A routine that tries to syncronously load a file into a buffer in memory (surely this exists already somewhere?)
*/
GnomeVFSResult
cong_vfs_new_buffer_from_uri(GnomeVFSURI* uri, char** buffer, GnomeVFSFileSize* size);

/* Toolbar hooks: */
gint toolbar_callback_open(GtkWidget *widget, gpointer data);
gint toolbar_callback_save_as(GtkWidget *w, gpointer data);
gint toolbar_callback_cut(GtkWidget *w, gpointer data);
gint toolbar_callback_copy(GtkWidget *w, gpointer data);
gint toolbar_callback_paste(GtkWidget *w, gpointer data);

/* Menu hooks: */
void menu_callback_debug_error(gpointer callback_data,
			       guint callback_action,
			       GtkWidget *widget);
void menu_callback_debug_document_types(gpointer callback_data,
					guint callback_action,
					GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_html(gpointer callback_data,
						   guint callback_action,
						   GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_xhtml(gpointer callback_data,
						    guint callback_action,
						    GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_html_help(gpointer callback_data,
							guint callback_action,
							GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_javahelp(gpointer callback_data,
						       guint callback_action,
						       GtkWidget *widget);
void menu_callback_debug_transform_docbook_to_fo(gpointer callback_data,
						 guint callback_action,
						 GtkWidget *widget);
void menu_callback_debug_preview_fo(gpointer callback_data,
				    guint callback_action,
				    GtkWidget *widget);

#if PRINT_TESTS
void cong_gnome_print_render_xslfo(xmlDocPtr xml_doc, GnomePrintMaster *gpm);
#endif
void menu_callback_debug_dtd(gpointer callback_data,
			    guint callback_action,
			    GtkWidget *widget);

void menu_callback_debug_dialog(gpointer callback_data,
				guint callback_action,
				GtkWidget *widget);

/* Experimental new implementation of the editor as a custom widget; to be a fully MVC view from the beginning; currently it's a GtkDrawingArea */
typedef GtkDrawingArea CongEditorWidget2;
typedef struct CongElementEditor CongElementEditor;

GtkWidget *cong_editor_widget2_new(CongDocument *doc);
CongDocument *cong_editor_widget2_get_document(CongEditorWidget2 *editor_widget);
CongDispspec *cong_editor_widget2_get_dispspec(CongEditorWidget2 *editor_widget);
void cong_editor_widget2_force_layout_update(CongEditorWidget2 *editor_widget);
#define CONG_EDITOR_WIDGET2(x) ((CongEditorWidget2*)(x))

/* Third attempt at the editor widget: */
typedef struct CongEditorArea CongEditorArea;
typedef struct CongEditorNode CongEditorNode;


/* PLUGIN INTERFACE: 
   These types are fully opaque, to try to minimise ABI issues.
*/
typedef struct CongPlugin CongPlugin;
typedef struct CongPluginManager CongPluginManager;

typedef struct CongFunctionality CongFunctionality;
#define CONG_FUNCTIONALITY(x) ((CongFunctionality*)(x))

/* The following are all castable to CongFunctionality: */
typedef struct CongDocumentFactory CongDocumentFactory;
typedef struct CongImporter CongImporter;
typedef struct CongExporter CongExporter;
#if ENABLE_PRINTING
typedef struct CongPrintMethod CongPrintMethod;
#endif
typedef struct CongThumbnailer CongThumbnailer;
typedef struct CongPluginEditorElement CongPluginEditorElement;
typedef struct CongTool CongTool;
typedef struct CongCustomPropertyDialog CongCustomPropertyDialog;

/* The File->New GUI: */
typedef struct CongNewFileAssistant CongNewFileAssistant;


/* Function pointers to be exposed by .so/.dll files: */
typedef gboolean (*CongPluginCallbackInit)(CongPlugin *plugin); /* exposed as "plugin_init"? */
typedef gboolean (*CongPluginCallbackUninit)(CongPlugin *plugin); /* exposed as "plugin_uninit"? */
typedef gboolean (*CongPluginCallbackRegister)(CongPlugin *plugin); /* exposed as "plugin_register"? */
typedef gboolean (*CongPluginCallbackConfigure)(CongPlugin *plugin);  /* exposed as "plugin_configure"? legitimate for it not to be present */

/* Function pointers that are registered by plugins: */
typedef void (*CongDocumentFactoryPageCreationCallback)(CongDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data);
typedef void (*CongDocumentFactoryActionCallback)(CongDocumentFactory *factory, CongNewFileAssistant *assistant, gpointer user_data);
typedef gboolean (*CongImporterMimeFilter)(CongImporter *importer, const gchar *mime_type, gpointer user_data);
typedef void (*CongImporterActionCallback)(CongImporter *importer, const gchar *uri, const gchar *mime_type, gpointer user_data, GtkWindow *toplevel_window);
typedef gboolean (*CongExporterDocumentFilter)(CongExporter *exporter, CongDocument *doc, gpointer user_data);
typedef void (*CongExporterActionCallback)(CongExporter *exporter, CongDocument *doc, const gchar *uri, gpointer user_data, GtkWindow *toplevel_window);
typedef gboolean (*CongToolDocumentFilter)(CongTool *tool, CongDocument *doc, gpointer user_data);
typedef void (*CongToolActionCallback)(CongTool *tool, CongPrimaryWindow *primary_window, gpointer user_data);
typedef GtkWidget* (*CongCustomPropertyFactoryMethod)(CongCustomPropertyDialog *custom_property_dialog, CongDocument *doc, CongNodePtr node);

#if ENABLE_PRINTING
typedef gboolean (*CongPrintMethodDocumentFilter)(CongPrintMethod *print_method, CongDocument *doc, gpointer user_data);
typedef void (*CongPrintMethodActionCallback)(CongPrintMethod *print_method, CongDocument *doc, GnomePrintContext *gpc, gpointer user_data, GtkWindow *toplevel_window);
#endif

typedef CongElementEditor* (*CongEditorElementFactoryMethod)(CongPluginEditorElement *plugin_editor_element, CongEditorWidget2 *editor_widget, CongNodePtr node, gpointer user_data);

void cong_menus_create_items(GtkItemFactory *item_factory, 
			     CongPrimaryWindow *primary_window);

/* UI routines for invocation by menus/toolbars: */
void
cong_ui_file_import(GtkWindow *toplevel_window);

void
cong_ui_file_export(CongDocument *doc,
		    GtkWindow *toplevel_window);

GnomeVFSResult
cong_xml_save_to_vfs(xmlDocPtr doc_ptr, 
		     GnomeVFSURI *file_uri,	
		     GnomeVFSFileSize *output_file_size);

/* Extensions to libxml: */
xmlAttrPtr	xmlNewProp_NUMBER	(xmlNodePtr node,
					 const xmlChar *name,
					 int value);

/* Handy utility functions: */

/**
   Handy function for taking xml text and turning it into something you can see in a log: tabs and carriage returns etc are turned into escape sequences.
*/
gchar* cong_util_cleanup_text(const xmlChar *text);


/**
   Convert a URI into a POSIX, path, assuming that this is valid: 
*/
gchar *cong_util_get_local_path_from_uri(GnomeVFSURI *uri);

/**
   Dave Malcolm:
   Norman Walsh's stylesheets for DocBook seem to be present on every modern Linux
   distribution I've tried.  Unfortunately, there doesn't yet seem to
   be a standard about where they should be installed.

   The functions are currently hardcoded to assume Red Hat 8, and can be hacked to support SuSE 7.1

   If you have another Linux distro, or one of the BSDs, or Solaris etc, then please let the conglomerate-devel mailing list
   know where the standard location is (hopefully we can agree on a standard for this!)

   These functions hide this problem:
*/
const gchar*
cong_utils_get_norman_walsh_stylesheet_path(void);

gchar*
cong_utils_get_norman_walsh_stylesheet(const gchar *stylesheet_relative_path);

/**
   Icon loading; take an icon basename e.g. "cong-docbook-set", convert to a filename and load it.
 */
GdkPixbuf *cong_util_load_icon(const gchar *icon_basename);

void cong_util_append(gchar **string, const gchar *to_add);

#if ENABLE_PRINTING
void cong_util_print_xslfo (GtkWindow *toplevel_window, 
			    GnomePrintContext *gpc, 
			    xmlDocPtr xml_doc);
#endif

/* macro adapted from libxml's error.c; surely this exists in GLib somewhere? */
#define CONG_GET_VAR_STR(msg, str) {				\
    int       size;						\
    int       chars;						\
    char      *larger;						\
    va_list   ap;						\
								\
    str = (gchar *)g_malloc(150);				\
								\
    size = 150;							\
								\
    while (1) {							\
	va_start(ap, msg);					\
  	chars = vsnprintf(str, size, msg, ap);			\
	va_end(ap);						\
	if ((chars > -1) && (chars < size))			\
	    break;						\
	if (chars > -1)						\
	    size += chars + 1;					\
	else							\
	    size += 100;					\
	if ((larger = (char *) g_realloc(str, size)) == NULL) { \
	    g_free(str);					\
	    return;						\
	}							\
	str = larger;						\
    }								\
}

G_END_DECLS
