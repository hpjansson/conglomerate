/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#include "global.h"
#include "cong-document.h"
#include "cong-error-dialog.h"
#include "cong-plugin.h"
#include "cong-app.h"
#include "cong-primary-window.h"
#include "cong-edit-find-and-replace.h"

#include <glade/glade.h>

#if 0
struct CongFindDialogDetails
{
	GladeXML *xml;
	CongDocument *doc;
};

static gboolean
on_dialog_destroy (GtkWidget *widget,
		   gpointer user_data);
#endif


/**
 * cong_document_find:
 *
 * @doc: the #CongDocument for which the find dialog is to be run
 *
 * Opens the Find dialog for this #CongDocument
 *
 */
void
cong_document_find (CongDocument *doc)
{
	GladeXML *xml;
	GtkDialog *dialog;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	xml = cong_util_load_glade_file ("glade/cong-find-replace.glade", 
					 "find_dialog",
					 doc,
					 NULL);
	dialog = GTK_DIALOG (glade_xml_get_widget(xml, "find_dialog"));

}

void
cong_document_find_next (CongDocument *doc)
{
	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	CONG_DO_UNIMPLEMENTED_DIALOG (NULL, "Find Next");
}

void
cong_document_find_prev (CongDocument *doc)
{
	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	CONG_DO_UNIMPLEMENTED_DIALOG (NULL, "Find Previous");
}

void
cong_document_replace (CongDocument *doc)
{
	GladeXML *xml;
	GtkDialog *dialog;

	g_return_if_fail (IS_CONG_DOCUMENT (doc));

	xml = cong_util_load_glade_file ("glade/cong-find-replace.glade", 
					 "replace_dialog",
					 doc,
					 NULL);
	dialog = GTK_DIALOG (glade_xml_get_widget(xml, "replace_dialog"));

}
