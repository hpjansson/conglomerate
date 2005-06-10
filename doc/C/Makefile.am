# temporary hack until yelp support qanda sets

BUILT_SOURCES = faq.xml

faq.xml: faq.xml.qna
	xsltproc --output $@ faq-hack.xsl $<

EXTRA_DIST = faq.xml.qna

# end of temporary hack until yelp support qanda sets

more_C_docs = \
  conglomerate-refentry.xml manpage.make conglomerate.1 \
  faq.xml

figdir = figures
docname = conglomerate
lang = C
omffile = conglomerate-C.omf
omfincfile = faq.xml
entities = global.ent cong_element_map_ref.xml conventions.xml \
	doc_type_support.xml glossary.xml notice_fdl.xml notice_gpl.xml \
	row_xml_lang_attrib.xml tbl_element_attrib.xml xds_dtd_def.xml \
	faq.xml

include $(top_srcdir)/xmldocs.make

dist-hook: app-dist-hook

