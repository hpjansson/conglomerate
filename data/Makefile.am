desktopdir 	 = $(datadir)/applications
desktop_in_files = conglomerate.desktop.in
desktop_DATA 	 = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

keysdir 	 = $(datadir)/mime-info
keys_in_files 	 = conglomerate.keys.in
keys_DATA 	 = $(keys_in_files:.keys.in=.keys) 
@INTLTOOL_KEYS_RULE@

app_DATA = conglomerate.applications
appdir = $(datadir)/application-registry

mimedir	= $(datadir)/mime-info
mime_DATA = conglomerate.mime

## bugzilla 122011 for catalog & catalog.xml



schemadir        = $(sysconfdir)/gconf/schemas/
schema_in_files  = conglomerate.schemas.in
schema_DATA      = $(schema_in_files:.schemas.in=.schemas)
%.schemas:       %.schemas.in       $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -x -u -c $(top_builddir)/po/.intltool-merge-cache


stylesheetdir = $(pkgdatadir)/stylesheets
stylesheet_DATA = selection-to-text.xsl

cssdir = $(pkgdatadir)/css
css_DATA = docbook.css

install-schemas:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	$(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)

EXTRA_DIST = $(desktop_in_files)	\
	$(desktop_DATA)			\
	$(keys_in_files)		\
	$(keys_DATA)			\
	$(mime_DATA)			\
	$(app_DATA)                     \
	$(stylesheet_DATA)		\
	$(css_DATA)                     \
	$(schema_DATA)
