##
## Stuff for Debian packaging.
##

NULL =

SUBDIRS = patches

# files for building the Debian package
debian_DISTS = \
    changelog \
    compat \
    control \
    copyright \
    rules \
    watch \
    $(NULL)

# package related files
pkg_DISTS = \
    conglomerate.dirs \
    conglomerate.linda-override \
    conglomerate.lintian-override \
    conglomerate.postinst \
    conglomerate.prerm \
    conglomerate-common.dirs \
    conglomerate-common.docs \
    conglomerate-common.lintian-override \
    conglomerate-common.menu \
    conglomerate-dbg.dirs \
    conglomerate_icon.xpm \
    $(NULL)

## but for the GNU autotools they are all the same
EXTRA_DIST = $(debian_DISTS) $(pkg_DISTS)

## end of Makefile.am
