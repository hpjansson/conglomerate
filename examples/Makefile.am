exampledir 	 = $(datadir)/conglomerate/examples
example_DATA 	 = \
	cnxml.xml \
	error-proposal.xml \
	file-roller.xml \
	i18n.xml \
	readme.xml \
	test-docbook.xml \
	test-entities.xml \
	test-table.xml

dispspecdir 	 = $(pkgdatadir)/dispspecs
dispspec_in_files = \
	docbook.xds.in \
	dispspec.xds.in \
	forrest-document.xds.in \
	kernel-traffic-newsletter.xds.in \
	xhtml1-strict.xds.in

dispspec_DATA = \
	cnxml.xds \
	dispspec-template.xds \
	readme.xds \
	relax-ng.xds \
	teixlite.xds \
	website-layout.xds \
	website-webpage.xds \
	xsl.xds \
	$(dispspec_in_files:.xds.in=.xds)

#INTLTOOL_XML_RULE assumes that the file extension is xml, not xds:
%.xds:       %.xds.in       $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -x -c $(top_builddir)/po/.intltool-merge-cache

#@INTLTOOL_XML_RULE@


# bugzilla #122011
dtddir 	 = $(datadir)/conglomerate/dtd
dtd_DATA	= xds.dtd

EXTRA_DIST = $(example_DATA)	\
	$(dispspec_DATA)	\
	$(dispspec_in_files)	\
	$(dtd_DATA)

