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
    docs \
    rules \
    watch \
    $(NULL)

# package related files
pkg_DISTS = \
    conglomerate.menu \
    conglomerate.postinst \
    conglomerate.prerm \
    conglomerate_icon.xpm \
    $(NULL)

## but for the GNU autotools they are all the same
EXTRA_DIST = $(debian_DISTS) $(pkg_DISTS)

## end of Makefile.am
