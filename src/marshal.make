#!/usr/bin/make -f
# run this script by invoking
#
#  ./marshal.make
#
# whenever cong.marshal is changed
#
all: cong-marshal.h cong-marshal.c

cong-marshal.h: cong.marshal
	glib-genmarshal $< --header --prefix=cong_cclosure_marshal > $@

cong-marshal.c: cong.marshal
	glib-genmarshal $< --body --prefix=cong_cclosure_marshal > $@

# end of file
