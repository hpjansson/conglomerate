SUBDIRS = src po data doc dispspecs examples pixmaps glade templates debian

NULL =

EXTRA_DIST = BUGS	\
  README.xml            \
  .diffexclude		\
  conglomerate.spec.in	\
  intltool-extract.in	\
  intltool-merge.in	\
  intltool-update.in	\
  omf.make              \
  xmldocs.make          \
  $(NULL)

## # when we switch to automake1.7, we will need this:
##distuninstallcheck_listfiles = find . \
##   -type f -print | \
##   grep -v '^\./var/scrollkeeper' | \
##   grep -v '^\./share/gnome/help/conglomerate/C/faq.xml'
## # but it will need more to get a fine `make distcheck`

DISTCHECK_CONFIGURE_FLAGS = --disable-schemas-install --disable-scrollkeeper --enable-gtk-doc
