/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"
#include "xml.h"

struct _CongDocument
{
	char dummy[128];
	TTREE *tt;
	CongDispspec *ds;
};

CongDocument*
cong_document_new_from_ttree(TTREE *tt, CongDispspec *ds)
{
	CongDocument *doc;

	g_return_val_if_fail(tt!=NULL, NULL);

	doc = g_new(struct _CongDocument,1);

	doc->tt=tt;
	doc->ds=ds;

	return doc;
}

void
cong_document_delete(CongDocument *doc)
{
	g_return_if_fail(doc);

	g_assert(doc->tt);

	ttree_branch_remove(doc->tt);
	
	g_free(doc);
}

CongNodePtr
cong_document_get_root(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return doc->tt;
}

CongDispspec*
cong_document_get_dispspec(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

	return doc->ds;
}

void
cong_document_save(CongDocument *doc, const char* filename)
{
	FILE *xml_f;

	g_return_if_fail(doc);
	g_return_if_fail(filename);

	xml_f = fopen(filename, "wt");
	if (!xml_f) return;

	xml_t_to_f(cong_document_get_root(doc), xml_f);
	fclose(xml_f);
}

