##
## Stuff for Debian packaging.
## The trick is to keep the tarball pristine
## and not to interfear with the normal build and install process.
##

NULL =

# files for building the Debian package
debian_DISTS = \
    changelog \
    compat \
    conglomerate.menu \
    conglomerate_icon.xpm \
    control \
    copyright \
    docs \
    rules \
    watch \
    $(NULL)


# "overhead" files for the deb directory
deb_DISTS = \
    Readme \
    build \
    polish \
    $(NULL)

## but for the GNU autotools they are all the same
EXTRA_DIST = $(debian_DISTS) $(deb_DISTS)

## end of Makefile.am
