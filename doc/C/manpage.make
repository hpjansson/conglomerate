#!/usr/bin/make -f
#
# this script documents how to generate
# the manual page of conglomerate
#

conglomerate.1: conglomerate-refentry.xml
	xmlto man conglomerate-refentry.xml
	@echo Do not forget to CVS commit the manpage.

# That's all folks
