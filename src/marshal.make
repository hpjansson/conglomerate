#!/usr/bin/make -f
# run this script by invoking
#
#  ./marshal.make
#
# whenever cong.marshal is changed
#
# delete this file when
# http://bugzilla.gnome.org/show_bug.cgi?id=137248
# is fixed
all: cong-marshal.h cong-marshal.c

cong-marshal.h: cong.marshal
	glib-genmarshal $< --header --prefix=cong_cclosure_marshal > $@

cong-marshal.c: cong.marshal
	glib-genmarshal $< --body --prefix=cong_cclosure_marshal > $@

# end of file
