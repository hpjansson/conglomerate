## the display specs files
NULL =

dispspecdir = $(pkgdatadir)/dispspecs
dispspec_in_files = \
	cnxml.xds.in \
	docbook.xds.in \
	dispspec.xds.in \
	forrest-document.xds.in \
	kernel-traffic-newsletter.xds.in \
	opml.xds.in \
	readme.xds.in \
	relax-ng.xds.in \
	teixlite.xds.in \
	website-layout.xds.in \
	website-webpage.xds.in \
	xhtml1-strict.xds.in \
	xsl.xds.in \
	$(NULL)

dispspec_DATA = \
	release.xds \
	$(dispspec_in_files:.xds.in=.xds)

#INTLTOOL_XML_RULE assumes that the file extension is xml, not xds:
%.xds:       %.xds.in       $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -x -u -c $(top_builddir)/po/.intltool-merge-cache

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

DISTCLEANFILES = \
	$(dispspec_DATA) \
	$(dtd_DATA) \
	$(NULL)

## end of this Makefile.am

-include $(top_srcdir)/git.mk
