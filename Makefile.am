SUBDIRS = src po data doc dispspecs examples pixmaps glade templates debian

NULL =

INTLTOOL_FILES = \
  intltool-extract.in \
  intltool-merge.in \
  intltool-update.in \
  $(NULL)

EXTRA_DIST = BUGS	\
  README.xml		\
  .diffexclude		\
  conglomerate.spec.in	\
  intltool-extract.in	\
  intltool-merge.in	\
  intltool-update.in	\
  omf.make              \
  xmldocs.make		\
  $(INTLTOOL_FILES)	\
  $(NULL)

DISTCLEANFILES = \
  intltool-extract \
  intltool-merge \
  intltool-update \
  $(NULL)

DISTCHECK_CONFIGURE_FLAGS = \
  --disable-schemas-install \
  --disable-scrollkeeper \
  --enable-gtk-doc \
  $(NULL)

# from pango/Makefile.am
MAINTAINERCLEANFILES = \
  INSTALL \
  aclocal.m4 \
  compile \
  config.guess \
  config.h.in \
  config.sub \
  depcomp \
  install-sh \
  ltmain.sh \
  missing \
  mkinstalldirs \
  omf.make \
  xmldocs.make \
  gtk-doc.make \
  gnome-doc-utils.make

# files to skip during `make dist`
distuninstallcheck_listfiles = find . -type f -print | grep -v scrollkeeper | grep -v /share/gnome/help/ | grep -v \.omf
-include $(top_srcdir)/git.mk
