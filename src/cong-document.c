/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#include <gtk/gtk.h>
#include "global.h"
#include "xml.h"

struct _cong_document
{
  char dummy[128];
  TTREE *tt;
};

cong_document*
cong_document_new_from_ttree(TTREE *tt)
{
  cong_document *doc;

  g_return_val_if_fail(tt!=NULL, NULL);

  doc = g_new(cong_document,1);

  doc->tt=tt;

  return doc;
}

void
cong_document_delete(cong_document *doc)
{
  g_return_if_fail(doc);

  g_assert(doc->tt);

  ttree_branch_remove(doc->tt);

  g_free(doc);
}

TTREE*
cong_document_get_root(cong_document *doc)
{
  g_return_val_if_fail(doc, NULL);

  return doc->tt;
}


void
cong_document_save(cong_document *doc, const char* filename)
{
  FILE *xml_f;

  g_return_if_fail(doc);
  g_return_if_fail(filename);

  xml_f = fopen(filename, "wt");
  if (!xml_f) return;

  xml_t_to_f(cong_document_get_root(doc), xml_f);
  fclose(xml_f);
  
}

