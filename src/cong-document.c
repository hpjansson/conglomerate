/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"
#include "xml.h"

struct _CongDocument
{
	char dummy[128];

#if NEW_XML_IMPLEMENTATION
	xmlDocPtr xml_doc;
#else
	TTREE *tt;
#endif  /* #if NEW_XML_IMPLEMENTATION */

	CongDispspec *ds;
};

#if NEW_XML_IMPLEMENTATION
CongDocument*
cong_document_new_from_xmldoc(xmlDocPtr xml_doc, CongDispspec *ds)
{
	CongDocument *doc;

	g_return_val_if_fail(xml_doc!=NULL, NULL);

	doc = g_new(struct _CongDocument,1);

	doc->xml_doc=xml_doc;
	doc->ds=ds;

	return doc;
}
#else
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
#endif  /* #if NEW_XML_IMPLEMENTATION */

void
cong_document_delete(CongDocument *doc)
{
	g_return_if_fail(doc);

#if NEW_XML_IMPLEMENTATION
	g_assert(0);
#else
	g_assert(doc->tt);

	ttree_branch_remove(doc->tt);
#endif  /* #if NEW_XML_IMPLEMENTATION */
	
	g_free(doc);
}

CongNodePtr
cong_document_get_root(CongDocument *doc)
{
	g_return_val_if_fail(doc, NULL);

#if NEW_XML_IMPLEMENTATION
	return doc->xml_doc->children;
#else
	return doc->tt->child;
#endif /* #if NEW_XML_IMPLEMENTATION */

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

#if NEW_XML_IMPLEMENTATION
	g_assert(0);
#else
	xml_f = fopen(filename, "wt");
	if (!xml_f) return;

	xml_t_to_f(cong_document_get_root(doc), xml_f);
	fclose(xml_f);
#endif  /* #if NEW_XML_IMPLEMENTATION */
}

