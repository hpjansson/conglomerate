## the display specs files
NULL =

dispspecdir = $(pkgdatadir)/dispspecs
dispspec_in_files = \
	docbook.xds.in \
	dispspec.xds.in \
	forrest-document.xds.in \
	kernel-traffic-newsletter.xds.in \
	xhtml1-strict.xds.in \
	$(NULL)

dispspec_DATA = \
	cnxml.xds \
	opml.xds \
	readme.xds \
	relax-ng.xds \
	release.xds \
	teixlite.xds \
	website-layout.xds \
	website-webpage.xds \
	xsl.xds \
	$(dispspec_in_files:.xds.in=.xds)

#INTLTOOL_XML_RULE assumes that the file extension is xml, not xds:
%.xds:       %.xds.in       $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -x -c $(top_builddir)/po/.intltool-merge-cache

#@INTLTOOL_XML_RULE@

# a strange file in this directory
release_dtd = release.dtd

# bugzilla #122011
dtddir = $(pkgdatadir)/dtd
dtd_DATA = \
	xds.dtd \
	$(release_dtd) \
	$(NULL)

EXTRA_DIST = \
	$(dispspec_DATA) \
	$(dispspec_in_files) \
	$(dtd_DATA) \
	$(NULL)

## end of this Makefile.am
