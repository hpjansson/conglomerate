desktopdir 	 = $(datadir)/applications
desktop_in_files = conglomerate.desktop.in
desktop_DATA 	 = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@


## bugzilla 122011 for catalog.xml

## bugzilla 125579 for conglomerate.schemas


EXTRA_DIST = $(desktop_in_files)	\
	$(desktop_DATA)			
	
