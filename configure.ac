dnl ---------------------------------------------
dnl Automake/autoconf input file for Conglomerate
dnl ---------------------------------------------

AC_PREREQ(2.52)

AC_INIT(conglomerate, 0.9.2)
AC_CONFIG_SRCDIR(src/main.c)
AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

AC_CONFIG_HEADERS([config.h])

AM_MAINTAINER_MODE
AC_CANONICAL_HOST

AC_C_CONST
AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
LT_INIT([disable-static])
IT_PROG_INTLTOOL([0.35.0])

GNOME_DOC_INIT

GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Package name for gettext])

ALL_LINGUAS="ar az bg ca cs da de dz el en_CA en_GB es eu fr gl hr ja nb ne nl oc pa pl pt pt_BR ru rw sl sq sr sr@Latn sv tr uk zh_CN"

AM_GLIB_GNU_GETTEXT

dnl --- Package configuration ---

GTK2_REQUIRED=2.20.0
GIO_REQUIRED=2.20.0
LIBXML_REQUIRED=2.0.0
LIBXSLT_REQUIRED=1.0.0
LIBGNOMEPRINT_REQUIRED=1.116.0
LIBGNOMEPRINTUI_REQUIRED=1.116.0
LIBGLADE2_REQUIRED=2.0.0
LIBFO_REQUIRED=0.2.3
SEWFOX_REQUIRED=0.0.0
GCONF2_REQUIRED=1.2.0
GTKSOURCEVIEW_REQUIRED=2.4
ENCHANT_REQUIRED=0.1.0

dnl --- Fundamentals ---

PKG_CHECK_MODULES([CONGLOMERATE], [
  gtk+-2.0 >= $GTK2_REQUIRED
  gio-2.0 >= $GIO_REQUIRED
  gconf-2.0 >= $GCONF2_REQUIRED
  libxml-2.0 >= $LIBXML_REQUIRED
  libxslt >= $LIBXSLT_REQUIRED
  libglade-2.0 >= $LIBGLADE2_REQUIRED])

AC_SUBST(CONGLOMERATE_LIBS)
AC_SUBST(CONGLOMERATE_CFLAGS)

dnl --- Printing support ---

AC_MSG_CHECKING([whether to build with printing support])
AC_ARG_ENABLE(printing, [  --enable-printing       build with printing support [default=no]], enable_printing="$enableval", enable_printing=no)
                                                                                               
if test x$enable_printing = xauto || test x$enable_printing = xyes ; then
   PKG_CHECK_MODULES(PRINTING, \
		    libgnomeprint-2.2 >= $LIBGNOMEPRINT_REQUIRED \
		    libgnomeprintui-2.2 >= $LIBGNOMEPRINTUI_REQUIRED, \
		    have_printing=yes, have_printing=no)
   if test x"$have_printing" = "xyes"; then
         enable_printing=yes
	 PRINTING_CFLAGS="$PRINTING_CFLAGS -DENABLE_PRINTING"
   else
         enable_printing=no 
	 PRINTING_CFLAGS=
	 PRINTING_LIBS=
   fi
else
   enable_printing=no         
   PRINTING_CFLAGS=
   PRINTING_LIBS=
fi

AM_CONDITIONAL(ENABLE_PRINTING, test x$enable_printing = xyes)
AC_SUBST(PRINTING_CFLAGS)
AC_SUBST(PRINTING_LIBS)
AC_MSG_RESULT($enable_printing)

dnl --- libfo support ---

AC_MSG_CHECKING([whether to build with libfo support])
AC_ARG_ENABLE(libfo, [  --enable-libfo          build with libfo support [default=no]], enable_libfo="$enableval", enable_libfo=no)
                                                                                               
if test x$enable_libfo = xauto || test x$enable_libfo = xyes ; then
   PKG_CHECK_MODULES(LIBFO, \
		    libfo-0.2 >= $LIBFO_REQUIRED, \
		    have_libfo=yes, have_libfo=no)
   if test x"$have_libfo" = "xyes"; then
         enable_libfo=yes
	 LIBFO_CFLAGS="$LIBFO_CFLAGS -DENABLE_LIBFO"
   else
         enable_libfo=no 
	 LIBFO_CFLAGS=
	 LIBFO_LIBS=
   fi
else
   enable_libfo=no         
   LIBFO_CFLAGS=
   LIBFO_LIBS=
fi

AM_CONDITIONAL(ENABLE_LIBFO, test x$enable_libfo = xyes)
AC_SUBST(LIBFO_CFLAGS)
AC_SUBST(LIBFO_LIBS)
AC_MSG_RESULT($enable_libfo)

dnl --- sewfox support ---

AC_MSG_CHECKING([whether to build with sewfox support])
AC_ARG_ENABLE(sewfox, [  --enable-sewfox         build with sewfox support [default=no]], enable_sewfox="$enableval", enable_sewfox=no)
                                                                                               
if test x$enable_sewfox = xauto || test x$enable_sewfox = xyes ; then
   PKG_CHECK_MODULES(SEWFOX, \
		    sewfox >= $SEWFOX_REQUIRED, \
		    have_sewfox=yes, have_sewfox=no)
   if test x"$have_sewfox" = "xyes"; then
         enable_sewfox=yes
	 SEWFOX_CFLAGS="$SEWFOX_CFLAGS -DENABLE_SEWFOX"
   else
         enable_sewfox=no 
	 SEWFOX_CFLAGS=
	 SEWFOX_LIBS=
   fi
else
   enable_sewfox=no         
   SEWFOX_CFLAGS=
   SEWFOX_LIBS=
fi

AM_CONDITIONAL(ENABLE_SEWFOX, test x$enable_sewfox = xyes)
AC_SUBST(SEWFOX_CFLAGS)
AC_SUBST(SEWFOX_LIBS)
AC_MSG_RESULT($enable_sewfox)

dnl --- GtkSourceView support ---

AC_MSG_CHECKING([whether to build with GtkSourceView support])
AC_ARG_ENABLE(gtksourceview, [  --enable-gtksourceview  build with GtkSourceView support [default=yes]], enable_gtksourceview="$enableval", enable_gtksourceview=yes)

if test x$enable_gtksourceview = xauto || test x$enable_gtksourceview = xyes ; then
        PKG_CHECK_MODULES(GTKSOURCEVIEW, \
                          gtksourceview-2.0 >= $GTKSOURCEVIEW_REQUIRED, \
                          have_gtksourceview=yes, have_gtksourceview=no)
        if test x"$have_gtksourceview" = "xyes"; then
                enable_gtksourceview=yes
                GTKSOURCEVIEW_CFLAGS="$GTKSOURCEVIEW_CFLAGS -DENABLE_GTKSOURCEVIEW"
        else
                enable_gtksourceview=no
                GTKSOURCEVIEW_CFLAGS=
                GTKSOURCEVIEW_LIBS=
        fi
else
        enable_gtksourceview=no
        GTKSOURCEVIEW_CFLAGS=
        GTKSOURCEVIEW_LIBS=
fi

AM_CONDITIONAL(ENABLE_GTKSOURCEVIEW, test x$enable_gtksourceview = xyes)
AC_SUBST(GTKSOURCEVIEW_CFLAGS)
AC_SUBST(GTKSOURCEVIEW_LIBS)
AC_MSG_RESULT($enable_gtksourceview)

 
dnl --- enchant support ---

AC_MSG_CHECKING([whether to build with enchant support])
AC_ARG_ENABLE(enchant, [  --enable-enchant        build with enchant support [default=yes]], enable_enchant="$enableval", enable_enchant=yes)

if test x$enable_enchant = xauto || test x$enable_enchant = xyes ; then
        PKG_CHECK_MODULES(ENCHANT, \
                          enchant >= $ENCHANT_REQUIRED, \
                          have_enchant=yes, have_enchant=no)
        if test x"$have_enchant" = "xyes"; then
                enable_enchant=yes
                ENCHANT_CFLAGS="$ENCHANT_CFLAGS -DENABLE_ENCHANT"
        else
                enable_enchant=no
                ENCHANT_CFLAGS=
                ENCHANT_LIBS=
        fi
else
        enable_enchant=no
        ENCHANT_CFLAGS=
        ENCHANT_LIBS=
fi

AM_CONDITIONAL(ENABLE_ENCHANT, test x$enable_enchant = xyes)
AC_SUBST(ENCHANT_CFLAGS)
AC_SUBST(ENCHANT_LIBS)
AC_MSG_RESULT($enable_enchant)

 
dnl -- gconf, kudos to metacity
 
AC_ARG_ENABLE(gconf,              [  --disable-gconf         disable gconf usage, effects unknown leave it enabled],,enable_gconf=yes)

if test x$enable_gconf = xyes; then
  AC_PATH_PROG(GCONFTOOL, gconftool-2, no)
  if test x"$GCONFTOOL" = xno; then
    AC_MSG_ERROR([gconftool-2 executable not found in your path - should be installed with GConf])
  fi
  AM_GCONF_SOURCE_2
fi

dnl --- debug plugin support ---

AC_MSG_CHECKING([whether to build debug plugin])
AC_ARG_ENABLE(debug_plugin,
[  --enable-debug-plugin        build debug plugin],
    debug_plugin=$enableval, debug_plugin=no)
AC_MSG_RESULT($debug_plugin)
AM_CONDITIONAL(ENABLE_DEBUG_PLUGIN, test x$enable_debug_plugin = xyes)
 
dnl --- General compilation options ---

AC_MSG_CHECKING([whether to support debugging])
AC_ARG_ENABLE(debug,
[  --enable-debug          enable debugging of application],
    debug=$enableval, debug=no)
AC_MSG_RESULT($debug)

AC_MSG_CHECKING([whether to support profiling])
AC_ARG_ENABLE(profile,
[  --enable-profile        enable profiling of application],
    profile=$enableval, profile=no)
AC_MSG_RESULT($profile)

AC_MSG_CHECKING([whether to optimize])
AC_ARG_ENABLE(optimization,
[  --enable-optimization   enable optimization of application],
    optimize=$enableval, optimize=no)
AC_MSG_RESULT($optimize)

dnl --- bugzilla 134747
AC_MSG_CHECKING([whether to compile with -Werror])
AC_ARG_ENABLE(werror, [  --enable-werror         compile with -Werror [default=no]], enable_werror=$enableval, enable_werror=no)
AM_CONDITIONAL(ENABLE_WERROR, test x$enable_werror = xyes)
AC_MSG_RESULT($enable_werror)

if test "$debug" = yes; then
  BASE_CFLAGS="$BASE_CFLAGS -g"
fi

if test "$profile" = yes; then
  BASE_CFLAGS="$BASE_CFLAGS -pg"
fi

if test "$optimize" = yes; then
  BASE_CFLAGS="$BASE_CFLAGS -O3 -fomit-frame-pointer"

  case "${target}" in
    i486*) BASE_CFLAGS="$BASE_CFLAGS -march=i486 -mcpu=i486"
           ;;
    i586*) BASE_CFLAGS="$BASE_CFLAGS -march=i586 -mcpu=i586"
           ;;
    i686*) BASE_CFLAGS="$BASE_CFLAGS -march=i686 -mcpu=i686"
           ;;
  esac
fi

if test "$enable_werror" = yes; then
  BASE_CFLAGS="$BASE_CFLAGS -Werror"
fi

dnl --- Set compiler flags ---

BASE_CFLAGS="$BASE_CFLAGS -Wall"
AC_SUBST(BASE_CFLAGS)

dnl ---  Check for gtk-doc.

GTK_DOC_CHECK(1.0)

dnl --- Output ---

AC_CONFIG_FILES(Makefile 
	        conglomerate.spec
                po/Makefile.in
                data/Makefile
	        debian/Makefile
	        debian/patches/Makefile
	        dispspecs/Makefile
	        doc/Makefile
	        doc/devel/Makefile
	        doc/reference/Makefile
	        doc/reference/version.xml
	        glade/Makefile
	        pixmaps/Makefile
	        pixmaps/icons/Makefile
		pixmaps/icons/16x16/Makefile
		pixmaps/icons/16x16/apps/Makefile
		pixmaps/icons/32x32/Makefile
		pixmaps/icons/32x32/apps/Makefile
		pixmaps/icons/48x48/Makefile
		pixmaps/icons/48x48/apps/Makefile
		pixmaps/icons/scalable/Makefile
		pixmaps/icons/scalable/apps/Makefile
	        examples/Makefile 
	        templates/Makefile 
                src/Makefile)

AC_OUTPUT

if test x$enable_gconf = xno; then
        echo "*** WARNING WARNING WARNING WARNING WARNING"
        echo "*** Building without GConf"
        echo "*** This means there's no way to change prefs except"
        echo "*** hacking source code, at least for now."
        echo "*** Also, some prefs may have broken defaults."
fi

dnl ==========================================================================
echo "
conglomerate-$VERSION:
 
                          prefix: ${prefix}
            source code location: ${srcdir}
                        compiler: ${CC}

                         -Werror: ${enable_werror}
                           GConf: ${enable_gconf}
                  gtk_doc enable: ${enable_gtk_doc}
                 printing enable: ${enable_printing}
                    libfo enable: ${enable_libfo}
                   sewfox enable: ${enable_sewfox}
            gtksourceview enable: ${enable_gtksourceview}
                  enchant enable: ${enable_enchant}
            debug plugin enabled: ${debug_plugin}
"

dnl -- end of configure.in
