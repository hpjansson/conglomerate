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

schemadir = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = @PACKAGE@.schemas

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	$(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)

EXTRA_DIST = $(desktop_in_files)	\
	$(desktop_DATA)			\
	$(keys_in_files)		\
	$(keys_DATA)			\
	$(mime_DATA)			\
	$(app_DATA)
