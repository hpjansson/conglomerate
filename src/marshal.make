#!/usr/bin/make -f
all: cong-marshal.h cong-marshal.c

cong-marshal.h: cong.marshal
	glib-genmarshal $< --header --prefix=cong_cclosure_marshal > $@

cong-marshal.c: cong.marshal
	glib-genmarshal $< --body --prefix=cong_cclosure_marshal > $@

# end of file
